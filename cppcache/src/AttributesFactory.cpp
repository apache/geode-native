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

#include <geode/Cache.hpp>
#include <geode/ExpirationAttributes.hpp>
#include <Utils.hpp>
#include <geode/DistributedSystem.hpp>
#include <cstdlib>
#include <string>
#include <geode/Pool.hpp>
#include <geode/PoolManager.hpp>

namespace apache {
namespace geode {
namespace client {

class Region;

AttributesFactory::AttributesFactory() : m_regionAttributes() {}

AttributesFactory::AttributesFactory(
    const std::shared_ptr<RegionAttributes>& regionAttributes)
    : m_regionAttributes(*regionAttributes) {}

AttributesFactory::~AttributesFactory() {}

AttributesFactory& AttributesFactory::setCacheLoader(
    const std::shared_ptr<CacheLoader>& cacheLoader) {
  m_regionAttributes.m_cacheLoader = cacheLoader;
  return *this;
}

AttributesFactory& AttributesFactory::setCacheWriter(
    const std::shared_ptr<CacheWriter>& cacheWriter) {
  m_regionAttributes.m_cacheWriter = cacheWriter;
  return *this;
}
AttributesFactory& AttributesFactory::setCacheListener(
    const std::shared_ptr<CacheListener>& aListener) {
  m_regionAttributes.m_cacheListener = aListener;
  return *this;
}
AttributesFactory& AttributesFactory::setPartitionResolver(
    const std::shared_ptr<PartitionResolver>& aResolver) {
  m_regionAttributes.m_partitionResolver = aResolver;
  return *this;
}

AttributesFactory& AttributesFactory::setCacheLoader(const char* lib,
                                                     const char* func) {
  m_regionAttributes.setCacheLoader(lib, func);
  return *this;
}

AttributesFactory& AttributesFactory::setCacheWriter(const char* lib,
                                                     const char* func) {
  m_regionAttributes.setCacheWriter(lib, func);
  return *this;
}

AttributesFactory& AttributesFactory::setCacheListener(const char* lib,
                                                       const char* func) {
  m_regionAttributes.setCacheListener(lib, func);
  return *this;
}

AttributesFactory& AttributesFactory::setPartitionResolver(const char* lib,
                                                           const char* func) {
  m_regionAttributes.setPartitionResolver(lib, func);
  return *this;
}

AttributesFactory& AttributesFactory::setEntryIdleTimeout(
    ExpirationAction::Action action, std::chrono::seconds idleTimeout) {
  m_regionAttributes.m_entryIdleTimeout = idleTimeout;
  m_regionAttributes.m_entryIdleTimeoutExpirationAction = action;
  return *this;
}

AttributesFactory& AttributesFactory::setEntryTimeToLive(
    ExpirationAction::Action action, std::chrono::seconds timeToLive) {
  m_regionAttributes.m_entryTimeToLive = timeToLive;
  m_regionAttributes.m_entryTimeToLiveExpirationAction = action;
  return *this;
}

AttributesFactory& AttributesFactory::setRegionIdleTimeout(
    ExpirationAction::Action action, std::chrono::seconds idleTimeout) {
  m_regionAttributes.m_regionIdleTimeout = idleTimeout;
  m_regionAttributes.m_regionIdleTimeoutExpirationAction = action;
  return *this;
}

AttributesFactory& AttributesFactory::setRegionTimeToLive(
    ExpirationAction::Action action, std::chrono::seconds timeToLive) {
  m_regionAttributes.m_regionTimeToLive = timeToLive;
  m_regionAttributes.m_regionTimeToLiveExpirationAction = action;
  return *this;
}

AttributesFactory& AttributesFactory::setInitialCapacity(int initialCapacity) {
  m_regionAttributes.m_initialCapacity = initialCapacity;
  return *this;
}

AttributesFactory& AttributesFactory::setLoadFactor(float loadFactor) {
  m_regionAttributes.m_loadFactor = loadFactor;
  return *this;
}

AttributesFactory& AttributesFactory::setConcurrencyLevel(
    uint8_t concurrencyLevel) {
  m_regionAttributes.m_concurrencyLevel = concurrencyLevel;
  return *this;
}

std::unique_ptr<RegionAttributes> AttributesFactory::createRegionAttributes() {
  std::shared_ptr<RegionAttributes> res;
  validateAttributes(m_regionAttributes);
  return std::unique_ptr<RegionAttributes>(
      new RegionAttributes(m_regionAttributes));
}

void AttributesFactory::validateAttributes(RegionAttributes& attrs) {
  if (!attrs.m_caching) {
    if (attrs.m_entryTimeToLive != std::chrono::seconds::zero()) {
      throw IllegalStateException(
          "Entry TimeToLive use is incompatible with disabled caching");
    }

    if (attrs.m_entryIdleTimeout != std::chrono::seconds::zero()) {
      throw IllegalStateException(
          "Entry IdleTimeout use is incompatible with disabled caching");
    }

    if (attrs.m_lruEntriesLimit != 0) {
      throw IllegalStateException(
          "Non-zero LRU entries limit is incompatible with disabled caching");
    }
    if (attrs.m_diskPolicy != DiskPolicyType::NONE) {
      if (attrs.m_lruEntriesLimit == 0) {
        throw IllegalStateException(
            "When DiskPolicy is OVERFLOWS, LRU entries limit must be non-zero "
            "with disabled caching");
      }
    }
  }

  if (attrs.m_diskPolicy != DiskPolicyType::NONE) {
    if (attrs.m_persistenceManager == nullptr &&
        (attrs.m_persistenceLibrary == nullptr ||
         attrs.m_persistenceFactory == nullptr)) {
      throw IllegalStateException(
          "Persistence Manager must be set if DiskPolicy is OVERFLOWS");
    }
  }
  if (attrs.m_diskPolicy != DiskPolicyType::NONE) {
    if (attrs.m_lruEntriesLimit == 0) {
      throw IllegalStateException(
          "LRU entries limit cannot be zero if DiskPolicy is OVERFLOWS");
    }
  }
}

AttributesFactory& AttributesFactory::setLruEntriesLimit(
    const uint32_t entriesLimit) {
  m_regionAttributes.m_lruEntriesLimit = entriesLimit;
  return *this;
}

AttributesFactory& AttributesFactory::setDiskPolicy(
    const DiskPolicyType::PolicyType diskPolicy) {
  if (diskPolicy == DiskPolicyType::PERSIST) {
    throw IllegalStateException("Persistence feature is not supported");
  }
  m_regionAttributes.m_diskPolicy = diskPolicy;
  return *this;
}

AttributesFactory& AttributesFactory::setCachingEnabled(bool cachingEnabled) {
  m_regionAttributes.m_caching = cachingEnabled;
  return *this;
}

AttributesFactory& AttributesFactory::setPersistenceManager(
    const std::shared_ptr<PersistenceManager>& persistenceManager,
    const std::shared_ptr<Properties>& props) {
  m_regionAttributes.m_persistenceManager = persistenceManager;
  m_regionAttributes.m_persistenceProperties = props;
  return *this;
}

AttributesFactory& AttributesFactory::setPersistenceManager(
    const char* lib, const char* func,
    const std::shared_ptr<Properties>& config) {
  m_regionAttributes.setPersistenceManager(lib, func, config);
  return *this;
}

AttributesFactory& AttributesFactory::setPoolName(const char* name) {
  m_regionAttributes.setPoolName(name);
  return *this;
}

AttributesFactory& AttributesFactory::setCloningEnabled(bool isClonable) {
  m_regionAttributes.setCloningEnabled(isClonable);
  return *this;
}
AttributesFactory& AttributesFactory::setConcurrencyChecksEnabled(bool enable) {
  m_regionAttributes.setConcurrencyChecksEnabled(enable);
  return *this;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
