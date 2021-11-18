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

#include <chrono>
#include <thread>

#include <boost/thread/latch.hpp>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/EntryEvent.hpp>
#include <geode/RegionEvent.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "framework/Cluster.h"
#include "framework/Framework.h"
#include "framework/Gfsh.h"
#include "gmock_actions.hpp"
#include "mock/CacheListenerMock.hpp"
#include "util/concurrent/binary_semaphore.hpp"

namespace {

using apache::geode::client::binary_semaphore;
using apache::geode::client::Cache;
using apache::geode::client::CacheableInt16;
using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheListener;
using apache::geode::client::CacheListenerMock;
using apache::geode::client::EntryEvent;
using apache::geode::client::IllegalStateException;
using apache::geode::client::Region;
using apache::geode::client::RegionEvent;
using apache::geode::client::RegionShortcut;

using ::testing::_;
using ::testing::DoAll;
using ::testing::InvokeWithoutArgs;
using ::testing::Return;

constexpr auto kNumKeys = 100;

class MyCacheListener : public CacheListener {
  boost::latch& allKeysUpdatedLatch_;
  boost::latch& allKeysInvalidLatch_;

 public:
  MyCacheListener(boost::latch& allKeysUpdatedLatch,
                  boost::latch& allKeysInvalidLatch)
      : allKeysUpdatedLatch_(allKeysUpdatedLatch),
        allKeysInvalidLatch_(allKeysInvalidLatch) {}

  void afterUpdate(const EntryEvent&) override {
    allKeysUpdatedLatch_.count_down();
  }

  void afterInvalidate(const EntryEvent&) override {
    allKeysInvalidLatch_.count_down();
  }

  void reset() {}
};

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

  binary_semaphore sem{0};
  auto listener = std::make_shared<CacheListenerMock>();

  EXPECT_CALL(*listener, afterCreate(_)).WillRepeatedly(Return());
  EXPECT_CALL(*listener, afterRegionLive(_)).WillRepeatedly(Return());
  EXPECT_CALL(*listener, afterRegionDisconnected(_)).WillRepeatedly(Return());
  EXPECT_CALL(*listener, afterDestroy(_)).Times(1).WillOnce(ReleaseSem(&sem));

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

  EXPECT_TRUE(sem.try_acquire_for(std::chrono::minutes{1}));
}

TEST(RegisterKeysTest, RegisterAnyAndClusterRestart) {
  auto N = 100U;
  boost::latch create_latch{N};
  binary_semaphore live_sem{0};
  binary_semaphore shut_sem{1};

  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.start();

  auto& gfsh = cluster.getGfsh();
  gfsh.create().region().withName("region").withType("REPLICATE").execute();

  auto cache = createTestCache();
  {
    auto poolFactory =
        cache.getPoolManager().createFactory().setSubscriptionEnabled(true);
    cluster.applyLocators(poolFactory);
    poolFactory.create("default");
  }

  auto listener = std::make_shared<CacheListenerMock>();
  EXPECT_CALL(*listener, afterRegionLive(_))
      .WillRepeatedly(DoAll(ReleaseSem(&live_sem), AcquireSem(&shut_sem)));
  EXPECT_CALL(*listener, afterRegionDisconnected(_))
      .WillRepeatedly(DoAll(ReleaseSem(&shut_sem), AcquireSem(&live_sem)));
  EXPECT_CALL(*listener, afterCreate(_))
      .Times(N)
      .WillRepeatedly(CountDownLatch(&create_latch));

  auto region = cache.createRegionFactory(RegionShortcut::CACHING_PROXY)
                    .setPoolName("default")
                    .setCacheListener(listener)
                    .create("region");
  region->registerAllKeys(false, true);
  EXPECT_EQ(region->keys().size(), 0);

  auto producer = std::thread([&region, N] {
    for (auto i = 0U; i < N;) {
      auto key = "entry-" + std::to_string(i++);
      auto value = "{\"entryName\": \"" + key + "\"}";
      region->put(key, value);
    }
  });

  create_latch.wait();

  producer.join();
  gfsh.shutdown().execute();

  shut_sem.acquire();
  shut_sem.release();

  for (auto& server : cluster.getServers()) {
    server.start();
  }

  live_sem.acquire();
  live_sem.release();
  EXPECT_EQ(region->keys().size(), 0);
}

