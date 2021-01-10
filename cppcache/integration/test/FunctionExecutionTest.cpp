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
#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/CacheableBuiltins.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/FunctionService.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "framework/Cluster.h"
#include "framework/Gfsh.h"
#include "framework/TestConfig.h"

using apache::geode::client::Cache;
using apache::geode::client::Cacheable;
using apache::geode::client::CacheableArrayList;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheableVector;
using apache::geode::client::CacheFactory;
using apache::geode::client::FunctionExecutionException;
using apache::geode::client::FunctionService;
using apache::geode::client::NotConnectedException;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;
using apache::geode::client::ResultCollector;
const int ON_SERVERS_TEST_REGION_ENTRIES_SIZE = 34;

std::shared_ptr<Region> setupRegion(Cache &cache) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .create("region");

  return region;
}

class TestResultCollector : public ResultCollector {
  virtual std::shared_ptr<CacheableVector> getResult(
      std::chrono::milliseconds) override {
    return std::shared_ptr<CacheableVector>();
  }

  virtual void addResult(const std::shared_ptr<Cacheable> &) override {}

  virtual void endResults() override {}

  virtual void clearResults() override {}
};

TEST(FunctionExecutionTest, UnknownFunctionOnServer) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = cluster.createCache();
  auto region = setupRegion(cache);

  ASSERT_THROW(FunctionService::onServer(region->getRegionService())
                   .execute("I_Don_t_Exist"),
               FunctionExecutionException);
}

TEST(FunctionExecutionTest, UnknownFunctionOnRegion) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = cluster.createCache();
  auto region = setupRegion(cache);

  ASSERT_THROW(FunctionService::onRegion(region).execute("I_Don_t_Exist"),
               FunctionExecutionException);
}

TEST(FunctionExecutionTest, UnknownFunctionAsyncOnServer) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = cluster.createCache();
  auto region = setupRegion(cache);

  ASSERT_THROW(FunctionService::onServer(region->getRegionService())
                   .withCollector(std::make_shared<TestResultCollector>())
                   .execute("I_Don_t_Exist"),
               FunctionExecutionException);
}

TEST(FunctionExecutionTest, UnknownFunctionAsyncOnRegion) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = cluster.createCache();
  auto region = setupRegion(cache);

  ASSERT_THROW(FunctionService::onRegion(region)
                   .withCollector(std::make_shared<TestResultCollector>())
                   .execute("I_Don_t_Exist"),
               FunctionExecutionException);
}

TEST(FunctionExecutionTest,
     FunctionReturnsObjectWhichCantBeDeserializedOnServer) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();
  cluster.getGfsh()
      .deploy()
      .jar(getFrameworkString(FrameworkVariable::JavaObjectJarPath))
      .execute();
  auto cache = cluster.createCache();
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .create("region");
  const char *GetScopeSnapshotsFunction =
      "executeFunction_SendObjectWhichCantBeDeserialized";

  auto functionService = FunctionService::onRegion(region);

  ASSERT_THROW(functionService.execute(GetScopeSnapshotsFunction),
               apache::geode::client::MessageException);
  cache.close();
}

