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

#ifndef GEODE_CONCURRENTENTRIESMAP_H_
#define GEODE_CONCURRENTENTRIESMAP_H_
#include <atomic>

#include <geode/RegionEntry.hpp>
#include <geode/internal/geode_globals.hpp>

#include "EntriesMap.hpp"
#include "ExpMapEntry.hpp"
#include "MapSegment.hpp"

namespace apache {
namespace geode {
namespace client {
class RegionInternal;

/**
 * @brief Concurrent entries map.
 */
class APACHE_GEODE_EXPORT ConcurrentEntriesMap : public EntriesMap {
 protected:
  ExpiryTaskManager* m_expiryTaskManager;
  uint8_t m_concurrency;
  MapSegment* m_segments;
  std::atomic<uint32_t> m_size;
  RegionInternal* m_region;
  std::atomic<int32_t> m_numDestroyTrackers;
  bool m_concurrencyChecksEnabled;
  // TODO:  hashcode() is invoked 3-4 times -- need a better
  // implementation (STLport hash_map?) that will invoke it only once
  /**
   * Return a reference to the segment for which the given key would
   * be stored.
   */
  virtual MapSegment* segmentFor(
      const std::shared_ptr<CacheableKey>& key) const;

  /**
   * Return the segment index number for the given key.
   */
  int segmentIdx(const std::shared_ptr<CacheableKey>& key) const;

  /**
   * Return the segment index number for the given hash.
   */
  int segmentIdx(uint32_t hash) const;

 public:
  /**
   * @brief constructor, must call open before using map.
   */
  ConcurrentEntriesMap(ExpiryTaskManager* expiryTaskManager,
                       std::unique_ptr<EntryFactory> entryFactory,
                       bool concurrencyChecksEnabled, RegionInternal* region,
                       uint8_t concurrency = 16);

  /**
   * Initialize segments with proper EntryFactory.
   */
  virtual void open(uint32_t initialCapacity);

  virtual void close();

  ~ConcurrentEntriesMap() override;

  virtual void clear();

  virtual GfErrType put(const std::shared_ptr<CacheableKey>& key,
                        const std::shared_ptr<Cacheable>& newValue,
                        std::shared_ptr<MapEntryImpl>& me,
                        std::shared_ptr<Cacheable>& oldValue, int updateCount,
                        int destroyTracker,
                        std::shared_ptr<VersionTag> versionTag,
                        bool& isUpdate = EntriesMap::boolVal,
                        DataInput* delta = nullptr);
  virtual GfErrType invalidate(const std::shared_ptr<CacheableKey>& key,
                               std::shared_ptr<MapEntryImpl>& me,
                               std::shared_ptr<Cacheable>& oldValue,
                               std::shared_ptr<VersionTag> versionTag);
  virtual GfErrType create(const std::shared_ptr<CacheableKey>& key,
                           const std::shared_ptr<Cacheable>& newValue,
                           std::shared_ptr<MapEntryImpl>& me,
                           std::shared_ptr<Cacheable>& oldValue,
                           int updateCount, int destroyTracker,
                           std::shared_ptr<VersionTag> versionTag);
  virtual bool get(const std::shared_ptr<CacheableKey>& key,
                   std::shared_ptr<Cacheable>& value,
                   std::shared_ptr<MapEntryImpl>& me);

  /**
   * @brief get MapEntry for key.
   * TODO: return GfErrType like other methods
   */
  virtual void getEntry(const std::shared_ptr<CacheableKey>& key,
                        std::shared_ptr<MapEntryImpl>& result,
                        std::shared_ptr<Cacheable>& value) const;

  /**
   * @brief remove the entry for key from the map.
   */
  virtual GfErrType remove(const std::shared_ptr<CacheableKey>& key,
                           std::shared_ptr<Cacheable>& result,
                           std::shared_ptr<MapEntryImpl>& me, int updateCount,
                           std::shared_ptr<VersionTag> versionTag,
                           bool afterRemote);

  /**
   * @brief return true if there exists an entry for the key.
   */
  virtual bool containsKey(const std::shared_ptr<CacheableKey>& key) const;

  /**
   * @brief return the all the keys in a list.
   */
  virtual void getKeys(
      std::vector<std::shared_ptr<CacheableKey>>& result) const;

  /**
   * @brief return all the entries in a list.
   */
  virtual void getEntries(
      std::vector<std::shared_ptr<RegionEntry>>& result) const;

  /**
   * @brief return all values in a list.
   */
  virtual void getValues(std::vector<std::shared_ptr<Cacheable>>& result) const;

  /**
   * @brief return the number of entries in the map.
   */
  virtual uint32_t size() const;

  virtual int addTrackerForEntry(const std::shared_ptr<CacheableKey>& key,
                                 std::shared_ptr<Cacheable>& oldValue,
                                 bool addIfAbsent, bool failIfPresent,
                                 bool incUpdateCount);

  virtual void removeTrackerForEntry(const std::shared_ptr<CacheableKey>& key);

  virtual int addTrackerForAllEntries(MapOfUpdateCounters& updateCounterMap,
                                      bool addDestroyTracking);

  virtual void removeDestroyTracking();
  virtual void reapTombstones(std::map<uint16_t, int64_t>& gcVersions);

  virtual void reapTombstones(std::shared_ptr<CacheableHashSet> removedKeys);

  /**
   * for internal testing, returns if an entry is a tombstone
   */
  virtual GfErrType isTombstone(std::shared_ptr<CacheableKey>& key,
                                std::shared_ptr<MapEntryImpl>& me,
                                bool& result);

  /**
   * for internal testing, return the number of times any segment
   * has rehashed.
   */
  uint32_t totalSegmentRehashes() const;
};  // class EntriesMap
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CONCURRENTENTRIESMAP_H_
