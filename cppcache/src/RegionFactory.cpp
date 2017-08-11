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

#include <geode/CacheFactory.hpp>
#include <geode/RegionFactory.hpp>
#include <CppCacheLibrary.hpp>
#include <geode/Cache.hpp>
#include <CacheImpl.hpp>
#include <geode/SystemProperties.hpp>
#include <geode/PoolManager.hpp>
#include <CacheConfig.hpp>
#include <CacheRegionHelper.hpp>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Guard_T.h>
#include <map>
#include <string>

extern ACE_Recursive_Thread_Mutex* g_disconnectLock;

namespace apache {
namespace geode {
namespace client {

RegionFactory::RegionFactory(RegionShortcut preDefinedRegion,
                             CacheImpl* cacheImpl)
    : m_preDefinedRegion(preDefinedRegion),
      m_attributeFactory(std::make_shared<AttributesFactory>()),
      m_cacheImpl(cacheImpl) {
  setRegionShortcut();
}

RegionFactory::~RegionFactory() {}

RegionPtr RegionFactory::create(const char* name) {
  RegionPtr retRegionPtr = nullptr;
  RegionAttributesPtr regAttr = m_attributeFactory->createRegionAttributes();
  if (m_preDefinedRegion != LOCAL && (regAttr->getPoolName() == nullptr ||
                                      strlen(regAttr->getPoolName()) == 0)) {
    auto pool = m_cacheImpl->getPoolManager().getDefaultPool();
    if (!pool) {
      throw IllegalStateException("No pool for non-local region.");
    }
    m_attributeFactory->setPoolName(pool->getName());
    regAttr = m_attributeFactory->createRegionAttributes();
  }
  m_cacheImpl->createRegion(name, regAttr, retRegionPtr);

  return retRegionPtr;
}

void RegionFactory::setRegionShortcut() {
  switch (m_preDefinedRegion) {
    case PROXY: {
      m_attributeFactory->setCachingEnabled(false);
    } break;
    case CACHING_PROXY: {
      m_attributeFactory->setCachingEnabled(true);
    } break;
    case CACHING_PROXY_ENTRY_LRU: {
      m_attributeFactory->setCachingEnabled(true);
      m_attributeFactory->setLruEntriesLimit(DEFAULT_LRU_MAXIMUM_ENTRIES);
    } break;
    case LOCAL: {
    } break;
    case LOCAL_ENTRY_LRU: {
      m_attributeFactory->setLruEntriesLimit(DEFAULT_LRU_MAXIMUM_ENTRIES);
    } break;
  }
}

RegionFactoryPtr RegionFactory::setCacheLoader(
    const CacheLoaderPtr& cacheLoader) {
  m_attributeFactory->setCacheLoader(cacheLoader);
  return shared_from_this();
}

RegionFactoryPtr RegionFactory::setCacheWriter(
    const CacheWriterPtr& cacheWriter) {
  m_attributeFactory->setCacheWriter(cacheWriter);
  return shared_from_this();
}
RegionFactoryPtr RegionFactory::setCacheListener(
    const CacheListenerPtr& aListener) {
  m_attributeFactory->setCacheListener(aListener);
  return shared_from_this();
}
RegionFactoryPtr RegionFactory::setPartitionResolver(
    const PartitionResolverPtr& aResolver) {
  m_attributeFactory->setPartitionResolver(aResolver);
  return shared_from_this();
}

RegionFactoryPtr RegionFactory::setCacheLoader(const char* lib,
                                               const char* func) {
  m_attributeFactory->setCacheLoader(lib, func);
  return shared_from_this();
}

RegionFactoryPtr RegionFactory::setCacheWriter(const char* lib,
                                               const char* func) {
  m_attributeFactory->setCacheWriter(lib, func);
  return shared_from_this();
}

RegionFactoryPtr RegionFactory::setCacheListener(const char* lib,
                                                 const char* func) {
  m_attributeFactory->setCacheListener(lib, func);
  return shared_from_this();
}

RegionFactoryPtr RegionFactory::setPartitionResolver(const char* lib,
                                                     const char* func) {
  m_attributeFactory->setPartitionResolver(lib, func);
  return shared_from_this();
}

RegionFactoryPtr RegionFactory::setEntryIdleTimeout(
    ExpirationAction::Action action, int idleTimeout) {
  m_attributeFactory->setEntryIdleTimeout(action, idleTimeout);
  return shared_from_this();
}

RegionFactoryPtr RegionFactory::setEntryTimeToLive(
    ExpirationAction::Action action, int timeToLive) {
  m_attributeFactory->setEntryTimeToLive(action, timeToLive);
  return shared_from_this();
}

RegionFactoryPtr RegionFactory::setRegionIdleTimeout(
    ExpirationAction::Action action, int idleTimeout) {
  m_attributeFactory->setRegionIdleTimeout(action, idleTimeout);
  return shared_from_this();
}
RegionFactoryPtr RegionFactory::setRegionTimeToLive(
    ExpirationAction::Action action, int timeToLive) {
  m_attributeFactory->setRegionTimeToLive(action, timeToLive);
  return shared_from_this();
}

RegionFactoryPtr RegionFactory::setInitialCapacity(int initialCapacity) {
  char excpStr[256] = {0};
  if (initialCapacity < 0) {
    ACE_OS::snprintf(excpStr, 256, "initialCapacity must be >= 0 ");
    throw IllegalArgumentException(excpStr);
  }
  m_attributeFactory->setInitialCapacity(initialCapacity);
  return shared_from_this();
}

RegionFactoryPtr RegionFactory::setLoadFactor(float loadFactor) {
  m_attributeFactory->setLoadFactor(loadFactor);
  return shared_from_this();
}

RegionFactoryPtr RegionFactory::setConcurrencyLevel(uint8_t concurrencyLevel) {
  m_attributeFactory->setConcurrencyLevel(concurrencyLevel);
  return shared_from_this();
}
RegionFactoryPtr RegionFactory::setConcurrencyChecksEnabled(bool enable) {
  m_attributeFactory->setConcurrencyChecksEnabled(enable);
  return shared_from_this();
}
RegionFactoryPtr RegionFactory::setLruEntriesLimit(
    const uint32_t entriesLimit) {
  m_attributeFactory->setLruEntriesLimit(entriesLimit);
  return shared_from_this();
}

RegionFactoryPtr RegionFactory::setDiskPolicy(
    const DiskPolicyType::PolicyType diskPolicy) {
  m_attributeFactory->setDiskPolicy(diskPolicy);
  return shared_from_this();
}

RegionFactoryPtr RegionFactory::setCachingEnabled(bool cachingEnabled) {
  m_attributeFactory->setCachingEnabled(cachingEnabled);
  return shared_from_this();
}

RegionFactoryPtr RegionFactory::setPersistenceManager(
    const PersistenceManagerPtr& persistenceManager,
    const PropertiesPtr& config) {
  m_attributeFactory->setPersistenceManager(persistenceManager, config);
  return shared_from_this();
}

RegionFactoryPtr RegionFactory::setPersistenceManager(
    const char* lib, const char* func, const PropertiesPtr& config) {
  m_attributeFactory->setPersistenceManager(lib, func, config);
  return shared_from_this();
}

RegionFactoryPtr RegionFactory::setPoolName(const char* name) {
  m_attributeFactory->setPoolName(name);
  return shared_from_this();
}

RegionFactoryPtr RegionFactory::setCloningEnabled(bool isClonable) {
  m_attributeFactory->setCloningEnabled(isClonable);
  return shared_from_this();
}
}  // namespace client
}  // namespace geode
}  // namespace apache
