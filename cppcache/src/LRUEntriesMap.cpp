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

#include "LRUEntriesMap.hpp"

#include <mutex>

#include "CacheImpl.hpp"
#include "EvictionController.hpp"
#include "ExpiryTaskManager.hpp"
#include "LRUEntryProperties.hpp"
#include "MapSegment.hpp"
#include "util/concurrent/spinlock_mutex.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * @brief LRUAction for testing map outside of a region....
 */
class TestMapAction : public virtual LRUAction {
 private:
  EntriesMap* m_eMap;

 public:
  explicit TestMapAction(EntriesMap* eMap) : m_eMap(eMap) { m_destroys = true; }

  ~TestMapAction() noexcept override = default;

  bool evict(const std::shared_ptr<MapEntryImpl>& mePtr) override {
    std::shared_ptr<CacheableKey> keyPtr;
    mePtr->getKeyI(keyPtr);
    /** @TODO try catch.... return true or false. */
    std::shared_ptr<Cacheable> cPtr;  // old value.
    std::shared_ptr<MapEntryImpl> me;
    std::shared_ptr<VersionTag> versionTag;
    return (m_eMap->remove(keyPtr, cPtr, me, 0, versionTag, false) == GF_NOERR);
  }

  LRUAction::Action getType() override { return LRUAction::LOCAL_DESTROY; }
  friend class LRUAction;
};

LRUEntriesMap::LRUEntriesMap(ExpiryTaskManager* expiryTaskManager,
                             std::unique_ptr<EntryFactory> entryFactory,
                             RegionInternal* region,
                             const LRUAction::Action& lruAction,
                             const uint32_t limit,
                             bool concurrencyChecksEnabled,
                             const uint8_t concurrency, bool heapLRUEnabled)
    : ConcurrentEntriesMap(expiryTaskManager, std::move(entryFactory),
                           concurrencyChecksEnabled, region, concurrency),
      lru_queue_(),
      m_limit(limit),
      m_pmPtr(nullptr),
      m_validEntries(0),
      m_heapLRUEnabled(heapLRUEnabled) {
  m_currentMapSize = 0;
  m_action = nullptr;
  m_evictionControllerPtr = nullptr;
  // translate action type to an instance.
  if (region) {
    m_action = LRUAction::newLRUAction(lruAction, region, this);
    m_name = region->getName();
    CacheImpl* cImpl = region->getCacheImpl();
    if (cImpl != nullptr) {
      m_evictionControllerPtr = cImpl->getEvictionController();
      if (m_evictionControllerPtr != nullptr) {
        m_evictionControllerPtr->registerRegion(m_name);
        LOG_INFO("Heap LRU eviction controller registered region %s",
                 m_name.c_str());
      }
    }
  } else {
    m_action = new TestMapAction(this);
  }
}

void LRUEntriesMap::close() {
  if (m_evictionControllerPtr != nullptr) {
    m_evictionControllerPtr->incrementHeapSize(-m_currentMapSize);
    m_evictionControllerPtr->unregisterRegion(m_name);
  }
  ConcurrentEntriesMap::close();
}

void LRUEntriesMap::clear() {
  updateMapSize(-m_currentMapSize);
  ConcurrentEntriesMap::clear();
}

LRUEntriesMap::~LRUEntriesMap() noexcept { delete m_action; }

/**
 * @brief put an item in the map... if it is a new entry, then the LRU may
 * need to be consulted.
 * If LRUAction is LRUInvalidateAction, then increment if old value was absent.
 * If LRUAction is one of Destroys, then increment if old Entry was absent.
 */
