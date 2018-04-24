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

#pragma once

#ifndef GEODE_CACHEIMPL_H_
#define GEODE_CACHEIMPL_H_

#include <atomic>
#include <memory>

#include <ace/ACE.h>
#include <ace/Time_Value.h>
#include <ace/Guard_T.h>
#include <ace/Recursive_Thread_Mutex.h>

#include <geode/internal/geode_globals.hpp>
#include <geode/Cache.hpp>
#include <geode/CacheAttributes.hpp>
#include <geode/DistributedSystem.hpp>
#include <geode/TypeRegistry.hpp>

#include "MapWithLock.hpp"
#include "Condition.hpp"
#include "TcrConnectionManager.hpp"
#include "EvictionController.hpp"
#include "RemoteQueryService.hpp"
#include "AdminRegion.hpp"
#include "CachePerfStats.hpp"
#include "PdxTypeRegistry.hpp"
#include "MemberListForVersionStamp.hpp"
#include "ClientProxyMembershipIDFactory.hpp"
#include "NonCopyable.hpp"

#define DEFAULT_LRU_MAXIMUM_ENTRIES 100000
/** @todo period '.' consistency */
/** @todo fix returns to param documentation of result ptr... */

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

class CacheFactory;
class CacheStatistics;
class ExpiryTaskManager;
class PdxTypeRegistry;
class Pool;
class RegionAttributes;
class SerializationRegistry;
class ThreadPool;

/**
 * @class Cache Cache.hpp
 * Geode's implementation of a distributed C++ Cache.
 *
 * Caches are obtained from methods on the {@link CacheFactory} class.
 * <p>
 * When a cache is created a {@link DistributedSystem} must be specified.
 * This system tells the cache where to find other caches on the network
 * and how to communicate with them.
 * <p>
 * When a cache will no longer be used, it should be {@link #close closed}.
 * Once it {@link Cache::isClosed is closed} any attempt to use it

 * will cause a <code>CacheClosedException</code> to be thrown.
 *
 * <p>A cache can have multiple root regions, each with a different name.
 *
 */

