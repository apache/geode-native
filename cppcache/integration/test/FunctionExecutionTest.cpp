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
#include <CacheImpl.hpp>
#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/CacheableBuiltins.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/FunctionService.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "CacheRegionHelper.hpp"
#include "framework/Cluster.h"
#include "framework/Gfsh.h"
#include "framework/TestConfig.h"

using apache::geode::client::Cache;
using apache::geode::client::Cacheable;
using apache::geode::client::CacheableArrayList;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheableVector;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheImpl;
using apache::geode::client::CacheRegionHelper;
using apache::geode::client::Exception;
using apache::geode::client::FunctionException;
using apache::geode::client::FunctionService;
using apache::geode::client::NotConnectedException;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;
using apache::geode::client::ResultCollector;

using ::testing::Eq;
using ::testing::Le;

const int ON_SERVERS_TEST_REGION_ENTRIES_SIZE = 34;
const int PARTITION_REGION_ENTRIES_SIZE = 113;

std::shared_ptr<Region> setupRegion(Cache &cache) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .create("region");

  return region;
}

class TestResultCollector : public ResultCollector {
 private:
  std::shared_ptr<CacheableVector> resultList;

 public:
  TestResultCollector() : resultList(CacheableVector::create()) {}

  virtual std::shared_ptr<CacheableVector> getResult(
      std::chrono::milliseconds) override {
    return resultList;
  }

  virtual void addResult(const std::shared_ptr<Cacheable> &result) override {
    resultList->push_back(result);
  }

  virtual void endResults() override {}

  // Do not clear in order to detect duplicate results
  virtual void clearResults() override {
    throw Exception(
        "Clear should not be triggered when Function.isHa is set to false");
  }
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
               FunctionException);
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
               FunctionException);
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
               FunctionException);
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
               FunctionException);
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

void populateRegion(const std::shared_ptr<Region> &region) {
  for (int i = 0; i < PARTITION_REGION_ENTRIES_SIZE; i++) {
    region->put("KEY--" + std::to_string(i), "VALUE--" + std::to_string(i));
  }
}

void waitUntilPRMetadataIsRefreshed(CacheImpl *cacheImpl) {
  auto end = std::chrono::system_clock::now() + std::chrono::minutes(2);
  while (!cacheImpl->getAndResetPrMetadataUpdatedFlag()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ASSERT_FALSE(std::chrono::system_clock::now() > end);
  }
}

TEST(FunctionExecutionTest, FunctionExecutionWithIncompleteBucketLocations) {
  Cluster cluster{
      InitialLocators{{{"localhost", Framework::getAvailablePort()}}},
      InitialServers{{{"localhost", Framework::getAvailablePort()},
                      {"localhost", Framework::getAvailablePort()},
                      {"localhost", Framework::getAvailablePort()}}}};

  cluster.start([&]() {
    cluster.getGfsh()
        .deploy()
        .jar(getFrameworkString(FrameworkVariable::JavaObjectJarPath))
        .execute();
  });

  cluster.getGfsh()
      .create()
      .region()
      .withName("partition_region")
      .withType("PARTITION")
      .execute();

  auto cache = CacheFactory().create();
  auto poolFactory = cache.getPoolManager().createFactory();

  ServerAddress serverAddress = cluster.getServers()[2].getAddress();
  cluster.applyServer(poolFactory, serverAddress);

  auto pool =
      poolFactory.setPRSingleHopEnabled(true).setRetryAttempts(0).create(
          "pool");

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("pool")
                    .create("partition_region");

  // Populate region in a way that not all buckets are created.
  // Servers in this case will create 88 of possible 113 buckets.
  populateRegion(region);

  // Check that PR metadata is updated. This is done to be sure
  // that client will execute function in a non single hop manner
  // because metadata doesn't contain all bucket locations.
  // After metadata is refreshed, it will contain at least one
  // bucket location.
  CacheImpl *cacheImpl = CacheRegionHelper::getCacheImpl(&cache);
  waitUntilPRMetadataIsRefreshed(cacheImpl);

  auto functionService = FunctionService::onRegion(region);
  auto rc =
      functionService.withCollector(std::make_shared<TestResultCollector>())
          .execute("MultiGetAllFunctionNonHA");

  std::shared_ptr<TestResultCollector> resultCollector =
      std::dynamic_pointer_cast<TestResultCollector>(rc);

  // check that PR metadata is updated after function is executed
  waitUntilPRMetadataIsRefreshed(cacheImpl);

  // check that correct nubmer of events is received in function result
  auto result = rc->getResult(std::chrono::milliseconds(0));
  ASSERT_EQ(result->size(), PARTITION_REGION_ENTRIES_SIZE);

  cache.close();
}

