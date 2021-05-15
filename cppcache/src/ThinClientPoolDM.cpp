/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ThinClientPoolDM.hpp"

#include <algorithm>
#include <thread>

#include <geode/AuthInitialize.hpp>
#include <geode/PoolManager.hpp>
#include <geode/ResultCollector.hpp>
#include <geode/SystemProperties.hpp>

#include "CacheImpl.hpp"
#include "DistributedSystemImpl.hpp"
#include "ExecutionImpl.hpp"
#include "FunctionExpiryTask.hpp"
#include "TcrConnectionManager.hpp"
#include "TcrEndpoint.hpp"
#include "ThinClientRegion.hpp"
#include "ThinClientStickyManager.hpp"
#include "UserAttributes.hpp"
#include "statistics/PoolStatsSampler.hpp"
#include "util/exception.hpp"

/** Closes and Deletes connection only if it exists */
#define GF_SAFE_DELETE_CON(x) \
  do {                        \
    x->close();               \
    delete x;                 \
    x = nullptr;              \
  } while (0)

namespace apache {
namespace geode {
namespace client {

class GetAllWork : public PooledWork<GfErrType> {
  ThinClientPoolDM* m_poolDM;
  std::shared_ptr<BucketServerLocation> m_serverLocation;
  TcrMessage* m_request;
  TcrMessageReply* m_reply;
  bool m_attemptFailover;
  bool m_isBGThread;
  bool m_addToLocalCache;
  std::shared_ptr<UserAttributes> m_userAttribute;
  ChunkedGetAllResponse* m_responseHandler;
  std::string m_regionName;
  const std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> m_keys;
  const std::shared_ptr<Region> m_region;
  TcrChunkedResult* m_resultCollector;
  const std::shared_ptr<Serializable>& m_aCallbackArgument;

 public:
  GetAllWork(const GetAllWork&) = delete;
  GetAllWork& operator=(const GetAllWork&) = delete;
  GetAllWork(
      ThinClientPoolDM* poolDM, const std::shared_ptr<Region>& region,
      const std::shared_ptr<BucketServerLocation>& serverLocation,
      const std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>>& keys,
      bool attemptFailover, bool isBGThread, bool addToLocalCache,
      ChunkedGetAllResponse* responseHandler,
      const std::shared_ptr<Serializable>& aCallbackArgument)
      : m_poolDM(poolDM),
        m_serverLocation(serverLocation),
        m_attemptFailover(attemptFailover),
        m_isBGThread(isBGThread),
        m_addToLocalCache(addToLocalCache),
        m_userAttribute(nullptr),
        m_responseHandler(responseHandler),
        m_regionName(region->getFullPath()),
        m_keys(keys),
        m_region(region),
        m_aCallbackArgument(aCallbackArgument) {
    m_request = new TcrMessageGetAll(
        new DataOutput(region->getCache().createDataOutput()), region.get(),
        m_keys.get(), m_poolDM, m_aCallbackArgument);
    m_reply = new TcrMessageReply(true, m_poolDM);
    if (m_poolDM->isMultiUserMode()) {
      m_userAttribute = UserAttributes::threadLocalUserAttributes;
    }

    m_resultCollector = (new ChunkedGetAllResponse(
        *m_reply, dynamic_cast<ThinClientRegion*>(m_region.get()), m_keys.get(),
        m_responseHandler->getValues(), m_responseHandler->getExceptions(),
        m_responseHandler->getResultKeys(),
        m_responseHandler->getUpdateCounters(), 0, m_addToLocalCache,
        m_responseHandler->getResponseLock()));

    m_reply->setChunkedResultHandler(m_resultCollector);
  }

  ~GetAllWork() override {
    delete m_request;
    delete m_reply;
    delete m_resultCollector;
  }

  TcrMessage* getReply() { return m_reply; }

  void init() {}
  GfErrType execute() override {
    GuardUserAttributes gua;

    if (m_userAttribute != nullptr) {
      gua.setAuthenticatedView(m_userAttribute->getAuthenticatedView());
    }
    m_request->InitializeGetallMsg(
        m_request->getCallbackArgument());  // now init getall msg
    return m_poolDM->sendSyncRequest(*m_request, *m_reply, m_attemptFailover,
                                     m_isBGThread, m_serverLocation);
  }
};

const char* ThinClientPoolDM::NC_Ping_Thread = "NC Ping Thread";
const char* ThinClientPoolDM::NC_MC_Thread = "NC MC Thread";
#define PRIMARY_QUEUE_NOT_AVAILABLE -2

ThinClientPoolDM::ThinClientPoolDM(const char* name,
                                   std::shared_ptr<PoolAttributes> poolAttrs,
                                   TcrConnectionManager& connManager)
    : ThinClientBaseDM(connManager, nullptr),
      Pool(poolAttrs),
      m_poolName(name),
      m_stats(nullptr),
      m_sticky(false),
      update_locators_semaphore_(0),
      ping_semaphore_(0),
      m_isDestroyed(false),
      m_destroyPending(false),
      m_destroyPendingHADM(false),
      m_isMultiUserMode(false),
      m_locHelper(nullptr),
      m_poolSize(0),
      m_numRegions(0),
      m_server(0),
      conn_semaphore_(0),
      m_connManageTask(nullptr),
      m_pingTask(nullptr),
      m_updateLocatorListTask(nullptr),
      m_clientOps(0),
      connected_endpoints_(0),
      m_PoolStatsSampler(nullptr),
      m_clientMetadataService(nullptr),
      m_primaryServerQueueSize(PRIMARY_QUEUE_NOT_AVAILABLE) {
  static bool firstGuard = false;
  if (firstGuard) {
    ClientProxyMembershipID::increaseSynchCounter();
  }
  firstGuard = true;

  auto cacheImpl = m_connManager.getCacheImpl();
  auto& distributedSystem = cacheImpl->getDistributedSystem();

  auto& props = distributedSystem.getSystemProperties();
  // to set security flag at pool level
  m_isSecurityOn = cacheImpl->getAuthInitialize() != nullptr;

  const auto& durableId = props.durableClientId();

  std::string clientDurableId = durableId;
  if (!m_poolName.empty()) {
    clientDurableId += "_gem_" + m_poolName;
  }

  const auto durableTimeOut = props.durableTimeout();
  m_memId = cacheImpl->getClientProxyMembershipIDFactory().create(
      clientDurableId.c_str(), durableTimeOut);

  if (m_attrs->m_initLocList.empty() && m_attrs->m_initServList.empty()) {
    std::string msg = "No locators or servers provided for pool named ";
    msg += name;
    throw IllegalStateException(msg);
  }
  reset();
  m_locHelper = new ThinClientLocatorHelper(m_attrs->m_initLocList,
                                            m_attrs->m_sniProxyHost,
                                            m_attrs->m_sniProxyPort, this);

  m_stats = new PoolStats(
      cacheImpl->getStatisticsManager().getStatisticsFactory(), m_poolName);
  cacheImpl->getStatisticsManager().forceSample();

  if (!props.isEndpointShufflingDisabled()) {
    if (!m_attrs->m_initServList.empty()) {
      RandGen randgen;
      m_server = randgen(static_cast<uint32_t>(m_attrs->m_initServList.size()));
    }
  }
  if (m_attrs->getPRSingleHopEnabled()) {
    m_clientMetadataService =
        std::unique_ptr<ClientMetadataService>(new ClientMetadataService(this));
  }
  m_manager = new ThinClientStickyManager(this);

  clear_pdx_registry_ = props.onClientDisconnectClearPdxTypeIds();
}

void ThinClientPoolDM::init() {
  LOG_DEBUG("ThinClientPoolDM::init: Starting pool initialization");
  auto cacheImpl = m_connManager.getCacheImpl();
  m_isMultiUserMode = getMultiuserAuthentication();

  if (m_isMultiUserMode) {
    LOG_INFO("Multiuser authentication is enabled for pool %s",
             m_poolName.c_str());
  }
  // to set security flag at pool level
  m_isSecurityOn = cacheImpl->getAuthInitialize() != nullptr;

  LOG_DEBUG("ThinClientPoolDM::init: security in on/off = %d ", m_isSecurityOn);

  m_connManager.init(true);

  ThinClientPoolDM::startBackgroundThreads();

  LOG_DEBUG("ThinClientPoolDM::init: Completed initialization");
}

ThinClientPoolDM::~ThinClientPoolDM() {
  // TODO suspect
  // NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.VirtualCall)
  destroy();
  _GEODE_SAFE_DELETE(m_locHelper);
  _GEODE_SAFE_DELETE(m_stats);
  _GEODE_SAFE_DELETE(m_manager);
}

std::shared_ptr<Properties> ThinClientPoolDM::getCredentials(TcrEndpoint* ep) {
  auto cacheImpl = m_connManager.getCacheImpl();
  const auto& distributedSystem = cacheImpl->getDistributedSystem();
  const auto& tmpSecurityProperties =
      distributedSystem.getSystemProperties().getSecurityProperties();

  if (const auto& authInitialize = cacheImpl->getAuthInitialize()) {
    LOG_FINER(
        "ThinClientPoolDM::getCredentials: acquired handle to authLoader, "
        "invoking getCredentials %s",
        ep->name().c_str());
    const auto& tmpAuthIniSecurityProperties =
        authInitialize->getCredentials(tmpSecurityProperties, ep->name());
    LOG_FINER("Done getting credentials");
    return tmpAuthIniSecurityProperties;
  }

  return nullptr;
}

void ThinClientPoolDM::startBackgroundThreads() {
  auto& props = m_connManager.getCacheImpl()
                    ->getDistributedSystem()
                    .getSystemProperties();

  LOG_DEBUG("ThinClientPoolDM::startBackgroundThreads: Starting ping thread");
  m_pingTask =
      std::unique_ptr<Task<ThinClientPoolDM>>(new Task<ThinClientPoolDM>(
          this, &ThinClientPoolDM::pingServer, NC_Ping_Thread));
  m_pingTask->start();

  auto interval = getPingInterval();
  if (interval > std::chrono::seconds::zero()) {
    LOG_DEBUG(
        "ThinClientPoolDM::startBackgroundThreads: Scheduling ping task at %zu",
        interval.count());
    auto& manager = m_connManager.getCacheImpl()->getExpiryTaskManager();
    auto task = std::make_shared<FunctionExpiryTask>(
        manager, [this] { ping_semaphore_.release(); });
    ping_task_id_ =
        manager.schedule(std::move(task), std::chrono::seconds(1), interval);
  } else {
    LOG_DEBUG(
        "ThinClientPoolDM::startBackgroundThreads: Not Scheduling ping task as "
        "ping interval %ld",
        interval.count());
  }

  interval = getUpdateLocatorListInterval();
  if (interval > std::chrono::seconds::zero()) {
    m_updateLocatorListTask =
        std::unique_ptr<Task<ThinClientPoolDM>>(new Task<ThinClientPoolDM>(
            this, &ThinClientPoolDM::updateLocatorList, "NC_LocatorList"));
    m_updateLocatorListTask->start();

    LOG_DEBUG(
        "ThinClientPoolDM::startBackgroundThreads: Creating updateLocatorList "
        "task");
    auto& manager = m_connManager.getCacheImpl()->getExpiryTaskManager();
    auto task = std::make_shared<FunctionExpiryTask>(
        manager, [this] { update_locators_semaphore_.release(); });

    LOG_DEBUG(
        "ThinClientPoolDM::startBackgroundThreads: Scheduling updater Locator "
        "task at %ld",
        interval.count());
    update_locators_task_id_ =
        manager.schedule(std::move(task), std::chrono::seconds(1), interval);
  }

  LOG_DEBUG(
      "ThinClientPoolDM::startBackgroundThreads: Starting manageConnections "
      "thread");
  // Manage Connection Thread
  m_connManageTask =
      std::unique_ptr<Task<ThinClientPoolDM>>(new Task<ThinClientPoolDM>(
          this, &ThinClientPoolDM::manageConnections, NC_MC_Thread));
  m_connManageTask->start();

  auto idle = getIdleTimeout();
  auto load = getLoadConditioningInterval();

  if (load > std::chrono::milliseconds::zero() &&
      (load < idle || idle <= std::chrono::milliseconds::zero())) {
    idle = load;
  }

  if (idle > std::chrono::milliseconds::zero()) {
    LOG_DEBUG(
        "ThinClientPoolDM::startBackgroundThreads: Starting manageConnections "
        "task");
    auto& manager = m_connManager.getCacheImpl()->getExpiryTaskManager();
    auto task = std::make_shared<FunctionExpiryTask>(
        manager, [this] { conn_semaphore_.release(); });

    LOG_DEBUG(
        "ThinClientPoolDM::startBackgroundThreads: Scheduling "
        "manageConnections task");
    conns_mgmt_task_id_ =
        manager.schedule(std::move(task), std::chrono::seconds(1), idle);
  }

  LOG_DEBUG(
      "ThinClientPoolDM::startBackgroundThreads: Starting remote query "
      "service");
  // Init Query Service
  m_remoteQueryServicePtr =
      std::make_shared<RemoteQueryService>(m_connManager.getCacheImpl(), this);
  m_remoteQueryServicePtr->init();

  LOG_DEBUG(
      "ThinClientPoolDM::startBackgroundThreads: Starting pool stat sampler");
  if (!m_PoolStatsSampler &&
      getStatisticInterval() > std::chrono::milliseconds::zero() &&
      props.statisticsEnabled()) {
    m_PoolStatsSampler = std::unique_ptr<statistics::PoolStatsSampler>(
        new statistics::PoolStatsSampler(getStatisticInterval(),
                                         m_connManager.getCacheImpl(), this));
    m_PoolStatsSampler->start();
  }

  // starting chunk processing helper thread
  ThinClientBaseDM::init();

  if (m_clientMetadataService) {
    m_clientMetadataService->start();
  }
}
void ThinClientPoolDM::manageConnections(std::atomic<bool>& isRunning) {
  LOG_FINE("ThinClientPoolDM: starting manageConnections thread");

  conn_semaphore_.acquire();

  while (isRunning) {
    try {
      LOG_FINE(
          "ThinClientPoolDM::manageConnections: checking connections in "
          "pool");

      manageConnectionsInternal(isRunning);
    } catch (const Exception& e) {
      LOG_ERROR("ThinClientPoolDM::manageConnections: Geode Exception: \"%s\"",
                e.what());
      LOG_ERROR(e.getStackTrace());
    } catch (const std::exception& e) {
      LOG_ERROR(
          "ThinClientPoolDM::manageConnections: Standard exception: \"%s\"",
          e.what());
    } catch (...) {
      LOG_ERROR("ThinClientPoolDM::manageConnections: Unexpected exception");
    }

    conn_semaphore_.acquire();
  }

  LOG_FINE("ThinClientPoolDM: ending manageConnections thread");
}

void ThinClientPoolDM::cleanStaleConnections(std::atomic<bool>& isRunning) {
  if (!isRunning) {
    return;
  }

  LOG_DEBUG("Cleaning stale connections");

  auto _idle = getIdleTimeout();
  auto _nextIdle = _idle;

  std::vector<TcrConnection*> removelist;
  std::set<ServerLocation> excludeServers;

  auto availableConns = size();
  auto savedConns = 0;

  for (unsigned int i = 0; (i < availableConns) && isRunning; i++) {
    auto* conn = getNoWait();
    if (conn == nullptr) {
      break;
    }
    if (canItBeDeleted(conn)) {
      removelist.push_back(conn);
    } else if (conn) {
      auto nextIdle =
          _idle -
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - conn->getLastAccessed());
      if (nextIdle > std::chrono::seconds::zero() && nextIdle < _nextIdle) {
        _nextIdle = nextIdle;
      }
      put(conn, false);
      savedConns++;
    }
  }

