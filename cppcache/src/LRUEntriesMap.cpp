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
#include "LRUList.hpp"
#include "MapSegment.hpp"
#include "util/concurrent/spinlock_mutex.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * @brief LRUAction for testing map outside of a region....
 */
class APACHE_GEODE_EXPORT TestMapAction : public virtual LRUAction {
 private:
  EntriesMap* m_eMap;

 public:
  explicit TestMapAction(EntriesMap* eMap) : m_eMap(eMap) { m_destroys = true; }

  virtual ~TestMapAction() {}

  virtual bool evict(const std::shared_ptr<MapEntryImpl>& mePtr) {
    std::shared_ptr<CacheableKey> keyPtr;
    mePtr->getKeyI(keyPtr);
    /** @TODO try catch.... return true or false. */
    std::shared_ptr<Cacheable> cPtr;  // old value.
    std::shared_ptr<MapEntryImpl> me;
    std::shared_ptr<VersionTag> versionTag;
    return (m_eMap->remove(keyPtr, cPtr, me, 0, versionTag, false) == GF_NOERR);
  }

  virtual LRUAction::Action getType() { return LRUAction::LOCAL_DESTROY; }
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
      m_lruList(),
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
        LOGINFO("Heap LRU eviction controller registered region %s",
                m_name.c_str());
      }
    }
  } else {
    m_action = new TestMapAction(this);
  }
}

void LRUEntriesMap::close() {
  if (m_evictionControllerPtr != nullptr) {
    m_evictionControllerPtr->updateRegionHeapInfo((-1 * (m_currentMapSize)));
    m_evictionControllerPtr->deregisterRegion(m_name);
  }
  ConcurrentEntriesMap::close();
}

void LRUEntriesMap::clear() {
  updateMapSize((-1 * (m_currentMapSize)));
  ConcurrentEntriesMap::clear();
}

