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

#include <algorithm>
#include <ace/INET_Addr.h>

#include <geode/ResultCollector.hpp>
#include <geode/SystemProperties.hpp>
#include <geode/PoolManager.hpp>
#include <geode/AuthInitialize.hpp>

#include "statistics/PoolStatsSampler.hpp"
#include "ThinClientPoolDM.hpp"
#include "TcrEndpoint.hpp"
#include "ThinClientRegion.hpp"
#include "ExecutionImpl.hpp"
#include "ExpiryHandler_T.hpp"
#include "ExpiryTaskManager.hpp"
#include "DistributedSystemImpl.hpp"
#include "UserAttributes.hpp"
#include "ThinClientStickyManager.hpp"
#include "NonCopyable.hpp"
#include "util/exception.hpp"

using namespace apache::geode::client;
using namespace apache::geode::statistics;

/* adongre
 * CID 28730: Other violation (MISSING_COPY)
 * Class "GetAllWork" owns resources that are managed in its constructor and
 * destructor but has no user-written copy constructor.
 * FIX : Make the class NonCopyable
 */
class GetAllWork : public PooledWork<GfErrType>,
                   private NonCopyable,
                   private NonAssignable {
  ThinClientPoolDM* m_poolDM;
  std::shared_ptr<BucketServerLocation> m_serverLocation;
  TcrMessage* m_request;
  TcrMessageReply* m_reply;
  MapOfUpdateCounters m_mapOfUpdateCounters;
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
    m_request = new TcrMessageGetAll(region->getCache().createDataOutput(),
                                     region.get(), m_keys.get(), m_poolDM,
                                     m_aCallbackArgument);
    m_reply = new TcrMessageReply(true, m_poolDM);
    if (m_poolDM->isMultiUserMode()) {
      m_userAttribute = TSSUserAttributesWrapper::s_geodeTSSUserAttributes
                            ->getUserAttributes();
    }

    m_resultCollector = (new ChunkedGetAllResponse(
        *m_reply, dynamic_cast<ThinClientRegion*>(m_region.get()), m_keys.get(),
        m_responseHandler->getValues(), m_responseHandler->getExceptions(),
        m_responseHandler->getResultKeys(),
        m_responseHandler->getUpdateCounters(), 0, m_addToLocalCache,
        m_responseHandler->getResponseLock()));

    m_reply->setChunkedResultHandler(m_resultCollector);
  }

  ~GetAllWork() {
    delete m_request;
    delete m_reply;
    delete m_resultCollector;
  }

  TcrMessage* getReply() { return m_reply; }

  void init() {}
  GfErrType execute(void) {
    GuardUserAttribures gua;

    if (m_userAttribute != nullptr) {
      gua.setProxyCache(m_userAttribute->getProxyCache());
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
      m_updateLocatorListSema(0),
      m_pingSema(0),
      m_cliCallbackSema(0),
      m_isDestroyed(false),
      m_destroyPending(false),
      m_destroyPendingHADM(false),
      m_isMultiUserMode(false),
      m_locHelper(nullptr),
      m_poolSize(0),
      m_numRegions(0),
      m_server(0),
      m_connSema(0),
      m_connManageTask(nullptr),
      m_pingTask(nullptr),
      m_updateLocatorListTask(nullptr),
      m_cliCallbackTask(nullptr),
      m_pingTaskId(-1),
      m_updateLocatorListTaskId(-1),
      m_connManageTaskId(-1),
      m_PoolStatsSampler(nullptr),
      m_clientMetadataService(nullptr),
      m_primaryServerQueueSize(PRIMARY_QUEUE_NOT_AVAILABLE) {
  static bool firstGurd = false;
  if (firstGurd) ClientProxyMembershipID::increaseSynchCounter();
  firstGurd = true;

  auto cacheImpl = m_connManager.getCacheImpl();
  auto& distributedSystem = cacheImpl->getDistributedSystem();

  auto& sysProp = distributedSystem.getSystemProperties();
  // to set security flag at pool level
  this->m_isSecurityOn = cacheImpl->getAuthInitialize() != nullptr;

  ACE_TCHAR hostName[256];
  ACE_OS::hostname(hostName, sizeof(hostName) - 1);
  ACE_INET_Addr driver(hostName);
  uint32_t hostAddr = driver.get_ip_address();
  uint16_t hostPort = 0;
  auto&& durableId = sysProp.durableClientId();

  std::string clientDurableId = durableId;
  if (!m_poolName.empty()) {
    clientDurableId += "_gem_" + m_poolName;
  }

  const auto durableTimeOut = sysProp.durableTimeout();
  m_memId = cacheImpl->getClientProxyMembershipIDFactory().create(
      hostName, hostAddr, hostPort, clientDurableId.c_str(), durableTimeOut);

  if (m_attrs->m_initLocList.size() == 0 &&
      m_attrs->m_initServList.size() == 0) {
    std::string msg = "No locators or servers provided for pool named ";
    msg += name;
    throw IllegalStateException(msg);
  }
  reset();
  m_locHelper = new ThinClientLocatorHelper(m_attrs->m_initLocList, this);

  auto statisticsManager = distributedSystem.getStatisticsManager();
  m_stats =
      new PoolStats(statisticsManager->getStatisticsFactory(), m_poolName);
  statisticsManager->forceSample();

  if (!sysProp.isEndpointShufflingDisabled()) {
    if (m_attrs->m_initServList.size() > 0) {
      RandGen randgen;
      m_server = randgen(static_cast<uint32_t>(m_attrs->m_initServList.size()));
    }
  }
  if (m_attrs->getPRSingleHopEnabled()) {
    m_clientMetadataService = new ClientMetadataService(this);
  }
  m_manager = new ThinClientStickyManager(this);
}

void ThinClientPoolDM::init() {
  LOGDEBUG("ThinClientPoolDM::init: Starting pool initialization");
  auto cacheImpl = m_connManager.getCacheImpl();
  auto& sysProp = cacheImpl->getDistributedSystem().getSystemProperties();
  m_isMultiUserMode = this->getMultiuserAuthentication();

  if (m_isMultiUserMode) {
    LOGINFO("Multiuser authentication is enabled for pool %s",
            m_poolName.c_str());
  }
  // to set security flag at pool level
  this->m_isSecurityOn = cacheImpl->getAuthInitialize() != nullptr;

  LOGDEBUG("ThinClientPoolDM::init: security in on/off = %d ",
           this->m_isSecurityOn);

  m_connManager.init(true);

  LOGDEBUG("ThinClientPoolDM::init: is grid client = %d ",
           sysProp.isGridClient());

  if (!sysProp.isGridClient()) {
    ThinClientPoolDM::startBackgroundThreads();
  }

  LOGDEBUG("ThinClientPoolDM::init: Completed initialization");
}
std::shared_ptr<Properties> ThinClientPoolDM::getCredentials(TcrEndpoint* ep) {
  auto cacheImpl = m_connManager.getCacheImpl();
  const auto& distributedSystem = cacheImpl->getDistributedSystem();
  const auto& tmpSecurityProperties =
      distributedSystem.getSystemProperties().getSecurityProperties();

  if (const auto& authInitialize = cacheImpl->getAuthInitialize()) {
    LOGFINER(
        "ThinClientPoolDM::getCredentials: acquired handle to authLoader, "
        "invoking getCredentials %s",
        ep->name().c_str());
    const auto& tmpAuthIniSecurityProperties = authInitialize->getCredentials(
        tmpSecurityProperties, ep->name().c_str());
    LOGFINER("Done getting credentials");
    return tmpAuthIniSecurityProperties;
  }

  return nullptr;
}

void ThinClientPoolDM::startBackgroundThreads() {
  LOGDEBUG("ThinClientPoolDM::startBackgroundThreads: Starting ping thread");
  m_pingTask = new Task<ThinClientPoolDM>(this, &ThinClientPoolDM::pingServer,
                                          NC_Ping_Thread);
  m_pingTask->start();

  auto& props = m_connManager.getCacheImpl()
                    ->getDistributedSystem()
                    .getSystemProperties();

  if (props.onClientDisconnectClearPdxTypeIds() == true) {
    m_cliCallbackTask =
        new Task<ThinClientPoolDM>(this, &ThinClientPoolDM::cliCallback);
    m_cliCallbackTask->start();
  }

  LOGDEBUG("ThinClientPoolDM::startBackgroundThreads: Creating ping task");
  ACE_Event_Handler* pingHandler =
      new ExpiryHandler_T<ThinClientPoolDM>(this, &ThinClientPoolDM::doPing);

  long pingInterval = getPingInterval().count() / (1000 * 2);
  if (pingInterval > 0) {
    LOGDEBUG(
        "ThinClientPoolDM::startBackgroundThreads: Scheduling ping task at %ld",
        pingInterval);
    m_pingTaskId =
        m_connManager.getCacheImpl()->getExpiryTaskManager().scheduleExpiryTask(
            pingHandler, 1, pingInterval, false);
  } else {
    LOGDEBUG(
        "ThinClientPoolDM::startBackgroundThreads: Not Scheduling ping task as "
        "ping interval %ld",
        getPingInterval().count());
  }

  long updateLocatorListInterval = getUpdateLocatorListInterval().count();

  if (updateLocatorListInterval > 0) {
    m_updateLocatorListTask =
        new Task<ThinClientPoolDM>(this, &ThinClientPoolDM::updateLocatorList);
    m_updateLocatorListTask->start();

    updateLocatorListInterval = updateLocatorListInterval / 1000;  // seconds
    LOGDEBUG(
        "ThinClientPoolDM::startBackgroundThreads: Creating updateLocatorList "
        "task");
    ACE_Event_Handler* updateLocatorListHandler =
        new ExpiryHandler_T<ThinClientPoolDM>(
            this, &ThinClientPoolDM::doUpdateLocatorList);

    LOGDEBUG(
        "ThinClientPoolDM::startBackgroundThreads: Scheduling updater Locator "
        "task at %ld",
        updateLocatorListInterval);
    m_updateLocatorListTaskId =
        m_connManager.getCacheImpl()->getExpiryTaskManager().scheduleExpiryTask(
            updateLocatorListHandler, 1, updateLocatorListInterval, false);
  }

  LOGDEBUG(
      "ThinClientPoolDM::startBackgroundThreads: Starting manageConnections "
      "thread");
  // Manage Connection Thread
  m_connManageTask = new Task<ThinClientPoolDM>(
      this, &ThinClientPoolDM::manageConnections, NC_MC_Thread);
  m_connManageTask->start();

  auto idle = getIdleTimeout();
  auto load = getLoadConditioningInterval();

  if (load > std::chrono::milliseconds::zero()) {
    if (load < idle || idle <= std::chrono::milliseconds::zero()) {
      idle = load;
    }
  }

  if (idle > std::chrono::milliseconds::zero()) {
    LOGDEBUG(
        "ThinClientPoolDM::startBackgroundThreads: Starting manageConnections "
        "task");
    ACE_Event_Handler* connHandler = new ExpiryHandler_T<ThinClientPoolDM>(
        this, &ThinClientPoolDM::doManageConnections);

    LOGDEBUG(
        "ThinClientPoolDM::startBackgroundThreads: Scheduling "
        "manageConnections task");
    m_connManageTaskId =
        m_connManager.getCacheImpl()->getExpiryTaskManager().scheduleExpiryTask(
            connHandler, std::chrono::seconds(1), idle, false);
  }

  LOGDEBUG(
      "ThinClientPoolDM::startBackgroundThreads: Starting remote query "
      "service");
  // Init Query Service
  m_remoteQueryServicePtr =
      std::make_shared<RemoteQueryService>(m_connManager.getCacheImpl(), this);
  m_remoteQueryServicePtr->init();

  LOGDEBUG(
      "ThinClientPoolDM::startBackgroundThreads: Starting pool stat sampler");
  if (m_PoolStatsSampler == nullptr &&
      getStatisticInterval() > std::chrono::milliseconds::zero() &&
      props.statisticsEnabled()) {
    m_PoolStatsSampler = new PoolStatsSampler(
        getStatisticInterval(), m_connManager.getCacheImpl(), this);
    m_PoolStatsSampler->start();
  }

  // starting chunk processing helper thread
  ThinClientBaseDM::init();

  if (m_clientMetadataService != nullptr) {
    m_clientMetadataService->start();
  }
}
int ThinClientPoolDM::manageConnections(volatile bool& isRunning) {
  LOGFINE("ThinClientPoolDM: starting manageConnections thread");

  while (isRunning) {
    m_connSema.acquire();
    if (isRunning) {
      manageConnectionsInternal(isRunning);
      while (m_connSema.tryacquire() != -1) {
        ;
      }
    }
  }
  LOGFINE("ThinClientPoolDM: ending manageConnections thread");
  return 0;
}

void ThinClientPoolDM::cleanStaleConnections(volatile bool& isRunning) {
  if (!isRunning) {
    return;
  }

  LOGDEBUG("Cleaning stale connections");

  ACE_Time_Value _idle(getIdleTimeout());
  ACE_Time_Value _nextIdle = _idle;
  {
    TcrConnection* conn = nullptr;

    std::vector<TcrConnection*> savelist;
    std::vector<TcrConnection*> replacelist;
    std::set<ServerLocation> excludeServers;

    while ((conn = getNoWait()) != nullptr && isRunning) {
      if (canItBeDeleted(conn)) {
        replacelist.push_back(conn);
      } else if (conn) {
        ACE_Time_Value nextIdle =
            _idle - (ACE_OS::gettimeofday() - conn->getLastAccessed());
        if ((ACE_Time_Value(0, 0) < nextIdle) && (nextIdle < _nextIdle)) {
          _nextIdle = nextIdle;
        }
        savelist.push_back(conn);
      }
    }

    size_t replaceCount =
        m_attrs->getMinConnections() - static_cast<int>(savelist.size());

    for (std::vector<TcrConnection*>::const_iterator iter = replacelist.begin();
         iter != replacelist.end(); ++iter) {
      TcrConnection* conn = *iter;
      if (replaceCount <= 0) {
        GF_SAFE_DELETE_CON(conn);
        removeEPConnections(1, false);
        getStats().incLoadCondDisconnects();
        LOGDEBUG("Removed a connection");
      } else {
        GfErrType error = GF_NOERR;
        TcrConnection* newConn = nullptr;
        bool maxConnLimit = false;
        error = createPoolConnection(newConn, excludeServers, maxConnLimit,
                                     /*hasExpired(conn) ? nullptr :*/ conn);
        if (newConn) {
          ACE_Time_Value nextIdle =
              _idle - (ACE_OS::gettimeofday() - newConn->getLastAccessed());
          if ((ACE_Time_Value(0, 0) < nextIdle) && (nextIdle < _nextIdle)) {
            _nextIdle = nextIdle;
          }
          savelist.push_back(newConn);
          if (newConn != conn) {
            GF_SAFE_DELETE_CON(conn);
            removeEPConnections(1, false);
            getStats().incLoadCondDisconnects();
            LOGDEBUG("Removed a connection");
          }
        } else {
          if (hasExpired(conn)) {
            GF_SAFE_DELETE_CON(conn);
            removeEPConnections(1, false);
            getStats().incLoadCondDisconnects();
            LOGDEBUG("Removed a connection");
          } else {
            conn->updateCreationTime();
            ACE_Time_Value nextIdle =
                _idle - (ACE_OS::gettimeofday() - conn->getLastAccessed());
            if ((ACE_Time_Value(0, 0) < nextIdle) && (nextIdle < _nextIdle)) {
              _nextIdle = nextIdle;
            }
            savelist.push_back(conn);
          }
        }
      }
      replaceCount--;
    }

    LOGDEBUG("Preserving %d connections", savelist.size());

    for (std::vector<TcrConnection*>::const_iterator iter = savelist.begin();
         iter != savelist.end(); ++iter) {
      put(*iter, false);
    }
  }
  if (m_connManageTaskId >= 0 && isRunning &&
      m_connManager.getCacheImpl()->getExpiryTaskManager().resetTask(
          m_connManageTaskId, static_cast<uint32_t>(_nextIdle.sec() + 1))) {
    LOGERROR("Failed to reschedule connection manager");
  } else {
    LOGFINEST("Rescheduled next connection manager run after %d seconds",
              _nextIdle.sec() + 1);
  }

  LOGDEBUG("Pool size is %d, pool counter is %d", size(), m_poolSize.load());
}
void ThinClientPoolDM::cleanStickyConnections(volatile bool& isRunning) {}

void ThinClientPoolDM::restoreMinConnections(volatile bool& isRunning) {
  if (!isRunning) {
    return;
  }

  LOGDEBUG("Restoring minimum connection level");

  int min = m_attrs->getMinConnections();
  int limit = 2 * min;

  std::set<ServerLocation> excludeServers;

  int restored = 0;

  if (m_poolSize < min) {
    ACE_Guard<ACE_Recursive_Thread_Mutex> poolguard(m_queueLock);
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

  LOGDEBUG("Restored %d connections", restored);
  LOGDEBUG("Pool size is %d, pool counter is %d", size(), m_poolSize.load());
}

int ThinClientPoolDM::manageConnectionsInternal(volatile bool& isRunning) {
  try {
    LOGFINE(
        "ThinClientPoolDM::manageConnections(): checking connections in pool "
        "queue %d",
        size());

    cleanStaleConnections(isRunning);

    cleanStickyConnections(isRunning);

    restoreMinConnections(isRunning);

    getStats().setCurPoolConnections(m_poolSize);
  } catch (const Exception& e) {
    LOGERROR(e.what());
  } catch (const std::exception& e) {
    LOGERROR(e.what());
  } catch (...) {
    LOGERROR("Unexpected exception during manage connections");
  }
  return 0;
}

std::string ThinClientPoolDM::selectEndpoint(
    std::set<ServerLocation>& excludeServers,
    const TcrConnection* currentServer) {
  if (m_attrs->m_initLocList.size()) {  // query locators
    ServerLocation outEndpoint;
    std::string additionalLoc;
    LOGFINE("Asking locator for server from group [%s]",
            m_attrs->m_serverGrp.c_str());

    // Update Locator Request Stats
    getStats().incLoctorRequests();

    if (GF_NOERR != ((ThinClientLocatorHelper*)m_locHelper)
                        ->getEndpointForNewFwdConn(
                            outEndpoint, additionalLoc, excludeServers,
                            m_attrs->m_serverGrp, currentServer)) {
      throw IllegalStateException("Locator query failed");
    }
    // Update Locator stats
    getStats().setLocators(
        ((ThinClientLocatorHelper*)m_locHelper)->getCurLocatorsNum());
    getStats().incLoctorResposes();

    char epNameStr[128] = {0};
    ACE_OS::snprintf(epNameStr, 128, "%s:%d",
                     outEndpoint.getServerName().c_str(),
                     outEndpoint.getPort());
    LOGFINE("ThinClientPoolDM: Locator returned endpoint [%s]", epNameStr);
    return epNameStr;
  } else if (m_attrs->m_initServList
                 .size()) {  // use specified server endpoints
    // highly complex round-robin algorithm
    ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_endpointSelectionLock);
    if (m_server >= m_attrs->m_initServList.size()) {
      m_server = 0;
    }

    unsigned int epCount = 0;
    do {
      if (!excludeServer(m_attrs->m_initServList[m_server], excludeServers)) {
        LOGFINE("ThinClientPoolDM: Selecting endpoint [%s] from position %d",
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
    LOGERROR("No locators or servers provided");
    throw IllegalStateException("No locators or servers provided");
  }
}

void ThinClientPoolDM::addConnection(TcrConnection* conn) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(getPoolLock());

  put(conn, false);
  ++m_poolSize;
}
GfErrType ThinClientPoolDM::sendRequestToAllServers(
    const char* func, uint8_t getResult, std::chrono::milliseconds timeout,
    std::shared_ptr<Cacheable> args, std::shared_ptr<ResultCollector>& rs,
    std::shared_ptr<CacheableString>& exceptionPtr) {
  GfErrType err = GF_NOERR;

  getStats().setCurClientOps(++m_clientOps);

  auto resultCollectorLock = std::make_shared<ACE_Recursive_Thread_Mutex>();

  auto csArray = getServers();

  if (csArray != nullptr && csArray->length() == 0) {
    LOGWARN("No server found to execute the function");
    return GF_NOSERVER_FOUND;
  }

  int feIndex = 0;
  FunctionExecution* fePtrList = new FunctionExecution[csArray->length()];
  auto* threadPool = m_connManager.getCacheImpl()->getThreadPool();
  auto userAttr =
      TSSUserAttributesWrapper::s_geodeTSSUserAttributes->getUserAttributes();
  for (int i = 0; i < csArray->length(); i++) {
    auto cs = (*csArray)[i];
    std::string endpointStr(cs->value().c_str());
    TcrEndpoint* ep = nullptr;
    if (m_endpoints.find(endpointStr, ep)) {
      ep = addEP(cs->value().c_str());
    } else if (!ep->connected()) {
      LOGFINE(
          "ThinClientPoolDM::sendRequestToAllServers server not connected %s ",
          cs->value().c_str());
    }
    FunctionExecution* funcExe = &fePtrList[feIndex++];
    funcExe->setParameters(func, getResult, timeout, args, ep, this,
                           resultCollectorLock, &rs, userAttr);
    threadPool->perform(funcExe);
  }
  GfErrType finalErrorReturn = GF_NOERR;

  for (int i = 0; i < feIndex; i++) {
    FunctionExecution* funcExe = &fePtrList[i];
    err = funcExe->getResult();
    if (err != GF_NOERR) {
      if (funcExe->getException() == nullptr) {
        if (err == GF_TIMOUT) {
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
    } else if (!(finalErrorReturn == GF_AUTHENTICATION_FAILED_EXCEPTION ||
                 finalErrorReturn == GF_NOT_AUTHORIZED_EXCEPTION ||
                 finalErrorReturn ==
                     GF_AUTHENTICATION_REQUIRED_EXCEPTION))  // returning auth
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

  delete[] fePtrList;
  return finalErrorReturn;
}

const std::shared_ptr<CacheableStringArray> ThinClientPoolDM::getLocators()
    const {
  auto ptrArr =
      new std::shared_ptr<CacheableString>[m_attrs->m_initLocList.size()];
  int32_t i = 0;
  for (auto&& locator : m_attrs->m_initLocList) {
    ptrArr[i++] = CacheableString::create(locator);
  }
  return CacheableStringArray::create(
	  std::vector<std::shared_ptr<CacheableString>>(ptrArr, ptrArr + i));
}

const std::shared_ptr<CacheableStringArray> ThinClientPoolDM::getServers() {
  if (!m_attrs->m_initServList.empty()) {
    auto ptrArr =
        new std::shared_ptr<CacheableString>[m_attrs->m_initServList.size()];
    int32_t i = 0;
    for (auto&& server : m_attrs->m_initServList) {
      ptrArr[i++] = CacheableString::create(server);
    }
    return CacheableStringArray::create(
		std::vector<std::shared_ptr<CacheableString>>(ptrArr, ptrArr + i));
  } else if (!m_attrs->m_initLocList.empty()) {
    std::vector<ServerLocation> vec;
    // TODO thread - why is this member volatile?
    const_cast<ThinClientLocatorHelper*>(
        reinterpret_cast<volatile ThinClientLocatorHelper*>(m_locHelper))
        ->getAllServers(vec, m_attrs->m_serverGrp);

    auto ptrArr = new std::shared_ptr<CacheableString>[vec.size()];
    int32_t i = 0;
    for (auto&& serLoc : vec) {
      ptrArr[i++] = CacheableString::create(serLoc.getServerName() + ":" +
                                            std::to_string(serLoc.getPort()));
    }
    return CacheableStringArray::create(std::vector<std::shared_ptr<CacheableString>>(ptrArr, ptrArr + i));
  } else {
    return CacheableStringArray::create(std::vector<std::shared_ptr<CacheableString>>{});
  }
}

void ThinClientPoolDM::stopPingThread() {
  if (m_pingTask) {
    LOGFINE("ThinClientPoolDM::destroy(): Closing ping thread.");
    m_pingTask->stopNoblock();
    m_pingSema.release();
    m_pingTask->wait();
    _GEODE_SAFE_DELETE(m_pingTask);
    if (m_pingTaskId >= 0) {
      m_connManager.getCacheImpl()->getExpiryTaskManager().cancelTask(
          m_pingTaskId);
    }
  }
}

void ThinClientPoolDM::stopUpdateLocatorListThread() {
  if (m_updateLocatorListTask) {
    LOGFINE("ThinClientPoolDM::destroy(): Closing updateLocatorList thread.");
    m_updateLocatorListTask->stopNoblock();
    m_updateLocatorListSema.release();
    m_updateLocatorListTask->wait();
    _GEODE_SAFE_DELETE(m_updateLocatorListTask);
    if (m_updateLocatorListTaskId >= 0) {
      m_connManager.getCacheImpl()->getExpiryTaskManager().cancelTask(
          m_updateLocatorListTaskId);
    }
  }
}

void ThinClientPoolDM::stopCliCallbackThread() {
  if (m_cliCallbackTask) {
    LOGFINE("ThinClientPoolDM::destroy(): Closing cliCallback thread.");
    m_cliCallbackTask->stopNoblock();
    m_cliCallbackSema.release();
    m_cliCallbackTask->wait();
    _GEODE_SAFE_DELETE(m_cliCallbackTask);
  }
}

void ThinClientPoolDM::destroy(bool keepAlive) {
  LOGDEBUG("ThinClientPoolDM::destroy...");
  if (!m_isDestroyed && (!m_destroyPending || m_destroyPendingHADM)) {
    checkRegions();
    TcrMessage::setKeepAlive(keepAlive);
    if (m_remoteQueryServicePtr != nullptr) {
      m_remoteQueryServicePtr->close();
      m_remoteQueryServicePtr = nullptr;
    }

    LOGDEBUG("Closing PoolStatsSampler thread.");
    if (m_PoolStatsSampler != nullptr) {
      m_PoolStatsSampler->stop();
      _GEODE_SAFE_DELETE(m_PoolStatsSampler);
    }
    LOGDEBUG("PoolStatsSampler thread closed .");
    stopCliCallbackThread();
    LOGDEBUG("ThinClientPoolDM::destroy( ): Closing connection manager.");
    if (m_connManageTask) {
      m_connManageTask->stopNoblock();
      m_connSema.release();
      m_connManageTask->wait();
      _GEODE_SAFE_DELETE(m_connManageTask);
      if (m_connManageTaskId >= 0) {
        m_connManager.getCacheImpl()->getExpiryTaskManager().cancelTask(
            m_connManageTaskId);
      }
    }

    LOGDEBUG("Closing PoolStatsSampler thread.");
    stopPingThread();
    stopUpdateLocatorListThread();

    if (m_clientMetadataService != nullptr) {
      m_clientMetadataService->stop();
    }
    // closing all the thread local connections ( sticky).
    LOGDEBUG("ThinClientPoolDM::destroy( ): closing FairQueue, pool size = %d",
             m_poolSize.load());
    close();
    LOGDEBUG("ThinClientPoolDM::destroy( ): after close ");

    for (ACE_Map_Manager<std::string, TcrEndpoint*,
                         ACE_Recursive_Thread_Mutex>::iterator iter =
             m_endpoints.begin();
         iter != m_endpoints.end(); ++iter) {
      TcrEndpoint* ep = (*iter).int_id_;
      LOGFINE("ThinClientPoolDM: forcing endpoint delete for %d in destructor",
              ep->name().c_str());
      _GEODE_SAFE_DELETE(ep);
    }

    // Close Stats
    getStats().close();
    m_connManager.getCacheImpl()
        ->getDistributedSystem()
        .getStatisticsManager()
        ->forceSample();

    if (m_clientMetadataService != nullptr) {
      _GEODE_SAFE_DELETE(m_clientMetadataService);
    }

    m_connManager.getCacheImpl()->getPoolManager().removePool(
        m_poolName.c_str());

    stopChunkProcessor();
    m_manager->closeAllStickyConnections();
    m_isDestroyed = true;
    LOGDEBUG("ThinClientPoolDM::destroy( ): after close m_isDestroyed = %d ",
             m_isDestroyed);
  }
  if (m_poolSize != 0) {
    LOGFINE("Pool connection size is not zero %d", m_poolSize.load());
  }
}

bool ThinClientPoolDM::isDestroyed() const {
  // TODO: dummy implementation
  return m_isDestroyed;
}
std::shared_ptr<QueryService> ThinClientPoolDM::getQueryService() {
  // TODO:
  if (m_isMultiUserMode) {
    LOGERROR(
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
  auto& props = m_connManager.getCacheImpl()
                    ->getDistributedSystem()
                    .getSystemProperties();

  if (props.isGridClient()) {
    LOGWARN("Initializing query service while grid-client setting is enabled.");
    // Init Query Service
    m_remoteQueryServicePtr = std::make_shared<RemoteQueryService>(
        m_connManager.getCacheImpl(), this);
    m_remoteQueryServicePtr->init();
  } else {
    LOGWARN("Remote query service is not initialized.");
  }

  return m_remoteQueryServicePtr;
}
void ThinClientPoolDM::sendUserCacheCloseMessage(bool keepAlive) {
  LOGDEBUG("ThinClientPoolDM::sendUserCacheCloseMessage");
  auto userAttribute =
      TSSUserAttributesWrapper::s_geodeTSSUserAttributes->getUserAttributes();

  std::map<std::string, UserConnectionAttributes*>& uca =
      userAttribute->getUserConnectionServers();

  std::map<std::string, UserConnectionAttributes*>::iterator it;

  for (it = uca.begin(); it != uca.end(); it++) {
    UserConnectionAttributes* uca = (*it).second;
    if (uca->isAuthenticated() && uca->getEndpoint()->connected()) {
      TcrMessageRemoveUserAuth request(
          m_connManager.getCacheImpl()->getCache()->createDataOutput(),
          keepAlive, this);
      TcrMessageReply reply(true, this);

      sendRequestToEP(request, reply, uca->getEndpoint());

      uca->setUnAuthenticated();
    } else {
      uca->setUnAuthenticated();
    }
  }
}

TcrConnection* ThinClientPoolDM::getConnectionInMultiuserMode(
    std::shared_ptr<UserAttributes> userAttribute) {
  LOGDEBUG("ThinClientPoolDM::getConnectionInMultiuserMode:");
  UserConnectionAttributes* uca = userAttribute->getConnectionAttribute();
  if (uca != nullptr) {
    TcrEndpoint* ep = uca->getEndpoint();
    LOGDEBUG(
        "ThinClientPoolDM::getConnectionInMultiuserMode endpoint got = %s ",
        ep->name().c_str());
    return getFromEP(ep);
  } else {
    return nullptr;
  }
}

int32_t ThinClientPoolDM::GetPDXIdForType(
    std::shared_ptr<Serializable> pdxType) {
  LOGDEBUG("ThinClientPoolDM::GetPDXIdForType:");

  GfErrType err = GF_NOERR;

  TcrMessageGetPdxIdForType request(
      m_connManager.getCacheImpl()->getCache()->createDataOutput(), pdxType,
      this);

  TcrMessageReply reply(true, this);

  err = sendSyncRequest(request, reply);

  if (err != GF_NOERR) {
    GfErrTypeToException("Operation Failed", err);
  } else if (reply.getMessageType() == TcrMessage::EXCEPTION) {
    LOGDEBUG("ThinClientPoolDM::GetPDXTypeById: Exception = %s ",
             reply.getException());
    throw IllegalStateException("Failed to register PdxSerializable Type");
  }

  int32_t pdxTypeId =
      static_cast<CacheableInt32*>(reply.getValue().get())->value();

  // need to broadcast this id to all other pool
  {
    auto& poolManager =
        m_connManager.getCacheImpl()->getCache()->getPoolManager();
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
  LOGDEBUG("ThinClientPoolDM::GetPDXIdForType:");

  GfErrType err = GF_NOERR;

  TcrMessageAddPdxType request(
      m_connManager.getCacheImpl()->getCache()->createDataOutput(), pdxType,
      this, pdxTypeId);

  TcrMessageReply reply(true, this);

  err = sendSyncRequest(request, reply);

  if (err != GF_NOERR) {
    GfErrTypeToException("Operation Failed", err);
  } else if (reply.getMessageType() == TcrMessage::EXCEPTION) {
    LOGDEBUG("ThinClientPoolDM::GetPDXTypeById: Exception = %s ",
             reply.getException());
    throw IllegalStateException("Failed to register PdxSerializable Type");
  }
}
std::shared_ptr<Serializable> ThinClientPoolDM::GetPDXTypeById(int32_t typeId) {
  LOGDEBUG("ThinClientPoolDM::GetPDXTypeById:");

  GfErrType err = GF_NOERR;

  TcrMessageGetPdxTypeById request(
      m_connManager.getCacheImpl()->getCache()->createDataOutput(), typeId,
      this);

  TcrMessageReply reply(true, this);

  err = sendSyncRequest(request, reply);

  if (err != GF_NOERR) {
    GfErrTypeToException("Operation Failed", err);
  } else if (reply.getMessageType() == TcrMessage::EXCEPTION) {
    LOGDEBUG("ThinClientPoolDM::GetPDXTypeById: Exception = %s ",
             reply.getException());
    throw IllegalStateException("Failed to understand PdxSerializable Type");
  }

  return reply.getValue();
}

int32_t ThinClientPoolDM::GetEnumValue(std::shared_ptr<Serializable> enumInfo) {
  LOGDEBUG("ThinClientPoolDM::GetEnumValue:");

  GfErrType err = GF_NOERR;

  TcrMessageGetPdxIdForEnum request(
      m_connManager.getCacheImpl()->getCache()->createDataOutput(), enumInfo,
      this);

  TcrMessageReply reply(true, this);

  err = sendSyncRequest(request, reply);

  if (err != GF_NOERR) {
    GfErrTypeToException("Operation Failed", err);
  } else if (reply.getMessageType() == TcrMessage::EXCEPTION) {
    LOGDEBUG("ThinClientPoolDM::GetEnumValue: Exception = %s ",
             reply.getException());
    throw IllegalStateException("Failed to register Pdx enum Type");
  }

  int32_t enumVal =
      static_cast<CacheableInt32*>(reply.getValue().get())->value();

  // need to broadcast this id to all other pool
  {
    auto& poolManager =
        m_connManager.getCacheImpl()->getCache()->getPoolManager();
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
  LOGDEBUG("ThinClientPoolDM::GetEnum:");

  GfErrType err = GF_NOERR;

  TcrMessageGetPdxEnumById request(
      m_connManager.getCacheImpl()->getCache()->createDataOutput(), val, this);

  TcrMessageReply reply(true, this);

  err = sendSyncRequest(request, reply);

  if (err != GF_NOERR) {
    GfErrTypeToException("Operation Failed", err);
  } else if (reply.getMessageType() == TcrMessage::EXCEPTION) {
    LOGDEBUG("ThinClientPoolDM::GetEnum: Exception = %s ",
             reply.getException());
    throw IllegalStateException("Failed to understand enum Type");
  }

  return reply.getValue();
}

void ThinClientPoolDM::AddEnum(std::shared_ptr<Serializable> enumInfo,
                               int enumVal) {
  LOGDEBUG("ThinClientPoolDM::AddEnum:");

  GfErrType err = GF_NOERR;

  TcrMessageAddPdxEnum request(
      m_connManager.getCacheImpl()->getCache()->createDataOutput(), enumInfo,
      this, enumVal);

  TcrMessageReply reply(true, this);

  err = sendSyncRequest(request, reply);

  if (err != GF_NOERR) {
    GfErrTypeToException("Operation Failed", err);
  } else if (reply.getMessageType() == TcrMessage::EXCEPTION) {
    LOGDEBUG("ThinClientPoolDM::AddEnum: Exception = %s ",
             reply.getException());
    throw IllegalStateException("Failed to register enum Type");
  }
}

GfErrType ThinClientPoolDM::sendUserCredentials(
    std::shared_ptr<Properties> credentials, TcrConnection*& conn,
    bool isBGThread, bool& isServerException) {
  LOGDEBUG("ThinClientPoolDM::sendUserCredentials:");

  GfErrType err = GF_NOERR;

  TcrMessageUserCredential request(
      m_connManager.getCacheImpl()->getCache()->createDataOutput(), credentials,
      this);

  TcrMessageReply reply(true, this);

  err =
      conn->getEndpointObject()->sendRequestConnWithRetry(request, reply, conn);

  if (conn) err = handleEPError(conn->getEndpointObject(), reply, err);

  LOGDEBUG(
      "ThinClientPoolDM::sendUserCredentials: Error after sending cred request "
      "= %d ",
      err);

  if (err == GF_NOERR) {
    switch (reply.getMessageType()) {
      case TcrMessage::RESPONSE: {
        // nothing to be done;
        break;
      }
      case TcrMessage::EXCEPTION: {
        if (err == GF_NOERR) {
          putInQueue(
              conn, isBGThread);  // connFound is only relevant for Sticky conn.
        }
        // this will set error type if there is some server exception
        err = ThinClientRegion::handleServerException(
            "ThinClientPoolDM::sendUserCredentials AuthException",
            reply.getException());
        isServerException = true;
        break;
      }
      default: {
        if (err == GF_NOERR) {
          putInQueue(
              conn, isBGThread);  // connFound is only relevant for Sticky conn.
        }
        LOGERROR(
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
  if (m_clientMetadataService == nullptr || key == nullptr) return nullptr;
  auto r = request.getRegion();
  auto region = nullptr == r ? nullptr : r->shared_from_this();
  TcrEndpoint* ep = nullptr;
  if (region == nullptr) {
    m_connManager.getCacheImpl()->getRegion(request.getRegionName().c_str(),
                                            region);
  }
  if (region != nullptr) {
    m_clientMetadataService->getBucketServerLocation(
        region, key, request.getValueRef(), request.getCallbackArgumentRef(),
        request.forPrimary(), serverlocation, version);

    if (serverlocation != nullptr && serverlocation->isValid()) {
      LOGFINE("Server host and port are %s:%d",
              serverlocation->getServerName().c_str(),
              serverlocation->getPort());
      ep = getEndPoint(serverlocation, version, excludeServers);
    }
  }
  return ep;
}

TcrEndpoint* ThinClientPoolDM::getEndPoint(
    const std::shared_ptr<BucketServerLocation>& serverLocation,
    int8_t& version, std::set<ServerLocation>& excludeServers) {
  TcrEndpoint* ep = nullptr;
  if (serverLocation->isValid()) {
    if (excludeServer(serverLocation->getEpString(), excludeServers)) {
      LOGFINE("ThinClientPoolDM::getEndPoint Exclude Server true for %s ",
              serverLocation->getEpString().c_str());
      return ep;
    }

    if (m_endpoints.find(serverLocation->getEpString(), ep) != -1) {
      LOGDEBUG("Endpoint for single hop is %p", ep);
      return ep;
    }

    // do for pool with endpoints. Add endpoint into m_endpoints only when we
    // did not find it above and it is in the pool's m_initServList.
    for (std::vector<std::string>::iterator itr =
             m_attrs->m_initServList.begin();
         itr != m_attrs->m_initServList.end(); ++itr) {
      if ((ACE_OS::strcmp(serverLocation->getEpString().c_str(),
                          (*itr).c_str()) == 0)) {
        ep = addEP(*(serverLocation.get()));  // see if this is new endpoint
        break;
      }
    }

    // do only for locator
    // if servergroup is there, then verify otherwise you may reach to another
    // group
    if (m_attrs->m_initLocList.size()) {
      auto&& servGrp = this->getServerGroup();
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

  return ep;
}

// gets the endpoint from the list of endpoints using the endpoint Name
TcrEndpoint* ThinClientPoolDM::getEndPoint(std::string epNameStr) {
  TcrEndpoint* ep = nullptr;
  if (m_endpoints.find(epNameStr, ep) != -1) {
    LOGDEBUG("Endpoint for single hop is %p", ep);
    return ep;
  }
  return ep;
}

GfErrType ThinClientPoolDM::sendSyncRequest(TcrMessage& request,
                                            TcrMessageReply& reply,
                                            bool attemptFailover,
                                            bool isBGThread) {
  int32_t type = request.getMessageType();

  if (!request.forTransaction() && m_attrs->getPRSingleHopEnabled() &&
      (type == TcrMessage::GET_ALL_70 ||
       type == TcrMessage::GET_ALL_WITH_CALLBACK) &&
      m_clientMetadataService != nullptr) {
    GfErrType error = GF_NOERR;
    std::shared_ptr<Region> region;
    m_connManager.getCacheImpl()->getRegion(request.getRegionName().c_str(),
                                            region);
    auto locationMap = m_clientMetadataService->getServerToFilterMap(
        *(request.getKeys()), region, request.forPrimary());

    if (!locationMap) {
      request.InitializeGetallMsg(
          request.getCallbackArgument());  // now initialize getall msg
      return sendSyncRequest(request, reply, attemptFailover, isBGThread,
                             nullptr);
    }
    std::vector<GetAllWork*> getAllWorkers;
    auto* threadPool = m_connManager.getCacheImpl()->getThreadPool();
    ChunkedGetAllResponse* responseHandler =
        static_cast<ChunkedGetAllResponse*>(reply.getChunkedResultHandler());

    for (const auto& locationIter : *locationMap) {
      const auto& serverLocation = locationIter.first;
      if (serverLocation == nullptr) {
      }
      const auto& keys = locationIter.second;
      auto worker =
          new GetAllWork(this, region, serverLocation, keys, attemptFailover,
                         isBGThread, responseHandler->getAddToLocalCache(),
                         responseHandler, request.getCallbackArgument());
      threadPool->perform(worker);
      getAllWorkers.push_back(worker);
    }
    reply.setMessageType(TcrMessage::RESPONSE);

    for (std::vector<GetAllWork*>::iterator iter = getAllWorkers.begin();
         iter != getAllWorkers.end(); iter++) {
      GetAllWork* worker = *iter;
      GfErrType err = worker->getResult();

      if (err != GF_NOERR) {
        error = err;
      }

      TcrMessage* currentReply = worker->getReply();
      if (currentReply->getMessageType() != TcrMessage::RESPONSE) {
        reply.setMessageType(currentReply->getMessageType());
      }

      delete worker;
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
  LOGDEBUG("ThinClientPoolDM::sendSyncRequest: ....%d %s",
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
    reply.setTimeout(this->getReadTimeout());
    request.setTimeout(this->getReadTimeout());
  }

  bool retryAllEPsOnce = false;
  if (m_attrs->getRetryAttempts() == -1) {
    retryAllEPsOnce = true;
  }
  long retry = m_attrs->getRetryAttempts() + 1;
  TcrConnection* conn = nullptr;
  std::set<ServerLocation> excludeServers;
  type = request.getMessageType();
  bool isAuthRequireExcep = false;
  int isAuthRequireExcepMaxTry = 2;
  bool firstTry = true;
  LOGFINE("sendSyncRequest:: retry = %d", retry);
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
        error == GF_TIMOUT) {
      return error;
    }

    GfErrType queueErr = GF_NOERR;
    uint32_t lastExcludeSize = static_cast<uint32_t>(excludeServers.size());
    int8_t version = 0;

    bool isUserNeedToReAuthenticate = false;
    bool singleHopConnFound = false;
    bool connFound = false;
    if (!this->m_isMultiUserMode ||
        (!TcrMessage::isUserInitiativeOps(request))) {
      conn = getConnectionFromQueueW(&queueErr, excludeServers, isBGThread,
                                     request, version, singleHopConnFound,
                                     connFound, serverLocation);
    } else {
      userAttr = TSSUserAttributesWrapper::s_geodeTSSUserAttributes
                     ->getUserAttributes();
      if (userAttr == nullptr) {
        LOGWARN("Attempted operation type %d without credentials",
                request.getMessageType());
        return GF_NOT_AUTHORIZED_EXCEPTION;
      }
      // Can i assume here that we will always get connection here
      conn = getConnectionFromQueueW(&queueErr, excludeServers, isBGThread,
                                     request, version, singleHopConnFound,
                                     connFound, serverLocation);

      LOGDEBUG(
          "ThinClientPoolDM::sendSyncRequest: after "
          "getConnectionInMultiuserMode %d",
          isUserNeedToReAuthenticate);
      if (conn !=
          nullptr) {  // need to chk whether user is already authenticated
                      // to this endpoint or not.
        isUserNeedToReAuthenticate =
            !(userAttr->isEndpointAuthenticated(conn->getEndpointObject()));
      }
    }

    if (queueErr == GF_CLIENT_WAIT_TIMEOUT) {
      LOGFINE("Request timeout at client only");
      return GF_CLIENT_WAIT_TIMEOUT;
    } else if (queueErr == GF_CLIENT_WAIT_TIMEOUT_REFRESH_PRMETADATA) {
      // need to refresh meta data
      std::shared_ptr<Region> region;
      m_connManager.getCacheImpl()->getRegion(request.getRegionName().c_str(),
                                              region);
      if (region != nullptr) {
        LOGFINE(
            "Need to refresh pr-meta-data timeout in client only  with refresh "
            "metadata");
        ThinClientRegion* tcrRegion =
            dynamic_cast<ThinClientRegion*>(region.get());
        tcrRegion->setMetaDataRefreshed(false);
        m_clientMetadataService->enqueueForMetadataRefresh(
            region->getFullPath(), reply.getserverGroupVersion());
      }
      return GF_CLIENT_WAIT_TIMEOUT_REFRESH_PRMETADATA;
    }

    LOGDEBUG(
        "ThinClientPoolDM::sendSyncRequest: isUserNeedToReAuthenticate = %d ",
        isUserNeedToReAuthenticate);
    LOGDEBUG(
        "ThinClientPoolDM::sendSyncRequest: m_isMultiUserMode = %d  conn = %d  "
        "type = %d",
        m_isMultiUserMode, conn, type);

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
      LOGDEBUG(
          "ThinClientPoolDM::sendSyncRequest: sendSyncReq "
          "ep->isAuthenticated() = %d ",
          ep->isAuthenticated());
      GfErrType userCredMsgErr = GF_NOERR;
      bool isServerException = false;
      if (TcrMessage::isUserInitiativeOps(request) &&
          (this->m_isSecurityOn || this->m_isMultiUserMode)) {
        if (!this->m_isMultiUserMode && !ep->isAuthenticated()) {
          // first authenticate him on this endpoint
          userCredMsgErr = this->sendUserCredentials(
              this->getCredentials(ep), conn, isBGThread, isServerException);
        } else if (isUserNeedToReAuthenticate) {
          userCredMsgErr = this->sendUserCredentials(
              userAttr->getCredentials(), conn, isBGThread, isServerException);
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
          LOGDEBUG("putting connection back in queue");
          putInQueue(conn,
                     isBGThread ||
                         request.getMessageType() == TcrMessage::GET_ALL_70 ||
                         request.getMessageType() ==
                             TcrMessage::GET_ALL_WITH_CALLBACK ||
                         request.getMessageType() ==
                             TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP,
                     request.forTransaction());  // connFound is only relevant
                                                 // for Sticky conn.
          LOGDEBUG("putting connection back in queue DONE");
        } else {
          if (error != GF_TIMOUT) removeEPConnections(ep);
          // Update stats for the connection that failed.
          removeEPConnections(1, false);
          setStickyNull(isBGThread ||
                        request.getMessageType() == TcrMessage::GET_ALL_70 ||
                        request.getMessageType() ==
                            TcrMessage::GET_ALL_WITH_CALLBACK ||
                        request.getMessageType() ==
                            TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP);
          if (conn) {
            GF_SAFE_DELETE_CON(conn)
          }
          excludeServers.insert(ServerLocation(ep->name()));
        }
      } else {
        return error;  // server exception while sending credentail message to
      }
      // server...
    }

    if (error == GF_NOERR) {
      if ((this->m_isSecurityOn || this->m_isMultiUserMode)) {
        if (reply.getMessageType() == TcrMessage::EXCEPTION) {
          if (isAuthRequireException(reply.getException())) {
            TcrEndpoint* ep = conn->getEndpointObject();
            if (!this->m_isMultiUserMode) {
              ep->setAuthenticated(false);
            } else if (userAttr != nullptr) {
              userAttr->unAuthenticateEP(ep);
            }
            LOGFINEST(
                "After getting AuthenticationRequiredException trying again.");
            isAuthRequireExcepMaxTry--;
            isAuthRequireExcep = true;
            continue;
          } else if (isNotAuthorizedException(reply.getException())) {
            LOGDEBUG("received NotAuthorizedException");
            // TODO should we try again?
          }
        }
      }
      LOGFINER(
          "reply Metadata version is %d & bsl version is %d "
          "reply.isFEAnotherHop()=%d",
          reply.getMetaDataVersion(), version, reply.isFEAnotherHop());
      if (m_clientMetadataService != nullptr && request.forSingleHop() &&
          (reply.getMetaDataVersion() != 0 ||
           (request.getMessageType() == TcrMessage::EXECUTE_REGION_FUNCTION &&
            request.getKeyRef() != nullptr && reply.isFEAnotherHop()))) {
        // Need to get direct access to Region's name to avoid referencing
        // temp data and causing crashes
        std::shared_ptr<Region> region;
        m_connManager.getCacheImpl()->getRegion(request.getRegionName().c_str(),
                                                region);
        if (region != nullptr) {
          if (!connFound)  // max limit case then don't refresh otherwise always
                           // refresh
          {
            LOGFINE("Need to refresh pr-meta-data");
            ThinClientRegion* tcrRegion =
                dynamic_cast<ThinClientRegion*>(region.get());
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
      } else if (error == GF_TIMOUT) {
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
  } else if (error == GF_TIMOUT) {
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

void ThinClientPoolDM::removeEPConnections(int numConn,
                                           bool triggerManageConn) {
  // TODO: Delete EP

  reducePoolSize(numConn);

  // Raise Semaphore for manage thread
  if (triggerManageConn) {
    m_connSema.release();
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
  TcrEndpoint* theEP = getEndPoint(epNameStr);

  LOGFINE(
      "ThinClientPoolDM::getConnectionToAnEndPoint( ): Getting endpoint object "
      "for %s",
      epNameStr.c_str());
  if (theEP != nullptr && theEP->connected()) {
    LOGFINE(
        "ThinClientPoolDM::getConnectionToAnEndPoint( ): Getting connection "
        "for endpoint %s",
        epNameStr.c_str());
    conn = getFromEP(theEP);
    // if connection is null, possibly because there are no idle connections
    // to this endpoint, create a new pool connection to this endpoint.
    bool maxConnLimit = false;
    if (conn == nullptr) {
      LOGFINE(
          "ThinClientPoolDM::getConnectionToAnEndPoint( ): Create connection "
          "for endpoint %s",
          epNameStr.c_str());
      error = createPoolConnectionToAEndPoint(conn, theEP, maxConnLimit);
    }
  }

  // if connection is null, it has failed to get a connection to the specified
  // endpoint. Get a connection to any other server and failover the transaction
  // to that server.
  if (conn == nullptr) {
    std::set<ServerLocation> excludeServers;
    bool maxConnLimit = false;
    LOGFINE(
        "ThinClientPoolDM::getConnectionToAnEndPoint( ): No connection "
        "available for endpoint %s. Create connection to any endpoint.",
        epNameStr.c_str());
    conn = getConnectionFromQueue(true, &error, excludeServers, maxConnLimit);
    if (conn != nullptr && error == GF_NOERR) {
      if (conn->getEndpointObject()->name() != epNameStr) {
        LOGFINE(
            "ThinClientPoolDM::getConnectionToAnEndPoint( ): Endpoint %s "
            "different than the endpoint %s. New connection created and "
            "failing over.",
            epNameStr.c_str(), conn->getEndpointObject()->name().c_str());
        GfErrType failoverErr = doFailover(conn);
        if (failoverErr != GF_NOERR) {
          LOGFINE(
              "ThinClientPoolDM::getConnectionToAnEndPoint( ):Failed to "
              "failover transaction to another server. From endpoint %s to %s",
              epNameStr.c_str(), conn->getEndpointObject()->name().c_str());
          putInQueue(conn, false);
          conn = nullptr;
        }
      }
    }
  }

  if (conn == nullptr || error != GF_NOERR) {
    LOGFINE(
        "ThinClientPoolDM::getConnectionToAEndPoint( ):Failed to connect to %s",
        theEP->name().c_str());
    if (conn != nullptr) _GEODE_SAFE_DELETE(conn);
  }

  return error;
}

// Create a pool connection to specified endpoint. First checks if the number of
// connections has exceeded the maximum allowed. If not, create a connection to
// the specified endpoint. Else, throws an error.
GfErrType ThinClientPoolDM::createPoolConnectionToAEndPoint(
    TcrConnection*& conn, TcrEndpoint* theEP, bool& maxConnLimit,
    bool appThreadrequest) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_queueLock);
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
      LOGFINER(
          "ThinClientPoolDM::createPoolConnectionToAEndPoint( ): current pool "
          "size has reached limit %d, %d",
          m_poolSize.load(), max);
      return error;
    }
  }

  LOGFINE(
      "ThinClientPoolDM::createPoolConnectionToAEndPoint( ): creating a new "
      "connection to the endpoint %s",
      theEP->name().c_str());
  // if the pool size is within limits, create a new connection.
  error = theEP->createNewConnection(conn, false, false,
                                     m_connManager.getCacheImpl()
                                         ->getDistributedSystem()
                                         .getSystemProperties()
                                         .connectTimeout(),
                                     false, true, appThreadrequest);
  if (conn == nullptr || error != GF_NOERR) {
    LOGFINE("2Failed to connect to %s", theEP->name().c_str());
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
  m_connSema.release();

  return error;
}

void ThinClientPoolDM::reducePoolSize(int num) {
  LOGFINE("removing connection %d ,  pool-size =%d", num, m_poolSize.load());
  m_poolSize -= num;
  if (m_poolSize <= 0) {
    if (m_cliCallbackTask != NULL) m_cliCallbackSema.release();
  }
}
GfErrType ThinClientPoolDM::createPoolConnection(
    TcrConnection*& conn, std::set<ServerLocation>& excludeServers,
    bool& maxConnLimit, const TcrConnection* currentserver) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_queueLock);
  GfErrType error = GF_NOERR;
  int max = m_attrs->getMaxConnections();
  if (max == -1) {
    max = 0x7fffffff;
  }
  int min = m_attrs->getMinConnections();
  max = max > min ? max : min;
  LOGDEBUG(
      "ThinClientPoolDM::createPoolConnection( ): current pool size has "
      "reached limit %d, %d, %d",
      m_poolSize.load(), max, min);

  conn = nullptr;
  {
    if (m_poolSize >= max) {
      LOGDEBUG(
          "ThinClientPoolDM::createPoolConnection( ): current pool size has "
          "reached limit %d, %d",
          m_poolSize.load(), max);
      maxConnLimit = true;
      return error;
    }
  }

  bool fatal = false;
  GfErrType fatalError = GF_NOERR;

  while (true) {
    std::string epNameStr;
    try {
      epNameStr = selectEndpoint(excludeServers, currentserver);
    } catch (const NoAvailableLocatorsException&) {
      LOGFINE("Locator query failed");
      return GF_CACHE_LOCATOR_EXCEPTION;
    } catch (const Exception&) {
      LOGFINE("Endpoint selection failed");
      return GF_NOTCON;
    }
    LOGFINE("Connecting to %s", epNameStr.c_str());
    TcrEndpoint* ep = nullptr;
    ep = addEP(epNameStr.c_str());

    if (currentserver != nullptr &&
        epNameStr == currentserver->getEndpointObject()->name()) {
      LOGDEBUG("Updating existing connection: ", epNameStr.c_str());
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
      LOGFINE("1Failed to connect to %s", epNameStr.c_str());
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
        LOGFINE("Connection failed due to fatal client error %d", error);
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
  m_connSema.release();
  // if a fatal error occurred earlier and we don't have
  // a connection then return this saved error
  if (fatal && !conn && error != GF_NOERR) {
    return fatalError;
  }

  return error;
}

TcrConnection* ThinClientPoolDM::getConnectionFromQueue(
    bool timeout, GfErrType* error, std::set<ServerLocation>& excludeServers,
    bool& maxConnLimit) {
  std::chrono::microseconds timeoutTime = m_attrs->getFreeConnectionTimeout();

  getStats().setCurWaitingConnections(waiters());
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
  return mp;
}

bool ThinClientPoolDM::isEndpointAttached(TcrEndpoint* ep) { return true; }

GfErrType ThinClientPoolDM::sendRequestToEP(const TcrMessage& request,
                                            TcrMessageReply& reply,
                                            TcrEndpoint* currentEndpoint) {
  LOGDEBUG("ThinClientPoolDM::sendRequestToEP()");
  int isAuthRequireExcepMaxTry = 2;
  bool isAuthRequireExcep = true;
  GfErrType error = GF_NOERR;
  while (isAuthRequireExcep && isAuthRequireExcepMaxTry >= 0) {
    isAuthRequireExcep = false;
    TcrConnection* conn = getFromEP(currentEndpoint);

    bool isTmpConnectedStatus = false;
    bool putConnInPool = true;
    if (conn == nullptr) {
      LOGDEBUG(
          "ThinClientPoolDM::sendRequestToEP(): got nullptr connection from "
          "pool, "
          "creating new connection in the pool.");
      bool maxConnLimit = false;
      error =
          createPoolConnectionToAEndPoint(conn, currentEndpoint, maxConnLimit);
      if (conn == nullptr || error != GF_NOERR) {
        LOGDEBUG(
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
      LOGFINE("3Failed to connect to %s", currentEndpoint->name().c_str());
      if (conn != nullptr) {
        _GEODE_SAFE_DELETE(conn);
      }
      if (putConnInPool) {
        ACE_Guard<ACE_Recursive_Thread_Mutex> guard(getPoolLock());
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
      reply.setTimeout(this->getReadTimeout());
    }

    reply.setDM(this);
    std::shared_ptr<UserAttributes> ua = nullptr;
    // in multi user mode need to chk whether user is authenticated or not
    // and then follow usual process which we did in send syncrequest.
    // need to user initiative ops
    LOGDEBUG("ThinClientPoolDM::sendRequestToEP: this->m_isMultiUserMode = %d",
             this->m_isMultiUserMode);
    bool isServerException = false;
    if (TcrMessage::isUserInitiativeOps((request)) &&
        (this->m_isSecurityOn || this->m_isMultiUserMode)) {
      if (!this->m_isMultiUserMode && !currentEndpoint->isAuthenticated()) {
        // first authenticate him on this endpoint
        error = this->sendUserCredentials(this->getCredentials(currentEndpoint),
                                          conn, false, isServerException);
      } else if (this->m_isMultiUserMode) {
        ua = TSSUserAttributesWrapper::s_geodeTSSUserAttributes
                 ->getUserAttributes();
        if (ua == nullptr) {
          LOGWARN("Attempted operation type %d without credentials",
                  request.getMessageType());
          if (conn != nullptr)
            putInQueue(conn, false, request.forTransaction());
          return GF_NOT_AUTHORIZED_EXCEPTION;
        } else {
          UserConnectionAttributes* uca =
              ua->getConnectionAttribute(currentEndpoint);

          if (uca == nullptr) {
            error = this->sendUserCredentials(ua->getCredentials(), conn, false,
                                              isServerException);
          }
        }
      }
    }

    LOGDEBUG("ThinClientPoolDM::sendRequestToEP after getting creds");
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
        ACE_Guard<ACE_Recursive_Thread_Mutex> guard(getPoolLock());
        removeEPConnections(1);
      }
    }

    if (error == GF_NOERR || error == GF_CACHESERVER_EXCEPTION ||
        error == GF_AUTHENTICATION_REQUIRED_EXCEPTION) {
      if ((this->m_isSecurityOn || this->m_isMultiUserMode)) {
        if (reply.getMessageType() == TcrMessage::EXCEPTION) {
          if (isAuthRequireException(reply.getException())) {
            if (!this->m_isMultiUserMode) {
              currentEndpoint->setAuthenticated(false);
            } else if (ua != nullptr) {
              ua->unAuthenticateEP(currentEndpoint);
            }
            LOGFINEST(
                "After getting AuthenticationRequiredException trying again.");
            isAuthRequireExcepMaxTry--;
            isAuthRequireExcep = true;
            if (isAuthRequireExcepMaxTry >= 0) error = GF_NOERR;
            continue;
          }
        }
      }
    }
  }
  LOGDEBUG("ThinClientPoolDM::sendRequestToEP Done.");
  return error;
}

TcrEndpoint* ThinClientPoolDM::addEP(ServerLocation& serverLoc) {
  std::string serverName = serverLoc.getServerName();
  int port = serverLoc.getPort();
  char endpointName[100];
  ACE_OS::snprintf(endpointName, 100, "%s:%d", serverName.c_str(), port);

  return addEP(endpointName);
}

TcrEndpoint* ThinClientPoolDM::addEP(const char* endpointName) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_endpointsLock);
  TcrEndpoint* ep = nullptr;

  std::string fullName = endpointName;
  if (m_endpoints.find(fullName, ep)) {
    LOGFINE("Created new endpoint %s for pool %s", fullName.c_str(),
            m_poolName.c_str());
    ep = createEP(fullName.c_str());
    if (m_endpoints.bind(fullName, ep)) {
      LOGERROR("Failed to add endpoint %s to pool %s", fullName.c_str(),
               m_poolName.c_str());
      GF_DEV_ASSERT(
          "ThinClientPoolDM::addEP( ): failed to add endpoint" ? false : false);
    }
  }
  // Update Server Stats
  getStats().setServers(static_cast<int32_t>(m_endpoints.current_size()));
  return ep;
}

void ThinClientPoolDM::netDown() {
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(getPoolLock());
  close();
  reset();
}

void ThinClientPoolDM::pingServerLocal() {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(getPoolLock());
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_endpointsLock);
  for (ACE_Map_Manager<std::string, TcrEndpoint*,
                       ACE_Recursive_Thread_Mutex>::iterator it =
           m_endpoints.begin();
       it != m_endpoints.end(); it++) {
    if ((*it).int_id_->connected()) {
      (*it).int_id_->pingServer(this);
      if (!(*it).int_id_->connected()) {
        removeEPConnections((*it).int_id_);
        removeCallbackConnection((*it).int_id_);
      }
    }
  }
}

int ThinClientPoolDM::updateLocatorList(volatile bool& isRunning) {
  LOGFINE("Starting updateLocatorList thread for pool %s", m_poolName.c_str());
  while (isRunning) {
    m_updateLocatorListSema.acquire();
    if (isRunning && !m_connManager.isNetDown()) {
      ((ThinClientLocatorHelper*)m_locHelper)
          ->updateLocators(this->getServerGroup());
    }
  }
  LOGFINE("Ending updateLocatorList thread for pool %s", m_poolName.c_str());
  return 0;
}

int ThinClientPoolDM::pingServer(volatile bool& isRunning) {
  LOGFINE("Starting ping thread for pool %s", m_poolName.c_str());
  while (isRunning) {
    m_pingSema.acquire();
    if (isRunning && !m_connManager.isNetDown()) {
      pingServerLocal();
      while (m_pingSema.tryacquire() != -1) {
        ;
      }
    }
  }
  LOGFINE("Ending ping thread for pool %s", m_poolName.c_str());
  return 0;
}

int ThinClientPoolDM::cliCallback(volatile bool& isRunning) {
  LOGFINE("Starting cliCallback thread for pool %s", m_poolName.c_str());
  while (isRunning) {
    m_cliCallbackSema.acquire();
    if (isRunning) {
      LOGFINE("Clearing Pdx Type Registry");
      // this call for csharp client
      DistributedSystemImpl::CallCliCallBack(
          *(m_connManager.getCacheImpl()->getCache()));
      // this call for cpp client
      m_connManager.getCacheImpl()->getPdxTypeRegistry()->clear();
      while (m_cliCallbackSema.tryacquire() != -1) {
        ;
      }
    }
  }
  LOGFINE("Ending cliCallback thread for pool %s", m_poolName.c_str());
  return 0;
}

int ThinClientPoolDM::doPing(const ACE_Time_Value&, const void*) {
  m_pingSema.release();
  return 0;
}

int ThinClientPoolDM::doUpdateLocatorList(const ACE_Time_Value&, const void*) {
  m_updateLocatorListSema.release();
  return 0;
}

int ThinClientPoolDM::doManageConnections(const ACE_Time_Value&, const void*) {
  m_connSema.release();
  return 0;
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
    bool queue = false;
    {
      ACE_Guard<ACE_Recursive_Thread_Mutex> poolguard(m_queueLock);  // PXR
      ACE_Guard<ACE_Recursive_Thread_Mutex> guardQueue(
          endPt->getQueueHostedMutex());
      queue = endPt->isQueueHosted();
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
  if (excludeServers.size() == 0 ||
      excludeServers.find(ServerLocation(endpoint)) == excludeServers.end()) {
    return false;
  } else {
    return true;
  }
}

bool ThinClientPoolDM::excludeConnection(
    TcrConnection* conn, std::set<ServerLocation>& excludeServers) {
  return excludeServer(conn->getEndpointObject()->name(), excludeServers);
}

TcrConnection* ThinClientPoolDM::getFromEP(TcrEndpoint* theEP) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_queueLock);
  for (std::deque<TcrConnection*>::iterator itr = m_queue.begin();
       itr != m_queue.end(); itr++) {
    if ((*itr)->getEndpointObject() == theEP) {
      LOGDEBUG("ThinClientPoolDM::getFromEP got connection");
      TcrConnection* retVal = *itr;
      m_queue.erase(itr);
      return retVal;
    }
  }

  return nullptr;
}

void ThinClientPoolDM::removeEPConnections(TcrEndpoint* theEP) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_queueLock);
  int32_t size = static_cast<int32_t>(m_queue.size());
  int numConn = 0;

  while (size--) {
    TcrConnection* curConn = m_queue.back();
    m_queue.pop_back();
    if (curConn->getEndpointObject() != theEP) {
      m_queue.push_front(curConn);
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
    ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_queueLock);

    do {
      returnT = popFromQueue(isClosed);
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

bool ThinClientPoolDM::exclude(TcrConnection* conn,
                               std::set<ServerLocation>& excludeServers) {
  return excludeConnection(conn, excludeServers);
}

void ThinClientPoolDM::incRegionCount() {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_queueLock);

  if (!m_isDestroyed && !m_destroyPending) {
    m_numRegions++;
  } else {
    throw IllegalStateException("Pool has been destroyed.");
  }
}

void ThinClientPoolDM::decRegionCount() {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_queueLock);

  m_numRegions--;
}

void ThinClientPoolDM::checkRegions() {
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_queueLock);

  if (m_numRegions > 0) {
    throw IllegalStateException(
        "Failed to destroy pool because regions are connected with it.");
  }

  m_destroyPending = true;
}
void ThinClientPoolDM::updateNotificationStats(bool isDeltaSuccess,
                                               long timeInNanoSecond) {
  if (isDeltaSuccess) {
    getStats().incProcessedDeltaMessages();
    getStats().incProcessedDeltaMessagesTime(timeInNanoSecond);
  } else {
    getStats().incDeltaMessageFailures();
  }
}

GfErrType ThinClientPoolDM::doFailover(TcrConnection* conn) {
  m_manager->setStickyConnection(conn, true);
  TcrMessageTxFailover request(
      m_connManager.getCacheImpl()->getCache()->createDataOutput());
  TcrMessageReply reply(true, nullptr);

  GfErrType err = this->sendSyncRequest(request, reply);

  if (err == GF_NOERR) {
    switch (reply.getMessageType()) {
      case TcrMessage::REPLY: {
        break;
      }
      case TcrMessage::EXCEPTION: {
        const char* exceptionMsg = reply.getException();
        err = ThinClientRegion::handleServerException(
            "CacheTransactionManager::failover", exceptionMsg);
        break;
      }
      case TcrMessage::REQUEST_DATA_ERROR: {
        err = GF_CACHESERVER_EXCEPTION;
        break;
      }
      default: {
        LOGERROR("Unknown message type in failover reply %d",
                 reply.getMessageType());
        err = GF_MSG;
        break;
      }
    }
  }

  return err;
}
