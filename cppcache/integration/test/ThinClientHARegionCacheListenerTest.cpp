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

#include <boost/log/core.hpp>

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
::std::unique_ptr<T> make_unique(Args&&... args) {
  return ::std::unique_ptr<T>(new T(::std::forward<Args>(args)...));
}

MATCHER_P(CashableEq, value, "") {
  return arg->toString() == value->toString();
}

class ThinClientHARegionCacheListenerTest : public ::testing::Test {
  /* It would be nice to create a listener instance per test case, and let it
   * verify itself at the end of the test. Unfortunately, listeners - reference
   * counted by their regions, themselves reference count their regions, so you
   * get a circular dependency preventing it from falling out of scope and
   * allowing the mock to do it's thing. Therefore, you must verify and clear
   * each mock manually at the end of the test, or you'll get a false positive
   * fail condition and memory leak. */
 protected:
  static ::std::unique_ptr<::Cluster> cluster_;
  static ::std::shared_ptr<::apache::geode::client::Nice_MockListener>
      listener_;
  static ::std::unique_ptr<::apache::geode::client::Cache> cache_;
  static ::std::shared_ptr<::apache::geode::client::Region> region_;

 public:
  static void SetUpTestSuite() {
    ::boost::log::core::get()->set_logging_enabled(false);
    listener_ =
        ::std::make_shared<::apache::geode::client::Nice_MockListener>();

    cluster_ =
        make_unique<::Cluster>(Name{"ThinClientHARegionCacheListenerTest"},
                               LocatorCount{1}, ServerCount{1});

    cluster_->start();

    cache_ = make_unique<::apache::geode::client::Cache>(
        cluster_->createCache({}, ::Cluster::Subscription_State::Enabled));

    region_ = cache_
                  ->createRegionFactory(
                      ::apache::geode::client::RegionShortcut::PROXY)
                  .setPoolName("default")
                  .setCacheListener(listener_)
                  .create("region");
  }

  static void TearDownTestSuite() {
    region_.reset();  // Be sure to reset the region before the cache.
    cache_.reset();   // Be sure to reset the cache before the cluster.
    cluster_.reset();
    listener_.reset();
    ::boost::log::core::get()->set_logging_enabled();
  }

 protected:
  void TearDown() {
    ::testing::Mock::VerifyAndClearExpectations(
        listener_.get());  // Just to make sure it's clean for the next test.

    cluster_->getGfsh()
        .destroy()
        .region()
        .ifExists()
        .withName("region")
        .execute();
  }

  ~ThinClientHARegionCacheListenerTest() = default;

  void cycle_server_EP_connection() {
    cluster_->getServers()[0].stop();
    std::this_thread::sleep_for(std::chrono::seconds{
        1});  // We have a timing bug, where for now, we're forced to wait so
              // whatever the problem it, it resolves itself.
    cluster_->getServers()[0].start();
  }
};

::std::unique_ptr<::Cluster> ThinClientHARegionCacheListenerTest::cluster_{};

::std::shared_ptr<::apache::geode::client::Nice_MockListener>
    ThinClientHARegionCacheListenerTest::listener_{};

::std::unique_ptr<::apache::geode::client::Cache>
    ThinClientHARegionCacheListenerTest::cache_{};
::std::shared_ptr<::apache::geode::client::Region>
    ThinClientHARegionCacheListenerTest::region_{};
}  // namespace

using ::testing::_;  // Match all
using ::testing::AllOf;
using ::testing::Eq;
using ::testing::IsNull;
using ::testing::Property;

using ::apache::geode::client::CacheableString;
using ::apache::geode::client::EntryEvent;
using ::apache::geode::client::RegionEvent;

TEST_F(ThinClientHARegionCacheListenerTest, afterCreate) {
  auto key = std::make_shared<CacheableString>("key");
  auto value = std::make_shared<CacheableString>("value");

  EXPECT_CALL(
      *listener_,
      afterCreate(AllOf(Property(&EntryEvent::getRegion, Eq(region_)),
                        Property(&EntryEvent::getKey, CashableEq(key)),
                        Property(&EntryEvent::getNewValue, CashableEq(value)))))
      .Times(1);

  cluster_->getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  region_->put(key, value);

  ::testing::Mock::VerifyAndClearExpectations(listener_.get());
}

