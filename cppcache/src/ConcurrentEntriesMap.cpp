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
#include "ConcurrentEntriesMap.hpp"

#include <algorithm>

#include "RegionInternal.hpp"
#include "TableOfPrimes.hpp"

namespace apache {
namespace geode {
namespace client {

bool EntriesMap::boolVal = false;

ConcurrentEntriesMap::ConcurrentEntriesMap(
    ExpiryTaskManager* expiryTaskManager,
    std::unique_ptr<EntryFactory> entryFactory, bool concurrencyChecksEnabled,
    RegionInternal* region, uint8_t concurrency)
    : EntriesMap(std::move(entryFactory)),
      m_expiryTaskManager(expiryTaskManager),
      m_concurrency(0),
      m_segments(nullptr),
      m_size(0),
      m_region(region),
      m_numDestroyTrackers(0),
      m_concurrencyChecksEnabled(concurrencyChecksEnabled) {
  uint8_t maxConcurrency = TableOfPrimes::getMaxPrimeForConcurrency();
  if (concurrency > maxConcurrency) {
    m_concurrency = maxConcurrency;
  } else {
    m_concurrency = TableOfPrimes::nextLargerPrimeForConcurrency(concurrency);
  }
}

void ConcurrentEntriesMap::open(uint32_t initialCapacity) {
  uint32_t segSize = 1 + (initialCapacity - 1) / m_concurrency;
  m_segments = new MapSegment[m_concurrency];
  for (int index = 0; index < m_concurrency; ++index) {
    m_segments[index].open(m_region, getEntryFactory(), m_expiryTaskManager,
                           segSize, &m_numDestroyTrackers,
                           m_concurrencyChecksEnabled);
  }
}

void ConcurrentEntriesMap::close() {
  for (int index = 0; index < m_concurrency; ++index) {
    m_segments[index].close();
  }
}
void ConcurrentEntriesMap::clear() {
  for (uint32_t index = 0; index < m_concurrency; index++) {
    m_segments[index].clear();
  }
  m_size = 0;
}

ConcurrentEntriesMap::~ConcurrentEntriesMap() { delete[] m_segments; }

GfErrType ConcurrentEntriesMap::create(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Cacheable>& newValue,
    std::shared_ptr<MapEntryImpl>& me, std::shared_ptr<Cacheable>& oldValue,
    int updateCount, int destroyTracker,
    std::shared_ptr<VersionTag> versionTag) {
  GfErrType err = GF_NOERR;
  if ((err = segmentFor(key)->create(key, newValue, me, oldValue, updateCount,
                                     destroyTracker, versionTag)) == GF_NOERR &&
      oldValue == nullptr) {
    ++m_size;
  }
  return err;
}

GfErrType ConcurrentEntriesMap::invalidate(
    const std::shared_ptr<CacheableKey>& key, std::shared_ptr<MapEntryImpl>& me,
    std::shared_ptr<Cacheable>& oldValue,
    std::shared_ptr<VersionTag> versionTag) {
  bool isTokenAdded = false;
  GfErrType err =
      segmentFor(key)->invalidate(key, me, oldValue, versionTag, isTokenAdded);
  if (isTokenAdded) {
    ++m_size;
  }
  return err;
}

GfErrType ConcurrentEntriesMap::put(const std::shared_ptr<CacheableKey>& key,
                                    const std::shared_ptr<Cacheable>& newValue,
                                    std::shared_ptr<MapEntryImpl>& me,
                                    std::shared_ptr<Cacheable>& oldValue,
                                    int updateCount, int destroyTracker,
                                    std::shared_ptr<VersionTag> versionTag,
                                    bool& isUpdate, DataInput* delta) {
  GfErrType err = GF_NOERR;
  if ((err = segmentFor(key)->put(key, newValue, me, oldValue, updateCount,
                                  destroyTracker, isUpdate, versionTag,
                                  delta)) != GF_NOERR) {
    return err;
  }
  if (!isUpdate) {
    ++m_size;
  }
  return err;
}

bool ConcurrentEntriesMap::get(const std::shared_ptr<CacheableKey>& key,
                               std::shared_ptr<Cacheable>& value,
                               std::shared_ptr<MapEntryImpl>& me) {
  return segmentFor(key)->getEntry(key, me, value);
}

void ConcurrentEntriesMap::getEntry(const std::shared_ptr<CacheableKey>& key,
                                    std::shared_ptr<MapEntryImpl>& result,
                                    std::shared_ptr<Cacheable>& value) const {
  segmentFor(key)->getEntry(key, result, value);
}

GfErrType ConcurrentEntriesMap::remove(const std::shared_ptr<CacheableKey>& key,
                                       std::shared_ptr<Cacheable>& result,
                                       std::shared_ptr<MapEntryImpl>& me,
                                       int updateCount,
                                       std::shared_ptr<VersionTag> versionTag,
                                       bool afterRemote) {
  bool isEntryFound = true;
  GfErrType err;
  if ((err = segmentFor(key)->remove(key, result, me, updateCount, versionTag,
                                     afterRemote, isEntryFound)) == GF_NOERR) {
    //  decrement only if entry is present
    if (isEntryFound) --m_size;
  }
  return err;
}

bool ConcurrentEntriesMap::containsKey(
    const std::shared_ptr<CacheableKey>& key) const {
  //  MapSegment* segment = segmentFor( key );
  return segmentFor(key)->containsKey(key);
}

void ConcurrentEntriesMap::getKeys(
    std::vector<std::shared_ptr<CacheableKey>>& result) const {
  result.reserve(this->size());
  for (int index = 0; index < m_concurrency; ++index) {
    m_segments[index].getKeys(result);
  }
}

void ConcurrentEntriesMap::getEntries(
    std::vector<std::shared_ptr<RegionEntry>>& result) const {
  result.reserve(this->size());
  for (int index = 0; index < m_concurrency; ++index) {
    m_segments[index].getEntries(result);
  }
}

void ConcurrentEntriesMap::getValues(
    std::vector<std::shared_ptr<Cacheable>>& result) const {
  result.reserve(this->size());
  for (int index = 0; index < m_concurrency; ++index) {
    m_segments[index].getValues(result);
  }
}

bool ConcurrentEntriesMap::empty() const { return m_size == 0; }

uint32_t ConcurrentEntriesMap::size() const { return m_size; }

int ConcurrentEntriesMap::addTrackerForEntry(
    const std::shared_ptr<CacheableKey>& key,
    std::shared_ptr<Cacheable>& oldValue, bool addIfAbsent, bool failIfPresent,
    bool incUpdateCount) {
  // This function is disabled if concurrency checks are enabled. The versioning
  // changes takes care of the version and no need for tracking the entry
  if (m_concurrencyChecksEnabled) return -1;
  return segmentFor(key)->addTrackerForEntry(key, oldValue, addIfAbsent,
                                             failIfPresent, incUpdateCount);
}

void ConcurrentEntriesMap::removeTrackerForEntry(
    const std::shared_ptr<CacheableKey>& key) {
  // This function is disabled if concurrency checks are enabled. The versioning
  // changes takes care of the version and no need for tracking the entry
  if (m_concurrencyChecksEnabled) return;
  segmentFor(key)->removeTrackerForEntry(key);
}

int ConcurrentEntriesMap::addTrackerForAllEntries(
    MapOfUpdateCounters& updateCounterMap, bool addDestroyTracking) {
  // This function is disabled if concurrency checks are enabled. The versioning
  // changes takes care of the version and no need for tracking the entry
  if (m_concurrencyChecksEnabled) return -1;
  for (int index = 0; index < m_concurrency; ++index) {
    m_segments[index].addTrackerForAllEntries(updateCounterMap);
  }
  if (addDestroyTracking) {
    return ++m_numDestroyTrackers;
  }
  return 0;
}

void ConcurrentEntriesMap::removeDestroyTracking() {
  // This function is disabled if concurrency checks are enabled. The versioning
  // changes takes care of the version and no need for tracking the entry
  if (m_concurrencyChecksEnabled) return;
  if (--m_numDestroyTrackers == 0) {
    for (int index = 0; index < m_concurrency; ++index) {
      m_segments[index].removeDestroyTracking();
    }
  }
}

/**
 * @brief return the number of times any segment has rehashed.
 */
uint32_t ConcurrentEntriesMap::totalSegmentRehashes() const {
  uint32_t result = 0;
  for (int index = 0; index < m_concurrency; ++index) {
    result += m_segments[index].rehashCount();
  }
  return result;
}
void ConcurrentEntriesMap::reapTombstones(
    std::map<uint16_t, int64_t>& gcVersions) {
  for (int index = 0; index < m_concurrency; ++index) {
    m_segments[index].reapTombstones(gcVersions);
  }
}
void ConcurrentEntriesMap::reapTombstones(
    std::shared_ptr<CacheableHashSet> removedKeys) {
  for (int index = 0; index < m_concurrency; ++index) {
    m_segments[index].reapTombstones(removedKeys);
  }
}
GfErrType ConcurrentEntriesMap::isTombstone(std::shared_ptr<CacheableKey>& key,
                                            std::shared_ptr<MapEntryImpl>& me,
                                            bool& result) {
  return segmentFor(key)->isTombstone(key, me, result);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
