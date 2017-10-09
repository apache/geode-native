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
#include "DistributedSystemImpl.hpp"
#include "CacheXmlParser.hpp"
#include "CacheRegionHelper.hpp"
#include <geode/Cache.hpp>
#include "CacheImpl.hpp"
#include "UserAttributes.hpp"
#include "ProxyRegion.hpp"
#include <geode/FunctionService.hpp>
#include "ProxyRemoteQueryService.hpp"
#include "FunctionServiceImpl.hpp"
#include "ProxyCache.hpp"
#include <string>
#include <geode/PoolManager.hpp>
#include "ThinClientPoolDM.hpp"
#include "PdxInstanceFactoryImpl.hpp"

using namespace apache::geode::client;

/**
 * Indicates if this cache has been closed.
 * After a new cache object is created, this method returns false;
 * After the close is called on this cache object, this method
 * returns true.
 *
 * @return true, if this cache is closed; false, otherwise
 */
bool ProxyCache::isClosed() const { return m_isProxyCacheClosed; }

/**
 * Terminates this object cache and releases all the local resources.
 * After this cache is closed, any further
 * method call on this cache or any region object will throw
 * <code>CacheClosedException</code>, unless otherwise noted.
 * @param keepalive whether to keep the durable client's queue
 * @throws CacheClosedException,  if the cache is already closed.
 */
void ProxyCache::close() {
  LOGDEBUG("ProxyCache::close: isProxyCacheClosed = %d", m_isProxyCacheClosed);
  if (!m_isProxyCacheClosed) {
    if (m_remoteQueryService != nullptr) {
      ProxyRemoteQueryService* prqs =
          static_cast<ProxyRemoteQueryService*>(m_remoteQueryService.get());
      prqs->closeCqs(false);
    }

    GuardUserAttribures gua(shared_from_this());
    m_isProxyCacheClosed = true;
    m_userAttributes->unSetCredentials();
    // send message to server
    PoolPtr userAttachedPool = m_userAttributes->getPool();
    PoolPtr pool = m_cacheImpl->getCache()->getPoolManager().find(
        userAttachedPool->getName());
    if (pool != nullptr && pool.get() == userAttachedPool.get()) {
      auto poolDM = std::static_pointer_cast<ThinClientPoolDM>(pool);
      if (!poolDM->isDestroyed()) {
        poolDM->sendUserCacheCloseMessage(false);
      }
    }
    return;
  }
  throw IllegalStateException("User cache has been closed.");
}

RegionPtr ProxyCache::getRegion(const char* path) {
  LOGDEBUG("ProxyCache::getRegion:");

  if (!m_isProxyCacheClosed) {
    RegionPtr result;

    if (m_cacheImpl != nullptr && !m_cacheImpl->isClosed()) {
      m_cacheImpl->getRegion(path, result);
    }

    if (result != nullptr) {
      PoolPtr userAttachedPool = m_userAttributes->getPool();
      PoolPtr pool = m_cacheImpl->getCache()->getPoolManager().find(
          result->getAttributes()->getPoolName());
      if (pool != nullptr && pool.get() == userAttachedPool.get() &&
          !pool->isDestroyed()) {
        return std::make_shared<ProxyRegion>(
            shared_from_this(),
            std::static_pointer_cast<RegionInternal>(result));
      }
      throw IllegalArgumentException(
          "The Region argument is not attached with the pool, which used to "
          "create this user cache.");
    }

    return result;
  }
  throw IllegalStateException("User cache has been closed.");
}

/**
 * Returns a set of root regions in the cache. Does not cause any
 * shared regions to be mapped into the cache. This set is a snapshot and
 * is not backed by the Cache. The regions passed in are cleared.
 *
 * @param regions the region collection object containing the returned set of
 * regions when the function returns
 */

QueryServicePtr ProxyCache::getQueryService() {
  if (!m_isProxyCacheClosed) {
    if (m_remoteQueryService != nullptr) return m_remoteQueryService;
    auto prqsPtr =
        std::make_shared<ProxyRemoteQueryService>(this->shared_from_this());
    m_remoteQueryService = prqsPtr;
    return prqsPtr;
  }
  throw IllegalStateException("User cache has been closed.");
}

VectorOfRegion ProxyCache::rootRegions() {
  LOGDEBUG("ProxyCache::rootRegions:");

  VectorOfRegion regions;

  if (!m_isProxyCacheClosed && m_cacheImpl && !m_cacheImpl->isClosed()) {
    VectorOfRegion tmp;

    // this can cause issue when pool attached with region in multiuserSecure
    // mode
    m_cacheImpl->rootRegions(tmp);
    regions.reserve(tmp.size());

    for (const auto& reg : tmp) {
      if (strcmp(m_userAttributes->getPool()->getName(),
                 reg->getAttributes()->getPoolName()) == 0) {
        auto pRegion = std::make_shared<ProxyRegion>(
            shared_from_this(), std::static_pointer_cast<RegionInternal>(reg));
        regions.push_back(pRegion);
      }
    }
  }

  return regions;
}

ProxyCache::ProxyCache(PropertiesPtr credentials, PoolPtr pool,
                       CacheImpl* cacheImpl)
    : m_remoteQueryService(nullptr),
      m_isProxyCacheClosed(false),
      m_userAttributes(
          std::make_shared<UserAttributes>(credentials, pool, this)),
      m_cacheImpl(cacheImpl) {}

ProxyCache::~ProxyCache() {}

PdxInstanceFactoryPtr ProxyCache::createPdxInstanceFactory(
    const char* className) {
  return std::make_shared<PdxInstanceFactoryImpl>(
      className, &(m_cacheImpl->getCachePerfStats()),
      m_cacheImpl->getPdxTypeRegistry(), m_cacheImpl->getCache(),
      m_cacheImpl->getDistributedSystem()
          .getSystemProperties()
          .getEnableTimeStatistics());
}
