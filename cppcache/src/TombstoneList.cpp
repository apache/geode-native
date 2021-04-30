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

#include <unordered_set>

#include <geode/SystemProperties.hpp>

#include "CacheImpl.hpp"
#include "MapSegment.hpp"
#include "TombstoneEntry.hpp"
#include "TombstoneExpiryTask.hpp"

namespace apache {
namespace geode {
namespace client {

// TODO. Review this overhead is OK
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

ExpiryTask::id_t TombstoneList::add(
    const std::shared_ptr<MapEntryImpl>& entry) {
  // This function is not guarded as all functions of this class are called from
  // MapSegment read TombstoneTImeout from systemProperties.
  auto tombstone = std::make_shared<TombstoneEntry>(entry);
  std::shared_ptr<CacheableKey> key;
  entry->getKeyI(key);

  auto duration =
      cache_.getDistributedSystem().getSystemProperties().tombstoneTimeout();
  auto& manager = cache_.getExpiryTaskManager();
  auto task =
      std::make_shared<TombstoneExpiryTask>(manager, segment_, tombstone);
  auto task_id = manager.schedule(std::move(task), duration);
  tombstone->task_id(task_id);
  tombstones_[key] = tombstone;

  auto& perf_stats = cache_.getCachePerfStats();

  perf_stats.incTombstoneCount();
  perf_stats.incTombstoneSize(key->objectSize() + SIZEOF_TOMBSTONEOVERHEAD);

  return task_id;
}

// Reaps the tombstones which have been gc'ed on server.
// A map that has identifier for ClientProxyMembershipID as key
// and server version of the tombstone with highest version as the
// value is passed as paramter
void TombstoneList::reap_tombstones(std::map<uint16_t, int64_t>& gcVersions) {
  // This function is not guarded as all functions of this class are called from
  // MapSegment
  std::unordered_set<std::shared_ptr<CacheableKey>,
                     dereference_hash<std::shared_ptr<CacheableKey>>,
                     dereference_equal_to<std::shared_ptr<CacheableKey>>>
      tobeDeleted;
  for (const auto& queIter : tombstones_) {
    auto const& mapIter = gcVersions.find(
        queIter.second->entry()->getVersionStamp().getMemberId());

    if (mapIter == gcVersions.end()) {
      continue;
    }

    auto const& version = (*mapIter).second;
    if (version >=
        queIter.second->entry()->getVersionStamp().getRegionVersion()) {
      tobeDeleted.insert(queIter.first);
    }
  }

  for (const auto& key : tobeDeleted) {
    segment_.remove_entry(key);
  }
}

// Reaps the tombstones whose keys are specified in the hash set .
void TombstoneList::reap_tombstones(
    const std::shared_ptr<CacheableHashSet>& keys) {
  // This function is not guarded as all functions of this class are called from
  // MapSegment
  for (const auto& key : *keys) {
    segment_.remove_entry(key);
  }
}

bool TombstoneList::exists(const std::shared_ptr<CacheableKey>& key) const {
  return key != nullptr && tombstones_.find(key) != tombstones_.end();
}

bool TombstoneList::erase(const std::shared_ptr<CacheableKey>& key,
                          bool cancel_task) {
  // This function is not guarded as all functions of this class are called from
  // MapSegment

  auto&& iter = tombstones_.find(key);
  if (iter == tombstones_.end()) {
    return false;
  }

  iter->second->invalidate();

  if (cancel_task) {
    cache_.getExpiryTaskManager().cancel(iter->second->task_id());
  }

  auto& perf_stats = cache_.getCachePerfStats();

  perf_stats.decTombstoneCount();
  perf_stats.decTombstoneSize(key->objectSize() + SIZEOF_TOMBSTONEOVERHEAD);

  tombstones_.erase(iter);
  return true;
}

void TombstoneList::cleanup() {
  // This function is not guarded as all functions of this class are called from
  // MapSegment
  auto& manager = cache_.getExpiryTaskManager();
  for (const auto& iter : tombstones_) {
    manager.cancel(iter.second->task_id());
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
