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

#include <geode/AttributesMutator.hpp>
#include <geode/Cache.hpp>
#include <geode/CacheListener.hpp>
#include <geode/EntryEvent.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionEvent.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "mock/CacheListenerMock.hpp"

namespace {
// Until we get C++14 support...
template <typename T, typename... Args>
::std::unique_ptr<T> make_unique(Args &&... args) {
  return ::std::unique_ptr<T>(new T(::std::forward<Args>(args)...));
}

// A simple comparator for cachables wrapped in shared pointers.
MATCHER_P(CashableEq, value, "") {
  return arg->toString() == value->toString();
}

class HARegionCacheListenerWithClusterRegionTest : public ::testing::Test {
 protected:
  static ::std::unique_ptr<::Cluster> cluster_;
  /* Cache listener mocks make a circular reference to the region, and the
   * region retains a circular reference back to the cache. The reference is
   * established when a callback is called and is resolved when the mock is
   * verified and cleared. The circular reference prevents the cache from
   * falling out of scope and shutting down, and it prevents the mock instance
   * from verifying and clearing itself. You have to manually verify and clear
   * the mock at the end of every test. */
  static ::std::shared_ptr<::apache::geode::client::Nice_MockListener>
      listener_;
  static ::std::unique_ptr<::apache::geode::client::Cache> cache_;
  static ::std::shared_ptr<::apache::geode::client::Region> region_;

  /* Surprise! There are different code paths for event callbacks. Some are
  sequential, the result of a server response; Some of them are concurrent - the
  result of a server message.

  TODO: Build out both sequential and concurrent listener events. */
  ::std::mutex cv_m_;
  ::std::condition_variable cv_;
  bool verified_;
  std::function<void(const apache::geode::client::RegionEvent &)>
      verify_the_regionevent_callback;
  std::function<void(const apache::geode::client::EntryEvent &)>
      verify_the_entryevent_callback;
  std::function<bool()> the_verification_condition;

  /* SetUp and TearDown happen after test fixture construction and before test
   * fixture destruction to avoid superious noise in the mock. */
  void SetUp() override {
    ::apache::geode::client::AttributesMutator(region_).setCacheListener(
        listener_);
  }

  void TearDown() override {
    ::apache::geode::client::AttributesMutator(region_).setCacheListener({});

    ::testing::Mock::VerifyAndClearExpectations(
        listener_.get());  // Just to make sure it's clean for the next test.
  }

  HARegionCacheListenerWithClusterRegionTest()
      : verified_{false},
        verify_the_regionevent_callback{::testing::Invoke(
            this, &HARegionCacheListenerWithClusterRegionTest::
                      verify_regionevent_callback)},
        verify_the_entryevent_callback{::testing::Invoke(
            this, &HARegionCacheListenerWithClusterRegionTest::
                      verify_entryevent_callback)},
        the_verification_condition{std::bind(
            &HARegionCacheListenerWithClusterRegionTest::is_verified, this)} {
    create_the_test_region();
  }

  ~HARegionCacheListenerWithClusterRegionTest() override {
    destroy_any_test_region();
  }

  //   void clear_marker_processed_flag() {
  //     /* As of this writing, you typically only get afterRegionLive once.
  //     This
  //      * means, if almost any any prior test runs, you'll likely have already
  //      * processed this event. But there is an exception: the state is reset
  //      if
  //      * the endpoint disconnects.
  //      */
  //     cluster_->getServers()[0].stop();
  //     cluster_->getServers()[0].start();
  //   }

  void create_the_test_region() {
    cluster_->getGfsh()
        .create()
        .region()
        .withName(get_the_test_region_name())
        .withType("PARTITION")
        .execute();
  }

  void destroy_any_test_region() {
    cluster_->getGfsh()
        .destroy()
        .region()
        .ifExists()
        .withName(get_the_test_region_name())
        .execute();
  }

  static const std::string &get_the_test_region_name() {
    static const std::string test_region_name{"region"};

    return test_region_name;
  }

  void verify_regionevent_callback(
      const ::apache::geode::client::RegionEvent &) {
    auto lk = std::unique_lock<std::mutex>(cv_m_);
    verified_ = true;
    cv_.notify_one();
  }

  void verify_entryevent_callback(const ::apache::geode::client::EntryEvent &) {
    auto lk = std::unique_lock<std::mutex>(cv_m_);
    verified_ = true;
    cv_.notify_one();
  }

  bool is_verified() { return verified_; }

 public:
  static void SetUpTestSuite() {
    listener_ =
        ::std::make_shared<::apache::geode::client::Nice_MockListener>();

    cluster_ = make_unique<::Cluster>(
        Name{"HARegionCacheListenerWithClusterRegionTest"}, LocatorCount{1},
        ServerCount{1});

    cluster_->start();

    cache_ = make_unique<::apache::geode::client::Cache>(
        cluster_->createCache({}, ::Cluster::SubscriptionState::Enabled));

    region_ = cache_
                  ->createRegionFactory(
                      ::apache::geode::client::RegionShortcut::PROXY)
                  .setPoolName("default")
                  .create(get_the_test_region_name());
  }

  static void TearDownTestSuite() {
    region_.reset();
    cache_.reset();
    cluster_.reset();
    listener_.reset();
  }
};

::std::unique_ptr<::Cluster>
    HARegionCacheListenerWithClusterRegionTest::cluster_{};

::std::shared_ptr<::apache::geode::client::Nice_MockListener>
    HARegionCacheListenerWithClusterRegionTest::listener_{};

::std::unique_ptr<::apache::geode::client::Cache>
    HARegionCacheListenerWithClusterRegionTest::cache_{};
::std::shared_ptr<::apache::geode::client::Region>
    HARegionCacheListenerWithClusterRegionTest::region_{};
}  // namespace

using ::testing::AllOf;
using ::testing::Eq;
using ::testing::Property;

using ::apache::geode::client::CacheableString;
using ::apache::geode::client::EntryEvent;
using ::apache::geode::client::RegionEvent;

TEST_F(HARegionCacheListenerWithClusterRegionTest, afterCreateSingleThreaded) {
  auto key = std::make_shared<CacheableString>("key");
  auto value = std::make_shared<CacheableString>("value");
  auto with_these_properties =
      AllOf(Property(&EntryEvent::getRegion, Eq(region_)),
            Property(&EntryEvent::getKey, CashableEq(key)),
            Property(&EntryEvent::getNewValue, CashableEq(value)));

  EXPECT_CALL(*listener_, afterCreate(with_these_properties));

  region_->put(key, value);

  ::testing::Mock::VerifyAndClearExpectations(listener_.get());
}

TEST_F(HARegionCacheListenerWithClusterRegionTest,
       DISABLED_afterRegionDestroy) {
  auto with_these_properties = Property(&RegionEvent::getRegion, Eq(region_));

  EXPECT_CALL(*listener_, afterRegionDestroy(with_these_properties))
      .WillOnce(verify_the_regionevent_callback);

  auto lk = std::unique_lock<std::mutex>(cv_m_);

  cluster_->getGfsh().destroy().region().withName("region").execute();

  cv_.wait_for(lk, ::std::chrono::seconds{5}, the_verification_condition);

  ::testing::Mock::VerifyAndClearExpectations(listener_.get());
}
