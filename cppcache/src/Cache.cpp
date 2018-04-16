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

#include <memory>

#include <geode/internal/geode_globals.hpp>
#include <geode/FunctionService.hpp>
#include <geode/PoolManager.hpp>
#include <geode/DistributedSystem.hpp>
#include <geode/Cache.hpp>

#include "DistributedSystemImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "CacheImpl.hpp"
#include "UserAttributes.hpp"
#include "ProxyRegion.hpp"

#define DEFAULT_DS_NAME "default_GeodeDS"

namespace apache {
namespace geode {
namespace client {

/** Returns the name of this cache.
 * This method does not throw
 * <code>CacheClosedException</code> if the cache is closed.
 * @return the string name of this cache
 */
const std::string& Cache::getName() const { return m_cacheImpl->getName(); }

/**
 * Indicates if this cache has been closed.
 * After a new cache object is created, this method returns false;
 * After the close is called on this cache object, this method
 * returns true.
 *
 * @return true, if this cache is closed; false, otherwise
 */
bool Cache::isClosed() const { return m_cacheImpl->isClosed(); }

/**
 * Returns the distributed system that this cache was
 * {@link CacheFactory::create created} with. This method does not throw
 * <code>CacheClosedException</code> if the cache is closed.
 */
DistributedSystem& Cache::getDistributedSystem() const {
  return m_cacheImpl->getDistributedSystem();
}

void Cache::close() { close(false); }

/**
 * Terminates this object cache and releases all the local resources.
 * After this cache is closed, any further
 * method call on this cache or any region object will throw
 * <code>CacheClosedException</code>, unless otherwise noted.
 * @param keepalive whether to keep the durable client's queue
 * @throws CacheClosedException,  if the cache is already closed.
 */
void Cache::close(bool keepalive) { m_cacheImpl->close(keepalive); }

std::shared_ptr<Region> Cache::getRegion(const std::string& path) const {
  return m_cacheImpl->getRegion(path);
}

/**
 * Returns a set of root regions in the cache. Does not cause any
 * shared regions to be mapped into the cache. This set is a snapshot and
 * is not backed by the Cache. The regions passed in are cleared.
 *
 * @param regions the region collection object containing the returned set of
 * regions when the function returns
 */

std::vector<std::shared_ptr<Region>> Cache::rootRegions() const {
  std::vector<std::shared_ptr<Region>> regions;
  m_cacheImpl->rootRegions(regions);
  return regions;
}

RegionFactory Cache::createRegionFactory(RegionShortcut preDefinedRegion) {
  return m_cacheImpl->createRegionFactory(preDefinedRegion);
}

std::shared_ptr<QueryService> Cache::getQueryService() {
  return m_cacheImpl->getQueryService();
}
std::shared_ptr<QueryService> Cache::getQueryService(
    const std::string& poolName) const {
  return m_cacheImpl->getQueryService(poolName.c_str());
}
std::shared_ptr<CacheTransactionManager> Cache::getCacheTransactionManager()
    const {
  return m_cacheImpl->getCacheTransactionManager();
}

TypeRegistry& Cache::getTypeRegistry() { return *(m_typeRegistry.get()); }

Cache::Cache(std::shared_ptr<Properties> dsProp, bool ignorePdxUnreadFields,
             bool readPdxSerialized,
             const std::shared_ptr<AuthInitialize>& authInitialize) {
  auto distributedSystem = DistributedSystem::create(DEFAULT_DS_NAME, dsProp);
  m_cacheImpl = std::unique_ptr<CacheImpl>(
      new CacheImpl(this, std::move(distributedSystem), ignorePdxUnreadFields,
                    readPdxSerialized, authInitialize));
  m_cacheImpl->getDistributedSystem().connect(this);
  m_typeRegistry =
      std::unique_ptr<TypeRegistry>(new TypeRegistry(m_cacheImpl.get()));
}

Cache::Cache(Cache&& other) noexcept
    : m_cacheImpl(std::move(other.m_cacheImpl)),
      m_typeRegistry(std::move(other.m_typeRegistry)) {
  m_cacheImpl->setCache(this);
}

Cache& Cache::operator=(Cache&& other) noexcept {
  if (this != &other) {
    m_cacheImpl = std::move(other.m_cacheImpl);
    m_typeRegistry = std::move(other.m_typeRegistry);
    m_cacheImpl->setCache(this);
  }
  return *this;
}

Cache::~Cache() = default;

void Cache::initializeDeclarativeCache(const std::string& cacheXml) {
  m_cacheImpl->initializeDeclarativeCache(cacheXml);
}

void Cache::readyForEvents() { m_cacheImpl->readyForEvents(); }

bool Cache::isPoolInMultiuserMode(std::shared_ptr<Region> regionPtr) {
  return CacheImpl::isPoolInMultiuserMode(regionPtr);
}

bool Cache::getPdxIgnoreUnreadFields() const {
  return m_cacheImpl->getPdxIgnoreUnreadFields();
}

bool Cache::getPdxReadSerialized() const {
  return m_cacheImpl->getPdxReadSerialized();
}

std::shared_ptr<PdxInstanceFactory> Cache::createPdxInstanceFactory(
    const std::string& className) const {
  return m_cacheImpl->createPdxInstanceFactory(className);
}

AuthenticatedView Cache::createAuthenticatedView(
    const std::shared_ptr<Properties>& userSecurityProperties,
    const std::string& poolName) {
  return m_cacheImpl->createAuthenticatedView(userSecurityProperties, poolName);
}

PoolManager& Cache::getPoolManager() const {
  return m_cacheImpl->getPoolManager();
}

std::unique_ptr<DataInput> Cache::createDataInput(const uint8_t* m_buffer,
                                                  size_t len) const {
  return m_cacheImpl->createDataInput(m_buffer, len);
}

std::unique_ptr<DataOutput> Cache::createDataOutput() const {
  return m_cacheImpl->createDataOutput();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
