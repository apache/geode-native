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
#include <PoolAttributes.hpp>
#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <ProxyCache.hpp>
#include <ThinClientPoolHADM.hpp>
/**
 * @file
 */

using namespace apache::geode::client;

Pool::Pool(PoolAttributesPtr attr) : m_attrs(attr) {}
Pool::~Pool() {}

int Pool::getFreeConnectionTimeout() const {
  return m_attrs->getFreeConnectionTimeout();
}
int Pool::getLoadConditioningInterval() const {
  return m_attrs->getLoadConditioningInterval();
}
int Pool::getSocketBufferSize() const { return m_attrs->getSocketBufferSize(); }
int Pool::getReadTimeout() const { return m_attrs->getReadTimeout(); }
int Pool::getMinConnections() const { return m_attrs->getMinConnections(); }
int Pool::getMaxConnections() const { return m_attrs->getMaxConnections(); }
long Pool::getIdleTimeout() const { return m_attrs->getIdleTimeout(); }
long Pool::getPingInterval() const { return m_attrs->getPingInterval(); }
long Pool::getUpdateLocatorListInterval() const {
  return m_attrs->getUpdateLocatorListInterval();
}
int Pool::getStatisticInterval() const {
  return m_attrs->getStatisticInterval();
}
int Pool::getRetryAttempts() const { return m_attrs->getRetryAttempts(); }
// bool Pool::getThreadLocalConnections() const { return false; }
bool Pool::getSubscriptionEnabled() const {
  return m_attrs->getSubscriptionEnabled();
}
int Pool::getSubscriptionRedundancy() const {
  return m_attrs->getSubscriptionRedundancy();
}
int Pool::getSubscriptionMessageTrackingTimeout() const {
  return m_attrs->getSubscriptionMessageTrackingTimeout();
}
int Pool::getSubscriptionAckInterval() const {
  return m_attrs->getSubscriptionAckInterval();
}
const char* Pool::getServerGroup() const { return m_attrs->getServerGroup(); }
bool Pool::getThreadLocalConnections() const {
  return m_attrs->getThreadLocalConnectionSetting();
}
bool Pool::getMultiuserAuthentication() const {
  return m_attrs->getMultiuserSecureModeEnabled();
}
RegionServicePtr Pool::createSecureUserCache(PropertiesPtr credentials,
                                             CacheImpl* cacheImpl) {
  if (this->getMultiuserAuthentication()) {
    if (cacheImpl == nullptr) {
      throw IllegalStateException("cache has not been created yet.");
    }

    if (cacheImpl->isClosed()) {
      throw IllegalStateException("cache has been closed. ");
    }

    if (credentials != nullptr && credentials.get() == nullptr) {
      LOGDEBUG("Pool::createSecureUserCache creds are null");
      credentials = nullptr;
    }

    // TODO: this will return cache with userattribtes
    return std::make_shared<ProxyCache>(credentials, shared_from_this(),
                                        cacheImpl);
  }

  throw IllegalStateException(
      "This operation is only allowed when attached pool is in "
      "multiuserSecureMode");
  // return nullptr;
}
bool Pool::getPRSingleHopEnabled() const {
  return m_attrs->getPRSingleHopEnabled();
}
// void Pool::releaseThreadLocalConnection(){}

int Pool::getPendingEventCount() const {
  const auto poolHADM = dynamic_cast<const ThinClientPoolHADM*>(this);
  if (nullptr == poolHADM || poolHADM->isReadyForEvent()) {
    LOGERROR("This operation should only be called before readyForEvents.");
    throw IllegalStateException(
        "This operation should only be called before readyForEvents");
  }
  TcrConnectionManager& tccm = poolHADM->getConnectionManager();
  if (!tccm.isDurable()) {
    LOGERROR("This operation should only be called by durable client.");
    throw IllegalStateException(
        "This operation should only be called by durable client");
  }
  return poolHADM->getPrimaryServerQueueSize();
}