  auto replaceCount = m_attrs->getMinConnections() - savedConns;

  LOG_DEBUG("Preserving %d connections", savedConns);

  int count = 0;

  for (std::vector<TcrConnection*>::const_iterator iter = removelist.begin();
       iter != removelist.end(); ++iter) {
    TcrConnection* conn = *iter;
    if (replaceCount <= 0) {
      try {
        GF_SAFE_DELETE_CON(conn);
      } catch (...) {
      }
      removeEPConnections(1, false);
      getStats().incLoadCondDisconnects();
      LOG_DEBUG("Removed a connection");
    } else {
      TcrConnection* newConn = nullptr;
      bool maxConnLimit = false;
      createPoolConnection(newConn, excludeServers, maxConnLimit,
                           /*hasExpired(conn) ? nullptr :*/ conn);
      if (newConn) {
        auto nextIdle =
            _idle -
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - conn->getLastAccessed());
        if (nextIdle > std::chrono::seconds::zero() && nextIdle < _nextIdle) {
          _nextIdle = nextIdle;
        }
        put(newConn, false);
        if (newConn != conn) {
          try {
            GF_SAFE_DELETE_CON(conn);
          } catch (...) {
          }
          removeEPConnections(1, false);
          getStats().incLoadCondDisconnects();
          LOG_DEBUG("Removed a connection");
        }
      } else {
        if (hasExpired(conn)) {
          try {
            GF_SAFE_DELETE_CON(conn);
          } catch (...) {
          }
          removeEPConnections(1, false);
          getStats().incLoadCondDisconnects();
          LOG_DEBUG("Removed a connection");
        } else {
          conn->updateCreationTime();
          auto nextIdle =
              _idle -
              std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::steady_clock::now() - conn->getLastAccessed());
          if (nextIdle > std::chrono::seconds::zero() && nextIdle < _nextIdle) {
            _nextIdle = nextIdle;
          }
          put(conn, false);
        }
      }
    }
    replaceCount--;
    count++;

    if (count % 10 == 0) {
      std::this_thread::yield();
    }
  }

  if (isRunning) {
    auto& manager = m_connManager.getCacheImpl()->getExpiryTaskManager();
    if (manager.reset(conns_mgmt_task_id_, _nextIdle) < 0) {
      LOG_ERROR("Failed to reschedule connection manager");
    } else {
      LOG_FINEST("Rescheduled next connection manager run after %s",
                 to_string(_nextIdle).c_str());
    }
  }

  LOG_DEBUG("Pool size is %zu, pool counter is %d", size(), m_poolSize.load());
}

void ThinClientPoolDM::cleanStickyConnections(std::atomic<bool>&) {}

void ThinClientPoolDM::restoreMinConnections(std::atomic<bool>& isRunning) {
  if (!isRunning) {
    return;
  }

  LOG_DEBUG("Restoring minimum connection level");

  int min = m_attrs->getMinConnections();
  int limit = 2 * min;

  std::set<ServerLocation> excludeServers;

  int restored = 0;

  if (m_poolSize < min) {
    while (m_poolSize < min && limit-- && isRunning) {
      TcrConnection* conn = nullptr;
      bool maxConnLimit = false;
      createPoolConnection(conn, excludeServers, maxConnLimit);
      if (conn) {
        put(conn, false);
        restored++;
        getStats().incMinPoolSizeConnects();
      }
    }
  }

  LOG_DEBUG("Restored %d connections", restored);
  LOG_DEBUG("Pool size is %zu, pool counter is %d", size(), m_poolSize.load());
}

void ThinClientPoolDM::manageConnectionsInternal(std::atomic<bool>& isRunning) {
  try {
    LOG_FINE(
        "ThinClientPoolDM::manageConnections(): checking connections in pool "
        "queue %zu",
        size());

    cleanStaleConnections(isRunning);

    cleanStickyConnections(isRunning);

    restoreMinConnections(isRunning);

    getStats().setCurPoolConnections(m_poolSize);
  } catch (const Exception& e) {
    LOG_ERROR(e.what());
  } catch (const std::exception& e) {
    LOG_ERROR(e.what());
  } catch (...) {
    LOG_ERROR("Unexpected exception during manage connections");
  }
}

std::string ThinClientPoolDM::selectEndpoint(
    std::set<ServerLocation>& excludeServers,
    const TcrConnection* currentServer) {
  if (!m_attrs->m_initLocList.empty()) {  // query locators
    ServerLocation outEndpoint;
    std::string additionalLoc;
    LOG_FINE("Asking locator for server from group [%s]",
             m_attrs->m_serverGrp.c_str());

    // Update Locator Request Stats
    getStats().incLoctorRequests();

    if (GF_NOERR != (m_locHelper)
                        ->getEndpointForNewFwdConn(
                            outEndpoint, additionalLoc, excludeServers,
                            m_attrs->m_serverGrp, currentServer)) {
      throw IllegalStateException("Locator query failed selecting an endpoint");
    }
    // Update Locator stats
    getStats().setLocators(
        static_cast<int32_t>(m_locHelper->getCurLocatorsNum()));
    getStats().incLoctorResposes();

    std::string epNameStr = outEndpoint.getServerName() + ":" +
                            std::to_string(outEndpoint.getPort());
    LOG_FINE("ThinClientPoolDM: Locator returned endpoint [%s]",
             epNameStr.c_str());
    return epNameStr;
  } else if (!m_attrs->m_initServList.empty()) {
    // use specified server endpoints
    // highly complex round-robin algorithm
    std::lock_guard<decltype(m_endpointSelectionLock)> _guard(
        m_endpointSelectionLock);
    if (m_server >= m_attrs->m_initServList.size()) {
      m_server = 0;
    }

    unsigned int epCount = 0;
    do {
      if (!excludeServer(m_attrs->m_initServList[m_server], excludeServers)) {
        LOG_FINE("ThinClientPoolDM: Selecting endpoint [%s] from position %d",
                 m_attrs->m_initServList[m_server].c_str(), m_server);
        return m_attrs->m_initServList[m_server++];
      } else {
        if (++m_server >= m_attrs->m_initServList.size()) {
          m_server = 0;
        }
      }
    } while (++epCount < m_attrs->m_initServList.size());

    throw NotConnectedException("No server endpoints are available.");
  } else {
    LOG_ERROR("No locators or servers provided");
    throw IllegalStateException("No locators or servers provided");
  }
}

void ThinClientPoolDM::addConnection(TcrConnection* conn) {
  std::lock_guard<decltype(mutex_)> lock(mutex_);
  put(conn, false);
  ++m_poolSize;
}

