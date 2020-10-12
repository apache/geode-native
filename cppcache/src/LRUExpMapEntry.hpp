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
#include "LRUList.hpp"
#include "MapEntryImpl.hpp"
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
  virtual ~LRUExpMapEntry() {}

  virtual LRUEntryProperties& getLRUProperties() { return *this; }

  virtual ExpEntryProperties& getExpProperties() { return *this; }

  virtual void cleanup(const CacheEventFlags eventFlags) {
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

 private:
  // disabled
  LRUExpMapEntry(const LRUExpMapEntry&);
  LRUExpMapEntry& operator=(const LRUExpMapEntry&);
};

class APACHE_GEODE_EXPORT VersionedLRUExpMapEntry : public LRUExpMapEntry,
                                                    public VersionStamp {
 public:
  virtual ~VersionedLRUExpMapEntry() {}

  virtual VersionStamp& getVersionStamp() { return *this; }

 protected:
  inline explicit VersionedLRUExpMapEntry(bool) : LRUExpMapEntry(true) {}

  inline VersionedLRUExpMapEntry(ExpiryTaskManager* expiryTaskManager,
                                 const std::shared_ptr<CacheableKey>& key)
      : LRUExpMapEntry(expiryTaskManager, key) {}

 private:
  // disabled
  VersionedLRUExpMapEntry(const VersionedLRUExpMapEntry&);
  VersionedLRUExpMapEntry& operator=(const VersionedLRUExpMapEntry&);
};

class APACHE_GEODE_EXPORT LRUExpEntryFactory : public EntryFactory {
 public:
  using EntryFactory::EntryFactory;

  virtual ~LRUExpEntryFactory() {}

  virtual void newMapEntry(ExpiryTaskManager* expiryTaskManager,
                           const std::shared_ptr<CacheableKey>& key,
                           std::shared_ptr<MapEntryImpl>& result) const;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_LRUEXPMAPENTRY_H_
