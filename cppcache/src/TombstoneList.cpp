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

#include "TombstoneList.hpp"

#include <unordered_map>

#include "MapSegment.hpp"
#include "TombstoneExpiryHandler.hpp"

namespace apache {
namespace geode {
namespace client {

#define SIZEOF_PTR (sizeof(void*))
#define SIZEOF_SHAREDPTR (SIZEOF_PTR + 4)
// 3 variables in expiry handler, two variables for ace_reactor expiry, one
// pointer to expiry handle
#define SIZEOF_EXPIRYHANDLER ((SIZEOF_PTR * 5) + 4)
#define SIZEOF_TOMBSTONEENTRY (SIZEOF_PTR + 8 + 8)
// one shared ptr for map entry, one sharedPtr for tombstone entry, one
// sharedptr for key, one shared ptr for tombstone value,
// one ptr for tombstone list, one ptr for mapsegment, one tombstone entry
#define SIZEOF_TOMBSTONELISTENTRY \
  (SIZEOF_SHAREDPTR * 4 + SIZEOF_PTR * 2 + SIZEOF_TOMBSTONEENTRY)
#define SIZEOF_TOMBSTONEOVERHEAD \
  (SIZEOF_EXPIRYHANDLER + SIZEOF_TOMBSTONELISTENTRY)

ExpiryTaskManager::id_type TombstoneList::getExpiryTask(
    TombstoneExpiryHandler** handler) {
  // This function is not guarded as all functions of this class are called from
  // MapSegment
  auto duration = m_cacheImpl->getDistributedSystem()
                      .getSystemProperties()
                      .tombstoneTimeout();

  auto tombstoneEntryPtr = std::make_shared<TombstoneEntry>(nullptr);
  *handler = new TombstoneExpiryHandler(tombstoneEntryPtr, this, duration,
                                        m_cacheImpl);
  tombstoneEntryPtr->setHandler(*handler);
  auto id = m_cacheImpl->getExpiryTaskManager().scheduleExpiryTask(
      *handler, duration, std::chrono::seconds::zero());
  return id;
}

void TombstoneList::add(const std::shared_ptr<MapEntryImpl>& entry,
                        TombstoneExpiryHandler* handler,
                        ExpiryTaskManager::id_type taskid) {
  // This function is not guarded as all functions of this class are called from
  // MapSegment read TombstoneTImeout from systemProperties.
  auto tombstoneEntryPtr = std::make_shared<TombstoneEntry>(entry);
  handler->setTombstoneEntry(tombstoneEntryPtr);
  tombstoneEntryPtr->setHandler(handler);
  std::shared_ptr<CacheableKey> key;
  entry->getKeyI(key);

  tombstoneEntryPtr->setExpiryTaskId(taskid);
  m_tombstoneMap[key] = tombstoneEntryPtr;
  m_cacheImpl->getCachePerfStats().incTombstoneCount();
  auto tombstonesize = key->objectSize() + SIZEOF_TOMBSTONEOVERHEAD;
  m_cacheImpl->getCachePerfStats().incTombstoneSize(tombstonesize);
}

// Reaps the tombstones which have been gc'ed on server.
// A map that has identifier for ClientProxyMembershipID as key
// and server version of the tombstone with highest version as the
// value is passed as paramter
void TombstoneList::reapTombstones(std::map<uint16_t, int64_t>& gcVersions) {
  // This function is not guarded as all functions of this class are called from
  // MapSegment
  std::unordered_set<std::shared_ptr<CacheableKey>,
                     dereference_hash<std::shared_ptr<CacheableKey>>,
                     dereference_equal_to<std::shared_ptr<CacheableKey>>>
      tobeDeleted;
  for (const auto& queIter : m_tombstoneMap) {
    auto const& mapIter = gcVersions.find(
        queIter.second->getEntry()->getVersionStamp().getMemberId());

    if (mapIter == gcVersions.end()) {
      continue;
    }

    auto const& version = (*mapIter).second;
    if (version >=
        queIter.second->getEntry()->getVersionStamp().getRegionVersion()) {
      tobeDeleted.insert(queIter.first);
    }
  }
  for (const auto& itr : tobeDeleted) {
    unguardedRemoveEntryFromMapSegment(itr);
  }
}

// Reaps the tombstones whose keys are specified in the hash set .
void TombstoneList::reapTombstones(
    std::shared_ptr<CacheableHashSet> removedKeys) {
  // This function is not guarded as all functions of this class are called from
  // MapSegment
  for (auto queIter = removedKeys->begin(); queIter != removedKeys->end();
       ++queIter) {
    unguardedRemoveEntryFromMapSegment(*queIter);
  }
}
// Call this when the lock of MapSegment has not been taken
void TombstoneList::removeEntryFromMapSegment(
    std::shared_ptr<CacheableKey> key) {
  m_mapSegment->removeActualEntry(key, false);
}

// Call this when the lock of MapSegment has already been taken
void TombstoneList::unguardedRemoveEntryFromMapSegment(
    std::shared_ptr<CacheableKey> key) {
  m_mapSegment->unguardedRemoveActualEntry(key);
}

bool TombstoneList::exists(const std::shared_ptr<CacheableKey>& key) const {
  if (key) {
    return m_tombstoneMap.find(key) != m_tombstoneMap.end();
  }
  return false;
}

void TombstoneList::eraseEntryFromTombstoneList(
    const std::shared_ptr<CacheableKey>& key, bool cancelTask) {
  // This function is not guarded as all functions of this class are called from
  // MapSegment
  if (exists(key)) {
    if (cancelTask) {
      m_cacheImpl->getExpiryTaskManager().cancelTask(
          m_tombstoneMap[key]->getExpiryTaskId());
      delete m_tombstoneMap[key]->getHandler();
    }

    m_cacheImpl->getCachePerfStats().decTombstoneCount();
    auto tombstonesize = key->objectSize() + SIZEOF_TOMBSTONEOVERHEAD;
    m_cacheImpl->getCachePerfStats().decTombstoneSize(tombstonesize);
    m_tombstoneMap.erase(key);
  }
}

ExpiryTaskManager::id_type
TombstoneList::eraseEntryFromTombstoneListWithoutCancelTask(
    const std::shared_ptr<CacheableKey>& key,
    TombstoneExpiryHandler*& handler) {
  // This function is not guarded as all functions of this class are called from
  // MapSegment
  ExpiryTaskManager::id_type taskid = -1;
  if (exists(key)) {
    taskid = m_tombstoneMap[key]->getExpiryTaskId();
    handler = m_tombstoneMap[key]->getHandler();
    m_cacheImpl->getCachePerfStats().decTombstoneCount();
    auto tombstonesize = key->objectSize() + SIZEOF_TOMBSTONEOVERHEAD;
    m_cacheImpl->getCachePerfStats().decTombstoneSize(tombstonesize);
    m_tombstoneMap.erase(key);
  }
  return taskid;
}

void TombstoneList::cleanUp() {
  // This function is not guarded as all functions of this class are called from
  // MapSegment
  auto& expiryTaskManager = m_cacheImpl->getExpiryTaskManager();
  for (const auto& queIter : m_tombstoneMap) {
    expiryTaskManager.cancelTask(queIter.second->getExpiryTaskId());
    delete queIter.second->getHandler();
  }
}

TombstoneEntry::TombstoneEntry(const std::shared_ptr<MapEntryImpl>& entry)
    : m_entry(entry),
      m_tombstoneCreationTime(TombstoneEntry::clock::now()),
      m_expiryTaskId(0),
      m_handler(nullptr) {}

std::shared_ptr<MapEntryImpl> TombstoneEntry::getEntry() { return m_entry; }

TombstoneEntry::time_point TombstoneEntry::getTombstoneCreationTime() {
  return m_tombstoneCreationTime;
}

ExpiryTaskManager::id_type TombstoneEntry::getExpiryTaskId() {
  return m_expiryTaskId;
}

void TombstoneEntry::setExpiryTaskId(ExpiryTaskManager::id_type expiryTaskId) {
  m_expiryTaskId = expiryTaskId;
}

TombstoneExpiryHandler* TombstoneEntry::getHandler() { return m_handler; }

void TombstoneEntry::setHandler(TombstoneExpiryHandler* handler) {
  m_handler = handler;
}

TombstoneList::TombstoneList(MapSegment* mapSegment, CacheImpl* cacheImpl)
    : m_mapSegment(mapSegment), m_cacheImpl(cacheImpl) {}

TombstoneList::~TombstoneList() { cleanUp(); }
}  // namespace client
}  // namespace geode
}  // namespace apache
