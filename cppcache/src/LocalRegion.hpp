#pragma once

#ifndef GEODE_LOCALREGION_H_
#define GEODE_LOCALREGION_H_

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

/**
 * @file
 */

#include <geode/geode_globals.hpp>
#include <geode/CacheStatistics.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/CacheableKey.hpp>
#include <geode/Cacheable.hpp>
#include <geode/Cache.hpp>
#include <geode/EntryEvent.hpp>
#include <geode/RegionEvent.hpp>
#include "EventType.hpp"
#include <geode/PersistenceManager.hpp>
#include <geode/RegionEntry.hpp>
#include <geode/CacheListener.hpp>
#include <geode/CacheWriter.hpp>
#include <geode/CacheLoader.hpp>
#include <geode/AttributesMutator.hpp>
#include <geode/AttributesFactory.hpp>

#include "RegionInternal.hpp"
#include "RegionStats.hpp"
#include "EntriesMapFactory.hpp"
#include "SerializationRegistry.hpp"
#include "MapWithLock.hpp"
#include "CacheableToken.hpp"
#include "ExpMapEntry.hpp"
#include "TombstoneList.hpp"

#include <ace/ACE.h>
#include <ace/Hash_Map_Manager_T.h>
#include <ace/Recursive_Thread_Mutex.h>

#include <string>
#include <unordered_map>
#include "TSSTXStateWrapper.hpp"