GfErrType LRUEntriesMap::create(const std::shared_ptr<CacheableKey>& key,
                                const std::shared_ptr<Cacheable>& newValue,
                                std::shared_ptr<MapEntryImpl>& me,
                                std::shared_ptr<Cacheable>& oldValue,
                                int updateCount, int destroyTracker,
                                std::shared_ptr<VersionTag> versionTag) {
  MapSegment* segmentRPtr = segmentFor(key);
  GfErrType err = GF_NOERR;
  {  // SYNCHRONIZE_SEGMENT(segmentRPtr);
    std::shared_ptr<MapEntryImpl> mePtr;
    if ((err = segmentRPtr->create(key, newValue, me, oldValue, updateCount,
                                   destroyTracker, versionTag)) != GF_NOERR) {
      return err;
    }
    // TODO:  can newValue ever be a token ??
    if (!CacheableToken::isToken(newValue)) {
      m_validEntries++;
    }
    //  oldValue can be an invalid token when "createIfInvalid" is true
    if (!CacheableToken::isInvalid(oldValue)) {
      m_size++;
    }
    std::shared_ptr<Cacheable> tmpValue;
    segmentRPtr->getEntry(key, mePtr, tmpValue);
    if (mePtr == nullptr) {
      return err;
    }

    lru_queue_.push(mePtr);
    me = mePtr;
  }
  if (m_evictionControllerPtr != nullptr) {
    int64_t newSize =
        static_cast<int64_t>(Utils::checkAndGetObjectSize(newValue));
    newSize += static_cast<int64_t>(Utils::checkAndGetObjectSize(key));
    if (oldValue != nullptr) {
      newSize -= static_cast<int64_t>(oldValue->objectSize());
    } else {
      newSize -= static_cast<int64_t>(sizeof(void*));
    }
    updateMapSize(newSize);
  }
  err = processLRU();
  return err;
}

GfErrType LRUEntriesMap::processLRU() {
  GfErrType canEvict = GF_NOERR;
  while (canEvict == GF_NOERR && mustEvict()) {
    canEvict = evictionHelper();
  }
  return canEvict;
}

GfErrType LRUEntriesMap::evictionHelper() {
  GfErrType err = GF_NOERR;
  auto entry = lru_queue_.pop();
  if (entry == nullptr) {
    err = GF_ENOENT;
    return err;
  }
  bool IsEvictDone = m_action->evict(entry);
  if (m_action->overflows() && IsEvictDone) {
    --m_validEntries;
  }
  if (!IsEvictDone) {
    err = GF_DISKFULL;
    return err;
  }
  return err;
}

void LRUEntriesMap::processLRU(int32_t numEntriesToEvict) {
  int32_t evicted = 0;
  for (int32_t i = 0; i < numEntriesToEvict; i++) {
    if (m_validEntries > 0 && size() > 0) {
      if (evictionHelper() == GF_NOERR) {
        evicted++;
      }
    } else {
      break;
    }
  }
}

GfErrType LRUEntriesMap::invalidate(const std::shared_ptr<CacheableKey>& key,
                                    std::shared_ptr<MapEntryImpl>& me,
                                    std::shared_ptr<Cacheable>& oldValue,
                                    std::shared_ptr<VersionTag> versionTag) {
  MapSegment* segmentRPtr = segmentFor(key);
  bool isTokenAdded = false;
  GfErrType err =
      segmentRPtr->invalidate(key, me, oldValue, versionTag, isTokenAdded);
  if (isTokenAdded) {
    ++m_size;
  }
  if (err != GF_NOERR) {
    return err;
  }
  bool isOldValueToken = CacheableToken::isToken(oldValue);
  //  get the old value first which is required for heapLRU
  // calculation and for listeners; note even though there is a race
  // here between get and destroy, it will not harm if we get a slightly
  // later value
  // TODO: assess any other effects of this race
  if (CacheableToken::isOverflowed(oldValue)) {
    std::lock_guard<MapSegment> _guard(*segmentRPtr);
    auto&& persistenceInfo = me->getLRUProperties().persistence_info();
    //  get the old value first which is required for heapLRU
    // calculation and for listeners; note even though there is a race
    // here between get and destroy, it will not harm if we get a slightly
    // older value
    // TODO: there is also a race between segment remove and destroy here
    // need to assess the effect of this; also assess the affect of above
    // mentioned race
    oldValue = m_pmPtr->read(key, persistenceInfo);
    if (oldValue != nullptr) {
      m_pmPtr->destroy(key, persistenceInfo);
    }
  }
  if (!isOldValueToken) {
    --m_validEntries;
    lru_queue_.remove(me);
    auto newSize = CacheableToken::invalid()->objectSize();
    if (oldValue != nullptr) {
      newSize -= oldValue->objectSize();
    } else {
      newSize -= sizeof(void*);
    }
    if (m_evictionControllerPtr != nullptr) {
      if (newSize != 0) {
        updateMapSize(newSize);
      }
    }
  }
  return err;
}

