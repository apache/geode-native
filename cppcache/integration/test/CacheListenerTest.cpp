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
#include <gmock/gmock.h>

#include <future>
#include <iostream>
#include <random>
#include <thread>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/CacheListener.hpp>
#include <geode/EntryEvent.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionEvent.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "mock/CacheListenerMock.hpp"

namespace {
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}  // namespace

class CacheListenerTest : public ::testing::Test {
 protected:
  static std::unique_ptr<Cluster> cluster;

  static void SetUpTestSuite() {
    std::cout << "Setup Cluster.\n";
    cluster = make_unique<Cluster>(LocatorCount{1}, ServerCount{1});
    std::cout << "Start Cluster.\n";

    cluster->start();

    std::cout << "Create Region.\n";
    cluster->getGfsh()
        .create()
        .region()
        .withName("region")
        .withType("PARTITION")
        .execute();
    std::cout << "Setup complete.\n";
  }

  static void TearDownTestSuite() {
    cluster->stop();
    cluster.reset();
  }

  CacheListenerTest() = default;
  ~CacheListenerTest() = default;
};

std::unique_ptr<Cluster> CacheListenerTest::cluster;

using ::testing::AllOf;
using ::testing::Eq;
using ::testing::IsNull;
using ::testing::Property;
using ::testing::_; //Match all

TEST_F(CacheListenerTest, afterCreate) {
  using apache::geode::client::EntryEvent;
  using apache::geode::client::Nice_MockListener;
  using apache::geode::client::RegionShortcut;

  auto cache = cluster->createCache({}, true);

  auto listener = std::make_shared<Nice_MockListener>();

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .setCacheListener(listener)
                    .create("region");

  EXPECT_CALL(*listener, afterCreate)
     .With(AllOf(Property(&EntryEvent::getRegion, Eq(region)),
                        // Property(&EntryEvent::getKey, Eq("key")),
                        Property(&EntryEvent::getOldValue, IsNull()),
                        // Property(&EntryEvent::getNewValue, Eq("value")),
                        Property(&EntryEvent::getCallbackArgument, IsNull()),
                        Property(&EntryEvent::remoteOrigin, Eq(false)))))
      .Times(1);

  region->put("key", "value");
}

TEST_F(CacheListenerTest, afterUpdate) {}
TEST_F(CacheListenerTest, afterInvalidate) {}
TEST_F(CacheListenerTest, afterDestroy) {}
TEST_F(CacheListenerTest, afterRegionInvalidate) {}
TEST_F(CacheListenerTest, afterRegionDestroy) {}
TEST_F(CacheListenerTest, afterRegionClear) {}
TEST_F(CacheListenerTest, afterRegionLive) {}
TEST_F(CacheListenerTest, close) {}
TEST_F(CacheListenerTest, afterRegionDisconnected) {}