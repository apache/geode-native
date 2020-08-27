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
#include <future>
#include <iostream>
#include <random>
#include <thread>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "CacheRegionHelper.hpp"
#include "framework/Cluster.h"
#include "framework/Framework.h"
#include "framework/Gfsh.h"

namespace {

using apache::geode::client::Cache;
using apache::geode::client::Cacheable;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::HashMapOfCacheable;
using apache::geode::client::Pool;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

using std::chrono::minutes;

Cache createCache() {
  using apache::geode::client::CacheFactory;

  auto cache = CacheFactory()
                   .set("log-level", "debug")
                   .set("statistic-sampling-enabled", "false")
                   .create();

  return cache;
}

std::shared_ptr<Pool> createPool(Cluster& cluster, Cache& cache,
                                 bool singleHop) {
  auto poolFactory = cache.getPoolManager().createFactory();
  cluster.applyLocators(poolFactory);
  poolFactory.setPRSingleHopEnabled(singleHop);
  return poolFactory.create("default");
}

std::shared_ptr<Region> setupRegion(Cache& cache,
                                    const std::shared_ptr<Pool>& pool) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName(pool->getName())
                    .create("region");

  return region;
}

void putEntries(std::shared_ptr<Region> region, int numEntries,
                int offsetForValue) {
  for (int i = 0; i < numEntries; i++) {
    auto key = CacheableKey::create(i);
    region->put(key, Cacheable::create(std::to_string(i + offsetForValue)));
  }
}

void getEntries(std::shared_ptr<Region> region, int numEntries) {
  for (int i = 0; i < numEntries; i++) {
    auto key = CacheableKey::create(i);
    auto value = region->get(key);
    ASSERT_NE(nullptr, value);
  }
}

void putPartitionedRegionWithRedundancyServerGoesDown(bool singleHop) {
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
  auto pool = createPool(cluster, cache, singleHop);
  auto region = setupRegion(cache, pool);

  int ENTRIES = 30;

  putEntries(region, ENTRIES, 0);

  cluster.getServers()[1].stop();

  putEntries(region, ENTRIES, 1);

  cluster.getServers()[1].start();

  putEntries(region, ENTRIES, 2);
}

void getPartitionedRegionWithRedundancyServerGoesDown(bool singleHop) {
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
  auto pool = createPool(cluster, cache, singleHop);
  auto region = setupRegion(cache, pool);

  int ENTRIES = 30;

  putEntries(region, ENTRIES, 0);

  getEntries(region, ENTRIES);

  cluster.getServers()[1].stop();

  getEntries(region, ENTRIES);

  cluster.getServers()[1].start();

  getEntries(region, ENTRIES);
}

/**
 * In this test case we verify that in a partition region with redundancy
 * when one server goes down, all gets are still served.
 * Single-hop is enabled in the client.
 * It can be observed in the logs that when one of the server goes down
 * the bucketServerLocations for that server are removed from the
 * client metadata.
 */
TEST(PartitionRegionOpsTest,
     getPartitionedRegionWithRedundancyServerGoesDownSingleHop) {
  getPartitionedRegionWithRedundancyServerGoesDown(true);
}

/**
 * In this test case we verify that in a partition region with redundancy
 * when one server goes down, all puts are still served.
 * Single-hop is enabled in the client.
 * It can be observed in the logs that when one of the server goes down
 * the bucketServerLocations for that server are removed from the
 * client metadata.
 * When the server is brought back again, the meta data is refreshed
 * after putting again values.
 */
TEST(PartitionRegionOpsTest,
     putPartitionedRegionWithRedundancyServerGoesDownSingleHop) {
  putPartitionedRegionWithRedundancyServerGoesDown(true);
}

/**
 * In this test case we verify that in a partition region with redundancy
 * when one server goes down, all gets are still served.
 * Single hop is not enabled in the client.
 */
TEST(PartitionRegionOpsTest,
     getPartitionedRegionWithRedundancyServerGoesDownNoSingleHop) {
  getPartitionedRegionWithRedundancyServerGoesDown(false);
}

/**
 * In this test case we verify that in a partition region with redundancy
 * when one server goes down, all puts are still served.
 * Single-hop is not enabled in the client.
 */
TEST(PartitionRegionOpsTest,
     putPartitionedRegionWithRedundancyServerGoesDownNoSingleHop) {
  putPartitionedRegionWithRedundancyServerGoesDown(false);
}

}  // namespace
