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

#include <geode/geode_globals.hpp>
#include <memory>

#include <geode/DistributedSystem.hpp>
#include <DistributedSystemImpl.hpp>
#include <CacheXmlParser.hpp>
#include <CacheRegionHelper.hpp>
#include <geode/Cache.hpp>
#include <CacheImpl.hpp>
#include <UserAttributes.hpp>
#include <ProxyRegion.hpp>
#include <geode/FunctionService.hpp>
#include <geode/PoolManager.hpp>
#include <PdxInstanceFactoryImpl.hpp>

extern ACE_Recursive_Thread_Mutex* g_disconnectLock;

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
void Cache::close(bool keepalive) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> connectGuard(*g_disconnectLock);
  if (DistributedSystemImpl::currentInstances() > 0) return;
  m_cacheImpl->close(keepalive);

  try {
    getDistributedSystem().disconnect();
  } catch (const apache::geode::client::NotConnectedException&) {
  } catch (const apache::geode::client::Exception&) {
  } catch (...) {
  }
}

RegionPtr Cache::getRegion(const char* path) {
  LOGDEBUG("Cache::getRegion");
  RegionPtr result;
  m_cacheImpl->getRegion(path, result);

  if (result != nullptr) {
    if (isPoolInMultiuserMode(result)) {
      LOGWARN(
          "Pool [%s] attached with region [%s] is in multiuser authentication "
          "mode. "
          "Operations may fail as this instance does not have any credentials.",
          result->getAttributes()->getPoolName(), result->getFullPath());
    }
  }

  return result;
}

/**
 * Returns a set of root regions in the cache. Does not cause any
 * shared regions to be mapped into the cache. This set is a snapshot and
 * is not backed by the Cache. The regions passed in are cleared.
 *
 * @param regions the region collection object containing the returned set of
 * regions when the function returns
 */

VectorOfRegion Cache::rootRegions() {
  VectorOfRegion regions;
  m_cacheImpl->rootRegions(regions);
  return regions;
}

RegionFactory Cache::createRegionFactory(RegionShortcut preDefinedRegion) {
  return m_cacheImpl->createRegionFactory(preDefinedRegion);
}

QueryServicePtr Cache::getQueryService() {
  return m_cacheImpl->getQueryService();
}

QueryServicePtr Cache::getQueryService(const char* poolName) {
  return m_cacheImpl->getQueryService(poolName);
}

CacheTransactionManagerPtr Cache::getCacheTransactionManager() {
  return m_cacheImpl->getCacheTransactionManager();
}

TypeRegistry& Cache::getTypeRegistry() { return *(m_typeRegistry.get()); }

Cache::Cache(const std::string& name, PropertiesPtr dsProp,
             bool ignorePdxUnreadFields, bool readPdxSerialized,
             const AuthInitializePtr& authInitialize) {
  auto dsPtr = DistributedSystem::create(DEFAULT_DS_NAME, this, dsProp);
  dsPtr->connect();
  m_cacheImpl = std::unique_ptr<CacheImpl>(
      new CacheImpl(this, name, std::move(dsPtr), ignorePdxUnreadFields,
                    readPdxSerialized, authInitialize));
  m_typeRegistry = std::unique_ptr<TypeRegistry>(new TypeRegistry(*this));
}

Cache::~Cache() = default;

/** Initialize the cache by the contents of an xml file
 * @param  cacheXml
 *         The xml file
 * @throws OutOfMemoryException
 * @throws CacheXmlException
 *         Something went wrong while parsing the XML
 * @throws IllegalStateException
 *         If xml file is well-flrmed but not valid
 * @throws RegionExistsException if a region is already in
 *         this cache
 * @throws CacheClosedException if the cache is closed
 *         at the time of region creation
 * @throws UnknownException otherwise
 */
void Cache::initializeDeclarativeCache(const char* cacheXml) {
  CacheXmlParser* xmlParser = CacheXmlParser::parse(cacheXml, this);
  xmlParser->setAttributes(this);
  m_cacheImpl->initServices();
  xmlParser->create(this);
  delete xmlParser;
  xmlParser = nullptr;
}

void Cache::readyForEvents() { m_cacheImpl->readyForEvents(); }

bool Cache::isPoolInMultiuserMode(RegionPtr regionPtr) {
  const char* poolName = regionPtr->getAttributes()->getPoolName();

  if (poolName != nullptr) {
    PoolPtr poolPtr = regionPtr->getCache()->getPoolManager().find(poolName);
    if (poolPtr != nullptr && !poolPtr->isDestroyed()) {
      return poolPtr->getMultiuserAuthentication();
    }
  }
  return false;
}

bool Cache::getPdxIgnoreUnreadFields() {
  return m_cacheImpl->getPdxIgnoreUnreadFields();
}

bool Cache::getPdxReadSerialized() {
  return m_cacheImpl->getPdxReadSerialized();
}

PdxInstanceFactoryPtr Cache::createPdxInstanceFactory(const char* className) {
  return std::make_shared<PdxInstanceFactoryImpl>(
      className, m_cacheImpl->m_cacheStats, m_cacheImpl->getPdxTypeRegistry(),
      this,
      m_cacheImpl->getDistributedSystem()
          .getSystemProperties()
          .getEnableTimeStatistics());
}

RegionServicePtr Cache::createAuthenticatedView(
    PropertiesPtr userSecurityProperties, const char* poolName) {
  if (poolName == nullptr) {
    auto pool = m_cacheImpl->getPoolManager().getDefaultPool();
    if (!this->isClosed() && pool != nullptr) {
      return pool->createSecureUserCache(userSecurityProperties,
                                         m_cacheImpl.get());
    }

    throw IllegalStateException(
        "Either cache has been closed or there are more than two pool."
        "Pass poolname to get the secure Cache");
  } else {
    if (!this->isClosed()) {
      if (poolName != nullptr) {
        PoolPtr poolPtr = m_cacheImpl->getPoolManager().find(poolName);
        if (poolPtr != nullptr && !poolPtr->isDestroyed()) {
          return poolPtr->createSecureUserCache(userSecurityProperties,
                                                m_cacheImpl.get());
        }
        throw IllegalStateException(
            "Either pool not found or it has been destroyed");
      }
      throw IllegalArgumentException("poolname is nullptr");
    }

    throw IllegalStateException("Cache has been closed");
  }
  return nullptr;
}

StatisticsFactory* Cache::getStatisticsFactory() const {
  return m_cacheImpl->getDistributedSystem()
      .getStatisticsManager()
      ->getStatisticsFactory();
}

PoolManager& Cache::getPoolManager() const {
  return m_cacheImpl->getPoolManager();
}

std::unique_ptr<DataInput> Cache::createDataInput(const uint8_t* m_buffer,
                                                  int32_t len) const {
  return std::unique_ptr<DataInput>(new DataInput(m_buffer, len, this));
}

std::unique_ptr<DataOutput> Cache::createDataOutput() const {
  return std::unique_ptr<DataOutput>(new DataOutput(this));
}

}  // namespace client
}  // namespace geode
}  // namespace apache
