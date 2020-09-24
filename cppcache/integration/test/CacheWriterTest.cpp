/* Licensed to the Apache Software Foundation (ASF) under one or more *
 * contributor license agreements.  See the NOTICE file distributed with this
 * work for additional information regarding copyright ownership. The ASF
 * licenses this file to You under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/CacheListener.hpp>
#include <geode/CacheTransactionManager.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "framework/Cluster.h"
#include "framework/Framework.h"
#include "framework/Gfsh.h"

namespace {

using apache::geode::client::Cache;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheTransactionManager;
using apache::geode::client::CacheWriter;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

/*
 * Verify that it is possible to write in region from CacheWriter listener that
 * is triggered by transaction. Region is not defined on client, but only on a
 * server.
 */
TEST(CacheWriterTest, WriteInRegionFromCacheWriterTriggeredByTransaction) {
  Cluster cluster{
      LocatorCount{1}, ServerCount{1},
      CacheXMLFiles(
          {std::string(getFrameworkString(FrameworkVariable::TestCacheXmlDir)) +
               "/cacheWriterTransaction1.xml",
           std::string(getFrameworkString(FrameworkVariable::TestCacheXmlDir)) +
               "/cacheWriterTransaction1.xml"})};
  cluster.start([&]() {
    cluster.getGfsh()
        .deploy()
        .jar(getFrameworkString(FrameworkVariable::JavaObjectJarPath))
        .execute();
  });

  CacheFactory cacheFactory;
  auto cache = cacheFactory.set("log-level", "none")
                   .set("statistic-sampling-enabled", "false")
                   .create();

  auto poolFactory = cache.getPoolManager().createFactory();
  cluster.applyLocators(poolFactory);

  auto pool = poolFactory.create("pool");
  auto regionFactory = cache.createRegionFactory(RegionShortcut::CACHING_PROXY);
  auto region = regionFactory.setPoolName("pool").create("partition_region");

  auto transactionManager = cache.getCacheTransactionManager();
  transactionManager->begin();
  auto key = CacheableString::create("key1");
  auto value = CacheableString::create("value1");
  region->put(key, value);
  transactionManager->commit();

  auto v1 = std::dynamic_pointer_cast<CacheableString>(region->get("key1"));
  EXPECT_EQ("value1", v1->value());

  // check that CacheWriter has successfully put data in shadow_region
  auto shadow_region =
      regionFactory.setPoolName("pool").create("shadow_region");

  auto v2 = std::dynamic_pointer_cast<CacheableString>(region->get("key1"));
  EXPECT_EQ("value1", v2->value());
}
}  // namespace
