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
#include <geode/geode_globals.hpp>
#include <geode/Cache.hpp>
#include "ConcurrentEntriesMap.hpp"
#include "LRUAction.hpp"
#include "LRUList.hpp"
#include "LRUMapEntry.hpp"
#include "MapEntryT.hpp"

#include "util/concurrent/spinlock_mutex.hpp"

#include "NonCopyable.hpp"

namespace apache {
namespace geode {
namespace client {
class EvictionController;

/**
 * @brief Concurrent entries map with LRU behavior.
 * Not designed for subclassing...
 */

/* adongre
 * CID 28728: Other violation (MISSING_COPY)
 * Class "apache::geode::client::LRUEntriesMap" owns resources that are managed
 * in its
 * constructor and destructor but has no user-written copy constructor.
 *
 * FIX : Make the class non copyable
 *
 * CID 28714: Other violation (MISSING_ASSIGN)
 * Class "apache::geode::client::LRUEntriesMap" owns resources that are managed
 * in
 * its constructor and destructor but has no user-written assignment operator.
 * Fix : Make the class Non Assinable
 */
class CPPCACHE_EXPORT LRUEntriesMap : public ConcurrentEntriesMap,
                                      private NonCopyable,
                                      private NonAssignable {
 protected:
  LRUAction* m_action;
  LRUList<MapEntryImpl, MapEntryT<LRUMapEntry, 0, 0> > m_lruList;
  uint32_t m_limit;
  std::shared_ptr<PersistenceManager> m_pmPtr;
  EvictionController* m_evictionControllerPtr;
  int64_t m_currentMapSize;
  spinlock_mutex m_mapInfoLock;
  std::string m_name;
  std::atomic<uint32_t> m_validEntries;
  bool m_heapLRUEnabled;

 public:
  LRUEntriesMap(ExpiryTaskManager* expiryTaskManager,
                std::unique_ptr<EntryFactory> entryFactory,
                RegionInternal* region, const LRUAction::Action& lruAction,
                const uint32_t limit, bool concurrencyChecksEnabled,
                const uint8_t concurrency = 16, bool heapLRUEnabled = false);

  virtual ~LRUEntriesMap();

  virtual GfErrType put(const std::shared_ptr<CacheableKey>& key,
                        const std::shared_ptr<Cacheable>& newValue, std::shared_ptr<MapEntryImpl>& me,
                        std::shared_ptr<Cacheable>& oldValue, int updateCount,
                        int destroyTracker, std::shared_ptr<VersionTag> versionTag,
                        bool& isUpdate = EntriesMap::boolVal,
                        DataInput* delta = nullptr);
  virtual GfErrType invalidate(const std::shared_ptr<CacheableKey>& key, std::shared_ptr<MapEntryImpl>& me,
                               std::shared_ptr<Cacheable>& oldValue,
                               std::shared_ptr<VersionTag> versionTag);
  virtual GfErrType create(const std::shared_ptr<CacheableKey>& key,
                           const std::shared_ptr<Cacheable>& newValue, std::shared_ptr<MapEntryImpl>& me,
                           std::shared_ptr<Cacheable>& oldValue, int updateCount,
                           int destroyTracker, std::shared_ptr<VersionTag> versionTag);
  virtual bool get(const std::shared_ptr<CacheableKey>& key, std::shared_ptr<Cacheable>& returnPtr,
                   std::shared_ptr<MapEntryImpl>& me);
  virtual std::shared_ptr<Cacheable> getFromDisk(const std::shared_ptr<CacheableKey>& key,
                                   std::shared_ptr<MapEntryImpl>& me) const;
  GfErrType processLRU();
  void processLRU(int32_t numEntriesToEvict);
  GfErrType evictionHelper();
  void updateMapSize(int64_t size);
  inline void setPersistenceManager(std::shared_ptr<PersistenceManager>& pmPtr) {
    m_pmPtr = pmPtr;
  }

  /**
   * @brief remove an entry, marking it evicted for LRUList maintainance.
   */
  virtual GfErrType remove(const std::shared_ptr<CacheableKey>& key, std::shared_ptr<Cacheable>& result,
                           std::shared_ptr<MapEntryImpl>& me, int updateCount,
                           std::shared_ptr<VersionTag> versionTag, bool afterRemote);

  virtual void close();

  inline bool mustEvict() const {
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

  inline uint32_t validEntriesSize() const { return m_validEntries; }

  inline void adjustLimit(uint32_t limit) { m_limit = limit; }

  virtual void clear();

};  // class LRUEntriesMap
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_LRUENTRIESMAP_H_
