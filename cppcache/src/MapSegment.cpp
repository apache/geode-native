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

#include "MapSegment.hpp"

#include <chrono>

#include "MapEntry.hpp"
#include "RegionInternal.hpp"
#include "TableOfPrimes.hpp"
#include "ThinClientPoolDM.hpp"
#include "ThinClientRegion.hpp"
#include "TombstoneEntry.hpp"
#include "TombstoneExpiryTask.hpp"
#include "Utils.hpp"
#include "util/concurrent/spinlock_mutex.hpp"

namespace apache {
namespace geode {
namespace client {

bool MapSegment::boolVal = false;

void MapSegment::open(RegionInternal* region, const EntryFactory* entryFactory,
                      ExpiryTaskManager* expiryTaskManager, uint32_t size,
                      std::atomic<int32_t>* destroyTrackers,
                      bool concurrencyChecksEnabled) {
  uint32_t mapSize = TableOfPrimes::nextLargerPrime(size, m_primeIndex);
  LOG_FINER("Initializing MapSegment with size %d (given size %d).", mapSize,
            size);
  m_map.reserve(mapSize);
  m_entryFactory = entryFactory;
  m_region = region;
  m_tombstoneList =
      std::make_shared<TombstoneList>(*this, *m_region->getCacheImpl());
  expiry_manager_ = expiryTaskManager;
  m_numDestroyTrackers = destroyTrackers;
  m_concurrencyChecksEnabled = concurrencyChecksEnabled;
}

void MapSegment::close() {}

void MapSegment::clear() {
  std::lock_guard<decltype(m_spinlock)> lk(m_spinlock);
  m_map.clear();
}

void MapSegment::lock() { m_segmentMutex.lock(); }

void MapSegment::unlock() { m_segmentMutex.unlock(); }

GfErrType MapSegment::create(const std::shared_ptr<CacheableKey>& key,
                             const std::shared_ptr<Cacheable>& newValue,
                             std::shared_ptr<MapEntryImpl>& me,
                             std::shared_ptr<Cacheable>& oldValue,
                             int updateCount, int destroyTracker,
                             std::shared_ptr<VersionTag> versionTag) {
  GfErrType err = GF_NOERR;
  std::lock_guard<decltype(m_spinlock)> lk(m_spinlock);
  // if size is greater than 75 percent of prime, rehash
  auto mapSize = TableOfPrimes::getPrime(m_primeIndex);
  if (((m_map.size() * 75) / 100) > mapSize) {
    rehash();
  }

  const auto& find = m_map.find(key);
  if (find == m_map.end()) {
    if ((err = putNoEntry(key, newValue, me, updateCount, destroyTracker,
                          versionTag)) != GF_NOERR) {
      return err;
    }
  } else {
    auto& entry = find->second;
    auto entryImpl = entry->getImplPtr();
    entryImpl->getValueI(oldValue);
    if (oldValue == nullptr || CacheableToken::isTombstone(oldValue)) {
      // pass the version stamp
      VersionStamp versionStamp;
      if (m_concurrencyChecksEnabled) {
        versionStamp = entry->getVersionStamp();
        if (versionTag) {
          err =
              versionStamp.processVersionTag(m_region, key, versionTag, false);
          if (err != GF_NOERR) return err;
          versionStamp.setVersions(versionTag);
        }
      }
      // good case; go ahead with the create
      if (oldValue == nullptr) {
        err = putForTrackedEntry(key, newValue, entry, entryImpl, updateCount,
                                 versionStamp);
      } else {
        remove_entry(key);
        err = putNoEntry(key, newValue, me, updateCount, destroyTracker,
                         versionTag, &versionStamp);
      }

      oldValue = nullptr;

    } else {
      err = GF_CACHE_ENTRY_EXISTS;
    }
    if (err == GF_NOERR) {
      me = entryImpl;
    }
  }

  return err;
}

/**
 * @brief put a value in the map, replacing if key already exists.
 */
GfErrType MapSegment::put(const std::shared_ptr<CacheableKey>& key,
                          const std::shared_ptr<Cacheable>& newValue,
                          std::shared_ptr<MapEntryImpl>& me,
                          std::shared_ptr<Cacheable>& oldValue, int updateCount,
                          int destroyTracker, bool& isUpdate,
                          std::shared_ptr<VersionTag> versionTag,
                          DataInput* delta) {
  GfErrType err = GF_NOERR;
  std::lock_guard<decltype(m_spinlock)> lk(m_spinlock);
  // if size is greater than 75 percent of prime, rehash
  uint32_t mapSize = TableOfPrimes::getPrime(m_primeIndex);
  if (((m_map.size() * 75) / 100) > mapSize) {
    rehash();
  }

  const auto& find = m_map.find(key);
  if (find == m_map.end()) {
    if (delta != nullptr) {
      return GF_INVALID_DELTA;  // You can not apply delta when there is no
    }
    // entry hence ask for full object
    isUpdate = false;
    err =
        putNoEntry(key, newValue, me, updateCount, destroyTracker, versionTag);
  } else {
    auto& entry = find->second;
    auto entryImpl = entry->getImplPtr();
    std::shared_ptr<Cacheable> meOldValue;
    entryImpl->getValueI(meOldValue);
    // pass the version stamp
    VersionStamp versionStamp;
    if (m_concurrencyChecksEnabled) {
      versionStamp = entry->getVersionStamp();
      if (versionTag) {
        if (delta == nullptr) {
          err =
              versionStamp.processVersionTag(m_region, key, versionTag, false);
        } else {
          err = versionStamp.processVersionTag(m_region, key, versionTag, true);
        }

        if (err != GF_NOERR) return err;
        versionStamp.setVersions(versionTag);
      }
    }
    if (CacheableToken::isTombstone(meOldValue)) {
      remove_entry(key);
      err = putNoEntry(key, newValue, me, updateCount, destroyTracker,
                       versionTag, &versionStamp);
      meOldValue = nullptr;
      isUpdate = false;
    } else if ((err = putForTrackedEntry(key, newValue, entry, entryImpl,
                                         updateCount, versionStamp, delta)) ==
               GF_NOERR) {
      me = entryImpl;
      oldValue = meOldValue;
      isUpdate = (meOldValue != nullptr);
    }
  }

  return err;
}

GfErrType MapSegment::invalidate(const std::shared_ptr<CacheableKey>& key,
                                 std::shared_ptr<MapEntryImpl>& me,
                                 std::shared_ptr<Cacheable>& oldValue,
                                 std::shared_ptr<VersionTag> versionTag,
                                 bool& isTokenAdded) {
  std::lock_guard<decltype(m_spinlock)> lk(m_spinlock);
  isTokenAdded = false;
  GfErrType err = GF_NOERR;

  const auto& find = m_map.find(key);
  if (find != m_map.end()) {
    auto entry = find->second;
    VersionStamp versionStamp;
    if (m_concurrencyChecksEnabled) {
      versionStamp = entry->getVersionStamp();
      if (versionTag) {
        err = versionStamp.processVersionTag(m_region, key, versionTag, false);
        if (err != GF_NOERR) return err;
        versionStamp.setVersions(versionTag);
      }
    }
    auto entryImpl = entry->getImplPtr();
    entryImpl->getValueI(oldValue);
    if (CacheableToken::isTombstone(oldValue)) {
      oldValue = nullptr;
      return GF_CACHE_ENTRY_NOT_FOUND;
    }
    entryImpl->setValueI(CacheableToken::invalid());
    if (m_concurrencyChecksEnabled) {
      entryImpl->getVersionStamp().setVersions(versionStamp);
    }
    (void)incrementUpdateCount(key, entry);
    if (oldValue != nullptr) {
      me = entryImpl;
    }
  } else {
    // create new entry for the key if concurrencychecksEnabled is true
    if (m_concurrencyChecksEnabled) {
      if ((err = putNoEntry(key, CacheableToken::invalid(), me, -1, -1,
                            versionTag)) != GF_NOERR) {
        return err;
      }
      isTokenAdded = true;
    }
    err = GF_CACHE_ENTRY_NOT_FOUND;
  }
  return err;
}

GfErrType MapSegment::removeWhenConcurrencyEnabled(
    const std::shared_ptr<CacheableKey>& key,
    std::shared_ptr<Cacheable>& oldValue, std::shared_ptr<MapEntryImpl>& me,
    int updateCount, std::shared_ptr<VersionTag> versionTag, bool afterRemote,
    bool& isEntryFound) {
  GfErrType err = GF_NOERR;
  VersionStamp versionStamp;
  // If entry found, else return no entry
  const auto& find = m_map.find(key);
  if (find != m_map.end()) {
    auto entry = find->second;
    isEntryFound = true;
    // If the version tag is null, use the version tag of
    // the existing entry
    versionStamp = entry->getVersionStamp();
    if (versionTag) {
      std::shared_ptr<CacheableKey> keyPtr;
      entry->getImplPtr()->getKeyI(keyPtr);
      if ((err = entry->getVersionStamp().processVersionTag(
               m_region, keyPtr, versionTag, false)) != GF_NOERR) {
        return err;
      }
      versionStamp.setVersions(versionTag);
    }
    // Get the old value for returning
    auto entryImpl = entry->getImplPtr();
    entryImpl->getValueI(oldValue);

    if (oldValue) me = entryImpl;

    if ((err = putForTrackedEntry(key, CacheableToken::tombstone(), entry,
                                  entryImpl, updateCount, versionStamp)) ==
        GF_NOERR) {
      m_tombstoneList->add(entryImpl);
    }
    if (CacheableToken::isTombstone(oldValue)) {
      oldValue = nullptr;
      if (afterRemote) {
        return GF_NOERR;  // We are here because a remote op succeeded, no need
                          // to throw an error
      } else {
        return GF_CACHE_ENTRY_NOT_FOUND;
      }
    }
  } else {
    // If entry not found than add a tombstone for this entry
    // so that any future updates for this entry are checked for version
    // no entry
    if (versionTag) {
      std::shared_ptr<MapEntryImpl> mapEntry;
      putNoEntry(key, CacheableToken::tombstone(), mapEntry, -1, 0, versionTag);
      m_tombstoneList->add(mapEntry->getImplPtr());
    }
    oldValue = nullptr;
    isEntryFound = false;
    if (afterRemote) {
      err = GF_NOERR;  // We are here because a remote op succeeded, no need to
                       // throw an error
    } else {
      err = GF_CACHE_ENTRY_NOT_FOUND;
    }
  }
  return err;
}
/**
 * @brief remove entry, setting oldValue.
 */
GfErrType MapSegment::remove(const std::shared_ptr<CacheableKey>& key,
                             std::shared_ptr<Cacheable>& oldValue,
                             std::shared_ptr<MapEntryImpl>& me, int updateCount,
                             std::shared_ptr<VersionTag> versionTag,
                             bool afterRemote, bool& isEntryFound) {
  if (m_concurrencyChecksEnabled) {
    GfErrType err;
    {
      std::lock_guard<decltype(m_spinlock)> lk(m_spinlock);
      err = removeWhenConcurrencyEnabled(key, oldValue, me, updateCount,
                                         versionTag, afterRemote, isEntryFound);
    }

    return err;
  }

  std::lock_guard<decltype(m_spinlock)> lk(m_spinlock);

  auto&& iter = m_map.find(key);
  if (iter == m_map.end()) {
    // didn't unbind, probably no entry...
    oldValue = nullptr;
    volatile int destroyTrackers = *m_numDestroyTrackers;
    if (destroyTrackers > 0) {
      m_destroyedKeys[key] = destroyTrackers + 1;
    }
    return GF_CACHE_ENTRY_NOT_FOUND;
  }

  auto entry = iter->second;
  m_map.erase(iter);

  if (updateCount >= 0 && updateCount != entry->getUpdateCount()) {
    // this is the case when entry has been updated while being tracked
    return GF_CACHE_ENTRY_UPDATED;
  }

  auto entryImpl = entry->getImplPtr();
  entryImpl->getValueI(oldValue);
  if (CacheableToken::isTombstone(oldValue)) oldValue = nullptr;
  if (oldValue) {
    me = entryImpl;
  }

  return GF_NOERR;
}

void MapSegment::remove_entry(const std::shared_ptr<CacheableKey>& key) {
  m_tombstoneList->erase(key);
  m_map.erase(key);
}

bool MapSegment::remove_tomb_entry(
    const std::shared_ptr<TombstoneEntry>& entry) {
  std::lock_guard<decltype(m_spinlock)> lk(m_spinlock);
  if (!entry->valid()) {
    return false;
  }

  std::shared_ptr<CacheableKey> key;
  entry->entry()->getKeyI(key);

  if (!m_tombstoneList->erase(key, false)) {
    return false;
  }

  m_map.erase(key);
  return true;
}
/**
 * @brief get MapEntry for key. throws NoEntryException if absent.
 */
bool MapSegment::getEntry(const std::shared_ptr<CacheableKey>& key,
                          std::shared_ptr<MapEntryImpl>& result,
                          std::shared_ptr<Cacheable>& value) {
  std::lock_guard<decltype(m_spinlock)> lk(m_spinlock);

  const auto& find = m_map.find(key);
  if (find == m_map.end()) {
    result = nullptr;
    value = nullptr;
    return false;
  }
  auto entry = find->second;

  // If the value is a tombstone return not found
  auto mePtr = entry->getImplPtr();
  mePtr->getValueI(value);
  if (value == nullptr || CacheableToken::isTombstone(value)) {
    result = nullptr;
    value = nullptr;
    return false;
  }
  result = mePtr;
  return true;
}

/**
 * @brief return true if there exists an entry for the key.
 */
bool MapSegment::containsKey(const std::shared_ptr<CacheableKey>& key) {
  std::lock_guard<decltype(m_spinlock)> lk(m_spinlock);

  const auto& find = m_map.find(key);
  if (find == m_map.end()) {
    return false;
  }
  auto mePtr = find->second;

  // If the value is a tombstone return not found
  std::shared_ptr<Cacheable> value;
  auto mePtr1 = mePtr->getImplPtr();
  mePtr1->getValueI(value);
  if (value != nullptr && CacheableToken::isTombstone(value)) return false;

  return true;
}

/**
 * @brief return the all the keys in the provided list.
 */
void MapSegment::getKeys(std::vector<std::shared_ptr<CacheableKey>>& result) {
  std::lock_guard<decltype(m_spinlock)> lk(m_spinlock);

  for (const auto& kv : m_map) {
    std::shared_ptr<Cacheable> valuePtr;
    kv.second->getImplPtr()->getValueI(valuePtr);
    if (!CacheableToken::isTombstone(valuePtr)) {
      result.push_back(kv.first);
    }
  }
}

/**
 * @brief return all the entries in the provided list.
 */
void MapSegment::getEntries(std::vector<std::shared_ptr<RegionEntry>>& result) {
  std::lock_guard<decltype(m_spinlock)> lk(m_spinlock);

  for (const auto& kv : m_map) {
    std::shared_ptr<CacheableKey> keyPtr;
    std::shared_ptr<Cacheable> valuePtr;
    auto me = kv.second->getImplPtr();
    me->getValueI(valuePtr);
    if (valuePtr && !CacheableToken::isTombstone(valuePtr)) {
      if (CacheableToken::isInvalid(valuePtr)) {
        valuePtr = nullptr;
      }
      me->getKeyI(keyPtr);
      auto rePtr = m_region->createRegionEntry(keyPtr, valuePtr);
      result.push_back(rePtr);
    }
  }
}

/**
 * @brief return all values in the provided list.
 */
void MapSegment::getValues(std::vector<std::shared_ptr<Cacheable>>& result) {
  std::lock_guard<decltype(m_spinlock)> lk(m_spinlock);
  for (const auto& kv : m_map) {
    auto& entry = kv.second;
    std::shared_ptr<Cacheable> value;
    entry->getValue(value);
    auto entryImpl = entry->getImplPtr();

    if (value && !CacheableToken::isInvalid(value) &&
        !CacheableToken::isDestroyed(value) &&
        !CacheableToken::isTombstone(value)) {
      if (CacheableToken::isOverflowed(value)) {  // get Value from disc.
        auto& key = kv.first;
        value = getFromDisc(key, entryImpl);
        entryImpl->setValueI(value);
      }
      result.push_back(value);
    }
  }
}

// This function will not get called if concurrency checks are enabled. The
// versioning
// changes takes care of the version and no need for tracking the entry
int MapSegment::addTrackerForEntry(const std::shared_ptr<CacheableKey>& key,
                                   std::shared_ptr<Cacheable>& oldValue,
                                   bool addIfAbsent, bool failIfPresent,
                                   bool incUpdateCount) {
  if (m_concurrencyChecksEnabled) return -1;
  std::lock_guard<decltype(m_spinlock)> lk(m_spinlock);
  std::shared_ptr<MapEntry> entry;
  std::shared_ptr<MapEntry> newEntry;
  const auto& find = m_map.find(key);
  if (find == m_map.end()) {
    oldValue = nullptr;
    if (addIfAbsent) {
      std::shared_ptr<MapEntryImpl> entryImpl;
      // add a new entry with value as destroyed
      m_entryFactory->newMapEntry(expiry_manager_, key, entryImpl);
      entryImpl->setValueI(CacheableToken::destroyed());
      entry = entryImpl;
      newEntry = entryImpl;
    } else {
      // return -1 without adding an entry
      return -1;
    }
  } else {
    entry = find->second;
    entry->getValue(oldValue);
    if (failIfPresent) {
      // return -1 without adding an entry; the callee should check on
      // oldValue to distinguish this case from "addIfAbsent==false" case
      return -1;
    }
  }
  int updateCount;
  if (incUpdateCount) {
    (void)entry->addTracker(newEntry);
    updateCount = entry->incrementUpdateCount(newEntry);
  } else {
    updateCount = entry->addTracker(newEntry);
  }
  if (newEntry) {
    if (find == m_map.end()) {
      m_map.emplace(key, newEntry);
    } else {
      find->second = newEntry;
    }
  }
  return updateCount;
}

// This function will not get called if concurrency checks are enabled. The
// versioning
// changes takes care of the version and no need for tracking the entry
void MapSegment::removeTrackerForEntry(
    const std::shared_ptr<CacheableKey>& key) {
  if (m_concurrencyChecksEnabled) return;
  std::lock_guard<decltype(m_spinlock)> lk(m_spinlock);

  const auto& find = m_map.find(key);
  if (find != m_map.end()) {
    auto& entry = find->second;
    auto impl = entry->getImplPtr();
    removeTrackerForEntry(key, entry, impl);
  }
}

// This function will not get called if concurrency checks are enabled. The
// versioning
// changes takes care of the version and no need for tracking the entry
void MapSegment::addTrackerForAllEntries(
    MapOfUpdateCounters& updateCounterMap) {
  if (m_concurrencyChecksEnabled) return;
  std::lock_guard<decltype(m_spinlock)> lk(m_spinlock);

  std::shared_ptr<MapEntry> newEntry;
  std::shared_ptr<CacheableKey> key;
  for (auto& kv : m_map) {
    kv.second->getKey(key);
    int updateCount = kv.second->addTracker(newEntry);
    if (newEntry != nullptr) {
      kv.second = newEntry;
    }
    updateCounterMap.emplace(key, updateCount);
  }
}

// This function will not get called if concurrency checks are enabled. The
// versioning
// changes takes care of the version and no need for tracking the entry
void MapSegment::removeDestroyTracking() {
  if (m_concurrencyChecksEnabled) return;
  std::lock_guard<decltype(m_spinlock)> lk(m_spinlock);
  m_destroyedKeys.clear();
}

/**
 * @brief replace the existing hash map with one that is wider
 *   to reduce collision chains.
 */
void MapSegment::rehash() {
  // Only called from put, segment must already be locked...
  auto newMapSize = TableOfPrimes::getPrime(++m_primeIndex);
  LOG_FINER("Rehashing MapSegment to size %d.", newMapSize);
  m_map.reserve(newMapSize);
  m_rehashCount++;
}
std::shared_ptr<Cacheable> MapSegment::getFromDisc(
    std::shared_ptr<CacheableKey> key,
    std::shared_ptr<MapEntryImpl>& entryImpl) {
  auto* lregion = static_cast<LocalRegion*>(m_region);
  EntriesMap* em = lregion->getEntryMap();
  return em->getFromDisk(key, entryImpl);
}

GfErrType MapSegment::putForTrackedEntry(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Cacheable>& newValue,
    std::shared_ptr<MapEntry>& entry, std::shared_ptr<MapEntryImpl>& entryImpl,
    int updateCount, VersionStamp& versionStamp, DataInput* delta) {
  if (updateCount < 0 || m_concurrencyChecksEnabled) {
    // for a non-tracked put (e.g. from notification) go ahead with the
    // create/update and increment the update counter
    auto* thinClientRegion = dynamic_cast<ThinClientRegion*>(m_region);
    ThinClientPoolDM* m_poolDM = nullptr;
    if (thinClientRegion) {
      m_poolDM =
          dynamic_cast<ThinClientPoolDM*>(thinClientRegion->getDistMgr());
    }

    if (delta != nullptr) {
      std::shared_ptr<Cacheable> oldValue;
      entryImpl->getValueI(oldValue);
      if (oldValue == nullptr || CacheableToken::isDestroyed(oldValue) ||
          CacheableToken::isInvalid(oldValue) ||
          CacheableToken::isTombstone(oldValue)) {
        if (m_poolDM) {
          m_poolDM->updateNotificationStats(false, std::chrono::nanoseconds(0));
        }
        return GF_INVALID_DELTA;
      } else if (CacheableToken::isOverflowed(
                     oldValue)) {  // get Value from disc.
        oldValue = getFromDisc(key, entryImpl);
        if (oldValue == nullptr) {
          if (m_poolDM) {
            m_poolDM->updateNotificationStats(false,
                                              std::chrono::nanoseconds(0));
          }
          return GF_INVALID_DELTA;
        }
      }

      using clock = std::chrono::steady_clock;

      auto valueWithDelta = std::dynamic_pointer_cast<Delta>(oldValue);
      auto& newValue1 = const_cast<std::shared_ptr<Cacheable>&>(newValue);
      try {
        if (m_region->getAttributes().getCloningEnabled()) {
          auto tempVal = valueWithDelta->clone();
          auto currTimeBefore = clock::now();
          tempVal->fromDelta(*delta);

          if (m_poolDM) {
            m_poolDM->updateNotificationStats(true,
                                              clock::now() - currTimeBefore);
          }
          newValue1 = std::dynamic_pointer_cast<Serializable>(tempVal);
          entryImpl->setValueI(newValue1);
        } else {
          auto currTimeBefore = clock::now();
          valueWithDelta->fromDelta(*delta);
          newValue1 = std::dynamic_pointer_cast<Serializable>(valueWithDelta);

          if (m_poolDM) {
            m_poolDM->updateNotificationStats(true,
                                              clock::now() - currTimeBefore);
          }
          entryImpl->setValueI(
              std::dynamic_pointer_cast<Serializable>(valueWithDelta));
        }
      } catch (InvalidDeltaException&) {
        return GF_INVALID_DELTA;
      }
    } else {
      entryImpl->setValueI(newValue);
    }
    if (m_concurrencyChecksEnabled) {
      // erase if the entry is in tombstone
      m_tombstoneList->erase(key, true);
      entryImpl->getVersionStamp().setVersions(versionStamp);
    }
    (void)incrementUpdateCount(key, entry);
    return GF_NOERR;
  } else if (updateCount == entry->getUpdateCount()) {
    // good case; go ahead with the create/update
    entryImpl->setValueI(newValue);
    removeTrackerForEntry(key, entry, entryImpl);
    return GF_NOERR;
  } else {
    // entry updated while tracking was being done
    // abort the create/update and do not change the oldValue or MapEntry
    removeTrackerForEntry(key, entry, entryImpl);
    return GF_CACHE_ENTRY_UPDATED;
  }
}
void MapSegment::reapTombstones(std::map<uint16_t, int64_t>& gcVersions) {
  std::lock_guard<decltype(m_spinlock)> lk(m_spinlock);
  m_tombstoneList->reap_tombstones(gcVersions);
}
void MapSegment::reapTombstones(std::shared_ptr<CacheableHashSet> removedKeys) {
  std::lock_guard<decltype(m_spinlock)> lk(m_spinlock);
  m_tombstoneList->reap_tombstones(removedKeys);
}

GfErrType MapSegment::isTombstone(std::shared_ptr<CacheableKey> key,
                                  std::shared_ptr<MapEntryImpl>& me,
                                  bool& result) {
  std::shared_ptr<Cacheable> value;
  std::shared_ptr<MapEntryImpl> mePtr;
  const auto& find = m_map.find(key);
  if (find == m_map.end()) {
    result = false;
    return GF_NOERR;
  }
  auto& entry = find->second;
  mePtr = entry->getImplPtr();

  if (!mePtr) {
    result = false;
    return GF_NOERR;
  }

  mePtr->getValueI(value);
  if (!value) {
    result = false;
    return GF_NOERR;
  }

  if (CacheableToken::isTombstone(value)) {
    if (m_tombstoneList->exists(key)) {
      const auto& findInTombstoneList = m_map.find(key);
      if (findInTombstoneList != m_map.end()) {
        mePtr = findInTombstoneList->second->getImplPtr();
        me = mePtr;
      }
      result = true;
      return GF_NOERR;
    } else {
      LOG_FINER("1 result= false return GF_CACHE_ILLEGAL_STATE_EXCEPTION");
      result = false;
      return GF_CACHE_ILLEGAL_STATE_EXCEPTION;
    }

  } else {
    if (m_tombstoneList->exists(key)) {
      LOG_FINER(" 2 result= false return GF_CACHE_ILLEGAL_STATE_EXCEPTION");
      result = false;
      return GF_CACHE_ILLEGAL_STATE_EXCEPTION;
    } else {
      result = false;
      return GF_NOERR;
    }
  }
}
}  // namespace client
}  // namespace geode
}  // namespace apache