GfErrType ThinClientPoolDM::sendRequestToAllServers(
    const char* func, uint8_t getResult, std::chrono::milliseconds timeout,
    std::shared_ptr<Cacheable> args, std::shared_ptr<ResultCollector>& rs,
    std::shared_ptr<CacheableString>& exceptionPtr) {
  getStats().setCurClientOps(++m_clientOps);

  auto resultCollectorLock = std::make_shared<std::recursive_mutex>();

  auto csArray = getServers();

  if (csArray != nullptr && csArray->length() == 0) {
    LOG_WARN("No server found to execute the function");
    return GF_NOSERVER_FOUND;
  }

  std::vector<std::shared_ptr<FunctionExecution>> fePtrList;
  fePtrList.reserve(csArray->length());
  auto& threadPool = m_connManager.getCacheImpl()->getThreadPool();
  auto userAttr = UserAttributes::threadLocalUserAttributes;
  for (int i = 0; i < csArray->length(); i++) {
    auto cs = (*csArray)[i];
    auto endpointStr = cs->value();
    auto ep = getEndpoint(endpointStr);
    if (!ep) {
      ep = addEP(cs->value());
    } else if (!ep->connected()) {
      LOG_FINE(
          "ThinClientPoolDM::sendRequestToAllServers server not connected "
          "%s ",
          cs->value().c_str());
    }
    auto funcExe = std::make_shared<FunctionExecution>();
    funcExe->setParameters(func, getResult, timeout, args, ep.get(), this,
                           resultCollectorLock, &rs, userAttr);
    fePtrList.push_back(funcExe);
    threadPool.perform(funcExe);
  }
  GfErrType finalErrorReturn = GF_NOERR;

  for (auto& funcExe : fePtrList) {
    auto err = funcExe->getResult();
    if (err != GF_NOERR) {
      if (funcExe->getException() == nullptr) {
        if (err == GF_TIMEOUT) {
          getStats().incTimeoutClientOps();
        } else {
          getStats().incFailedClientOps();
        }
        if (err == GF_IOERR) {
          err = GF_NOTCON;
        }
      } else {
        exceptionPtr = funcExe->getException();
      }
    }

    if (err == GF_AUTHENTICATION_FAILED_EXCEPTION ||
        err == GF_NOT_AUTHORIZED_EXCEPTION ||
        err == GF_AUTHENTICATION_REQUIRED_EXCEPTION) {
      finalErrorReturn = err;
    } else if ((err != GF_NOERR) &&
               (!(finalErrorReturn == GF_AUTHENTICATION_FAILED_EXCEPTION ||
                  finalErrorReturn == GF_NOT_AUTHORIZED_EXCEPTION ||
                  finalErrorReturn ==
                      GF_AUTHENTICATION_REQUIRED_EXCEPTION)))  // returning auth
                                                               // errors
    // to client..preference
    // over other errors..
    {
      finalErrorReturn = err;
    }
  }

  if (static_cast<uint8_t>(getResult & 2) == static_cast<uint8_t>(2)) {
    rs->endResults();
  }

  getStats().setCurClientOps(--m_clientOps);
  getStats().incSucceedClientOps();

  return finalErrorReturn;
}

const std::shared_ptr<CacheableStringArray> ThinClientPoolDM::getLocators()
    const {
  std::vector<std::shared_ptr<CacheableString>> locators;
  locators.reserve(m_attrs->m_initLocList.size());
  for (auto&& locator : m_attrs->m_initLocList) {
    locators.emplace_back(CacheableString::create(locator));
  }
  return CacheableStringArray::create(std::move(locators));
}

const std::shared_ptr<CacheableStringArray> ThinClientPoolDM::getServers() {
  std::vector<std::shared_ptr<CacheableString>> servers;
  if (!m_attrs->m_initServList.empty()) {
    servers.reserve(m_attrs->m_initServList.size());

    for (auto&& server : m_attrs->m_initServList) {
      servers.emplace_back(CacheableString::create(server));
    }
    return CacheableStringArray::create(std::move(servers));
  } else if (!m_attrs->m_initLocList.empty()) {
    std::vector<std::shared_ptr<ServerLocation>> vec;
    m_locHelper->getAllServers(vec, m_attrs->m_serverGrp);

    servers.reserve(vec.size());
    for (auto&& serLoc : vec) {
      servers.emplace_back(CacheableString::create(
          serLoc->getServerName() + ":" + std::to_string(serLoc->getPort())));
    }
    return CacheableStringArray::create(std::move(servers));
  }
  return CacheableStringArray::create(std::move(servers));
}

void ThinClientPoolDM::stopPingThread() {
  if (m_pingTask) {
    LOG_FINE("ThinClientPoolDM::destroy(): Closing ping thread.");
    m_pingTask->stopNoblock();
    ping_semaphore_.release();
    m_pingTask->wait();
    m_pingTask = nullptr;
    if (ping_task_id_ != ExpiryTask::invalid()) {
      auto& manager = m_connManager.getCacheImpl()->getExpiryTaskManager();
      manager.cancel(ping_task_id_);
    }
  }
}

void ThinClientPoolDM::stopUpdateLocatorListThread() {
  if (m_updateLocatorListTask) {
    LOG_FINE("ThinClientPoolDM::destroy(): Closing updateLocatorList thread.");
    m_updateLocatorListTask->stopNoblock();
    update_locators_semaphore_.release();
    m_updateLocatorListTask->wait();
    m_updateLocatorListTask = nullptr;
    if (update_locators_task_id_ != ExpiryTask::invalid()) {
      auto& manager = m_connManager.getCacheImpl()->getExpiryTaskManager();
      manager.cancel(update_locators_task_id_);
    }
  }
}

void ThinClientPoolDM::destroy(bool keepAlive) {
  LOG_DEBUG("ThinClientPoolDM::destroy...");
  if (!m_isDestroyed && (!m_destroyPending || m_destroyPendingHADM)) {
    checkRegions();

    m_keepAlive = keepAlive;

    if (m_remoteQueryServicePtr != nullptr) {
      m_remoteQueryServicePtr->close();
      m_remoteQueryServicePtr = nullptr;
    }

    LOG_DEBUG("Closing PoolStatsSampler thread.");
    if (m_PoolStatsSampler) {
      m_PoolStatsSampler->stop();
      m_PoolStatsSampler = nullptr;
    }
    LOG_DEBUG("PoolStatsSampler thread closed .");
    LOG_DEBUG("ThinClientPoolDM::destroy( ): Closing connection manager.");
    auto cacheImpl = m_connManager.getCacheImpl();
    if (m_connManageTask) {
      m_connManageTask->stopNoblock();
      conn_semaphore_.release();
      m_connManageTask->wait();
      m_connManageTask = nullptr;
      cacheImpl->getExpiryTaskManager().cancel(conns_mgmt_task_id_);
    }

    LOG_DEBUG("Closing PoolStatsSampler thread.");
    // TODO suspect
    // NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.VirtualCall)
    stopPingThread();
    // TODO suspect
    // NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.VirtualCall)
    stopUpdateLocatorListThread();

    if (m_clientMetadataService) {
      m_clientMetadataService->stop();
      // m_clientMetadataService = nullptr;
    }
    // closing all the thread local connections ( sticky).
    LOG_DEBUG(
        "ThinClientPoolDM::destroy( ): closing ConnectionQueue, pool size = "
        "%d",
        m_poolSize.load());
    close();
    LOG_DEBUG("ThinClientPoolDM::destroy( ): after close ");

    // Close Stats
    // TODO suspect
    // NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.VirtualCall)
    getStats().close();
    cacheImpl->getStatisticsManager().forceSample();

    cacheImpl->getPoolManager().removePool(m_poolName);

    stopChunkProcessor();
    m_manager->closeAllStickyConnections();
    m_isDestroyed = true;
    LOG_DEBUG("ThinClientPoolDM::destroy( ): after close m_isDestroyed = %d ",
              m_isDestroyed);
  }
  if (m_poolSize != 0) {
    LOG_FINE("Pool connection size is not zero %d", m_poolSize.load());
  }
}

bool ThinClientPoolDM::isDestroyed() const {
  // TODO: dummy implementation
  return m_isDestroyed;
}
std::shared_ptr<QueryService> ThinClientPoolDM::getQueryService() {
  // TODO:
  if (m_isMultiUserMode) {
    LOG_ERROR(
        "Pool is in multiuser authentication mode. Get query service using "
        "RegionService.getQueryService()");
    throw UnsupportedOperationException(
        "Pool is in multiuser authentication mode. Get QueryService() using "
        "RegionService.getQueryService()");
  }

  return getQueryServiceWithoutCheck();
}
std::shared_ptr<QueryService> ThinClientPoolDM::getQueryServiceWithoutCheck() {
  if (!(m_remoteQueryServicePtr == nullptr)) {
    return m_remoteQueryServicePtr;
  }

  LOG_WARN("Remote query service is not initialized.");

  return m_remoteQueryServicePtr;
}
void ThinClientPoolDM::sendUserCacheCloseMessage(bool keepAlive) {
  LOG_DEBUG("ThinClientPoolDM::sendUserCacheCloseMessage");
  auto userAttribute = UserAttributes::threadLocalUserAttributes;

  auto& ucs = userAttribute->getUserConnectionServers();

  for (const auto& it : ucs) {
    auto uca = it.second;
    if (uca->isAuthenticated() && uca->getEndpoint()->connected()) {
      TcrMessageRemoveUserAuth request(
          new DataOutput(m_connManager.getCacheImpl()->createDataOutput()),
          keepAlive, this);
      TcrMessageReply reply(true, this);

      sendRequestToEP(request, reply, uca->getEndpoint());

      uca->setUnAuthenticated();
    } else {
      uca->setUnAuthenticated();
    }
  }
}

int32_t ThinClientPoolDM::GetPDXIdForType(
    std::shared_ptr<Serializable> pdxType) {
  LOG_DEBUG("ThinClientPoolDM::GetPDXIdForType:");

  TcrMessageGetPdxIdForType request(
      new DataOutput(m_connManager.getCacheImpl()->createDataOutput()), pdxType,
      this);

  TcrMessageReply reply(true, this);

  auto err = sendSyncRequest(request, reply);
  throwExceptionIfError("Operation Failed", err);

  if (reply.getMessageType() == TcrMessage::EXCEPTION) {
    LOG_DEBUG("ThinClientPoolDM::GetPDXTypeById: Exception = " +
              reply.getException());
    throw IllegalStateException("Failed to register PdxSerializable Type");
  }

  auto pdxTypeId =
      std::dynamic_pointer_cast<CacheableInt32>(reply.getValue())->value();

  // need to broadcast this id to all other pool
  {
    auto& poolManager = m_connManager.getCacheImpl()->getPoolManager();
    for (const auto& iter : poolManager.getAll()) {
      auto currPool = static_cast<ThinClientPoolDM*>(iter.second.get());

      if (currPool != this) {
        currPool->AddPdxType(pdxType, pdxTypeId);
      }
    }
  }

  return pdxTypeId;
}

