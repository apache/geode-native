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
#include <string>
#include <string>

#include <ace/OS.h>
#include <geode/CacheStatistics.hpp>
#include <geode/PoolManager.hpp>
#include <geode/SystemProperties.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionAttributes.hpp>
#include <geode/PersistenceManager.hpp>

#include "CacheImpl.hpp"
#include "Utils.hpp"
#include "LocalRegion.hpp"
#include "ExpiryTaskManager.hpp"
#include "RegionExpiryHandler.hpp"
#include "TcrMessage.hpp"
#include "ThinClientRegion.hpp"
#include "ThinClientHARegion.hpp"
#include "ThinClientPoolRegion.hpp"
#include "ThinClientPoolDM.hpp"
#include "Version.hpp"
#include "ClientProxyMembershipID.hpp"
#include "AutoDelete.hpp"
#include "ThinClientPoolHADM.hpp"
#include "InternalCacheTransactionManager2PCImpl.hpp"
#include "PdxTypeRegistry.hpp"
#include "SerializationRegistry.hpp"
#include "ThreadPool.hpp"

using namespace apache::geode::client;

CacheImpl::CacheImpl(Cache* c, const std::string& name,
                     std::unique_ptr<DistributedSystem> sys, bool iPUF,
                     bool readPdxSerialized,
                     const AuthInitializePtr& authInitialize)
    : m_name(name),
      m_defaultPool(nullptr),
      m_ignorePdxUnreadFields(iPUF),
      m_readPdxSerialized(readPdxSerialized),
      m_closed(false),
      m_initialized(false),
      m_distributedSystem(std::move(sys)),
      m_implementee(c),
      m_cond(m_mutex),
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
      m_pdxTypeRegistry(std::make_shared<PdxTypeRegistry>(c)),
      m_expiryTaskManager(
          std::unique_ptr<ExpiryTaskManager>(new ExpiryTaskManager())),
      m_clientProxyMembershipIDFactory(m_distributedSystem->getName()),
      m_threadPool(new ThreadPool(
          m_distributedSystem->getSystemProperties().threadPoolSize())),
      m_authInitialize(authInitialize) {
  m_cacheTXManager = InternalCacheTransactionManager2PCPtr(
      new InternalCacheTransactionManager2PCImpl(c));

  m_regions = new MapOfRegionWithLock();
  auto& prop = m_distributedSystem->getSystemProperties();
  if (prop.heapLRULimitEnabled()) {
    m_evictionControllerPtr =
        new EvictionController(prop.heapLRULimit(), prop.heapLRUDelta(), this);
    m_evictionControllerPtr->start();
    LOGINFO("Heap LRU eviction controller thread started");
  }

  m_cacheStats = new CachePerfStats(m_distributedSystem.get()
                                        ->getStatisticsManager()
                                        ->getStatisticsFactory());
  m_expiryTaskManager->begin();

  m_initialized = true;

  m_poolManager = std::unique_ptr<PoolManager>(new PoolManager(*m_implementee));
}

void CacheImpl::initServices() {
  m_tcrConnectionManager = new TcrConnectionManager(this);
  if (!m_initDone && m_attributes != nullptr && m_attributes->getEndpoints()) {
    if (getCache()->getPoolManager().getAll().size() > 0 && getCacheMode()) {
      LOGWARN(
          "At least one pool has been created so ignoring cache level "
          "redundancy setting");
    }
    m_tcrConnectionManager->init();
    m_remoteQueryServicePtr = std::make_shared<RemoteQueryService>(this);
    // StartAdminRegion
    auto& prop = m_distributedSystem->getSystemProperties();
    if (prop.statisticsEnabled()) {
      m_adminRegion = AdminRegion::create(this);
    }
    m_initDone = true;
  }
}

