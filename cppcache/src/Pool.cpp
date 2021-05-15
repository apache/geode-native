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
#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/Pool.hpp>

#include "CacheImpl.hpp"
#include "PoolAttributes.hpp"
#include "TcrConnectionManager.hpp"
#include "ThinClientPoolHADM.hpp"

namespace apache {
namespace geode {
namespace client {

Pool::Pool(std::shared_ptr<PoolAttributes> attr) : m_attrs(attr) {}
Pool::~Pool() {}

std::chrono::milliseconds Pool::getFreeConnectionTimeout() const {
  return m_attrs->getFreeConnectionTimeout();
}

std::chrono::milliseconds Pool::getLoadConditioningInterval() const {
  return m_attrs->getLoadConditioningInterval();
}

int Pool::getSocketBufferSize() const { return m_attrs->getSocketBufferSize(); }

std::chrono::milliseconds Pool::getReadTimeout() const {
  return m_attrs->getReadTimeout();
}

std::string Pool::getSniProxyHost() const { return m_attrs->getSniProxyHost(); }
int Pool::getSniProxyPort() const { return m_attrs->getSniProxyPort(); }

int Pool::getMinConnections() const { return m_attrs->getMinConnections(); }

int Pool::getMaxConnections() const { return m_attrs->getMaxConnections(); }

std::chrono::milliseconds Pool::getIdleTimeout() const {
  return m_attrs->getIdleTimeout();
}

std::chrono::milliseconds Pool::getPingInterval() const {
  return m_attrs->getPingInterval();
}

std::chrono::milliseconds Pool::getUpdateLocatorListInterval() const {
  return m_attrs->getUpdateLocatorListInterval();
}

std::chrono::milliseconds Pool::getStatisticInterval() const {
  return m_attrs->getStatisticInterval();
}

int Pool::getRetryAttempts() const { return m_attrs->getRetryAttempts(); }

bool Pool::getSubscriptionEnabled() const {
  return m_attrs->getSubscriptionEnabled();
}

int Pool::getSubscriptionRedundancy() const {
  return m_attrs->getSubscriptionRedundancy();
}

std::chrono::milliseconds Pool::getSubscriptionMessageTrackingTimeout() const {
  return m_attrs->getSubscriptionMessageTrackingTimeout();
}

std::chrono::milliseconds Pool::getSubscriptionAckInterval() const {
  return m_attrs->getSubscriptionAckInterval();
}

const std::string& Pool::getServerGroup() const {
  return m_attrs->getServerGroup();
}

bool Pool::getThreadLocalConnections() const {
  return m_attrs->getThreadLocalConnectionSetting();
}

bool Pool::getMultiuserAuthentication() const {
  return m_attrs->getMultiuserSecureModeEnabled();
}

AuthenticatedView Pool::createAuthenticatedView(
    std::shared_ptr<Properties> credentials, CacheImpl* cacheImpl) {
  if (this->getMultiuserAuthentication()) {
    if (cacheImpl == nullptr) {
      throw IllegalStateException("cache has not been created yet.");
    }

    if (cacheImpl->isClosed()) {
      throw IllegalStateException("cache has been closed. ");
    }

    if (!credentials) {
      LOG_DEBUG("Pool::createSecureUserCache creds are null");
    }

    return AuthenticatedView(credentials, shared_from_this(), cacheImpl);
  }

  throw IllegalStateException(
      "This operation is only allowed when attached pool is in "
      "multiuserSecureMode");
}

bool Pool::getPRSingleHopEnabled() const {
  return m_attrs->getPRSingleHopEnabled();
}

int Pool::getPendingEventCount() const {
  const auto poolHADM = dynamic_cast<const ThinClientPoolHADM*>(this);
  if (nullptr == poolHADM || poolHADM->isReadyForEvent()) {
    LOG_ERROR("This operation should only be called before readyForEvents.");
    throw IllegalStateException(
        "This operation should only be called before readyForEvents");
  }
  TcrConnectionManager& tccm = poolHADM->getConnectionManager();
  if (!tccm.isDurable()) {
    LOG_ERROR("This operation should only be called by durable client.");
    throw IllegalStateException(
        "This operation should only be called by durable client");
  }
  return poolHADM->getPrimaryServerQueueSize();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
