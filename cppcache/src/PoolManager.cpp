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
#include <mutex>

#include <geode/PoolManager.hpp>

using namespace apache::geode::client;

class PoolManager::Impl {
 public:
  Impl(const Cache& cache) : m_cache(cache) {}
  void removePool(const char* name);

  PoolFactoryPtr createFactory();

  void close(bool keepAlive);

  PoolPtr find(const char* name);

  PoolPtr find(RegionPtr region);

  const HashMapOfPools& getAll();

  void addPool(const char* name, const PoolPtr& pool);

  PoolPtr getDefaultPool();

 private:
  HashMapOfPools m_connectionPools;
  std::recursive_mutex m_connectionPoolsLock;
  PoolPtr m_defaultPool;
  const Cache& m_cache;
};

void PoolManager::Impl::removePool(const char* name) {
  std::lock_guard<std::recursive_mutex> guard(m_connectionPoolsLock);
  m_connectionPools.erase(name);
}

PoolFactoryPtr PoolManager::Impl::createFactory() {
  return std::shared_ptr<PoolFactory>(new PoolFactory(m_cache));
}

void PoolManager::Impl::close(bool keepAlive) {
  std::lock_guard<std::recursive_mutex> guard(m_connectionPoolsLock);

  std::vector<PoolPtr> poolsList;

  for (const auto& c : m_connectionPools) {
    poolsList.push_back(c.second);
  }

  for (const auto& iter : poolsList) {
    iter->destroy(keepAlive);
  }
}

PoolPtr PoolManager::Impl::find(const char* name) {
  std::lock_guard<std::recursive_mutex> guard(m_connectionPoolsLock);

  if (name) {
    const auto& iter = m_connectionPools.find(name);

    PoolPtr poolPtr = nullptr;

    if (iter != m_connectionPools.end()) {
      poolPtr = iter->second;
      GF_DEV_ASSERT(poolPtr != nullptr);
    }

    return poolPtr;
  } else {
    return m_connectionPools.empty() ? nullptr
                                     : m_connectionPools.begin()->second;
  }
}

PoolPtr PoolManager::Impl::find(RegionPtr region) {
  return find(region->getAttributes()->getPoolName());
}

const HashMapOfPools& PoolManager::Impl::getAll() { return m_connectionPools; }

void PoolManager::Impl::addPool(const char* name, const PoolPtr& pool) {
  std::lock_guard<std::recursive_mutex> guard(m_connectionPoolsLock);

  if (!m_defaultPool) {
    m_defaultPool = pool;
  }

  m_connectionPools.emplace(name, pool);
}

PoolPtr PoolManager::Impl::getDefaultPool() { return m_defaultPool; }

PoolManager::PoolManager(const Cache& cache)
    : m_pimpl(new Impl(cache), [](Impl* impl) { delete impl; }) {}

void PoolManager::removePool(const char* name) { m_pimpl->removePool(name); }

PoolFactoryPtr PoolManager::createFactory() { return m_pimpl->createFactory(); }

void PoolManager::close(bool keepAlive) { m_pimpl->close(keepAlive); }

PoolPtr PoolManager::find(const char* name) { return m_pimpl->find(name); }

PoolPtr PoolManager::find(RegionPtr region) { return m_pimpl->find(region); }

const HashMapOfPools& PoolManager::getAll() { return m_pimpl->getAll(); }

void PoolManager::addPool(const char* name, const PoolPtr& pool) {
  m_pimpl->addPool(name, pool);
}

PoolPtr PoolManager::getDefaultPool() { return m_pimpl->getDefaultPool(); }
