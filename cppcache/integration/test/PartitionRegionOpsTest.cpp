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

#include <chrono>
#include <fstream>
#include <future>
#include <iostream>
#include <random>
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
using apache::geode::client::Cacheable;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheImpl;
using apache::geode::client::CacheRegionHelper;
using apache::geode::client::HashMapOfCacheable;
using apache::geode::client::Pool;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

using std::chrono::minutes;

constexpr int ENTRIES = 113;
constexpr int WARMUP_ENTRIES = 1000;

Cache createCache() {
  using apache::geode::client::CacheFactory;

  auto cache =
      CacheFactory().set("statistic-sampling-enabled", "false").create();

  return cache;
}

std::shared_ptr<Pool> createPool(Cluster& cluster, Cache& cache,
                                 bool singleHop) {
  auto poolFactory = cache.getPoolManager().createFactory();
  cluster.applyLocators(poolFactory);
  poolFactory.setPRSingleHopEnabled(singleHop);
  poolFactory.setLoadConditioningInterval(std::chrono::milliseconds::zero());
  poolFactory.setIdleTimeout(std::chrono::milliseconds::zero());
  return poolFactory.create("default");
}

std::shared_ptr<Region> setupRegion(Cache& cache,
                                    const std::shared_ptr<Pool>& pool) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName(pool->getName())
                    .create("region");

  return region;
}

int putEntries(Cache& cache, std::shared_ptr<Region> region, int numEntries) {
  CacheImpl* cacheImpl = CacheRegionHelper::getCacheImpl(&cache);

  auto numPutsRequiringHop = 0;
  for (int i = 0; i < numEntries; i++) {
    auto key = CacheableKey::create(i);
    auto value = Cacheable::create(std::to_string(i));
    region->put(key, value);

    if (cacheImpl->getAndResetNetworkHopFlag()) {
      numPutsRequiringHop++;
    }
  }

  return numPutsRequiringHop;
}

int putAllEntries(Cache& cache, std::shared_ptr<Region> region,
                  int numEntries) {
  CacheImpl* cacheImpl = CacheRegionHelper::getCacheImpl(&cache);

  HashMapOfCacheable map;
  auto numPutAllsRequiringHop = 0;

  for (int i = 0; i < numEntries; i++) {
    auto key = CacheableKey::create(i);
    auto value = Cacheable::create(std::to_string(i));
    map.emplace(key, value);
  }

  region->putAll(map);

  if (cacheImpl->getAndResetNetworkHopFlag()) {
    numPutAllsRequiringHop++;
  }

  return numPutAllsRequiringHop;
}

int getEntries(Cache& cache, std::shared_ptr<Region> region, int numEntries) {
  CacheImpl* cacheImpl = CacheRegionHelper::getCacheImpl(&cache);

  auto numGetsRequiringHop = 0;
  for (int i = 0; i < numEntries; i++) {
    auto key = CacheableKey::create(i);
    auto value = region->get(key);

    if (cacheImpl->getAndResetNetworkHopFlag()) {
      numGetsRequiringHop++;
    }

    EXPECT_EQ(i, std::stoi(value->toString()));
  }

  return numGetsRequiringHop;
}

int getAllEntries(Cache& cache, std::shared_ptr<Region> region,
                  int numEntries) {
  CacheImpl* cacheImpl = CacheRegionHelper::getCacheImpl(&cache);

  std::vector<std::shared_ptr<CacheableKey>> keys{};
  HashMapOfCacheable expectedMap;
  for (int i = 0; i < numEntries; i++) {
    auto key = CacheableKey::create(i);
    keys.push_back(key);
    auto value = Cacheable::create(std::to_string(i));
    expectedMap.emplace(key, value);
  }

  HashMapOfCacheable actualMap = region->getAll(keys);

  int numGetAllsRequiringHop = 0;
  if (cacheImpl->getAndResetNetworkHopFlag()) {
    numGetAllsRequiringHop++;
  }

  for (int i = 0; i < numEntries; i++) {
    auto key = CacheableKey::create(i);
    EXPECT_EQ(expectedMap[key]->toString(), actualMap[key]->toString());
  }

  return numGetAllsRequiringHop;
}

int putget(bool useSingleHop) {
  Cluster cluster{LocatorCount{1}, ServerCount{2}};
  cluster.start();
  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .withRedundantCopies("1")
      .execute();

  auto cache = createCache();
  auto pool = createPool(cluster, cache, useSingleHop);
  auto region = setupRegion(cache, pool);

  // Warmup to get metaData
  putEntries(cache, region, WARMUP_ENTRIES);

  auto numOpsRequiringHop = putEntries(cache, region, ENTRIES);
  numOpsRequiringHop += getEntries(cache, region, ENTRIES);

  auto& targetServer = cluster.getServers()[1];
  targetServer.stop();
  targetServer.wait();

  numOpsRequiringHop += getEntries(cache, region, ENTRIES);

  targetServer.start();

  numOpsRequiringHop += getEntries(cache, region, ENTRIES);
  return numOpsRequiringHop;
}

int putAllgetAll(bool useSingleHop) {
  Cluster cluster{LocatorCount{1}, ServerCount{2}};
  cluster.start();
  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .withRedundantCopies("1")
      .execute();

  auto cache = createCache();
  auto pool = createPool(cluster, cache, useSingleHop);
  auto region = setupRegion(cache, pool);

  // Warmup to get metaData
  putAllEntries(cache, region, WARMUP_ENTRIES);

  auto numOpsRequiringHop = putAllEntries(cache, region, ENTRIES);
  numOpsRequiringHop += getAllEntries(cache, region, ENTRIES);

  auto& targetServer = cluster.getServers()[1];
  targetServer.stop();
  targetServer.wait();

  numOpsRequiringHop += getAllEntries(cache, region, ENTRIES);

  targetServer.start();

  numOpsRequiringHop += getAllEntries(cache, region, ENTRIES);
  return numOpsRequiringHop;
}

/**
 * In this test case we verify that in a partition region with redundancy
 * when one server goes down, all puts and gets are still served.
 *
 * Single-hop is enabled in the client.
 *
 */
TEST(PartitionRegionWithRedundancyTest, putgetWithSingleHop) {
  auto useSingleHop = true;
  auto numSingleHopsAfterWarmup = putget(useSingleHop);
  EXPECT_EQ(numSingleHopsAfterWarmup, 0);
}

/**
 * In this test case we verify that in a partition region with redundancy
 * when one server goes down, all putAlls and getAlls are still served.
 *
 * Single-hop is enabled in the client.
 *
 */
TEST(PartitionRegionWithRedundancyTest, putAllgetAllWithSingleHop) {
  auto useSingleHop = true;
  auto numSingleHopsAfterWarmup = putAllgetAll(useSingleHop);
  EXPECT_EQ(numSingleHopsAfterWarmup, 0);
}

/**
 * In this test case we verify that in a partition region with redundancy
 * when one server goes down, all puts and gets are still served.
 *
 * Single hop is not enabled in the client.
 *
 */
TEST(PartitionRegionWithRedundancyTest, putgetWithoutSingleHop) {
  auto useSingleHop = false;
  putget(useSingleHop);
}

/**
 * In this test case we verify that in a partition region with redundancy
 * when one server goes down, all putAlls and getAlls are still served.
 *
 * Single hop is not enabled in the client.
 *
 */
TEST(PartitionRegionWithRedundancyTest, putAllgetAllWithoutSingleHop) {
  auto useSingleHop = false;
  putAllgetAll(useSingleHop);
}

}  // namespace
