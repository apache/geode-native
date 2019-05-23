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

#include "PoolManagerImpl.hpp"

#include <geode/Pool.hpp>
#include <geode/PoolFactory.hpp>
#include <geode/Region.hpp>

#include "CacheImpl.hpp"

namespace apache {
namespace geode {
namespace client {

void PoolManagerImpl::removePool(const std::string& name) {
  std::lock_guard<std::recursive_mutex> guard(m_connectionPoolsLock);
  m_connectionPools.erase(name);
}
PoolFactory PoolManagerImpl::createFactory() const {
  return PoolFactory(*m_cache->getCache());
}

void PoolManagerImpl::close(bool keepAlive) {
  std::lock_guard<std::recursive_mutex> guard(m_connectionPoolsLock);

  std::vector<std::shared_ptr<Pool>> poolsList;

  for (const auto& c : m_connectionPools) {
    poolsList.push_back(c.second);
  }

  for (const auto& iter : poolsList) {
    iter->destroy(keepAlive);
  }
}

std::shared_ptr<Pool> PoolManagerImpl::find(const std::string& name) const {
  std::lock_guard<std::recursive_mutex> guard(m_connectionPoolsLock);

  if (name.empty()) {
    return m_connectionPools.empty() ? nullptr
                                     : m_connectionPools.begin()->second;
  } else {
    const auto& iter = m_connectionPools.find(name);

    std::shared_ptr<Pool> poolPtr = nullptr;

    if (iter != m_connectionPools.end()) {
      poolPtr = iter->second;
    }

    return poolPtr;
  }
}

std::shared_ptr<Pool> PoolManagerImpl::find(
    std::shared_ptr<Region> region) const {
  return find(region->getAttributes().getPoolName());
}

const HashMapOfPools& PoolManagerImpl::getAll() const {
  return m_connectionPools;
}

void PoolManagerImpl::addPool(std::string name,
                              const std::shared_ptr<Pool>& pool) {
  std::lock_guard<std::recursive_mutex> guard(m_connectionPoolsLock);

  if (!m_defaultPool) {
    m_defaultPool = pool;
  }

  m_connectionPools.emplace(name, pool);
}

const std::shared_ptr<Pool>& PoolManagerImpl::getDefaultPool() const {
  return m_defaultPool;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
