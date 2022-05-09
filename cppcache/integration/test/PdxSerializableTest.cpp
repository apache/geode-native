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

#include <iostream>
#include <memory>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/FunctionService.hpp>
#include <geode/PdxWrapper.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/TypeRegistry.hpp>

#include "DeliveryAddress.hpp"

namespace {

using apache::geode::client::Cache;
using apache::geode::client::Cacheable;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheableVector;
using apache::geode::client::FunctionService;
using apache::geode::client::PdxReader;
using apache::geode::client::PdxSerializable;
using apache::geode::client::PdxWriter;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

using PdxTests::DeliveryAddress;

std::shared_ptr<Region> setupRegion(Cache& cache) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .create("region");

  return region;
}

/**
 * This test perform the following steps:
 * 1. Writes a version 1 entry of DeliveryAddress named 'entry.v1'
 * 2. From another cache, fetches the entry 'entry.v1'
 * 3. From the same cache as step 2, it call function PutDeliveryAddress that
 *    writes a version 2 entry of DeliveryAddress named 'entry.v2'
 * 4. From another cache, try to fetch the entry 'entry.v2' and it should match
 *    the entry written in step 3
 *
 * The purpose of this test is to verify that even if there are 2 versions of
 * the same PdxSerializable class, the right PdxType will be used during
 * (de)serialization
 */
TEST(PdxSerializableTest, testRightPdxTypeForDiffPdxVersions) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
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

  std::shared_ptr<DeliveryAddress> entryV1;
  std::shared_ptr<DeliveryAddress> entryV2;

  {
    auto cache = cluster.createCache();
    auto region = setupRegion(cache);
    cache.getTypeRegistry().registerPdxType(
        DeliveryAddress::createDeserializable);

    DeliveryAddress::setSerializationVersion(DeliveryAddress::VERSION_1);
    entryV1 = std::make_shared<DeliveryAddress>(
        "Some address line", "Some city", "Some country", std::string{},
        std::shared_ptr<CacheableVector>{});

    region->put("entry.v1", entryV1);
  }

  {
    auto cache = cluster.createCache();
    auto region = setupRegion(cache);
    cache.getTypeRegistry().registerPdxType(
        DeliveryAddress::createDeserializable);

    DeliveryAddress::setSerializationVersion(DeliveryAddress::VERSION_1);
    auto entry = region->get("entry.v1");
    EXPECT_TRUE(entry);

    auto address = std::dynamic_pointer_cast<DeliveryAddress>(entry);
    EXPECT_TRUE(address);
    EXPECT_EQ(*entryV1, *address);

    DeliveryAddress::setSerializationVersion(DeliveryAddress::VERSION_2);
    entryV2 = std::make_shared<DeliveryAddress>(
        "Some address line", "Some city", "Some country", "Some instructions",
        std::shared_ptr<CacheableVector>{});

    auto args = CacheableVector::create();
    args->push_back(CacheableString::create("region"));
    args->push_back(CacheableString::create("entry.v2"));
    args->push_back(CacheableString::create("Some address line"));
    args->push_back(CacheableString::create("Some city"));
    args->push_back(CacheableString::create("Some country"));
    args->push_back(CacheableString::create("Some instructions"));

    auto collector = FunctionService::onServer(region->getRegionService())
                      .withArgs(args)
                      .execute("PutDeliveryAddress");
    EXPECT_TRUE(collector);

    auto resultSet = collector->getResult();
    EXPECT_TRUE(resultSet);

    auto result = std::dynamic_pointer_cast<CacheableString>((*resultSet)[0]);
    std::cerr << result->toString() << std::endl;
  }

  {
    auto cache = cluster.createCache();
    auto region = setupRegion(cache);
    cache.getTypeRegistry().registerPdxType(
        DeliveryAddress::createDeserializable);

    auto entry = region->get("entry.v2");
    EXPECT_TRUE(entry);

    auto address = std::dynamic_pointer_cast<DeliveryAddress>(entry);
    EXPECT_TRUE(address);
    EXPECT_EQ(*entryV2, *address);
  }
}

}  // namespace
