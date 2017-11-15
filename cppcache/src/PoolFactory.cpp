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

#include <ace/Recursive_Thread_Mutex.h>
#include <ace/INET_Addr.h>

#include <geode/PoolFactory.hpp>
#include <geode/Pool.hpp>
#include <geode/SystemProperties.hpp>
#include <geode/PoolManager.hpp>

#include "CacheImpl.hpp"
#include "PoolAttributes.hpp"
#include "ThinClientPoolDM.hpp"
#include "ThinClientPoolHADM.hpp"
#include "ThinClientPoolStickyDM.hpp"
#include "ThinClientPoolStickyHADM.hpp"
#include "CacheRegionHelper.hpp"

using namespace apache::geode::client;

constexpr const char* PoolFactory::DEFAULT_SERVER_GROUP;

const std::chrono::milliseconds PoolFactory::DEFAULT_FREE_CONNECTION_TIMEOUT =
    std::chrono::seconds{10};

const std::chrono::milliseconds
    PoolFactory::DEFAULT_LOAD_CONDITIONING_INTERVAL = std::chrono::minutes{5};

const std::chrono::milliseconds PoolFactory::DEFAULT_READ_TIMEOUT =
    std::chrono::seconds{10};

const std::chrono::milliseconds PoolFactory::DEFAULT_IDLE_TIMEOUT =
    std::chrono::seconds{5};

const std::chrono::milliseconds PoolFactory::DEFAULT_PING_INTERVAL =
    std::chrono::seconds{10};

const std::chrono::milliseconds
    PoolFactory::DEFAULT_UPDATE_LOCATOR_LIST_INTERVAL = std::chrono::seconds{5};

const std::chrono::milliseconds PoolFactory::DEFAULT_STATISTIC_INTERVAL =
    std::chrono::milliseconds{-1};

const std::chrono::milliseconds
    PoolFactory::DEFAULT_SUBSCRIPTION_MESSAGE_TRACKING_TIMEOUT =
        std::chrono::seconds{900};

const std::chrono::milliseconds PoolFactory::DEFAULT_SUBSCRIPTION_ACK_INTERVAL =
    std::chrono::seconds{100};

PoolFactory::PoolFactory(const Cache& cache)
    : m_attrs(std::make_shared<PoolAttributes>()),
      m_isSubscriptionRedundancy(false),
      m_addedServerOrLocator(false),
      m_cache(cache) {}

PoolFactory::~PoolFactory() {}

void PoolFactory::setFreeConnectionTimeout(
    std::chrono::milliseconds connectionTimeout) {
  // TODO GEODE-3136 - Is this true?
  if (connectionTimeout <= std::chrono::milliseconds::zero()) {
    throw std::invalid_argument("connectionTimeout must greater than 0.");
  }

  m_attrs->setFreeConnectionTimeout(connectionTimeout);
}

void PoolFactory::setLoadConditioningInterval(
    std::chrono::milliseconds loadConditioningInterval) {
  // TODO GEODE-3136 - Is this true?
  if (loadConditioningInterval <= std::chrono::milliseconds::zero()) {
    throw std::invalid_argument(
        "loadConditioningInterval must greater than 0.");
  }

  m_attrs->setLoadConditioningInterval(loadConditioningInterval);
}

void PoolFactory::setSocketBufferSize(int bufferSize) {
  m_attrs->setSocketBufferSize(bufferSize);
}

void PoolFactory::setThreadLocalConnections(bool threadLocalConnections) {
  m_attrs->setThreadLocalConnectionSetting(threadLocalConnections);
}

void PoolFactory::setReadTimeout(std::chrono::milliseconds timeout) {
  // TODO GEODE-3136 - Is this true?
  if (timeout <= std::chrono::milliseconds::zero()) {
    throw std::invalid_argument("timeout must greater than 0.");
  }

  m_attrs->setReadTimeout(timeout);
}

void PoolFactory::setMinConnections(int minConnections) {
  m_attrs->setMinConnections(minConnections);
}

void PoolFactory::setMaxConnections(int maxConnections) {
  m_attrs->setMaxConnections(maxConnections);
}

void PoolFactory::setIdleTimeout(std::chrono::milliseconds idleTimeout) {
  m_attrs->setIdleTimeout(idleTimeout);
}

void PoolFactory::setRetryAttempts(int retryAttempts) {
  m_attrs->setRetryAttempts(retryAttempts);
}

void PoolFactory::setPingInterval(std::chrono::milliseconds pingInterval) {
  // TODO GEODE-3136 - Is this true?
  if (pingInterval <= std::chrono::milliseconds::zero()) {
    throw std::invalid_argument("timeout must greater than 0.");
  }

  m_attrs->setPingInterval(pingInterval);
}

void PoolFactory::setUpdateLocatorListInterval(
    const std::chrono::milliseconds updateLocatorListInterval) {
  // TODO GEODE-3136 - Is this true?
  if (updateLocatorListInterval < std::chrono::milliseconds::zero()) {
    throw std::invalid_argument("timeout must be positive.");
  }

  m_attrs->setUpdateLocatorListInterval(updateLocatorListInterval);
}

void PoolFactory::setStatisticInterval(
    std::chrono::milliseconds statisticInterval) {
  // TODO GEODE-3136 - Consider 0 to disable
  if (statisticInterval.count() <= -1) {
    throw std::invalid_argument("timeout must greater than -1.");
  }

  m_attrs->setStatisticInterval(statisticInterval);
}

