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

#include "CacheImpl.hpp"

#include <string>

#include <geode/CacheStatistics.hpp>
#include <geode/PersistenceManager.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionAttributes.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/SystemProperties.hpp>

#include "AutoDelete.hpp"
#include "CacheXmlParser.hpp"
#include "ClientProxyMembershipID.hpp"
#include "ExpiryTaskManager.hpp"
#include "InternalCacheTransactionManager2PCImpl.hpp"
#include "LocalRegion.hpp"
#include "PdxTypeRegistry.hpp"
#include "RegionExpiryHandler.hpp"
#include "SerializationRegistry.hpp"
#include "TcrMessage.hpp"
#include "ThinClientHARegion.hpp"
#include "ThinClientPoolDM.hpp"
#include "ThinClientPoolHADM.hpp"
#include "ThinClientPoolRegion.hpp"
#include "ThinClientRegion.hpp"
#include "ThreadPool.hpp"
#include "Utils.hpp"
#include "Version.hpp"

#define DEFAULT_DS_NAME "default_GeodeDS"

namespace apache {
namespace geode {
namespace client {

CacheImpl::CacheImpl(Cache* c, const std::shared_ptr<Properties>& dsProps,
                     bool ignorePdxUnreadFields, bool readPdxSerialized,
                     const std::shared_ptr<AuthInitialize>& authInitialize)
    : m_ignorePdxUnreadFields(ignorePdxUnreadFields),
      m_readPdxSerialized(readPdxSerialized),
      m_expiryTaskManager(
          std::unique_ptr<ExpiryTaskManager>(new ExpiryTaskManager())),
      m_statisticsManager(nullptr),
      m_closed(false),
      m_initialized(false),
      m_distributedSystem(DistributedSystem::create(DEFAULT_DS_NAME, dsProps)),
      m_clientProxyMembershipIDFactory(m_distributedSystem.getName()),
      m_cache(c),
      m_attributes(nullptr),
      m_evictionControllerPtr(nullptr),
      m_tcrConnectionManager(nullptr),
      m_remoteQueryServicePtr(nullptr),
      m_destroyPending(false),
      m_initDone(false),
      m_adminRegion(nullptr),
      m_memberListForVersionStamp(
          *(std::make_shared<MemberListForVersionStamp>())),
      m_serializationRegistry(std::make_shared<SerializationRegistry>()),
      m_pdxTypeRegistry(nullptr),
      m_threadPool(new ThreadPool(
          m_distributedSystem.getSystemProperties().threadPoolSize())),
      m_authInitialize(authInitialize) {
  using apache::geode::statistics::StatisticsManager;

  m_cacheTXManager = std::shared_ptr<InternalCacheTransactionManager2PC>(
      new InternalCacheTransactionManager2PCImpl(this));

  m_regions = new MapOfRegionWithLock();
  auto& prop = m_distributedSystem.getSystemProperties();
  if (prop.heapLRULimitEnabled()) {
    m_evictionControllerPtr =
        new EvictionController(prop.heapLRULimit(), prop.heapLRUDelta(), this);
    m_evictionControllerPtr->start();
    LOGINFO("Heap LRU eviction controller thread started");
  }

  m_expiryTaskManager->begin();

  m_initialized = true;
  m_pdxTypeRegistry = std::make_shared<PdxTypeRegistry>(this);
  m_poolManager = std::unique_ptr<PoolManager>(new PoolManager(this));
  m_typeRegistry = std::unique_ptr<TypeRegistry>(new TypeRegistry(this));

  try {
    m_statisticsManager =
        std::unique_ptr<StatisticsManager>(new StatisticsManager(
            prop.statisticsArchiveFile().c_str(),
            prop.statisticsSampleInterval(), prop.statisticsEnabled(), this,
            prop.statsFileSizeLimit(), prop.statsDiskSpaceLimit()));
    m_cacheStats =
        new CachePerfStats(m_statisticsManager->getStatisticsFactory());
  } catch (const NullPointerException&) {
    Log::close();
    throw;
  }

  m_distributedSystem.connect();
}

void CacheImpl::initServices() {
  m_tcrConnectionManager = new TcrConnectionManager(this);
  if (!m_initDone && m_attributes != nullptr &&
      !m_attributes->getEndpoints().empty()) {
    if (getPoolManager().getAll().size() > 0 && getCacheMode()) {
      LOGWARN(
          "At least one pool has been created so ignoring cache level "
          "redundancy setting");
    }
    m_tcrConnectionManager->init();
    m_remoteQueryServicePtr = std::make_shared<RemoteQueryService>(this);
    // StartAdminRegion
    auto& prop = m_distributedSystem.getSystemProperties();
    if (prop.statisticsEnabled()) {
      m_adminRegion = AdminRegion::create(this);
    }
    m_initDone = true;
  }
}

void CacheImpl::netDown() {
  m_tcrConnectionManager->netDown();

  for (const auto& itr : getPoolManager().getAll()) {
    auto currPool = itr.second;
    if (auto poolHADM =
            std::dynamic_pointer_cast<ThinClientPoolHADM>(currPool)) {
      poolHADM->netDown();
    }
  }
}

void CacheImpl::revive() { m_tcrConnectionManager->revive(); }

CacheImpl::RegionKind CacheImpl::getRegionKind(
    RegionAttributes regionAttributes) const {
  RegionKind regionKind = CPP_REGION;
  std::string endpoints;

  if (m_attributes != nullptr &&
      !(endpoints = m_attributes->getEndpoints()).empty() &&
      (m_attributes->getRedundancyLevel() > 0 ||
       m_tcrConnectionManager->isDurable())) {
    regionKind = THINCLIENT_HA_REGION;
  } else if (!endpoints.empty() && regionAttributes.getEndpoints().empty()) {
    regionAttributes.setEndpoints(endpoints);
  }

  if (!(endpoints = regionAttributes.getEndpoints()).empty()) {
    if ("none" == endpoints) {
      regionKind = CPP_REGION;
    } else if (regionKind != THINCLIENT_HA_REGION) {
      regionKind = THINCLIENT_REGION;
    }
  } else if (!regionAttributes.getPoolName().empty()) {
    auto pPtr = getPoolManager().find(regionAttributes.getPoolName());
    if ((pPtr != nullptr && (pPtr->getSubscriptionRedundancy() > 0 ||
                             pPtr->getSubscriptionEnabled())) ||
        m_tcrConnectionManager->isDurable()) {
      regionKind = THINCLIENT_HA_REGION;  // As of now ThinClinetHARegion deals
      // with Pool as well.
    } else {
      regionKind = THINCLIENT_POOL_REGION;
    }
  }

  return regionKind;
}

int CacheImpl::removeRegion(const char* name) {
  TryReadGuard guardCacheDestroy(m_destroyCacheMutex, m_destroyPending);
  if (m_destroyPending) {
    return 0;
  }

  MapOfRegionGuard guard(m_regions->mutex());
  return m_regions->unbind(name);
}

std::shared_ptr<QueryService> CacheImpl::getQueryService(bool noInit) {
  if (auto&& defaultPool = getPoolManager().getDefaultPool()) {
    if (defaultPool->isDestroyed()) {
      throw IllegalStateException("Pool has been destroyed.");
    }
    return defaultPool->getQueryService();
  }

  if (m_remoteQueryServicePtr == nullptr) {
    m_tcrConnectionManager->init();
    m_remoteQueryServicePtr = std::make_shared<RemoteQueryService>(this);
  }
  if (!noInit) {
    m_remoteQueryServicePtr->init();
  }
  return m_remoteQueryServicePtr;
}

std::shared_ptr<QueryService> CacheImpl::getQueryService(const char* poolName) {
  if (poolName == nullptr || strlen(poolName) == 0) {
    throw IllegalArgumentException("PoolName is nullptr or not defined..");
  }
  auto pool = getPoolManager().find(poolName);

  if (pool != nullptr) {
    if (pool->isDestroyed()) {
      throw IllegalStateException("Pool has been destroyed.");
    }
    return pool->getQueryService();
  } else {
    throw IllegalArgumentException("Pool not found..");
  }
}

CacheImpl::~CacheImpl() {
  if (!m_closed) {
    close();
  }

  if (m_regions != nullptr) {
    delete m_regions;
  }
}

const std::string& CacheImpl::getName() const {
  this->throwIfClosed();

  return m_distributedSystem.getName();
}

bool CacheImpl::isClosed() const { return m_closed; }

void CacheImpl::setAttributes(
    const std::shared_ptr<CacheAttributes>& attributes) {
  if (m_attributes == nullptr && attributes != nullptr) {
    m_attributes = attributes;
  }
}

DistributedSystem& CacheImpl::getDistributedSystem() {
  return m_distributedSystem;
}

TypeRegistry& CacheImpl::getTypeRegistry() {
  this->throwIfClosed();
  return *m_typeRegistry;
}

void CacheImpl::sendNotificationCloseMsgs() {
  for (const auto& iter : getPoolManager().getAll()) {
    if (const auto& pool =
            std::dynamic_pointer_cast<ThinClientPoolHADM>(iter.second)) {
      pool->sendNotificationCloseMsgs();
    }
  }
}

void CacheImpl::close(bool keepalive) {
  this->throwIfClosed();

  TcrMessage::setKeepAlive(keepalive);
  // bug #247 fix for durable clients missing events when recycled
  sendNotificationCloseMsgs();
  {
    TryWriteGuard guardCacheDestroy(m_destroyCacheMutex, m_destroyPending);
    if (m_destroyPending) {
      return;
    }
    m_destroyPending = true;
  }

  if (m_closed || (!m_initialized)) return;

  // Close the distribution manager used for queries.
  if (m_remoteQueryServicePtr != nullptr) {
    m_remoteQueryServicePtr->close();
    m_remoteQueryServicePtr = nullptr;
  }

  // Close AdminRegion
  if (m_adminRegion != nullptr) {
    m_adminRegion->close();
    m_adminRegion = nullptr;
  }

  // The TCCM gets destroyed when CacheImpl is destroyed, but after that there
  // is still a window for the ping related registered task to get activated
  // because expiryTaskManager is closed in DS::disconnect. If this happens
  // then the handler will work on an already destroyed object which would
  // lead to a SEGV. So cancelling the task in TcrConnectionManager::close().
  if (m_tcrConnectionManager != nullptr) {
    m_tcrConnectionManager->close();
  }

  MapOfRegionWithLock regions;
  getSubRegions(regions);

  for (MapOfRegionWithLock::iterator q = regions.begin(); q != regions.end();
       ++q) {
    // TODO: remove dynamic_cast here by having RegionInternal in the regions
    // map
    auto rImpl = std::dynamic_pointer_cast<RegionInternal>((*q).int_id_);
    if (rImpl != nullptr) {
      rImpl->destroyRegionNoThrow(
          nullptr, false,
          CacheEventFlags::LOCAL | CacheEventFlags::CACHE_CLOSE);
    }
  }

  if (m_evictionControllerPtr != nullptr) {
    m_evictionControllerPtr->stop();
    _GEODE_SAFE_DELETE(m_evictionControllerPtr);
  }

  // Close CachePef Stats
  if (m_cacheStats) {
    m_cacheStats->close();
  }

  m_poolManager->close(keepalive);

  LOGFINE("Closed pool manager with keepalive %s",
          keepalive ? "true" : "false");

  // Close CachePef Stats
  if (m_cacheStats) {
    _GEODE_SAFE_DELETE(m_cacheStats);
  }

  m_regions->unbind_all();
  LOGDEBUG("CacheImpl::close( ): destroyed regions.");

  _GEODE_SAFE_DELETE(m_tcrConnectionManager);
  m_cacheTXManager = nullptr;

  m_expiryTaskManager->stopExpiryTaskManager();

  try {
    getDistributedSystem().disconnect();
  } catch (const apache::geode::client::NotConnectedException&) {
  } catch (const apache::geode::client::Exception&) {
  } catch (...) {
  }

  m_closed = true;

  LOGFINE("Cache closed.");
}

bool CacheImpl::isCacheDestroyPending() const { return m_destroyPending; }

void CacheImpl::validateRegionAttributes(
    const std::string& name, const RegionAttributes regionAttributes) const {
  auto&& kind = getRegionKind(regionAttributes);
  if (regionAttributes.m_clientNotificationEnabled && kind == CPP_REGION) {
    throw UnsupportedOperationException(
        "Cache::createRegion: \"" + name +
        "\" Client notification can be enabled only for native client region");
  }
}

// We'll pass a nullptr loader function pointer and let the region.get method to
// do a load using a real C++ loader, instead of passing a member function
// pointer here
void CacheImpl::createRegion(std::string name,
                             RegionAttributes regionAttributes,
                             std::shared_ptr<Region>& regionPtr) {
  {
    std::lock_guard<decltype(m_initDoneLock)> _guard(m_initDoneLock);
    if (!m_initDone) {
      if (regionAttributes.getPoolName().empty()) {
        m_tcrConnectionManager->init();
        m_remoteQueryServicePtr = std::make_shared<RemoteQueryService>(this);
        auto& prop = m_distributedSystem.getSystemProperties();
        if (prop.statisticsEnabled()) {
          m_adminRegion = AdminRegion::create(this);
        }
      }
      m_initDone = true;
    }
  }

  this->throwIfClosed();

  if (name.find('/') != std::string::npos) {
    throw IllegalArgumentException(
        "Malformed name string, contains region path seperator '/'");
  }

  validateRegionAttributes(name, regionAttributes);
  std::shared_ptr<RegionInternal> rpImpl = nullptr;
  {
    // For multi threading and the operations between bind and find seems to be
    // hard to be atomic since a regionImpl needs to be valid before it can be
    // bound
    MapOfRegionGuard guard1(m_regions->mutex());
    std::shared_ptr<Region> tmp;
    if (0 == m_regions->find(name, tmp)) {
      throw RegionExistsException("Cache::createRegion: \"" + name +
                                  "\" region exists in local cache");
    }

    auto cacheStatistics = std::make_shared<CacheStatistics>();
    try {
      rpImpl = createRegion_internal(name, nullptr, regionAttributes,
                                     cacheStatistics, false);
    } catch (const AuthenticationFailedException&) {
      throw;
    } catch (const AuthenticationRequiredException&) {
      throw;
    } catch (const Exception&) {
      throw;
    } catch (std::exception& ex) {
      throw UnknownException("Cache::createRegion: Failed to create Region \"" +
                             name +
                             "\" due to unknown exception: " + ex.what());
    } catch (...) {
      throw UnknownException("Cache::createRegion: Failed to create Region \"" +
                             name + "\" due to unknown exception.");
    }

    if (rpImpl == nullptr) {
      throw RegionCreationFailedException(
          "Cache::createRegion: Failed to create Region \"" + name + "\"");
    }

    regionPtr = rpImpl;
    rpImpl->addDisMessToQueue();
    // Instantiate a PersistenceManager object if DiskPolicy is overflow
    if (regionAttributes.getDiskPolicy() == DiskPolicyType::OVERFLOWS) {
      auto pmPtr = regionAttributes.getPersistenceManager();
      if (pmPtr == nullptr) {
        throw NullPointerException(
            "PersistenceManager could not be instantiated");
      }
      auto props = regionAttributes.getPersistenceProperties();
      pmPtr->init(regionPtr, props);
      rpImpl->setPersistenceManager(pmPtr);
    }

    rpImpl->acquireReadLock();
    m_regions->bind(regionPtr->getName(), regionPtr);

    // When region is created, added that region name in client meta data
    // service to fetch its
    // metadata for single hop.
    auto& props = m_distributedSystem.getSystemProperties();
    if (!props.isGridClient()) {
      const auto& poolName = regionAttributes.getPoolName();
      if (!poolName.empty()) {
        auto pool = getPoolManager().find(poolName);
        if (pool != nullptr && !pool->isDestroyed() &&
            pool->getPRSingleHopEnabled()) {
          ThinClientPoolDM* poolDM =
              dynamic_cast<ThinClientPoolDM*>(pool.get());
          if ((poolDM != nullptr) &&
              (poolDM->getClientMetaDataService() != nullptr)) {
            LOGFINE("enqueued region " + name +
                    " for initial metadata refresh for singlehop ");
            poolDM->getClientMetaDataService()->enqueueForMetadataRefresh(
                regionPtr->getFullPath(), 0);
          }
        }
      }
    }
  }

  // schedule the root region expiry if regionExpiry enabled.
  rpImpl->setRegionExpiryTask();
  rpImpl->releaseReadLock();
}

std::shared_ptr<Region> CacheImpl::getRegion(const std::string& path) {
  LOGDEBUG("Cache::getRegion " + path);

  this->throwIfClosed();

  TryReadGuard guardCacheDestroy(m_destroyCacheMutex, m_destroyPending);

  if (m_destroyPending) {
    return nullptr;
  }

  MapOfRegionGuard guard(m_regions->mutex());
  static const std::string slash("/");
  if (path == slash || path.length() < 1) {
    LOGERROR("Cache::getRegion: path [" + path + "] is not valid.");
    throw IllegalArgumentException("Cache::getRegion: path is empty or a /");
  }
  auto fullname = path;
  if (fullname.substr(0, 1) == slash) {
    fullname = path.substr(1);
  }

  // find second separator
  auto idx = static_cast<uint32_t>(fullname.find('/'));
  auto stepname = fullname.substr(0, idx);

  std::shared_ptr<Region> region = nullptr;

  if (0 == m_regions->find(stepname, region)) {
    if (stepname != fullname) {
      auto remainder = fullname.substr(stepname.length() + 1);

      if (region != nullptr) {
        region = region->getSubregion(remainder.c_str());
      }
    }
  }

  if (region != nullptr && isPoolInMultiuserMode(region)) {
    LOGWARN("Pool " + region->getAttributes().getPoolName() +
            " attached with region " + region->getFullPath() +
            " is in multiuser authentication mode. Operations may fail as "
            "this instance does not have any credentials.");
  }

  return region;
}

std::shared_ptr<RegionInternal> CacheImpl::createRegion_internal(
    const std::string& name, const std::shared_ptr<RegionInternal>& rootRegion,
    const RegionAttributes& attrs,
    const std::shared_ptr<CacheStatistics>& csptr, bool shared) {
  std::shared_ptr<RegionInternal> rptr = nullptr;
  RegionKind regionKind = getRegionKind(attrs);
  const auto& poolName = attrs.getPoolName();
  const auto& regionEndpoints = attrs.getEndpoints();
  const std::string cacheEndpoints =
      m_attributes ? m_attributes->getEndpoints() : "";

  if (!poolName.empty()) {
    auto pool = getPoolManager().find(poolName);
    if (pool != nullptr && !pool->isDestroyed()) {
      bool isMultiUserSecureMode = pool->getMultiuserAuthentication();
      if (isMultiUserSecureMode && (attrs.getCachingEnabled())) {
        LOGERROR(
            "Pool [%s] is in multiuser authentication mode so region local "
            "caching is not supported.",
            poolName.c_str());
        throw IllegalStateException(
            "Pool is in multiuser authentication so region local caching is "
            "not supported.");
      }
    }
  }

  if (!poolName.empty() &&
      (!regionEndpoints.empty() || !cacheEndpoints.empty())) {
    LOGERROR(
        "Cache or region endpoints cannot be specified when pool name is "
        "specified for region %s",
        name.c_str());
    throw IllegalArgumentException(
        "Cache or region endpoints cannot be specified when pool name is "
        "specified");
  }

  if (regionKind == THINCLIENT_REGION) {
    LOGINFO("Creating region " + name + " with region endpoints " +
            attrs.getEndpoints().c_str());
    auto tmp = std::make_shared<ThinClientRegion>(name, this, rootRegion, attrs,
                                                  csptr, shared);
    tmp->initTCR();
    rptr = tmp;
  } else if (regionKind == THINCLIENT_HA_REGION) {
    LOGINFO("Creating region " + name + " with subscriptions enabled");
    auto tmp = std::make_shared<ThinClientHARegion>(name, this, rootRegion,
                                                    attrs, csptr, shared);
    tmp->initTCR();
    rptr = tmp;
  } else if (regionKind == THINCLIENT_POOL_REGION) {
    LOGINFO("Creating region " + name + " attached to pool " +
            attrs.getPoolName());
    auto tmp = std::make_shared<ThinClientPoolRegion>(name, this, rootRegion,
                                                      attrs, csptr, shared);
    tmp->initTCR();
    rptr = tmp;
  } else {
    LOGINFO("Creating local region " + name);
    rptr = std::make_shared<LocalRegion>(name, this, rootRegion, attrs, csptr,
                                         shared);
  }
  return rptr;
}

std::vector<std::shared_ptr<Region>> CacheImpl::rootRegions() {
  this->throwIfClosed();

  std::vector<std::shared_ptr<Region>> regions;

  MapOfRegionGuard guard(m_regions->mutex());

  if (m_regions->current_size() != 0) {
    regions.reserve(static_cast<int32_t>(m_regions->current_size()));

    for (MapOfRegionWithLock::iterator q = m_regions->begin();
         q != m_regions->end(); ++q) {
      if (!(*q).int_id_->isDestroyed()) {
        regions.push_back((*q).int_id_);
      }
    }
  }

  return regions;
}

void CacheImpl::initializeDeclarativeCache(const std::string& cacheXml) {
  this->throwIfClosed();

  auto xmlParser = std::unique_ptr<CacheXmlParser>(
      CacheXmlParser::parse(cacheXml.c_str(), m_cache));
  xmlParser->setAttributes(m_cache);
  initServices();
  xmlParser->create(m_cache);
}

EvictionController* CacheImpl::getEvictionController() {
  return m_evictionControllerPtr;
}

void CacheImpl::readyForEvents() {
  this->throwIfClosed();

  bool autoReadyForEvents =
      m_distributedSystem.getSystemProperties().autoReadyForEvents();
  bool isDurable = m_tcrConnectionManager->isDurable();

  if (!isDurable && autoReadyForEvents) {
    LOGERROR(
        "Only durable clients or clients with the "
        "auto-ready-for-events property set to false should call "
        "readyForEvents()");
    throw IllegalStateException(
        "Only durable clients or clients with the "
        "auto-ready-for-events property set to false should call "
        "readyForEvents()");
  }

  // Send the CLIENT_READY message to the server
  if (m_tcrConnectionManager->getNumEndPoints() > 0 && isDurable) {
    m_tcrConnectionManager->readyForEvents();
    return;
  }

  const auto& pools = getPoolManager().getAll();
  if (pools.empty()) throw IllegalStateException("No pools found.");
  for (const auto& itr : pools) {
    const auto& currPool = itr.second;
    LOGDEBUG("Sending readyForEvents( ) with pool " + currPool->getName());
    try {
      if (const auto& poolHADM =
              std::dynamic_pointer_cast<ThinClientPoolHADM>(currPool)) {
        poolHADM->readyForEvents();
      }
    } catch (Exception& ex) {
      LOGWARN("readyForEvents( ) failed for pool " + currPool->getName() +
              " with exception: " + ex.getMessage());
    }
  }
}

bool CacheImpl::isPoolInMultiuserMode(std::shared_ptr<Region> regionPtr) {
  const auto& poolName = regionPtr->getAttributes().getPoolName();

  if (!poolName.empty()) {
    auto poolPtr = regionPtr->getCache().getPoolManager().find(poolName);
    if (poolPtr != nullptr && !poolPtr->isDestroyed()) {
      return poolPtr->getMultiuserAuthentication();
    }
  }

  return false;
}

bool CacheImpl::getEndpointStatus(const std::string& endpoint) {
  const auto& pools = getPoolManager().getAll();
  std::string fullName = endpoint;

  if (pools.empty()) {
    return m_tcrConnectionManager->getEndpointStatus(fullName);
  }

  fullName = endpoint.find(':') == std::string::npos
                 ? endpoint
                 : Utils::convertHostToCanonicalForm(endpoint.c_str());

  const auto firstPool =
      std::static_pointer_cast<ThinClientPoolDM>(pools.begin()->second);

  auto& mutex = firstPool->m_endpointsLock;
  std::lock_guard<decltype(mutex)> guard(mutex);
  for (const auto& itr : firstPool->m_endpoints) {
    const auto& ep = itr.int_id_;
    if (ep->name().find(fullName) != std::string::npos) {
      return ep->getServerQueueStatusTEST();
    }
  }
  return false;
}

void CacheImpl::processMarker() {
  TryReadGuard guardCacheDestroy(m_destroyCacheMutex, m_destroyPending);
  if (m_destroyPending) {
    return;
  }

  MapOfRegionGuard guard(m_regions->mutex());

  for (const auto& q : *m_regions) {
    if (!q.int_id_->isDestroyed()) {
      if (const auto tcrHARegion =
              std::dynamic_pointer_cast<ThinClientHARegion>(q.int_id_)) {
        auto regionMsg = new TcrMessageClientMarker(
            new DataOutput(createDataOutput()), true);
        tcrHARegion->receiveNotification(regionMsg);
        for (const auto& iter : tcrHARegion->subregions(true)) {
          if (!iter->isDestroyed()) {
            if (const auto subregion =
                    std::dynamic_pointer_cast<ThinClientHARegion>(iter)) {
              regionMsg = new TcrMessageClientMarker(
                  new DataOutput(createDataOutput()), true);
              subregion->receiveNotification(regionMsg);
            }
          }
        }
      }
    }
  }
}

int CacheImpl::getPoolSize(const char* poolName) {
  if (const auto pool = getPoolManager().find(poolName)) {
    if (const auto dm = std::dynamic_pointer_cast<ThinClientPoolDM>(pool)) {
      return dm->m_poolSize;
    }
  }
  return -1;
}

RegionFactory CacheImpl::createRegionFactory(RegionShortcut preDefinedRegion) {
  this->throwIfClosed();

  return RegionFactory(preDefinedRegion, this);
}

std::map<std::string, RegionAttributes> CacheImpl::getRegionShortcut() {
  std::map<std::string, RegionAttributes> preDefined;

  // PROXY
  auto regAttr_PROXY = RegionAttributes();
  regAttr_PROXY.setCachingEnabled(false);
  preDefined["PROXY"] = regAttr_PROXY;

  // CACHING_PROXY
  auto regAttr_CACHING_PROXY = RegionAttributes();
  regAttr_CACHING_PROXY.setCachingEnabled(true);
  preDefined["CACHING_PROXY"] = regAttr_CACHING_PROXY;

  // CACHING_PROXY_ENTRY_LRU
  auto regAttr_CACHING_PROXY_LRU = RegionAttributes();
  regAttr_CACHING_PROXY_LRU.setCachingEnabled(true);
  regAttr_CACHING_PROXY_LRU.setLruEntriesLimit(DEFAULT_LRU_MAXIMUM_ENTRIES);
  preDefined["CACHING_PROXY_ENTRY_LRU"] = regAttr_CACHING_PROXY_LRU;

  // LOCAL
  auto regAttr_LOCAL = RegionAttributes();
  preDefined["LOCAL"] = regAttr_LOCAL;

  // LOCAL_ENTRY_LRU
  auto regAttr_LOCAL_LRU = RegionAttributes();
  regAttr_LOCAL_LRU.setLruEntriesLimit(DEFAULT_LRU_MAXIMUM_ENTRIES);
  preDefined["LOCAL_ENTRY_LRU"] = regAttr_LOCAL_LRU;

  return preDefined;
}

std::shared_ptr<PdxTypeRegistry> CacheImpl::getPdxTypeRegistry() const {
  return m_pdxTypeRegistry;
}

std::shared_ptr<SerializationRegistry> CacheImpl::getSerializationRegistry()
    const {
  return m_serializationRegistry;
}

ThreadPool* CacheImpl::getThreadPool() { return m_threadPool; }
std::shared_ptr<CacheTransactionManager>
CacheImpl::getCacheTransactionManager() {
  this->throwIfClosed();

  return m_cacheTXManager;
}

std::shared_ptr<MemberListForVersionStamp>
CacheImpl::getMemberListForVersionStamp() {
  static auto versionStampMemIdList =
      new std::shared_ptr<MemberListForVersionStamp>(
          new MemberListForVersionStamp());
  return *versionStampMemIdList;
}

DataOutput CacheImpl::createDataOutput() const {
  this->throwIfClosed();

  return CacheImpl::createDataOutput(nullptr);
}

DataOutput CacheImpl::createDataOutput(Pool* pool) const {
  this->throwIfClosed();

  if (!pool) {
    pool = this->getPoolManager().getDefaultPool().get();
  }

  return DataOutput(this, pool);
}

DataInput CacheImpl::createDataInput(const uint8_t* buffer, size_t len) const {
  this->throwIfClosed();

  return CacheImpl::createDataInput(buffer, len, nullptr);
}

DataInput CacheImpl::createDataInput(const uint8_t* buffer, size_t len,
                                     Pool* pool) const {
  if (!pool) {
    pool = this->getPoolManager().getDefaultPool().get();
  }
  return DataInput(buffer, len, this, pool);
}

PdxInstanceFactory CacheImpl::createPdxInstanceFactory(
    const std::string& className) const {
  this->throwIfClosed();

  return PdxInstanceFactory(
      className, *m_cacheStats, *m_pdxTypeRegistry, *this,
      m_distributedSystem.getSystemProperties().getEnableTimeStatistics());
}

AuthenticatedView CacheImpl::createAuthenticatedView(
    std::shared_ptr<Properties> userSecurityProperties,
    const std::string& poolName) {
  this->throwIfClosed();

  if (poolName.empty()) {
    auto pool = m_poolManager->getDefaultPool();
    if (!this->isClosed() && pool != nullptr) {
      return pool->createAuthenticatedView(userSecurityProperties, this);
    }

    throw IllegalStateException(
        "Either cache has been closed or there are more than two pool."
        "Pass poolname to get the secure Cache");
  } else {
    if (!this->isClosed()) {
      if (!poolName.empty()) {
        auto poolPtr = m_poolManager->find(poolName);
        if (poolPtr != nullptr && !poolPtr->isDestroyed()) {
          return poolPtr->createAuthenticatedView(userSecurityProperties, this);
        }
        throw IllegalStateException(
            "Either pool not found or it has been destroyed");
      }
      throw IllegalArgumentException("poolname is nullptr");
    }

    throw IllegalStateException("Cache has been closed");
  }
}

void CacheImpl::setCache(Cache* cache) { m_cache = cache; }
}  // namespace client
}  // namespace geode
}  // namespace apache
