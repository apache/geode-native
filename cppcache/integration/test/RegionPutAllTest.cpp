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
                   .set("log-file", "RegionPutAllTest.log")
                   .set("log-file-size-limit", "100")
                   .set("log-disk-space-limit", "1000")
                   .set("statistic-sampling-enabled", "false")
                   .create();

  return cache;
}

std::shared_ptr<Pool> createPool(Cluster& cluster, Cache& cache) {
  auto poolFactory = cache.getPoolManager().createFactory();
  cluster.applyLocators(poolFactory);
  poolFactory.setPRSingleHopEnabled(true);
  return poolFactory.create("default");
}

std::shared_ptr<Region> setupRegion(Cache& cache,
                                    const std::shared_ptr<Pool>& pool) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName(pool->getName())
                    .create("region");

  return region;
}

TEST(RegionPutAllTest, putAllToPartitionedRegion) {
  Cluster cluster{LocatorCount{1}, ServerCount{2}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  auto cache = createCache();
  auto pool = createPool(cluster, cache);
  auto region = setupRegion(cache, pool);

  HashMapOfCacheable all;
  for (int i = 0; i < 100; i++) {
    region->put(CacheableKey::create(i), Cacheable::create(std::to_string(i)));
    all.emplace(CacheableKey::create(i), Cacheable::create(std::to_string(i)));
  }

  for (int i = 0; i < 100; i++) {
    // TODO some way force synchronous client metadata update first.
    region->putAll(all);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

//
// verifies that putall works when not all metadata is present, i.e. not all
// buckets exist yet on the cluster.
//
TEST(RegionPutAllTest, putAllAndVerifyKeysExist) {
  Cluster cluster{LocatorCount{1}, ServerCount{3}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  auto cache = createCache();
  auto pool = createPool(cluster, cache);
  auto region = setupRegion(cache, pool);

  for (int i = 0; i < 50; i++) {
    region->put(std::to_string(i), Cacheable::create(i));
  }

  HashMapOfCacheable all;
  for (int i = 0; i < 113; i++) {
    all.emplace(CacheableKey::create(std::to_string(i)), Cacheable::create(i));
  }

  std::this_thread::sleep_for(std::chrono::seconds(10));
  region->putAll(all);
  for (auto& key : all) {
    ASSERT_TRUE(region->containsKeyOnServer(key.first));
  }
}
}  // namespace
