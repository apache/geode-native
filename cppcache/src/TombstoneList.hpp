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

#ifndef GEODE_TOMBSTONELIST_H_
#define GEODE_TOMBSTONELIST_H_

#include <chrono>
#include <list>
#include <memory>
#include <unordered_map>

#include <geode/CacheableBuiltins.hpp>
#include <geode/internal/functional.hpp>

#include "MapEntry.hpp"

namespace apache {
namespace geode {
namespace client {

class MapSegment;
class TombstoneExpiryHandler;

class TombstoneEntry {
 public:
  using clock = std::chrono::steady_clock;
  using time_point = clock::time_point;

  explicit TombstoneEntry(const std::shared_ptr<MapEntryImpl>& entry)
      : m_entry(entry),
        m_tombstoneCreationTime(TombstoneEntry::clock::now()),
        m_expiryTaskId(0),
        m_handler(nullptr) {}
  virtual ~TombstoneEntry() {}
  std::shared_ptr<MapEntryImpl> getEntry() { return m_entry; }
  time_point getTombstoneCreationTime() { return m_tombstoneCreationTime; }
  ExpiryTaskManager::id_type getExpiryTaskId() { return m_expiryTaskId; }
  void setExpiryTaskId(ExpiryTaskManager::id_type expiryTaskId) {
    m_expiryTaskId = expiryTaskId;
  }
  TombstoneExpiryHandler* getHandler() { return m_handler; };
  void setHandler(TombstoneExpiryHandler* handler) { m_handler = handler; };

 private:
  std::shared_ptr<MapEntryImpl> m_entry;
  time_point m_tombstoneCreationTime;
  ExpiryTaskManager::id_type m_expiryTaskId;
  TombstoneExpiryHandler* m_handler;
};

class TombstoneList {
 public:
  TombstoneList(MapSegment* mapSegment, CacheImpl* cacheImpl)
      : m_mapSegment(mapSegment), m_cacheImpl(cacheImpl) {}
  virtual ~TombstoneList() { cleanUp(); }
  void add(const std::shared_ptr<MapEntryImpl>& entry,
           TombstoneExpiryHandler* handler, ExpiryTaskManager::id_type taskID);

  // Reaps the tombstones which have been gc'ed on server.
  // A map that has identifier for ClientProxyMembershipID as key
  // and server version of the tombstone with highest version as the
  // value is passed as paramter
  void reapTombstones(std::map<uint16_t, int64_t>& gcVersions);
  void reapTombstones(std::shared_ptr<CacheableHashSet> removedKeys);
  void eraseEntryFromTombstoneList(const std::shared_ptr<CacheableKey>& key,
                                   bool cancelTask = true);
  ExpiryTaskManager::id_type eraseEntryFromTombstoneListWithoutCancelTask(
      const std::shared_ptr<CacheableKey>& key,
      TombstoneExpiryHandler*& handler);
  void cleanUp();
  ExpiryTaskManager::id_type getExpiryTask(TombstoneExpiryHandler** handler);
  bool exists(const std::shared_ptr<CacheableKey>& key) const;

 private:
  void removeEntryFromMapSegment(std::shared_ptr<CacheableKey> key);
  void unguardedRemoveEntryFromMapSegment(std::shared_ptr<CacheableKey> key);
  typedef std::unordered_map<
      std::shared_ptr<CacheableKey>, std::shared_ptr<TombstoneEntry>,
      dereference_hash<std::shared_ptr<CacheableKey>>,
      dereference_equal_to<std::shared_ptr<CacheableKey>>>
      TombstoneMap;
  TombstoneMap m_tombstoneMap;
  MapSegment* m_mapSegment;
  CacheImpl* m_cacheImpl;
  friend class TombstoneExpiryHandler;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TOMBSTONELIST_H_
