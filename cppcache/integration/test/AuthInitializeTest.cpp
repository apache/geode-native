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
 public:
  std::shared_ptr<Properties> getCredentials(
      const std::shared_ptr<Properties>& securityprops,
      const std::string& /*server*/) override {
    std::cout << "SimpleAuthInitialize::GetCredentials called\n";
    //    Exception ex("Debugging SimpleAuthInitialize::getCredentials");
    //    std::cout << ex.getStackTrace() << std::endl;

    securityprops->insert("security-username", username_);
    securityprops->insert("security-password", password_);

    countOfGetCredentialsCalls_++;
    return securityprops;
  }

  void close() override { std::cout << "SimpleAuthInitialize::close called\n"; }

  SimpleAuthInitialize()
      : AuthInitialize(),
        username_("root"),
        password_("root-password"),
        countOfGetCredentialsCalls_(0) {
    std::cout << "SimpleAuthInitialize::SimpleAuthInitialize called\n";
  }
  SimpleAuthInitialize(std::string username, std::string password)
      : username_(std::move(username)),
        password_(std::move(password)),
        countOfGetCredentialsCalls_(0) {}

  ~SimpleAuthInitialize() override = default;

  int32_t getGetCredentialsCallCount() { return countOfGetCredentialsCalls_; }

 private:
  std::string username_;
  std::string password_;
  int32_t countOfGetCredentialsCalls_;
};

Cache createCache(std::shared_ptr<SimpleAuthInitialize> auth) {
  auto cache = CacheFactory()
                   .set("log-level", "debug")
                   .set("log-file", "geode_native.log")
                   .set("statistic-sampling-enabled", "false")
                   .setAuthInitialize(auth)
                   .create();

  return cache;
}

Cache createCacheWithBadPassword(std::shared_ptr<SimpleAuthInitialize> auth) {
  auto cache = CacheFactory()
                   .set("log-level", "debug")
                   .set("log-file", "geode_native.log")
                   .set("statistic-sampling-enabled", "false")
                   .setAuthInitialize(auth)
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

TEST(AuthInitializeTest, putGetWithBasicAuth) {
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
  auto authInitialize = std::make_shared<SimpleAuthInitialize>(
      "unauthorized-user", "root-password");
  auto cache = createCache(authInitialize);
  auto pool = createPool(cluster, cache, false);
  auto region = setupRegion(cache, pool);

  try {
    region->put("foo", "bar");
  } catch (const apache::geode::client::NotConnectedException&) {
  } catch (const apache::geode::client::Exception& ex) {
    std::cerr << "Caught unexpected exception: " << ex.what() << std::endl;
    FAIL();
  }

  ASSERT_GT(authInitialize->getGetCredentialsCallCount(), 0);
}

TEST(AuthInitializeTest, putWithBadPassword) {
  Cluster cluster(
      Name(std::string(::testing::UnitTest::GetInstance()
                           ->current_test_info()
                           ->test_case_name()) +
           "/" +
           ::testing::UnitTest::GetInstance()->current_test_info()->name()),
      Classpath{getFrameworkString(FrameworkVariable::JavaObjectJarPath)},
      SecurityManager{"javaobject.SimpleSecurityManager"}, User{"root"},
      Password{"root-password"}, LocatorCount{1}, ServerCount{1});

  auto authInitialize =
      std::make_shared<SimpleAuthInitialize>("root", "bad-password");
  auto cache = createCache(authInitialize);
  auto pool = createPool(cluster, cache, false);
  auto region = setupRegion(cache, pool);

  try {
    region->put("foo", "bar");
  } catch (const apache::geode::client::NotConnectedException&) {
  } catch (const apache::geode::client::Exception& ex) {
    std::cerr << "Caught unexpected exception: " << ex.what() << std::endl;
    FAIL();
  }

  ASSERT_GT(authInitialize->getGetCredentialsCallCount(), 0);
}

TEST(AuthInitializeTest, badCredentialsWithSubscriptionEnabled) {
  Cluster cluster(
      Name(std::string(::testing::UnitTest::GetInstance()
                           ->current_test_info()
                           ->test_case_name()) +
           "/" +
           ::testing::UnitTest::GetInstance()->current_test_info()->name()),
      Classpath{getFrameworkString(FrameworkVariable::JavaObjectJarPath)},
      SecurityManager{"javaobject.SimpleSecurityManager"}, User{"root"},
      Password{"root-password"}, LocatorCount{1}, ServerCount{1});

  auto authInitialize =
      std::make_shared<SimpleAuthInitialize>("root", "bad-password");
  auto cache = createCache(authInitialize);

  try {
    createPool(cluster, cache, true);
  } catch (const apache::geode::client::AuthenticationFailedException&) {
  } catch (const apache::geode::client::Exception& ex) {
    std::cerr << "Caught unexpected exception: " << ex.what() << std::endl;
    FAIL();
  }

  ASSERT_GT(authInitialize->getGetCredentialsCallCount(), 0);
}

}  // namespace
