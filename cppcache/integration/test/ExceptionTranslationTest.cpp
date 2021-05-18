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

#include <random>

#include <gtest/gtest.h>

#include <geode/QueryService.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "framework/Cluster.h"

using apache::geode::client::Cache;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::LowMemoryException;
using apache::geode::client::Pool;
using apache::geode::client::QueryExecutionLowMemoryException;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

using std::chrono::minutes;

namespace {
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
  return poolFactory.create("default");
}

std::shared_ptr<Region> setupRegion(Cache& cache,
                                    const std::shared_ptr<Pool>& pool) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName(pool->getName())
                    .create("region");

  return region;
}

void createEntries(const std::shared_ptr<Region>& region, std::size_t count,
                   std::size_t size) {
  std::mt19937 generator{std::random_device{}()};
  std::string base{"ABCDEFGHIJKLMNOPQRSTUVWXYZ"};

  while (base.length() < size) {
    base += base;
  }
  base = base.substr(0, size);

  for (std::size_t i = 0U; i < count;) {
    std::string value_str{base};
    std::shuffle(value_str.begin(), value_str.end(), generator);

    auto key = CacheableKey::create(std::to_string(i++));
    auto value = CacheableString::create(value_str);
    region->put(key, value);
  }
}
}  // namespace

TEST(ExceptionTranslationTest, testLowMemoryException) {
  const auto ENTRY_SIZE = 1024U;
  const auto ENTRIES = 1U << 20U;

  const auto xml_files = CacheXMLFiles{
      {getFrameworkString(FrameworkVariable::NewTestResourcesDir) +
       std::string{"/lowmemory_cacheserver.xml"}}};
  Cluster cluster{LocatorCount{1}, ServerCount{1}, xml_files};

  cluster.start();

  auto cache = createCache();
  auto pool = createPool(cluster, cache);
  auto region = setupRegion(cache, pool);

  EXPECT_THROW(createEntries(region, ENTRIES, ENTRY_SIZE), LowMemoryException);
}

TEST(ExceptionTranslationTest, testQueryLowMemoryException) {
  const auto ENTRY_SIZE = 1024U;
  const auto ENTRIES = 1U << 20U;

  const auto xml_files = CacheXMLFiles{
      {getFrameworkString(FrameworkVariable::NewTestResourcesDir) +
       std::string{"/lowmemory_cacheserver.xml"}}};
  Cluster cluster{LocatorCount{1}, ServerCount{1}, xml_files};

  cluster.start();

  auto cache = createCache();
  auto pool = createPool(cluster, cache);
  auto region = setupRegion(cache, pool);

  EXPECT_THROW(createEntries(region, ENTRIES, ENTRY_SIZE), LowMemoryException);
  auto query = pool->getQueryService()->newQuery("SELECT * FROM /region");
  EXPECT_THROW(query->execute(), QueryExecutionLowMemoryException);
}