void ThinClientPoolDM::AddPdxType(std::shared_ptr<Serializable> pdxType,
                                  int32_t pdxTypeId) {
  LOG_DEBUG("ThinClientPoolDM::GetPDXIdForType:");

  TcrMessageAddPdxType request(
      new DataOutput(m_connManager.getCacheImpl()->createDataOutput()), pdxType,
      this, pdxTypeId);

  TcrMessageReply reply(true, this);

  throwExceptionIfError("Operation Failed", sendSyncRequest(request, reply));

  if (reply.getMessageType() == TcrMessage::EXCEPTION) {
    LOG_DEBUG("ThinClientPoolDM::GetPDXTypeById: Exception = " +
              reply.getException());
    throw IllegalStateException("Failed to register PdxSerializable Type");
  }
}
std::shared_ptr<Serializable> ThinClientPoolDM::GetPDXTypeById(int32_t typeId) {
  LOG_DEBUG("ThinClientPoolDM::GetPDXTypeById:");

  TcrMessageGetPdxTypeById request(
      new DataOutput(m_connManager.getCacheImpl()->createDataOutput()), typeId,
      this);

  TcrMessageReply reply(true, this);

  throwExceptionIfError("Operation Failed", sendSyncRequest(request, reply));

  if (reply.getMessageType() == TcrMessage::EXCEPTION) {
    LOG_DEBUG("ThinClientPoolDM::GetPDXTypeById: Exception = " +
              reply.getException());
    throw IllegalStateException("Failed to understand PdxSerializable Type");
  }

  return reply.getValue();
}

int32_t ThinClientPoolDM::GetEnumValue(std::shared_ptr<Serializable> enumInfo) {
  LOG_DEBUG("ThinClientPoolDM::GetEnumValue:");

  TcrMessageGetPdxIdForEnum request(
      new DataOutput(m_connManager.getCacheImpl()->createDataOutput()),
      enumInfo, this);

  TcrMessageReply reply(true, this);

  throwExceptionIfError("Operation Failed", sendSyncRequest(request, reply));

  if (reply.getMessageType() == TcrMessage::EXCEPTION) {
    LOG_DEBUG("ThinClientPoolDM::GetEnumValue: Exception = " +
              reply.getException());
    throw IllegalStateException("Failed to register Pdx enum Type");
  }

  auto enumVal =
      std::dynamic_pointer_cast<CacheableInt32>(reply.getValue())->value();

  // need to broadcast this id to all other pool
  {
    auto& poolManager = m_connManager.getCacheImpl()->getPoolManager();
    for (const auto& iter : poolManager.getAll()) {
      const auto& currPool =
          std::dynamic_pointer_cast<ThinClientPoolDM>(iter.second);

      if (currPool && this != currPool.get()) {
        currPool->AddEnum(enumInfo, enumVal);
      }
    }
  }

  return enumVal;
}
std::shared_ptr<Serializable> ThinClientPoolDM::GetEnum(int32_t val) {
  LOG_DEBUG("ThinClientPoolDM::GetEnum:");

  TcrMessageGetPdxEnumById request(
      new DataOutput(m_connManager.getCacheImpl()->createDataOutput()), val,
      this);

  TcrMessageReply reply(true, this);

  throwExceptionIfError("Operation Failed", sendSyncRequest(request, reply));

  if (reply.getMessageType() == TcrMessage::EXCEPTION) {
    LOG_DEBUG("ThinClientPoolDM::GetEnum: Exception = " + reply.getException());
    throw IllegalStateException("Failed to understand enum Type");
  }

  return reply.getValue();
}

void ThinClientPoolDM::AddEnum(std::shared_ptr<Serializable> enumInfo,
                               int enumVal) {
  LOG_DEBUG("ThinClientPoolDM::AddEnum:");

  TcrMessageAddPdxEnum request(
      new DataOutput(m_connManager.getCacheImpl()->createDataOutput()),
      enumInfo, this, enumVal);

  TcrMessageReply reply(true, this);

  throwExceptionIfError("Operation Failed", sendSyncRequest(request, reply));

  if (reply.getMessageType() == TcrMessage::EXCEPTION) {
    LOG_DEBUG("ThinClientPoolDM::AddEnum: Exception = " + reply.getException());
    throw IllegalStateException("Failed to register enum Type");
  }
}

GfErrType ThinClientPoolDM::sendUserCredentials(
    std::shared_ptr<Properties> credentials, TcrConnection*& conn,
    bool isBGThread, bool& isServerException) {
  LOG_DEBUG("ThinClientPoolDM::sendUserCredentials:");

  TcrMessageUserCredential request(
      new DataOutput(m_connManager.getCacheImpl()->createDataOutput()),
      credentials, this);

  TcrMessageReply reply(true, this);

  auto err =
      conn->getEndpointObject()->sendRequestConnWithRetry(request, reply, conn);

  if (conn) {
    err = handleEPError(conn->getEndpointObject(), reply, err);
  }

  LOG_DEBUG(
      "ThinClientPoolDM::sendUserCredentials: Error after sending cred "
      "request "
      "= %d ",
      err);

  if (err == GF_NOERR) {
    switch (reply.getMessageType()) {
      case TcrMessage::RESPONSE: {
        // nothing to be done;
        break;
      }
      case TcrMessage::EXCEPTION: {
        if (err == GF_NOERR && conn) {
          putInQueue(
              conn,
              isBGThread);  // connFound is only relevant for Sticky conn.
        }
        // this will set error type if there is some server exception
        err = ThinClientRegion::handleServerException(
            "ThinClientPoolDM::sendUserCredentials AuthException",
            reply.getException());
        isServerException = true;
        break;
      }
      default: {
        if (err == GF_NOERR && conn) {
          putInQueue(
              conn,
              isBGThread);  // connFound is only relevant for Sticky conn.
        }
        LOG_ERROR(
            "Unknown message type %d during secure response, possible "
            "serialization mismatch",
            reply.getMessageType());
        err = GF_MSG;

        break;
      }
    }
  }
  return err;
}

TcrEndpoint* ThinClientPoolDM::getSingleHopServer(
    TcrMessage& request, int8_t& version,
    std::shared_ptr<BucketServerLocation>& serverlocation,
    std::set<ServerLocation>& excludeServers) {
  const std::shared_ptr<CacheableKey>& key = request.getKeyRef();
  if (!m_clientMetadataService || key == nullptr) return nullptr;
  auto r = request.getRegion();
  auto region = nullptr == r ? nullptr : r->shared_from_this();
  TcrEndpoint* ep = nullptr;
  if (region == nullptr) {
    region = m_connManager.getCacheImpl()->getRegion(request.getRegionName());
  }
  if (region != nullptr) {
    m_clientMetadataService->getBucketServerLocation(
        region, key, request.getValueRef(), request.getCallbackArgumentRef(),
        request.forPrimary(), serverlocation, version);

    if (serverlocation != nullptr && serverlocation->isValid()) {
      LOG_FINE("Server host and port are %s:%d",
               serverlocation->getServerName().c_str(),
               serverlocation->getPort());
      ep = getEndPoint(serverlocation, version, excludeServers);
    }
  }
  return ep;
}

TcrEndpoint* ThinClientPoolDM::getEndPoint(
    const std::shared_ptr<BucketServerLocation>& serverLocation, int8_t&,
    std::set<ServerLocation>& excludeServers) {
  std::shared_ptr<TcrEndpoint> ep = nullptr;
  if (serverLocation->isValid()) {
    if (excludeServer(serverLocation->getEpString(), excludeServers)) {
      LOG_FINE("ThinClientPoolDM::getEndPoint Exclude Server true for %s ",
               serverLocation->getEpString().c_str());
      return ep.get();
    }

    ep = getEndpoint(serverLocation->getEpString());
    if (ep) {
      return ep.get();
    }

    // do for pool with endpoints. Add endpoint into m_endpoints only when we
    // did not find it above and it is in the pool's m_initServList.
    for (const auto& itr : m_attrs->m_initServList) {
      if (serverLocation->getEpString() == itr) {
        ep = addEP(*serverLocation);  // see if this is new endpoint
        break;
      }
    }

    // do only for locator
    // if servergroup is there, then verify otherwise you may reach to another
    // group
    if (!m_attrs->m_initLocList.empty()) {
      auto&& servGrp = getServerGroup();
      if (servGrp.length() > 0) {
        auto groups = serverLocation->getServerGroups();
        if ((groups != nullptr) && (groups->length() > 0)) {
          for (int i = 0; i < groups->length(); i++) {
            auto cs = (*groups)[i];
            if (cs->length() > 0) {
              auto&& str = cs->toString();
              if (str == servGrp) {
                // see if this is new endpoint
                ep = addEP(*serverLocation);
                break;
              }
            }
          }
        }
      } else  // just add it
      {
        ep = addEP(*(serverLocation.get()));  // see if this is new endpoint
      }
    }
  }

  return ep.get();
}

std::shared_ptr<TcrEndpoint> ThinClientPoolDM::getEndpoint(
    const std::string& endpointName) {
  m_endpoints.make_lock();
  const auto& find = m_endpoints.find(endpointName);
  if (find == m_endpoints.end()) {
    return {};
  }
  return find->second;
}

GfErrType ThinClientPoolDM::sendSyncRequest(TcrMessage& request,
                                            TcrMessageReply& reply,
                                            bool attemptFailover,
                                            bool isBGThread) {
  int32_t type = request.getMessageType();

  if (!request.forTransaction() && m_attrs->getPRSingleHopEnabled() &&
      (type == TcrMessage::GET_ALL_70 ||
       type == TcrMessage::GET_ALL_WITH_CALLBACK) &&
      m_clientMetadataService) {
    GfErrType error = GF_NOERR;

    auto region =
        m_connManager.getCacheImpl()->getRegion(request.getRegionName());
    auto locationMap = m_clientMetadataService->getServerToFilterMap(
        *(request.getKeys()), region, request.forPrimary());

    if (!locationMap) {
      request.InitializeGetallMsg(
          request.getCallbackArgument());  // now initialize getall msg
      return sendSyncRequest(request, reply, attemptFailover, isBGThread,
                             nullptr);
    }
    std::vector<std::shared_ptr<GetAllWork>> getAllWorkers;
    auto& threadPool = m_connManager.getCacheImpl()->getThreadPool();
    auto responseHandler =
        static_cast<ChunkedGetAllResponse*>(reply.getChunkedResultHandler());

    for (const auto& locationIter : *locationMap) {
      const auto& serverLocation = locationIter.first;
      const auto& keys = locationIter.second;
      auto worker = std::make_shared<GetAllWork>(
          this, region, serverLocation, keys, attemptFailover, isBGThread,
          responseHandler->getAddToLocalCache(), responseHandler,
          request.getCallbackArgument());
      threadPool.perform(worker);
      getAllWorkers.push_back(worker);
    }
    reply.setMessageType(TcrMessage::RESPONSE);

    for (auto& worker : getAllWorkers) {
      auto err = worker->getResult();

      if (err != GF_NOERR) {
        error = err;
      }

      TcrMessage* currentReply = worker->getReply();
      if (currentReply->getMessageType() != TcrMessage::RESPONSE) {
        reply.setMessageType(currentReply->getMessageType());
      }
    }
    return error;
  } else {
    if (type == TcrMessage::GET_ALL_70 ||
        type == TcrMessage::GET_ALL_WITH_CALLBACK) {
      request.InitializeGetallMsg(
          request.getCallbackArgument());  // now initialize getall msg
    }
    return sendSyncRequest(request, reply, attemptFailover, isBGThread,
                           nullptr);
  }
}