void PoolFactory::setServerGroup(const char* group) {
  m_attrs->setServerGroup(group);
}

void PoolFactory::addLocator(const char* host, int port) {
  addCheck(host, port);
  m_attrs->addLocator(host, port);
  m_addedServerOrLocator = true;
}

void PoolFactory::addServer(const char* host, int port) {
  addCheck(host, port);
  m_attrs->addServer(host, port);
  m_addedServerOrLocator = true;
}

void PoolFactory::setSubscriptionEnabled(bool enabled) {
  m_attrs->setSubscriptionEnabled(enabled);
}

void PoolFactory::setSubscriptionRedundancy(int redundancy) {
  m_isSubscriptionRedundancy = true;
  m_attrs->setSubscriptionRedundancy(redundancy);
}

void PoolFactory::setSubscriptionMessageTrackingTimeout(
    std::chrono::milliseconds messageTrackingTimeout) {
  // TODO GEODE-3136 - Is this true?
  if (messageTrackingTimeout <= std::chrono::milliseconds::zero()) {
    throw std::invalid_argument("timeout must greater than 0.");
  }

  m_attrs->setSubscriptionMessageTrackingTimeout(messageTrackingTimeout);
}

void PoolFactory::setSubscriptionAckInterval(
    std::chrono::milliseconds ackInterval) {
  // TODO GEODE-3136 - Is this true?
  if (ackInterval <= std::chrono::milliseconds::zero()) {
    throw std::invalid_argument("timeout must greater than 0.");
  }

  m_attrs->setSubscriptionAckInterval(ackInterval);
}

void PoolFactory::setMultiuserAuthentication(bool multiuserAuthentication) {
  m_attrs->setMultiuserSecureModeEnabled(multiuserAuthentication);
}

void PoolFactory::reset() {
  m_attrs = std::shared_ptr<PoolAttributes>(new PoolAttributes);
}

void PoolFactory::setPRSingleHopEnabled(bool enabled) {
  m_attrs->setPRSingleHopEnabled(enabled);
}
std::shared_ptr<Pool> PoolFactory::create(const char* name) {
  std::shared_ptr<ThinClientPoolDM> poolDM;
  {
    if (m_cache.getPoolManager().find(name) != nullptr) {
      throw IllegalStateException("Pool with the same name already exists");
    }
    // Create a clone of Attr;
    auto copyAttrs = m_attrs->clone();

    CacheImpl* cacheImpl = CacheRegionHelper::getCacheImpl(&m_cache);

    if (m_cache.isClosed()) {
      throw CacheClosedException("Cache is closed");
    }
    if (cacheImpl->getCacheMode() && m_isSubscriptionRedundancy) {
      LOGWARN(
          "At least one pool has been created so ignoring cache level "
          "redundancy setting");
    }
    TcrConnectionManager& tccm = cacheImpl->tcrConnectionManager();

    LOGDEBUG("PoolFactory::create mulitusermode = %d ",
             copyAttrs->getMultiuserSecureModeEnabled());
    if (copyAttrs->getMultiuserSecureModeEnabled()) {
      if (copyAttrs->getThreadLocalConnectionSetting()) {
        LOGERROR(
            "When pool [%s] is in multiuser authentication mode then thread "
            "local connections are not supported.",
            name);
        throw IllegalArgumentException(
            "When pool is in multiuser authentication mode then thread local "
            "connections are not supported.");
      }
    }
    if (!copyAttrs->getSubscriptionEnabled() &&
        copyAttrs->getSubscriptionRedundancy() == 0 && !tccm.isDurable()) {
      if (copyAttrs
              ->getThreadLocalConnectionSetting() /*&& !copyAttrs->getPRSingleHopEnabled()*/) {
        // TODO: what should we do for sticky connections
        poolDM =
            std::make_shared<ThinClientPoolStickyDM>(name, copyAttrs, tccm);
      } else {
        LOGDEBUG("ThinClientPoolDM created ");
        poolDM = std::make_shared<ThinClientPoolDM>(name, copyAttrs, tccm);
      }
    } else {
      LOGDEBUG("ThinClientPoolHADM created ");
      if (copyAttrs
              ->getThreadLocalConnectionSetting() /*&& !copyAttrs->getPRSingleHopEnabled()*/) {
        poolDM =
            std::make_shared<ThinClientPoolStickyHADM>(name, copyAttrs, tccm);
      } else {
        poolDM = std::make_shared<ThinClientPoolHADM>(name, copyAttrs, tccm);
      }
    }

    cacheImpl->getPoolManager().addPool(name,
                                        std::static_pointer_cast<Pool>(poolDM));
  }

  // TODO: poolDM->init() should not throw exceptions!
  // Pool DM should only be inited once.
  if (m_cache.getDistributedSystem()
          .getSystemProperties()
          .autoReadyForEvents()) {
    poolDM->init();
  }

  return std::static_pointer_cast<Pool>(poolDM);
}

void PoolFactory::addCheck(const char* host, int port) {
  if (port <= 0) {
    char buff[100];
    ACE_OS::snprintf(buff, 100, "port must be greater than 0 but was %d", port);
    throw IllegalArgumentException(buff);
  }
  ACE_INET_Addr addr(port, host);
  if (!(addr.get_ip_address())) {
    char buff[100];
    ACE_OS::snprintf(buff, 100, "Unknown host %s", host);
    throw IllegalArgumentException(buff);
  }
}
