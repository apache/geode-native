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

#include <thread>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/CacheTransactionManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "framework/Cluster.h"

namespace {

using apache::geode::client::CacheableString;

std::shared_ptr<apache::geode::client::Region> setupRegion(
    apache::geode::client::Cache &cache) {
  auto region =
      cache.createRegionFactory(apache::geode::client::RegionShortcut::PROXY)
          .setPoolName("default")
          .create("region");
  return region;
}

TEST(ClientTransactionXATest, interTxand2PCTx) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  auto cache = cluster.createCache();
  auto region = setupRegion(cache);

  cache.getCacheTransactionManager()->begin();
  region->put("one", "one");
  cache.getCacheTransactionManager()->commit();
  auto v1 = std::dynamic_pointer_cast<CacheableString>(region->get("one"));
  EXPECT_EQ("one", v1->value());

  cache.getCacheTransactionManager()->begin();
  region->put("two", "two");
  cache.getCacheTransactionManager()->prepare();
  cache.getCacheTransactionManager()->commit();
  auto v2 = std::dynamic_pointer_cast<CacheableString>(region->get("two"));
  EXPECT_EQ("two", v2->value());

  cache.getCacheTransactionManager()->begin();
  region->put("two", "three");
  cache.getCacheTransactionManager()->prepare();
  cache.getCacheTransactionManager()->rollback();
  auto v3 = std::dynamic_pointer_cast<CacheableString>(region->get("two"));
  EXPECT_EQ("two", v3->value());

  cache.getCacheTransactionManager()->begin();
  region->put("one", "three");
  cache.getCacheTransactionManager()->rollback();
  auto v4 = std::dynamic_pointer_cast<CacheableString>(region->get("one"));
  EXPECT_EQ("one", v4->value());

  cache.getCacheTransactionManager()->begin();
  region->put("one", "two");
  cache.getCacheTransactionManager()->prepare();
  cache.getCacheTransactionManager()->commit();
  auto v5 = std::dynamic_pointer_cast<CacheableString>(region->get("one"));
  EXPECT_EQ("two", v5->value());
}

}  // namespace
