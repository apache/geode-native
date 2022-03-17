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

#include <gtest/gtest.h>

#include <geode/CacheTransactionManager.hpp>
#include <geode/EntryEvent.hpp>
#include <geode/RegionEvent.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "framework/Cluster.h"
#include "gmock_actions.hpp"
#include "internal/concurrent/binary_semaphore.hpp"
#include "mock/CacheListenerMock.hpp"

namespace {

using apache::geode::client::binary_semaphore;

using apache::geode::client::Cache;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheListener;
using apache::geode::client::CacheListenerMock;
using apache::geode::client::Exception;

using ::testing::_;
using ::testing::DoAll;
using ::testing::NiceMock;

Cache createTestCache() {
  CacheFactory cacheFactory;
  return cacheFactory.set("log-level", "none")
      .set("statistic-sampling-enabled", "false")
      .create();
}

void createTestPool(Cache& cache, Cluster& cluster) {
  auto poolFactory =
      cache.getPoolManager().createFactory().setSubscriptionEnabled(true);
  cluster.applyLocators(poolFactory);
  poolFactory.create("default");
}

std::shared_ptr<apache::geode::client::Region> setupRegion(
    Cache& cache, const std::shared_ptr<CacheListener>& listener) {
  auto region = cache
                    .createRegionFactory(
                        apache::geode::client::RegionShortcut::CACHING_PROXY)
                    .setPoolName("default")
                    .setCacheListener(listener)
                    .setConcurrencyChecksEnabled(false)
                    .create("region");
  return region;
}

TEST(TransactionCleaningTest, txWithStoppedServer) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  auto cache = createTestCache();
  createTestPool(cache, cluster);

  binary_semaphore live_sem{0};
  binary_semaphore shut_sem{1};
  auto listener = std::make_shared<NiceMock<CacheListenerMock>>();
  EXPECT_CALL(*listener, afterRegionLive(_))
      .WillRepeatedly(DoAll(ReleaseSem(&live_sem), AcquireSem(&shut_sem)));
  EXPECT_CALL(*listener, afterRegionDisconnected(_))
      .WillRepeatedly(DoAll(ReleaseSem(&shut_sem), AcquireSem(&live_sem)));
  auto region = setupRegion(cache, listener);

  cache.getCacheTransactionManager()->begin();
  region->put("one", "one");
  cache.getCacheTransactionManager()->commit();

  cluster.getServers()[0].stop();
  shut_sem.acquire();
  shut_sem.release();

  cache.getCacheTransactionManager()->begin();
  EXPECT_THROW(region->put("one", "two"), Exception);

  cache.getCacheTransactionManager()->rollback();

  cluster.getServers()[0].start();
  live_sem.acquire();
  live_sem.release();

  cache.getCacheTransactionManager()->begin();
  region->put("one", "three");
  cache.getCacheTransactionManager()->commit();
}

}  // namespace
