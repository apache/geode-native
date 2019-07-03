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

#include <geode/CqAttributesFactory.hpp>
#include <geode/QueryService.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "CacheRegionHelper.hpp"
#include "SimpleCqListener.hpp"
#include "framework/Cluster.h"
#include "framework/Framework.h"
#include "framework/Gfsh.h"

namespace {

using apache::geode::client::Cache;
using apache::geode::client::Cacheable;
using apache::geode::client::CacheableKey;
using apache::geode::client::CqAttributesFactory;
using apache::geode::client::HashMapOfCacheable;
using apache::geode::client::Pool;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

using std::chrono::minutes;

//
// TODO: Use a random number of entries.  Need to investigate how to log this
// from/import it to a test first.
//
const int32_t CQ_TEST_REGION_ENTRY_COUNT = 100;

Cache createCache() {
  using apache::geode::client::CacheFactory;

  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("statistic-sampling-enabled", "false")
                   .create();

  return cache;
}

std::shared_ptr<Pool> createPool(Cluster& cluster, Cache& cache) {
  auto poolFactory = cache.getPoolManager().createFactory();
  cluster.applyLocators(poolFactory);
  poolFactory.setPRSingleHopEnabled(true);
  poolFactory.setSubscriptionEnabled(true);
  return poolFactory.create("default");
}

std::shared_ptr<Region> setupRegion(Cache& cache,
                                    const std::shared_ptr<Pool>& pool) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName(pool->getName())
                    .create("region");

  return region;
}

TEST(CqTest, testCqCreateUpdateDestroy) {
  Cluster cluster{LocatorCount{1}, ServerCount{2}};
  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  auto cache = createCache();
  auto pool = createPool(cluster, cache);
  auto region = setupRegion(cache, pool);
  auto queryService = cache.getQueryService();

  CqAttributesFactory attributesFactory;
  auto testListener = std::make_shared<SimpleCqListener>();
  attributesFactory.addCqListener(testListener);
  auto cqAttributes = attributesFactory.create();

  auto query =
      queryService->newCq("SimpleCQ", "SELECT * FROM /region", cqAttributes);

  query->execute();

  for (int i = 0; i < CQ_TEST_REGION_ENTRY_COUNT; i++) {
    region->put("key" + std::to_string(i), "value" + std::to_string(i));
  }

  for (int i = 0; i < CQ_TEST_REGION_ENTRY_COUNT; i++) {
    region->put("key" + std::to_string(i), "value" + std::to_string(i + 1));
  }

  for (int i = 0; i < CQ_TEST_REGION_ENTRY_COUNT; i++) {
    region->destroy("key" + std::to_string(i));
  }

  for (int i = 0; i < 100; i++) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (testListener->getCreationCount() == CQ_TEST_REGION_ENTRY_COUNT) {
      break;
    }
  }

  ASSERT_EQ(testListener->getCreationCount(), CQ_TEST_REGION_ENTRY_COUNT);
  ASSERT_EQ(testListener->getUpdateCount(), CQ_TEST_REGION_ENTRY_COUNT);
  ASSERT_EQ(testListener->getDestructionCount(), CQ_TEST_REGION_ENTRY_COUNT);
}

}  // namespace
