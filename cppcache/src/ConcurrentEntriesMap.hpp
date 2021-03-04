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
class ConcurrentEntriesMap : public EntriesMap {
 protected:
  ExpiryTaskManager* m_expiryTaskManager;
  uint8_t m_concurrency;
  MapSegment* m_segments;
  std::atomic<uint32_t> m_size;
  RegionInternal* m_region;
  std::atomic<int32_t> m_numDestroyTrackers;
  bool m_concurrencyChecksEnabled;

  /**
   * Return a reference to the segment for which the given key would
   * be stored.
   */
  MapSegment* segmentFor(
      const std::shared_ptr<CacheableKey>& key) const override {
    return &(m_segments[segmentIdx(key)]);
  }

  /**
   * Return the segment index number for the given key.
   */
  inline int segmentIdx(const std::shared_ptr<CacheableKey>& key) const {
    return segmentIdx(key->hashcode());
  }

  /**
   * Return the segment index number for the given hash.
   */
  inline int segmentIdx(uint32_t hash) const { return (hash % m_concurrency); }

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
  void open(uint32_t initialCapacity) override;

  void close() override;

  ~ConcurrentEntriesMap() noexcept override;

  void clear() override;

  GfErrType put(const std::shared_ptr<CacheableKey>& key,
                const std::shared_ptr<Cacheable>& newValue,
                std::shared_ptr<MapEntryImpl>& me,
                std::shared_ptr<Cacheable>& oldValue, int updateCount,
                int destroyTracker, std::shared_ptr<VersionTag> versionTag,
                bool& isUpdate = EntriesMap::boolVal,
                DataInput* delta = nullptr) override;
  GfErrType invalidate(const std::shared_ptr<CacheableKey>& key,
                       std::shared_ptr<MapEntryImpl>& me,
                       std::shared_ptr<Cacheable>& oldValue,
                       std::shared_ptr<VersionTag> versionTag) override;
  GfErrType create(const std::shared_ptr<CacheableKey>& key,
                   const std::shared_ptr<Cacheable>& newValue,
                   std::shared_ptr<MapEntryImpl>& me,
                   std::shared_ptr<Cacheable>& oldValue, int updateCount,
                   int destroyTracker,
                   std::shared_ptr<VersionTag> versionTag) override;
  bool get(const std::shared_ptr<CacheableKey>& key,
           std::shared_ptr<Cacheable>& value,
           std::shared_ptr<MapEntryImpl>& me) override;

  /**
   * @brief get MapEntry for key.
   * TODO: return GfErrType like other methods
   */
  void getEntry(const std::shared_ptr<CacheableKey>& key,
                std::shared_ptr<MapEntryImpl>& result,
                std::shared_ptr<Cacheable>& value) const override;

  /**
   * @brief remove the entry for key from the map.
   */
  GfErrType remove(const std::shared_ptr<CacheableKey>& key,
                   std::shared_ptr<Cacheable>& result,
                   std::shared_ptr<MapEntryImpl>& me, int updateCount,
                   std::shared_ptr<VersionTag> versionTag,
                   bool afterRemote) override;

  /**
   * @brief return true if there exists an entry for the key.
   */
  bool containsKey(const std::shared_ptr<CacheableKey>& key) const override;

  /**
   * @brief return the all the keys in a list.
   */
  void getKeys(
      std::vector<std::shared_ptr<CacheableKey>>& result) const override;

  /**
   * @brief return all the entries in a list.
   */
  void getEntries(
      std::vector<std::shared_ptr<RegionEntry>>& result) const override;

  /**
   * @brief return all values in a list.
   */
  void getValues(
      std::vector<std::shared_ptr<Cacheable>>& result) const override;

  /**
   * @brief return whether there are no entries.
   */
  bool empty() const override;

  /**
   * @brief return the number of entries in the map.
   */
  uint32_t size() const override;

  int addTrackerForEntry(const std::shared_ptr<CacheableKey>& key,
                         std::shared_ptr<Cacheable>& oldValue, bool addIfAbsent,
                         bool failIfPresent, bool incUpdateCount) override;

  void removeTrackerForEntry(const std::shared_ptr<CacheableKey>& key) override;

  int addTrackerForAllEntries(MapOfUpdateCounters& updateCounterMap,
                              bool addDestroyTracking) override;

  void removeDestroyTracking() override;
  void reapTombstones(std::map<uint16_t, int64_t>& gcVersions) override;

  void reapTombstones(std::shared_ptr<CacheableHashSet> removedKeys) override;

  /**
   * for internal testing, returns if an entry is a tombstone
   */
  GfErrType isTombstone(std::shared_ptr<CacheableKey>& key,
                        std::shared_ptr<MapEntryImpl>& me,
                        bool& result) override;

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