GfErrType ThinClientPoolDM::sendSyncRequest(
    TcrMessage& request, TcrMessageReply& reply, bool attemptFailover,
    bool isBGThread,
    const std::shared_ptr<BucketServerLocation>& serverLocation) {
  LOG_DEBUG("ThinClientPoolDM::sendSyncRequest: ....%d %s",
            request.getMessageType(), m_poolName.c_str());
  // Increment clientOps
  getStats().setCurClientOps(++m_clientOps);

  GfErrType error = GF_NOTCON;

  std::shared_ptr<UserAttributes> userAttr = nullptr;
  reply.setDM(this);

  int32_t type = request.getMessageType();

  if (!(type == TcrMessage::QUERY ||
        type == TcrMessage::QUERY_WITH_PARAMETERS ||
        type == TcrMessage::PUTALL ||
        type == TcrMessage::PUT_ALL_WITH_CALLBACK ||
        type == TcrMessage::EXECUTE_FUNCTION ||
        type == TcrMessage::EXECUTE_REGION_FUNCTION ||
        type == TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP ||
        type == TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE)) {
    // set only when message is not query, putall and executeCQ
    reply.setTimeout(getReadTimeout());
    request.setTimeout(getReadTimeout());
  }

  bool retryAllEPsOnce = false;
  if (m_attrs->getRetryAttempts() == -1) {
    retryAllEPsOnce = true;
  }
  auto retry = m_attrs->getRetryAttempts() + 1;
  TcrConnection* conn = nullptr;
  std::set<ServerLocation> excludeServers;
  type = request.getMessageType();
  bool isAuthRequireExcep = false;
  int isAuthRequireExcepMaxTry = 2;
  bool firstTry = true;
  LOG_FINE("sendSyncRequest:: retry = %d", retry);
  while (retryAllEPsOnce || retry-- ||
         (isAuthRequireExcep && isAuthRequireExcepMaxTry >= 0)) {
    isAuthRequireExcep = false;
    if (!firstTry) request.updateHeaderForRetry();
    // if it's a query or putall and we had a timeout, just return with the
    // newly selected endpoint without failover-retry
    if ((type == TcrMessage::QUERY ||
         type == TcrMessage::QUERY_WITH_PARAMETERS ||
         type == TcrMessage::PUTALL ||
         type == TcrMessage::PUT_ALL_WITH_CALLBACK ||
         type == TcrMessage::EXECUTE_FUNCTION ||
         type == TcrMessage::EXECUTE_REGION_FUNCTION ||
         type == TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP ||
         type == TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE) &&
        error == GF_TIMEOUT) {
      return error;
    }

    GfErrType queueErr = GF_NOERR;
    auto lastExcludeSize = static_cast<uint32_t>(excludeServers.size());
    int8_t version = 0;

    bool isUserNeedToReAuthenticate = false;
    bool singleHopConnFound = false;
    bool connFound = false;
    if (!m_isMultiUserMode || (!TcrMessage::isUserInitiativeOps(request))) {
      conn = getConnectionFromQueueW(&queueErr, excludeServers, isBGThread,
                                     request, version, singleHopConnFound,
                                     connFound, serverLocation);
    } else {
      userAttr = UserAttributes::threadLocalUserAttributes;
      if (userAttr == nullptr) {
        LOG_WARN("Attempted operation type %d without credentials",
                 request.getMessageType());
        return GF_NOT_AUTHORIZED_EXCEPTION;
      }
      // Can i assume here that we will always get connection here
      conn = getConnectionFromQueueW(&queueErr, excludeServers, isBGThread,
                                     request, version, singleHopConnFound,
                                     connFound, serverLocation);

      if (conn != nullptr) {  // need to chk whether user is already
                              // authenticated to this endpoint or not.
        isUserNeedToReAuthenticate =
            !(userAttr->isEndpointAuthenticated(conn->getEndpointObject()));
      }
    }

    if (queueErr == GF_CLIENT_WAIT_TIMEOUT) {
      LOG_FINE("Request timeout at client only");
      return GF_CLIENT_WAIT_TIMEOUT;
    } else if (queueErr == GF_CLIENT_WAIT_TIMEOUT_REFRESH_PRMETADATA) {
      // need to refresh meta data
      auto region =
          m_connManager.getCacheImpl()->getRegion(request.getRegionName());

      if (region != nullptr) {
        LOG_FINE(
            "Need to refresh pr-meta-data timeout in client only  with "
            "refresh "
            "metadata");
        auto* tcrRegion = dynamic_cast<ThinClientRegion*>(region.get());
        tcrRegion->setMetaDataRefreshed(false);
        m_clientMetadataService->enqueueForMetadataRefresh(
            region->getFullPath(), reply.getserverGroupVersion());
      }
      return GF_CLIENT_WAIT_TIMEOUT_REFRESH_PRMETADATA;
    }

    LOG_DEBUG(
        "ThinClientPoolDM::sendSyncRequest: isUserNeedToReAuthenticate = %d ",
        isUserNeedToReAuthenticate);
    LOG_DEBUG(
        "ThinClientPoolDM::sendSyncRequest: m_isMultiUserMode = {}  conn = {}  "
        "type = {}",
        m_isMultiUserMode, static_cast<void*>(conn), type);

    if (!conn) {
      // lets assume all connection are in use will happen
      if (queueErr == GF_NOERR) {
        queueErr = GF_ALL_CONNECTIONS_IN_USE_EXCEPTION;
        getStats().setCurClientOps(--m_clientOps);
        getStats().incFailedClientOps();
        return queueErr;
      } else if (queueErr == GF_IOERR) {
        error = GF_NOTCON;
      } else {
        error = queueErr;
      }
    }
    if (conn) {
      TcrEndpoint* ep = conn->getEndpointObject();
      LOG_DEBUG(
          "ThinClientPoolDM::sendSyncRequest: sendSyncReq "
          "ep->isAuthenticated() = %d ",
          ep->isAuthenticated());
      GfErrType userCredMsgErr = GF_NOERR;
      bool isServerException = false;
      if (TcrMessage::isUserInitiativeOps(request) &&
          (m_isSecurityOn || m_isMultiUserMode)) {
        if (!m_isMultiUserMode && !ep->isAuthenticated()) {
          // first authenticate him on this endpoint
          userCredMsgErr = sendUserCredentials(getCredentials(ep), conn,
                                               isBGThread, isServerException);
        } else if (isUserNeedToReAuthenticate) {
          userCredMsgErr = sendUserCredentials(userAttr->getCredentials(), conn,
                                               isBGThread, isServerException);
        }
      }

      if (userCredMsgErr == GF_NOERR) {
        error = ep->sendRequestConnWithRetry(request, reply, conn);
        error = handleEPError(ep, reply, error);
      } else {
        error = userCredMsgErr;
      }

      if (!isServerException) {
        if (error == GF_NOERR) {
          LOG_DEBUG("putting connection back in queue");
          putInQueue(conn,
                     isBGThread ||
                         request.getMessageType() == TcrMessage::GET_ALL_70 ||
                         request.getMessageType() ==
                             TcrMessage::GET_ALL_WITH_CALLBACK ||
                         request.getMessageType() ==
                             TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP,
                     request.forTransaction());  // connFound is only relevant
                                                 // for Sticky conn.
          LOG_DEBUG("putting connection back in queue DONE");
        } else {
          if (error != GF_TIMEOUT) removeEPConnections(ep);
          // Update stats for the connection that failed.
          removeEPConnections(1, false);
          setStickyNull(isBGThread ||
                        request.getMessageType() == TcrMessage::GET_ALL_70 ||
                        request.getMessageType() ==
                            TcrMessage::GET_ALL_WITH_CALLBACK ||
                        request.getMessageType() ==
                            TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP);
          if (conn) {
            try {
              GF_SAFE_DELETE_CON(conn);
            } catch (...) {
            }
          }
          excludeServers.insert(ServerLocation(ep->name()));
          removeEPFromMetadataIfError(error, ep);
        }
      } else {
        return error;  // server exception while sending credential message to
      }
      // server...
    }

    if (error == GF_NOERR) {
      if ((m_isSecurityOn || m_isMultiUserMode)) {
        if (reply.getMessageType() == TcrMessage::EXCEPTION) {
          if (isAuthRequireException(reply.getException())) {
            TcrEndpoint* ep = conn->getEndpointObject();
            if (!m_isMultiUserMode) {
              ep->setAuthenticated(false);
            } else if (userAttr != nullptr) {
              userAttr->unAuthenticateEP(ep);
            }
            LOG_FINEST(
                "After getting AuthenticationRequiredException trying "
                "again.");
            isAuthRequireExcepMaxTry--;
            isAuthRequireExcep = true;
            continue;
          } else if (isNotAuthorizedException(reply.getException())) {
            LOG_DEBUG("received NotAuthorizedException");
            // TODO should we try again?
          }
        }
      }
      LOG_FINER(
          "reply Metadata version is %d & bsl version is %d "
          "reply.isFEAnotherHop()=%d",
          reply.getMetaDataVersion(), version, reply.isFEAnotherHop());
      if (m_clientMetadataService && request.forSingleHop() &&
          (reply.getMetaDataVersion() != 0 ||
           (request.getMessageType() == TcrMessage::EXECUTE_REGION_FUNCTION &&
            request.getKeyRef() != nullptr && reply.isFEAnotherHop()))) {
        // Need to get direct access to Region's name to avoid referencing
        // temp data and causing crashes
        auto region =
            m_connManager.getCacheImpl()->getRegion(request.getRegionName());

        if (region != nullptr) {
          if (!connFound)  // max limit case then don't refresh otherwise
                           // always refresh
          {
            LOG_FINE("Need to refresh pr-meta-data");
            auto* tcrRegion = dynamic_cast<ThinClientRegion*>(region.get());
            tcrRegion->setMetaDataRefreshed(false);
          }
          m_clientMetadataService->enqueueForMetadataRefresh(
              region->getFullPath(), reply.getserverGroupVersion());
        }
      }
    }

    if (excludeServers.size() == lastExcludeSize) {
      excludeServers.clear();
      if (retryAllEPsOnce) {
        break;
      }
    }

    if (!attemptFailover || error == GF_NOERR) {
      getStats().setCurClientOps(--m_clientOps);
      if (error == GF_NOERR) {
        getStats().incSucceedClientOps(); /*inc Id for clientOs stat*/
      } else if (error == GF_TIMEOUT) {
        getStats().incTimeoutClientOps();
      } else {
        getStats().incFailedClientOps();
      }
      // Top-level only sees NotConnectedException
      if (error == GF_IOERR) {
        error = GF_NOTCON;
      }
      return error;
    }

    conn = nullptr;
    firstTry = false;
  }  // While

  getStats().setCurClientOps(--m_clientOps);

  if (error == GF_NOERR) {
    getStats().incSucceedClientOps();
  } else if (error == GF_TIMEOUT) {
    getStats().incTimeoutClientOps();
  } else {
    getStats().incFailedClientOps();
  }

  // Top-level only sees NotConnectedException
  if (error == GF_IOERR) {
    error = GF_NOTCON;
  }
  return error;
}

