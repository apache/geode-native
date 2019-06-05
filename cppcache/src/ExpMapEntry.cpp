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

#include "ExpMapEntry.hpp"

#include "MapEntryT.hpp"

namespace apache {
namespace geode {
namespace client {

ExpMapEntry::~ExpMapEntry() {}

ExpEntryProperties& ExpMapEntry::getExpProperties() { return *this; }

void ExpMapEntry::cleanup(const CacheEventFlags eventFlags) {
    if (!eventFlags.isExpiration()) {
      cancelExpiryTaskId(m_key);
    }
  }

ExpMapEntry::ExpMapEntry(bool)
      : MapEntryImpl(true), ExpEntryProperties(true) {}

ExpMapEntry::ExpMapEntry(ExpiryTaskManager* expiryTaskManager,
                     const std::shared_ptr<CacheableKey>& key)
      : MapEntryImpl(key), ExpEntryProperties(expiryTaskManager) {}

VersionedExpMapEntry::VersionedExpMapEntry(ExpiryTaskManager* expiryTaskManager,
                              const std::shared_ptr<CacheableKey>& key)
      : ExpMapEntry(expiryTaskManager, key) {}

VersionedExpMapEntry::VersionedExpMapEntry(bool) : ExpMapEntry(true) {}

VersionedExpMapEntry::~VersionedExpMapEntry() {}

VersionStamp& VersionedExpMapEntry::getVersionStamp() { return *this; }

ExpEntryFactory::~ExpEntryFactory() {}

void ExpEntryFactory::newMapEntry(ExpiryTaskManager* expiryTaskManager,
                                  const std::shared_ptr<CacheableKey>& key,
                                  std::shared_ptr<MapEntryImpl>& result) const {
  if (m_concurrencyChecksEnabled) {
    result =
        MapEntryT<VersionedExpMapEntry, 0, 0>::create(expiryTaskManager, key);
  } else {
    result = MapEntryT<ExpMapEntry, 0, 0>::create(expiryTaskManager, key);
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
