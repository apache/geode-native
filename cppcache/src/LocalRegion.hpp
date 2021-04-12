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

#ifndef GEODE_LOCALREGION_H_
#define GEODE_LOCALREGION_H_

#include <string>
#include <unordered_map>

#include <geode/AttributesMutator.hpp>
#include <geode/Cache.hpp>
#include <geode/CacheListener.hpp>
#include <geode/CacheLoader.hpp>
#include <geode/CacheStatistics.hpp>
#include <geode/CacheWriter.hpp>
#include <geode/CacheableKey.hpp>
#include <geode/EntryEvent.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/PersistenceManager.hpp>
#include <geode/RegionEntry.hpp>
#include <geode/RegionEvent.hpp>
#include <geode/Serializable.hpp>
#include <geode/internal/geode_globals.hpp>

#include "EntriesMap.hpp"
#include "EventType.hpp"
#include "RegionInternal.hpp"
#include "RegionStats.hpp"
#include "TSSTXStateWrapper.hpp"
#include "TombstoneList.hpp"
#include "util/synchronized_map.hpp"

namespace apache {
namespace geode {
namespace client {

#ifndef CHECK_DESTROY_PENDING
#define CHECK_DESTROY_PENDING(lock, function)             \
  lock checkGuard(m_rwLock, m_destroyPending);            \
  do {                                                    \
    if (m_destroyPending) {                               \
      std::string err_msg = #function;                    \
      err_msg += ": region " + m_fullPath + " destroyed"; \
      throw RegionDestroyedException(err_msg.c_str());    \
    }                                                     \
  } while (0)
#endif

#ifndef CHECK_DESTROY_PENDING_NOTHROW
#define CHECK_DESTROY_PENDING_NOTHROW(lock)       \
  lock checkGuard(m_rwLock, m_destroyPending);    \
  do {                                            \
    if (m_destroyPending) {                       \
      return GF_CACHE_REGION_DESTROYED_EXCEPTION; \
    }                                             \
  } while (0)
#endif

class CreateActions;
class DestroyActions;
class InvalidateActions;
class InterestResultPolicy;
class PutActions;
class PutActionsTx;
class RemoveActions;

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

class APACHE_GEODE_EXPORT LocalRegion : public RegionInternal {
  /**
   * @brief Public Methods for Region
   */
 public:
  LocalRegion(const std::string& name, CacheImpl* cache,
              const std::shared_ptr<RegionInternal>& rPtr,
              RegionAttributes attributes,
              const std::shared_ptr<CacheStatistics>& stats,
              bool enableTimeStatistics = true);

  LocalRegion(const LocalRegion&) = delete;
  LocalRegion& operator=(const LocalRegion&) = delete;

  ~LocalRegion() noexcept override;

