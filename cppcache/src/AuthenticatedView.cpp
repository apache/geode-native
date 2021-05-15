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
#include <string>

#include <geode/AuthenticatedView.hpp>
#include <geode/Cache.hpp>
#include <geode/FunctionService.hpp>
#include <geode/PoolManager.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "CacheXmlParser.hpp"
#include "DistributedSystemImpl.hpp"
#include "ProxyRegion.hpp"
#include "ProxyRemoteQueryService.hpp"
#include "ThinClientPoolDM.hpp"
#include "UserAttributes.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * Indicates if this cache has been closed.
 * After a new cache object is created, this method returns false;
 * After the close is called on this cache object, this method
 * returns true.
 *
 * @return true, if this cache is closed; false, otherwise
 */
bool AuthenticatedView::isClosed() const { return m_isAuthenticatedViewClosed; }

/**
 * Terminates this object cache and releases all the local resources.
 * After this cache is closed, any further
 * method call on this cache or any region object will throw
 * <code>CacheClosedException</code>, unless otherwise noted.
 * @throws CacheClosedException,  if the cache is already closed.
 */
void AuthenticatedView::close() {
  LOG_DEBUG("AuthenticatedView::close: isAuthenticatedViewClosed = %d",
            m_isAuthenticatedViewClosed);
  if (!m_isAuthenticatedViewClosed) {
    if (m_remoteQueryService != nullptr) {
      ProxyRemoteQueryService* prqs =
          static_cast<ProxyRemoteQueryService*>(m_remoteQueryService.get());
      prqs->closeCqs(false);
    }

    GuardUserAttributes gua(this);
    m_isAuthenticatedViewClosed = true;
    m_userAttributes->unSetCredentials();
    // send message to server
    auto userAttachedPool = m_userAttributes->getPool();
    auto pool = m_cacheImpl->getCache()->getPoolManager().find(
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
std::shared_ptr<Region> AuthenticatedView::getRegion(
    const std::string& path) const {
  LOG_DEBUG("AuthenticatedView::getRegion:");

  if (!m_isAuthenticatedViewClosed) {
    std::shared_ptr<Region> result;

    if (m_cacheImpl != nullptr && !m_cacheImpl->isClosed()) {
      result = m_cacheImpl->getRegion(path);
    }

    if (result != nullptr) {
      auto userAttachedPool = m_userAttributes->getPool();
      auto pool = m_cacheImpl->getCache()->getPoolManager().find(
          result->getAttributes().getPoolName());
      if (pool != nullptr && pool.get() == userAttachedPool.get() &&
          !pool->isDestroyed()) {
        return std::make_shared<ProxyRegion>(
            const_cast<AuthenticatedView&>(*this),
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
 */
std::shared_ptr<QueryService> AuthenticatedView::getQueryService() {
  if (!m_isAuthenticatedViewClosed) {
    if (m_remoteQueryService != nullptr) return m_remoteQueryService;
    auto prqsPtr = std::make_shared<ProxyRemoteQueryService>(this);
    m_remoteQueryService = prqsPtr;
    return std::move(prqsPtr);
  }
  throw IllegalStateException("User cache has been closed.");
}

std::vector<std::shared_ptr<Region>> AuthenticatedView::rootRegions() const {
  LOG_DEBUG("AuthenticatedView::rootRegions:");

  std::vector<std::shared_ptr<Region>> regions;

  if (!m_isAuthenticatedViewClosed && m_cacheImpl && !m_cacheImpl->isClosed()) {
    // this can cause issue when pool attached with region in multiuserSecure
    // mode
    auto tmp = m_cacheImpl->rootRegions();
    regions.reserve(tmp.size());

    for (const auto& reg : tmp) {
      if (m_userAttributes->getPool()->getName() ==
          reg->getAttributes().getPoolName()) {
        auto pRegion = std::make_shared<ProxyRegion>(
            const_cast<AuthenticatedView&>(*this),
            std::static_pointer_cast<RegionInternal>(reg));
        regions.push_back(pRegion);
      }
    }
  }

  return regions;
}

AuthenticatedView::AuthenticatedView(std::shared_ptr<Properties> credentials,
                                     std::shared_ptr<Pool> pool,
                                     CacheImpl* cacheImpl)
    : m_userAttributes(
          std::make_shared<UserAttributes>(credentials, pool, this)),
      m_isAuthenticatedViewClosed(false),
      m_remoteQueryService(nullptr),
      m_cacheImpl(cacheImpl) {}

AuthenticatedView::~AuthenticatedView() = default;

PdxInstanceFactory AuthenticatedView::createPdxInstanceFactory(
    const std::string& className, bool expectDomainClass) const {
  return PdxInstanceFactory(className, expectDomainClass,
                            m_cacheImpl->getCachePerfStats(),
                            *m_cacheImpl->getPdxTypeRegistry(), *m_cacheImpl,
                            m_cacheImpl->getDistributedSystem()
                                .getSystemProperties()
                                .getEnableTimeStatistics());
}

PdxInstanceFactory AuthenticatedView::createPdxInstanceFactory(
    const std::string& className) const {
  return createPdxInstanceFactory(className, true);
}
}  // namespace client
}  // namespace geode
}  // namespace apache