GfErrType LRUEntriesMap::put(const std::shared_ptr<CacheableKey>& key,
                             const std::shared_ptr<Cacheable>& newValue,
                             std::shared_ptr<MapEntryImpl>& me,
                             std::shared_ptr<Cacheable>& oldValue,
                             int updateCount, int destroyTracker,
                             std::shared_ptr<VersionTag> versionTag,
                             bool& isUpdate, DataInput* delta) {
  MapSegment* segmentRPtr = segmentFor(key);
  GfErrType err = GF_NOERR;
  bool segmentLocked = false;
  {
    if (m_action != nullptr &&
        m_action->getType() == LRUAction::OVERFLOW_TO_DISK) {
      segmentRPtr->lock();
      segmentLocked = true;
    }
    std::shared_ptr<MapEntryImpl> mePtr;
    if ((err = segmentRPtr->put(key, newValue, me, oldValue, updateCount,
                                destroyTracker, isUpdate, versionTag, delta)) !=
        GF_NOERR) {
      if (segmentLocked == true) segmentRPtr->unlock();
      return err;
    }

    bool isOldValueToken = CacheableToken::isToken(oldValue);
    // TODO:  need tests for checking that oldValue is returned
    // correctly to listeners for overflow -- this is for all operations
    // put, invalidate, destroy
    // TODO:  need tests for checking if the overflowed entry is
    // destroyed *from disk* in put
    //  existing overflowed entry should be returned correctly in
    // put and removed from disk
    // TODO: need to verify if segment mutex lock is enough here
    // TODO: This whole class needs to be rethought and reworked for locking
    // and concurrency issues. The basic issue is two concurrent operations
    // on the same key regardless of whether we need to go to the persistence
    // manager or not. So all operations on the same key should be protected
    // unless very careful thought has gone into it.
    if (CacheableToken::isOverflowed(oldValue)) {
      if (!segmentLocked) {
        segmentRPtr->unlock();
        segmentLocked = true;
      }
      auto&& persistenceInfo = me->getLRUProperties().persistence_info();
      oldValue = m_pmPtr->read(key, persistenceInfo);
      if (oldValue != nullptr) {
        m_pmPtr->destroy(key, persistenceInfo);
      }
    }
    // SpinLock& lock = segmentRPtr->getSpinLock();
    // std::lock_guard<spinlock_mutex> mapGuard( lock );

    // TODO:  when can newValue be a token ??
    if (CacheableToken::isToken(newValue) && !isOldValueToken) {
      --m_validEntries;
    }
    if (!CacheableToken::isToken(newValue) && isOldValueToken) {
      ++m_validEntries;
    }

    // Add new entry to LRU list
    if (isUpdate == false) {
      ++m_size;
      ++m_validEntries;
      std::shared_ptr<Cacheable> tmpValue;
      segmentRPtr->getEntry(key, mePtr, tmpValue);
      // mePtr cannot be null, we just put it...
      // must convert to an std::shared_ptr<LRUMapEntryImpl>...
      lru_queue_.push(mePtr);
      me = mePtr;
    } else {
      if (!CacheableToken::isToken(newValue) && isOldValueToken) {
        std::shared_ptr<Cacheable> tmpValue;
        segmentRPtr->getEntry(key, mePtr, tmpValue);
        lru_queue_.push(mePtr);
        me = mePtr;
      }
    }
  }
  if (m_evictionControllerPtr != nullptr) {
    int64_t newSize =
        static_cast<int64_t>(Utils::checkAndGetObjectSize(newValue));
    /*
    if (newSize == 0) {
      LOG_WARN("Object size for class ID %d should not be zero when HeapLRU is
    enabled", newValue->classId());
      LOG_DEBUG("Type ID is %d for the object returning zero HeapLRU size",
    newValue->typeId());
    }
    */
    if (isUpdate == false) {
      newSize += static_cast<int64_t>(Utils::checkAndGetObjectSize(key));
    } else {
      if (oldValue != nullptr) {
        newSize -= static_cast<int64_t>(oldValue->objectSize());
      } else {
        newSize -= static_cast<int64_t>(sizeof(void*));
      }
    }
    updateMapSize(newSize);
  }

  err = processLRU();

  if (segmentLocked) {
    segmentRPtr->unlock();
  }
  return err;
}

/**
 * @brief Get the value from an entry, if the entry exists it will be marked
 * as recently used. Note, getEntry, entries, and values do not mark entries
 * as recently used.
 */
