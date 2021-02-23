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

#include <framework/Cluster.h>
#include <framework/Gfsh.h>

#include <random>
#include <thread>

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/CacheTransactionManager.hpp>
#include <geode/Region.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

namespace {
using apache::geode::client::Cache;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheTransactionManager;
using apache::geode::client::CommitConflictException;
using apache::geode::client::Exception;
using apache::geode::client::IllegalStateException;
using apache::geode::client::Pool;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

const std::string regionName = "region";

Cache createCache() {
  return CacheFactory()
      .set("statistic-sampling-enabled", "false")
      .set("log-level", "none")
      .create();
}

std::shared_ptr<Pool> createPool(Cluster& cluster, Cache& cache) {
  auto poolFactory = cache.getPoolManager().createFactory();
  cluster.applyLocators(poolFactory);
  poolFactory.setPRSingleHopEnabled(true);
  return poolFactory.create("default");
}

std::shared_ptr<Region> setupRegion(Cache& cache) {
  return cache.createRegionFactory(RegionShortcut::PROXY)
      .setPoolName("default")
      .create(regionName);
}

void runClientOperations(Cache& cache, std::shared_ptr<Region> region,
                         int minEntryKey, int maxEntryKey, int numTx) {
  std::random_device randomDevice;
  std::default_random_engine randomEngine(randomDevice());
  std::uniform_int_distribution<decltype(maxEntryKey)> distribution(
      minEntryKey, maxEntryKey);

  auto transactionManager = cache.getCacheTransactionManager();

  for (int i = 0; i < numTx; i++) {
    auto theKey = distribution(randomEngine);
    std::string theValue = "theValue";
    try {
      transactionManager->begin();
      region->put(theKey, theValue);
      transactionManager->commit();
    } catch (...) {
      if (transactionManager->exists()) {
        transactionManager->rollback();
      }
    }
  }
}

TEST(TransactionsTest, ExceptionWhenRollingBackTx) {
  int NUM_THREADS = 16;
  int MAX_ENTRY_KEY = 100000;
  int TX_PER_CLIENT = 1000;
  auto keyRangeSize = (MAX_ENTRY_KEY / NUM_THREADS);

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
  auto region = setupRegion(cache);

  std::vector<std::thread> clientThreads;
  for (int i = 0; i < NUM_THREADS; i++) {
    auto minKey = (i * keyRangeSize);
    auto maxKey = minKey + keyRangeSize - 1;
    std::thread th(runClientOperations, std::ref(cache), region, minKey, maxKey,
                   TX_PER_CLIENT);
    clientThreads.push_back(std::move(th));
  }

  cluster.getServers()[1].stop();

  for (std::thread& th : clientThreads) {
    if (th.joinable()) {
      th.join();
    }
  }

}  // TEST

TEST(TransactionsTest, IlegalStateExceptionNoTx) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.start();

  // Create regions
  cluster.getGfsh()
      .create()
      .region()
      .withName(regionName)
      .withType("PARTITION")
      .execute();

  auto cache = createCache();
  auto txm = cache.getCacheTransactionManager();
  auto pool = createPool(cluster, cache);
  auto region = setupRegion(cache);

  EXPECT_THROW(txm->prepare(), IllegalStateException);
  EXPECT_THROW(txm->commit(), IllegalStateException);
  EXPECT_THROW(txm->rollback(), IllegalStateException);
}  // TEST

TEST(TransactionsTest, ExceptionConflictOnPrepare) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.start();

  // Create regions
  cluster.getGfsh()
      .create()
      .region()
      .withName(regionName)
      .withType("PARTITION")
      .execute();

  auto cache = createCache();
  auto txm = cache.getCacheTransactionManager();
  auto pool = createPool(cluster, cache);
  auto region = setupRegion(cache);

  txm->begin();
  region->put("key", "A");
  auto& tx_first_id = txm->suspend();
  txm->begin();
  region->put("key", "B");
  txm->prepare();
  auto& tx_second_id = txm->suspend();
  txm->resume(tx_first_id);

  EXPECT_THROW(txm->prepare(), CommitConflictException);
  txm->resume(tx_second_id);
  txm->rollback();

}  // TEST

TEST(TransactionsTest, ExceptionConflictOnCommit) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.start();

  // Create regions
  cluster.getGfsh()
      .create()
      .region()
      .withName(regionName)
      .withType("PARTITION")
      .execute();

  auto cache = createCache();
  auto txm = cache.getCacheTransactionManager();
  auto pool = createPool(cluster, cache);
  auto region = setupRegion(cache);

  txm->begin();
  region->put("key", "A");
  auto& tx_id = txm->suspend();
  txm->begin();
  region->put("key", "B");
  txm->commit();
  txm->resume(tx_id);

  EXPECT_THROW(txm->commit(), CommitConflictException);
}  // TEST

}  // namespace
