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
#include <framework/Gfsh.h>

#include <future>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <thread>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/FunctionService.hpp>
#include <geode/PdxInstanceFactory.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/TypeRegistry.hpp>

#include "CacheRegionHelper.hpp"
#include "LocalRegion.hpp"
#include "NestedPdxObject.hpp"
#include "PdxType.hpp"

namespace {
using apache::geode::client::Cache;
using apache::geode::client::CacheableArrayList;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheRegionHelper;
using apache::geode::client::CacheServerException;
using apache::geode::client::FunctionService;
using apache::geode::client::IllegalStateException;
using apache::geode::client::LocalRegion;
using apache::geode::client::PdxFieldTypes;
using apache::geode::client::PdxInstance;
using apache::geode::client::PdxInstanceFactory;
using apache::geode::client::PdxSerializable;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

using PdxTests::Address;
using PdxTests::PdxType;

using testobject::ChildPdx;
using testobject::ParentPdx;

const std::string gemfireJsonClassName = "__GEMFIRE_JSON";

std::shared_ptr<Region> setupRegion(Cache& cache) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .create("region");

  return region;
}

TEST(PdxJsonTypeTest, testGfshQueryJsonInstances) {
  Cluster cluster{LocatorCount{1}, ServerCount{1},
                  CacheXMLFiles{std::vector<std::string>{
                      std::string(getFrameworkString(
                          FrameworkVariable::NewTestResourcesDir)) +
                      "/pdxjson_cacheserver.xml"}}};

  cluster.start([&cluster]() {
    cluster.getGfsh()
        .deploy()
        .jar(getFrameworkString(FrameworkVariable::JavaObjectJarPath))
        .execute();
  });

  auto cache = cluster.createCache();
  auto region = setupRegion(cache);
  auto execution = FunctionService::onRegion(region);
  auto query = CacheableString::create("SELECT * FROM /region");

  region->put("non-java-domain-class-entry",
              cache.createPdxInstanceFactory(gemfireJsonClassName, false)
                  .writeString("entryName", "non-java-domain-class-entry")
                  .create());
  ASSERT_NO_THROW(execution.withArgs(query).execute("QueryFunction"));

  region->put("java-domain-class-entry",
              cache.createPdxInstanceFactory(gemfireJsonClassName)
                  .writeString("entryName", "java-domain-class-entry")
                  .create());
  ASSERT_NO_THROW(execution.withArgs(query).execute("QueryFunction"));
}

TEST(PdxJsonTypeTest, testCreateTwoJsonInstances) {
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
  auto pdxInstanceFactory =
      cache.createPdxInstanceFactory(gemfireJsonClassName);

  pdxInstanceFactory.writeString("foo", "bar");
  auto pdxInstance = pdxInstanceFactory.create();

  region->put("simpleObject", pdxInstance);

  auto retrievedValue = region->get("simpleObject");

  pdxInstance = std::dynamic_pointer_cast<PdxInstance>(retrievedValue);

  EXPECT_FALSE(pdxInstance == nullptr);
  EXPECT_TRUE(pdxInstance->hasField("foo"));
  EXPECT_EQ(pdxInstance->getFieldType("foo"), PdxFieldTypes::STRING);
  EXPECT_EQ(pdxInstance->getStringField("foo"), std::string{"bar"});

  auto pdxInstanceFactory2 =
      cache.createPdxInstanceFactory(gemfireJsonClassName);
  pdxInstanceFactory2.writeInt("baz", 42);
  pdxInstance = pdxInstanceFactory2.create();

  region->put("anotherSimpleObject", pdxInstance);
  retrievedValue = region->get("anotherSimpleObject");

  pdxInstance = std::dynamic_pointer_cast<PdxInstance>(retrievedValue);

  EXPECT_FALSE(pdxInstance == nullptr);
  EXPECT_TRUE(pdxInstance->hasField("baz"));
  EXPECT_EQ(pdxInstance->getFieldType("baz"), PdxFieldTypes::INT);
  EXPECT_EQ(pdxInstance->getIntField("baz"), 42);
}

TEST(PdxJsonTypeTest, testTwoConsecutiveGets) {
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
  auto pdxInstanceFactory =
      cache.createPdxInstanceFactory(gemfireJsonClassName);

  pdxInstanceFactory.writeString("foo", "bar");
  auto pdxInstance = pdxInstanceFactory.create();

  region->put("simpleObject", pdxInstance);

  auto cache2 = cluster.createCache();
  auto region2 = setupRegion(cache2);

  region2->get("simpleObject");  // Throw the first one away

  EXPECT_TRUE(
      std::dynamic_pointer_cast<PdxInstance>(region2->get("simpleObject")));
}

}  // namespace
