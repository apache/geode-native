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
using apache::geode::client::Pool;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

std::shared_ptr<Cache> createCache() {
  auto cache = CacheFactory().set("log-level", "none").create();
  return std::make_shared<Cache>(std::move(cache));
}

std::shared_ptr<Pool> createPool(Cluster& cluster,
                                 std::shared_ptr<Cache> cache) {
  auto poolFactory = cache->getPoolManager().createFactory();
  cluster.applyLocators(poolFactory);
  poolFactory.setPRSingleHopEnabled(true);
  return poolFactory.create("default");
}

void runClientOperations(std::shared_ptr<Cache> cache,
                         std::shared_ptr<Region> region, int minEntryKey,
                         int maxEntryKey, int numTx) {
  auto transactionManager = cache->getCacheTransactionManager();

  for (int i = 0; i < numTx; i++) {
    auto theKey = (rand() % (maxEntryKey - minEntryKey)) + minEntryKey;
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
  auto region = cache->createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .create("region");

  std::vector<std::thread> clientThreads;
  for (int i = 0; i < NUM_THREADS; i++) {
    auto minKey = (i * keyRangeSize);
    auto maxKey = minKey + keyRangeSize - 1;
    std::thread th(runClientOperations, cache, region, minKey, maxKey,
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

}  // namespace
