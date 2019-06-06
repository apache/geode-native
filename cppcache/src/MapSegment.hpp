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

#ifndef GEODE_MAPSEGMENT_H_
#define GEODE_MAPSEGMENT_H_

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include <geode/CacheableKey.hpp>
#include <geode/Delta.hpp>
#include <geode/RegionEntry.hpp>
#include <geode/internal/geode_globals.hpp>

#include "CacheableToken.hpp"
#include "MapEntry.hpp"
#include "MapWithLock.hpp"
#include "TombstoneList.hpp"
#include "util/concurrent/spinlock_mutex.hpp"

namespace apache {
namespace geode {
namespace client {

class RegionInternal;
typedef std::unordered_map<std::shared_ptr<CacheableKey>,
                           std::shared_ptr<MapEntry>,
                           dereference_hash<std::shared_ptr<CacheableKey>>,
                           dereference_equal_to<std::shared_ptr<CacheableKey>>>
    CacheableKeyHashMap;

/** @brief type wrapper around the std::unordered_map implementation. */
class APACHE_GEODE_EXPORT MapSegment {
 private:
  // contain
  CacheableKeyHashMap* m_map;
  // refers to object managed by the entries map...
  // does not need deletion here.
  const EntryFactory* m_entryFactory;
  RegionInternal* m_region;
  ExpiryTaskManager* m_expiryTaskManager;

  // index of the current prime in the primes table
  uint32_t m_primeIndex;
  spinlock_mutex m_spinlock;
  std::recursive_mutex m_segmentMutex;

  bool m_concurrencyChecksEnabled;
  // number of operations that are tracking destroys
  // the m_destroyedKeys contains the set for which destroys
  // have been received as long as this number is greater than zero
  std::atomic<int32_t>* m_numDestroyTrackers;
  MapOfUpdateCounters m_destroyedKeys;

  uint32_t m_rehashCount;
  void rehash();
  std::shared_ptr<TombstoneList> m_tombstoneList;

  // increment update counter of the given entry and return true if entry
  // was rebound
  bool incrementUpdateCount(const std::shared_ptr<CacheableKey>& key,
                            std::shared_ptr<MapEntry>& entry);

  // remove a tracker for the given entry
  void removeTrackerForEntry(const std::shared_ptr<CacheableKey>& key,
                             std::shared_ptr<MapEntry>& entry,
                             std::shared_ptr<MapEntryImpl>& entryImpl);

  GfErrType putNoEntry(const std::shared_ptr<CacheableKey>& key,
                       const std::shared_ptr<Cacheable>& newValue,
                       std::shared_ptr<MapEntryImpl>& newEntry, int updateCount,
                       int destroyTracker,
                       std::shared_ptr<VersionTag> versionTag,
                       VersionStamp* versionStamp = nullptr);

  GfErrType putForTrackedEntry(const std::shared_ptr<CacheableKey>& key,
                               const std::shared_ptr<Cacheable>& newValue,
                               std::shared_ptr<MapEntry>& entry,
                               std::shared_ptr<MapEntryImpl>& entryImpl,
                               int updateCount, VersionStamp& versionStamp,
                               DataInput* delta = nullptr);

  std::shared_ptr<Cacheable> getFromDisc(
      std::shared_ptr<CacheableKey> key,
      std::shared_ptr<MapEntryImpl>& entryImpl);

  GfErrType removeWhenConcurrencyEnabled(
      const std::shared_ptr<CacheableKey>& key,
      std::shared_ptr<Cacheable>& oldValue, std::shared_ptr<MapEntryImpl>& me,
      int updateCount, std::shared_ptr<VersionTag> versionTag, bool afterRemote,
      bool& isEntryFound, ExpiryTaskManager::id_type expiryTaskID,
      TombstoneExpiryHandler* handler, bool& expTaskSet);

 public:
  MapSegment();

  ~MapSegment();

  // methods for BasicLockable
  void lock();
  void unlock();