void ThinClientPoolDM::removeEPFromMetadataIfError(const GfErrType& error,
                                                   const TcrEndpoint* ep) {
  if ((error == GF_IOERR || error == GF_TIMEOUT) && (m_clientMetadataService)) {
    auto sl = std::make_shared<BucketServerLocation>(ep->name());
    LOG_INFO("Removing bucketServerLocation %s due to %s",
             sl->toString().c_str(),
             (error == GF_IOERR ? "GF_IOERR" : "GF_TIMEOUT"));
    m_clientMetadataService->removeBucketServerLocation(sl);
  }
}

void ThinClientPoolDM::removeEPConnections(int numConn,
                                           bool triggerManageConn) {
  // TODO: Delete EP

  reducePoolSize(numConn);

  // Raise Semaphore for manage thread
  if (triggerManageConn) {
    conn_semaphore_.release();
  }
}

// Tries to get connection to a endpoint. If no connection is available, it
// tries
// to create one. If it fails to create one,  it gets connection to any other
// server
// and fails over the transaction to that server.
// This function is used when the transaction is to be resumed to a specified
// server.
GfErrType ThinClientPoolDM::getConnectionToAnEndPoint(std::string epNameStr,
                                                      TcrConnection*& conn) {
  conn = nullptr;

  GfErrType error = GF_NOERR;
  auto theEP = getEndpoint(epNameStr);

  LOG_FINE(
      "ThinClientPoolDM::getConnectionToAnEndPoint( ): Getting endpoint "
      "object "
      "for %s",
      epNameStr.c_str());
  if (theEP && theEP->connected()) {
    LOG_FINE(
        "ThinClientPoolDM::getConnectionToAnEndPoint( ): Getting connection "
        "for endpoint %s",
        epNameStr.c_str());
    conn = getFromEP(theEP.get());
    // if connection is null, possibly because there are no idle connections
    // to this endpoint, create a new pool connection to this endpoint.
    bool maxConnLimit = false;
    if (conn == nullptr) {
      LOG_FINE(
          "ThinClientPoolDM::getConnectionToAnEndPoint( ): Create connection "
          "for endpoint %s",
          epNameStr.c_str());
      error = createPoolConnectionToAEndPoint(conn, theEP.get(), maxConnLimit);
    }
  }

  // if connection is null, it has failed to get a connection to the specified
  // endpoint. Get a connection to any other server and failover the
  // transaction to that server.
  if (!conn) {
    std::set<ServerLocation> excludeServers;
    bool maxConnLimit = false;
    LOG_FINE(
        "ThinClientPoolDM::getConnectionToAnEndPoint( ): No connection "
        "available for endpoint %s. Create connection to any endpoint.",
        epNameStr.c_str());
    conn = getConnectionFromQueue(true, &error, excludeServers, maxConnLimit);
    if (conn && error == GF_NOERR) {
      if (conn->getEndpointObject()->name() != epNameStr) {
        LOG_FINE(
            "ThinClientPoolDM::getConnectionToAnEndPoint( ): Endpoint %s "
            "different than the endpoint %s. New connection created and "
            "failing over.",
            epNameStr.c_str(), conn->getEndpointObject()->name().c_str());
        auto failoverErr = doFailover(conn);
        if (failoverErr != GF_NOERR) {
          LOG_FINE(
              "ThinClientPoolDM::getConnectionToAnEndPoint( ):Failed to "
              "failover transaction to another server. From endpoint %s to "
              "%s",
              epNameStr.c_str(), conn->getEndpointObject()->name().c_str());
          putInQueue(conn, false);
          conn = nullptr;
        }
      }
    }
  }

  if (!(conn && error == GF_NOERR)) {
    LOG_FINE(
        "ThinClientPoolDM::getConnectionToAEndPoint( ):Failed to connect to "
        "%s",
        epNameStr.c_str());
    if (conn) {
      _GEODE_SAFE_DELETE(conn);
    }
  }

  return error;
}

// Create a pool connection to specified endpoint. First checks if the number
// of connections has exceeded the maximum allowed. If not, create a
// connection to the specified endpoint. Else, throws an error.
GfErrType ThinClientPoolDM::createPoolConnectionToAEndPoint(
    TcrConnection*& conn, TcrEndpoint* theEP, bool& maxConnLimit,
    bool appThreadrequest) {
  GfErrType error = GF_NOERR;
  conn = nullptr;
  int min = 0;
  {
    // Check if the pool size has exceeded maximum allowed.

    int max = m_attrs->getMaxConnections();
    if (max == -1) {
      max = 0x7fffffff;
    }
    min = m_attrs->getMinConnections();
    max = max > min ? max : min;

    if (m_poolSize >= max) {
      maxConnLimit = true;
      LOG_FINER(
          "ThinClientPoolDM::createPoolConnectionToAEndPoint( ): current "
          "pool "
          "size has reached limit %d, %d",
          m_poolSize.load(), max);
      return error;
    }
  }

  LOG_FINE(
      "ThinClientPoolDM::createPoolConnectionToAEndPoint( ): creating a new "
      "connection to the endpoint %s",
      theEP->name().c_str());
  // if the pool size is within limits, create a new connection.
  error = theEP->createNewConnection(conn, false, false,
                                     m_connManager.getCacheImpl()
                                         ->getDistributedSystem()
                                         .getSystemProperties()
                                         .connectTimeout(),
                                     false, appThreadrequest);
  if (conn == nullptr || error != GF_NOERR) {
    LOG_FINE("2Failed to connect to %s", theEP->name().c_str());
    if (conn != nullptr) _GEODE_SAFE_DELETE(conn);
  } else {
    theEP->setConnected();
    ++m_poolSize;
    if (m_poolSize > min) {
      getStats().incLoadCondConnects();
    }
    // Update Stats
    getStats().incPoolConnects();
    getStats().setCurPoolConnections(m_poolSize);
  }

  conn_semaphore_.release();

  return error;
}

void ThinClientPoolDM::reducePoolSize(int num) {
  LOG_FINE("Removing %d connections, pool-size=%d", num, m_poolSize.load());
  m_poolSize -= num;
}

GfErrType ThinClientPoolDM::createPoolConnection(
    TcrConnection*& conn, std::set<ServerLocation>& excludeServers,
    bool& maxConnLimit, const TcrConnection* currentserver) {
  GfErrType error = GF_NOERR;
  int max = m_attrs->getMaxConnections();
  if (max == -1) {
    max = 0x7fffffff;
  }
  int min = m_attrs->getMinConnections();
  max = max > min ? max : min;

  conn = nullptr;
  if (m_poolSize >= max) {
    maxConnLimit = true;
    return error;
  }

  bool fatal = false;
  GfErrType fatalError = GF_NOERR;

  while (true) {
    std::string epNameStr;
    try {
      epNameStr = selectEndpoint(excludeServers, currentserver);
    } catch (const NoAvailableLocatorsException&) {
      LOG_FINE("Locator query failed while creating pool connection");
      return GF_CACHE_LOCATOR_EXCEPTION;
    } catch (const Exception&) {
      LOG_FINE("Endpoint selection failed");
      return GF_NOTCON;
    }

    LOG_FINE("Connecting to %s", epNameStr.c_str());
    auto ep = addEP(epNameStr);

    if (currentserver != nullptr &&
        epNameStr == currentserver->getEndpointObject()->name()) {
      LOG_DEBUG("Updating existing connection: ", epNameStr.c_str());
      conn = const_cast<TcrConnection*>(currentserver);
      conn->updateCreationTime();
      break;
    } else {
      error = ep->createNewConnection(conn, false, false,
                                      m_connManager.getCacheImpl()
                                          ->getDistributedSystem()
                                          .getSystemProperties()
                                          .connectTimeout(),
                                      false);
    }

    if (conn == nullptr || error != GF_NOERR) {
      LOG_FINE("1Failed to connect to %s", epNameStr.c_str());
      excludeServers.insert(ServerLocation(ep->name()));
      if (conn != nullptr) {
        _GEODE_SAFE_DELETE(conn);
      }
      if (ThinClientBaseDM::isFatalError(error)) {
        // save this error for later to override the
        // error code to be returned
        fatalError = error;
        fatal = true;
      }
      if (ThinClientBaseDM::isFatalClientError(error)) {
        //  log the error string instead of error number.
        LOG_FINE("Connection failed due to fatal client error %d", error);
        return error;
      }
    } else {
      ep->setConnected();
      if (++m_poolSize > min) {
        getStats().incLoadCondConnects();
      }
      // Update Stats
      getStats().incPoolConnects();
      getStats().setCurPoolConnections(m_poolSize);
      break;
    }
  }

  conn_semaphore_.release();
  // if a fatal error occurred earlier and we don't have
  // a connection then return this saved error
  if (fatal && !conn && error != GF_NOERR) {
    return fatalError;
  }

  return error;
}

TcrConnection* ThinClientPoolDM::getConnectionFromQueue(
    bool, GfErrType* error, std::set<ServerLocation>& excludeServers,
    bool& maxConnLimit) {
  std::chrono::microseconds timeoutTime = m_attrs->getFreeConnectionTimeout();

  getStats().incCurWaitingConnections();
  getStats().incWaitingConnections();

  /*get the start time for connectionWaitTime stat*/
  bool enableTimeStatistics = m_connManager.getCacheImpl()
                                  ->getDistributedSystem()
                                  .getSystemProperties()
                                  .getEnableTimeStatistics();
  auto sampleStartNanos = enableTimeStatistics ? Utils::startStatOpTime() : 0;
  auto mp = getUntil(timeoutTime, error, excludeServers, maxConnLimit);
  /*Update the time stat for clientOpsTime */
  if (enableTimeStatistics) {
    Utils::updateStatOpTime(getStats().getStats(),
                            getStats().getTotalWaitingConnTimeId(),
                            sampleStartNanos);
  }
  getStats().decCurWaitingConnections();
  return mp;
}

bool ThinClientPoolDM::isEndpointAttached(TcrEndpoint*) { return true; }

