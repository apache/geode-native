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
#include "PoolAttributes.hpp"

#include <geode/ExceptionTypes.hpp>
#include <geode/PoolFactory.hpp>

namespace apache {
namespace geode {
namespace client {

PoolAttributes::PoolAttributes()
    : m_isThreadLocalConn(PoolFactory::DEFAULT_THREAD_LOCAL_CONN),
      m_freeConnTimeout(PoolFactory::DEFAULT_FREE_CONNECTION_TIMEOUT),
      m_loadCondInterval(PoolFactory::DEFAULT_LOAD_CONDITIONING_INTERVAL),
      m_sockBufferSize(PoolFactory::DEFAULT_SOCKET_BUFFER_SIZE),
      m_readTimeout(PoolFactory::DEFAULT_READ_TIMEOUT),
      m_minConns(PoolFactory::DEFAULT_MIN_CONNECTIONS),
      m_maxConns(PoolFactory::DEFAULT_MAX_CONNECTIONS),
      m_retryAttempts(PoolFactory::DEFAULT_RETRY_ATTEMPTS),
      m_statsInterval(PoolFactory::DEFAULT_STATISTIC_INTERVAL),
      m_redundancy(PoolFactory::DEFAULT_SUBSCRIPTION_REDUNDANCY),
      m_msgTrackTimeout(
          PoolFactory::DEFAULT_SUBSCRIPTION_MESSAGE_TRACKING_TIMEOUT),
      m_subsAckInterval(PoolFactory::DEFAULT_SUBSCRIPTION_ACK_INTERVAL),
      m_idleTimeout(PoolFactory::DEFAULT_IDLE_TIMEOUT),
      m_pingInterval(PoolFactory::DEFAULT_PING_INTERVAL),
      m_updateLocatorListInterval(
          PoolFactory::DEFAULT_UPDATE_LOCATOR_LIST_INTERVAL),
      m_subsEnabled(PoolFactory::DEFAULT_SUBSCRIPTION_ENABLED),
      m_multiuserSecurityMode(PoolFactory::DEFAULT_MULTIUSER_SECURE_MODE),
      m_isPRSingleHopEnabled(PoolFactory::DEFAULT_PR_SINGLE_HOP_ENABLED),
      m_serverGrp(PoolFactory::DEFAULT_SERVER_GROUP) {}
std::shared_ptr<PoolAttributes> PoolAttributes::clone() {
  return std::make_shared<PoolAttributes>(*this);
}

/** Return true if all the attributes are equal to those of other. */
bool PoolAttributes::operator==(const PoolAttributes& other) const {
  if (m_isThreadLocalConn != other.m_isThreadLocalConn) return false;
  if (m_freeConnTimeout != other.m_freeConnTimeout) return false;
  if (m_loadCondInterval != other.m_loadCondInterval) return false;
  if (m_sockBufferSize != other.m_sockBufferSize) return false;
  if (m_readTimeout != other.m_readTimeout) return false;
  if (m_minConns != other.m_minConns) return false;
  if (m_maxConns != other.m_maxConns) return false;
  if (m_retryAttempts != other.m_retryAttempts) return false;
  if (m_statsInterval != other.m_statsInterval) return false;
  if (m_redundancy != other.m_redundancy) return false;
  if (m_msgTrackTimeout != other.m_msgTrackTimeout) return false;
  if (m_subsAckInterval != other.m_subsAckInterval) return false;
  if (m_idleTimeout != other.m_idleTimeout) return false;
  if (m_pingInterval != other.m_pingInterval) return false;
  if (m_updateLocatorListInterval != other.m_updateLocatorListInterval) {
    return false;
  }
  if (m_subsEnabled != other.m_subsEnabled) return false;
  if (m_multiuserSecurityMode != other.m_multiuserSecurityMode) return false;
  if (m_isPRSingleHopEnabled != other.m_isPRSingleHopEnabled) return false;
  if (m_serverGrp != other.m_serverGrp) return false;

  if (m_initLocList.size() != other.m_initLocList.size()) return false;
  if (m_initServList.size() != other.m_initServList.size()) return false;

  if (!compareVectorOfStrings(m_initLocList, other.m_initLocList)) return false;
  if (!compareVectorOfStrings(m_initServList, other.m_initServList)) {
    return false;
  }

  return true;
}

bool PoolAttributes::compareVectorOfStrings(
    const std::vector<std::string>& thisVector,
    const std::vector<std::string>& otherVector) {
  for (auto&& it : thisVector) {
    bool found = false;
    for (auto&& itOther : otherVector) {
      if (it == itOther) {
        found = true;
        break;
      }
    }

    if (!found) return false;
  }
  return true;
}

void PoolAttributes::addLocator(const std::string& host, int port) {
  if (!m_initServList.empty()) {
    throw IllegalArgumentException(
        "Cannot add both locators and servers to a pool");
  }
  m_initLocList.push_back(host + ":" + std::to_string(port));
}

void PoolAttributes::addServer(const std::string& host, int port) {
  if (!m_initLocList.empty()) {
    throw IllegalArgumentException(
        "Cannot add both locators and servers to a pool");
  }
  m_initServList.push_back(host + ":" + std::to_string(port));
}

const std::chrono::milliseconds& PoolAttributes::getFreeConnectionTimeout()
    const {
  return m_freeConnTimeout;
}

void PoolAttributes::setFreeConnectionTimeout(
    const std::chrono::milliseconds& connectionTimeout) {
  m_freeConnTimeout = connectionTimeout;
}

const std::chrono::milliseconds& PoolAttributes::getLoadConditioningInterval()
    const {
  return m_loadCondInterval;
}

void PoolAttributes::setLoadConditioningInterval(
    const std::chrono::milliseconds& loadConditioningInterval) {
  m_loadCondInterval = loadConditioningInterval;
}

int PoolAttributes::getSocketBufferSize() const { return m_sockBufferSize; }

void PoolAttributes::setSocketBufferSize(int bufferSize) {
  m_sockBufferSize = bufferSize;
}

const std::chrono::milliseconds& PoolAttributes::getReadTimeout() const {
  return m_readTimeout;
}

void PoolAttributes::setReadTimeout(const std::chrono::milliseconds& timeout) {
  m_readTimeout = timeout;
}

bool PoolAttributes::getThreadLocalConnectionSetting() {
  return m_isThreadLocalConn;
}

void PoolAttributes::setThreadLocalConnectionSetting(bool isThreadLocal) {
  m_isThreadLocalConn = isThreadLocal;
}

int PoolAttributes::getMinConnections() const { return m_minConns; }

void PoolAttributes::setMinConnections(int minConnections) {
  m_minConns = minConnections;
}

int PoolAttributes::getMaxConnections() const { return m_maxConns; }

void PoolAttributes::setMaxConnections(int maxConnections) {
  m_maxConns = maxConnections;
}

const std::chrono::milliseconds& PoolAttributes::getIdleTimeout() const {
  return m_idleTimeout;
}

void PoolAttributes::setIdleTimeout(
    const std::chrono::milliseconds& idleTimeout) {
  m_idleTimeout = idleTimeout;
}

int PoolAttributes::getRetryAttempts() const { return m_retryAttempts; }

void PoolAttributes::setRetryAttempts(int retryAttempts) {
  m_retryAttempts = retryAttempts;
}

const std::chrono::milliseconds& PoolAttributes::getPingInterval() const {
  return m_pingInterval;
}

void PoolAttributes::setPingInterval(
    const std::chrono::milliseconds& pingInterval) {
  m_pingInterval = pingInterval;
}

const std::chrono::milliseconds& PoolAttributes::getUpdateLocatorListInterval()
    const {
  return m_updateLocatorListInterval;
}

void PoolAttributes::setUpdateLocatorListInterval(
    const std::chrono::milliseconds& updateLocatorListInterval) {
  m_updateLocatorListInterval = updateLocatorListInterval;
}

const std::chrono::milliseconds& PoolAttributes::getStatisticInterval() const {
  return m_statsInterval;
}

void PoolAttributes::setStatisticInterval(
    const std::chrono::milliseconds& statisticInterval) {
  m_statsInterval = statisticInterval;
}

const std::string& PoolAttributes::getServerGroup() const {
  return m_serverGrp;
}

void PoolAttributes::setServerGroup(std::string group) { m_serverGrp = group; }

bool PoolAttributes::getSubscriptionEnabled() const { return m_subsEnabled; }

void PoolAttributes::setSubscriptionEnabled(bool enabled) {
  m_subsEnabled = enabled;
}

int PoolAttributes::getSubscriptionRedundancy() const { return m_redundancy; }

void PoolAttributes::setSubscriptionRedundancy(int redundancy) {
  m_redundancy = redundancy;
}

const std::chrono::milliseconds&
PoolAttributes::getSubscriptionMessageTrackingTimeout() const {
  return m_msgTrackTimeout;
}

void PoolAttributes::setSubscriptionMessageTrackingTimeout(
    const std::chrono::milliseconds& messageTrackingTimeout) {
  m_msgTrackTimeout = messageTrackingTimeout;
}

const std::chrono::milliseconds& PoolAttributes::getSubscriptionAckInterval()
    const {
  return m_subsAckInterval;
}

void PoolAttributes::setSubscriptionAckInterval(
    const std::chrono::milliseconds& ackInterval) {
  m_subsAckInterval = ackInterval;
}

bool PoolAttributes::getPRSingleHopEnabled() const {
  return m_isPRSingleHopEnabled;
}

void PoolAttributes::setPRSingleHopEnabled(bool enabled) {
  m_isPRSingleHopEnabled = enabled;
}

bool PoolAttributes::getMultiuserSecureModeEnabled() const {
  return m_multiuserSecurityMode;
}

void PoolAttributes::setMultiuserSecureModeEnabled(bool multiuserSecureMode) {
  m_multiuserSecurityMode = multiuserSecureMode;
}
}  // namespace client
}  // namespace geode
}  // namespace apache
