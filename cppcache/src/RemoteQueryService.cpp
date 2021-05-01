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

#include <boost/thread/lock_types.hpp>

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
  LOGFINEST("Initialized m_tccdm");
}

void RemoteQueryService::init() {
  boost::unique_lock<decltype(mutex_)> guard{mutex_};
  if (m_invalid) {
    LOGFINEST("RemoteQueryService::init: initializing TCCDM");
    if (dynamic_cast<ThinClientCacheDistributionManager*>(m_tccdm)) {
      m_tccdm->init();
    }

    m_invalid = false;
    LOGFINEST("RemoteQueryService::init: done initialization");
  }
}

std::shared_ptr<Query> RemoteQueryService::newQuery(std::string querystring) {
  boost::shared_lock<decltype(mutex_)> guard{mutex_};
  if (m_invalid) {
    throw CacheClosedException(
        "QueryService::newQuery: Cache has been closed.");
  }

  LOGDEBUG("RemoteQueryService::newQuery: multiuserMode = %d ",
           m_tccdm->isMultiUserMode());
  LOGDEBUG("RemoteQueryService: creating a new query: " + querystring);

  if (m_tccdm->isMultiUserMode()) {
    return std::make_shared<RemoteQuery>(
        querystring, shared_from_this(), m_tccdm,
        UserAttributes::threadLocalUserAttributes->getAuthenticatedView());
  } else {
    return std::make_shared<RemoteQuery>(querystring, shared_from_this(),
                                         m_tccdm);
  }
}

void RemoteQueryService::close() {
  LOGFINEST("RemoteQueryService::close: starting close");
  boost::unique_lock<decltype(mutex_)> guard{mutex_};

  if (m_cqService != nullptr) {
    LOGFINEST("RemoteQueryService::close: starting CQ service close");
    m_cqService->closeCqService();
    m_cqService = nullptr;
    LOGFINEST("RemoteQueryService::close: completed CQ service close");
  }

  if (dynamic_cast<ThinClientCacheDistributionManager*>(m_tccdm)) {
    if (!m_invalid) {
      LOGFINEST("RemoteQueryService::close: destroying DM");
      m_tccdm->destroy();
    }
    _GEODE_SAFE_DELETE(m_tccdm);
    m_invalid = true;
  }

  if (!m_CqPoolsConnected.empty()) {
    m_CqPoolsConnected.clear();
  }

  LOGFINEST("RemoteQueryService::close: completed");
}

/**
 * execute all cqs on the endpoint after failover
 */
GfErrType RemoteQueryService::executeAllCqs(TcrEndpoint* endpoint) {
  boost::shared_lock<decltype(mutex_)> guard{mutex_};
  if (m_invalid) {
    LOGFINE("QueryService::executeAllCqs(endpoint): Not initialized.");
    return GF_NOERR;
  }

  if (m_cqService == nullptr) {
    LOGFINE("RemoteQueryService: no cq to execute after failover to endpoint[" +
            endpoint->name() + "]");
    return GF_NOERR;
  } else {
    LOGFINE("RemoteQueryService: execute all cqs after failover to endpoint[" +
            endpoint->name() + "]");
    return m_cqService->executeAllClientCqs(endpoint);
  }
}

void RemoteQueryService::executeAllCqs(bool failover) {
  boost::shared_lock<decltype(mutex_)> guard{mutex_};
  if (m_invalid) {
    LOGFINE("QueryService::executeAllCqs: Not initialized.");
    return;
  }

  /*if cq has not been started, then failover will not start it.*/
  if (m_cqService != nullptr) {
    LOGFINE("RemoteQueryService: execute all cqs after failover");
    m_cqService->executeAllClientCqs(failover);
  } else {
    LOGFINE("RemoteQueryService: no cq to execute after failover");
  }
}

std::shared_ptr<CqQuery> RemoteQueryService::newCq(
    std::string querystr, const std::shared_ptr<CqAttributes>& cqAttr,
    bool isDurable) {
  boost::shared_lock<decltype(mutex_)> guard{mutex_};

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
  boost::shared_lock<decltype(mutex_)> guard{mutex_};

  if (m_invalid) {
    throw CacheClosedException("QueryService::newCq: Cache has been closed.");
  }

  initCqService();
  return m_cqService->newCq(name, querystr, cqAttr, isDurable);
}

void RemoteQueryService::closeCqs() {
  boost::shared_lock<decltype(mutex_)> guard{mutex_};

  if (m_invalid) {
    LOGFINE("QueryService::closeCqs: Cache has been closed.");
    return;
  }

  // If cqService has not started, then no cq exists
  if (m_cqService != nullptr) {
    m_cqService->closeAllCqs();
  }
}

CqService::query_container_type RemoteQueryService::getCqs() const {
  boost::shared_lock<decltype(mutex_)> guard{mutex_};

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
  boost::shared_lock<decltype(mutex_)> guard{mutex_};

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
  boost::shared_lock<decltype(mutex_)> guard{mutex_};

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
  boost::shared_lock<decltype(mutex_)> guard{mutex_};

  if (m_invalid) {
    LOGFINE("QueryService::stopCqs: Cache has been closed.");
    return;
  }

  // If cqService has not started, then no cq exists
  if (m_cqService != nullptr) {
    m_cqService->stopAllClientCqs();
  }
}

std::shared_ptr<CqServiceStatistics>
RemoteQueryService::getCqServiceStatistics() const {
  boost::shared_lock<decltype(mutex_)> guard{mutex_};

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
    boost::shared_lock<decltype(mutex_)> guard{mutex_};
    if (m_invalid || !m_cqService || !m_cqService->checkAndAcquireLock()) {
      return;
    }
  }

  m_cqService->receiveNotification(msg);
}

std::shared_ptr<CacheableArrayList>
RemoteQueryService::getAllDurableCqsFromServer() const {
  boost::shared_lock<decltype(mutex_)> guard{mutex_};

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
      LOGDEBUG("Returning since pools connection status matched.");
      return;
    } else {
      LOGDEBUG("Inserting since pools connection status did not match.");
      m_CqPoolsConnected[poolName] = connected;
    }
  }
  m_cqService->invokeCqConnectedListeners(poolName, connected);
}

boost::shared_mutex& RemoteQueryService::getMutex() { return mutex_; }

}  // namespace client
}  // namespace geode
}  // namespace apache