GfErrType ThinClientPoolDM::sendRequestToEP(const TcrMessage& request,
                                            TcrMessageReply& reply,
                                            TcrEndpoint* currentEndpoint) {
  LOG_DEBUG("ThinClientPoolDM::sendRequestToEP()");
  int isAuthRequireExcepMaxTry = 2;
  bool isAuthRequireExcep = true;
  GfErrType error = GF_NOERR;
  while (isAuthRequireExcep && isAuthRequireExcepMaxTry >= 0) {
    isAuthRequireExcep = false;
    TcrConnection* conn = getFromEP(currentEndpoint);

    bool isTmpConnectedStatus = false;
    bool putConnInPool = true;
    if (conn == nullptr) {
      LOG_DEBUG(
          "ThinClientPoolDM::sendRequestToEP(): got nullptr connection from "
          "pool, "
          "creating new connection in the pool.");
      bool maxConnLimit = false;
      error =
          createPoolConnectionToAEndPoint(conn, currentEndpoint, maxConnLimit);
      if (conn == nullptr || error != GF_NOERR) {
        LOG_DEBUG(
            "ThinClientPoolDM::sendRequestToEP(): couldnt create a pool "
            "connection, creating a temporary connection.");
        error =
            currentEndpoint->createNewConnection(conn, false, false,
                                                 m_connManager.getCacheImpl()
                                                     ->getDistributedSystem()
                                                     .getSystemProperties()
                                                     .connectTimeout(),
                                                 false);
        putConnInPool = false;
        currentEndpoint->setConnectionStatus(true);
      }
    }

    if (conn == nullptr || error != GF_NOERR) {
      LOG_FINE("3Failed to connect to %s", currentEndpoint->name().c_str());
      if (conn != nullptr) {
        _GEODE_SAFE_DELETE(conn);
      }
      if (putConnInPool) {
        reducePoolSize(1);
      }
      currentEndpoint->setConnectionStatus(false);
      return error;
    } else if (!putConnInPool && !currentEndpoint->connected()) {
      isTmpConnectedStatus = true;
      currentEndpoint->setConnectionStatus(true);
    }

    int32_t type = request.getMessageType();

    if (!(type == TcrMessage::QUERY || type == TcrMessage::PUTALL ||
          type == TcrMessage::PUT_ALL_WITH_CALLBACK ||
          type == TcrMessage::EXECUTE_FUNCTION ||
          type == TcrMessage::EXECUTE_REGION_FUNCTION ||
          type == TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP ||
          type == TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE)) {
      reply.setTimeout(getReadTimeout());
    }

    reply.setDM(this);
    std::shared_ptr<UserAttributes> ua = nullptr;
    // in multi user mode need to chk whether user is authenticated or not
    // and then follow usual process which we did in send syncrequest.
    // need to user initiative ops
    LOG_DEBUG("ThinClientPoolDM::sendRequestToEP: m_isMultiUserMode = %d",
              m_isMultiUserMode);
    bool isServerException = false;
    if (TcrMessage::isUserInitiativeOps((request)) &&
        (m_isSecurityOn || m_isMultiUserMode)) {
      if (!m_isMultiUserMode && !currentEndpoint->isAuthenticated()) {
        // first authenticate him on this endpoint
        error = sendUserCredentials(getCredentials(currentEndpoint), conn,
                                    false, isServerException);
      } else if (m_isMultiUserMode) {
        ua = UserAttributes::threadLocalUserAttributes;
        if (ua) {
          UserConnectionAttributes* uca =
              ua->getConnectionAttribute(currentEndpoint);

          if (uca == nullptr) {
            error = sendUserCredentials(ua->getCredentials(), conn, false,
                                        isServerException);
          }
        } else {
          LOG_WARN("Attempted operation type %d without credentials",
                   request.getMessageType());
          if (conn) {
            putInQueue(conn, false, request.forTransaction());
          }
          return GF_NOT_AUTHORIZED_EXCEPTION;
        }
      }
    }

    LOG_DEBUG("ThinClientPoolDM::sendRequestToEP after getting creds");
    if (error == GF_NOERR && conn != nullptr) {
      error =
          currentEndpoint->sendRequestConnWithRetry(request, reply, conn, true);
    }

    if (isServerException) return error;

    if (error == GF_NOERR) {
      int32_t replyMsgType = reply.getMessageType();
      if (replyMsgType == TcrMessage::EXCEPTION ||
          replyMsgType == TcrMessage::CQ_EXCEPTION_TYPE ||
          replyMsgType == TcrMessage::CQDATAERROR_MSG_TYPE) {
        error = ThinClientRegion::handleServerException(
            "ThinClientPoolDM::sendRequestToEP", reply.getException());
      }

      if (putConnInPool) {
        put(conn, false);
      } else {
        if (isTmpConnectedStatus) currentEndpoint->setConnectionStatus(false);
        conn->close();
        _GEODE_SAFE_DELETE(conn);
      }
    } else if (error != GF_NOERR) {
      currentEndpoint->setConnectionStatus(false);
      if (putConnInPool) {
        removeEPConnections(1);
      }
      removeEPFromMetadataIfError(error, currentEndpoint);
    }

    if (error == GF_NOERR || error == GF_CACHESERVER_EXCEPTION ||
        error == GF_AUTHENTICATION_REQUIRED_EXCEPTION) {
      if ((m_isSecurityOn || m_isMultiUserMode)) {
        if (reply.getMessageType() == TcrMessage::EXCEPTION) {
          if (isAuthRequireException(reply.getException())) {
            if (!m_isMultiUserMode) {
              currentEndpoint->setAuthenticated(false);
            } else if (ua != nullptr) {
              ua->unAuthenticateEP(currentEndpoint);
            }
            LOG_FINEST(
                "After getting AuthenticationRequiredException trying "
                "again.");
            isAuthRequireExcepMaxTry--;
            isAuthRequireExcep = true;
            if (isAuthRequireExcepMaxTry >= 0) error = GF_NOERR;
            continue;
          }
        }
      }
    }
  }
  LOG_DEBUG("ThinClientPoolDM::sendRequestToEP Done.");
  return error;
}

std::shared_ptr<TcrEndpoint> ThinClientPoolDM::addEP(
    ServerLocation& serverLoc) {
  return addEP(serverLoc.getServerName() + ":" +
               std::to_string(serverLoc.getPort()));
}

std::shared_ptr<TcrEndpoint> ThinClientPoolDM::addEP(
    const std::string& endpointName) {
  std::lock_guard<decltype(m_endpointsLock)> guard(m_endpointsLock);

  auto ep = getEndpoint(endpointName);
  if (!ep) {
    ep = createEP(endpointName.c_str());
    LOG_FINE("Created new endpoint %s for pool %s", endpointName.c_str(),
             m_poolName.c_str());
    if (!m_endpoints.emplace(endpointName, ep).second) {
      LOG_ERROR("Failed to add endpoint %s to pool %s", endpointName.c_str(),
                m_poolName.c_str());
    }
  }

  // Update Server Stats
  getStats().setServers(static_cast<int32_t>(m_endpoints.size()));
  return ep;
}

void ThinClientPoolDM::netDown() {
  close();
  reset();
}

void ThinClientPoolDM::pingServerLocal() {
  std::lock_guard<decltype(m_endpointsLock)> guard(m_endpointsLock);
  for (auto& it : m_endpoints) {
    auto endpoint = it.second;
    if (endpoint->connected()) {
      endpoint->pingServer(this);
      if (!endpoint->connected()) {
        removeEPConnections(endpoint.get());
        removeCallbackConnection(endpoint.get());
      }
    }
  }
}

void ThinClientPoolDM::updateLocatorList(std::atomic<bool>& isRunning) {
  LOG_FINE("Starting updateLocatorList thread for pool %s", m_poolName.c_str());

  while (isRunning) {
    update_locators_semaphore_.acquire();
    if (isRunning && !m_connManager.isNetDown()) {
      (m_locHelper)->updateLocators(getServerGroup());
    }
  }

  LOG_FINE("Ending updateLocatorList thread for pool %s", m_poolName.c_str());
}

void ThinClientPoolDM::incConnectedEndpoints() {
  auto val = ++connected_endpoints_;
  LOG_DEBUG("Pool %s has incremented to %d the number of connected endpoints",
            m_poolName.c_str(), val);
}

void ThinClientPoolDM::decConnectedEndpoints() {
  auto val = --connected_endpoints_;
  LOG_DEBUG("Pool %s has decremented to %d the number of connected endpoints",
            m_poolName.c_str(), val);
  if (val <= 0 && clear_pdx_registry_) {
    clearPdxTypeRegistry();
  }
}

void ThinClientPoolDM::pingServer(std::atomic<bool>& isRunning) {
  LOG_FINE("Starting ping thread for pool %s", m_poolName.c_str());

  ping_semaphore_.acquire();
  while (isRunning) {
    if (!m_connManager.isNetDown()) {
      pingServerLocal();
    }

    ping_semaphore_.acquire();
  }

  LOG_FINE("Ending ping thread for pool %s", m_poolName.c_str());
}

void ThinClientPoolDM::clearPdxTypeRegistry() {
  LOG_FINE("Clearing PdxTypeRegistry of pool %s", m_poolName.c_str());
  auto cache_impl = m_connManager.getCacheImpl();

  // C# call
  DistributedSystemImpl::CallCliCallBack(*(cache_impl->getCache()));

  // C++ call
  cache_impl->getPdxTypeRegistry()->clear();
}

void ThinClientPoolDM::releaseThreadLocalConnection() {
  m_manager->releaseThreadLocalConnection();
}
void ThinClientPoolDM::setThreadLocalConnection(TcrConnection* conn) {
  m_manager->addStickyConnection(conn);
}

inline bool ThinClientPoolDM::hasExpired(TcrConnection* conn) {
  return conn->hasExpired(getLoadConditioningInterval());
}

bool ThinClientPoolDM::canItBeDeleted(TcrConnection* conn) {
  const auto& load = getLoadConditioningInterval();
  auto idle = getIdleTimeout();
  int min = getMinConnections();

  if (load > std::chrono::milliseconds::zero()) {
    if (load < idle || idle <= std::chrono::milliseconds::zero()) {
      idle = load;
    }
  }

  bool hasExpired = conn->hasExpired(load);
  bool isIdle = conn->isIdle(idle);

  bool candidateForDeletion = hasExpired || (isIdle && m_poolSize > min);
  bool canBeDeleted = false;

  if (conn && candidateForDeletion) {
    TcrEndpoint* endPt = conn->getEndpointObject();
    {
      std::lock_guard<decltype(endPt->getQueueHostedMutex())> guardQueue(
          endPt->getQueueHostedMutex());
      bool queue = endPt->isQueueHosted();
      if (queue) {
        TcrConnection* connTemp = getFromEP(endPt);
        if (connTemp) {
          put(connTemp, false);
          canBeDeleted = true;
        }
      } else {
        canBeDeleted = true;
      }
    }
  }

  return canBeDeleted;
}

bool ThinClientPoolDM::excludeServer(std::string endpoint,
                                     std::set<ServerLocation>& excludeServers) {
  return !(excludeServers.empty() || excludeServers.find(ServerLocation(
                                         endpoint)) == excludeServers.end());
}

bool ThinClientPoolDM::excludeConnection(
    TcrConnection* conn, std::set<ServerLocation>& excludeServers) {
  return excludeServer(conn->getEndpointObject()->name(), excludeServers);
}

TcrConnection* ThinClientPoolDM::getFromEP(TcrEndpoint* theEP) {
  std::lock_guard<decltype(mutex_)> lock(mutex_);
  for (auto itr = queue_.begin(); itr != queue_.end(); itr++) {
    if ((*itr)->getEndpointObject() == theEP) {
      LOG_DEBUG("ThinClientPoolDM::getFromEP got connection");
      TcrConnection* retVal = *itr;
      queue_.erase(itr);
      return retVal;
    }
  }

  return nullptr;
}

void ThinClientPoolDM::removeEPConnections(TcrEndpoint* theEP) {
  std::lock_guard<decltype(mutex_)> lock(mutex_);
  auto size = static_cast<int32_t>(queue_.size());
  int numConn = 0;

  while (size--) {
    TcrConnection* curConn = queue_.back();
    queue_.pop_back();
    if (curConn->getEndpointObject() != theEP) {
      queue_.push_front(curConn);
    } else {
      curConn->close();
      _GEODE_SAFE_DELETE(curConn);
      numConn++;
    }
  }

  removeEPConnections(numConn);
}

