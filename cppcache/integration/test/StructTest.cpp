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
#include <hacks/range.h>

#include <iostream>
#include <unordered_map>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/PoolManager.hpp>
#include <geode/QueryService.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/Struct.hpp>

namespace {

using apache::geode::client::Cache;
using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableString;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;
using apache::geode::client::Struct;

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
  cluster.start();
  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = cluster.createCache();
  auto region = setupRegion(cache);

  std::unordered_map<int, std::string> values = {
      {1, "one"}, {2, "two"}, {3, "three"}};

  for (auto&& value : values) {
    region->put(value.first, value.second);
  }

  auto&& queryResult =
      cache.getQueryService()
          ->newQuery("SELECT e.key, e.value FROM /region.entries e")
          ->execute();
  EXPECT_EQ(3, queryResult->size());

  for (auto&& row : hacks::range(*queryResult)) {
    auto rowStruct = std::dynamic_pointer_cast<Struct>(row);
    ASSERT_NE(nullptr, rowStruct);
    EXPECT_EQ(2, rowStruct->size());

    auto key = -1;
    for (auto&& column : *rowStruct) {
      // Expect to read: key:int, value:string
      if (auto columnValue =
              std::dynamic_pointer_cast<CacheableInt32>(column)) {
        key = columnValue->value();
        EXPECT_NE(values.end(), values.find(key));
      } else if (auto columnValue =
                     std::dynamic_pointer_cast<CacheableString>(column)) {
        auto value = columnValue->value();
        EXPECT_EQ(values.find(key)->second, value);
      } else {
        FAIL() << "Column is not int or string.";
      }
    }
  }
}

}  // namespace
