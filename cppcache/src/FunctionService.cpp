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
#include <geode/AuthenticatedView.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/FunctionService.hpp>
#include <geode/PoolManager.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "ExecutionImpl.hpp"
#include "ProxyRegion.hpp"
#include "UserAttributes.hpp"

namespace apache {
namespace geode {
namespace client {

Execution FunctionService::onRegion(const std::shared_ptr<Region>& region) {
  LOG_DEBUG("FunctionService::onRegion(std::shared_ptr<Region> region)");
  if (region == nullptr) {
    throw NullPointerException("FunctionService::onRegion: region is null");
  }

  auto realRegion = region;
  auto pool = realRegion->getPool();

  if (pool == nullptr) {
    throw IllegalArgumentException("Pool attached with region is closed.");
  }
  AuthenticatedView* authenticatedView = nullptr;

  if (pool->getMultiuserAuthentication()) {
    if (auto pr = std::dynamic_pointer_cast<ProxyRegion>(realRegion)) {
      LOG_DEBUG(
          "FunctionService::onRegion(std::shared_ptr<Region> region) proxy "
          "cache");
      // it is in multiuser mode
      authenticatedView = pr->m_authenticatedView;
      auto userAttachedPool = authenticatedView->m_userAttributes->getPool();
      pool = realRegion->getCache().getPoolManager().find(
          userAttachedPool->getName());
      if (!(pool != nullptr && pool.get() == userAttachedPool.get() &&
            !pool->isDestroyed())) {
        throw IllegalStateException(
            "Pool has been closed with attached Logical Cache.");
      }
      // getting real region to execute function on region
      if (!realRegion->getCache().isClosed()) {
        realRegion = realRegion->getCache().m_cacheImpl->getRegion(
            realRegion->getName());
      } else {
        throw IllegalStateException("Cache has been closed");
      }

      if (realRegion == nullptr) {
        throw IllegalStateException("Real region has been closed.");
      }
    } else {
      throw IllegalArgumentException(
          "onRegion() argument region should have get from RegionService.");
    }
  }

  return Execution(std::unique_ptr<ExecutionImpl>(
      new ExecutionImpl(realRegion, authenticatedView, pool)));
}

Execution FunctionService::onServerWithPool(const std::shared_ptr<Pool>& pool) {
  if (pool == nullptr) {
    throw NullPointerException("FunctionService::onServer: pool is null");
  }
  if (pool->getMultiuserAuthentication()) {
    throw UnsupportedOperationException(
        "This API is not supported in multiuser mode. "
        "Please use FunctionService::onServer(RegionService) API.");
  }
  return Execution(std::unique_ptr<ExecutionImpl>(new ExecutionImpl(pool)));
}

Execution FunctionService::onServersWithPool(
    const std::shared_ptr<Pool>& pool) {
  if (pool == nullptr) {
    throw NullPointerException("FunctionService::onServers: pool is null");
  }
  if (pool->getMultiuserAuthentication()) {
    throw UnsupportedOperationException(
        "This API is not supported in multiuser mode. "
        "Please use FunctionService::onServers(RegionService) API.");
  }

  return Execution(
      std::unique_ptr<ExecutionImpl>(new ExecutionImpl(pool, true)));
}

Execution FunctionService::onServerWithCache(RegionService& cache) {
  if (cache.isClosed()) {
    throw IllegalStateException("Cache has been closed");
  }

  LOG_DEBUG("FunctionService::onServer:");
  if (auto pc = dynamic_cast<AuthenticatedView*>(&cache)) {
    auto userAttachedPool = pc->m_userAttributes->getPool();
    auto pool =
        pc->m_cacheImpl->getPoolManager().find(userAttachedPool->getName());
    if (pool != nullptr && pool.get() == userAttachedPool.get() &&
        !pool->isDestroyed()) {
      return Execution(
          std::unique_ptr<ExecutionImpl>(new ExecutionImpl(pool, false, pc)));
    }
    throw IllegalStateException(
        "Pool has been close to execute function on server");
  } else {
    auto& realcache = static_cast<Cache&>(cache);
    return FunctionService::onServer(
        realcache.m_cacheImpl->getPoolManager().getDefaultPool());
  }
}

Execution FunctionService::onServersWithCache(RegionService& cache) {
  if (cache.isClosed()) {
    throw IllegalStateException("Cache has been closed");
  }

  LOG_DEBUG("FunctionService::onServers:");
  if (auto pc = dynamic_cast<AuthenticatedView*>(&cache)) {
    auto userAttachedPool = pc->m_userAttributes->getPool();
    auto pool = pc->m_cacheImpl->getCache()->getPoolManager().find(
        userAttachedPool->getName());
    if (pool != nullptr && pool.get() == userAttachedPool.get() &&
        !pool->isDestroyed()) {
      return Execution(
          std::unique_ptr<ExecutionImpl>(new ExecutionImpl(pool, false, pc)));
    }
    throw IllegalStateException(
        "Pool has been close to execute function on server");
  } else {
    auto& realcache = static_cast<Cache&>(cache);
    return FunctionService::onServers(
        realcache.m_cacheImpl->getPoolManager().getDefaultPool());
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
