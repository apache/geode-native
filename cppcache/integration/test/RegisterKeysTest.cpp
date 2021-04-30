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

#include <gmock/gmock.h>

#include <condition_variable>
#include <mutex>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/EntryEvent.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "framework/Cluster.h"
#include "framework/Framework.h"
#include "framework/Gfsh.h"

class CacheListenerMock : public apache::geode::client::CacheListener {
 public:
  MOCK_METHOD1(afterDestroy,
               void(const apache::geode::client::EntryEvent& event));
};

namespace {

using apache::geode::client::Cache;
using apache::geode::client::CacheableInt16;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::IllegalStateException;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;
using ::testing::_;
using ::testing::DoAll;
using ::testing::InvokeWithoutArgs;

ACTION_P(CvNotifyOne, cv) { cv->notify_one(); }

Cache createTestCache() {
  CacheFactory cacheFactory;
  return cacheFactory.set("log-level", "none")
      .set("statistic-sampling-enabled", "false")
      .create();
}

std::shared_ptr<Region> setupCachingProxyRegion(Cache& cache,
                                                bool consistency = true) {
  auto region = cache.createRegionFactory(RegionShortcut::CACHING_PROXY)
                    .setPoolName("default")
                    .setConcurrencyChecksEnabled(consistency)
                    .create("region");

  return region;
}

std::shared_ptr<Region> setupProxyRegion(Cache& cache) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .create("region");

  return region;
}

TEST(RegisterKeysTest, RegisterAllWithCachingRegion) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  {
    auto cache = createTestCache();
    auto poolFactory =
        cache.getPoolManager().createFactory().setSubscriptionEnabled(true);
    cluster.applyLocators(poolFactory);
    poolFactory.create("default");
    auto region = setupCachingProxyRegion(cache);

    region->put("one", std::make_shared<CacheableInt16>(1));
    region->put("two", std::make_shared<CacheableInt16>(2));
    region->put("three", std::make_shared<CacheableInt16>(3));
  }

  {
    auto cache2 = createTestCache();
    auto poolFactory =
        cache2.getPoolManager().createFactory().setSubscriptionEnabled(true);
    cluster.applyLocators(poolFactory);
    poolFactory.create("default");
    auto region2 = setupCachingProxyRegion(cache2);

    auto&& entryBefore = region2->getEntry("one");
    ASSERT_EQ(entryBefore, nullptr);

    // 2nd parameter is getInitialValues, default is false, which leads to the
    // first update notification being passed in with a NULL "oldValue".  Set it
    // to true here, and verify the entry retrieved is valid, i.e. the initial
    // value was retrieved.
    region2->registerAllKeys(false, true);

    auto&& uncastedEntry = region2->getEntry("one");
    auto&& entryAfter =
        std::dynamic_pointer_cast<CacheableInt16>(uncastedEntry->getValue());
    ASSERT_NE(entryAfter, nullptr);
    ASSERT_EQ(entryAfter->value(), 1);
  }
}

TEST(RegisterKeysTest, RegisterAllWithConsistencyDisabled) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  auto producer_cache = createTestCache();
  auto listener_cache = createTestCache();
  std::shared_ptr<Region> producer_region;
  std::shared_ptr<Region> listener_region;

  {
    auto poolFactory = producer_cache.getPoolManager().createFactory();
    cluster.applyLocators(poolFactory);
    poolFactory.create("default");
    producer_region = setupProxyRegion(producer_cache);
  }

  std::mutex cv_mutex;
  bool destroyed = false;
  std::condition_variable cv;
  auto listener = std::make_shared<CacheListenerMock>();
  EXPECT_CALL(*listener, afterDestroy(_))
      .Times(1)
      .WillOnce(DoAll(InvokeWithoutArgs([&destroyed] { destroyed = true; }),
                      CvNotifyOne(&cv)));

  {
    auto poolFactory =
        listener_cache.getPoolManager().createFactory().setSubscriptionEnabled(
            true);
    cluster.applyLocators(poolFactory);
    poolFactory.create("default");
    listener_region =
        listener_cache.createRegionFactory(RegionShortcut::CACHING_PROXY)
            .setPoolName("default")
            .setCacheListener(listener)
            .setConcurrencyChecksEnabled(false)
            .create("region");
    listener_region->registerAllKeys();
  }

  producer_region->put("one", std::make_shared<CacheableInt16>(1));
  producer_region->destroy("one");

  {
    std::unique_lock<std::mutex> lock(cv_mutex);
    EXPECT_TRUE(cv.wait_for(lock, std::chrono::minutes(1),
                            [&destroyed] { return destroyed; }));
  }
}

TEST(RegisterKeysTest, RegisterAnyWithCachingRegion) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  {
    auto cache = createTestCache();
    auto poolFactory =
        cache.getPoolManager().createFactory().setSubscriptionEnabled(true);
    cluster.applyLocators(poolFactory);
    poolFactory.create("default");
    auto region = setupCachingProxyRegion(cache);

    region->put("one", std::make_shared<CacheableInt16>(1));
    region->put("two", std::make_shared<CacheableInt16>(2));
    region->put("three", std::make_shared<CacheableInt16>(3));

    cache.close();
  }

  {
    auto cache2 = createTestCache();
    auto poolFactory =
        cache2.getPoolManager().createFactory().setSubscriptionEnabled(true);
    cluster.applyLocators(poolFactory);
    poolFactory.create("default");
    auto region2 = setupCachingProxyRegion(cache2);
    std::vector<std::shared_ptr<CacheableKey> > keys;
    keys.push_back(std::make_shared<CacheableString>("one"));

    auto&& entryBefore = region2->getEntry("one");
    ASSERT_EQ(entryBefore, nullptr);

    region2->registerKeys(keys, false, true);

    auto&& entryAfterGet = std::dynamic_pointer_cast<CacheableInt16>(
        region2->getEntry("one")->getValue());
    ASSERT_NE(entryAfterGet, nullptr);
    ASSERT_EQ(entryAfterGet->value(), 1);
  }
}

TEST(RegisterKeysTest, RegisterAllWithProxyRegion) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();
  auto cache = createTestCache();
  auto poolFactory =
      cache.getPoolManager().createFactory().setSubscriptionEnabled(true);
  cluster.applyLocators(poolFactory);
  poolFactory.create("default");
  auto region = setupProxyRegion(cache);

  EXPECT_THROW(region->registerAllKeys(false, true), IllegalStateException);
  cache.close();
}

TEST(RegisterKeysTest, RegisterAnyWithProxyRegion) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();
  auto cache = createTestCache();
  auto poolFactory =
      cache.getPoolManager().createFactory().setSubscriptionEnabled(true);
  cluster.applyLocators(poolFactory);
  poolFactory.create("default");
  auto region = setupProxyRegion(cache);
  std::vector<std::shared_ptr<CacheableKey> > keys;
  keys.push_back(std::make_shared<CacheableInt16>(2));

  EXPECT_THROW(region->registerKeys(keys, false, true), IllegalStateException);
  cache.close();
}

}  // namespace
