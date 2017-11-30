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

#include "RegionInternal.hpp"
#include <geode/RegionEntry.hpp>
#include "TombstoneList.hpp"

namespace apache {
namespace geode {
namespace client {

// Static initializers for CacheEventFlags
const CacheEventFlags CacheEventFlags::NORMAL(CacheEventFlags::GF_NORMAL);
const CacheEventFlags CacheEventFlags::LOCAL(CacheEventFlags::GF_LOCAL);
const CacheEventFlags CacheEventFlags::NOTIFICATION(
    CacheEventFlags::GF_NOTIFICATION);
const CacheEventFlags CacheEventFlags::NOTIFICATION_UPDATE(
    CacheEventFlags::GF_NOTIFICATION_UPDATE);
const CacheEventFlags CacheEventFlags::EVICTION(CacheEventFlags::GF_EVICTION);
const CacheEventFlags CacheEventFlags::EXPIRATION(
    CacheEventFlags::GF_EXPIRATION);
const CacheEventFlags CacheEventFlags::CACHE_CLOSE(
    CacheEventFlags::GF_CACHE_CLOSE);
const CacheEventFlags CacheEventFlags::NOCACHEWRITER(
    CacheEventFlags::GF_NOCACHEWRITER);

RegionInternal::RegionInternal(
    Cache* cache,
    const std::shared_ptr<RegionAttributes>& attributes)
    : Region(cache), m_regionAttributes(attributes) {}

RegionInternal::~RegionInternal() {}

void RegionInternal::registerKeys(
    const std::vector<std::shared_ptr<CacheableKey>>& keys, bool isDurable,
    bool getInitialValues, bool receiveValues) {
  throw UnsupportedOperationException(
      "registerKeys only supported by "
      "Thin Client Region.");
}

void RegionInternal::unregisterKeys(
    const std::vector<std::shared_ptr<CacheableKey>>& keys) {
  throw UnsupportedOperationException(
      "unregisterKeys only supported by "
      "Thin Client Region.");
}

void RegionInternal::registerAllKeys(bool isDurable,
                                     bool getInitialValues,
                                     bool receiveValues) {
  throw UnsupportedOperationException(
      "registerAllKeys only supported by Thin Client Region.");
}

void RegionInternal::unregisterAllKeys() {
  throw UnsupportedOperationException(
      "unregisterAllKeys only supported by Thin Client Region.");
}

void RegionInternal::registerRegex(const char* regex, bool isDurable,
                                   bool getInitialValues, bool receiveValues) {
  throw UnsupportedOperationException(
      "registerRegex only supported by Thin Client Region.");
}

void RegionInternal::unregisterRegex(const char* regex) {
  throw UnsupportedOperationException(
      "unregisterRegex only supported by Thin Client Region.");
}
std::shared_ptr<SelectResults> RegionInternal::query(
    const char* predicate, std::chrono::milliseconds timeout) {
  throw UnsupportedOperationException(
      "query only supported by Thin Client Region.");
}

bool RegionInternal::existsValue(const char* predicate,
                                 std::chrono::milliseconds timeout) {
  throw UnsupportedOperationException(
      "existsValue only supported by Thin Client Region.");
}
std::shared_ptr<Serializable> RegionInternal::selectValue(
    const char* predicate, std::chrono::milliseconds timeout) {
  throw UnsupportedOperationException(
      "selectValue only supported by Thin Client Region.");
}
std::shared_ptr<TombstoneList> RegionInternal::getTombstoneList() {
  throw UnsupportedOperationException(
      "getTombstoneList only supported by LocalRegion.");
}
std::shared_ptr<RegionEntry> RegionInternal::createRegionEntry(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Cacheable>& value) {
  return std::make_shared<RegionEntry>(shared_from_this(), key, value);
}

void RegionInternal::setLruEntriesLimit(uint32_t limit) {
  if (m_regionAttributes != nullptr) {
    m_regionAttributes->m_lruEntriesLimit = limit;
  }
}

void RegionInternal::setRegionTimeToLiveExpirationAction(
    ExpirationAction::Action action) {
  if (m_regionAttributes != nullptr) {
    m_regionAttributes->m_regionTimeToLiveExpirationAction = action;
  }
}

void RegionInternal::setRegionIdleTimeoutExpirationAction(
    ExpirationAction::Action action) {
  if (m_regionAttributes != nullptr) {
    m_regionAttributes->m_regionIdleTimeoutExpirationAction = action;
  }
}

void RegionInternal::setEntryTimeToLiveExpirationAction(
    ExpirationAction::Action action) {
  if (m_regionAttributes != nullptr) {
    m_regionAttributes->m_entryTimeToLiveExpirationAction = action;
  }
}

void RegionInternal::setEntryIdleTimeoutExpirationAction(
    ExpirationAction::Action action) {
  if (m_regionAttributes != nullptr) {
    m_regionAttributes->m_entryIdleTimeoutExpirationAction = action;
  }
}

void RegionInternal::setRegionTimeToLive(const std::chrono::seconds& duration) {
  if (m_regionAttributes != nullptr) {
    m_regionAttributes->m_regionTimeToLive = duration;
  }
}

void RegionInternal::setRegionIdleTimeout(
    const std::chrono::seconds& duration) {
  if (m_regionAttributes != nullptr) {
    m_regionAttributes->m_regionIdleTimeout = duration;
  }
}

void RegionInternal::setEntryTimeToLive(const std::chrono::seconds& duration) {
  if (m_regionAttributes != nullptr) {
    m_regionAttributes->m_entryTimeToLive = duration;
  }
}

void RegionInternal::setEntryIdleTimeout(const std::chrono::seconds& duration) {
  if (m_regionAttributes != nullptr) {
    m_regionAttributes->m_entryIdleTimeout = duration;
  }
}

void RegionInternal::setCacheListener(const std::shared_ptr<CacheListener>& aListener) {
  if (m_regionAttributes != nullptr) {
    m_regionAttributes->m_cacheListener = aListener;
  }
}

void RegionInternal::setCacheListener(const char* libpath,
                                      const char* factoryFuncName) {
  if (m_regionAttributes != nullptr) {
    m_regionAttributes->setCacheListener(libpath, factoryFuncName);
  }
}

void RegionInternal::setPartitionResolver(
    const std::shared_ptr<PartitionResolver>& aResolver) {
  if (m_regionAttributes != nullptr) {
    m_regionAttributes->m_partitionResolver = aResolver;
  }
}

void RegionInternal::setPartitionResolver(const char* libpath,
                                          const char* factoryFuncName) {
  if (m_regionAttributes != nullptr) {
    m_regionAttributes->setPartitionResolver(libpath, factoryFuncName);
  }
}

void RegionInternal::setCacheLoader(const std::shared_ptr<CacheLoader>& aLoader) {
  if (m_regionAttributes != nullptr) {
    m_regionAttributes->m_cacheLoader = aLoader;
  }
}

void RegionInternal::setCacheLoader(const char* libpath,
                                    const char* factoryFuncName) {
  if (m_regionAttributes != nullptr) {
    m_regionAttributes->setCacheLoader(libpath, factoryFuncName);
  }
}

void RegionInternal::setCacheWriter(const std::shared_ptr<CacheWriter>& aWriter) {
  if (m_regionAttributes != nullptr) {
    m_regionAttributes->m_cacheWriter = aWriter;
  }
}

void RegionInternal::setCacheWriter(const char* libpath,
                                    const char* factoryFuncName) {
  if (m_regionAttributes != nullptr) {
    m_regionAttributes->setCacheWriter(libpath, factoryFuncName);
  }
}

void RegionInternal::setEndpoints(const char* endpoints) {
  if (m_regionAttributes != nullptr) {
    m_regionAttributes->setEndpoints(endpoints);
  }
}

void RegionInternal::setClientNotificationEnabled(
    bool clientNotificationEnabled) {
  if (m_regionAttributes != nullptr) {
    m_regionAttributes->m_clientNotificationEnabled = clientNotificationEnabled;
  }
}

void RegionInternal::txDestroy(const std::shared_ptr<CacheableKey>& key,
                               const std::shared_ptr<Serializable>& callBack,
                               std::shared_ptr<VersionTag> versionTag) {
  throw UnsupportedOperationException(
      "txDestroy only supported by Thin Client Region.");
}

void RegionInternal::txInvalidate(const std::shared_ptr<CacheableKey>& key,
                                  const std::shared_ptr<Serializable>& callBack,
                                  std::shared_ptr<VersionTag> versionTag) {
  throw UnsupportedOperationException(
      "txInvalidate only supported by Thin Client Region.");
}

void RegionInternal::txPut(const std::shared_ptr<CacheableKey>& key,
                           const std::shared_ptr<Cacheable>& value,
                           const std::shared_ptr<Serializable>& callBack,
                           std::shared_ptr<VersionTag> versionTag) {
  throw UnsupportedOperationException(
      "txPut only supported by Thin Client Region.");
}
}  // namespace client
}  // namespace geode
}  // namespace apache