namespace apache {
namespace geode {
namespace client {

#ifndef CHECK_DESTROY_PENDING
#define CHECK_DESTROY_PENDING(lock, function)                      \
  lock checkGuard(m_rwLock, m_destroyPending);                     \
  if (m_destroyPending) {                                          \
    std::string err_msg = ": region " + m_fullPath + " destroyed"; \
    throw RegionDestroyedException(#function, err_msg.c_str());    \
  }
#endif

#ifndef CHECK_DESTROY_PENDING_NOTHROW
#define CHECK_DESTROY_PENDING_NOTHROW(lock)     \
  lock checkGuard(m_rwLock, m_destroyPending);  \
  if (m_destroyPending) {                       \
    return GF_CACHE_REGION_DESTROYED_EXCEPTION; \
  }
#endif

class PutActions;
class PutActionsTx;
class CreateActions;
class DestroyActions;
class RemoveActions;
class InvalidateActions;

typedef std::unordered_map<CacheableKeyPtr, std::pair<CacheablePtr, int> >
    MapOfOldValue;

/**
 * @class LocalRegion LocalRegion.hpp
 *
 * This class manages subregions and cached data. Each region
 * can contain multiple subregions and entries for data.
 * Regions provide a hierachical name space
 * within the cache. Also, a region can be used to group cached
 * objects for management purposes.
 *
 * The Region interface basically contains two set of APIs: Region management
 * APIs; and (potentially) distributed operations on entries. Non-distributed
 * operations on entries  are provided by <code>RegionEntry</code>.
 *
 * Each <code>Cache</code>  defines regions called the root regions.
 * User applications can use the root regions to create subregions
 * for isolated name space and object grouping.
 *
 * A region's name can be any String except that it should not contain
 * the region name separator, a forward slash (/).
 *
 * <code>Regions</code>  can be referenced by a relative path name from any
 * region
 * higher in the hierarchy in {@link Region::getSubregion}. You can get the
 * relative
 * path from the root region with {@link Region::getFullPath}. The name
 * separator is used to concatenate all the region names together from the root,
 * starting with the root's subregions.
 */
typedef std::shared_ptr<LocalRegion> LocalRegionPtr;

class CPPCACHE_EXPORT LocalRegion : public RegionInternal {
  /**
   * @brief Public Methods for Region
   */
 public:
  /**
   * @brief constructor/destructor
   */
  LocalRegion(const std::string& name, CacheImpl* cache,
              const RegionInternalPtr& rPtr,
              const RegionAttributesPtr& attributes,
              const CacheStatisticsPtr& stats, bool shared = false,
              bool enableTimeStatistics = true);
  virtual ~LocalRegion();

  const char* getName() const override;
  const char* getFullPath() const override;
  RegionPtr getParentRegion() const override;
  RegionAttributesPtr getAttributes() const override {
    return m_regionAttributes;
  }
  AttributesMutatorPtr getAttributesMutator() const override {
    return std::make_shared<AttributesMutator>(
        std::const_pointer_cast<LocalRegion>(
            std::static_pointer_cast<const LocalRegion>(shared_from_this())));
  }
  void updateAccessAndModifiedTime(bool modified) override;
  CacheStatisticsPtr getStatistics() const override;
  virtual void clear(
      const SerializablePtr& aCallbackArgument = nullptr) override;
  virtual void localClear(
      const SerializablePtr& aCallbackArgument = nullptr) override;
  GfErrType localClearNoThrow(
      const SerializablePtr& aCallbackArgument = nullptr,
      const CacheEventFlags eventFlags = CacheEventFlags::NORMAL);
  void invalidateRegion(
      const SerializablePtr& aCallbackArgument = nullptr) override;
  void localInvalidateRegion(
      const SerializablePtr& aCallbackArgument = nullptr) override;
  void destroyRegion(
      const SerializablePtr& aCallbackArgument = nullptr) override;
  void localDestroyRegion(
      const SerializablePtr& aCallbackArgument = nullptr) override;
  RegionPtr getSubregion(const char* path) override;
  RegionPtr createSubregion(
      const char* subregionName,
      const RegionAttributesPtr& aRegionAttributes) override;
  VectorOfRegion subregions(const bool recursive) override;
  RegionEntryPtr getEntry(const CacheableKeyPtr& key) override;
  void getEntry(const CacheableKeyPtr& key, CacheablePtr& valuePtr);
  CacheablePtr get(const CacheableKeyPtr& key,
                   const SerializablePtr& aCallbackArgument) override;
  void put(const CacheableKeyPtr& key, const CacheablePtr& value,
           const SerializablePtr& aCallbackArgument = nullptr) override;
  void localPut(const CacheableKeyPtr& key, const CacheablePtr& value,
                const SerializablePtr& aCallbackArgument = nullptr) override;
  void create(const CacheableKeyPtr& key, const CacheablePtr& value,
              const SerializablePtr& aCallbackArgument = nullptr) override;
  void localCreate(const CacheableKeyPtr& key, const CacheablePtr& value,
                   const SerializablePtr& aCallbackArgument = nullptr) override;
  void invalidate(const CacheableKeyPtr& key,
                  const SerializablePtr& aCallbackArgument = nullptr) override;
  void localInvalidate(
      const CacheableKeyPtr& key,
      const SerializablePtr& aCallbackArgument = nullptr) override;
  void destroy(const CacheableKeyPtr& key,
               const SerializablePtr& aCallbackArgument = nullptr) override;
  void localDestroy(
      const CacheableKeyPtr& key,
      const SerializablePtr& aCallbackArgument = nullptr) override;
  bool remove(const CacheableKeyPtr& key, const CacheablePtr& value,
              const SerializablePtr& aCallbackArgument = nullptr) override;
  bool removeEx(const CacheableKeyPtr& key,
                const SerializablePtr& aCallbackArgument = nullptr) override;
  bool localRemove(const CacheableKeyPtr& key, const CacheablePtr& value,
                   const SerializablePtr& aCallbackArgument = nullptr) override;
  bool localRemoveEx(const CacheableKeyPtr& key,
                     const SerializablePtr& aCallbackArgument = nullptr) override;
  VectorOfCacheableKey keys() override;
  VectorOfCacheableKey serverKeys() override;
  VectorOfCacheable values() override;
  VectorOfRegionEntry entries(bool recursive) override;

  HashMapOfCacheable getAll(const VectorOfCacheableKey& keys,
                            const SerializablePtr& aCallbackArgument = nullptr) override;

  HashMapOfCacheable getAll_internal(const VectorOfCacheableKey& keys,
                                     const SerializablePtr& aCallbackArgument,
                                     bool addToLocalCache) override;

  void putAll(const HashMapOfCacheable& map,
              std::chrono::milliseconds timeout = DEFAULT_RESPONSE_TIMEOUT,
              const SerializablePtr& aCallbackArgument = nullptr) override;
  void removeAll(const VectorOfCacheableKey& keys,
                 const SerializablePtr& aCallbackArgument = nullptr) override;
  uint32_t size() override;
  virtual uint32_t size_remote();
  RegionServicePtr getRegionService() const override;
  virtual bool containsValueForKey_remote(const CacheableKeyPtr& keyPtr) const;
  bool containsValueForKey(const CacheableKeyPtr& keyPtr) const  override;
  bool containsKey(const CacheableKeyPtr& keyPtr) const override;
  virtual bool containsKeyOnServer(const CacheableKeyPtr& keyPtr) const override;
  virtual VectorOfCacheableKey getInterestList() const override;
  virtual VectorOfCacheableString getInterestListRegex() const override;

  /** @brief Public Methods from RegionInternal
   *  There are all virtual methods
   */
  PersistenceManagerPtr getPersistenceManager() override {
    return m_persistenceManager;
  }
  void setPersistenceManager(PersistenceManagerPtr& pmPtr) override;

  virtual GfErrType getNoThrow(
      const CacheableKeyPtr& key, CacheablePtr& value,
      const SerializablePtr& aCallbackArgument) override;
  virtual GfErrType getAllNoThrow(
      const VectorOfCacheableKey& keys, const HashMapOfCacheablePtr& values,
      const HashMapOfExceptionPtr& exceptions, const bool addToLocalCache,
      const SerializablePtr& aCallbackArgument = nullptr) override;
  virtual GfErrType putNoThrow(const CacheableKeyPtr& key,
                               const CacheablePtr& value,
                               const SerializablePtr& aCallbackArgument,
                               CacheablePtr& oldValue, int updateCount,
                               const CacheEventFlags eventFlags,
                               VersionTagPtr versionTag,
                               DataInput* delta = nullptr,
                               EventIdPtr eventId = nullptr) override;
  virtual GfErrType putNoThrowTX(const CacheableKeyPtr& key,
                                 const CacheablePtr& value,
                                 const SerializablePtr& aCallbackArgument,
                                 CacheablePtr& oldValue, int updateCount,
                                 const CacheEventFlags eventFlags,
                                 VersionTagPtr versionTag,
                                 DataInput* delta = nullptr,
                                 EventIdPtr eventId = nullptr);
  virtual GfErrType createNoThrow(const CacheableKeyPtr& key,
                                  const CacheablePtr& value,
                                  const SerializablePtr& aCallbackArgument,
                                  int updateCount,
                                  const CacheEventFlags eventFlags,
                                  VersionTagPtr versionTag) override;
  virtual GfErrType destroyNoThrow(const CacheableKeyPtr& key,
                                   const SerializablePtr& aCallbackArgument,
                                   int updateCount,
                                   const CacheEventFlags eventFlags,
                                   VersionTagPtr versionTag) override;
  virtual GfErrType destroyNoThrowTX(const CacheableKeyPtr& key,
                                     const SerializablePtr& aCallbackArgument,
                                     int updateCount,
                                     const CacheEventFlags eventFlags,
                                     VersionTagPtr versionTag);
  virtual GfErrType removeNoThrow(const CacheableKeyPtr& key,
                                  const CacheablePtr& value,
                                  const SerializablePtr& aCallbackArgument,
                                  int updateCount,
                                  const CacheEventFlags eventFlags,
                                  VersionTagPtr versionTag) override;
  virtual GfErrType removeNoThrowEx(const CacheableKeyPtr& key,
                                    const SerializablePtr& aCallbackArgument,
                                    int updateCount,
                                    const CacheEventFlags eventFlags,
                                    VersionTagPtr versionTag);
  virtual GfErrType putAllNoThrow(
      const HashMapOfCacheable& map,
      std::chrono::milliseconds timeout = DEFAULT_RESPONSE_TIMEOUT,
      const SerializablePtr& aCallbackArgument = nullptr);
  virtual GfErrType removeAllNoThrow(
      const VectorOfCacheableKey& keys,
      const SerializablePtr& aCallbackArgument = nullptr);
  virtual GfErrType invalidateNoThrow(const CacheableKeyPtr& keyPtr,
                                      const SerializablePtr& aCallbackArgument,
                                      int updateCount,
                                      const CacheEventFlags eventFlags,
                                      VersionTagPtr versionTag) override;
  virtual GfErrType invalidateNoThrowTX(const CacheableKeyPtr& keyPtr,
                                        const SerializablePtr& aCallbackArgument,
                                        int updateCount,
                                        const CacheEventFlags eventFlags,
                                        VersionTagPtr versionTag);
  GfErrType invalidateRegionNoThrow(const SerializablePtr& aCallbackArgument,
                                    const CacheEventFlags eventFlags) override;
  GfErrType destroyRegionNoThrow(const SerializablePtr& aCallbackArgument,
                                 bool removeFromParent,
                                 const CacheEventFlags eventFlags) override;
  void tombstoneOperationNoThrow(const CacheableHashMapPtr& tombstoneVersions,
                                 const CacheableHashSetPtr& tombstoneKeys);

  //  moved putLocal to public since this is used by a few other
  // classes like CacheableObjectPartList now
  /** put an entry in local cache without invoking any callbacks */
  GfErrType putLocal(const char* name, bool isCreate,
                     const CacheableKeyPtr& keyPtr,
                     const CacheablePtr& valuePtr, CacheablePtr& oldValue,
                     bool cachingEnabled, int updateCount, int destroyTracker,
                     VersionTagPtr versionTag, DataInput* delta = nullptr,
                     EventIdPtr eventId = nullptr);
  GfErrType invalidateLocal(const char* name, const CacheableKeyPtr& keyPtr,
                            const CacheablePtr& value,
                            const CacheEventFlags eventFlags,
                            VersionTagPtr versionTag);

  void setRegionExpiryTask() override;
  void acquireReadLock() override { m_rwLock.acquire_read(); }
  void releaseReadLock() override { m_rwLock.release(); }

  // behaviors for attributes mutator
  uint32_t adjustLruEntriesLimit(uint32_t limit) override;
  ExpirationAction::Action adjustRegionExpiryAction(
      ExpirationAction::Action action) override;
  ExpirationAction::Action adjustEntryExpiryAction(
      ExpirationAction::Action action) override;
  std::chrono::seconds adjustRegionExpiryDuration(
      const std::chrono::seconds& duration) override;
  std::chrono::seconds adjustEntryExpiryDuration(
      const std::chrono::seconds& duration) override;

  // other public methods
  RegionStats* getRegionStats() override { return m_regionStats; }
  inline bool cacheEnabled() override {
    return m_regionAttributes->getCachingEnabled();
  }
  inline bool cachelessWithListener() {
    return !m_regionAttributes->getCachingEnabled() && (m_listener != nullptr);
  }
  virtual bool isDestroyed() const override { return m_destroyPending; }
  /* above public methods are inherited from RegionInternal */

  virtual void adjustCacheListener(const CacheListenerPtr& aListener) override;
  virtual void adjustCacheListener(const char* libpath,
                                   const char* factoryFuncName) override;
  virtual void adjustCacheLoader(const CacheLoaderPtr& aLoader) override;
  virtual void adjustCacheLoader(const char* libpath,
                                 const char* factoryFuncName) override;
  virtual void adjustCacheWriter(const CacheWriterPtr& aWriter) override;
  virtual void adjustCacheWriter(const char* libpath,
                                 const char* factoryFuncName) override;
  virtual CacheImpl* getCacheImpl() const override;
  virtual void evict(int32_t percentage) override;

  virtual void acquireGlobals(bool isFailover){};
  virtual void releaseGlobals(bool isFailover){};

  virtual bool getProcessedMarker() { return true; }
  EntriesMap* getEntryMap() { return m_entries; }
  virtual TombstoneListPtr getTombstoneList() override;

 protected:
  /* virtual protected methods */
  virtual void release(bool invokeCallbacks = true);
  virtual GfErrType getNoThrow_remote(const CacheableKeyPtr& keyPtr,
                                      CacheablePtr& valPtr,
                                      const SerializablePtr& aCallbackArgument,
                                      VersionTagPtr& versionTag);
  virtual GfErrType putNoThrow_remote(const CacheableKeyPtr& keyPtr,
                                      const CacheablePtr& cvalue,
                                      const SerializablePtr& aCallbackArgument,
                                      VersionTagPtr& versionTag,
                                      bool checkDelta = true);
  virtual GfErrType putAllNoThrow_remote(
      const HashMapOfCacheable& map,
      VersionedCacheableObjectPartListPtr& versionedObjPartList,
      std::chrono::milliseconds timeout,
      const SerializablePtr& aCallbackArgument);
  virtual GfErrType removeAllNoThrow_remote(
      const VectorOfCacheableKey& keys,
      VersionedCacheableObjectPartListPtr& versionedObjPartList,
      const SerializablePtr& aCallbackArgument);
  virtual GfErrType createNoThrow_remote(const CacheableKeyPtr& keyPtr,
                                         const CacheablePtr& cvalue,
                                         const SerializablePtr& aCallbackArgument,
                                         VersionTagPtr& versionTag);
  virtual GfErrType destroyNoThrow_remote(const CacheableKeyPtr& keyPtr,
                                          const SerializablePtr& aCallbackArgument,
                                          VersionTagPtr& versionTag);
  virtual GfErrType removeNoThrow_remote(const CacheableKeyPtr& keyPtr,
                                         const CacheablePtr& cvalue,
                                         const SerializablePtr& aCallbackArgument,
                                         VersionTagPtr& versionTag);
  virtual GfErrType removeNoThrowEX_remote(const CacheableKeyPtr& keyPtr,
                                           const SerializablePtr& aCallbackArgument,
                                           VersionTagPtr& versionTag);
  virtual GfErrType invalidateNoThrow_remote(
      const CacheableKeyPtr& keyPtr, const SerializablePtr& aCallbackArgument,
      VersionTagPtr& versionTag);
  virtual GfErrType getAllNoThrow_remote(
      const VectorOfCacheableKey* keys, const HashMapOfCacheablePtr& values,
      const HashMapOfExceptionPtr& exceptions,
      const VectorOfCacheableKeyPtr& resultKeys, bool addToLocalCache,
      const SerializablePtr& aCallbackArgument);
  virtual GfErrType invalidateRegionNoThrow_remote(
      const SerializablePtr& aCallbackArgument);
  virtual GfErrType destroyRegionNoThrow_remote(
      const SerializablePtr& aCallbackArgument);
  virtual GfErrType unregisterKeysBeforeDestroyRegion();
  virtual const PoolPtr& getPool() override { return m_attachedPool; }

  void setPool(const PoolPtr& p) { m_attachedPool = p; }

  TXState* getTXState() const {
    return TSSTXStateWrapper::s_geodeTSSTXState->getTXState();
  }

  CacheablePtr handleReplay(GfErrType& err, CacheablePtr value) const;

  bool isLocalOp(const CacheEventFlags* eventFlags = nullptr) {
    return typeid(*this) == typeid(LocalRegion) ||
           (eventFlags && eventFlags->isLocal());
  }

  // template method for put and create
  template <typename TAction>
  GfErrType updateNoThrow(const CacheableKeyPtr& key, const CacheablePtr& value,
                          const SerializablePtr& aCallbackArgument,
                          CacheablePtr& oldValue, int updateCount,
                          const CacheEventFlags eventFlags,
                          VersionTagPtr versionTag, DataInput* delta = nullptr,
                          EventIdPtr eventId = nullptr);

  template <typename TAction>
  GfErrType updateNoThrowTX(const CacheableKeyPtr& key,
                            const CacheablePtr& value,
                            const SerializablePtr& aCallbackArgument,
                            CacheablePtr& oldValue, int updateCount,
                            const CacheEventFlags eventFlags,
                            VersionTagPtr versionTag,
                            DataInput* delta = nullptr,
                            EventIdPtr eventId = nullptr);

  int64_t startStatOpTime();
  void updateStatOpTime(Statistics* m_regionStats, int32_t statId,
                        int64_t start);

  /* protected attributes */
  std::string m_name;
  RegionPtr m_parentRegion;
  MapOfRegionWithLock m_subRegions;
  std::string m_fullPath;
  CacheImpl* m_cacheImpl;
  volatile bool m_destroyPending;
  CacheListenerPtr m_listener;
  CacheWriterPtr m_writer;
  CacheLoaderPtr m_loader;
  volatile bool m_released;
  EntriesMap* m_entries;  // map containing cache entries...
  RegionStats* m_regionStats;
  CacheStatisticsPtr m_cacheStatistics;
  bool m_transactionEnabled;
  TombstoneListPtr m_tombstoneList;
  bool m_isPRSingleHopEnabled;
  PoolPtr m_attachedPool;
  bool m_enableTimeStatistics;

  mutable ACE_RW_Thread_Mutex m_rwLock;
  VectorOfCacheableKey keys_internal();
  bool containsKey_internal(const CacheableKeyPtr& keyPtr) const;
  int removeRegion(const std::string& name);

  bool invokeCacheWriterForEntryEvent(const CacheableKeyPtr& key,
                                      CacheablePtr& oldValue,
                                      const CacheablePtr& newValue,
                                      const SerializablePtr& aCallbackArgument,
                                      CacheEventFlags eventFlags,
                                      EntryEventType type);
  bool invokeCacheWriterForRegionEvent(const SerializablePtr& aCallbackArgument,
                                       CacheEventFlags eventFlags,
                                       RegionEventType type);
  GfErrType invokeCacheListenerForEntryEvent(
      const CacheableKeyPtr& key, CacheablePtr& oldValue,
      const CacheablePtr& newValue, const SerializablePtr& aCallbackArgument,
      CacheEventFlags eventFlags, EntryEventType type, bool isLocal = false);
  GfErrType invokeCacheListenerForRegionEvent(
      const SerializablePtr& aCallbackArgument, CacheEventFlags eventFlags,
      RegionEventType type);
  // functions related to expirations.
  void updateAccessAndModifiedTimeForEntry(MapEntryImplPtr& ptr,
                                           bool modified) override;
  void registerEntryExpiryTask(MapEntryImplPtr& entry);
  VectorOfRegion subregions_internal(const bool recursive);
  void entries_internal(VectorOfRegionEntry& me, const bool recursive);

  PersistenceManagerPtr m_persistenceManager;

  bool isStatisticsEnabled();
  bool useModifiedTimeForRegionExpiry();
  bool useModifiedTimeForEntryExpiry();
  bool isEntryIdletimeEnabled();
  ExpirationAction::Action getEntryExpirationAction() const;
  ExpirationAction::Action getRegionExpiryAction() const;
  std::chrono::seconds getRegionExpiryDuration() const;
  std::chrono::seconds getEntryExpiryDuration() const;
  void invokeAfterAllEndPointDisconnected();
  // Disallow copy constructor and assignment operator.
  LocalRegion(const LocalRegion&);
  LocalRegion& operator=(const LocalRegion&);

  virtual GfErrType getNoThrow_FullObject(EventIdPtr eventId,
                                          CacheablePtr& fullObject,
                                          VersionTagPtr& versionTag);

  // these classes encapsulate actions specific to update operations
  // used by the template <code>updateNoThrow</code> class
  friend class PutActions;
  friend class PutActionsTx;
  friend class CreateActions;
  friend class DestroyActions;
  friend class RemoveActions;
  friend class InvalidateActions;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_LOCALREGION_H_
