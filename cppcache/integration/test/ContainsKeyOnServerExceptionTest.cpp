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
#include <framework/TestConfig.h>

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/CacheTransactionManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

namespace {
using apache::geode::client::Cache;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheTransactionManager;
using apache::geode::client::Pool;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

std::string getClientLogName() {
  std::string testSuiteName(::testing::UnitTest::GetInstance()
                                ->current_test_info()
                                ->test_suite_name());
  std::string testCaseName(
      ::testing::UnitTest::GetInstance()->current_test_info()->name());
  std::string logFileName(testSuiteName + "/" + testCaseName + "/client.log");
  return logFileName;
}

std::shared_ptr<Cache> createCache() {
  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("log-file", getClientLogName())
                   .create();
  return std::make_shared<Cache>(std::move(cache));
}

TEST(ContainsKeyOnServerExceptionTest, handleException) {
  Cluster cluster{
      LocatorCount{1}, ServerCount{3}, ConserveSockets(true),
      CacheXMLFiles({
          std::string(getFrameworkString(FrameworkVariable::TestCacheXmlDir)) +
              "/cacheserver1_fpr_transaction.xml",
          std::string(getFrameworkString(FrameworkVariable::TestCacheXmlDir)) +
              "/cacheserver2_fpr_transaction.xml",
          std::string(getFrameworkString(FrameworkVariable::TestCacheXmlDir)) +
              "/cacheserver3_fpr_transaction.xml",
      })};
  cluster.start([&]() {
    cluster.getGfsh()
        .deploy()
        .jar(getFrameworkString(FrameworkVariable::JavaObjectJarPath))
        .execute();
  });

  auto cache = createCache();
  auto poolFactory = cache->getPoolManager().createFactory();
  ServerAddress serverAddress = cluster.getServers()[0].getAddress();
  serverAddress.port = 40401;
  cluster.applyServer(poolFactory, serverAddress);
  auto pool = poolFactory.create("default");
  auto region = cache->createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .create("region");

  auto transactionManager = cache->getCacheTransactionManager();

  // this key will be always routed towards server[1]
  int theKey = 7;
  std::string theValue = "theValue";
  try {
    transactionManager->begin();
    region->put(theKey, theValue);
    transactionManager->commit();
  } catch (...) {
    EXPECT_THROW(
        region->containsKeyOnServer(CacheableKey::create(7)),
        apache::geode::client::TransactionDataNodeHasDepartedException);
  }

  EXPECT_FALSE(region->containsKey(7));
}

}  // namespace
