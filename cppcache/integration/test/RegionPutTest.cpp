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

#include <gmock/gmock.h>

#include <chrono>
#include <future>
#include <iostream>
#include <random>
#include <thread>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/CacheTransactionManager.hpp>
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
using apache::geode::client::CommitConflictException;
using apache::geode::client::Pool;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

using std::chrono::minutes;

using ::testing::Eq;
using ::testing::IsNull;
using ::testing::NotNull;

Cache createCache() {
  using apache::geode::client::CacheFactory;

  auto cache = CacheFactory()
                   .set("log-level", "debug")
                   .set("statistic-sampling-enabled", "false")
                   .create();

  return cache;
}

std::shared_ptr<Pool> createPool(Cluster& cluster, Cache& cache) {
  auto poolFactory = cache.getPoolManager().createFactory();
  cluster.applyLocators(poolFactory);
  return poolFactory.create("default");
}

std::shared_ptr<Region> setupRegion(Cache& cache,
                                    const std::shared_ptr<Pool>& pool) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName(pool->getName())
                    .create("region");

  return region;
}

std::shared_ptr<Region> setupCachingRegion(Cache& cache,
                                           const std::shared_ptr<Pool>& pool) {
  auto region = cache.createRegionFactory(RegionShortcut::CACHING_PROXY)
                    .setPoolName(pool->getName())
                    .create("region");

  return region;
}

TEST(RegionPutTest, testPutIfAbsentNotExistingEntry) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = createCache();
  auto pool = createPool(cluster, cache);
  auto region = setupRegion(cache, pool);
  auto key = CacheableKey::create("key-1");
  auto value = CacheableString::create("value");

  EXPECT_THAT(region->putIfAbsent(key, value), IsNull());

  auto retrieved = region->get(key);
  EXPECT_THAT(retrieved, NotNull());

  auto converted = std::dynamic_pointer_cast<CacheableString>(retrieved);
  EXPECT_THAT(converted, NotNull());
  EXPECT_THAT(converted->toString(), Eq(value->toString()));
}

TEST(RegionPutTest, testPutIfAbsentNotExistingEntryWithCaching) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = createCache();
  auto pool = createPool(cluster, cache);
  auto region = setupCachingRegion(cache, pool);
  auto key = CacheableKey::create("key-1");
  auto value = CacheableString::create("value");

  EXPECT_THAT(region->putIfAbsent(key, value), IsNull());

  auto retrieved = region->get(key);
  EXPECT_THAT(retrieved, NotNull());

  auto converted = std::dynamic_pointer_cast<CacheableString>(retrieved);
  EXPECT_THAT(converted, NotNull());
  EXPECT_THAT(converted->toString(), Eq(value->toString()));

  // Verify cached value matches expected
  auto entry = region->getEntry(key);
  EXPECT_THAT(entry, NotNull());

  retrieved = entry->getValue();
  EXPECT_THAT(converted, NotNull());

  converted = std::dynamic_pointer_cast<CacheableString>(retrieved);
  EXPECT_THAT(converted, NotNull());
  EXPECT_THAT(converted->toString(), Eq(value->toString()));
}

TEST(RegionPutTest, testPutIfAbsentExistingEntry) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = createCache();
  auto pool = createPool(cluster, cache);
  auto region = setupRegion(cache, pool);
  auto key = CacheableKey::create("key-1");

  EXPECT_NO_THROW(region->put(key, CacheableString::create("previous-value")));

  auto value = std::dynamic_pointer_cast<CacheableString>(region->get(key));

  auto previous =
      region->putIfAbsent(key, CacheableString::create("new-value"));
  EXPECT_THAT(previous, NotNull());

  auto converted = std::dynamic_pointer_cast<CacheableString>(previous);
  EXPECT_THAT(converted, NotNull());
  EXPECT_THAT(converted->toString(), Eq(value->toString()));
}

TEST(RegionPutTest, testPutIfAbsentExistingEntryCaching) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = createCache();
  auto pool = createPool(cluster, cache);
  auto region = setupCachingRegion(cache, pool);
  auto key = CacheableKey::create("key-1");

  EXPECT_NO_THROW(region->put(key, CacheableString::create("previous-value")));

  auto value = std::dynamic_pointer_cast<CacheableString>(region->get(key));

  auto previous =
      region->putIfAbsent(key, CacheableString::create("new-value"));
  EXPECT_THAT(previous, NotNull());

  auto converted = std::dynamic_pointer_cast<CacheableString>(previous);
  EXPECT_THAT(converted, NotNull());
  EXPECT_THAT(converted->toString(), Eq(value->toString()));

  // Verify cached value matches expected
  auto entry = region->getEntry(key);
  EXPECT_THAT(entry, NotNull());

  auto retrieved = entry->getValue();
  EXPECT_THAT(converted, NotNull());

  converted = std::dynamic_pointer_cast<CacheableString>(retrieved);
  EXPECT_THAT(converted, NotNull());
  EXPECT_THAT(converted->toString(), Eq(value->toString()));
}

TEST(RegionPutTest, testPutIfAbsentWIthinTxAndOutOfTx) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = createCache();
  auto pool = createPool(cluster, cache);
  auto region = setupRegion(cache, pool);
  auto key = CacheableKey::create("key-1");
  auto value = CacheableString::create("value");
  auto txMgr = cache.getCacheTransactionManager();

  txMgr->begin();
  EXPECT_THAT(region->putIfAbsent(key, value), IsNull());

  auto retrieved = region->get(key);
  EXPECT_THAT(retrieved, NotNull());

  auto converted = std::dynamic_pointer_cast<CacheableString>(retrieved);
  EXPECT_THAT(converted, NotNull());
  EXPECT_THAT(converted->toString(), Eq(value->toString()));
  auto& txId = txMgr->suspend();

  EXPECT_THAT(region->putIfAbsent(key, value), IsNull());

  txMgr->resume(txId);
  EXPECT_THROW(txMgr->commit(), CommitConflictException);
}

TEST(RegionPutTest, testPutIfAbsentExistingEntryTX) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = createCache();
  auto pool = createPool(cluster, cache);
  auto region = setupRegion(cache, pool);
  auto key = CacheableKey::create("key-1");
  EXPECT_NO_THROW(region->put(key, CacheableString::create("previous-value")));

  auto value = std::dynamic_pointer_cast<CacheableString>(region->get(key));

  auto previous =
      region->putIfAbsent(key, CacheableString::create("new-value"));
  EXPECT_THAT(previous, NotNull());

  auto converted = std::dynamic_pointer_cast<CacheableString>(previous);
  EXPECT_THAT(converted, NotNull());
  EXPECT_THAT(converted->toString(), Eq(value->toString()));
}

}  // namespace
