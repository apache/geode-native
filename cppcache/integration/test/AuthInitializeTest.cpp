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
#include <future>
#include <iostream>
#include <random>
#include <thread>

#include <gtest/gtest.h>

#include <geode/AuthInitialize.hpp>
#include <geode/Cache.hpp>
#include <geode/CqAttributes.hpp>
#include <geode/CqAttributesFactory.hpp>
#include <geode/CqEvent.hpp>
#include <geode/CqListener.hpp>
#include <geode/PoolManager.hpp>
#include <geode/QueryService.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "CacheRegionHelper.hpp"
#include "SimpleAuthInitialize.hpp"
#include "SimpleCqListener.hpp"
#include "framework/Cluster.h"
#include "framework/Framework.h"
#include "framework/Gfsh.h"

using apache::geode::client::AuthenticationFailedException;
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
using apache::geode::client::NotConnectedException;
using apache::geode::client::Pool;
using apache::geode::client::Properties;
using apache::geode::client::QueryService;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

using std::chrono::minutes;

const int32_t CQ_PLUS_AUTH_TEST_REGION_ENTRY_COUNT = 100000;

Cache createCache(std::shared_ptr<SimpleAuthInitialize> auth) {
  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("statistic-sampling-enabled", "false")
                   .setAuthInitialize(auth)
                   .create();

  return cache;
}

std::shared_ptr<Pool> createPool(Cluster& cluster, Cache& cache,
                                 bool subscriptionEnabled) {
  auto poolFactory = cache.getPoolManager().createFactory();

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

TEST(AuthInitializeTest, putGetWithBasicAuth) {
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
  auto pool = createPool(cluster, cache, false);
  auto region = setupRegion(cache, pool);

  region->put("foo", "bar");
  auto value = region->get("foo");
  auto stringValue = std::dynamic_pointer_cast<CacheableString>(value)->value();
  ASSERT_EQ(stringValue, std::string("bar"));
  ASSERT_GT(authInitialize->getGetCredentialsCallCount(), 0);
}

TEST(AuthInitializeTest, putWithBadUsername) {
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
  auto authInitialize = std::make_shared<SimpleAuthInitialize>(
      "unauthorized-user", "root-password");
  auto cache = createCache(authInitialize);
  auto pool = createPool(cluster, cache, false);
  auto region = setupRegion(cache, pool);

  try {
    region->put("foo", "bar");
  } catch (const NotConnectedException&) {
  } catch (const Exception& ex) {
    std::cerr << "Caught unexpected exception: " << ex.what() << std::endl;
    FAIL();
  }

  ASSERT_GT(authInitialize->getGetCredentialsCallCount(), 0);
}

TEST(AuthInitializeTest, putWithBadPassword) {
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

  auto authInitialize =
      std::make_shared<SimpleAuthInitialize>("root", "bad-password");
  auto cache = createCache(authInitialize);
  auto pool = createPool(cluster, cache, false);
  auto region = setupRegion(cache, pool);

  try {
    region->put("foo", "bar");
  } catch (const NotConnectedException&) {
  } catch (const Exception& ex) {
    std::cerr << "Caught unexpected exception: " << ex.what() << std::endl;
    FAIL();
  }

  ASSERT_GT(authInitialize->getGetCredentialsCallCount(), 0);
}

TEST(AuthInitializeTest, badCredentialsWithSubscriptionEnabled) {
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

  auto authInitialize =
      std::make_shared<SimpleAuthInitialize>("root", "bad-password");
  auto cache = createCache(authInitialize);

  try {
    createPool(cluster, cache, true);
  } catch (const AuthenticationFailedException&) {
  } catch (const Exception& ex) {
    std::cerr << "Caught unexpected exception: " << ex.what() << std::endl;
    FAIL();
  }

  ASSERT_GT(authInitialize->getGetCredentialsCallCount(), 0);
}
