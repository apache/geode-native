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

#include <chrono>
#include <iostream>
#include <thread>

#include <boost/thread/latch.hpp>

#include <gtest/gtest.h>

#include <geode/AuthInitialize.hpp>
#include <geode/Cache.hpp>
#include <geode/CqAttributesFactory.hpp>
#include <geode/CqEvent.hpp>
#include <geode/PoolManager.hpp>
#include <geode/QueryService.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "CacheRegionHelper.hpp"
#include "SimpleAuthInitialize.hpp"
#include "SimpleCqListener.hpp"
#include "framework/Cluster.h"

namespace {

using apache::geode::client::AuthInitialize;
using apache::geode::client::Cache;
using apache::geode::client::Cacheable;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::CqAttributes;
using apache::geode::client::CqAttributesFactory;
using apache::geode::client::CqEvent;
using apache::geode::client::CqListener;
using apache::geode::client::CqOperation;
using apache::geode::client::Exception;
using apache::geode::client::HashMapOfCacheable;
using apache::geode::client::Pool;
using apache::geode::client::Properties;
using apache::geode::client::QueryService;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

using std::chrono::minutes;

const int32_t CQ_PLUS_AUTH_TEST_REGION_ENTRY_COUNT = 50000;
const int32_t CQ_REAUTH_TEST_REGION_ENTRY_COUNT = 5000;

Cache createCache(std::shared_ptr<SimpleAuthInitialize> auth) {
  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("statistic-sampling-enabled", "false")
                   .setAuthInitialize(auth)
                   .create();

  return cache;
}

Cache createDurableCache(std::shared_ptr<SimpleAuthInitialize> auth) {
  auto pp = Properties::create();
  pp->insert("durable-client-id", "DurableClient-1111");
  pp->insert("durable-timeout", std::chrono::seconds(600));
  auto cache = CacheFactory(pp)
                   .set("log-level", "none")
                   .set("statistic-sampling-enabled", "false")
                   .setAuthInitialize(auth)
                   .create();

  return cache;
}

std::shared_ptr<Pool> createPool(Cluster& cluster, Cache& cache,
                                 bool subscriptionEnabled) {
  auto poolFactory = cache.getPoolManager().createFactory().setIdleTimeout(
      std::chrono::milliseconds(0));

  cluster.applyLocators(poolFactory);
  poolFactory.setPRSingleHopEnabled(true).setSubscriptionEnabled(
      subscriptionEnabled);

  return poolFactory.create("default");
}

std::shared_ptr<Region> setupRegion(Cache& cache,
                                    const std::shared_ptr<Pool>& pool) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName(pool->getName())
                    .create("region");

  return region;
}

