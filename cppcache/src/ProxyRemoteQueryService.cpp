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
#include "ThinClientPoolDM.hpp"
#include <geode/PoolManager.hpp>
#include "CqQueryImpl.hpp"

ProxyRemoteQueryService::ProxyRemoteQueryService(ProxyCachePtr cptr)
    : m_proxyCache(cptr) {}

QueryPtr ProxyRemoteQueryService::newQuery(const char* querystring) {
  if (!m_proxyCache->isClosed()) {
    auto userAttachedPool = m_proxyCache->m_userAttributes->getPool();
    auto pool = m_proxyCache->m_cacheImpl->getCache()->getPoolManager().find(
        userAttachedPool->getName());
    if (pool != nullptr && pool.get() == userAttachedPool.get() &&
        !pool->isDestroyed()) {
      GuardUserAttribures gua(m_proxyCache);
      auto poolDM = std::static_pointer_cast<ThinClientPoolDM>(pool);
      if (!poolDM->isDestroyed()) {
        return poolDM->getQueryServiceWithoutCheck()->newQuery(querystring);
      }
    }
    throw IllegalStateException("Pool has been closed.");
  }
  throw IllegalStateException("UserCache has been closed.");
}

void ProxyRemoteQueryService::unSupportedException(const char* operationName) {
  char msg[256] = {'\0'};
  ACE_OS::snprintf(msg, 256,
                   "%s operation is not supported when pool is in multiuser "
                   "authentication mode.",
                   operationName);
  throw UnsupportedOperationException(msg);
}

CqQueryPtr ProxyRemoteQueryService::newCq(const char* querystr,
                                          const CqAttributesPtr& cqAttr,
                                          bool isDurable) {
  if (!m_proxyCache->isClosed()) {
    auto userAttachedPool = m_proxyCache->m_userAttributes->getPool();
    auto pool = m_proxyCache->m_cacheImpl->getCache()->getPoolManager().find(
        userAttachedPool->getName());
    if (pool != nullptr && pool.get() == userAttachedPool.get() &&
        !pool->isDestroyed()) {
      GuardUserAttribures gua(m_proxyCache);
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

void ProxyRemoteQueryService::addCqQuery(const CqQueryPtr& cqQuery) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_cqQueryListLock);
  m_cqQueries.push_back(cqQuery);
}

CqQueryPtr ProxyRemoteQueryService::newCq(const char* name,
                                          const char* querystr,
                                          const CqAttributesPtr& cqAttr,
                                          bool isDurable) {
  if (!m_proxyCache->isClosed()) {
    auto userAttachedPool = m_proxyCache->m_userAttributes->getPool();
    auto pool = m_proxyCache->m_cacheImpl->getCache()->getPoolManager().find(
        userAttachedPool->getName());
    if (pool != nullptr && pool.get() == userAttachedPool.get() &&
        !pool->isDestroyed()) {
      GuardUserAttribures gua(m_proxyCache);
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
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_cqQueryListLock);

  for (auto& q : m_cqQueries) {
    std::string cqName = q->getName();
    try {
      if (!(q->isDurable() && keepAlive)) {
        q->close();
      } else {
        // need to just cleanup client side data structure
        auto cqImpl = std::static_pointer_cast<CqQueryImpl>(q);
        cqImpl->close(false);
      }
    } catch (QueryException& qe) {
      Log::fine(("Failed to close the CQ, CqName : " + cqName +
                 " Error : " + qe.getMessage())
                    .c_str());
    } catch (CqClosedException& cce) {
      Log::fine(("Failed to close the CQ, CqName : " + cqName +
                 " Error : " + cce.getMessage())
                    .c_str());
    }
  }
}

QueryService::query_container_type ProxyRemoteQueryService::getCqs() {
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_cqQueryListLock);
  return m_cqQueries;
}

CqQueryPtr ProxyRemoteQueryService::getCq(const char* name) {
  if (!m_proxyCache->isClosed()) {
    auto userAttachedPool = m_proxyCache->m_userAttributes->getPool();
    auto pool = m_proxyCache->m_cacheImpl->getCache()->getPoolManager().find(
        userAttachedPool->getName());
    if (pool != nullptr && pool.get() == userAttachedPool.get() &&
        !pool->isDestroyed()) {
      GuardUserAttribures gua(m_proxyCache);
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
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_cqQueryListLock);

  for (auto& q : m_cqQueries) {
    std::string cqName = q->getName();
    try {
      q->execute();
    } catch (QueryException& qe) {
      Log::fine(("Failed to excecue the CQ, CqName : " + cqName +
                 " Error : " + qe.getMessage())
                    .c_str());
    } catch (CqClosedException& cce) {
      Log::fine(("Failed to excecue the CQ, CqName : " + cqName +
                 " Error : " + cce.getMessage())
                    .c_str());
    }
  }
}

void ProxyRemoteQueryService::stopCqs() {
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_cqQueryListLock);

  for (auto& q : m_cqQueries) {
    std::string cqName = q->getName();
    try {
      q->stop();
    } catch (QueryException& qe) {
      Log::fine(("Failed to stop the CQ, CqName : " + cqName +
                 " Error : " + qe.getMessage())
                    .c_str());
    } catch (CqClosedException& cce) {
      Log::fine(("Failed to stop the CQ, CqName : " + cqName +
                 " Error : " + cce.getMessage())
                    .c_str());
    }
  }
}

CqServiceStatisticsPtr ProxyRemoteQueryService::getCqServiceStatistics() {
  unSupportedException("getCqServiceStatistics()");
  return nullptr;
}

CacheableArrayListPtr ProxyRemoteQueryService::getAllDurableCqsFromServer() {
  unSupportedException("getAllDurableCqsFromServer()");
  return nullptr;
}