  const std::string& getName() const override;
  const std::string& getFullPath() const override;
  std::shared_ptr<Region> getParentRegion() const override;
  const RegionAttributes& getAttributes() const override {
    return m_regionAttributes;
  }
  std::shared_ptr<AttributesMutator> getAttributesMutator() const override {
    return std::make_shared<AttributesMutator>(
        std::const_pointer_cast<LocalRegion>(
            std::static_pointer_cast<const LocalRegion>(shared_from_this())));
  }
  void updateAccessAndModifiedTime(bool modified) override;
  std::shared_ptr<CacheStatistics> getStatistics() const override;
  void clear(const std::shared_ptr<Serializable>& aCallbackArgument =
                 nullptr) override;
  void localClear(const std::shared_ptr<Serializable>& aCallbackArgument =
                      nullptr) override;
  GfErrType localClearNoThrow(
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr,
      const CacheEventFlags eventFlags = CacheEventFlags::NORMAL);
  void invalidateRegion(const std::shared_ptr<Serializable>& aCallbackArgument =
                            nullptr) override;
  void localInvalidateRegion(const std::shared_ptr<Serializable>&
                                 aCallbackArgument = nullptr) override;
  void destroyRegion(const std::shared_ptr<Serializable>& aCallbackArgument =
                         nullptr) override;
  void localDestroyRegion(const std::shared_ptr<Serializable>&
                              aCallbackArgument = nullptr) override;
  std::shared_ptr<Region> getSubregion(const std::string& path) override;
  std::shared_ptr<Region> createSubregion(
      const std::string& subregionName,
      RegionAttributes regionAttributes) override;
  std::vector<std::shared_ptr<Region>> subregions(
      const bool recursive) override;
  std::shared_ptr<RegionEntry> getEntry(
      const std::shared_ptr<CacheableKey>& key) override;
  void getEntry(const std::shared_ptr<CacheableKey>& key,
                std::shared_ptr<Cacheable>& valuePtr);
  std::shared_ptr<Cacheable> get(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument) override;
  void put(const std::shared_ptr<CacheableKey>& key,
           const std::shared_ptr<Cacheable>& value,
           const std::shared_ptr<Serializable>& aCallbackArgument =
               nullptr) override;
  void localPut(const std::shared_ptr<CacheableKey>& key,
                const std::shared_ptr<Cacheable>& value,
                const std::shared_ptr<Serializable>& aCallbackArgument =
                    nullptr) override;
  void create(const std::shared_ptr<CacheableKey>& key,
              const std::shared_ptr<Cacheable>& value,
              const std::shared_ptr<Serializable>& aCallbackArgument =
                  nullptr) override;
  void localCreate(const std::shared_ptr<CacheableKey>& key,
                   const std::shared_ptr<Cacheable>& value,
                   const std::shared_ptr<Serializable>& aCallbackArgument =
                       nullptr) override;
  void invalidate(const std::shared_ptr<CacheableKey>& key,
                  const std::shared_ptr<Serializable>& aCallbackArgument =
                      nullptr) override;
  void localInvalidate(const std::shared_ptr<CacheableKey>& key,
                       const std::shared_ptr<Serializable>& aCallbackArgument =
                           nullptr) override;
  void destroy(const std::shared_ptr<CacheableKey>& key,
               const std::shared_ptr<Serializable>& aCallbackArgument =
                   nullptr) override;
  void localDestroy(const std::shared_ptr<CacheableKey>& key,
                    const std::shared_ptr<Serializable>& aCallbackArgument =
                        nullptr) override;
  virtual void localDestroyNoCallbacks(
      const std::shared_ptr<CacheableKey>& key);
  bool remove(const std::shared_ptr<CacheableKey>& key,
              const std::shared_ptr<Cacheable>& value,
              const std::shared_ptr<Serializable>& aCallbackArgument =
                  nullptr) override;
  bool removeEx(const std::shared_ptr<CacheableKey>& key,
                const std::shared_ptr<Serializable>& aCallbackArgument =
                    nullptr) override;
  bool localRemove(const std::shared_ptr<CacheableKey>& key,
                   const std::shared_ptr<Cacheable>& value,
                   const std::shared_ptr<Serializable>& aCallbackArgument =
                       nullptr) override;
  bool localRemoveEx(const std::shared_ptr<CacheableKey>& key,
                     const std::shared_ptr<Serializable>& aCallbackArgument =
                         nullptr) override;
  std::vector<std::shared_ptr<CacheableKey>> keys() override;
  std::vector<std::shared_ptr<CacheableKey>> serverKeys() override;
  std::vector<std::shared_ptr<Cacheable>> values() override;
  std::vector<std::shared_ptr<RegionEntry>> entries(bool recursive) override;

  HashMapOfCacheable getAll(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      const std::shared_ptr<Serializable>& aCallbackArgument =
          nullptr) override;

  HashMapOfCacheable getAll_internal(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      bool addToLocalCache) override;

  void putAll(const HashMapOfCacheable& map,
              std::chrono::milliseconds timeout = DEFAULT_RESPONSE_TIMEOUT,
              const std::shared_ptr<Serializable>& aCallbackArgument =
                  nullptr) override;
  void removeAll(const std::vector<std::shared_ptr<CacheableKey>>& keys,
                 const std::shared_ptr<Serializable>& aCallbackArgument =
                     nullptr) override;
  uint32_t size() override;
  virtual uint32_t size_remote();
  RegionService& getRegionService() const override;
  virtual bool containsValueForKey_remote(
      const std::shared_ptr<CacheableKey>& keyPtr) const;
  bool containsValueForKey(
      const std::shared_ptr<CacheableKey>& keyPtr) const override;
  bool containsKey(const std::shared_ptr<CacheableKey>& keyPtr) const override;
  virtual bool containsKeyOnServer(
      const std::shared_ptr<CacheableKey>& keyPtr) const override;
  std::vector<std::shared_ptr<CacheableKey>> getInterestList() const override;
  std::vector<std::shared_ptr<CacheableString>> getInterestListRegex()
      const override;

