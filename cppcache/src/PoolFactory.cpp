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

#include <geode/Pool.hpp>
#include <geode/PoolFactory.hpp>
#include <geode/SystemProperties.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "PoolAttributes.hpp"
#include "TcrConnectionManager.hpp"
#include "ThinClientPoolDM.hpp"
#include "ThinClientPoolHADM.hpp"
#include "ThinClientPoolStickyDM.hpp"
#include "ThinClientPoolStickyHADM.hpp"

namespace apache {
namespace geode {
namespace client {

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
    std::chrono::milliseconds::zero();

const std::chrono::milliseconds
    PoolFactory::DEFAULT_SUBSCRIPTION_MESSAGE_TRACKING_TIMEOUT =
        std::chrono::seconds{900};

const std::chrono::milliseconds PoolFactory::DEFAULT_SUBSCRIPTION_ACK_INTERVAL =
    std::chrono::seconds{100};

const std::string PoolFactory::DEFAULT_SERVER_GROUP = "";

PoolFactory::PoolFactory(const Cache& cache)
    : m_attrs(std::make_shared<PoolAttributes>()),
      m_isSubscriptionRedundancy(false),
      m_addedServerOrLocator(false),
      m_cache(cache) {}

PoolFactory& PoolFactory::setFreeConnectionTimeout(
    std::chrono::milliseconds connectionTimeout) {
  if (connectionTimeout <= std::chrono::milliseconds::zero()) {
    throw IllegalArgumentException("connectionTimeout must be greater than 0.");
  }

  m_attrs->setFreeConnectionTimeout(connectionTimeout);
  return *this;
}

PoolFactory& PoolFactory::setLoadConditioningInterval(
    std::chrono::milliseconds loadConditioningInterval) {
  if (loadConditioningInterval < std::chrono::milliseconds::zero()) {
    throw IllegalArgumentException(
        "loadConditioningInterval must be greater than or equal to 0.");
  }

  m_attrs->setLoadConditioningInterval(loadConditioningInterval);
  return *this;
}

PoolFactory& PoolFactory::setSocketBufferSize(int bufferSize) {
  m_attrs->setSocketBufferSize(bufferSize);
  return *this;
}

PoolFactory& PoolFactory::setThreadLocalConnections(
    bool threadLocalConnections) {
  m_attrs->setThreadLocalConnectionSetting(threadLocalConnections);
  return *this;
}

PoolFactory& PoolFactory::setReadTimeout(std::chrono::milliseconds timeout) {
  if (timeout <= std::chrono::milliseconds::zero()) {
    throw IllegalArgumentException("timeout must be greater than 0.");
  }

  m_attrs->setReadTimeout(timeout);
  return *this;
}

PoolFactory& PoolFactory::setMinConnections(int minConnections) {
  m_attrs->setMinConnections(minConnections);
  return *this;
}

PoolFactory& PoolFactory::setMaxConnections(int maxConnections) {
  m_attrs->setMaxConnections(maxConnections);
  return *this;
}

PoolFactory& PoolFactory::setIdleTimeout(
    std::chrono::milliseconds idleTimeout) {
  if (idleTimeout < std::chrono::milliseconds::zero()) {
    throw IllegalArgumentException(
        "idleTimeout must be greater than or equal to 0.");
  }

  m_attrs->setIdleTimeout(idleTimeout);
  return *this;
}

PoolFactory& PoolFactory::setRetryAttempts(int retryAttempts) {
  m_attrs->setRetryAttempts(retryAttempts);
  return *this;
}

PoolFactory& PoolFactory::setPingInterval(
    std::chrono::milliseconds pingInterval) {
  if (pingInterval <= std::chrono::milliseconds::zero()) {
    throw IllegalArgumentException("timeout must be greater than 0.");
  }

  m_attrs->setPingInterval(pingInterval);
  return *this;
}

PoolFactory& PoolFactory::setUpdateLocatorListInterval(
    const std::chrono::milliseconds updateLocatorListInterval) {
  if (updateLocatorListInterval < std::chrono::milliseconds::zero()) {
    throw IllegalArgumentException("timeout must be positive.");
  }

  m_attrs->setUpdateLocatorListInterval(updateLocatorListInterval);
  return *this;
}

PoolFactory& PoolFactory::setStatisticInterval(
    std::chrono::milliseconds statisticInterval) {
  if (statisticInterval < std::chrono::milliseconds::zero()) {
    throw IllegalArgumentException(
        "timeout must be greater than or equal to 0.");
  }

  m_attrs->setStatisticInterval(statisticInterval);
  return *this;
}

PoolFactory& PoolFactory::setServerGroup(std::string group) {
  m_attrs->setServerGroup(group);
  return *this;
}

PoolFactory& PoolFactory::addLocator(const std::string& host, int port) {
  addCheck(host, port);
  m_attrs->addLocator(host, port);
  m_addedServerOrLocator = true;
  return *this;
}

PoolFactory& PoolFactory::addServer(const std::string& host, int port) {
  addCheck(host, port);
  m_attrs->addServer(host, port);
  m_addedServerOrLocator = true;
  return *this;
}

PoolFactory& PoolFactory::setSniProxy(const std::string& hostname,
                                      const int port) {
  m_attrs->setSniProxyHost(hostname);
  m_attrs->setSniProxyPort(port);
  return *this;
}

PoolFactory& PoolFactory::setSubscriptionEnabled(bool enabled) {
  m_attrs->setSubscriptionEnabled(enabled);
  return *this;
}

PoolFactory& PoolFactory::setSubscriptionRedundancy(int redundancy) {
  m_isSubscriptionRedundancy = true;
  m_attrs->setSubscriptionRedundancy(redundancy);
  return *this;
}

PoolFactory& PoolFactory::setSubscriptionMessageTrackingTimeout(
    std::chrono::milliseconds messageTrackingTimeout) {
  if (messageTrackingTimeout <= std::chrono::milliseconds::zero()) {
    throw IllegalArgumentException("timeout must be greater than 0.");
  }

  m_attrs->setSubscriptionMessageTrackingTimeout(messageTrackingTimeout);
  return *this;
}

PoolFactory& PoolFactory::setSubscriptionAckInterval(
    std::chrono::milliseconds ackInterval) {
  if (ackInterval <= std::chrono::milliseconds::zero()) {
    throw IllegalArgumentException("timeout must be greater than 0.");
  }

  m_attrs->setSubscriptionAckInterval(ackInterval);
  return *this;
}

PoolFactory& PoolFactory::setMultiuserAuthentication(
    bool multiuserAuthentication) {
  m_attrs->setMultiuserSecureModeEnabled(multiuserAuthentication);
  return *this;
}

PoolFactory& PoolFactory::reset() {
  m_attrs = std::shared_ptr<PoolAttributes>(new PoolAttributes);
  return *this;
}

PoolFactory& PoolFactory::setPRSingleHopEnabled(bool enabled) {
  m_attrs->setPRSingleHopEnabled(enabled);
  return *this;
}
std::shared_ptr<Pool> PoolFactory::create(std::string name) {
  std::shared_ptr<ThinClientPoolDM> poolDM;

  auto&& poolManager = m_cache.getPoolManager();

  if (poolManager.find(name) != nullptr) {
    throw IllegalStateException("Pool with the same name already exists");
  }
  // Create a clone of Attr;
  auto copyAttrs = m_attrs->clone();

  auto cacheImpl = CacheRegionHelper::getCacheImpl(&m_cache);

  if (m_cache.isClosed()) {
    throw CacheClosedException("Cache is closed");
  }

  auto&& tccm = cacheImpl->tcrConnectionManager();

  LOGDEBUG("PoolFactory::create mulitusermode = %d ",
           copyAttrs->getMultiuserSecureModeEnabled());
  if (copyAttrs->getMultiuserSecureModeEnabled()) {
    if (copyAttrs->getThreadLocalConnectionSetting()) {
      LOGERROR(
          "When pool [%s] is in multiuser authentication mode then thread "
          "local connections are not supported.",
          name.c_str());
      throw IllegalArgumentException(
          "When pool is in multiuser authentication mode then thread local "
          "connections are not supported.");
    }
  }
  if (!copyAttrs->getSubscriptionEnabled() &&
      copyAttrs->getSubscriptionRedundancy() == 0 && !tccm.isDurable()) {
    if (copyAttrs->getThreadLocalConnectionSetting()) {
      // TODO: what should we do for sticky connections
      poolDM = std::make_shared<ThinClientPoolStickyDM>(name.c_str(), copyAttrs,
                                                        tccm);
    } else {
      LOGDEBUG("ThinClientPoolDM created ");
      poolDM =
          std::make_shared<ThinClientPoolDM>(name.c_str(), copyAttrs, tccm);
    }
  } else {
    LOGDEBUG("ThinClientPoolHADM created ");
    if (copyAttrs->getThreadLocalConnectionSetting()) {
      poolDM = std::make_shared<ThinClientPoolStickyHADM>(name.c_str(),
                                                          copyAttrs, tccm);
    } else {
      poolDM =
          std::make_shared<ThinClientPoolHADM>(name.c_str(), copyAttrs, tccm);
    }
  }

  poolManager.addPool(name, poolDM);

  // TODO: poolDM->init() should not throw exceptions!
  // Pool DM should only be inited once.
  if (cacheImpl->getDistributedSystem()
          .getSystemProperties()
          .autoReadyForEvents()) {
    poolDM->init();
  }

  return std::move(poolDM);
}

PoolFactory& PoolFactory::addCheck(const std::string& host, int port) {
  if (port <= 0) {
    throw IllegalArgumentException("port must be greater than 0 but was " +
                                   std::to_string(port));
  }

  if (m_attrs->getSniProxyHost().empty()) {
    ACE_INET_Addr addr(port, host.c_str());
#ifdef WITH_IPV6
    // check unknown host
    // ACE will not initialize port if hostname is not resolved.
    if (port != addr.get_port_number()) {
#else
    if (!(addr.get_ip_address())) {
#endif
      throw IllegalArgumentException("Unknown host " + host);
    }
  }
  return *this;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