class APACHE_GEODE_EXPORT CacheImpl : private NonCopyable,
                                      private NonAssignable {
  /**
   * @brief public methods
   */
 public:
  // added netDown and revive for tests to simulate client crash and network
  // drop
  void netDown();
  void revive();
  void setClientCrashTEST() { m_tcrConnectionManager->setClientCrashTEST(); }

  // For PrSingleHop C++unit testing.
  void setNetworkHopFlag(bool networkhopflag) {
    m_networkhop = networkhopflag;
  };

  bool getAndResetNetworkHopFlag() { return m_networkhop.exchange(false); }

  int getBlackListBucketTimeouts() { return m_blacklistBucketTimeout; }

  void incBlackListBucketTimeouts() { ++m_blacklistBucketTimeout; }

  int8_t getAndResetServerGroupFlag() { return m_serverGroupFlag.exchange(0); }

  void setServerGroupFlag(int8_t serverGroupFlag) {
    m_serverGroupFlag = serverGroupFlag;
  }

  std::shared_ptr<MemberListForVersionStamp> getMemberListForVersionStamp();

  /** Returns the name of this cache.
   * @return the string name of this cache
   */
  const std::string& getName() const;

  /**
   * Indicates if this cache has been closed.
   * After a new cache object is created, this method returns false;
   * After the close is called on this cache object, this method
   * returns true.
   *
   * @return true, if this cache is closed; false, otherwise
   */
  bool isClosed() const;

  /** Get the <code>CacheAttributes</code> for this cache. */
  inline std::shared_ptr<CacheAttributes> getAttributes() const {
    return m_attributes;
  }

  /** Set the <code>CacheAttributes</code> for this cache. */
  void setAttributes(const std::shared_ptr<CacheAttributes>& attributes);

  /**
   * Returns the distributed system that this cache was
   * {@link CacheFactory::create created} with.
   */
  DistributedSystem& getDistributedSystem();

  /**
   * Returns the type registry that this cache was
   * {@link CacheFactory::create created} with.
   */
  TypeRegistry& getTypeRegistry();

  /**
   * Terminates this object cache and releases all the local resources.
   * After this cache is closed, any further
   * method call on this cache or any region object will throw
   * <code>CacheClosedException</code>, unless otherwise noted.
   * @param keepalive whether to keep a durable client's queue alive.
   * @throws CacheClosedException,  if the cache is already closed.
   */
  void close(bool keepalive = false);

  /**
   * Creates a region  using the specified
   * RegionAttributes.
   *
   * @param name the name of the region to create
   * @param aRegionAttributes the attributes of the root region
   * @todo change return to param for regionPtr...
   * @param regionPtr the pointer object pointing to the returned region object
   * when the function returns
   * @throws InvalidArgumentException if the attributePtr is nullptr.
   * @throws RegionExistsException if a region is already in
   * this cache
   * @throws CacheClosedException if the cache is closed
   * @throws OutOfMemoryException if the memory allocation failed
   * @throws NotConnectedException if the cache is not connected
   * @throws UnknownException otherwise
   */
  void createRegion(std::string name, RegionAttributes aRegionAttributes,
                    std::shared_ptr<Region>& regionPtr);

  std::shared_ptr<Region> getRegion(const std::string& path);

  /**
   * Returns a set of root regions in the cache. Does not cause any
   * shared regions to be mapped into the cache. This set is a snapshot and
   * is not backed by the Cache. The regions passed in are cleared.
   *
   * @param regions the region collection object containing the returned set of
   * regions when the function returns
   */
  std::vector<std::shared_ptr<Region>> rootRegions();

  virtual RegionFactory createRegionFactory(RegionShortcut preDefinedRegion);

  void initializeDeclarativeCache(const std::string& cacheXml);

  std::shared_ptr<CacheTransactionManager> getCacheTransactionManager();

  /**
   * @brief destructor
   */
  virtual ~CacheImpl();

  /**
   * @brief constructors
   */
  CacheImpl(Cache* c, const std::shared_ptr<Properties>& dsProps,
            bool ignorePdxUnreadFields, bool readPdxSerialized,
            const std::shared_ptr<AuthInitialize>& authInitialize);

  void initServices();
  EvictionController* getEvictionController();

  ExpiryTaskManager& getExpiryTaskManager() { return *m_expiryTaskManager; }

  ClientProxyMembershipIDFactory& getClientProxyMembershipIDFactory() {
    return m_clientProxyMembershipIDFactory;
  }

  Cache* getCache() const { return m_cache; }

  TcrConnectionManager& tcrConnectionManager() {
    return *m_tcrConnectionManager;
  }

  int removeRegion(const char* name);

  std::shared_ptr<QueryService> getQueryService(bool noInit = false);

  std::shared_ptr<QueryService> getQueryService(const char* poolName);

  std::shared_ptr<RegionInternal> createRegion_internal(
      const std::string& name,
      const std::shared_ptr<RegionInternal>& rootRegion,
      const RegionAttributes& attrs,
      const std::shared_ptr<CacheStatistics>& csptr, bool shared);

  /**
   * Send the "client ready" message to the server.
   */
  void readyForEvents();

  static bool isPoolInMultiuserMode(std::shared_ptr<Region> regionPtr);

  //  TESTING: Durable clients. Not thread safe.
  bool getEndpointStatus(const std::string& endpoint);

  void processMarker();

  // Pool helpers for unit tests
  int getPoolSize(const char* poolName);

  bool getCacheMode() {
    return m_attributes == nullptr ? false : m_attributes->m_cacheMode;
  }

  bool getPdxIgnoreUnreadFields() { return m_ignorePdxUnreadFields; }

  void setPdxIgnoreUnreadFields(bool ignore) {
    m_ignorePdxUnreadFields = ignore;
  }

  void setPdxReadSerialized(bool val) { m_readPdxSerialized = val; }

  bool getPdxReadSerialized() { return m_readPdxSerialized; }

  bool isCacheDestroyPending() const;

  static std::map<std::string, RegionAttributes> getRegionShortcut();

  std::shared_ptr<PdxTypeRegistry> getPdxTypeRegistry() const;

  std::shared_ptr<SerializationRegistry> getSerializationRegistry() const;
  inline CachePerfStats& getCachePerfStats() { return *m_cacheStats; }

  PoolManager& getPoolManager() const { return *m_poolManager; }

  SystemProperties& getSystemProperties() const {
    return m_distributedSystem.getSystemProperties();
  }

  ThreadPool* getThreadPool();

  inline const std::shared_ptr<AuthInitialize>& getAuthInitialize() {
    return m_authInitialize;
  }

  statistics::StatisticsManager& getStatisticsManager() const {
    return *(m_statisticsManager.get());
  }

  virtual std::unique_ptr<DataOutput> createDataOutput() const;

  virtual std::unique_ptr<DataOutput> createDataOutput(Pool* pool) const;

  virtual std::unique_ptr<DataInput> createDataInput(const uint8_t* buffer,
                                                     size_t len) const;

  virtual std::unique_ptr<DataInput> createDataInput(const uint8_t* buffer,
                                                     size_t len,
                                                     Pool* pool) const;

  PdxInstanceFactory createPdxInstanceFactory(
      const std::string& className) const;

  AuthenticatedView createAuthenticatedView(
      std::shared_ptr<Properties> userSecurityProperties,
      const std::string& poolName);

 private:
  std::atomic<bool> m_networkhop;
  std::atomic<int> m_blacklistBucketTimeout;
  std::atomic<int8_t> m_serverGroupFlag;
  bool m_ignorePdxUnreadFields;
  bool m_readPdxSerialized;
  std::unique_ptr<ExpiryTaskManager> m_expiryTaskManager;

  // CachePerfStats
  CachePerfStats* m_cacheStats;

  std::unique_ptr<PoolManager> m_poolManager;

  std::unique_ptr<statistics::StatisticsManager> m_statisticsManager;

  enum RegionKind {
    CPP_REGION,
    THINCLIENT_REGION,
    THINCLIENT_HA_REGION,
    THINCLIENT_POOL_REGION
  };

  RegionKind getRegionKind(RegionAttributes rattrs) const;

  void sendNotificationCloseMsgs();

  void validateRegionAttributes(const std::string& name,
                                const RegionAttributes attrs) const;

  inline void getSubRegions(MapOfRegionWithLock& srm) {
    MapOfRegionGuard guard(m_regions->mutex());
    if (m_regions->current_size() == 0) return;
    for (MapOfRegionWithLock::iterator p = m_regions->begin();
         p != m_regions->end(); ++p) {
      srm.bind((*p).ext_id_, (*p).int_id_);
    }
  }

  void setCache(Cache* cache);

  bool m_closed;
  bool m_initialized;

  DistributedSystem m_distributedSystem;
  ClientProxyMembershipIDFactory m_clientProxyMembershipIDFactory;
  MapOfRegionWithLock* m_regions;
  Cache* m_cache;
  ACE_Recursive_Thread_Mutex m_mutex;
  Condition m_cond;
  std::shared_ptr<CacheAttributes> m_attributes;
  EvictionController* m_evictionControllerPtr;
  TcrConnectionManager* m_tcrConnectionManager;
  std::shared_ptr<RemoteQueryService> m_remoteQueryServicePtr;
  ACE_RW_Thread_Mutex m_destroyCacheMutex;
  volatile bool m_destroyPending;
  volatile bool m_initDone;
  ACE_Thread_Mutex m_initDoneLock;
  std::shared_ptr<AdminRegion> m_adminRegion;
  std::shared_ptr<CacheTransactionManager> m_cacheTXManager;

  MemberListForVersionStamp& m_memberListForVersionStamp;
  std::shared_ptr<SerializationRegistry> m_serializationRegistry;
  std::shared_ptr<PdxTypeRegistry> m_pdxTypeRegistry;
  ThreadPool* m_threadPool;
  const std::shared_ptr<AuthInitialize> m_authInitialize;
  std::unique_ptr<TypeRegistry> m_typeRegistry;

  friend class CacheFactory;
  friend class Cache;
  friend class DistributedSystemImpl;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHEIMPL_H_