  /** @brief Public Methods from RegionInternal
   *  There are all virtual methods
   */
  std::shared_ptr<PersistenceManager> getPersistenceManager() override {
    return m_persistenceManager;
  }
  void setPersistenceManager(
      std::shared_ptr<PersistenceManager>& pmPtr) override;

  GfErrType getNoThrow(
      const std::shared_ptr<CacheableKey>& key,
      std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument) override;
  GfErrType getAllNoThrow(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      const std::shared_ptr<HashMapOfCacheable>& values,
      const std::shared_ptr<HashMapOfException>& exceptions,
      const bool addToLocalCache,
      const std::shared_ptr<Serializable>& aCallbackArgument =
          nullptr) override;
  GfErrType putNoThrow(const std::shared_ptr<CacheableKey>& key,
                       const std::shared_ptr<Cacheable>& value,
                       const std::shared_ptr<Serializable>& aCallbackArgument,
                       std::shared_ptr<Cacheable>& oldValue, int updateCount,
                       const CacheEventFlags eventFlags,
                       std::shared_ptr<VersionTag> versionTag,
                       DataInput* delta = nullptr,
                       std::shared_ptr<EventId> eventId = nullptr) override;
  GfErrType putNoThrowTX(const std::shared_ptr<CacheableKey>& key,
                         const std::shared_ptr<Cacheable>& value,
                         const std::shared_ptr<Serializable>& aCallbackArgument,
                         std::shared_ptr<Cacheable>& oldValue, int updateCount,
                         const CacheEventFlags eventFlags,
                         std::shared_ptr<VersionTag> versionTag,
                         DataInput* delta = nullptr,
                         std::shared_ptr<EventId> eventId = nullptr);
  GfErrType createNoThrow(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
      const CacheEventFlags eventFlags,
      std::shared_ptr<VersionTag> versionTag) override;
  GfErrType destroyNoThrow(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
      const CacheEventFlags eventFlags,
      std::shared_ptr<VersionTag> versionTag) override;
  virtual GfErrType destroyNoThrowTX(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
      const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag);
  GfErrType removeNoThrow(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
      const CacheEventFlags eventFlags,
      std::shared_ptr<VersionTag> versionTag) override;
  virtual GfErrType removeNoThrowEx(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
      const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag);
  virtual GfErrType putAllNoThrow(
      const HashMapOfCacheable& map,
      std::chrono::milliseconds timeout = DEFAULT_RESPONSE_TIMEOUT,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  virtual GfErrType removeAllNoThrow(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  GfErrType invalidateNoThrow(
      const std::shared_ptr<CacheableKey>& keyPtr,
      const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
      const CacheEventFlags eventFlags,
      std::shared_ptr<VersionTag> versionTag) override;
  virtual GfErrType invalidateNoThrowTX(
      const std::shared_ptr<CacheableKey>& keyPtr,
      const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
      const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag);
  GfErrType invalidateRegionNoThrow(
      const std::shared_ptr<Serializable>& aCallbackArgument,
      const CacheEventFlags eventFlags) override;
  GfErrType destroyRegionNoThrow(
      const std::shared_ptr<Serializable>& aCallbackArgument,
      bool removeFromParent, const CacheEventFlags eventFlags) override;
  void tombstoneOperationNoThrow(
      const std::shared_ptr<CacheableHashMap>& tombstoneVersions,
      const std::shared_ptr<CacheableHashSet>& tombstoneKeys);

  //  moved putLocal to public since this is used by a few other
  // classes like CacheableObjectPartList now
  /** put an entry in local cache without invoking any callbacks */
  GfErrType putLocal(const std::string& name, bool isCreate,
                     const std::shared_ptr<CacheableKey>& keyPtr,
                     const std::shared_ptr<Cacheable>& valuePtr,
                     std::shared_ptr<Cacheable>& oldValue, bool cachingEnabled,
                     int updateCount, int destroyTracker,
                     std::shared_ptr<VersionTag> versionTag,
                     DataInput* delta = nullptr,
                     std::shared_ptr<EventId> eventId = nullptr);
  GfErrType invalidateLocal(const std::string& name,
                            const std::shared_ptr<CacheableKey>& keyPtr,
                            const std::shared_ptr<Cacheable>& value,
                            const CacheEventFlags eventFlags,
                            std::shared_ptr<VersionTag> versionTag);

  void setRegionExpiryTask() override;
  void acquireReadLock() override { m_rwLock.acquire_read(); }
  void releaseReadLock() override { m_rwLock.release(); }

  // behaviors for attributes mutator
  uint32_t adjustLruEntriesLimit(uint32_t limit) override;
  ExpirationAction adjustRegionExpiryAction(ExpirationAction action) override;
  ExpirationAction adjustEntryExpiryAction(ExpirationAction action) override;
  std::chrono::seconds adjustRegionExpiryDuration(
      const std::chrono::seconds& duration) override;
  std::chrono::seconds adjustEntryExpiryDuration(
      const std::chrono::seconds& duration) override;

  // other public methods
  RegionStats* getRegionStats() override { return m_regionStats; }

  bool cacheEnabled() override {
    return m_regionAttributes.getCachingEnabled();
  }

  inline bool cachelessWithListener() {
    return !m_regionAttributes.getCachingEnabled() && (m_listener != nullptr);
  }

  bool isDestroyed() const override { return m_destroyPending; }
  /* above public methods are inherited from RegionInternal */

  void adjustCacheListener(
      const std::shared_ptr<CacheListener>& aListener) override;

  void adjustCacheListener(const std::string& libpath,
                           const std::string& factoryFuncName) override;
  void adjustCacheLoader(const std::shared_ptr<CacheLoader>& aLoader) override;

  void adjustCacheLoader(const std::string& libpath,
                         const std::string& factoryFuncName) override;
  void adjustCacheWriter(const std::shared_ptr<CacheWriter>& aWriter) override;

  void adjustCacheWriter(const std::string& libpath,
                         const std::string& factoryFuncName) override;
  CacheImpl* getCacheImpl() const override;

  void evict(float percentage) override;

  virtual void acquireGlobals(bool isFailover);

  virtual void releaseGlobals(bool isFailover);

  virtual bool getProcessedMarker() { return true; }

  EntriesMap* getEntryMap() { return m_entries; }

  std::shared_ptr<TombstoneList> getTombstoneList() override;

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
      std::chrono::milliseconds timeout,
      const std::shared_ptr<Serializable>& aCallbackArgument);
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
  const std::shared_ptr<Pool>& getPool() const override {
    return m_attachedPool;
  }

  void setPool(const std::shared_ptr<Pool>& p) { m_attachedPool = p; }

  TXState* getTXState() const { return TSSTXStateWrapper::get().getTXState(); }

  std::shared_ptr<Cacheable> handleReplay(
      GfErrType& err, std::shared_ptr<Cacheable> value) const;

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
  synchronized_map<std::unordered_map<std::string, std::shared_ptr<Region>>,
                   std::recursive_mutex>
      m_subRegions;
  std::string m_fullPath;
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
  void removeRegion(const std::string& name);

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
  void updateAccessAndModifiedTimeForEntry(std::shared_ptr<MapEntryImpl>& ptr,
                                           bool modified) override;
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
  ExpirationAction getEntryExpirationAction() const;
  ExpirationAction getRegionExpiryAction() const;
  std::chrono::seconds getRegionExpiryDuration() const;
  std::chrono::seconds getEntryExpiryDuration() const;
  void invokeAfterAllEndPointDisconnected();

  virtual GfErrType getNoThrow_FullObject(
      std::shared_ptr<EventId> eventId, std::shared_ptr<Cacheable>& fullObject,
      std::shared_ptr<VersionTag>& versionTag);

  void clearKeysOfInterest(
      const std::unordered_map<std::shared_ptr<CacheableKey>,
                               InterestResultPolicy>& interest_list);
  void clearKeysOfInterestRegex(const std::string& regex);
  void clearKeysOfInterestRegex(
      const std::unordered_map<std::string, InterestResultPolicy>&
          interest_list);

 private:
  std::shared_ptr<Region> findSubRegion(const std::string& name);
  GfErrType invalidateRegionNoThrowOnSubRegions(
      const std::shared_ptr<Serializable>& aCallbackArgument,
      const CacheEventFlags eventFlags);

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