TEST(CqPlusAuthInitializeTest, putInALoopWhileSubscribedAndAuthenticated) {
  Cluster cluster(
      Name(std::string(::testing::UnitTest::GetInstance()
                           ->current_test_info()
                           ->test_suite_name()) +
           "/" +
           ::testing::UnitTest::GetInstance()->current_test_info()->name()),
      Classpath{getFrameworkString(FrameworkVariable::JavaObjectJarPath)},
      SecurityManager{"javaobject.SimpleSecurityManager"}, User{"root"},
      Password{"root-password"}, LocatorCount{1}, ServerCount{1});

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  auto authInitialize = std::make_shared<SimpleAuthInitialize>();
  auto cache = createCache(authInitialize);
  auto pool = createPool(cluster, cache, true);
  auto region = setupRegion(cache, pool);

  try {
    region->put("foo", "bar");
  } catch (const Exception& ex) {
    std::cerr << "Caught exception: " << ex.what() << std::endl;
    std::cerr << "In initial region put" << std::endl;
    std::cerr << "Callstack" << ex.getStackTrace() << std::endl;
    FAIL();
  }

  auto queryService = cache.getQueryService();

  auto createLatch =
      std::make_shared<boost::latch>(CQ_PLUS_AUTH_TEST_REGION_ENTRY_COUNT);
  auto updateLatch =
      std::make_shared<boost::latch>(CQ_PLUS_AUTH_TEST_REGION_ENTRY_COUNT);
  auto destroyLatch =
      std::make_shared<boost::latch>(CQ_PLUS_AUTH_TEST_REGION_ENTRY_COUNT);
  auto testListener = std::make_shared<SimpleCqListener>(
      createLatch, updateLatch, destroyLatch);

  CqAttributesFactory attributesFactory;
  attributesFactory.addCqListener(testListener);
  auto cqAttributes = attributesFactory.create();

  auto query =
      queryService->newCq("SimpleCQ", "SELECT * FROM /region", cqAttributes);

  try {
    query->execute();
  } catch (const Exception& ex) {
    std::cerr << "Caught exception: " << ex.what() << std::endl;
    std::cerr << "While executing Cq" << std::endl;
    std::cerr << "Callstack" << ex.getStackTrace() << std::endl;
    FAIL();
  }

  int32_t i = 0;

  try {
    for (i = 0; i < CQ_PLUS_AUTH_TEST_REGION_ENTRY_COUNT; i++) {
      region->put("key" + std::to_string(i), "value" + std::to_string(i));
      std::this_thread::yield();
    }
  } catch (const Exception& ex) {
    std::cerr << "Caught exception: " << ex.what() << std::endl;
    std::cerr << "In value create loop, i=" << i << std::endl;
    std::cerr << "Callstack" << ex.getStackTrace() << std::endl;
    FAIL();
  }

  try {
    for (i = 0; i < CQ_PLUS_AUTH_TEST_REGION_ENTRY_COUNT; i++) {
      region->put("key" + std::to_string(i), "value" + std::to_string(i + 1));
      std::this_thread::yield();
    }
  } catch (const Exception& ex) {
    std::cerr << "Caught exception: " << ex.what() << std::endl;
    std::cerr << "In value update loop, i=" << i << std::endl;
    std::cerr << "Callstack" << ex.getStackTrace() << std::endl;
    FAIL();
  }

  try {
    for (i = 0; i < CQ_PLUS_AUTH_TEST_REGION_ENTRY_COUNT; i++) {
      region->destroy("key" + std::to_string(i));
      std::this_thread::yield();
    }
  } catch (const Exception& ex) {
    std::cerr << "Caught exception: " << ex.what() << std::endl;
    std::cerr << "In value destroy loop, i=" << i << std::endl;
    std::cerr << "Callstack" << ex.getStackTrace() << std::endl;
    FAIL();
  }

  EXPECT_EQ(boost::cv_status::no_timeout,
            createLatch->wait_for(boost::chrono::seconds(30)));
  EXPECT_EQ(boost::cv_status::no_timeout,
            updateLatch->wait_for(boost::chrono::seconds(30)));
  EXPECT_EQ(boost::cv_status::no_timeout,
            destroyLatch->wait_for(boost::chrono::seconds(30)));

  EXPECT_GT(authInitialize->getGetCredentialsCallCount(), 0);
}

