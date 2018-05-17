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

#include <geode/PoolManager.hpp>

#include "PoolManagerImpl.hpp"

namespace apache {
namespace geode {
namespace client {

PoolManager::PoolManager(CacheImpl* cache)
    : m_pimpl(std::make_shared<PoolManagerImpl>(cache)) {}

void PoolManager::removePool(const std::string& name) {
  m_pimpl->removePool(name);
}

PoolFactory PoolManager::createFactory() const {
  return m_pimpl->createFactory();
}

void PoolManager::close(bool keepAlive) { m_pimpl->close(keepAlive); }

std::shared_ptr<Pool> PoolManager::find(const std::string& name) const {
  return m_pimpl->find(name.c_str());
}

std::shared_ptr<Pool> PoolManager::find(
    const std::shared_ptr<Region>& region) const {
  return m_pimpl->find(region);
}

const HashMapOfPools& PoolManager::getAll() const { return m_pimpl->getAll(); }

void PoolManager::addPool(std::string name, const std::shared_ptr<Pool>& pool) {
  m_pimpl->addPool(name, pool);
}

const std::shared_ptr<Pool>& PoolManager::getDefaultPool() const {
  return m_pimpl->getDefaultPool();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
