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

#ifndef GEODE_LRUEXPMAPENTRY_H_
#define GEODE_LRUEXPMAPENTRY_H_

#include <geode/internal/geode_globals.hpp>

#include "ExpEntryProperties.hpp"
#include "LRUEntryProperties.hpp"
#include "MapEntryImpl.hpp"
#include "VersionStamp.hpp"

namespace apache {
namespace geode {
namespace client {
/**
 * @brief Hold region mapped entry value and lru information.
 */
class LRUExpMapEntry : public MapEntryImpl,
                       public LRUEntryProperties,
                       public ExpEntryProperties {
 public:
  LRUExpMapEntry(const LRUExpMapEntry&) = delete;
  LRUExpMapEntry& operator=(const LRUExpMapEntry&) = delete;

  ~LRUExpMapEntry() noexcept override = default;

  LRUEntryProperties& getLRUProperties() override { return *this; }

  ExpEntryProperties& getExpProperties() override { return *this; }

  void cleanup(const CacheEventFlags eventFlags) override {
    if (!eventFlags.isExpiration()) {
      cancel_task();
    }
  }

 protected:
  inline explicit LRUExpMapEntry(bool)
      : MapEntryImpl(true),
        LRUEntryProperties(true),
        ExpEntryProperties(true) {}

  inline LRUExpMapEntry(ExpiryTaskManager* expiryTaskManager,
                        const std::shared_ptr<CacheableKey>& key)
      : MapEntryImpl(key), ExpEntryProperties(expiryTaskManager) {}
};

class VersionedLRUExpMapEntry : public LRUExpMapEntry, public VersionStamp {
 public:
  VersionedLRUExpMapEntry(const VersionedLRUExpMapEntry&) = delete;
  VersionedLRUExpMapEntry& operator=(const VersionedLRUExpMapEntry&) = delete;

  ~VersionedLRUExpMapEntry() noexcept override = default;

  VersionStamp& getVersionStamp() override { return *this; }

 protected:
  inline explicit VersionedLRUExpMapEntry(bool) : LRUExpMapEntry(true) {}

  inline VersionedLRUExpMapEntry(ExpiryTaskManager* expiryTaskManager,
                                 const std::shared_ptr<CacheableKey>& key)
      : LRUExpMapEntry(expiryTaskManager, key) {}
};

class LRUExpEntryFactory : public EntryFactory {
 public:
  using EntryFactory::EntryFactory;

  ~LRUExpEntryFactory() noexcept override = default;

  void newMapEntry(ExpiryTaskManager* expiryTaskManager,
                   const std::shared_ptr<CacheableKey>& key,
                   std::shared_ptr<MapEntryImpl>& result) const override;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_LRUEXPMAPENTRY_H_