  /**
   * @brief initialize underlying map structures. Not called by constructor.
   * Used when allocated in arrays by EntriesMap implementations.
   */
  void open(RegionInternal* region, const EntryFactory* entryFactory,
            ExpiryTaskManager* expiryTaskManager, uint32_t size,
            std::atomic<int32_t>* destroyTrackers,
            bool concurrencyChecksEnabled);

  void close();
  void clear();

  /**
   * @brief put a new value in the map, failing if key already exists.
   * return error code if key already existing or something goes wrong.
   */
  GfErrType create(const std::shared_ptr<CacheableKey>& key,
                   const std::shared_ptr<Cacheable>& newValue,
                   std::shared_ptr<MapEntryImpl>& me,
                   std::shared_ptr<Cacheable>& oldValue, int updateCount,
                   int destroyTracker, std::shared_ptr<VersionTag> versionTag);

  /**
   * @brief put a value in the map, replacing if key already exists.
   * return error code is something goes wrong.
   */
  GfErrType put(const std::shared_ptr<CacheableKey>& key,
                const std::shared_ptr<Cacheable>& newValue,
                std::shared_ptr<MapEntryImpl>& me,
                std::shared_ptr<Cacheable>& oldValue, int updateCount,
                int destroyTracker, bool& isUpdate,
                std::shared_ptr<VersionTag> versionTag,
                DataInput* delta = nullptr);

  GfErrType invalidate(const std::shared_ptr<CacheableKey>& key,
                       std::shared_ptr<MapEntryImpl>& me,
                       std::shared_ptr<Cacheable>& oldValue,
                       std::shared_ptr<VersionTag> versionTag,
                       bool& isTokenAdded);

  /**
   * @brief remove an entry from the map, setting oldValue.
   */
  GfErrType remove(const std::shared_ptr<CacheableKey>& key,
                   std::shared_ptr<Cacheable>& oldValue,
                   std::shared_ptr<MapEntryImpl>& me, int updateCount,
                   std::shared_ptr<VersionTag> versionTag, bool afterRemote,
                   bool& isEntryFound);

  /**
   * @brief get a value and MapEntry out of the map.
   * return true if the key is present, false otherwise.
   */
  bool getEntry(const std::shared_ptr<CacheableKey>& key,
                std::shared_ptr<MapEntryImpl>& result,
                std::shared_ptr<Cacheable>& value);

  /**
   * @brief return true if there exists an entry for the key.
   */
  bool containsKey(const std::shared_ptr<CacheableKey>& key);

  /**
   * @brief return the all the keys in the provided list.
   */
  void getKeys(std::vector<std::shared_ptr<CacheableKey>>& result);

  /**
   * @brief return all the entries in the provided list.
   */
  void getEntries(std::vector<std::shared_ptr<RegionEntry>>& result);

  /**
   * @brief return all values in the provided list.
   */
  void getValues(std::vector<std::shared_ptr<Cacheable>>& result);

  inline uint32_t rehashCount() { return m_rehashCount; }

  int addTrackerForEntry(const std::shared_ptr<CacheableKey>& key,
                         std::shared_ptr<Cacheable>& oldValue, bool addIfAbsent,
                         bool failIfPresent, bool incUpdateCount);

  void removeTrackerForEntry(const std::shared_ptr<CacheableKey>& key);

  void addTrackerForAllEntries(MapOfUpdateCounters& updateCounterMap);

  void removeDestroyTracking();

  void reapTombstones(std::map<uint16_t, int64_t>& gcVersions);

  void reapTombstones(std::shared_ptr<CacheableHashSet> removedKeys);

  bool removeActualEntry(const std::shared_ptr<CacheableKey>& key,
                         bool cancelTask = true);

  bool unguardedRemoveActualEntryWithoutCancelTask(
      const std::shared_ptr<CacheableKey>& key,
      TombstoneExpiryHandler*& handler, ExpiryTaskManager::id_type& taskid);

  bool unguardedRemoveActualEntry(const std::shared_ptr<CacheableKey>& key,
                                  bool cancelTask = true);

  GfErrType isTombstone(std::shared_ptr<CacheableKey> key,
                        std::shared_ptr<MapEntryImpl>& me, bool& result);

  static bool boolVal;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_MAPSEGMENT_H_
