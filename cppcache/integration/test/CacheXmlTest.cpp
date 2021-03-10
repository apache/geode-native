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
#include <framework/TestConfig.h>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/PartitionResolver.hpp>
#include <geode/PoolManager.hpp>
#include <geode/Region.hpp>

#include "cpp-integration-test_export.h"

namespace {

using apache::geode::client::Cache;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheXmlException;
using apache::geode::client::EntryEvent;
using apache::geode::client::PartitionResolver;

apache::geode::client::Cache createCacheUsingXmlConfig(
    const std::string& xmlFile) {
  using apache::geode::client::CacheFactory;

  CacheFactory cacheFactory;

  auto cache = cacheFactory.set("log-level", "none")
                   .set("statistic-sampling-enabled", "false")
                   .set("cache-xml-file", xmlFile)
                   .create();

  return cache;
}

class TestAppPartitionResolver : public PartitionResolver {
 public:
  const std::string& getName() override {
    static std::string name = "TestAppPartitionResolver";
    return name;
  }

  std::shared_ptr<CacheableKey> getRoutingObject(const EntryEvent&) override {
    return {};
  }
};
}  // namespace

extern "C" CPP_INTEGRATION_TEST_EXPORT PartitionResolver*
CacheXmlTest_createAppPartitionResolver() {
  return new TestAppPartitionResolver{};
}

/**
 * Example test using 2 servers and waiting for async tasks to synchronize using
 * furtures.
 */
TEST(CacheXmlTest, loadCacheXml) {
  auto cacheXml =
      std::string(getFrameworkString(FrameworkVariable::TestCacheXmlDir)) +
      "/valid_cache_refid.xml";
  auto cache = createCacheUsingXmlConfig(cacheXml);
}

/**
 * Example test using 2 servers and waiting for async tasks to synchronize using
 * furtures.
 */
TEST(CacheXmlTest, loadCacheXmlWithBadSchema) {
  auto cacheXml =
      std::string(getFrameworkString(FrameworkVariable::TestCacheXmlDir)) +
      "/bad_schema.xml";
  EXPECT_NO_THROW(createCacheUsingXmlConfig(cacheXml));
}

TEST(CacheXmlTest, loadCacheWithUnnamedPool) {
  Cluster cluster{LocatorCount{1}, ServerCount{2}};
  auto cacheXml =
      std::string(getFrameworkString(FrameworkVariable::TestCacheXmlDir)) +
      "/unnamed_pool.xml";

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();
  EXPECT_THROW(createCacheUsingXmlConfig(cacheXml), CacheXmlException);
}

TEST(CacheXmlTest, testApplicationPartitionResolver) {
  auto cache_xml = getFrameworkString(FrameworkVariable::NewTestResourcesDir) +
                   std::string{"/pr_app_client_cache.xml"};
  auto cache = createCacheUsingXmlConfig(cache_xml);

  auto region = cache.getRegion("region");
  auto pr = region->getAttributes().getPartitionResolver();

  EXPECT_TRUE(pr);
  EXPECT_EQ(pr->getName(), "TestAppPartitionResolver");
}

TEST(CacheXmlTest, testSharedLibPartitionResolver) {
  auto cache_xml = getFrameworkString(FrameworkVariable::NewTestResourcesDir) +
                   std::string{"/pr_lib_client_cache.xml"};
  auto cache = createCacheUsingXmlConfig(cache_xml);

  auto region = cache.getRegion("region");
  auto pr = region->getAttributes().getPartitionResolver();

  EXPECT_TRUE(pr);
  EXPECT_EQ(pr->getName(), "TestLibPartitionResolver");
}
