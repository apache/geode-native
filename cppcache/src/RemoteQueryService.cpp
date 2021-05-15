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

#include "RemoteQueryService.hpp"

#include "CacheImpl.hpp"
#include "CqServiceVsdStats.hpp"
#include "ReadWriteLock.hpp"
#include "RemoteQuery.hpp"
#include "ThinClientCacheDistributionManager.hpp"
#include "ThinClientPoolDM.hpp"
#include "UserAttributes.hpp"
#include "statistics/StatisticsManager.hpp"

namespace apache {
namespace geode {
namespace client {

RemoteQueryService::RemoteQueryService(CacheImpl* cache,
                                       ThinClientPoolDM* poolDM)
    : m_invalid(true),
      m_cqService(nullptr),
      m_statisticsFactory(
          cache->getStatisticsManager().getStatisticsFactory()) {
  if (poolDM) {
    m_tccdm = poolDM;
  } else {
    m_tccdm =
        new ThinClientCacheDistributionManager(cache->tcrConnectionManager());
  }
  LOG_FINEST("Initialized m_tccdm");
}

void RemoteQueryService::init() {
  TryWriteGuard guard(m_rwLock, m_invalid);

  if (m_invalid) {
    LOG_FINEST("RemoteQueryService::init: initializing TCCDM");
    if (dynamic_cast<ThinClientCacheDistributionManager*>(m_tccdm)) {
      m_tccdm->init();
    }
    m_invalid = false;
    LOG_FINEST("RemoteQueryService::init: done initialization");
  }
}

std::shared_ptr<Query> RemoteQueryService::newQuery(std::string querystring) {
  LOG_DEBUG("RemoteQueryService::newQuery: multiuserMode = %d ",
            m_tccdm->isMultiUserMode());
  if (!m_tccdm->isMultiUserMode()) {
    TryReadGuard guard(m_rwLock, m_invalid);

    if (m_invalid) {
      throw CacheClosedException(
          "QueryService::newQuery: Cache has been closed.");
    }

    LOG_DEBUG("RemoteQueryService: creating a new query: " + querystring);
    return std::shared_ptr<Query>(
        new RemoteQuery(querystring, shared_from_this(), m_tccdm));
  } else {
    TryReadGuard guard(m_rwLock, m_invalid);

    if (m_invalid) {
      throw CacheClosedException(
          "QueryService::newQuery: Cache has been closed.");
    }

    LOG_DEBUG("RemoteQueryService: creating a new query: " + querystring);
    return std::shared_ptr<Query>(new RemoteQuery(
        querystring, shared_from_this(), m_tccdm,
        UserAttributes::threadLocalUserAttributes->getAuthenticatedView()));
  }
}

void RemoteQueryService::close() {
  LOG_FINEST("RemoteQueryService::close: starting close");
  TryWriteGuard guard(m_rwLock, m_invalid);

  if (m_cqService != nullptr) {
    LOG_FINEST("RemoteQueryService::close: starting CQ service close");
    m_cqService->closeCqService();
    m_cqService = nullptr;
    LOG_FINEST("RemoteQueryService::close: completed CQ service close");
  }

  if (dynamic_cast<ThinClientCacheDistributionManager*>(m_tccdm)) {
    if (!m_invalid) {
      LOG_FINEST("RemoteQueryService::close: destroying DM");
      m_tccdm->destroy();
    }
    _GEODE_SAFE_DELETE(m_tccdm);
    m_invalid = true;
  }

  if (!m_CqPoolsConnected.empty()) {
    m_CqPoolsConnected.clear();
  }

  LOG_FINEST("RemoteQueryService::close: completed");
}

/**
 * execute all cqs on the endpoint after failover
 */
GfErrType RemoteQueryService::executeAllCqs(TcrEndpoint* endpoint) {
  TryReadGuard guard(m_rwLock, m_invalid);

  if (m_invalid) {
    LOG_FINE("QueryService::executeAllCqs(endpoint): Not initialized.");
    return GF_NOERR;
  }

  if (m_cqService == nullptr) {
    LOG_FINE(
        "RemoteQueryService: no cq to execute after failover to endpoint[" +
        endpoint->name() + "]");
    return GF_NOERR;
  } else {
    LOG_FINE("RemoteQueryService: execute all cqs after failover to endpoint[" +
             endpoint->name() + "]");
    return m_cqService->executeAllClientCqs(endpoint);
  }
}

void RemoteQueryService::executeAllCqs(bool failover) {
  TryReadGuard guard(m_rwLock, m_invalid);

  if (m_invalid) {
    LOG_FINE("QueryService::executeAllCqs: Not initialized.");
    return;
  }

  /*if cq has not been started, then failover will not start it.*/
  if (m_cqService != nullptr) {
    LOG_FINE("RemoteQueryService: execute all cqs after failover");
    m_cqService->executeAllClientCqs(failover);
  } else {
    LOG_FINE("RemoteQueryService: no cq to execute after failover");
  }
}

std::shared_ptr<CqQuery> RemoteQueryService::newCq(
    std::string querystr, const std::shared_ptr<CqAttributes>& cqAttr,
    bool isDurable) {
  TryReadGuard guard(m_rwLock, m_invalid);

  if (m_invalid) {
    throw CacheClosedException("QueryService::newCq: Cache has been closed.");
  }
  initCqService();
  // use query string as name for now
  std::string name("_default");
  name += querystr;
  return m_cqService->newCq(name, querystr, cqAttr, isDurable);
}

std::shared_ptr<CqQuery> RemoteQueryService::newCq(
    std::string name, std::string querystr,
    const std::shared_ptr<CqAttributes>& cqAttr, bool isDurable) {
  TryReadGuard guard(m_rwLock, m_invalid);

  if (m_invalid) {
    throw CacheClosedException("QueryService::newCq: Cache has been closed.");
  }

  initCqService();
  return m_cqService->newCq(name, querystr, cqAttr, isDurable);
}

void RemoteQueryService::closeCqs() {
  TryReadGuard guard(m_rwLock, m_invalid);

  if (m_invalid) {
    LOG_FINE("QueryService::closeCqs: Cache has been closed.");
    return;
  }

  // If cqService has not started, then no cq exists
  if (m_cqService != nullptr) {
    m_cqService->closeAllCqs();
  }
}

CqService::query_container_type RemoteQueryService::getCqs() const {
  TryReadGuard guard(m_rwLock, m_invalid);

  if (m_invalid) {
    throw CacheClosedException("QueryService::getCqs: Cache has been closed.");
  }

  // If cqService has not started, then no cq exists
  CqService::query_container_type vec;
  if (m_cqService) {
    vec = m_cqService->getAllCqs();
  }

  return vec;
}

std::shared_ptr<CqQuery> RemoteQueryService::getCq(
    const std::string& name) const {
  TryReadGuard guard(m_rwLock, m_invalid);

  if (m_invalid) {
    throw CacheClosedException("QueryService::getCq: Cache has been closed.");
  }

  // If cqService has not started, then no cq exists
  if (m_cqService) {
    return m_cqService->getCq(name);
  }

  return nullptr;
}

void RemoteQueryService::executeCqs() {
  TryReadGuard guard(m_rwLock, m_invalid);

  if (m_invalid) {
    throw CacheClosedException(
        "QueryService::executeCqs: Cache has been closed.");
  }

  // If cqService has not started, then no cq exists
  if (m_cqService != nullptr) {
    m_cqService->executeAllClientCqs();
  }
}

void RemoteQueryService::stopCqs() {
  TryReadGuard guard(m_rwLock, m_invalid);

  if (m_invalid) {
    LOG_FINE("QueryService::stopCqs: Cache has been closed.");
    return;
  }

  // If cqService has not started, then no cq exists
  if (m_cqService != nullptr) {
    m_cqService->stopAllClientCqs();
  }
}

std::shared_ptr<CqServiceStatistics>
RemoteQueryService::getCqServiceStatistics() const {
  TryReadGuard guard(m_rwLock, m_invalid);

  if (m_invalid) {
    throw CacheClosedException(
        "QueryService::getCqServiceStatistics: Cache has been closed.");
  }

  // If cqService has not started, then no cq exists
  if (m_cqService) {
    return m_cqService->getCqServiceStatistics();
  }

  return nullptr;
}

void RemoteQueryService::receiveNotification(TcrMessage& msg) {
  {
    TryReadGuard guard(m_rwLock, m_invalid);

    if (m_invalid) {
      //  do we need this check?
      return;
    }
    /*if cq has not been started, then  no cq exists */
    if (!m_cqService) {
      return;
    }

    if (!m_cqService->checkAndAcquireLock()) {
      return;
    }
  }

  m_cqService->receiveNotification(msg);
}

std::shared_ptr<CacheableArrayList>
RemoteQueryService::getAllDurableCqsFromServer() const {
  TryReadGuard guard(m_rwLock, m_invalid);

  if (m_invalid) {
    throw CacheClosedException(
        "QueryService::getAllDurableCqsFromServer: Cache has been closed.");
  }

  // If cqService has not started, then no cq exists
  if (m_cqService) {
    return m_cqService->getAllDurableCqsFromServer();
  }

  return nullptr;
}

void RemoteQueryService::invokeCqConnectedListeners(ThinClientPoolDM* pool,
                                                    bool connected) {
  if (!m_cqService) {
    return;
  }

  std::string poolName;
  pool = dynamic_cast<ThinClientPoolDM*>(m_tccdm);
  if (pool != nullptr) {
    poolName = pool->getName();
    CqPoolsConnected::iterator itr = m_CqPoolsConnected.find(poolName);
    if (itr != m_CqPoolsConnected.end() && itr->second == connected) {
      LOG_DEBUG("Returning since pools connection status matched.");
      return;
    } else {
      LOG_DEBUG("Inserting since pools connection status did not match.");
      m_CqPoolsConnected[poolName] = connected;
    }
  }
  m_cqService->invokeCqConnectedListeners(poolName, connected);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
