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

#include <iostream>
#include <unordered_map>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/Struct.hpp>
#include <geode/QueryService.hpp>

#include "framework/Framework.h"
#include "framework/Gfsh.h"
#include "framework/Cluster.h"

namespace {

using namespace apache::geode::client;

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
TEST(StructTest, queryResultForRange) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = cluster.createCache();
  auto region = setupRegion(cache);

  std::unordered_map<int, std::string> values = {
      {1, "one"}, {2, "two"}, {3, "3"}};

  region->put(1, "one");
  region->put(2, "two");
  region->put(3, "three");

  auto queryResult =
      cache.getQueryService()
          ->newQuery("SELECT e.key, e.value FROM /region.entries e")
          ->execute();
  for (auto&& row : *queryResult) {
    auto rowStruct = std::dynamic_pointer_cast<Struct>(row);
    ASSERT_NE(nullptr, rowStruct);

    auto key = -1;
    for (auto&& column : *rowStruct) {
      // Expect to read: key:int, value:string
      if (auto columnValue = std::dynamic_pointer_cast<CacheableInt32>(column)) {
        key = columnValue->value();
        EXPECT_NE(values.end(), values.find(key));
        break;
      }
      auto columnValue = std::dynamic_pointer_cast<CacheableString>(column);
      ASSERT_NE(nullptr, columnValue);
      auto value = columnValue->value();
      EXPECT_NE(values.find(key)->second, value);
    }
  }
}

}  // namespace
