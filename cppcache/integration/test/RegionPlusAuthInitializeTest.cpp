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
using apache::geode::client::HashMapOfCacheable;
using apache::geode::client::Pool;
using apache::geode::client::Properties;
using apache::geode::client::QueryService;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

using std::chrono::minutes;

class SimpleAuthInitialize : public apache::geode::client::AuthInitialize {
  std::shared_ptr<Properties> getCredentials(
      const std::shared_ptr<Properties>& securityprops,
      const std::string& /*server*/) override {
    std::cout << "SimpleAuthInitialize::GetCredentials called\n";

    securityprops->insert("security-username", "root");
    securityprops->insert("security-password", "root-password");

    return securityprops;
  }

  void close() override { std::cout << "SimpleAuthInitialize::close called\n"; }

 public:
  SimpleAuthInitialize() : AuthInitialize() {
    std::cout << "SimpleAuthInitialize::SimpleAuthInitialize called\n";
  }
  ~SimpleAuthInitialize() override = default;
};

Cache createCache() {
  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("statistic-sampling-enabled", "false")
                   .setAuthInitialize(std::make_shared<SimpleAuthInitialize>())
                   .create();

  return cache;
}

class SimpleCqListener : public CqListener {
 public:
  void onEvent(const CqEvent& cqEvent) override {
    auto opStr = "Default";

    auto key(dynamic_cast<CacheableString*>(cqEvent.getKey().get()));
    auto value(dynamic_cast<CacheableString*>(cqEvent.getNewValue().get()));

    switch (cqEvent.getQueryOperation()) {
      case CqOperation::OP_TYPE_CREATE:
        opStr = "CREATE";
        std::cout << "CqListener CREATE event received" << std::endl;
        break;
      case CqOperation::OP_TYPE_UPDATE:
        opStr = "UPDATE";
        std::cout << "CqListener UPDATE event received" << std::endl;
        break;
      case CqOperation::OP_TYPE_DESTROY:
        opStr = "DESTROY";
        std::cout << "CqListener DESTROY event received" << std::endl;
        break;
      default:
        break;
    }
  }

  void onError(const CqEvent& cqEvent) override {
    std::cout << __FUNCTION__ << " called"
              << dynamic_cast<CacheableString*>(cqEvent.getKey().get())->value()
              << std::endl;
  }

  void close() override { std::cout << __FUNCTION__ << " called" << std::endl; }
};

std::shared_ptr<Pool> createPool(Cluster& cluster, Cache& cache) {
  auto poolFactory =
      cache.getPoolManager().createFactory().setSubscriptionEnabled(true);

  cluster.applyLocators(poolFactory);
  poolFactory.setPRSingleHopEnabled(true);

  return poolFactory.create("default");
}

std::shared_ptr<Region> setupRegion(Cache& cache,
                                    const std::shared_ptr<Pool>& pool) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName(pool->getName())
                    .create("region");

  return region;
}

TEST(RegionPlusAuthInitializeTest, putInALoopWhileSubscribedAndAuthenticated) {
  auto locatorPort = Framework::getAvailablePort();
  auto jmxManagerPort = Framework::getAvailablePort();
  auto serverPort = Framework::getAvailablePort();

  auto gfe = GfshExecute();
  gfe.start()
      .locator()
      .withDir("putInALoopWhileSubscribedAndAuthenticated")
      //      .withName("locator")
      .withBindAddress("localhost")
      .withPort(locatorPort)
      .withClasspath("../../../../tests/javaobject/javaobject.jar")
      .withSecurityManager("javaobject.SimpleSecurityManager")
      .withJmxManagerPort(jmxManagerPort)
      .withHttpServicePort(0)
      //            .withStartJmxManager("true")
      //      .withConnect("false")
      .execute();

  gfe.start()
      .server()
      .withDir("putInALoopWhileSubscribedAndAuthenticated")
      //      .withName("server")
      .withBindAddress("localhost")
      .withPort(serverPort)
      .withMaxHeap("1g")
      .withClasspath("../../../../tests/javaobject/javaobject.jar")
      .withSecurityManager("javaobject.SimpleSecurityManager")
      .withUser("root")
      .withPassword("root-password")

      .execute();

  gfe.create().region().withName("region").withType("PARTITION").execute();

  auto cache = createCache();

  auto pool = cache.getPoolManager()
                  .createFactory()
                  .setSubscriptionEnabled(true)
                  .addServer("localhost", serverPort)
                  .create("default");

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName(pool->getName())
                    .create("region");
  std::cerr << "Created region" << std::endl;

  auto queryService = cache.getQueryService();
  std::cerr << "Got query service" << std::endl;
  CqAttributesFactory attributesFactory;
  attributesFactory.addCqListener(std::make_shared<SimpleCqListener>());
  std::cerr << "Added CqListener" << std::endl;

  auto attributes = attributesFactory.create();

  auto query = queryService->newCq("select * from /region", attributes);
  std::cerr << "Created query" << std::endl;

  for (int i = 0; i < 100; i++) {
    auto key = "key" + std::to_string(i);
    auto value = "value" + std::to_string(i);
    region->put(key, value);
    std::cerr << "Put (" << key << ", " << value << ")" << std::endl;
  }
}

}  // namespace
