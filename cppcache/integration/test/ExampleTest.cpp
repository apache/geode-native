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
#include <framework/Framework.h>
#include <framework/Gfsh.h>

#include <future>
#include <iostream>
#include <random>
#include <thread>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

namespace {

using apache::geode::client::Cache;
using apache::geode::client::CacheableString;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;
using std::chrono::minutes;

std::shared_ptr<Region> setupRegion(Cache& cache) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .create("region");

  return region;
}

/**
 * Example test using 2 servers and waiting for async tasks to synchronize using
 * furtures.
 */
TEST(ExampleTest, DISABLED_putAndGetWith2Servers) {
  Cluster cluster{LocatorCount{1}, ServerCount{2}};
  cluster.start();
  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto task1 = std::async(std::launch::async, [&] {
    auto cache = cluster.createCache();
    auto region = setupRegion(cache);

    region->put(1, "one");
  });

  auto task2 = std::async(std::launch::async, [&] {
    auto cache = cluster.createCache();
    auto region = setupRegion(cache);

    auto status = task1.wait_for(debug_safe(minutes(1)));
    ASSERT_EQ(std::future_status::ready, status);

    auto v1 = std::dynamic_pointer_cast<CacheableString>(region->get(1));

    EXPECT_EQ("one", v1->value());
  });
  ASSERT_EQ(std::future_status::ready, task2.wait_for(debug_safe(minutes(1))));
}

/**
 * Example test using single server and waiting for async put and update
 * operations to synchronize using promises.
 */
TEST(ExampleTest, DISABLED_putGetAndUpdateWith1Server) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.start();
  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  std::promise<void> putPromise;
  std::promise<void> updatePromise;

  auto task1 = std::async(std::launch::async, [&] {
    SCOPED_TRACE("task1");

    auto cache = cluster.createCache();
    auto region = setupRegion(cache);

    region->put(1, "one");
    putPromise.set_value();

    ASSERT_EQ(std::future_status::ready,
              updatePromise.get_future().wait_for(debug_safe(minutes(1))));

    auto v1 = std::dynamic_pointer_cast<CacheableString>(region->get(1));
    EXPECT_EQ("two", v1->value());
  });

  auto task2 = std::async(std::launch::async, [&] {
    SCOPED_TRACE("task2");

    auto cache = cluster.createCache();
    auto region = setupRegion(cache);

    ASSERT_EQ(std::future_status::ready,
              putPromise.get_future().wait_for(debug_safe(minutes(1))));

    auto v1 = std::dynamic_pointer_cast<CacheableString>(region->get(1));
    EXPECT_EQ("one", v1->value());

    region->put(1, "two");
    updatePromise.set_value();
  });
  ASSERT_EQ(std::future_status::ready, task2.wait_for(debug_safe(minutes(1))));
}

}  // namespace
