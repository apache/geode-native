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
#include "SimpleCqListener.hpp"
#include "framework/Cluster.h"
#include "framework/Framework.h"
#include "framework/Gfsh.h"

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

const int32_t CQ_PLUS_AUTH_TEST_REGION_ENTRY_COUNT = 100000;

class SimpleAuthInitialize : public apache::geode::client::AuthInitialize {
  std::shared_ptr<Properties> getCredentials(
      const std::shared_ptr<Properties>& securityprops,
      const std::string& /*server*/) override {
    std::cout << "SimpleAuthInitialize::GetCredentials called\n";
    //    Exception ex("Debugging SimpleAuthInitialize::getCredentials");
    //    std::cout << ex.getStackTrace() << std::endl;

    securityprops->insert("security-username", username_);
    securityprops->insert("security-password", password_);

    return securityprops;
  }

  void close() override { std::cout << "SimpleAuthInitialize::close called\n"; }

 public:
  SimpleAuthInitialize()
      : AuthInitialize(), username_("root"), password_("root-password") {
    std::cout << "SimpleAuthInitialize::SimpleAuthInitialize called\n";
  }
  SimpleAuthInitialize(std::string username, std::string password)
      : username_(std::move(username)), password_(std::move(password)) {}

  ~SimpleAuthInitialize() override = default;

 private:
  std::string username_;
  std::string password_;
};

Cache createCache() {
  auto cache = CacheFactory()
                   .set("log-level", "debug")
                   .set("log-file", "geode_native.log")
                   .set("statistic-sampling-enabled", "false")
                   .setAuthInitialize(std::make_shared<SimpleAuthInitialize>())
                   .create();

  return cache;
}

Cache createCacheWithBadPassword() {
  auto cache = CacheFactory()
                   .set("log-level", "debug")
                   .set("log-file", "geode_native.log")
                   .set("statistic-sampling-enabled", "false")
                   .setAuthInitialize(std::make_shared<SimpleAuthInitialize>(
                       "root", "bad-password"))
                   .create();

  return cache;
}

Cache createCacheWithBadUsername() {
  auto cache = CacheFactory()
                   .set("log-level", "debug")
                   .set("log-file", "geode_native.log")
                   .set("statistic-sampling-enabled", "false")
                   .setAuthInitialize(std::make_shared<SimpleAuthInitialize>(
                       "unauthorized-user", "root-password"))
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

TEST(CqPlusAuthInitializeTest, putInALoopWhileSubscribedAndAuthenticated) {
  Cluster cluster(
      Name(std::string(::testing::UnitTest::GetInstance()
                           ->current_test_info()
                           ->test_case_name()) +
           "/" +
           ::testing::UnitTest::GetInstance()->current_test_info()->name()),
      Classpath{getFrameworkString(FrameworkVariable::JavaObjectJarPath)},
      SecurityManager{"javaobject.SimpleSecurityManager"}, User{"root"},
      Password{"root-password"}, LocatorCount{1}, ServerCount{1});
  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  auto cache = createCache();
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

  CqAttributesFactory attributesFactory;
  auto testListener = std::make_shared<SimpleCqListener>();
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
    }
  } catch (const Exception& ex) {
    std::cerr << "Caught exception: " << ex.what() << std::endl;
    std::cerr << "In value destroy loop, i=" << i << std::endl;
    std::cerr << "Callstack" << ex.getStackTrace() << std::endl;
    FAIL();
  }

  for (i = 0; i < 100; i++) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (testListener->getCreationCount() ==
        CQ_PLUS_AUTH_TEST_REGION_ENTRY_COUNT) {
      break;
    }
  }

  ASSERT_EQ(testListener->getCreationCount(),
            CQ_PLUS_AUTH_TEST_REGION_ENTRY_COUNT);
  ASSERT_EQ(testListener->getUpdateCount(),
            CQ_PLUS_AUTH_TEST_REGION_ENTRY_COUNT);
  ASSERT_EQ(testListener->getDestructionCount(),
            CQ_PLUS_AUTH_TEST_REGION_ENTRY_COUNT);
}

}  // namespace
