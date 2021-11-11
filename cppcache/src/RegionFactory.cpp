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

#include <map>
#include <string>

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/SystemProperties.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "CppCacheLibrary.hpp"

namespace apache {
namespace geode {
namespace client {

RegionFactory::RegionFactory(RegionShortcut preDefinedRegion,
                             CacheImpl* cacheImpl)
    : m_preDefinedRegion(preDefinedRegion),
      m_regionAttributesFactory(std::make_shared<RegionAttributesFactory>()),
      m_cacheImpl(cacheImpl) {
  setRegionShortcut();
}
std::shared_ptr<Region> RegionFactory::create(std::string name) {
  std::shared_ptr<Region> retRegionPtr = nullptr;
  auto regionAttributes = m_regionAttributesFactory->create();
  if (m_preDefinedRegion != RegionShortcut::LOCAL &&
      regionAttributes.getPoolName().empty()) {
    auto pool = m_cacheImpl->getPoolManager().getDefaultPool();
    if (!pool) {
      throw IllegalStateException("No pool for non-local region.");
    }
    m_regionAttributesFactory->setPoolName(pool->getName());
    regionAttributes = m_regionAttributesFactory->create();
  }
  m_cacheImpl->createRegion(name, regionAttributes, retRegionPtr);

  return retRegionPtr;
}

void RegionFactory::setRegionShortcut() {
  switch (m_preDefinedRegion) {
    case RegionShortcut::PROXY: {
      m_regionAttributesFactory->setCachingEnabled(false);
    } break;
    case RegionShortcut::CACHING_PROXY: {
      m_regionAttributesFactory->setCachingEnabled(true);
    } break;
    case RegionShortcut::CACHING_PROXY_ENTRY_LRU: {
      m_regionAttributesFactory->setCachingEnabled(true);
      m_regionAttributesFactory->setLruEntriesLimit(
          DEFAULT_LRU_MAXIMUM_ENTRIES);
    } break;
    case RegionShortcut::LOCAL: {
    } break;
    case RegionShortcut::LOCAL_ENTRY_LRU: {
      m_regionAttributesFactory->setLruEntriesLimit(
          DEFAULT_LRU_MAXIMUM_ENTRIES);
    } break;
  }
}

RegionFactory& RegionFactory::setCacheLoader(
    const std::shared_ptr<CacheLoader>& cacheLoader) {
  m_regionAttributesFactory->setCacheLoader(cacheLoader);
  return *this;
}

RegionFactory& RegionFactory::setCacheWriter(
    const std::shared_ptr<CacheWriter>& cacheWriter) {
  m_regionAttributesFactory->setCacheWriter(cacheWriter);
  return *this;
}
RegionFactory& RegionFactory::setCacheListener(
    const std::shared_ptr<CacheListener>& aListener) {
  m_regionAttributesFactory->setCacheListener(aListener);
  return *this;
}
RegionFactory& RegionFactory::setPartitionResolver(
    const std::shared_ptr<PartitionResolver>& aResolver) {
  m_regionAttributesFactory->setPartitionResolver(aResolver);
  return *this;
}

RegionFactory& RegionFactory::setCacheLoader(const std::string& lib,
                                             const std::string& func) {
  m_regionAttributesFactory->setCacheLoader(lib, func);
  return *this;
}

RegionFactory& RegionFactory::setCacheWriter(const std::string& lib,
                                             const std::string& func) {
  m_regionAttributesFactory->setCacheWriter(lib, func);
  return *this;
}

RegionFactory& RegionFactory::setCacheListener(const std::string& lib,
                                               const std::string& func) {
  m_regionAttributesFactory->setCacheListener(lib, func);
  return *this;
}

RegionFactory& RegionFactory::setPartitionResolver(const std::string& lib,
                                                   const std::string& func) {
  m_regionAttributesFactory->setPartitionResolver(lib, func);
  return *this;
}

RegionFactory& RegionFactory::setEntryIdleTimeout(
    ExpirationAction action, std::chrono::seconds idleTimeout) {
  m_regionAttributesFactory->setEntryIdleTimeout(action, idleTimeout);
  return *this;
}

RegionFactory& RegionFactory::setEntryTimeToLive(
    ExpirationAction action, std::chrono::seconds timeToLive) {
  m_regionAttributesFactory->setEntryTimeToLive(action, timeToLive);
  return *this;
}

RegionFactory& RegionFactory::setRegionIdleTimeout(
    ExpirationAction action, std::chrono::seconds idleTimeout) {
  m_regionAttributesFactory->setRegionIdleTimeout(action, idleTimeout);
  return *this;
}

RegionFactory& RegionFactory::setRegionTimeToLive(
    ExpirationAction action, std::chrono::seconds timeToLive) {
  m_regionAttributesFactory->setRegionTimeToLive(action, timeToLive);
  return *this;
}

RegionFactory& RegionFactory::setInitialCapacity(int initialCapacity) {
  if (initialCapacity < 0) {
    throw IllegalArgumentException("initialCapacity must be >= 0");
  }
  m_regionAttributesFactory->setInitialCapacity(initialCapacity);
  return *this;
}

RegionFactory& RegionFactory::setLoadFactor(float loadFactor) {
  m_regionAttributesFactory->setLoadFactor(loadFactor);
  return *this;
}

RegionFactory& RegionFactory::setConcurrencyLevel(uint8_t concurrencyLevel) {
  m_regionAttributesFactory->setConcurrencyLevel(concurrencyLevel);
  return *this;
}
RegionFactory& RegionFactory::setConcurrencyChecksEnabled(bool enable) {
  m_regionAttributesFactory->setConcurrencyChecksEnabled(enable);
  return *this;
}
RegionFactory& RegionFactory::setLruEntriesLimit(const uint32_t entriesLimit) {
  m_regionAttributesFactory->setLruEntriesLimit(entriesLimit);
  return *this;
}

RegionFactory& RegionFactory::setDiskPolicy(const DiskPolicyType diskPolicy) {
  m_regionAttributesFactory->setDiskPolicy(diskPolicy);
  return *this;
}

RegionFactory& RegionFactory::setCachingEnabled(bool cachingEnabled) {
  m_regionAttributesFactory->setCachingEnabled(cachingEnabled);
  return *this;
}

RegionFactory& RegionFactory::setPersistenceManager(
    const std::shared_ptr<PersistenceManager>& persistenceManager,
    const std::shared_ptr<Properties>& config) {
  m_regionAttributesFactory->setPersistenceManager(persistenceManager, config);
  return *this;
}

RegionFactory& RegionFactory::setPersistenceManager(
    const std::string& lib, const std::string& func,
    const std::shared_ptr<Properties>& config) {
  m_regionAttributesFactory->setPersistenceManager(lib, func, config);
  return *this;
}

RegionFactory& RegionFactory::setPoolName(const std::string& name) {
  m_regionAttributesFactory->setPoolName(name);
  return *this;
}

RegionFactory& RegionFactory::setCloningEnabled(bool isClonable) {
  m_regionAttributesFactory->setCloningEnabled(isClonable);
  return *this;
}
}  // namespace client
}  // namespace geode
}  // namespace apache