void CacheImpl::netDown() {
  m_tcrConnectionManager->netDown();

  for (const auto& itr : getCache()->getPoolManager().getAll()) {
    auto currPool = itr.second;
    if (auto poolHADM =
            std::dynamic_pointer_cast<ThinClientPoolHADM>(currPool)) {
      poolHADM->netDown();
    }
  }
}

void CacheImpl::revive() { m_tcrConnectionManager->revive(); }

CacheImpl::RegionKind CacheImpl::getRegionKind(
    const RegionAttributesPtr& rattrs) const {
  RegionKind regionKind = CPP_REGION;
  const char* endpoints = nullptr;

  if (m_attributes != nullptr &&
      (endpoints = m_attributes->getEndpoints()) != nullptr &&
      (m_attributes->getRedundancyLevel() > 0 ||
       m_tcrConnectionManager->isDurable())) {
    regionKind = THINCLIENT_HA_REGION;
  } else if (endpoints != nullptr && rattrs->getEndpoints() == nullptr) {
    rattrs->setEndpoints(endpoints);
  }

  if ((endpoints = rattrs->getEndpoints()) != nullptr) {
    if (strcmp(endpoints, "none") == 0) {
      regionKind = CPP_REGION;
    } else if (regionKind != THINCLIENT_HA_REGION) {
      regionKind = THINCLIENT_REGION;
    }
  } else if (rattrs->getPoolName()) {
    PoolPtr pPtr = getCache()->getPoolManager().find(rattrs->getPoolName());
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

QueryServicePtr CacheImpl::getQueryService(bool noInit) {
  if (m_defaultPool != nullptr) {
    if (m_defaultPool->isDestroyed()) {
      throw IllegalStateException("Pool has been destroyed.");
    }
    return m_defaultPool->getQueryService();
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

QueryServicePtr CacheImpl::getQueryService(const char* poolName) {
  if (poolName == nullptr || strlen(poolName) == 0) {
    throw IllegalArgumentException("PoolName is nullptr or not defined..");
  }
  PoolPtr pool = getCache()->getPoolManager().find(poolName);

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
  if (m_closed || m_destroyPending) {
    throw CacheClosedException("Cache::getName: cache closed");
  }
  return m_name;
}

bool CacheImpl::isClosed() const { return m_closed; }

void CacheImpl::setAttributes(const CacheAttributesPtr& attrs) {
  if (m_attributes == nullptr && attrs != nullptr) {
    m_attributes = attrs;
  }
}

DistributedSystem& CacheImpl::getDistributedSystem() const {
  return *m_distributedSystem;
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
    RegionInternalPtr rImpl =
        std::dynamic_pointer_cast<RegionInternal>((*q).int_id_);
    if (rImpl != nullptr) {
      rImpl->destroyRegionNoThrow(
          nullptr, false,
          CacheEventFlags::LOCAL | CacheEventFlags::CACHE_CLOSE);
    }
  }

  if (m_evictionControllerPtr != nullptr) {
    m_evictionControllerPtr->stop();
    GF_SAFE_DELETE(m_evictionControllerPtr);
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
    GF_SAFE_DELETE(m_cacheStats);
  }

  m_regions->unbind_all();
  LOGDEBUG("CacheImpl::close( ): destroyed regions.");

  GF_SAFE_DELETE(m_tcrConnectionManager);
  m_cacheTXManager = nullptr;

  m_expiryTaskManager->stopExpiryTaskManager();

  m_closed = true;

  LOGFINE("Cache closed.");
}

bool CacheImpl::isCacheDestroyPending() const { return m_destroyPending; }

void CacheImpl::validateRegionAttributes(
    const char* name, const RegionAttributesPtr& attrs) const {
  RegionKind kind = getRegionKind(attrs);
  std::string buffer = "Cache::createRegion: \"";
  buffer += name;
  buffer += "\" ";

  if (attrs->m_clientNotificationEnabled && kind == CPP_REGION) {
    buffer +=
        "Client notification can be enabled only for native client region";
    throw UnsupportedOperationException(buffer.c_str());
  }
}

// We'll pass a nullptr loader function pointer and let the region.get method to
// do a load using a real C++ loader, instead of passing a member function
// pointer here
void CacheImpl::createRegion(const char* name,
                             const RegionAttributesPtr& aRegionAttributes,
                             RegionPtr& regionPtr) {
  {
    ACE_Guard<ACE_Thread_Mutex> _guard(m_initDoneLock);
    if (!m_initDone) {
      if (!(aRegionAttributes->getPoolName())) {
        m_tcrConnectionManager->init();
        m_remoteQueryServicePtr = std::make_shared<RemoteQueryService>(this);
        auto& prop = m_distributedSystem->getSystemProperties();
        if (prop.statisticsEnabled()) {
          m_adminRegion = AdminRegion::create(this);
        }
      }
      m_initDone = true;
    }
  }

  if (m_closed || m_destroyPending) {
    throw CacheClosedException("Cache::createRegion: cache closed");
  }
  if (aRegionAttributes == nullptr) {
    throw IllegalArgumentException(
        "Cache::createRegion: RegionAttributes is null");
  }
  std::string namestr(name);

  if (namestr.find('/') != std::string::npos) {
    throw IllegalArgumentException(
        "Malformed name string, contains region path seperator '/'");
  }

  validateRegionAttributes(name, aRegionAttributes);
  RegionInternalPtr rpImpl = nullptr;
  {
    // For multi threading and the operations between bind and find seems to be
    // hard to be atomic since a regionImpl needs to be valid before it can be
    // bound
    MapOfRegionGuard guard1(m_regions->mutex());
    RegionPtr tmp;
    if (0 == m_regions->find(namestr, tmp)) {
      char buffer[256];
      ACE_OS::snprintf(
          buffer, 256,
          "Cache::createRegion: \"%s\" region exists in local cache",
          namestr.c_str());
      throw RegionExistsException(buffer);
    }

    auto csptr = std::make_shared<CacheStatistics>();
    try {
      rpImpl = createRegion_internal(namestr.c_str(), nullptr,
                                     aRegionAttributes, csptr, false);
    } catch (const AuthenticationFailedException&) {
      throw;
    } catch (const AuthenticationRequiredException&) {
      throw;
    } catch (const Exception&) {
      //      LOGERROR( "Cache::createRegion: region creation failed, caught
      //      exception: %s", ex.getMessage() );
      //      ex.printStackTrace();
      throw;
    } catch (std::exception& ex) {
      char buffer[512];
      ACE_OS::snprintf(buffer, 512,
                       "Cache::createRegion: Failed to create Region \"%s\" "
                       "due to unknown exception: %s",
                       namestr.c_str(), ex.what());
      throw UnknownException(buffer);
    } catch (...) {
      char buffer[256];
      ACE_OS::snprintf(buffer, 256,
                       "Cache::createRegion: Failed to create Region \"%s\" "
                       "due to unknown exception.",
                       namestr.c_str());
      throw UnknownException(buffer);
    }
    if (rpImpl == nullptr) {
      char buffer[256];
      ACE_OS::snprintf(buffer, 256,
                       "Cache::createRegion: Failed to create Region \"%s\"",
                       namestr.c_str());
      throw RegionCreationFailedException(buffer);
    }
    regionPtr = rpImpl;
    rpImpl->addDisMessToQueue();
    // Instantiate a PersistenceManager object if DiskPolicy is overflow
    if (aRegionAttributes->getDiskPolicy() == DiskPolicyType::OVERFLOWS) {
      PersistenceManagerPtr pmPtr = aRegionAttributes->getPersistenceManager();
      if (pmPtr == nullptr) {
        throw NullPointerException(
            "PersistenceManager could not be instantiated");
      }
      PropertiesPtr props = aRegionAttributes->getPersistenceProperties();
      pmPtr->init(regionPtr, props);
      rpImpl->setPersistenceManager(pmPtr);
    }

    rpImpl->acquireReadLock();
    m_regions->bind(regionPtr->getName(), regionPtr);

    // When region is created, added that region name in client meta data
    // service to fetch its
    // metadata for single hop.
    auto& props = m_distributedSystem->getSystemProperties();
    if (!props.isGridClient()) {
      const char* poolName = aRegionAttributes->getPoolName();
      if (poolName != nullptr) {
        PoolPtr pool = getCache()->getPoolManager().find(poolName);
        if (pool != nullptr && !pool->isDestroyed() &&
            pool->getPRSingleHopEnabled()) {
          ThinClientPoolDM* poolDM =
              dynamic_cast<ThinClientPoolDM*>(pool.get());
          if ((poolDM != nullptr) &&
              (poolDM->getClientMetaDataService() != nullptr)) {
            LOGFINE(
                "enqueued region %s for initial metadata refresh for "
                "singlehop ",
                name);
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
  //   LOGFINE( "Returning from CacheImpl::createRegion call for Region %s",
  //   regionPtr->getFullPath() );
}

/**
 * Return the existing region (or subregion) with the specified
 * path that already exists or is already mapped into the cache.
 * Whether or not the path starts with a forward slash it is interpreted as a
 * full path starting at a root.
 *
 * @param path the path to the region
 * @param[out] rptr the region pointer that is returned
 * @return the Region or null if not found
 * @throws IllegalArgumentException if path is null, the empty string, or "/"
 */

void CacheImpl::getRegion(const char* path, RegionPtr& rptr) {
  TryReadGuard guardCacheDestroy(m_destroyCacheMutex, m_destroyPending);
  if (m_destroyPending) {
    rptr = nullptr;
    return;
  }

  MapOfRegionGuard guard(m_regions->mutex());
  std::string pathstr;
  if (path != nullptr) {
    pathstr = path;
  }
  rptr = nullptr;
  std::string slash("/");
  if ((path == (void*)nullptr) || (pathstr == slash) ||
      (pathstr.length() < 1)) {
    LOGERROR("Cache::getRegion: path [%s] is not valid.", pathstr.c_str());
    throw IllegalArgumentException("Cache::getRegion: path is null or a /");
  }
  std::string fullname = pathstr;
  if (fullname.substr(0, 1) == slash) {
    fullname = pathstr.substr(1);
  }
  // find second separator
  uint32_t idx = static_cast<uint32_t>(fullname.find('/'));
  std::string stepname = fullname.substr(0, idx);
  RegionPtr region;
  if (0 == m_regions->find(stepname, region)) {
    if (stepname == fullname) {
      // done...
      rptr = region;
      return;
    }
    std::string remainder = fullname.substr(stepname.length() + 1);
    if (region != nullptr) {
      rptr = region->getSubregion(remainder.c_str());
    } else {
      rptr = nullptr;
      return;  // Return null if the parent region was not found.
    }
  }
}

std::shared_ptr<RegionInternal> CacheImpl::createRegion_internal(
    const std::string& name, const std::shared_ptr<RegionInternal>& rootRegion,
    const RegionAttributesPtr& attrs, const CacheStatisticsPtr& csptr,
    bool shared) {
  if (attrs == nullptr) {
    throw IllegalArgumentException(
        "createRegion: "
        "RegionAttributes is null");
  }

  RegionInternalPtr rptr = nullptr;
  RegionKind regionKind = getRegionKind(attrs);
  const char* poolName = attrs->getPoolName();
  const char* regionEndpoints = attrs->getEndpoints();
  const char* cacheEndpoints =
      m_attributes == nullptr ? nullptr : m_attributes->getEndpoints();

  /*if(m_defaultPool != nullptr && (poolName == nullptr || strlen(poolName) ==
  0))
  {
    attrs->setPoolName(m_defaultPool->getName());
  }*/

  if (poolName != nullptr) {
    PoolPtr pool = getCache()->getPoolManager().find(poolName);
    if (pool != nullptr && !pool->isDestroyed()) {
      bool isMultiUserSecureMode = pool->getMultiuserAuthentication();
      if (isMultiUserSecureMode && (attrs->getCachingEnabled())) {
        LOGERROR(
            "Pool [%s] is in multiuser authentication mode so region local "
            "caching is not supported.",
            poolName);
        throw IllegalStateException(
            "Pool is in multiuser authentication so region local caching is "
            "not supported.");
      }
    }
  }

  if (poolName && strlen(poolName) &&
      ((regionEndpoints && strlen(regionEndpoints)) ||
       (cacheEndpoints && strlen(cacheEndpoints)))) {
    LOGERROR(
        "Cache or region endpoints cannot be specified when pool name is "
        "specified for region %s",
        name.c_str());
    throw IllegalArgumentException(
        "Cache or region endpoints cannot be specified when pool name is "
        "specified");
  }

  if (regionKind == THINCLIENT_REGION) {
    LOGINFO("Creating region %s with region endpoints %s", name.c_str(),
            attrs->getEndpoints());
    auto tmp = std::make_shared<ThinClientRegion>(name, this, rootRegion, attrs,
                                                  csptr, shared);
    tmp->initTCR();
    rptr = tmp;
  } else if (regionKind == THINCLIENT_HA_REGION) {
    LOGINFO("Creating region %s with subscriptions enabled", name.c_str());
    auto tmp = std::make_shared<ThinClientHARegion>(name, this, rootRegion,
                                                    attrs, csptr, shared);
    tmp->initTCR();
    rptr = tmp;
  } else if (regionKind == THINCLIENT_POOL_REGION) {
    LOGINFO("Creating region %s attached to pool %s", name.c_str(),
            attrs->getPoolName());
    auto tmp = std::make_shared<ThinClientPoolRegion>(name, this, rootRegion,
                                                      attrs, csptr, shared);
    tmp->initTCR();
    rptr = tmp;
  } else {
    LOGINFO("Creating local region %s", name.c_str());
    rptr = std::make_shared<LocalRegion>(name, this, rootRegion, attrs, csptr,
                                         shared);
  }
  return rptr;
}

void CacheImpl::rootRegions(VectorOfRegion& regions) {
  regions.clear();
  MapOfRegionGuard guard(m_regions->mutex());
  if (m_regions->current_size() == 0) return;
  regions.reserve(static_cast<int32_t>(m_regions->current_size()));
  for (MapOfRegionWithLock::iterator q = m_regions->begin();
       q != m_regions->end(); ++q) {
    if ((*q).int_id_->isDestroyed() == false) {
      regions.push_back((*q).int_id_);
    }
  }
}

EvictionController* CacheImpl::getEvictionController() {
  return m_evictionControllerPtr;
}

void CacheImpl::readyForEvents() {
  bool autoReadyForEvents =
      m_distributedSystem->getSystemProperties().autoReadyForEvents();
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

  const auto& pools = getCache()->getPoolManager().getAll();
  if (pools.empty()) throw IllegalStateException("No pools found.");
  for (const auto& itr : pools) {
    const auto& currPool = itr.second;
    LOGDEBUG("Sending readyForEvents( ) with pool %s", currPool->getName());
    try {
      if (const auto& poolHADM =
              std::dynamic_pointer_cast<ThinClientPoolHADM>(currPool)) {
        poolHADM->readyForEvents();
      }
    } catch (Exception& ex) {
      LOGWARN("readyForEvents( ) failed for pool %s with exception: %s",
              currPool->getName(), ex.getMessage());
    }
  }
}

bool CacheImpl::getEndpointStatus(const std::string& endpoint) {
  const auto& pools = getCache()->getPoolManager().getAll();
  std::string fullName = endpoint;

  if (pools.empty()) {
    return m_tcrConnectionManager->getEndpointStatus(fullName);
  }

  fullName = endpoint.find(':') == std::string::npos
                 ? endpoint
                 : Utils::convertHostToCanonicalForm(endpoint.c_str());

  const auto firstPool =
      std::static_pointer_cast<ThinClientPoolDM>(pools.begin()->second);

  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(firstPool->m_endpointsLock);
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
            this->getCache()->createDataOutput(), true);
        tcrHARegion->receiveNotification(regionMsg);
        for (const auto& iter : tcrHARegion->subregions(true)) {
          if (!iter->isDestroyed()) {
            if (const auto subregion =
                    std::dynamic_pointer_cast<ThinClientHARegion>(iter)) {
              regionMsg = new TcrMessageClientMarker(
                  this->getCache()->createDataOutput(), true);
              subregion->receiveNotification(regionMsg);
            }
          }
        }
      }
    }
  }
}

int CacheImpl::getPoolSize(const char* poolName) {
  if (const auto pool = getCache()->getPoolManager().find(poolName)) {
    if (const auto dm = std::dynamic_pointer_cast<ThinClientPoolDM>(pool)) {
      return dm->m_poolSize;
    }
  }
  return -1;
}

RegionFactory CacheImpl::createRegionFactory(RegionShortcut preDefinedRegion) {
  return RegionFactory(preDefinedRegion, this);
}

std::map<std::string, RegionAttributesPtr> CacheImpl::getRegionShortcut() {
  std::map<std::string, RegionAttributesPtr> preDefined;

  {
    // PROXY
    auto regAttr_PROXY = std::make_shared<RegionAttributes>();
    regAttr_PROXY->setCachingEnabled(false);
    preDefined["PROXY"] = regAttr_PROXY;
  }

  {
    // CACHING_PROXY
    auto regAttr_CACHING_PROXY = std::make_shared<RegionAttributes>();
    regAttr_CACHING_PROXY->setCachingEnabled(true);
    preDefined["CACHING_PROXY"] = regAttr_CACHING_PROXY;
  }

  {
    // CACHING_PROXY_ENTRY_LRU
    auto regAttr_CACHING_PROXY_LRU = std::make_shared<RegionAttributes>();
    regAttr_CACHING_PROXY_LRU->setCachingEnabled(true);
    regAttr_CACHING_PROXY_LRU->setLruEntriesLimit(DEFAULT_LRU_MAXIMUM_ENTRIES);
    preDefined["CACHING_PROXY_ENTRY_LRU"] = regAttr_CACHING_PROXY_LRU;
  }

  {
    // LOCAL
    auto regAttr_LOCAL = std::make_shared<RegionAttributes>();
    preDefined["LOCAL"] = regAttr_LOCAL;
  }

  {
    // LOCAL_ENTRY_LRU
    auto regAttr_LOCAL_LRU = std::make_shared<RegionAttributes>();
    regAttr_LOCAL_LRU->setLruEntriesLimit(DEFAULT_LRU_MAXIMUM_ENTRIES);
    preDefined["LOCAL_ENTRY_LRU"] = regAttr_LOCAL_LRU;
  }

  return preDefined;
}

PdxTypeRegistryPtr CacheImpl::getPdxTypeRegistry() const {
  return m_pdxTypeRegistry;
}

SerializationRegistryPtr CacheImpl::getSerializationRegistry() const {
  return m_serializationRegistry;
}

ThreadPool* CacheImpl::getThreadPool() { return m_threadPool; }

CacheTransactionManagerPtr CacheImpl::getCacheTransactionManager() {
  return m_cacheTXManager;
}
MemberListForVersionStampPtr CacheImpl::getMemberListForVersionStamp() {
  static auto versionStampMemIdList =
      new std::shared_ptr<MemberListForVersionStamp>(
          new MemberListForVersionStamp());
  return *versionStampMemIdList;
}
