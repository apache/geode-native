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

#include <VariousPdxTypes.hpp>
#include <future>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <thread>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/TypeRegistry.hpp>

namespace {

using apache::geode::client::Cache;
using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheServerException;
using apache::geode::client::HashMapOfCacheable;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;
using apache::geode::client::TypeFactoryMethodPdx;

using PdxTests::PdxTypes1;
using PdxTests::PdxTypes10;
using PdxTests::PdxTypes2;
using PdxTests::PdxTypes3;
using PdxTests::PdxTypes4;
using PdxTests::PdxTypes5;
using PdxTests::PdxTypes6;
using PdxTests::PdxTypes7;
using PdxTests::PdxTypes8;
using PdxTests::PdxTypes9;

template <int32_t Index, class PdxType>
struct PdxTypesHelper {
  static constexpr int32_t index = Index;
  using type = PdxType;
};

using PdxTypesHelper1 = PdxTypesHelper<1, PdxTypes1>;
using PdxTypesHelper2 = PdxTypesHelper<2, PdxTypes2>;
using PdxTypesHelper3 = PdxTypesHelper<3, PdxTypes3>;
using PdxTypesHelper4 = PdxTypesHelper<4, PdxTypes4>;
using PdxTypesHelper5 = PdxTypesHelper<5, PdxTypes5>;
using PdxTypesHelper6 = PdxTypesHelper<6, PdxTypes6>;
using PdxTypesHelper7 = PdxTypesHelper<7, PdxTypes7>;
using PdxTypesHelper8 = PdxTypesHelper<8, PdxTypes8>;
using PdxTypesHelper9 = PdxTypesHelper<9, PdxTypes9>;
using PdxTypesHelper10 = PdxTypesHelper<10, PdxTypes10>;

std::shared_ptr<Region> setupRegion(Cache &cache) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .create("region");

  return region;
}

void registerPdxTypes(Cache &cache,
                      const std::initializer_list<TypeFactoryMethodPdx>
                          &typeFactoryMethodPdxList) {
  auto &pdxTypeRegistry = cache.getTypeRegistry();

  for (const auto &typeFactoryMethodPdx : typeFactoryMethodPdxList) {
    pdxTypeRegistry.registerPdxType(typeFactoryMethodPdx);
  }
}

void setupPdxTypes(Cache &cache) {
  registerPdxTypes(
      cache,
      {PdxTypes1::createDeserializable, PdxTypes2::createDeserializable,
       PdxTypes3::createDeserializable, PdxTypes4::createDeserializable,
       PdxTypes5::createDeserializable, PdxTypes6::createDeserializable,
       PdxTypes7::createDeserializable, PdxTypes8::createDeserializable,
       PdxTypes9::createDeserializable, PdxTypes10::createDeserializable});
}

HashMapOfCacheable setupMap() {
  HashMapOfCacheable map;
  map.emplace(CacheableInt32::create(PdxTypesHelper1::index),
              std::make_shared<PdxTypesHelper1::type>());
  map.emplace(CacheableInt32::create(PdxTypesHelper2::index),
              std::make_shared<PdxTypesHelper2::type>());
  map.emplace(CacheableInt32::create(PdxTypesHelper3::index),
              std::make_shared<PdxTypesHelper3::type>());
  map.emplace(CacheableInt32::create(PdxTypesHelper4::index),
              std::make_shared<PdxTypesHelper4::type>());
  map.emplace(CacheableInt32::create(PdxTypesHelper5::index),
              std::make_shared<PdxTypesHelper5::type>());
  map.emplace(CacheableInt32::create(PdxTypesHelper6::index),
              std::make_shared<PdxTypesHelper6::type>());
  map.emplace(CacheableInt32::create(PdxTypesHelper7::index),
              std::make_shared<PdxTypesHelper7::type>());
  map.emplace(CacheableInt32::create(PdxTypesHelper8::index),
              std::make_shared<PdxTypesHelper8::type>());
  map.emplace(CacheableInt32::create(PdxTypesHelper9::index),
              std::make_shared<PdxTypesHelper9::type>());
  map.emplace(CacheableInt32::create(PdxTypesHelper10::index),
              std::make_shared<PdxTypesHelper10::type>());
  return map;
}

template <class T>
void assert_eq(HashMapOfCacheable &expected, HashMapOfCacheable &actual) {
  const auto key = CacheableInt32::create(T::index);
  ASSERT_TRUE(
      std::dynamic_pointer_cast<typename T::type>(expected[key])
          ->equals(std::dynamic_pointer_cast<typename T::type>(actual[key])))
      << "Expected key " << key->value() << " with value of type "
      << typeid(typename T::type).name() << " to be equal.";
}

void assert_eq(HashMapOfCacheable &expected, HashMapOfCacheable &actual) {
  ASSERT_EQ(expected.size(), actual.size());

  assert_eq<PdxTypesHelper1>(expected, actual);
  assert_eq<PdxTypesHelper2>(expected, actual);
  assert_eq<PdxTypesHelper3>(expected, actual);
  assert_eq<PdxTypesHelper4>(expected, actual);
  assert_eq<PdxTypesHelper5>(expected, actual);
  assert_eq<PdxTypesHelper6>(expected, actual);
  assert_eq<PdxTypesHelper7>(expected, actual);
  assert_eq<PdxTypesHelper8>(expected, actual);
  assert_eq<PdxTypesHelper9>(expected, actual);
  assert_eq<PdxTypesHelper10>(expected, actual);
}

template <class Key, class... Tail>
std::vector<Key> to_keys(std::unordered_map<Key, Tail...> map) {
  std::vector<Key> keys;
  keys.reserve(map.size());
  for (const auto &entry : map) {
    keys.push_back(entry.first);
  }
  return keys;
}

template <class Container>
void localDestroy(Region &region, const Container keys) {
  for (const auto &key : keys) {
    region.localDestroy(key);
  }
}

/**
 * Port of testThinClientPutGetAll
 */
TEST(RegionPutGetAllTest, variousPdxTypes) {
  Cluster cluster{LocatorCount{1}, ServerCount{2}};
  cluster.start();
  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = cluster.createCache();
  auto region = setupRegion(cache);

  setupPdxTypes(cache);

  auto putAllMap = setupMap();
  auto keys = to_keys(putAllMap);

  // TODO - Understand: Put one tests putAll doesn't fail for existing entry?
  region->put(putAllMap.cbegin()->first, putAllMap.cbegin()->second);

  region->putAll(putAllMap);

  // TODO - Understand: Clear local cache to force fetch from server?
  localDestroy(*region, keys);

  auto getAllMap = region->getAll(keys);

  assert_eq(putAllMap, getAllMap);
}

TEST(RegionPutGetAllTest, nullValue) {
  Cluster cluster{LocatorCount{1}, ServerCount{2}};
  cluster.start();
  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = cluster.createCache();
  auto region = setupRegion(cache);

  setupPdxTypes(cache);

  HashMapOfCacheable map;

  // Add a null value
  map.emplace(CacheableInt32::create(PdxTypesHelper1::index),
              std::shared_ptr<PdxTypesHelper1::type>());

  auto keys = to_keys(map);

  ASSERT_THROW(region->putAll(map), CacheServerException);
}

}  // namespace
