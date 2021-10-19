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
#include <gmock/gmock.h>

#include <initializer_list>
#include <memory>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/EntryEvent.hpp>
#include <geode/PdxWrapper.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionEvent.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/TypeRegistry.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "Order.hpp"
#include "mock/CacheListenerMock.hpp"

ACTION_P(CvNotifyOne, cv) { cv->notify_one(); }

namespace {

using apache::geode::client::Cache;
using apache::geode::client::Cacheable;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheListenerMock;
using apache::geode::client::CacheRegionHelper;
using apache::geode::client::EntryEvent;
using apache::geode::client::PdxReader;
using apache::geode::client::PdxSerializable;
using apache::geode::client::PdxSerializer;
using apache::geode::client::PdxWrapper;
using apache::geode::client::PdxWriter;
using apache::geode::client::Region;
using apache::geode::client::RegionEvent;
using apache::geode::client::RegionShortcut;
using apache::geode::client::UserObjectSizer;

using ::testing::_;
using ::testing::Return;

using PdxTests::Order;

Cache createTestCache() {
  CacheFactory cacheFactory;
  return cacheFactory.set("log-level", "none")
      .set("on-client-disconnect-clear-pdxType-Ids", "true")
      .set("statistic-sampling-enabled", "false")
      .create();
}

TEST(PdxSerializerTest, canSerializeNonWhileClusterRestarting) {
  std::mutex mutex_shutdown;
  std::condition_variable cv_shutdown;

  std::mutex mutex_live;
  std::condition_variable cv_live;

  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.start();

  auto& gfsh = cluster.getGfsh();
  gfsh.create().region().withName("region").withType("REPLICATE").execute();

  auto cache = createTestCache();
  auto cacheImpl = CacheRegionHelper::getCacheImpl(&cache);

  auto listener = std::make_shared<CacheListenerMock>();
  EXPECT_CALL(*listener, afterCreate(_)).WillRepeatedly(Return());
  EXPECT_CALL(*listener, afterRegionDestroy(_)).WillRepeatedly(Return());
  EXPECT_CALL(*listener, close(_)).WillRepeatedly(Return());
  EXPECT_CALL(*listener, afterRegionLive(_))
      .WillRepeatedly(CvNotifyOne(&cv_live));
  EXPECT_CALL(*listener, afterRegionDisconnected(_))
      .WillRepeatedly(CvNotifyOne(&cv_shutdown));

  {
    auto poolFactory = cache.getPoolManager()
                           .createFactory()
                           .setReadTimeout(std::chrono::seconds{1})
                           .setPingInterval(std::chrono::seconds{1})
                           .setSubscriptionEnabled(true);
    cluster.applyLocators(poolFactory);
    poolFactory.create("default");
  }

  auto region = cache.createRegionFactory(RegionShortcut::CACHING_PROXY)
                    .setPoolName("default")
                    .setCacheListener(listener)
                    .create("region");

  cache.getTypeRegistry().registerPdxType(Order::createDeserializable);

  // Create local PDX type in the registry
  region->put("2", std::make_shared<Order>(2, "product x", 37));

  auto& m = cacheImpl->getPdxSerializationTestMutex();

  m.lock();
  auto thread = std::thread([&region] {
    region->put("3", std::make_shared<Order>(3, "product y", 37));
  });

  std::this_thread::sleep_for(std::chrono::seconds{5});
  gfsh.shutdown().execute();

  {
    std::unique_lock<std::mutex> lock(mutex_shutdown);
    cv_shutdown.wait(lock);
  }

  std::this_thread::sleep_for(std::chrono::seconds{5});
  for (auto& server : cluster.getServers()) {
    server.start();
  }

  {
    std::unique_lock<std::mutex> lock(mutex_live);
    cv_live.wait(lock);
  }

  m.unlock();
  thread.join();
}

}  // namespace