LRUEntriesMap::~LRUEntriesMap() { delete m_action; }

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
    m_lruList.appendEntry(mePtr);
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
  std::shared_ptr<MapEntryImpl> lruEntryPtr;
  m_lruList.getLRUEntry(lruEntryPtr);
  if (lruEntryPtr == nullptr) {
    err = GF_ENOENT;
    return err;
  }
  bool IsEvictDone = m_action->evict(lruEntryPtr);
  if (m_action->overflows() && IsEvictDone) {
    --m_validEntries;
    lruEntryPtr->getLRUProperties().setEvicted();
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
  int64_t newSize = 0;
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
    auto&& persistenceInfo = me->getLRUProperties().getPersistenceInfo();
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
    me->getLRUProperties().setEvicted();
    newSize = CacheableToken::invalid()->objectSize();
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
      auto&& persistenceInfo = me->getLRUProperties().getPersistenceInfo();
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

      m_lruList.appendEntry(mePtr);
      me = mePtr;
    } else {
      if (!CacheableToken::isToken(newValue) && isOldValueToken) {
        std::shared_ptr<Cacheable> tmpValue;
        segmentRPtr->getEntry(key, mePtr, tmpValue);
        mePtr->getLRUProperties().clearEvicted();
        m_lruList.appendEntry(
            std::shared_ptr<MapEntryImpl>(mePtr->getImplPtr()));
        me = mePtr;
      }
    }
  }
  if (m_evictionControllerPtr != nullptr) {
    int64_t newSize =
        static_cast<int64_t>(Utils::checkAndGetObjectSize(newValue));
    /*
    if (newSize == 0) {
      LOGWARN("Object size for class ID %d should not be zero when HeapLRU is
    enabled", newValue->classId());
      LOGDEBUG("Type ID is %d for the object returning zero HeapLRU size",
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
                        std::shared_ptr<Cacheable>& returnPtr,
                        std::shared_ptr<MapEntryImpl>& me) {
  bool doProcessLRU = false;
  MapSegment* segmentRPtr = segmentFor(key);
  bool segmentLocked = false;
  if (m_action != nullptr &&
      m_action->getType() == LRUAction::OVERFLOW_TO_DISK) {
    segmentRPtr->lock();
    segmentLocked = true;
  }
  {
    returnPtr = nullptr;
    std::shared_ptr<MapEntryImpl> mePtr;
    if (false == segmentRPtr->getEntry(key, mePtr, returnPtr)) {
      if (segmentLocked == true) segmentRPtr->unlock();
      return false;
    }
    // segmentRPtr->get(key, returnPtr, mePtr);
    auto nodeToMark = mePtr;
    LRUEntryProperties& lruProps = nodeToMark->getLRUProperties();
    if (returnPtr != nullptr && CacheableToken::isOverflowed(returnPtr)) {
      auto&& persistenceInfo = lruProps.getPersistenceInfo();
      std::shared_ptr<Cacheable> tmpObj;
      try {
        tmpObj = m_pmPtr->read(key, persistenceInfo);
      } catch (Exception& ex) {
        LOGERROR("read on the persistence layer failed - %s", ex.what());
        if (segmentLocked == true) segmentRPtr->unlock();
        return false;
      }
      m_region->getRegionStats()->incRetrieves();
      m_region->getCacheImpl()->getCachePerfStats().incRetrieves();

      returnPtr = tmpObj;

      std::shared_ptr<Cacheable> oldValue;
      bool isUpdate;
      std::shared_ptr<VersionTag> versionTag;
      if (GF_NOERR == segmentRPtr->put(key, tmpObj, mePtr, oldValue, 0, 0,
                                       isUpdate, versionTag, nullptr)) {
        // m_entriesRetrieved++;
        ++m_validEntries;
        lruProps.clearEvicted();
        m_lruList.appendEntry(nodeToMark);
      }
      doProcessLRU = true;
      if (m_evictionControllerPtr != nullptr) {
        int64_t newSize = 0;
        if (tmpObj != nullptr) {
          newSize += static_cast<int64_t>(
              tmpObj->objectSize() - CacheableToken::invalid()->objectSize());
        } else {
          newSize += sizeof(void*);
        }
        updateMapSize(newSize);
      }
    }
    me = mePtr;
    // lruProps.clearEvicted();
    lruProps.setRecentlyUsed();
    if (doProcessLRU) {
      GfErrType IsProcessLru = processLRU();
      if ((IsProcessLru != GF_NOERR)) {
        if (segmentLocked) {
          segmentRPtr->unlock();
        }
        return false;
      }
    }
    if (segmentLocked) {
      segmentRPtr->unlock();
    }
    return true;
  }
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
      LRUEntryProperties& lruProps = me->getLRUProperties();
      lruProps.setEvicted();
      if (isEntryFound) --m_size;
      if (!CacheableToken::isToken(result)) {
        --m_validEntries;
      }
      if (CacheableToken::isOverflowed(result)) {
        std::lock_guard<MapSegment> _guard(*segmentRPtr);
        auto&& persistenceInfo = lruProps.getPersistenceInfo();
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
        updateMapSize((-1 * sizeToRemove));
      }
    }
  }
  return err;
}

void LRUEntriesMap::updateMapSize(int64_t size) {
  // TODO: check and remove null check since this has already been done
  // by all the callers
  if (m_evictionControllerPtr != nullptr) {
    {
      std::lock_guard<spinlock_mutex> __guard(m_mapInfoLock);
      m_currentMapSize += size;
    }
    m_evictionControllerPtr->updateRegionHeapInfo(size);
  }
}
std::shared_ptr<Cacheable> LRUEntriesMap::getFromDisk(
    const std::shared_ptr<CacheableKey>& key,
    std::shared_ptr<MapEntryImpl>& me) const {
  auto&& persistenceInfo = me->getLRUProperties().getPersistenceInfo();
  std::shared_ptr<Cacheable> tmpObj;
  try {
    LOGDEBUG("Reading value from persistence layer for key: %s",
             key->toString().c_str());
    tmpObj = m_pmPtr->read(key, persistenceInfo);
  } catch (Exception& ex) {
    LOGERROR("read on the persistence layer failed - %s", ex.what());
    return nullptr;
  }
  return tmpObj;
}

void LRUEntriesMap::setPersistenceManager(
    std::shared_ptr<PersistenceManager>& pmPtr) {
  m_pmPtr = pmPtr;
}

bool LRUEntriesMap::mustEvict() const {
  if (m_action == nullptr) {
    LOGFINE("Eviction action is nullptr");
    return false;
  }
  if (m_action->overflows()) {
    return validEntriesSize() > m_limit;
  } else if ((m_heapLRUEnabled) && (m_limit == 0)) {
    return false;
  } else {
    return size() > m_limit;
  }
}

uint32_t LRUEntriesMap::validEntriesSize() const { return m_validEntries; }

void LRUEntriesMap::adjustLimit(uint32_t limit) { m_limit = limit; }
}  // namespace client
}  // namespace geode
}  // namespace apache