const std::vector<std::string> serverResultsToStrings(
    std::shared_ptr<CacheableVector> serverResults) {
  std::vector<std::string> resultList;
  for (auto result : *serverResults) {
    auto resultArray = std::dynamic_pointer_cast<CacheableArrayList>(result);
    for (decltype(resultArray->size()) i = 0; i < resultArray->size(); i++) {
      auto value =
          std::dynamic_pointer_cast<CacheableString>(resultArray->at(i));
      if (value != nullptr) {
        resultList.push_back(value->toString());
      }
    }
  }

  return resultList;
}
TEST(FunctionExecutionTest, OnServersWithReplicatedRegionsInPool) {
  Cluster cluster{
      LocatorCount{1}, ServerCount{2},
      CacheXMLFiles(
          {std::string(getFrameworkString(FrameworkVariable::TestCacheXmlDir)) +
               "/func_cacheserver1_pool.xml",
           std::string(getFrameworkString(FrameworkVariable::TestCacheXmlDir)) +
               "/func_cacheserver2_pool.xml"})};

  cluster.start([&]() {
    cluster.getGfsh()
        .deploy()
        .jar(getFrameworkString(FrameworkVariable::JavaObjectJarPath))
        .execute();
  });

  auto cache = CacheFactory().set("log-level", "none").create();
  auto poolFactory = cache.getPoolManager().createFactory();

  cluster.applyLocators(poolFactory);

  auto pool = poolFactory.create("pool");

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("pool")
                    .create("partition_region");

  for (int i = 0; i < ON_SERVERS_TEST_REGION_ENTRIES_SIZE; i++) {
    region->put("KEY--" + std::to_string(i), "VALUE--" + std::to_string(i));
  }

  // Filter on odd keys
  auto routingObj = CacheableVector::create();
  for (int i = 0; i < ON_SERVERS_TEST_REGION_ENTRIES_SIZE; i++) {
    if (i % 2 == 0) {
      continue;
    }
    routingObj->push_back(CacheableString::create("KEY--" + std::to_string(i)));
  }

  auto execution = FunctionService::onServers(pool);

  auto rc = execution.withArgs(routingObj).execute("MultiGetFunctionI");
  auto executeFunctionResult = rc->getResult();

  // Executed on 2 servers, we should have two sets of results
  ASSERT_EQ(executeFunctionResult->size(), 2);

  auto resultList = serverResultsToStrings(executeFunctionResult);

  // We filtered on odd keys, we should have 1/2 as many results in each set,
  // for a total of ON_SERVERS_TEST_REGION_ENTRIES_SIZE results
  ASSERT_EQ(resultList.size(), ON_SERVERS_TEST_REGION_ENTRIES_SIZE);

  for (decltype(resultList.size()) i = 0;
       i < ON_SERVERS_TEST_REGION_ENTRIES_SIZE / 2; i++) {
    // Each entry in the first result set (first half of this vector) should be
    // equal to its corresponding entry in the second set (2nd half of vector)
    ASSERT_EQ(resultList[i],
              resultList[i + ON_SERVERS_TEST_REGION_ENTRIES_SIZE / 2]);
  }
}

void executeTestFunctionOnLoopAndExpectNotConnectedException(
    std::shared_ptr<apache::geode::client::Pool> pool) {
  // Filter on odd keys
  auto routingObj = CacheableVector::create();
  for (int i = 0; i < ON_SERVERS_TEST_REGION_ENTRIES_SIZE; i++) {
    if (i % 2 == 0) {
      continue;
    }
    routingObj->push_back(CacheableString::create("KEY--" + std::to_string(i)));
  }

  auto execution = FunctionService::onServers(pool);

  ASSERT_THROW(
      {
        while (true) {
          // This call must eventually throw the NotConnectedException
          auto rc =
              execution.withArgs(routingObj).execute("MultiGetFunctionISlow");

          auto executeFunctionResult = rc->getResult();

          auto resultList = serverResultsToStrings(executeFunctionResult);

          // Executed on 2 servers, we should have two sets of results
          ASSERT_EQ(resultList.size(), ON_SERVERS_TEST_REGION_ENTRIES_SIZE);
        }
      },
      apache::geode::client::NotConnectedException);
}

TEST(FunctionExecutionTest, OnServersOneServerGoesDown) {
  Cluster cluster{
      LocatorCount{1}, ServerCount{2},
      CacheXMLFiles(
          {std::string(getFrameworkString(FrameworkVariable::TestCacheXmlDir)) +
               "/func_cacheserver1_pool.xml",
           std::string(getFrameworkString(FrameworkVariable::TestCacheXmlDir)) +
               "/func_cacheserver2_pool.xml"})};

  cluster.start([&]() {
    cluster.getGfsh()
        .deploy()
        .jar(getFrameworkString(FrameworkVariable::JavaObjectJarPath))
        .execute();
  });

  auto cache = CacheFactory().set("log-level", "none").create();
  auto poolFactory = cache.getPoolManager().createFactory();

  cluster.applyLocators(poolFactory);

  auto pool =
      poolFactory.setLoadConditioningInterval(std::chrono::milliseconds::zero())
          .setIdleTimeout(std::chrono::milliseconds::zero())
          .create("pool");

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("pool")
                    .create("partition_region");

  for (int i = 0; i < ON_SERVERS_TEST_REGION_ENTRIES_SIZE; i++) {
    region->put("KEY--" + std::to_string(i), "VALUE--" + std::to_string(i));
  }

  auto threadAux = std::make_shared<std::thread>(
      executeTestFunctionOnLoopAndExpectNotConnectedException, pool);

  // Sleep a bit to allow for some successful responses before the exception
  std::this_thread::sleep_for(std::chrono::seconds(5));

  cluster.getServers()[1].stop();

  threadAux->join();
}