TEST(RegisterKeysTest, RegisterRegexAndClusterRestart) {
  auto N_1 = 10U;
  auto N_2 = 90U;
  auto N = N_1 + N_2;
  binary_semaphore live_sem{0};
  binary_semaphore shut_sem{1};
  boost::latch create_latch{N};

  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.start();

  auto& gfsh = cluster.getGfsh();
  gfsh.create().region().withName("region").withType("REPLICATE").execute();

  auto cache = createTestCache();
  {
    auto poolFactory =
        cache.getPoolManager().createFactory().setSubscriptionEnabled(true);
    cluster.applyLocators(poolFactory);
    poolFactory.create("default");
  }

  auto listener = std::make_shared<CacheListenerMock>();
  EXPECT_CALL(*listener, afterRegionLive(_))
      .WillRepeatedly(DoAll(ReleaseSem(&live_sem), AcquireSem(&shut_sem)));
  EXPECT_CALL(*listener, afterRegionDisconnected(_))
      .WillRepeatedly(DoAll(ReleaseSem(&shut_sem), AcquireSem(&live_sem)));
  EXPECT_CALL(*listener, afterCreate(_))
      .Times(N)
      .WillRepeatedly(CountDownLatch(&create_latch));

  auto region = cache.createRegionFactory(RegionShortcut::CACHING_PROXY)
                    .setPoolName("default")
                    .setCacheListener(listener)
                    .create("region");
  region->registerRegex("interest-.*", false, true);
  EXPECT_EQ(region->keys().size(), 0);

  auto producer_non_interest = std::thread([&region, N_1] {
    for (auto i = 0U; i < N_1;) {
      auto key = "entry-" + std::to_string(i++);
      auto value = "{\"entryName\": \"" + key + "\"}";
      region->put(key, value);
    }
  });

  auto producer_interest = std::thread([&region, N_2] {
    for (auto i = 0U; i < N_2;) {
      auto key = "interest-" + std::to_string(i++);
      auto value = "{\"entryName\": \"" + key + "\"}";
      region->put(key, value);
    }
  });

  create_latch.wait();

  producer_non_interest.join();
  producer_interest.join();

  gfsh.shutdown().execute();

  shut_sem.acquire();
  shut_sem.release();

  for (auto& server : cluster.getServers()) {
    server.start();
  }

  live_sem.acquire();
  live_sem.release();
  EXPECT_EQ(region->keys().size(), N_1);
}