TEST_F(ThinClientHARegionCacheListenerTest, afterUpdate) {
  auto key = std::make_shared<CacheableString>("key");
  auto value_1 = std::make_shared<CacheableString>("value_1");
  auto value_2 = std::make_shared<CacheableString>("value_2");

  EXPECT_CALL(*listener_, afterUpdate(_)).Times(0);

  cluster_->getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  region_->put(key, value_1);
  region_->put(key, value_2);

  ::testing::Mock::VerifyAndClearExpectations(listener_.get());
}

TEST_F(ThinClientHARegionCacheListenerTest, afterInvalidate) {
  auto key = std::make_shared<CacheableString>("key");
  auto value = std::make_shared<CacheableString>("value");

  EXPECT_CALL(*listener_, afterInvalidate(AllOf(
                              Property(&EntryEvent::getRegion, Eq(region_)),
                              Property(&EntryEvent::getKey, CashableEq(key)))))
      .Times(1);

  cluster_->getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  region_->put(key, value);
  region_->invalidate(key);

  ::testing::Mock::VerifyAndClearExpectations(listener_.get());
}

TEST_F(ThinClientHARegionCacheListenerTest, afterDestroy) {
  auto key = std::make_shared<CacheableString>("key");
  auto value = std::make_shared<CacheableString>("value");

  EXPECT_CALL(*listener_, afterDestroy(AllOf(
                              Property(&EntryEvent::getRegion, Eq(region_)),
                              Property(&EntryEvent::getKey, CashableEq(key)))))
      .Times(1);

  cluster_->getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  region_->put(key, value);
  region_->destroy(key);

  ::testing::Mock::VerifyAndClearExpectations(listener_.get());
}

TEST_F(ThinClientHARegionCacheListenerTest, afterRegionInvalidate) {}

TEST_F(ThinClientHARegionCacheListenerTest, afterRegionDestroy) {
  EXPECT_CALL(*listener_, afterRegionDestroy(_)).Times(0);

  cluster_->getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  cluster_->getGfsh().destroy().region().withName("region").execute();

  ::testing::Mock::VerifyAndClearExpectations(listener_.get());
}

TEST_F(ThinClientHARegionCacheListenerTest, afterRegionClear) {}

TEST_F(ThinClientHARegionCacheListenerTest, afterRegionLive) {
  /* As of this writing, you typically only get afterRegionLive once. This
   * means, if any prior test runs, you'll have already processed this event.
   * But there is an exception: the state is reset if the endpoint disconnects.
   * So here is a trick to assure this test will function successfully and
   * independently of other tests. */
  cycle_server_EP_connection();

  EXPECT_CALL(*listener_,
              afterRegionLive(Property(&RegionEvent::getRegion, Eq(region_))))
      .Times(1);

  cluster_->getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  ::testing::Mock::VerifyAndClearExpectations(listener_.get());
}

TEST_F(ThinClientHARegionCacheListenerTest, afterRegionLiveONLYONCE) {
  /* As of this writing, you typically only get afterRegionLive once. This
   * means, if any prior test runs, you'll have already processed this event.
   * But there is an exception: the state is reset if the endpoint disconnects.
   * So here is a trick to assure this test will function successfully and
   * independently of other tests. */
  cycle_server_EP_connection();

  cluster_->getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  cluster_->getGfsh().destroy().region().withName("region").execute();

  EXPECT_CALL(*listener_, afterRegionLive(_)).Times(0);

  cluster_->getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  ::testing::Mock::VerifyAndClearExpectations(listener_.get());
}

TEST_F(ThinClientHARegionCacheListenerTest, afterRegionLiveAfterEPDisconnect) {
  // Assure we've already received an afterRegionLive event by creating the
  // region.
  cluster_->getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  // Then destroy it for the test.
  cluster_->getGfsh().destroy().region().withName("region").execute();

  // Cycle the endpoint to flush the client-internal process marker.
  // We should be able to get a second afterRegionLive after this.
  cycle_server_EP_connection();

  EXPECT_CALL(*listener_,
              afterRegionLive(Property(&RegionEvent::getRegion, Eq(region_))))
      .Times(1);

  cluster_->getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  ::testing::Mock::VerifyAndClearExpectations(listener_.get());
}

TEST_F(ThinClientHARegionCacheListenerTest, close) {}

TEST_F(ThinClientHARegionCacheListenerTest, afterRegionDisconnected) {}