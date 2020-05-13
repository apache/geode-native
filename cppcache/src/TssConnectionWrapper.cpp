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

#include "TssConnectionWrapper.hpp"

#include <geode/Pool.hpp>

#include "TcrConnection.hpp"
#include "ThinClientPoolDM.hpp"

namespace apache {
namespace geode {
namespace client {

thread_local TssConnectionWrapper TssConnectionWrapper::instance_;

TssConnectionWrapper::TssConnectionWrapper() : connection_(nullptr) {}

TssConnectionWrapper::~TssConnectionWrapper() {
  // if cache close happening during this then we should NOT call this..
  if (connection_) {
    // this should be call in lock and release connection
    // but still race-condition is there if now cache-close starts happens
    // m_tcrConn->close();
    pool_->releaseThreadLocalConnection();
    connection_ = nullptr;
  }
}

void TssConnectionWrapper::setSHConnection(const TcrEndpoint& endpoint,
                                           TcrConnection* connection) {
  const auto& poolName = endpoint.getPoolHADM()->getName();
  PoolWrapper* poolWrapper = nullptr;
  const auto& iter = poolNameToPoolWrapperMap_.find(poolName);
  if (iter == poolNameToPoolWrapperMap_.end()) {
    poolWrapper = new PoolWrapper();
    poolNameToPoolWrapperMap_.emplace(poolName, poolWrapper);
  } else {
    poolWrapper = iter->second;
  }

  poolWrapper->setSHConnection(endpoint, connection);
}

TcrConnection* TssConnectionWrapper::getSHConnection(
    const TcrEndpoint& endpoint, const std::string& poolName) {
  const auto& iter = poolNameToPoolWrapperMap_.find(poolName);
  if (iter == poolNameToPoolWrapperMap_.end()) {
    return nullptr;
  }

  return iter->second->getSHConnection(endpoint);
}

void TssConnectionWrapper::releaseSHConnections(Pool& pool) {
  const auto& poolName = pool.getName();
  const auto& iter = poolNameToPoolWrapperMap_.find(poolName);
  if (iter == poolNameToPoolWrapperMap_.end()) {
    return;
  }

  iter->second->releaseSHConnections(pool);
  delete iter->second;
  poolNameToPoolWrapperMap_.erase(iter);
}

TcrConnection* TssConnectionWrapper::getAnyConnection(
    const std::string& poolName) const {
  const auto& iter = poolNameToPoolWrapperMap_.find(poolName);
  if (iter == poolNameToPoolWrapperMap_.end()) {
    return nullptr;
  }
  return iter->second->getAnyConnection();
}

TcrConnection* PoolWrapper::getSHConnection(const TcrEndpoint& ep) {
  const auto& iter = endpointsToConnectionMap_.find(ep.name());
  if (iter == endpointsToConnectionMap_.end()) {
    return nullptr;
  }

  auto tmp = iter->second;
  endpointsToConnectionMap_.erase(iter);
  return tmp;
}

void PoolWrapper::setSHConnection(const TcrEndpoint& ep, TcrConnection* conn) {
  endpointsToConnectionMap_.emplace(ep.name(), conn);
}

void PoolWrapper::releaseSHConnections(Pool& pool) {
  for (const auto& entry : endpointsToConnectionMap_) {
    auto connection = entry.second;
    connection->setAndGetBeingUsed(false, false);
    if (auto dm = dynamic_cast<ThinClientPoolDM*>(&pool)) {
      dm->put(connection, false);
    }
  }
  endpointsToConnectionMap_.clear();
}

TcrConnection* PoolWrapper::getAnyConnection() {
  const auto& iter = endpointsToConnectionMap_.begin();
  if (iter == endpointsToConnectionMap_.end()) {
    return nullptr;
  }

  auto connection = iter->second;
  endpointsToConnectionMap_.erase(iter);
  return connection;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
