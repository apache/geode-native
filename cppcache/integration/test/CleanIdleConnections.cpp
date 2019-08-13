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

#include <thread>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "framework/Cluster.h"
#include "framework/Framework.h"
#include "framework/Gfsh.h"

namespace {

using apache::geode::client::Cache;
using apache::geode::client::CacheImpl;
using apache::geode::client::CacheRegionHelper;
using apache::geode::client::Pool;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

CacheImpl* getCacheImpl(Cache* cptr) {
  return CacheRegionHelper::getCacheImpl(cptr);
}

Cache createCache() {
  using apache::geode::client::CacheFactory;

  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("statistic-sampling-enabled", "false")
                   .create();

  return cache;
}

std::shared_ptr<Pool> createPool(Cluster& cluster, Cache& cache,
                                 const int& minConns, const int& maxConns,
                                 const std::string& poolName) {
  auto poolFactory = cache.getPoolManager().createFactory();
  cluster.applyLocators(poolFactory);
  poolFactory.setPRSingleHopEnabled(true);
  poolFactory.setMinConnections(minConns);
  poolFactory.setMaxConnections(maxConns);
  return poolFactory.create(poolName);
}

std::shared_ptr<Region> setupRegion(Cache& cache,
                                    const std::shared_ptr<Pool>& pool) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName(pool->getName())
                    .create("region");

  return region;
}

void doGets(std::shared_ptr<Region> region, int entries) {
  for (auto i = 0; i < entries; i++) {
    region->get(i);
  }
}

TEST(CleanIdleConnectionsTest, cleanIdleConnectionsAfterOpsPaused) {
  Cluster cluster{LocatorCount{1}, ServerCount{2}};
  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  auto cache = createCache();
  auto minConns = 1;
  auto maxConns = -1;
  std::string poolName = "default";
  auto pool = createPool(cluster, cache, minConns, maxConns, poolName);
  auto region = setupRegion(cache, pool);

  int poolSize = getCacheImpl(&cache)->getPoolSize(poolName);
  ASSERT_EQ(poolSize, 0);

  std::this_thread::sleep_for(std::chrono::milliseconds(10000));

  poolSize = getCacheImpl(&cache)->getPoolSize(poolName);
  ASSERT_GE(poolSize, minConns);

  int entries = 10;
  for (auto i = 0; i < entries; i++) {
    region->put(i, "value");
  }

  std::vector<std::shared_ptr<std::thread>> tasks;
  int threads = 10;

  for (int i = 0; i < threads; i++) {
    std::shared_ptr<std::thread> threadAux =
        std::make_shared<std::thread>(doGets, region, entries);
    tasks.push_back(threadAux);
  }

  for (int i = 0; i < threads; i++) {
    tasks[i]->join();
  }

  poolSize = getCacheImpl(&cache)->getPoolSize(poolName);
  ASSERT_GT(poolSize, minConns);

  // As the default clientIdleTimeout is 5 secs, after 10 seconds
  // all idle connections must have been closed.
  std::this_thread::sleep_for(std::chrono::milliseconds(10000));

  poolSize = getCacheImpl(&cache)->getPoolSize(poolName);
  ASSERT_EQ(poolSize, minConns);
}

}  // namespace
