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

typedef std::unordered_map<std::shared_ptr<CacheableKey>,
                           std::pair<std::shared_ptr<Cacheable>, int>>
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

class CPPCACHE_EXPORT LocalRegion : public RegionInternal {
  /**
   * @brief Public Methods for Region
   */
 public:
  /**
   * @brief constructor/destructor
   */
  LocalRegion(const std::string& name, CacheImpl* cache,
              const std::shared_ptr<RegionInternal>& rPtr,
              const std::shared_ptr<RegionAttributes>& attributes,
              const std::shared_ptr<CacheStatistics>& stats,
              bool shared = false, bool enableTimeStatistics = true);
  virtual ~LocalRegion();

  const char* getName() const;
  const char* getFullPath() const;
  std::shared_ptr<Region> getParentRegion() const;
  std::shared_ptr<RegionAttributes> getAttributes() const {
    return m_regionAttributes;
  }
  std::shared_ptr<AttributesMutator> getAttributesMutator() const {
    return std::make_shared<AttributesMutator>(
        std::const_pointer_cast<LocalRegion>(
            std::static_pointer_cast<const LocalRegion>(shared_from_this())));
  }
  void updateAccessAndModifiedTime(bool modified);
  std::shared_ptr<CacheStatistics> getStatistics() const;
  virtual void clear(
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  virtual void localClear(
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  GfErrType localClearNoThrow(
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr,
      const CacheEventFlags eventFlags = CacheEventFlags::NORMAL);
  void invalidateRegion(const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  void localInvalidateRegion(
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  void destroyRegion(
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  void localDestroyRegion(
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  std::shared_ptr<Region> getSubregion(const char* path);
  std::shared_ptr<Region> createSubregion(
      const char* subregionName,
      const std::shared_ptr<RegionAttributes>& aRegionAttributes);
  std::vector<std::shared_ptr<Region>> subregions(const bool recursive);
  std::shared_ptr<RegionEntry> getEntry(
      const std::shared_ptr<CacheableKey>& key);
  void getEntry(const std::shared_ptr<CacheableKey>& key,
                std::shared_ptr<Cacheable>& valuePtr);
  std::shared_ptr<Cacheable> get(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument);
  void put(const std::shared_ptr<CacheableKey>& key,
           const std::shared_ptr<Cacheable>& value,
           const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  void localPut(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  void create(const std::shared_ptr<CacheableKey>& key,
              const std::shared_ptr<Cacheable>& value,
              const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  void localCreate(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  void invalidate(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  void localInvalidate(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  void destroy(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  void localDestroy(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  bool remove(const std::shared_ptr<CacheableKey>& key,
              const std::shared_ptr<Cacheable>& value,
              const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  bool removeEx(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  bool localRemove(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  bool localRemoveEx(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  std::vector<std::shared_ptr<CacheableKey>> keys();
  std::vector<std::shared_ptr<CacheableKey>> serverKeys();
  std::vector<std::shared_ptr<Cacheable>> values();
  std::vector<std::shared_ptr<RegionEntry>> entries(bool recursive);

  HashMapOfCacheable getAll(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);

  HashMapOfCacheable getAll_internal(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      bool addToLocalCache);

  void putAll(const HashMapOfCacheable& map,
              uint32_t timeout = DEFAULT_RESPONSE_TIMEOUT,
              const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  void removeAll(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  uint32_t size();
  virtual uint32_t size_remote();
  std::shared_ptr<RegionService> getRegionService() const;
  virtual bool containsValueForKey_remote(
      const std::shared_ptr<CacheableKey>& keyPtr) const;
  bool containsValueForKey(const std::shared_ptr<CacheableKey>& keyPtr) const;
  bool containsKey(const std::shared_ptr<CacheableKey>& keyPtr) const;
  virtual bool containsKeyOnServer(
      const std::shared_ptr<CacheableKey>& keyPtr) const;
  virtual std::vector<std::shared_ptr<CacheableKey>> getInterestList() const;
  virtual std::vector<std::shared_ptr<CacheableString>> getInterestListRegex()
      const;

  /** @brief Public Methods from RegionInternal
   *  There are all virtual methods
   */
  std::shared_ptr<PersistenceManager> getPersistenceManager() { return m_persistenceManager; }
  void setPersistenceManager(std::shared_ptr<PersistenceManager>& pmPtr);

  virtual GfErrType getNoThrow(
      const std::shared_ptr<CacheableKey>& key,
      std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument);
  virtual GfErrType getAllNoThrow(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      const std::shared_ptr<HashMapOfCacheable>& values,
      const std::shared_ptr<HashMapOfException>& exceptions,
      const bool addToLocalCache,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  virtual GfErrType putNoThrow(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<Cacheable>& oldValue, int updateCount,
      const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag,
      DataInput* delta = nullptr, std::shared_ptr<EventId> eventId = nullptr);
  virtual GfErrType putNoThrowTX(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<Cacheable>& oldValue, int updateCount,
      const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag,
      DataInput* delta = nullptr, std::shared_ptr<EventId> eventId = nullptr);
  virtual GfErrType createNoThrow(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
      const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag);
  virtual GfErrType destroyNoThrow(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
      const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag);
  virtual GfErrType destroyNoThrowTX(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
      const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag);
  virtual GfErrType removeNoThrow(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
      const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag);
  virtual GfErrType removeNoThrowEx(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
      const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag);
  virtual GfErrType putAllNoThrow(
      const HashMapOfCacheable& map,
      uint32_t timeout = DEFAULT_RESPONSE_TIMEOUT,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  virtual GfErrType removeAllNoThrow(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  virtual GfErrType invalidateNoThrow(
      const std::shared_ptr<CacheableKey>& keyPtr,
      const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
      const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag);
  virtual GfErrType invalidateNoThrowTX(
      const std::shared_ptr<CacheableKey>& keyPtr,
      const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
      const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag);
  GfErrType invalidateRegionNoThrow(
      const std::shared_ptr<Serializable>& aCallbackArgument,
      const CacheEventFlags eventFlags);
  GfErrType destroyRegionNoThrow(
      const std::shared_ptr<Serializable>& aCallbackArgument,
      bool removeFromParent, const CacheEventFlags eventFlags);
  void tombstoneOperationNoThrow(
      const std::shared_ptr<CacheableHashMap>& tombstoneVersions,
      const std::shared_ptr<CacheableHashSet>& tombstoneKeys);

  //  moved putLocal to public since this is used by a few other
  // classes like CacheableObjectPartList now
  /** put an entry in local cache without invoking any callbacks */
  GfErrType putLocal(const char* name, bool isCreate,
                     const std::shared_ptr<CacheableKey>& keyPtr,
                     const std::shared_ptr<Cacheable>& valuePtr,
                     std::shared_ptr<Cacheable>& oldValue, bool cachingEnabled,
                     int updateCount, int destroyTracker,
                     std::shared_ptr<VersionTag> versionTag,
                     DataInput* delta = nullptr,
                     std::shared_ptr<EventId> eventId = nullptr);
  GfErrType invalidateLocal(const char* name,
                            const std::shared_ptr<CacheableKey>& keyPtr,
                            const std::shared_ptr<Cacheable>& value,
                            const CacheEventFlags eventFlags,
                            std::shared_ptr<VersionTag> versionTag);

  void setRegionExpiryTask();
  void acquireReadLock() { m_rwLock.acquire_read(); }
  void releaseReadLock() { m_rwLock.release(); }

  // behaviors for attributes mutator
  uint32_t adjustLruEntriesLimit(uint32_t limit);
  ExpirationAction::Action adjustRegionExpiryAction(
      ExpirationAction::Action action);
  ExpirationAction::Action adjustEntryExpiryAction(
      ExpirationAction::Action action);
  int32_t adjustRegionExpiryDuration(int32_t duration);
  int32_t adjustEntryExpiryDuration(int32_t duration);

  // other public methods
  RegionStats* getRegionStats() { return m_regionStats; }
  inline bool cacheEnabled() { return m_regionAttributes->getCachingEnabled(); }
  inline bool cachelessWithListener() {
    return !m_regionAttributes->getCachingEnabled() && (m_listener != nullptr);
  }
  virtual bool isDestroyed() const { return m_destroyPending; }
  /* above public methods are inherited from RegionInternal */

  virtual void adjustCacheListener(const std::shared_ptr<CacheListener>& aListener);
  virtual void adjustCacheListener(const char* libpath,
                                   const char* factoryFuncName);
  virtual void adjustCacheLoader(const std::shared_ptr<CacheLoader>& aLoader);
  virtual void adjustCacheLoader(const char* libpath,
                                 const char* factoryFuncName);
  virtual void adjustCacheWriter(const std::shared_ptr<CacheWriter>& aWriter);
  virtual void adjustCacheWriter(const char* libpath,
                                 const char* factoryFuncName);
  virtual CacheImpl* getCacheImpl() const;
  virtual void evict(int32_t percentage);

  virtual void acquireGlobals(bool isFailover){};
  virtual void releaseGlobals(bool isFailover){};

  virtual bool getProcessedMarker() { return true; }
  EntriesMap* getEntryMap() { return m_entries; }
  virtual std::shared_ptr<TombstoneList> getTombstoneList();

 protected:
  /* virtual protected methods */
  virtual void release(bool invokeCallbacks = true);
  virtual GfErrType getNoThrow_remote(
      const std::shared_ptr<CacheableKey>& keyPtr,
      std::shared_ptr<Cacheable>& valPtr,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<VersionTag>& versionTag);
  virtual GfErrType putNoThrow_remote(
      const std::shared_ptr<CacheableKey>& keyPtr,
      const std::shared_ptr<Cacheable>& cvalue,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<VersionTag>& versionTag, bool checkDelta = true);
  virtual GfErrType putAllNoThrow_remote(
      const HashMapOfCacheable& map,
      std::shared_ptr<VersionedCacheableObjectPartList>& versionedObjPartList,
      uint32_t timeout, const std::shared_ptr<Serializable>& aCallbackArgument);
  virtual GfErrType removeAllNoThrow_remote(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      std::shared_ptr<VersionedCacheableObjectPartList>& versionedObjPartList,
      const std::shared_ptr<Serializable>& aCallbackArgument);
  virtual GfErrType createNoThrow_remote(
      const std::shared_ptr<CacheableKey>& keyPtr,
      const std::shared_ptr<Cacheable>& cvalue,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<VersionTag>& versionTag);
  virtual GfErrType destroyNoThrow_remote(
      const std::shared_ptr<CacheableKey>& keyPtr,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<VersionTag>& versionTag);
  virtual GfErrType removeNoThrow_remote(
      const std::shared_ptr<CacheableKey>& keyPtr,
      const std::shared_ptr<Cacheable>& cvalue,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<VersionTag>& versionTag);
  virtual GfErrType removeNoThrowEX_remote(
      const std::shared_ptr<CacheableKey>& keyPtr,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<VersionTag>& versionTag);
  virtual GfErrType invalidateNoThrow_remote(
      const std::shared_ptr<CacheableKey>& keyPtr,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<VersionTag>& versionTag);
  virtual GfErrType getAllNoThrow_remote(
      const std::vector<std::shared_ptr<CacheableKey>>* keys,
      const std::shared_ptr<HashMapOfCacheable>& values,
      const std::shared_ptr<HashMapOfException>& exceptions,
      const std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>>&
          resultKeys,
      bool addToLocalCache,
      const std::shared_ptr<Serializable>& aCallbackArgument);
  virtual GfErrType invalidateRegionNoThrow_remote(
      const std::shared_ptr<Serializable>& aCallbackArgument);
  virtual GfErrType destroyRegionNoThrow_remote(
      const std::shared_ptr<Serializable>& aCallbackArgument);
  virtual GfErrType unregisterKeysBeforeDestroyRegion();
  virtual const std::shared_ptr<Pool>& getPool() { return m_attachedPool; }

  void setPool(const std::shared_ptr<Pool>& p) { m_attachedPool = p; }

  TXState* getTXState() const {
    return TSSTXStateWrapper::s_geodeTSSTXState->getTXState();
  }

  std::shared_ptr<Cacheable> handleReplay(GfErrType& err, std::shared_ptr<Cacheable> value) const;

  bool isLocalOp(const CacheEventFlags* eventFlags = nullptr) {
    return typeid(*this) == typeid(LocalRegion) ||
           (eventFlags && eventFlags->isLocal());
  }

  // template method for put and create
  template <typename TAction>
  GfErrType updateNoThrow(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<Cacheable>& oldValue, int updateCount,
      const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag,
      DataInput* delta = nullptr, std::shared_ptr<EventId> eventId = nullptr);

  template <typename TAction>
  GfErrType updateNoThrowTX(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<Cacheable>& oldValue, int updateCount,
      const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag,
      DataInput* delta = nullptr, std::shared_ptr<EventId> eventId = nullptr);

  int64_t startStatOpTime();
  void updateStatOpTime(Statistics* m_regionStats, int32_t statId,
                        int64_t start);

  /* protected attributes */
  std::string m_name;
  std::shared_ptr<Region> m_parentRegion;
  MapOfRegionWithLock m_subRegions;
  std::string m_fullPath;
  CacheImpl* m_cacheImpl;
  volatile bool m_destroyPending;
  std::shared_ptr<CacheListener> m_listener;
  std::shared_ptr<CacheWriter> m_writer;
  std::shared_ptr<CacheLoader> m_loader;
  volatile bool m_released;
  EntriesMap* m_entries;  // map containing cache entries...
  RegionStats* m_regionStats;
  std::shared_ptr<CacheStatistics> m_cacheStatistics;
  bool m_transactionEnabled;
  std::shared_ptr<TombstoneList> m_tombstoneList;
  bool m_isPRSingleHopEnabled;
  std::shared_ptr<Pool> m_attachedPool;
  bool m_enableTimeStatistics;

  mutable ACE_RW_Thread_Mutex m_rwLock;
  std::vector<std::shared_ptr<CacheableKey>> keys_internal();
  bool containsKey_internal(const std::shared_ptr<CacheableKey>& keyPtr) const;
  int removeRegion(const std::string& name);

  bool invokeCacheWriterForEntryEvent(
      const std::shared_ptr<CacheableKey>& key,
      std::shared_ptr<Cacheable>& oldValue,
      const std::shared_ptr<Cacheable>& newValue,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      CacheEventFlags eventFlags, EntryEventType type);
  bool invokeCacheWriterForRegionEvent(
      const std::shared_ptr<Serializable>& aCallbackArgument,
      CacheEventFlags eventFlags, RegionEventType type);
  GfErrType invokeCacheListenerForEntryEvent(
      const std::shared_ptr<CacheableKey>& key,
      std::shared_ptr<Cacheable>& oldValue,
      const std::shared_ptr<Cacheable>& newValue,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      CacheEventFlags eventFlags, EntryEventType type, bool isLocal = false);
  GfErrType invokeCacheListenerForRegionEvent(
      const std::shared_ptr<Serializable>& aCallbackArgument,
      CacheEventFlags eventFlags, RegionEventType type);
  // functions related to expirations.
  void updateAccessAndModifiedTimeForEntry(std::shared_ptr<MapEntryImpl>& ptr, bool modified);
  void registerEntryExpiryTask(std::shared_ptr<MapEntryImpl>& entry);
  std::vector<std::shared_ptr<Region>> subregions_internal(
      const bool recursive);
  void entries_internal(std::vector<std::shared_ptr<RegionEntry>>& me,
                        const bool recursive);

  std::shared_ptr<PersistenceManager> m_persistenceManager;

  bool isStatisticsEnabled();
  bool useModifiedTimeForRegionExpiry();
  bool useModifiedTimeForEntryExpiry();
  bool isEntryIdletimeEnabled();
  ExpirationAction::Action getEntryExpirationAction() const;
  ExpirationAction::Action getRegionExpiryAction() const;
  uint32_t getRegionExpiryDuration() const;
  uint32_t getEntryExpiryDuration() const;
  void invokeAfterAllEndPointDisconnected();
  // Disallow copy constructor and assignment operator.
  LocalRegion(const LocalRegion&);
  LocalRegion& operator=(const LocalRegion&);

  virtual GfErrType getNoThrow_FullObject(
      std::shared_ptr<EventId> eventId, std::shared_ptr<Cacheable>& fullObject,
      std::shared_ptr<VersionTag>& versionTag);

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
