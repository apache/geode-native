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

#include "ProxyRemoteQueryService.hpp"

#include <geode/PoolManager.hpp>

#include "CacheImpl.hpp"
#include "CqQueryImpl.hpp"
#include "ThinClientPoolDM.hpp"

namespace apache {
namespace geode {
namespace client {

ProxyRemoteQueryService::ProxyRemoteQueryService(AuthenticatedView* cptr)
    : m_authenticatedView(cptr) {}

std::shared_ptr<Query> ProxyRemoteQueryService::newQuery(
    std::string querystring) {
  if (!m_authenticatedView->isClosed()) {
    auto userAttachedPool = m_authenticatedView->m_userAttributes->getPool();
    auto pool =
        m_authenticatedView->m_cacheImpl->getCache()->getPoolManager().find(
            userAttachedPool->getName());
    if (pool != nullptr && pool.get() == userAttachedPool.get() &&
        !pool->isDestroyed()) {
      GuardUserAttributes gua(m_authenticatedView);
      auto poolDM = std::static_pointer_cast<ThinClientPoolDM>(pool);
      if (!poolDM->isDestroyed()) {
        return poolDM->getQueryServiceWithoutCheck()->newQuery(querystring);
      }
    }
    throw IllegalStateException("Pool has been closed.");
  }
  throw IllegalStateException("UserCache has been closed.");
}

void ProxyRemoteQueryService::unSupportedException(
    const std::string& operationName) {
  throw UnsupportedOperationException(operationName +
                                      "operation is not supported when pool is "
                                      "in multiuser authentication mode.");
}

std::shared_ptr<CqQuery> ProxyRemoteQueryService::newCq(
    std::string querystr, const std::shared_ptr<CqAttributes>& cqAttr,
    bool isDurable) {
  if (!m_authenticatedView->isClosed()) {
    auto userAttachedPool = m_authenticatedView->m_userAttributes->getPool();
    auto pool =
        m_authenticatedView->m_cacheImpl->getCache()->getPoolManager().find(
            userAttachedPool->getName());
    if (pool != nullptr && pool.get() == userAttachedPool.get() &&
        !pool->isDestroyed()) {
      GuardUserAttributes gua(m_authenticatedView);
      auto pooDM = std::static_pointer_cast<ThinClientPoolDM>(pool);
      if (!pooDM->isDestroyed()) {
        auto cqQuery = pooDM->getQueryServiceWithoutCheck()->newCq(
            querystr, cqAttr, isDurable);
        addCqQuery(cqQuery);
        return cqQuery;
      }
    }
    throw IllegalStateException("Pool has been closed.");
  }
  throw IllegalStateException("Logical Cache has been closed.");
}

void ProxyRemoteQueryService::addCqQuery(
    const std::shared_ptr<CqQuery>& cqQuery) {
  std::lock_guard<decltype(m_cqQueryListLock)> guard(m_cqQueryListLock);
  m_cqQueries.push_back(cqQuery);
}

std::shared_ptr<CqQuery> ProxyRemoteQueryService::newCq(
    std::string name, std::string querystr,
    const std::shared_ptr<CqAttributes>& cqAttr, bool isDurable) {
  if (!m_authenticatedView->isClosed()) {
    auto userAttachedPool = m_authenticatedView->m_userAttributes->getPool();
    auto pool =
        m_authenticatedView->m_cacheImpl->getCache()->getPoolManager().find(
            userAttachedPool->getName());
    if (pool != nullptr && pool.get() == userAttachedPool.get() &&
        !pool->isDestroyed()) {
      GuardUserAttributes gua(m_authenticatedView);
      auto poolDM = std::static_pointer_cast<ThinClientPoolDM>(pool);
      if (!poolDM->isDestroyed()) {
        auto cqQuery = poolDM->getQueryServiceWithoutCheck()->newCq(
            name, querystr, cqAttr, isDurable);
        addCqQuery(cqQuery);
        return cqQuery;
      }
    }
    throw IllegalStateException("Pool has been closed.");
  }
  throw IllegalStateException("Logical Cache has been closed.");
}

void ProxyRemoteQueryService::closeCqs() { closeCqs(false); }

void ProxyRemoteQueryService::closeCqs(bool keepAlive) {
  std::lock_guard<decltype(m_cqQueryListLock)> guard(m_cqQueryListLock);

  for (auto&& q : m_cqQueries) {
    try {
      if (!(q->isDurable() && keepAlive)) {
        q->close();
      } else {
        // need to just cleanup client side data structure
        auto&& cqImpl = std::static_pointer_cast<CqQueryImpl>(q);
        cqImpl->close(false);
      }
    } catch (QueryException& qe) {
      LOG_FINE("Failed to close the CQ, CqName : " + q->getName() +
               " Error : " + qe.getMessage());
    } catch (CqClosedException& cce) {
      LOG_FINE("Failed to close the CQ, CqName : " + q->getName() +
               " Error : " + cce.getMessage());
    }
  }
}

QueryService::query_container_type ProxyRemoteQueryService::getCqs() const {
  std::lock_guard<decltype(m_cqQueryListLock)> guard(m_cqQueryListLock);
  return m_cqQueries;
}

std::shared_ptr<CqQuery> ProxyRemoteQueryService::getCq(
    const std::string& name) const {
  if (!m_authenticatedView->isClosed()) {
    auto userAttachedPool = m_authenticatedView->m_userAttributes->getPool();
    auto pool =
        m_authenticatedView->m_cacheImpl->getCache()->getPoolManager().find(
            userAttachedPool->getName());
    if (pool != nullptr && pool.get() == userAttachedPool.get() &&
        !pool->isDestroyed()) {
      GuardUserAttributes gua(m_authenticatedView);
      auto poolDM = std::static_pointer_cast<ThinClientPoolDM>(pool);
      if (!poolDM->isDestroyed()) {
        return poolDM->getQueryServiceWithoutCheck()->getCq(name);
      }
    }
    throw IllegalStateException("Pool has been closed.");
  }
  throw IllegalStateException("Logical Cache has been closed.");
}

void ProxyRemoteQueryService::executeCqs() {
  std::lock_guard<decltype(m_cqQueryListLock)> guard(m_cqQueryListLock);

  for (auto&& q : m_cqQueries) {
    try {
      q->execute();
    } catch (QueryException& qe) {
      LOG_FINE("Failed to execute the CQ, CqName : " + q->getName() +
               " Error : " + qe.getMessage());
    } catch (CqClosedException& cce) {
      LOG_FINE("Failed to execute the CQ, CqName : " + q->getName() +
               " Error : " + cce.getMessage());
    }
  }
}

void ProxyRemoteQueryService::stopCqs() {
  std::lock_guard<decltype(m_cqQueryListLock)> guard(m_cqQueryListLock);

  for (auto&& q : m_cqQueries) {
    try {
      q->stop();
    } catch (QueryException& qe) {
      LOG_FINE("Failed to stop the CQ, CqName : " + q->getName() +
               " Error : " + qe.getMessage());
    } catch (CqClosedException& cce) {
      LOG_FINE("Failed to stop the CQ, CqName : " + q->getName() +
               " Error : " + cce.getMessage());
    }
  }
}

std::shared_ptr<CqServiceStatistics>
ProxyRemoteQueryService::getCqServiceStatistics() const {
  unSupportedException("getCqServiceStatistics()");
}
std::shared_ptr<CacheableArrayList>
ProxyRemoteQueryService::getAllDurableCqsFromServer() const {
  unSupportedException("getAllDurableCqsFromServer()");
}

}  // namespace client
}  // namespace geode
}  // namespace apache