TEST(RegisterKeysTest, RegisterKeySetAndClusterRestart) {
  auto N_1 = 10U;
  std::vector<std::shared_ptr<CacheableKey>> interest_keys{
      CacheableKey::create("dolores-1"),
      CacheableKey::create("maeve-1"),
      CacheableKey::create("bernard-2"),
      CacheableKey::create("theodore-3"),
      CacheableKey::create("william-5"),
      CacheableKey::create("clementine-8"),
      CacheableKey::create("abernathy-13"),
      CacheableKey::create("ford-21"),
  };

  auto N = N_1 + interest_keys.size();
  binary_semaphore live_sem{0};
  binary_semaphore shut_sem{1};
  boost::latch create_latch{N};
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.start();

  auto& gfsh = cluster.getGfsh();
  gfsh.create().region().withName("region").withType("REPLICATE").execute();

  auto cache = createTestCache();
  {
    auto poolFactory =
        cache.getPoolManager().createFactory().setSubscriptionEnabled(true);
    cluster.applyLocators(poolFactory);
    poolFactory.create("default");
  }

  auto listener = std::make_shared<CacheListenerMock>();
  EXPECT_CALL(*listener, afterRegionLive(_))
      .WillRepeatedly(DoAll(ReleaseSem(&live_sem), AcquireSem(&shut_sem)));
  EXPECT_CALL(*listener, afterRegionDisconnected(_))
      .WillRepeatedly(DoAll(ReleaseSem(&shut_sem), AcquireSem(&live_sem)));
  EXPECT_CALL(*listener, afterCreate(_))
      .Times(N)
      .WillRepeatedly(CountDownLatch(&create_latch));

  auto region = cache.createRegionFactory(RegionShortcut::CACHING_PROXY)
                    .setPoolName("default")
                    .setCacheListener(listener)
                    .create("region");

  region->registerKeys(interest_keys, false, true);
  EXPECT_EQ(region->keys().size(), 0);

  auto producer_non_interest = std::thread([&region, N_1] {
    for (auto i = 0U; i < N_1;) {
      auto key = "entry-" + std::to_string(i++);
      auto value = "{\"entryName\": \"" + key + "\"}";
      region->put(key, value);
    }
  });

  auto producer_interest = std::thread([&region, &interest_keys] {
    for (auto key : interest_keys) {
      auto value = "{\"entryName\": \"" + key->toString() + "\"}";
      region->put(key, value);
    }
  });

  create_latch.wait();

  producer_non_interest.join();
  producer_interest.join();

  gfsh.shutdown().execute();

  shut_sem.acquire();
  shut_sem.release();

  for (auto& server : cluster.getServers()) {
    server.start();
  }

  live_sem.acquire();
  live_sem.release();
  EXPECT_EQ(region->keys().size(), N_1);
}

TEST(RegisterKeysTest, RegisterKeySetAndDestroyClusterRestart) {
  auto N_1 = 10U;
  std::vector<std::shared_ptr<CacheableKey>> interest_keys{
      CacheableKey::create("dolores-1"),
      CacheableKey::create("maeve-1"),
      CacheableKey::create("bernard-2"),
      CacheableKey::create("theodore-3"),
      CacheableKey::create("william-5"),
      CacheableKey::create("clementine-8"),
      CacheableKey::create("abernathy-13"),
      CacheableKey::create("ford-21"),
  };

  auto N = N_1 + interest_keys.size();
  binary_semaphore live_sem{0};
  binary_semaphore shut_sem{1};
  boost::latch create_latch{N};
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.start();

  auto& gfsh = cluster.getGfsh();
  gfsh.create().region().withName("region").withType("REPLICATE").execute();

  auto cache = createTestCache();
  {
    auto poolFactory =
        cache.getPoolManager().createFactory().setSubscriptionEnabled(true);
    cluster.applyLocators(poolFactory);
    poolFactory.create("default");
  }

  auto listener = std::make_shared<CacheListenerMock>();
  EXPECT_CALL(*listener, afterRegionLive(_))
      .WillRepeatedly(DoAll(ReleaseSem(&live_sem), AcquireSem(&shut_sem)));
  EXPECT_CALL(*listener, afterRegionDisconnected(_))
      .WillRepeatedly(DoAll(ReleaseSem(&shut_sem), AcquireSem(&live_sem)));
  EXPECT_CALL(*listener, afterCreate(_))
      .Times(N)
      .WillRepeatedly(CountDownLatch(&create_latch));
  EXPECT_CALL(*listener, afterDestroy(_)).Times(1).WillOnce(Return());

  auto region = cache.createRegionFactory(RegionShortcut::CACHING_PROXY)
                    .setPoolName("default")
                    .setCacheListener(listener)
                    .create("region");

  region->registerKeys(interest_keys, false, true);
  EXPECT_EQ(region->keys().size(), 0);

  auto producer_non_interest = std::thread([&region, N_1] {
    for (auto i = 0U; i < N_1;) {
      auto key = "entry-" + std::to_string(i++);
      auto value = "{\"entryName\": \"" + key + "\"}";
      region->put(key, value);
    }
  });

  auto producer_interest = std::thread([&region, &interest_keys] {
    for (auto key : interest_keys) {
      auto value = "{\"entryName\": \"" + key->toString() + "\"}";
      region->put(key, value);
    }
  });

  create_latch.wait();

  producer_non_interest.join();
  producer_interest.join();

  region->remove(interest_keys[0]);
  gfsh.shutdown().execute();

  shut_sem.acquire();
  shut_sem.release();

  for (auto& server : cluster.getServers()) {
    server.start();
  }

  live_sem.acquire();
  live_sem.release();
  EXPECT_EQ(region->keys().size(), N_1);
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
    std::vector<std::shared_ptr<CacheableKey>> keys;
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
  std::vector<std::shared_ptr<CacheableKey>> keys;
  keys.push_back(std::make_shared<CacheableInt16>(2));

  EXPECT_THROW(region->registerKeys(keys, false, true), IllegalStateException);
  cache.close();
}

