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

#include <geode/geode_globals.hpp>
#include "MapEntry.hpp"
#include "LRUList.hpp"
#include "VersionStamp.hpp"

namespace apache {
namespace geode {
namespace client {
/**
 * @brief Hold region mapped entry value and lru information.
 */
class CPPCACHE_EXPORT LRUExpMapEntry : public MapEntryImpl,
                                       public LRUEntryProperties,
                                       public ExpEntryProperties {
 public:
  virtual ~LRUExpMapEntry() {}

  virtual LRUEntryProperties& getLRUProperties() { return *this; }

  virtual ExpEntryProperties& getExpProperties() { return *this; }

  virtual void cleanup(const CacheEventFlags eventFlags) {
    if (!eventFlags.isExpiration()) {
      cancelExpiryTaskId(m_key);
    }
  }

 protected:
  inline explicit LRUExpMapEntry(bool noInit)
      : MapEntryImpl(true),
        LRUEntryProperties(true),
        ExpEntryProperties(true) {}

  inline LRUExpMapEntry(ExpiryTaskManager* expiryTaskManager,
                        const CacheableKeyPtr& key)
      : MapEntryImpl(key), ExpEntryProperties(expiryTaskManager) {}

 private:
  // disabled
  LRUExpMapEntry(const LRUExpMapEntry&);
  LRUExpMapEntry& operator=(const LRUExpMapEntry&);
};

typedef std::shared_ptr<LRUExpMapEntry> LRUExpMapEntryPtr;

class CPPCACHE_EXPORT VersionedLRUExpMapEntry : public LRUExpMapEntry,
                                                public VersionStamp {
 public:
  virtual ~VersionedLRUExpMapEntry() {}

  virtual VersionStamp& getVersionStamp() { return *this; }

 protected:
  inline explicit VersionedLRUExpMapEntry(bool noInit) : LRUExpMapEntry(true) {}

  inline VersionedLRUExpMapEntry(ExpiryTaskManager* expiryTaskManager,
                                 const CacheableKeyPtr& key)
      : LRUExpMapEntry(expiryTaskManager, key) {}

 private:
  // disabled
  VersionedLRUExpMapEntry(const VersionedLRUExpMapEntry&);
  VersionedLRUExpMapEntry& operator=(const VersionedLRUExpMapEntry&);
};

typedef std::shared_ptr<VersionedLRUExpMapEntry> VersionedLRUExpMapEntryPtr;

class CPPCACHE_EXPORT LRUExpEntryFactory : public EntryFactory {
 public:
  using EntryFactory::EntryFactory;

  virtual ~LRUExpEntryFactory() {}

  virtual void newMapEntry(ExpiryTaskManager* expiryTaskManager,
                           const CacheableKeyPtr& key,
                           MapEntryImplPtr& result) const;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_LRUEXPMAPENTRY_H_
