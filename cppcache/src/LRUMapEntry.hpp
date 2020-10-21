#pragma once

#ifndef GEODE_LRUMAPENTRY_H_
#define GEODE_LRUMAPENTRY_H_

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

#include <geode/CacheableKey.hpp>
#include <geode/internal/geode_globals.hpp>

#include "LRUList.hpp"
#include "MapEntryImpl.hpp"
#include "VersionStamp.hpp"

namespace apache {
namespace geode {
namespace client {
/**
 * This template class adds the recently used, eviction bits and persistence
 * info to the MapEntry class. The earlier design looked like below:
 *    LRUListNode     MapEntry
 *          \           /
 *           \         /
 *            \       /
 *           LRUMapEntry
 * This kept the implementation of LRUListNode independent from MapEntry
 * and let create LRUMapEntry using virtual MI to work as expected.
 * However, having LRUMapEntry as a list node meant that node properties
 * also went into the EntriesMap causing problems during unbind_all of
 * the MapEntry which would try to recursively destroy its successor
 * (bug #226). The primary problem with this design is that there is
 * no reason for a MapEntry to inherit node like properties of having
 * a successor. So we will like to split out list node functionality
 * to be inside the LRUList class, while just adding LRU bits to the
 * MapEntry. So we will like to have something like:
 *      ------------------------------
 *      |         LRUListNode        |
 *      |                            |
 *      |  LRUEntryProperties        | MapEntryImpl
 *      |       \                    |   /
 *      |        \           --------|---
 *      |         \         /        |
 *      |         LRUMapEntry        |
 *      |----------------------------|
 *
 *
 */
class APACHE_GEODE_EXPORT LRUMapEntry : public MapEntryImpl,
                                        public LRUEntryProperties {
 public:
  virtual ~LRUMapEntry() {}

  virtual LRUEntryProperties& getLRUProperties() { return *this; }

  virtual void cleanup(const CacheEventFlags eventFlags) {
    if (!eventFlags.isEviction()) {
      // TODO:  this needs an implementation of doubly-linked list
      // to remove from the list; also add this to LRUExpMapEntry since MI
      // has been removed
    }
  }

 protected:
  inline explicit LRUMapEntry(bool)
      : MapEntryImpl(true), LRUEntryProperties(true) {}

  inline explicit LRUMapEntry(const std::shared_ptr<CacheableKey>& key)
      : MapEntryImpl(key) {}

 private:
  // disabled
  LRUMapEntry(const LRUMapEntry&);
  LRUMapEntry& operator=(const LRUMapEntry&);
};

class APACHE_GEODE_EXPORT VersionedLRUMapEntry : public LRUMapEntry,
                                                 public VersionStamp {
 public:
  virtual ~VersionedLRUMapEntry() {}

  virtual VersionStamp& getVersionStamp() { return *this; }

 protected:
  inline explicit VersionedLRUMapEntry(bool) : LRUMapEntry(true) {}

  inline explicit VersionedLRUMapEntry(const std::shared_ptr<CacheableKey>& key)
      : LRUMapEntry(key) {}

 private:
  // disabled
  VersionedLRUMapEntry(const VersionedLRUMapEntry&);
  VersionedLRUMapEntry& operator=(const VersionedLRUMapEntry&);
};

class APACHE_GEODE_EXPORT LRUEntryFactory : public EntryFactory {
 public:
  using EntryFactory::EntryFactory;

  virtual ~LRUEntryFactory() {}

  virtual void newMapEntry(ExpiryTaskManager* expiryTaskManager,
                           const std::shared_ptr<CacheableKey>& key,
                           std::shared_ptr<MapEntryImpl>& result) const;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_LRUMAPENTRY_H_