apache::geode::client::Cache createCache() {
  return apache::geode::client::CacheFactory()
      .set("log-level", "debug")
      .set("log-file", "c:/temp/RegisterKeysTest.log")
      .set("statistic-sampling-enabled", "false")
      .create();
}

std::shared_ptr<apache::geode::client::Pool> createPool(
    Cluster& cluster, apache::geode::client::Cache& cache) {
  auto poolFactory = cache.getPoolManager().createFactory();
  cluster.applyLocators(poolFactory);
  poolFactory.setSubscriptionEnabled(true);  // Per the customer.
  return poolFactory.create("default");
}

std::shared_ptr<apache::geode::client::Region> setupRegion(
    apache::geode::client::Cache& cache,
    const std::shared_ptr<apache::geode::client::Pool>& pool) {
  auto region =
      cache
          .createRegionFactory(apache::geode::client::RegionShortcut::
                                   CACHING_PROXY)  // Per the customer.
          .setPoolName(pool->getName())
          .create("region");

  return region;
}

TEST(RegisterKeysTest, DontReceiveValues) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  auto cache1 = createCache();
  auto pool1 = createPool(cluster, cache1);
  auto region1 = setupRegion(cache1, pool1);
  auto attrMutator = region1->getAttributesMutator();

  boost::latch allKeysUpdatedLatch{kNumKeys};
  boost::latch allKeysInvalidLatch{kNumKeys};
  auto listener = std::make_shared<MyCacheListener>(allKeysUpdatedLatch,
                                                    allKeysInvalidLatch);

  attrMutator->setCacheListener(listener);

  auto cache2 = createCache();
  auto pool2 = createPool(cluster, cache2);
  auto region2 = setupRegion(cache2, pool2);

  for (int i = 0; i < kNumKeys; i++) {
    region2->put(CacheableInt32::create(i), CacheableInt32::create(i));
  }

  region1->registerAllKeys(false, false, false);

  for (int i = 0; i < kNumKeys; i++) {
    auto hasKey = region1->containsKey(CacheableInt32::create(i));
    EXPECT_FALSE(hasKey);
  }

  for (int i = 0; i < kNumKeys; i++) {
    auto value = region1->get(CacheableInt32::create(i));
  }

  allKeysInvalidLatch.reset(kNumKeys);
  listener->reset();

  for (int i = 0; i < kNumKeys; i++) {
    region2->put(CacheableInt32::create(i), CacheableInt32::create(i + 1000));
  }

  allKeysInvalidLatch.wait();

  for (int i = 0; i < kNumKeys; i++) {
    auto hasKey = region1->containsKey(CacheableInt32::create(i));
    EXPECT_TRUE(hasKey);

    auto hasValue = region1->containsValueForKey(CacheableInt32::create(i));
    EXPECT_FALSE(hasValue);
  }
}

