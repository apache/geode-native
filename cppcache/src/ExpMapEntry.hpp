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

#ifndef GEODE_EXPMAPENTRY_H_
#define GEODE_EXPMAPENTRY_H_

#include <geode/geode_globals.hpp>
#include "MapEntry.hpp"
#include "VersionStamp.hpp"

namespace apache {
namespace geode {
namespace client {
/**
 * @brief Hold region mapped entry value.
 * This subclass adds expiration times.
 */
class CPPCACHE_EXPORT ExpMapEntry : public MapEntryImpl,
                                    public ExpEntryProperties {
 public:
  virtual ~ExpMapEntry() {}

  virtual ExpEntryProperties& getExpProperties() { return *this; }

  virtual void cleanup(const CacheEventFlags eventFlags) {
    if (!eventFlags.isExpiration()) {
      cancelExpiryTaskId(m_key);
    }
  }

 protected:
  // this constructor deliberately skips touching or initializing any members
  inline explicit ExpMapEntry(bool noInit)
      : MapEntryImpl(true), ExpEntryProperties(true) {}

  inline ExpMapEntry(ExpiryTaskManager* expiryTaskManager,
                     const CacheableKeyPtr& key)
      : MapEntryImpl(key), ExpEntryProperties(expiryTaskManager) {}

 private:
  // disabled
  ExpMapEntry(const ExpMapEntry&);
  ExpMapEntry& operator=(const ExpMapEntry&);
};

typedef std::shared_ptr<ExpMapEntry> ExpMapEntryPtr;

class CPPCACHE_EXPORT VersionedExpMapEntry : public ExpMapEntry,
                                             public VersionStamp {
 public:
  virtual ~VersionedExpMapEntry() {}

  virtual VersionStamp& getVersionStamp() { return *this; }

 protected:
  inline explicit VersionedExpMapEntry(bool noInit) : ExpMapEntry(true) {}

  inline VersionedExpMapEntry(ExpiryTaskManager* expiryTaskManager,
                              const CacheableKeyPtr& key)
      : ExpMapEntry(expiryTaskManager, key) {}

 private:
  // disabled
  VersionedExpMapEntry(const VersionedExpMapEntry&);
  VersionedExpMapEntry& operator=(const VersionedExpMapEntry&);
};

typedef std::shared_ptr<VersionedExpMapEntry> VersionedExpMapEntryPtr;

class CPPCACHE_EXPORT ExpEntryFactory : public EntryFactory {
 public:
  using EntryFactory::EntryFactory;

  virtual ~ExpEntryFactory() {}

  virtual void newMapEntry(ExpiryTaskManager* expiryTaskManager,
                           const CacheableKeyPtr& key,
                           MapEntryImplPtr& result) const;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_EXPMAPENTRY_H_
