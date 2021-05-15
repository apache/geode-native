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

#ifndef GEODE_LRUENTRIESMAP_H_
#define GEODE_LRUENTRIESMAP_H_

#include <atomic>

#include <geode/Cache.hpp>
#include <geode/internal/geode_globals.hpp>

#include "ConcurrentEntriesMap.hpp"
#include "LRUAction.hpp"
#include "LRUMapEntry.hpp"
#include "LRUQueue.hpp"
#include "MapEntryT.hpp"
#include "util/concurrent/spinlock_mutex.hpp"

namespace apache {
namespace geode {
namespace client {

class EvictionController;

/**
 * @brief Concurrent entries map with LRU behavior.
 * Not designed for subclassing...
 */
class LRUEntriesMap : public ConcurrentEntriesMap {
 protected:
  using spinlock_mutex = ::apache::geode::util::concurrent::spinlock_mutex;

 protected:
  LRUAction* m_action;
  LRUQueue lru_queue_;
  uint32_t m_limit;
  std::shared_ptr<PersistenceManager> m_pmPtr;
  EvictionController* m_evictionControllerPtr;
  std::atomic<int64_t> m_currentMapSize;
  std::string m_name;
  std::atomic<uint32_t> m_validEntries;
  bool m_heapLRUEnabled;

 public:
  LRUEntriesMap(const LRUEntriesMap&) = delete;
  LRUEntriesMap& operator=(const LRUEntriesMap&) = delete;
  LRUEntriesMap(ExpiryTaskManager* expiryTaskManager,
                std::unique_ptr<EntryFactory> entryFactory,
                RegionInternal* region, const LRUAction::Action& lruAction,
                const uint32_t limit, bool concurrencyChecksEnabled,
                const uint8_t concurrency = 16, bool heapLRUEnabled = false);

  ~LRUEntriesMap() noexcept override;

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
  std::shared_ptr<Cacheable> getFromDisk(
      const std::shared_ptr<CacheableKey>& key,
      std::shared_ptr<MapEntryImpl>& me) const override;
  GfErrType processLRU();
  void processLRU(int32_t numEntriesToEvict);
  GfErrType evictionHelper();
  void updateMapSize(int64_t size);
  inline void setPersistenceManager(
      std::shared_ptr<PersistenceManager>& pmPtr) {
    m_pmPtr = pmPtr;
  }

  GfErrType remove(const std::shared_ptr<CacheableKey>& key,
                   std::shared_ptr<Cacheable>& result,
                   std::shared_ptr<MapEntryImpl>& me, int updateCount,
                   std::shared_ptr<VersionTag> versionTag,
                   bool afterRemote) override;
  void close() override;

  inline bool mustEvict() const {
    if (m_action == nullptr) {
      LOG_FINE("Eviction action is nullptr");
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

  inline uint32_t validEntriesSize() const { return m_validEntries; }

  inline void adjustLimit(uint32_t limit) { m_limit = limit; }

  void clear() override;

};  // class LRUEntriesMap

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_LRUENTRIESMAP_H_
