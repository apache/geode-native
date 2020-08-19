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
      m_serverGrp(PoolFactory::DEFAULT_SERVER_GROUP),
      m_sniProxyPort(0) {}

std::shared_ptr<PoolAttributes> PoolAttributes::clone() {
  return std::make_shared<PoolAttributes>(*this);
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

}  // namespace client
}  // namespace geode
}  // namespace apache
