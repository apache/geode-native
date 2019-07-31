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
using apache::geode::client::CacheableString;
using apache::geode::client::CacheableVector;
using apache::geode::client::CacheFactory;
using apache::geode::client::FunctionExecutionException;
using apache::geode::client::FunctionService;
using apache::geode::client::NotConnectedException;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;
using apache::geode::client::ResultCollector;

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
     Disabled_FunctionReturnsObjectWhichCantBeDeserializedOnServer) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
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
  //  CacheFactory().set("log-level", "none").create();
  // auto pool = cache.getPoolManager()
  //                .createFactory()
  //                .addLocator("localhost", 10334)
  //                .create("pool");
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .create("region");

  const char *GetScopeSnapshotsFunction =
      "executeFunction_SendObjectWhichCantBeDeserialized";
  auto functionService = FunctionService::onRegion(region);
  ASSERT_THROW(functionService.execute(GetScopeSnapshotsFunction),
               apache::geode::client::IllegalStateException);

  cache.close();
}

TEST(FunctionExecutionTest, OnServersWithReplicatedRegionsInPool) {
  Cluster cluster{
      LocatorCount{1}, ServerCount{2},
      CacheXMLFiles({"func_cacheserver1_pool.xml", "func_cacheserver2_pool"})
      // CacheXMLFile { "func_cacheserver1_pool.xml" }
  };

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

  auto cache = CacheFactory().set("log-level", "none").create();
  auto poolFactory = cache.getPoolManager().createFactory();

  cluster.applyLocators(poolFactory);

  auto pool = poolFactory.create("pool");

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("pool")
                    .create("region");

  //**********************

  // test data independant fucntion execution on all servers

  // std::vector<std::string> routingObj;
  // std::string[] routingObj = new std::string[]();
  // int j = 0;
  // for (int i = 0; i < 34; i++) {
  //  if (i % 2 == 0) continue;
  //  routingObj->at(j) = "KEY--" + i;
  //  j++;
  //}

  auto routingObj = CacheableVector::create();
  for (int i = 0; i < 34; i++) {
    if (i % 2 == 0) continue;
    routingObj->push_back(CacheableString::create("KEY--" + i));
  }

  auto execution = FunctionService::onServers(pool);

  auto rc = execution.withArgs(routingObj).execute("MultiGetFunctionI");
  auto executeFunctionResult = rc->getResult();

  ASSERT_EQ(executeFunctionResult->size(), 2);

  std::vector<std::string> resultList;
  // for (CacheableVector result: *executeFunctionResult)
  //  for (auto result2: result) { resultList.Add(item2); }
  //}
  // Util.Log("on all servers: result count= {0}.", resultList.Count);
  // Assert.IsTrue(resultList.Count == 34, "result count check failed");
  // for (int i = 0; i < resultList.Count; i++) {
  //  Util.Log("on all servers: get:result[{0}]={1}.", i,
  //(string)resultList[i]);
  //}
  //// TODO::enable it once the StringArray conversion is fixed.
  //// test withCollector
  // MyResultCollector<object> myRC = new MyResultCollector<object>();
  // rc =
  // exc.WithArgs<ArrayList>(args1).WithCollector(myRC).Execute(getFuncIName);
  //// executeFunctionResult = rc.GetResult();
  // Util.Log("add result count= {0}.", myRC.GetAddResultCount());
  // Util.Log("get result count= {0}.", myRC.GetGetResultCount());
  // Util.Log("end result count= {0}.", myRC.GetEndResultCount());
  // Util.Log("on all servers with collector: result count= {0}.",
  //         executeFunctionResult.Count);
  // Assert.IsTrue(myRC.GetResult().Count == 2, "result count check failed");

  // IList res = (IList)myRC.GetResult();

  // foreach (object o in res) {
  //  IList resList = (IList)o;
  //  Util.Log("results " + resList.Count);

  //  Assert.AreEqual(17, resList.Count);
  //}

  // MyResultCollector<object> myRC2 = new MyResultCollector<object>();
  // rc = exc.WithArgs<object>(args).WithCollector(myRC2).Execute(
  //    exFuncNameSendException);
  // executeFunctionResult = rc.GetResult();
  // Util.Log("add result count= {0}.", myRC2.GetAddResultCount());
  // Util.Log("get result count= {0}.", myRC2.GetGetResultCount());
  // Util.Log("end result count= {0}.", myRC2.GetEndResultCount());
  // Assert.IsTrue(myRC2.GetAddResultCount() == 2,
  //              "add result count check failed");
  // Assert.IsTrue(myRC2.GetGetResultCount() == 1,
  //              "get result count check failed");
  // Assert.IsTrue(myRC2.GetEndResultCount() == 1,
  //              "end result count check failed");
  // Util.Log("on Region with collector: result count= {0}.",
  //         executeFunctionResult.Count);
  // Assert.IsTrue(executeFunctionResult.Count == 2, "result count check
  // failed");
  // foreach (object item in executeFunctionResult) {
  //  Util.Log("on Region with collector: get:result {0}",
  //           (item as UserFunctionExecutionException).Message);
  //}
}