std::shared_ptr<CacheableVector> populateRegionReturnFilter(
    const std::shared_ptr<Region> &region, const int numberOfPuts) {
  auto routingObj = CacheableVector::create();
  for (int i = 0; i < numberOfPuts; i++) {
    region->put("KEY--" + std::to_string(i), "VALUE--" + std::to_string(i));
    routingObj->push_back(CacheableKey::create("KEY--" + std::to_string(i)));
  }
  return routingObj;
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

TEST(FunctionExecutionTest, testThatFunctionExecutionThrowsExceptionNonHA) {
  Cluster cluster{
      InitialLocators{{{"localhost", Framework::getAvailablePort()}}},
      InitialServers{{{"localhost", Framework::getAvailablePort()},
                      {"localhost", Framework::getAvailablePort()},
                      {"localhost", Framework::getAvailablePort()}}}};

  cluster.start([&]() {
    cluster.getGfsh()
        .deploy()
        .jar(getFrameworkString(FrameworkVariable::JavaObjectJarPath))
        .execute();
  });

  cluster.getGfsh()
      .create()
      .region()
      .withName("partition_region")
      .withType("PARTITION")
      .execute();

  cluster.getServers()[2].stop();

  auto cache = CacheFactory().create();
  auto poolFactory = cache.getPoolManager().createFactory();

  ServerAddress serverAddress = cluster.getServers()[1].getAddress();
  cluster.applyServer(poolFactory, serverAddress);

  auto pool = poolFactory.setPRSingleHopEnabled(true).create("pool");

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("pool")
                    .create("partition_region");

  populateRegionReturnFilter(region, 1000);
  //  Start the the server
  cluster.getServers()[2].start();

  // Do the rebalance, so that primary buckets
  // are transferred to the newly added server
  cluster.getGfsh().rebalance().execute();

  // InternalFunctionInvocationTargetException will happen
  // on servers, because client will try to execute the single-hop function
  // using old PR metadata (PR metadata before rebalance operation)
  // Client in this case should throw exception. Also client should not trigger
  // ResultCollector::clear results. If this happens then TestResultCollector
  // will throw the exception (Exception) and case will fail.
  bool isExceptionTriggered = false;
  auto functionService = FunctionService::onRegion(region);
  auto execute =
      functionService.withCollector(std::make_shared<TestResultCollector>());
  ASSERT_THROW(execute.execute("MultiGetAllFunctionNonHA"),
               FunctionException);
}

TEST(FunctionExecutionTest,
     testThatFunctionExecutionThrowsExceptionNonHAWithFilter) {
  Cluster cluster{
      InitialLocators{{{"localhost", Framework::getAvailablePort()}}},
      InitialServers{{{"localhost", Framework::getAvailablePort()},
                      {"localhost", Framework::getAvailablePort()},
                      {"localhost", Framework::getAvailablePort()}}}};

  cluster.start([&]() {
    cluster.getGfsh()
        .deploy()
        .jar(getFrameworkString(FrameworkVariable::JavaObjectJarPath))
        .execute();
  });

  cluster.getGfsh()
      .create()
      .region()
      .withName("partition_region")
      .withType("PARTITION")
      .execute();

  cluster.getServers()[2].stop();

  auto cache = CacheFactory().create();
  auto poolFactory = cache.getPoolManager().createFactory();

  ServerAddress serverAddress = cluster.getServers()[1].getAddress();
  cluster.applyServer(poolFactory, serverAddress);

  auto pool = poolFactory.setPRSingleHopEnabled(true).create("pool");

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("pool")
                    .create("partition_region");

  auto filter = populateRegionReturnFilter(region, 1000);

  //  Start the the server
  cluster.getServers()[2].start();

  // Do the rebalance, so that primary buckets
  // are transferred to the newly added server
  cluster.getGfsh().rebalance().execute();

  // InternalFunctionInvocationTargetException will happen
  // on servers, because client will try to execute the single-hop function
  // using old PR metadata (PR metadata before rebalance operation)
  // Client in this case should throw exception. Also client should not trigger
  // ResultCollector::clear results. If this happens then TestResultCollector
  // will throw the exception (Exception) and test case will fail.
  auto functionService = FunctionService::onRegion(region);
  auto execute =
      functionService.withCollector(std::make_shared<TestResultCollector>())
          .withFilter(filter);
  ASSERT_THROW(execute.execute("MultiGetAllFunctionNonHA"),
               FunctionException);
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
    // Each entry in the first result set (first half of this vector) should
    // be equal to its corresponding entry in the second set (2nd half of
    // vector)
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

TEST(FunctionExecutionTest,
     testUserFunctionExceptionThrowsRightException) {
  Cluster cluster{
      InitialLocators{{{"localhost", Framework::getAvailablePort()}}},
      InitialServers{{{"localhost", Framework::getAvailablePort()},
                      {"localhost", Framework::getAvailablePort()},
                      {"localhost", Framework::getAvailablePort()}}}};

  cluster.start([&]() {
    cluster.getGfsh()
        .deploy()
        .jar(getFrameworkString(FrameworkVariable::JavaObjectJarPath))
        .execute();
  });

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = cluster.createCache();
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .create("region");

  auto functionService = FunctionService::onRegion(region);
  auto execute =
      functionService.withCollector(std::make_shared<TestResultCollector>());
  EXPECT_THROW(execute.execute("UserExceptionFunction"), FunctionException);
}