TEST(RegisterKeysTest, ReceiveValuesLocalInvalidate) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  auto cache1 = createCache();
  auto pool1 = createPool(cluster, cache1);
  auto region1 = setupRegion(cache1, pool1);
  auto attrMutator = region1->getAttributesMutator();

  boost::latch allKeysUpdatedLatch{kNumKeys};
  boost::latch allKeysInvalidLatch{kNumKeys};
  auto listener = std::make_shared<MyCacheListener>(allKeysUpdatedLatch,
                                                    allKeysInvalidLatch);
  attrMutator->setCacheListener(listener);

  auto cache2 = createCache();
  auto pool2 = createPool(cluster, cache2);
  auto region2 = setupRegion(cache2, pool2);

  for (int i = 0; i < kNumKeys; i++) {
    region2->put(CacheableInt32::create(i), CacheableInt32::create(i));
  }

  region1->registerAllKeys(false, true, true);

  for (int i = 0; i < kNumKeys; i++) {
    auto hasKey = region1->containsKey(CacheableInt32::create(i));
    EXPECT_TRUE(hasKey);

    auto hasValue = region1->containsValueForKey(CacheableInt32::create(i));
    EXPECT_TRUE(hasValue);
  }

  for (int i = 0; i < kNumKeys; i++) {
    region1->localInvalidate(CacheableInt32::create(i));
  }

  for (int i = 0; i < kNumKeys; i++) {
    auto hasKey = region1->containsKey(CacheableInt32::create(i));
    EXPECT_TRUE(hasKey);

    auto hasValue = region1->containsValueForKey(CacheableInt32::create(i));
    EXPECT_FALSE(hasValue);
  }

  allKeysUpdatedLatch.reset(kNumKeys);
  listener->reset();

  for (int i = 0; i < kNumKeys; i++) {
    region2->put(CacheableInt32::create(i), CacheableInt32::create(i + 2000));
  }

  allKeysUpdatedLatch.wait();

  for (int i = 0; i < kNumKeys; i++) {
    auto hasKey = region1->containsKey(CacheableInt32::create(i));
    EXPECT_TRUE(hasKey);

    auto hasValue = region1->containsValueForKey(CacheableInt32::create(i));
    EXPECT_TRUE(hasValue);
  }
}

TEST(RegisterKeysTest, ReceiveValues) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  auto cache1 = createCache();
  auto pool1 = createPool(cluster, cache1);
  auto region1 = setupRegion(cache1, pool1);
  auto attrMutator = region1->getAttributesMutator();

  boost::latch allKeysUpdatedLatch{kNumKeys};
  boost::latch allKeysInvalidLatch{kNumKeys};
  auto listener = std::make_shared<MyCacheListener>(allKeysUpdatedLatch,
                                                    allKeysInvalidLatch);
  attrMutator->setCacheListener(listener);

  auto cache2 = createCache();
  auto pool2 = createPool(cluster, cache2);
  auto region2 = setupRegion(cache2, pool2);

  for (int i = 0; i < kNumKeys; i++) {
    region2->put(CacheableInt32::create(i), CacheableInt32::create(i));
  }

  region1->registerAllKeys(false, false, true);

  for (int i = 0; i < kNumKeys; i++) {
    auto hasKey = region1->containsKey(CacheableInt32::create(i));
    EXPECT_FALSE(hasKey);

    auto hasValue = region1->containsValueForKey(CacheableInt32::create(i));
    EXPECT_FALSE(hasValue);
  }

  allKeysUpdatedLatch.reset(kNumKeys);
  listener->reset();

  for (int i = 0; i < kNumKeys; i++) {
    region2->put(CacheableInt32::create(i), CacheableInt32::create(i + 2000));
  }

  allKeysUpdatedLatch.wait();

  for (int i = 0; i < kNumKeys; i++) {
    auto hasKey = region1->containsKey(CacheableInt32::create(i));
    EXPECT_TRUE(hasKey);

    auto hasValue = region1->containsValueForKey(CacheableInt32::create(i));
    EXPECT_TRUE(hasValue);
  }
}

}  // namespace
