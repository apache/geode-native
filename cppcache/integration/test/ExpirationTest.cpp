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

#include <future>
#include <iostream>
#include <random>
#include <thread>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/CacheableBuiltins.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "framework/Cluster.h"
#include "framework/Framework.h"
#include "framework/Gfsh.h"

namespace {

using apache::geode::client::Cache;
using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableString;
using apache::geode::client::ExpirationAction;
using apache::geode::client::Pool;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

using std::chrono::minutes;

const int TEST_TTL_ENTRY_TIMEOUT = 5;

Cache createCache() {
  using apache::geode::client::CacheFactory;

  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("statistic-sampling-enabled", "false")
                   .create();

  return cache;
}

std::shared_ptr<Region> setupRegion(Cache& cache) {
  auto region = cache.createRegionFactory(RegionShortcut::LOCAL)
                    .setEntryTimeToLive(ExpirationAction::LOCAL_INVALIDATE,
                                        std::chrono::seconds(5))
                    .create("region");

  return region;
}

TEST(ExpirationTest, verifyExpiration) {
  auto cache = createCache();
  auto region = setupRegion(cache);

  region->put(1, "one");
  std::this_thread::sleep_for(std::chrono::seconds(3));
  ASSERT_TRUE(region->containsValueForKey(1));
  auto result = std::dynamic_pointer_cast<CacheableString>(region->get(1));
  ASSERT_NE(result, nullptr);
  ASSERT_EQ(result->value(), "one");

  std::this_thread::sleep_for(std::chrono::seconds(5));
  ASSERT_FALSE(region->containsValueForKey(1));
}

}  // namespace
