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

#include "LRUList.hpp"
#include "MapEntry.hpp"
#include "VersionStamp.hpp"

namespace apache {
namespace geode {
namespace client {
/**
 * @brief Hold region mapped entry value and lru information.
 */
class APACHE_GEODE_EXPORT LRUExpMapEntry : public MapEntryImpl,
                                           public LRUEntryProperties,
                                           public ExpEntryProperties {
 public:
  ~LRUExpMapEntry() override = default;

  LRUEntryProperties& getLRUProperties() override;

  ExpEntryProperties& getExpProperties() override;

  void cleanup(const CacheEventFlags eventFlags) override;

 protected:
  LRUExpMapEntry(bool);

  LRUExpMapEntry(ExpiryTaskManager* expiryTaskManager,
                 const std::shared_ptr<CacheableKey>& key);

 private:
  // disabled
  LRUExpMapEntry(const LRUExpMapEntry&);
  LRUExpMapEntry& operator=(const LRUExpMapEntry&);
};

class APACHE_GEODE_EXPORT VersionedLRUExpMapEntry : public LRUExpMapEntry,
                                                    public VersionStamp {
 public:
  ~VersionedLRUExpMapEntry() override = default;

  VersionStamp& getVersionStamp() override;

 protected:
  VersionedLRUExpMapEntry(bool);

  VersionedLRUExpMapEntry(ExpiryTaskManager* expiryTaskManager,
                          const std::shared_ptr<CacheableKey>& key);

 private:
  // disabled
  VersionedLRUExpMapEntry(const VersionedLRUExpMapEntry&);
  VersionedLRUExpMapEntry& operator=(const VersionedLRUExpMapEntry&);
};

class APACHE_GEODE_EXPORT LRUExpEntryFactory : public EntryFactory {
 public:
  using EntryFactory::EntryFactory;

  ~LRUExpEntryFactory() override = default;

  void newMapEntry(ExpiryTaskManager* expiryTaskManager,
                   const std::shared_ptr<CacheableKey>& key,
                   std::shared_ptr<MapEntryImpl>& result) const override;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_LRUEXPMAPENTRY_H_
