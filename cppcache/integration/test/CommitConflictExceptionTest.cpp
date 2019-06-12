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

#include <gmock/gmock.h>

#include <future>
#include <thread>

#include <gtest/gtest.h>

#include <geode/CacheTransactionManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "framework/Cluster.h"

namespace {

std::shared_ptr<apache::geode::client::Region> setupRegion(
    apache::geode::client::Cache& cache) {
  auto region =
      cache.createRegionFactory(apache::geode::client::RegionShortcut::PROXY)
          .setPoolName("default")
          .create("region");
  return region;
}

TEST(CommitConflictExceptionTest, putPartitionTx) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.start();
  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  std::promise<void> begunTx1;
  std::promise<void> putTx1;
  std::promise<void> putTx2;
  std::promise<void> committedTx2;

  auto task1 = std::async(std::launch::async, [&cluster, &begunTx1, &putTx1,
                                               &putTx2, &committedTx2] {
    SCOPED_TRACE("task1");
    auto cache = cluster.createCache();
    auto region = setupRegion(cache);

    cache.getCacheTransactionManager()->begin();
    begunTx1.set_value();
    region->put("one", "one");
    ASSERT_EQ(
        std::future_status::ready,
        putTx2.get_future().wait_for(debug_safe(std::chrono::minutes(1))));
    putTx1.set_value();
    ASSERT_EQ(std::future_status::ready,
              committedTx2.get_future().wait_for(
                  debug_safe(std::chrono::minutes(1))));
    try {
      cache.getCacheTransactionManager()->commit();
      FAIL();
    } catch (apache::geode::client::Exception& ex) {
      using ::testing::StartsWith;
      EXPECT_THAT(ex.what(),
                  StartsWith("org.apache.geode.cache.CommitConflictException"));
    }

    cache.getCacheTransactionManager()->begin();
    region->put("one", "one");
    ASSERT_NO_THROW(cache.getCacheTransactionManager()->commit());
  });

  auto task2 = std::async(std::launch::async, [&cluster, &begunTx1, &putTx1,
                                               &putTx2, &committedTx2] {
    SCOPED_TRACE("task2");
    auto cache = cluster.createCache();
    auto region = setupRegion(cache);

    cache.getCacheTransactionManager()->begin();
    ASSERT_EQ(
        std::future_status::ready,
        begunTx1.get_future().wait_for(debug_safe(std::chrono::minutes(1))));
    region->put("one", "two");
    putTx2.set_value();
    ASSERT_EQ(
        std::future_status::ready,
        putTx1.get_future().wait_for(debug_safe(std::chrono::minutes(1))));
    cache.getCacheTransactionManager()->commit();
    committedTx2.set_value();
  });

  ASSERT_EQ(std::future_status::ready,
            task1.wait_for(debug_safe(std::chrono::minutes(1))));
  ASSERT_EQ(std::future_status::ready,
            task2.wait_for(debug_safe(std::chrono::minutes(1))));
}

}  // namespace