TEST(CqPlusAuthInitializeTest, reAuthenticateWithDurable) {
  // Start a cluster with user expiration support

  Cluster cluster(
      Name(std::string(::testing::UnitTest::GetInstance()
                           ->current_test_info()
                           ->test_suite_name()) +
           "/" +
           ::testing::UnitTest::GetInstance()->current_test_info()->name()),
      Classpath{getFrameworkString(FrameworkVariable::JavaObjectJarPath)},
      SecurityManager{"javaobject.SimulatedExpirationSecurityManager"},
      User{"root"}, Password{"root-password"}, LocatorCount{1}, ServerCount{1});

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  // Use a non-root user for reAuthentication testing.

  auto authInitialize =
      std::make_shared<SimpleAuthInitialize>("user", "user-password");
  auto cache = createDurableCache(authInitialize);
  auto pool = createPool(cluster, cache, true);
  auto region = setupRegion(cache, pool);

  try {
    region->put("foo", "bar");
  } catch (const Exception& ex) {
    std::cerr << "Caught exception: " << ex.what() << std::endl;
    std::cerr << "In initial region put" << std::endl;
    std::cerr << "Callstack" << ex.getStackTrace() << std::endl;
    FAIL();
  }

  auto queryService = cache.getQueryService();

  auto createLatch =
      std::make_shared<boost::latch>(CQ_REAUTH_TEST_REGION_ENTRY_COUNT);
  auto updateLatch =
      std::make_shared<boost::latch>(CQ_REAUTH_TEST_REGION_ENTRY_COUNT);
  auto destroyLatch =
      std::make_shared<boost::latch>(CQ_REAUTH_TEST_REGION_ENTRY_COUNT);
  auto testListener = std::make_shared<SimpleCqListener>(
      createLatch, updateLatch, destroyLatch);

  CqAttributesFactory attributesFactory;
  attributesFactory.addCqListener(testListener);
  auto cqAttributes = attributesFactory.create();

  auto query = queryService->newCq("SimpleCQ", "SELECT * FROM /region",
                                   cqAttributes, true);

  try {
    query->execute();
  } catch (const Exception& ex) {
    std::cerr << "Caught exception: " << ex.what() << std::endl;
    std::cerr << "While executing Cq" << std::endl;
    std::cerr << "Callstack" << ex.getStackTrace() << std::endl;
    FAIL();
  }

  int32_t i = 0;

  cache.readyForEvents();

  try {
    for (i = 0; i < CQ_REAUTH_TEST_REGION_ENTRY_COUNT; i++) {
      region->put("key" + std::to_string(i), "value" + std::to_string(i));
      std::this_thread::yield();
    }
  } catch (const Exception& ex) {
    std::cerr << "Caught exception: " << ex.what() << std::endl;
    std::cerr << "In value create loop, i=" << i << std::endl;
    std::cerr << "Callstack" << ex.getStackTrace() << std::endl;
    FAIL();
  }

  try {
    for (i = 0; i < CQ_REAUTH_TEST_REGION_ENTRY_COUNT; i++) {
      region->put("key" + std::to_string(i), "value" + std::to_string(i + 1));
      std::this_thread::yield();
    }
  } catch (const Exception& ex) {
    std::cerr << "Caught exception: " << ex.what() << std::endl;
    std::cerr << "In value update loop, i=" << i << std::endl;
    std::cerr << "Callstack" << ex.getStackTrace() << std::endl;
    FAIL();
  }

  try {
    for (i = 0; i < CQ_REAUTH_TEST_REGION_ENTRY_COUNT; i++) {
      region->destroy("key" + std::to_string(i));
      std::this_thread::yield();
    }
  } catch (const Exception& ex) {
    std::cerr << "Caught exception: " << ex.what() << std::endl;
    std::cerr << "In value destroy loop, i=" << i << std::endl;
    std::cerr << "Callstack" << ex.getStackTrace() << std::endl;
    FAIL();
  }

  EXPECT_EQ(boost::cv_status::no_timeout,
            createLatch->wait_for(boost::chrono::seconds(90)));
  EXPECT_EQ(boost::cv_status::no_timeout,
            updateLatch->wait_for(boost::chrono::seconds(90)));
  EXPECT_EQ(boost::cv_status::no_timeout,
            destroyLatch->wait_for(boost::chrono::seconds(90)));

  // SimulatedExpirationSecurityManager is set to throw
  // AuthenticationExpiredException for 1% of operations.
  // If the random number generator were perfect we'd expect reauth
  // to happen for 1% of the operations. Back it off to .5% since
  // it's not perfect.
  //
  // Number of operations is 3 * CQ_REAUTH_TEST_REGION_ENTRY_COUNT
  // since doing put, update, and destory.

  EXPECT_GT(authInitialize->getGetCredentialsCallCount(),
            3 * CQ_REAUTH_TEST_REGION_ENTRY_COUNT * .005);

  // Keep the durable messages so we can test getting them back
  // on reconnect.

  cache.close(true);

  // Create a new durable cache with the same durable-client-id and
  // ensure we get all the events that were created.

  auto cacheRecovery = createDurableCache(authInitialize);
  auto poolRecovery = createPool(cluster, cacheRecovery, true);
  auto regionRecovery = setupRegion(cacheRecovery, poolRecovery);

  auto queryServiceRecovery = cacheRecovery.getQueryService();

  auto createLatchRecovery =
      std::make_shared<boost::latch>(CQ_REAUTH_TEST_REGION_ENTRY_COUNT);
  auto updateLatchRecovery =
      std::make_shared<boost::latch>(CQ_REAUTH_TEST_REGION_ENTRY_COUNT);
  auto destroyLatchRecovery =
      std::make_shared<boost::latch>(CQ_REAUTH_TEST_REGION_ENTRY_COUNT);
  auto testListenerRecovery = std::make_shared<SimpleCqListener>(
      createLatchRecovery, updateLatchRecovery, destroyLatchRecovery);

  CqAttributesFactory attributesFactoryRecovery;
  attributesFactoryRecovery.addCqListener(testListenerRecovery);
  auto cqAttributesRecovery = attributesFactoryRecovery.create();

  auto queryRecovery = queryServiceRecovery->newCq(
      "SimpleCQ", "SELECT * FROM /region", cqAttributesRecovery, true);

  try {
    queryRecovery->execute();
  } catch (const Exception& ex) {
    std::cerr << "Caught exception: " << ex.what() << std::endl;
    std::cerr << "While executing Cq" << std::endl;
    std::cerr << "Callstack" << ex.getStackTrace() << std::endl;
    FAIL();
  }

  cacheRecovery.readyForEvents();

  EXPECT_EQ(boost::cv_status::no_timeout,
            createLatchRecovery->wait_for(boost::chrono::seconds(90)));
  EXPECT_EQ(boost::cv_status::no_timeout,
            updateLatchRecovery->wait_for(boost::chrono::seconds(90)));
  EXPECT_EQ(boost::cv_status::no_timeout,
            destroyLatchRecovery->wait_for(boost::chrono::seconds(90)));
}

}  // namespace
