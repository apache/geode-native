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
#include <gmock/gmock.h>

#include <memory>
#include <random>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/CacheableDate.hpp>
#include <geode/EntryEvent.hpp>
#include <geode/PdxWrapper.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionEvent.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "testobject/PdxType.hpp"

namespace {

using apache::geode::client::Cache;
using apache::geode::client::CacheableDate;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheRegionHelper;
using apache::geode::client::PdxReader;
using apache::geode::client::PdxSerializable;
using apache::geode::client::Region;
using apache::geode::client::RegionEvent;
using apache::geode::client::RegionShortcut;

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;

using PdxTests::DynamicFieldsType;

namespace {
constexpr auto DYNAMIC_FIELDS_TYPE_MODIFIERS_COUNT = 22;
std::function<void(DynamicFieldsType&)>
    DYNAMIC_FIELDS_TYPE_MODIFIERS[DYNAMIC_FIELDS_TYPE_MODIFIERS_COUNT] = {
        [](DynamicFieldsType& obj) { obj.setChar('a'); },
        [](DynamicFieldsType& obj) { obj.setBoolean(false); },
        [](DynamicFieldsType& obj) { obj.setByte(1); },
        [](DynamicFieldsType& obj) { obj.setShort(2); },
        [](DynamicFieldsType& obj) { obj.setInt(3); },
        [](DynamicFieldsType& obj) { obj.setLong(5); },
        [](DynamicFieldsType& obj) { obj.setFloat(8.f); },
        [](DynamicFieldsType& obj) { obj.setDouble(13.); },
        [](DynamicFieldsType& obj) { obj.setString("Test string"); },
        [](DynamicFieldsType& obj) {
          obj.setDate(CacheableDate::create(CacheableDate::clock::now()));
        },
        [](DynamicFieldsType& obj) {
          obj.setDate(CacheableDate::create(CacheableDate::clock::now()));
        },
        [](DynamicFieldsType& obj) {
          obj.setObject(CacheableString::create("An object string"));
        },
        [](DynamicFieldsType& obj) {
          obj.setCharArray({'a', 'b', 'c'});
        },
        [](DynamicFieldsType& obj) {
          obj.setBooleanArray({false, true, false});
        },
        [](DynamicFieldsType& obj) {
          obj.setByteArray({21, 34, 55});
        },
        [](DynamicFieldsType& obj) {
          obj.setByteArray({21, 34, 55});
        },
        [](DynamicFieldsType& obj) {
          obj.setShortArray({89, 144, 233});
        },
        [](DynamicFieldsType& obj) {
          obj.setIntArray({377, 610, 987});
        },
        [](DynamicFieldsType& obj) {
          obj.setLongArray({1597, 2584, 4181});
        },
        [](DynamicFieldsType& obj) {
          obj.setFloatArray({7402.f, 11583.f, 18985.f});
        },
        [](DynamicFieldsType& obj) {
          obj.setDoubleArray({30568., 49553., 80121.});
        },
        [](DynamicFieldsType& obj) {
          obj.setStringArray({"A test string", "Another test string",
                              "Yet another test string"});
        },
};
}  // namespace

Cache createTestCache() {
  CacheFactory cacheFactory;
  return cacheFactory.set("log-level", "none")
      .set("statistic-sampling-enabled", "false")
      .create();
}

TEST(PdxSerializableTest, deserializeNewFields) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.start();

  auto& gfsh = cluster.getGfsh();
  gfsh.create().region().withName("region").withType("REPLICATE").execute();

  auto cache = createTestCache();
  {
    auto poolFactory = cache.getPoolManager().createFactory();
    cluster.applyLocators(poolFactory);
    poolFactory.create("default");
  }

  auto region = cache.createRegionFactory(RegionShortcut::CACHING_PROXY)
                    .setPoolName("default")
                    .create("region");

  cache.getTypeRegistry().registerPdxType(
      DynamicFieldsType::createDeserializable);

  auto entry = std::make_shared<DynamicFieldsType>(2, "Line 1", "Line 2");
  region->put("1", entry);

  auto entryFetched = region->get("1");
  ASSERT_TRUE(entryFetched);

  auto address = std::dynamic_pointer_cast<DynamicFieldsType>(entryFetched);
  ASSERT_TRUE(address);

  ASSERT_EQ(*entry, *address);

  // This mechanism guarantees that a different set of fields
  // are introduced each execution in order ensure a proper behaviour
  std::mt19937 generator{std::random_device{}()};
  std::uniform_int_distribution<uint32_t> distribution(
      1, 1 << DYNAMIC_FIELDS_TYPE_MODIFIERS_COUNT);

  entry = std::make_shared<DynamicFieldsType>(3, "Line 1", "Line 2");
  auto mask = distribution(generator);
  for (auto i = 0; i < DYNAMIC_FIELDS_TYPE_MODIFIERS_COUNT; ++i) {
    if ((mask & (1ULL << i)) != 0) {
      DYNAMIC_FIELDS_TYPE_MODIFIERS[i](*entry);
    }
  }

  region->put("2", entry);

  entryFetched = region->get("2");
  ASSERT_TRUE(entryFetched);

  address = std::dynamic_pointer_cast<DynamicFieldsType>(entryFetched);
  ASSERT_TRUE(address);

  ASSERT_EQ(*entry, *address);
}

}  // namespace