TcrConnection* ThinClientPoolDM::getNoGetLock(
    bool& isClosed, GfErrType* error, std::set<ServerLocation>& excludeServers,
    bool& maxConnLimit) {
  TcrConnection* returnT = nullptr;
  {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    do {
      returnT = popNoLock(isClosed);
      if (returnT) {
        if (excludeConnection(returnT, excludeServers)) {
          returnT->close();
          _GEODE_SAFE_DELETE(returnT);
          removeEPConnections(1, false);
        } else {
          break;
        }
      } else {
        break;
      }
    } while (!returnT);
  }

  if (!returnT) {
    *error = createPoolConnection(returnT, excludeServers, maxConnLimit);
  }

  return returnT;
}

void ThinClientPoolDM::incRegionCount() {
  std::lock_guard<decltype(mutex_)> lock(mutex_);

  if (!m_isDestroyed && !m_destroyPending) {
    m_numRegions++;
  } else {
    throw IllegalStateException("Pool has been destroyed.");
  }
}

void ThinClientPoolDM::decRegionCount() {
  std::lock_guard<decltype(mutex_)> lock(mutex_);

  m_numRegions--;
}

void ThinClientPoolDM::checkRegions() {
  std::lock_guard<decltype(mutex_)> lock(mutex_);

  if (m_numRegions > 0) {
    throw IllegalStateException(
        "Failed to destroy pool because regions are connected with it.");
  }

  m_destroyPending = true;
}
void ThinClientPoolDM::updateNotificationStats(
    bool isDeltaSuccess, std::chrono::nanoseconds timeInNanoSecond) {
  if (isDeltaSuccess) {
    getStats().incProcessedDeltaMessages();
    getStats().incProcessedDeltaMessagesTime(timeInNanoSecond.count());
  } else {
    getStats().incDeltaMessageFailures();
  }
}

GfErrType ThinClientPoolDM::doFailover(TcrConnection* conn) {
  m_manager->setStickyConnection(conn, true);
  TcrMessageTxFailover request(
      new DataOutput(m_connManager.getCacheImpl()->createDataOutput()));
  TcrMessageReply reply(true, nullptr);

  auto err = sendSyncRequest(request, reply);

  if (err == GF_NOERR) {
    switch (reply.getMessageType()) {
      case TcrMessage::REPLY: {
        break;
      }
      case TcrMessage::EXCEPTION: {
        const auto& exceptionMsg = reply.getException();
        err = ThinClientRegion::handleServerException(
            "CacheTransactionManager::failover", exceptionMsg);
        break;
      }
      case TcrMessage::REQUEST_DATA_ERROR: {
        err = GF_CACHESERVER_EXCEPTION;
        break;
      }
      default: {
        LOG_ERROR("Unknown message type in failover reply %d",
                  reply.getMessageType());
        err = GF_MSG;
        break;
      }
    }
  }

  return err;
}

bool ThinClientPoolDM::canItBeDeletedNoImpl(TcrConnection*) { return false; }

void ThinClientPoolDM::putInQueue(TcrConnection* conn, bool,
                                  bool isTransaction) {
  if (isTransaction) {
    m_manager->setStickyConnection(conn, isTransaction);
  } else {
    put(conn, false);
  }
}

TcrConnection* ThinClientPoolDM::getConnectionFromQueueW(
    GfErrType* error, std::set<ServerLocation>& excludeServers, bool,
    TcrMessage& request, int8_t& version, bool& match, bool& connFound,
    const std::shared_ptr<BucketServerLocation>& serverLocation) {
  TcrConnection* conn = nullptr;
  TcrEndpoint* theEP = nullptr;
  LOG_DEBUG("prEnabled = %s, forSingleHop = %s %d",
            m_attrs->getPRSingleHopEnabled() ? "true" : "false",
            request.forSingleHop() ? "true" : "false",
            request.getMessageType());

  match = false;
  std::shared_ptr<BucketServerLocation> slTmp = nullptr;
  if (request.forTransaction()) {
    connFound =
        m_manager->getStickyConnection(conn, error, excludeServers, true);
    auto txState = TSSTXStateWrapper::get().getTXState();
    if (*error == GF_NOERR && !connFound &&
        (txState == nullptr || txState->isDirty())) {
      *error = doFailover(conn);
    }

    if (*error != GF_NOERR) {
      return nullptr;
    }

    if (txState != nullptr) {
      txState->setDirty();
    }
  } else if (serverLocation != nullptr /*&& excludeServers.size() == 0*/) {
    theEP = getEndPoint(serverLocation, version, excludeServers);
  } else if (m_attrs->getPRSingleHopEnabled() /*&& excludeServers.size() == 0*/
             && request.forSingleHop() &&
             (request.getMessageType() != TcrMessage::GET_ALL_70) &&
             (request.getMessageType() != TcrMessage::GET_ALL_WITH_CALLBACK)) {
    theEP = getSingleHopServer(request, version, slTmp, excludeServers);
    if (theEP != nullptr) {
      // if all buckets are not initialized
      //  match = true;
    }
    if (slTmp != nullptr && m_clientMetadataService) {
      if (m_clientMetadataService->isBucketMarkedForTimeout(
              request.getRegionName().c_str(), slTmp->getBucketId())) {
        *error = GF_CLIENT_WAIT_TIMEOUT;
        return nullptr;
      }
    }
    LOG_DEBUG("theEP is {}", static_cast<void*>(theEP));
  }
  bool maxConnLimit = false;
  if (theEP != nullptr) {
    conn = getFromEP(theEP);
    if (!conn) {
      LOG_FINER("Creating connection to endpoint as not found in pool ");
      *error = createPoolConnectionToAEndPoint(conn, theEP, maxConnLimit, true);
      if (*error == GF_CLIENT_WAIT_TIMEOUT ||
          *error == GF_CLIENT_WAIT_TIMEOUT_REFRESH_PRMETADATA) {
        if (!m_clientMetadataService || request.getKey() == nullptr) {
          return nullptr;
        }

        auto region =
            m_connManager.getCacheImpl()->getRegion(request.getRegionName());
        if (region != nullptr) {
          slTmp = nullptr;
          m_clientMetadataService
              ->markPrimaryBucketForTimeoutButLookSecondaryBucket(
                  region, request.getKey(), request.getValue(),
                  request.getCallbackArgument(), request.forPrimary(), slTmp,
                  version);
        }
        return nullptr;
      } else {
        removeEPFromMetadataIfError(*error, theEP);
      }
    }
  }
  if (conn == nullptr) {
    LOG_DEBUG("conn not found");
    match = false;
    LOG_DEBUG("looking For connection");
    conn = getConnectionFromQueue(true, error, excludeServers, maxConnLimit);
    LOG_DEBUG("Connection Found");
  }

  if (maxConnLimit) {
    // we reach max connection limit, found connection but endpoint is
    // (not)different, no need to refresh pr-meta-data
    connFound = true;
  } else {
    // if server hints pr-meta-data refresh then refresh
    // anything else???
  }

  LOG_DEBUG(
      "ThinClientPoolDM::getConnectionFromQueueW return conn = {} match = {} "
      "connFound={}",
      static_cast<void*>(conn), match, connFound);
  return conn;
}

bool ThinClientPoolDM::checkDupAndAdd(std::shared_ptr<EventId> eventid) {
  return m_connManager.checkDupAndAdd(eventid);
}

std::shared_ptr<TcrEndpoint> ThinClientPoolDM::createEP(
    const char* endpointName) {
  return std::make_shared<TcrPoolEndPoint>(
      endpointName, m_connManager.getCacheImpl(),
      m_connManager.failover_semaphore_, m_connManager.cleanup_semaphore_,
      m_connManager.redundancy_semaphore_, this);
}

GfErrType FunctionExecution::execute() {
  GuardUserAttributes gua;

  if (m_userAttr) {
    gua.setAuthenticatedView(m_userAttr->getAuthenticatedView());
  }

  std::string funcName(m_func);
  TcrMessageExecuteFunction request(
      new DataOutput(
          m_poolDM->getConnectionManager().getCacheImpl()->createDataOutput()),
      funcName, m_args, m_getResult, m_poolDM, m_timeout);
  TcrMessageReply reply(true, m_poolDM);
  auto resultProcessor = std::unique_ptr<ChunkedFunctionExecutionResponse>(
      new ChunkedFunctionExecutionResponse(reply, (m_getResult & 2) == 2, *m_rc,
                                           m_resultCollectorLock));
  reply.setChunkedResultHandler(resultProcessor.get());
  reply.setTimeout(m_timeout);
  reply.setDM(m_poolDM);

  LOG_DEBUG(
      "ThinClientPoolDM::sendRequestToAllServer sendRequest on endpoint[%s]!",
      m_ep->name().c_str());

  m_error = m_poolDM->sendRequestToEP(request, reply, m_ep);
  m_error = m_poolDM->handleEPError(m_ep, reply, m_error);
  if (m_error != GF_NOERR) {
    if (m_error == GF_NOTCON) {
      return GF_NOERR;  // if server is unavailable its not an error for
      // functionexec OnServers() case
    }
    LOG_DEBUG("FunctionExecution::execute failed on endpoint[%s]!. Error = %d ",
              m_ep->name().c_str(), m_error);
    if (reply.getMessageType() == TcrMessage::EXCEPTION) {
      exceptionPtr = CacheableString::create(reply.getException());
    }

    return m_error;
  } else if (reply.getMessageType() == TcrMessage::EXCEPTION ||
             reply.getMessageType() == TcrMessage::EXECUTE_FUNCTION_ERROR) {
    m_error = ThinClientRegion::handleServerException("Execute",
                                                      reply.getException());
    exceptionPtr = CacheableString::create(reply.getException());
  }

  return m_error;
}

OnRegionFunctionExecution::OnRegionFunctionExecution(
    std::string func, const Region* region, std::shared_ptr<Cacheable> args,
    std::shared_ptr<CacheableHashSet> routingObj, uint8_t getResult,
    std::chrono::milliseconds timeout, ThinClientPoolDM* poolDM,
    const std::shared_ptr<std::recursive_mutex>& rCL,
    std::shared_ptr<ResultCollector> rs,
    std::shared_ptr<UserAttributes> userAttr, bool isBGThread,
    const std::shared_ptr<BucketServerLocation>& serverLocation,
    bool allBuckets)
    : m_serverLocation(serverLocation),
      m_isBGThread(isBGThread),
      m_poolDM(poolDM),
      m_func(func),
      m_getResult(getResult),
      m_timeout(timeout),
      m_args(args),
      m_routingObj(routingObj),
      m_rc(rs),
      m_resultCollectorLock(rCL),
      m_userAttr(userAttr),
      m_region(region),
      m_allBuckets(allBuckets) {
  m_request = new TcrMessageExecuteRegionFunctionSingleHop(
      new DataOutput(
          m_poolDM->getConnectionManager().getCacheImpl()->createDataOutput()),
      m_func, m_region, m_args, m_routingObj, m_getResult, nullptr,
      m_allBuckets, timeout, m_poolDM);
  m_reply = new TcrMessageReply(true, m_poolDM);
  m_resultCollector = new ChunkedFunctionExecutionResponse(
      *m_reply, (m_getResult & 2) == 2, m_rc, m_resultCollectorLock);
  m_reply->setChunkedResultHandler(m_resultCollector);
  m_reply->setTimeout(m_timeout);
  m_reply->setDM(m_poolDM);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
