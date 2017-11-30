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

#include "CacheRegionHelper.hpp"
#include "CacheImpl.hpp"

using namespace apache::geode::client;

class PoolManager::Impl {
 public:
  Impl(CacheImpl* cache) : m_cache(cache) {}
  void removePool(const char* name);

  std::shared_ptr<PoolFactory> createFactory();

  void close(bool keepAlive);

  std::shared_ptr<Pool> find(const char* name);

  std::shared_ptr<Pool> find(std::shared_ptr<Region> region);

  const HashMapOfPools& getAll();

  void addPool(const char* name, const std::shared_ptr<Pool>& pool);

  std::shared_ptr<Pool> getDefaultPool();

 private:
  HashMapOfPools m_connectionPools;
  std::recursive_mutex m_connectionPoolsLock;
  std::shared_ptr<Pool> m_defaultPool;
  CacheImpl* m_cache;
};

void PoolManager::Impl::removePool(const char* name) {
  std::lock_guard<std::recursive_mutex> guard(m_connectionPoolsLock);
  m_connectionPools.erase(name);
}

std::shared_ptr<PoolFactory> PoolManager::Impl::createFactory() {
  return std::shared_ptr<PoolFactory>(new PoolFactory(*m_cache->getCache()));
}

void PoolManager::Impl::close(bool keepAlive) {
  std::lock_guard<std::recursive_mutex> guard(m_connectionPoolsLock);

  std::vector<std::shared_ptr<Pool>> poolsList;

  for (const auto& c : m_connectionPools) {
    poolsList.push_back(c.second);
  }

  for (const auto& iter : poolsList) {
    iter->destroy(keepAlive);
  }
}
 std::shared_ptr<Pool> PoolManager::Impl::find(const char* name) {
   std::lock_guard<std::recursive_mutex> guard(m_connectionPoolsLock);

   if (name) {
     const auto& iter = m_connectionPools.find(name);

     std::shared_ptr<Pool> poolPtr = nullptr;

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
 std::shared_ptr<Pool> PoolManager::Impl::find(std::shared_ptr<Region> region) {
  return find(region->getAttributes()->getPoolName());
 }

 const HashMapOfPools& PoolManager::Impl::getAll() { return m_connectionPools; }

 void PoolManager::Impl::addPool(const char* name,
                                 const std::shared_ptr<Pool>& pool) {
   std::lock_guard<std::recursive_mutex> guard(m_connectionPoolsLock);

   if (!m_defaultPool) {
     m_defaultPool = pool;
   }

   m_connectionPools.emplace(name, pool);
}
std::shared_ptr<Pool> PoolManager::Impl::getDefaultPool() {
  return m_defaultPool;
}

PoolManager::PoolManager(CacheImpl* cache)
    : m_pimpl(new Impl(cache), [](Impl* impl) { delete impl; }) {}

void PoolManager::removePool(const char* name) { m_pimpl->removePool(name); }
std::shared_ptr<PoolFactory> PoolManager::createFactory() {
  return m_pimpl->createFactory();
}

void PoolManager::close(bool keepAlive) { m_pimpl->close(keepAlive); }
std::shared_ptr<Pool> PoolManager::find(const char* name) {
  return m_pimpl->find(name);
}
std::shared_ptr<Pool> PoolManager::find(std::shared_ptr<Region> region) {
  return m_pimpl->find(region);
 }

 const HashMapOfPools& PoolManager::getAll() { return m_pimpl->getAll(); }

 void PoolManager::addPool(const char* name,
                           const std::shared_ptr<Pool>& pool) {
   m_pimpl->addPool(name, pool);
 }
 std::shared_ptr<Pool> PoolManager::getDefaultPool() {
   return m_pimpl->getDefaultPool();
 }
