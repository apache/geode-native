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
MATCHER_P(CashableEq, value, "") {
  return arg->toString() == value->toString();
}

class LocalRegionCacheListenerTest : public ::testing::Test {
  /* It would be nice to create a listener instance per test case, and let it
   * verify itself at the end of the test. Unfortunately, listeners - reference
   * counted by their regions, themselves reference count their regions, so you
   * get a circular dependency preventing it from falling out of scope and
   * allowing the mock to do it's thing. Therefore, you must verify and clear
   * each mock manually at the end of the test, or you'll get a false positive
   * fail condition and memory leak. */
 protected:
  static ::std::shared_ptr<::apache::geode::client::Nice_MockListener>
      listener_;

 public:
  static void SetUpTestSuite() {
    listener_ =
        ::std::make_shared<::apache::geode::client::Nice_MockListener>();
  }

  static void TearDownTestSuite() { listener_.reset(); }

 protected:
  ::apache::geode::client::Cache cache_;
  ::std::shared_ptr<::apache::geode::client::Region> region_;

  LocalRegionCacheListenerTest()
      : cache_{::apache::geode::client::CacheFactory()
                   .set("log-level", "none")
                   .set("statistic-sampling-enabled", "false")
                   .create()} {
    region_ =
        cache_
            .createRegionFactory(::apache::geode::client::RegionShortcut::LOCAL)
            .setCacheListener(listener_)
            .create("region");
  }

  void TearDown() {
    ::testing::Mock::VerifyAndClearExpectations(
        listener_.get());  // Just to make sure it's clean for the next test.
  };

  ~LocalRegionCacheListenerTest() = default;
};

::std::shared_ptr<::apache::geode::client::Nice_MockListener>
    LocalRegionCacheListenerTest::listener_{};
}  // namespace

using ::testing::_;  // Match all
using ::testing::AllOf;
using ::testing::Eq;
using ::testing::IsNull;
using ::testing::Property;

using ::apache::geode::client::CacheableString;
using ::apache::geode::client::EntryEvent;
using ::apache::geode::client::RegionEvent;

TEST_F(LocalRegionCacheListenerTest, afterCreate) {
  auto key = std::make_shared<CacheableString>("key");
  auto value = std::make_shared<CacheableString>("value");

  EXPECT_CALL(
      *listener_,
      afterCreate(AllOf(Property(&EntryEvent::getRegion, Eq(region_)),
                        Property(&EntryEvent::getKey, CashableEq(key)),
                        Property(&EntryEvent::getNewValue, CashableEq(value)))))
      .Times(1);

  region_->put(key, value);

  ::testing::Mock::VerifyAndClearExpectations(listener_.get());
}

TEST_F(LocalRegionCacheListenerTest, afterUpdate) {
  auto key = std::make_shared<CacheableString>("key");
  auto value_1 = std::make_shared<CacheableString>("value_1");
  auto value_2 = std::make_shared<CacheableString>("value_2");

  EXPECT_CALL(*listener_,
              afterUpdate(AllOf(
                  Property(&EntryEvent::getRegion, Eq(region_)),
                  Property(&EntryEvent::getKey, CashableEq(key)),
                  Property(&EntryEvent::getNewValue, CashableEq(value_2)),
                  Property(&EntryEvent::getOldValue, CashableEq(value_1)))))
      .Times(1);

  region_->put(key, value_1);
  region_->put(key, value_2);

  ::testing::Mock::VerifyAndClearExpectations(listener_.get());
}

TEST_F(LocalRegionCacheListenerTest, afterInvalidate) {
  auto key = std::make_shared<CacheableString>("key");
  auto value = std::make_shared<CacheableString>("value");

  EXPECT_CALL(*listener_, afterInvalidate(AllOf(
                              Property(&EntryEvent::getRegion, Eq(region_)),
                              Property(&EntryEvent::getKey, CashableEq(key)))))
      .Times(1);

  region_->put(key, value);
  region_->invalidate(key);

  ::testing::Mock::VerifyAndClearExpectations(listener_.get());
}

TEST_F(LocalRegionCacheListenerTest, afterDestroy) {
  auto key = std::make_shared<CacheableString>("key");
  auto value = std::make_shared<CacheableString>("value");

  EXPECT_CALL(*listener_, afterDestroy(AllOf(
                              Property(&EntryEvent::getRegion, Eq(region_)),
                              Property(&EntryEvent::getKey, CashableEq(key)))))
      .Times(1);

  region_->put(key, value);
  region_->destroy(key);

  ::testing::Mock::VerifyAndClearExpectations(listener_.get());
}

TEST_F(LocalRegionCacheListenerTest, afterRegionInvalidate) {}

TEST_F(LocalRegionCacheListenerTest, afterRegionDestroy) {
  EXPECT_CALL(*listener_, afterRegionDestroy(
                              Property(&RegionEvent::getRegion, Eq(region_))))
      .Times(1);

  cache_.close();

  ::testing::Mock::VerifyAndClearExpectations(listener_.get());
}

TEST_F(LocalRegionCacheListenerTest, afterRegionClear) {}

TEST_F(LocalRegionCacheListenerTest, DISABLED_afterRegionLive) {
  FAIL() << "This scenario is impossible.";
}

TEST_F(LocalRegionCacheListenerTest, close) {}

TEST_F(LocalRegionCacheListenerTest, afterRegionDisconnected) {}
