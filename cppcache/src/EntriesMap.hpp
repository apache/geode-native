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

#ifndef GEODE_ENTRIESMAP_H_
#define GEODE_ENTRIESMAP_H_

// This needs to be ace free so that the region can include it.

#include <memory>

#include <geode/CacheableKey.hpp>
#include <geode/RegionEntry.hpp>
#include <geode/internal/geode_globals.hpp>

#include "MapEntry.hpp"
#include "MapSegment.hpp"

namespace apache {
namespace geode {
namespace client {

#define SYNCHRONIZE_SEGMENT_FOR_KEY(keyPtr) \
  SegmentMutexGuard _segment_guard(((EntriesMap*)m_entries)->segmentFor(keyPtr))

/**
 * @brief Concurrent entries map. Not designed for subclassing...
 */
class EntriesMap {
 public:
  explicit EntriesMap(std::unique_ptr<EntryFactory> entryFactory)
      : m_entryFactory(std::move(entryFactory)) {}
  virtual ~EntriesMap() {}

  /**
   * Initialize segments with proper EntryFactory.
   */
  virtual void open(uint32_t initialCapacity = 5000) = 0;

  /** @brief Close the map. */
  virtual void close() = 0;

  /**
   * @brief put a value in the map, replacing if key already exists.
   */
  virtual GfErrType put(const std::shared_ptr<CacheableKey>& key,
                        const std::shared_ptr<Cacheable>& newValue,
                        std::shared_ptr<MapEntryImpl>& me,
                        std::shared_ptr<Cacheable>& oldValue, int updateCount,
                        int destroyTracker,
                        std::shared_ptr<VersionTag> versionTag,
                        bool& isUpdate = EntriesMap::boolVal,
                        DataInput* delta = nullptr) = 0;
  virtual GfErrType invalidate(const std::shared_ptr<CacheableKey>& key,
                               std::shared_ptr<MapEntryImpl>& me,
                               std::shared_ptr<Cacheable>& oldValue,
                               std::shared_ptr<VersionTag> versionTag) = 0;
  virtual GfErrType create(const std::shared_ptr<CacheableKey>& key,
                           const std::shared_ptr<Cacheable>& newValue,
                           std::shared_ptr<MapEntryImpl>& me,
                           std::shared_ptr<Cacheable>& oldValue,
                           int updateCount, int destroyTracker,
                           std::shared_ptr<VersionTag> versionTag) = 0;

  /**
   * @brief get a value out of the map; returns false if absent
   */
  virtual bool get(const std::shared_ptr<CacheableKey>& key,
                   std::shared_ptr<Cacheable>& value,
                   std::shared_ptr<MapEntryImpl>& me) = 0;

  /**
   * @brief get MapEntry for key; returns nullptr if absent
   */
  virtual void getEntry(const std::shared_ptr<CacheableKey>& key,
                        std::shared_ptr<MapEntryImpl>& result,
                        std::shared_ptr<Cacheable>& value) const = 0;
  /** @brief remove all entries in the map. */
  virtual void clear() = 0;

  /**
   * @brief remove the entry for key from the map;
   *   returns false and nullptr MapEntry if absent
   */
  virtual GfErrType remove(const std::shared_ptr<CacheableKey>& key,
                           std::shared_ptr<Cacheable>& result,
                           std::shared_ptr<MapEntryImpl>& me, int updateCount,
                           std::shared_ptr<VersionTag> versionTag,
                           bool afterRemote) = 0;

  /**
   * @brief return true if there exists an entry for the key.
   */
  virtual bool containsKey(const std::shared_ptr<CacheableKey>& key) const = 0;

  /**
   * @brief return the all the keys in a vector.
   */
  virtual void getKeys(
      std::vector<std::shared_ptr<CacheableKey>>& result) const = 0;

  /**
   * @brief return all the entries in a vector.
   */
  virtual void getEntries(
      std::vector<std::shared_ptr<RegionEntry>>& result) const = 0;

  /**
   * @brief return all values in a vector.
   */
  virtual void getValues(
      std::vector<std::shared_ptr<Cacheable>>& result) const = 0;

  /** @brief return whether there are no entryies. */
  virtual bool empty() const = 0;

  /** @brief return the number of entries in the map. */
  virtual uint32_t size() const = 0;

  /**
   * Add a watch for updates for the given entry. If the entry is present in
   * the cache then the current update counter for the entry is returned,
   * else a new entry is created with destroyed token and zero is returned.
   * A failure value of -1 is returned if <addIfAbsent> is false and an
   * existing entry is not found or <failIfPresent> parameter is true
   * and an existing entry is found.
   * The parameter <incUpdateCount> should be set to true if this is a write
   * operation and update counter sould be incremented for the entry.
   */
  virtual int addTrackerForEntry(const std::shared_ptr<CacheableKey>& key,
                                 std::shared_ptr<Cacheable>& oldValue,
                                 bool addIfAbsent, bool failIfPresent,
                                 bool incUpdateCount) = 0;

  virtual void removeTrackerForEntry(
      const std::shared_ptr<CacheableKey>& key) = 0;

  /**
   * Add trackers for all the entries and returns the map of keys to
   * tracking number. The "addDestroyTracking" flag also adds tracking
   * for any destroys (e.g. through notification) to the map so that
   * subsequent creates do not overwrite the destroys.
   */
  virtual int addTrackerForAllEntries(MapOfUpdateCounters& updateCounterMap,
                                      bool addDestroyTracking) = 0;

  /**
   * Decrement destroy tracking flag for this map by one.
   */
  virtual void removeDestroyTracking() = 0;

  /**
   * @brief The MapSegment can be used to protect against concurrency...
   * SYNCHRONIZE_SEGMENT_FOR_KEY( keyPtr ) creates a guard that locks
   * and release the segment.
   */
  virtual MapSegment* segmentFor(
      const std::shared_ptr<CacheableKey>& key) const = 0;

  virtual std::shared_ptr<Cacheable> getFromDisk(
      const std::shared_ptr<CacheableKey>& key,
      std::shared_ptr<MapEntryImpl>& me) const;

  virtual void reapTombstones(std::map<uint16_t, int64_t>& gcVersions) = 0;

  virtual void reapTombstones(
      std::shared_ptr<CacheableHashSet> removedKeys) = 0;

  /**
   * for internal testing, returns if an entry is a tombstone
   */
  virtual GfErrType isTombstone(std::shared_ptr<CacheableKey>& key,
                                std::shared_ptr<MapEntryImpl>& me,
                                bool& result) = 0;

  static bool boolVal;

 protected:
  const std::unique_ptr<EntryFactory> m_entryFactory;

  /** @brief return the instance of EntryFactory for the segments to use. */
  inline const EntryFactory* getEntryFactory() const {
    return m_entryFactory.get();
  }

};  // class EntriesMap

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_ENTRIESMAP_H_