bool LRUEntriesMap::get(const std::shared_ptr<CacheableKey>& key,
                        std::shared_ptr<Cacheable>& value,
                        std::shared_ptr<MapEntryImpl>& me) {
  value.reset();
  bool trigger_lru = false;
  auto* segment = segmentFor(key);
  std::shared_ptr<MapEntryImpl> map_entry;

  {
    std::unique_lock<MapSegment> lock;

    if (m_action != nullptr &&
        m_action->getType() == LRUAction::OVERFLOW_TO_DISK) {
      lock = decltype(lock){*segment};
    }

    if (!segment->getEntry(key, map_entry, value)) {
      return false;
    }

    LRUEntryProperties& lru_props = map_entry->getLRUProperties();
    if (CacheableToken::isOverflowed(value)) {
      auto&& persistenceInfo = lru_props.persistence_info();
      try {
        value = m_pmPtr->read(key, persistenceInfo);
      } catch (Exception& ex) {
        LOG_ERROR("read on the persistence layer failed - %s", ex.what());
        return false;
      }

      m_region->getRegionStats()->incRetrieves();
      m_region->getCacheImpl()->getCachePerfStats().incRetrieves();

      bool update;
      std::shared_ptr<VersionTag> version;
      std::shared_ptr<Cacheable> old_value;
      if (GF_NOERR != segment->put(key, value, map_entry, old_value, 0, 0,
                                   update, version, nullptr)) {
        return false;
      }

      ++m_validEntries;
      trigger_lru = true;
      lru_queue_.push(map_entry);

      if (m_evictionControllerPtr != nullptr) {
        int64_t newSize = 0;
        if (value != nullptr) {
          newSize += static_cast<int64_t>(
              value->objectSize() - CacheableToken::invalid()->objectSize());
        } else {
          newSize += sizeof(void*);
        }
        updateMapSize(newSize);
      }
    } else {
      lru_queue_.move_to_end(map_entry);
    }
  }

  me = std::move(map_entry);
  return !trigger_lru || processLRU() == GF_NOERR;
}

GfErrType LRUEntriesMap::remove(const std::shared_ptr<CacheableKey>& key,
                                std::shared_ptr<Cacheable>& result,
                                std::shared_ptr<MapEntryImpl>& me,
                                int updateCount,
                                std::shared_ptr<VersionTag> versionTag,
                                bool afterRemote) {
  MapSegment* segmentRPtr = segmentFor(key);
  bool isEntryFound = true;
  GfErrType err;
  if ((err = segmentRPtr->remove(key, result, me, updateCount, versionTag,
                                 afterRemote, isEntryFound)) == GF_NOERR) {
    // ACE_Guard<MapSegment> _guard(*segmentRPtr);
    if (result != nullptr && me != nullptr) {
      lru_queue_.remove(me);
      LRUEntryProperties& lru_prop = me->getLRUProperties();
      if (isEntryFound) --m_size;
      if (!CacheableToken::isToken(result)) {
        --m_validEntries;
      }
      if (CacheableToken::isOverflowed(result)) {
        std::lock_guard<MapSegment> _guard(*segmentRPtr);
        auto&& persistenceInfo = lru_prop.persistence_info();
        //  get the old value first which is required for heapLRU
        // calculation and for listeners; note even though there is a race
        // here between get and destroy, it will not harm if we get a slightly
        // older value
        // TODO: there is also a race between segment remove and destroy here
        // need to assess the effect of this; also assess the affect of above
        // mentioned race
        result = m_pmPtr->read(key, persistenceInfo);
        if (result != nullptr) {
          m_pmPtr->destroy(key, persistenceInfo);
        }
      }
      if (m_evictionControllerPtr != nullptr) {
        int64_t sizeToRemove = static_cast<int64_t>(key->objectSize());
        sizeToRemove += static_cast<int64_t>(result->objectSize());
        updateMapSize(-sizeToRemove);
      }
    }
  }

  return err;
}

void LRUEntriesMap::updateMapSize(int64_t size) {
  // TODO: check and remove null check since this has already been done
  // by all the callers
  if (m_evictionControllerPtr != nullptr) {
    m_currentMapSize += size;
    m_evictionControllerPtr->incrementHeapSize(size);
  }
}
std::shared_ptr<Cacheable> LRUEntriesMap::getFromDisk(
    const std::shared_ptr<CacheableKey>& key,
    std::shared_ptr<MapEntryImpl>& me) const {
  auto&& persistenceInfo = me->getLRUProperties().persistence_info();
  std::shared_ptr<Cacheable> tmpObj;
  try {
    LOG_DEBUG("Reading value from persistence layer for key: %s",
              key->toString().c_str());
    tmpObj = m_pmPtr->read(key, persistenceInfo);
  } catch (Exception& ex) {
    LOG_ERROR("read on the persistence layer failed - %s", ex.what());
    return nullptr;
  }
  return tmpObj;
}
}  // namespace client
}  // namespace geode
}  // namespace apache
