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

#include <list>
#include <thread>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/Serializable.hpp>

#include "framework/Cluster.h"

using apache::geode::client::Cache;
using apache::geode::client::Cacheable;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;
using apache::geode::client::Serializable;

namespace CachingProxy {

class CachingProxyTest : public ::testing::Test {
 protected:
  CachingProxyTest() {
    cluster.start();

    cluster.getGfsh()
        .create()
        .region()
        .withName("region")
        .withType("PARTITION")
        .execute();
  }

  ~CachingProxyTest() override = default;

  void SetUp() override {
    cache.getPoolManager()
        .createFactory()
        .addLocator("localhost", cluster.getLocatorPort())
        .create("pool");

    region = cache.createRegionFactory(RegionShortcut::CACHING_PROXY)
                 .setPoolName("pool")
                 .create("region");
  }

  void TearDown() override { cache.close(); }

  Cluster cluster = Cluster{LocatorCount{1}, ServerCount{1}};
  Cache cache = CacheFactory().create();
  std::string key = std::string("scharles");
  std::string value = std::string("Sylvia Charles");
  std::shared_ptr<Region> region;
};

TEST_F(CachingProxyTest, LocalRemoveAfterLocalInvalidate) {
  region->put(key, value);

  auto user = region->get(key);

  region->localInvalidate(key);

  auto resultLocalRemove = region->localRemove(key, value);
  ASSERT_FALSE(resultLocalRemove);

  resultLocalRemove = region->localRemove(key, nullptr);
  ASSERT_TRUE(resultLocalRemove);
}

TEST_F(CachingProxyTest, RemoveAfterInvalidate) {
  region->put(key, value);

  region->invalidate(key);

  auto resultRemove = region->remove(key, value);
  ASSERT_FALSE(resultRemove);

  resultRemove = region->remove(key, nullptr);
  ASSERT_TRUE(resultRemove);
}

TEST_F(CachingProxyTest, RemoveAfterLocalInvalidate) {
  region->put(key, value);

  region->localInvalidate(key);

  auto resultRemove = region->remove(key, nullptr);
  ASSERT_FALSE(resultRemove);

  auto user = region->get(key);
  ASSERT_EQ(std::dynamic_pointer_cast<CacheableString>(user)->value(), value);
}

TEST_F(CachingProxyTest, Remove) {
  region->put(key, value);

  auto resultRemove = region->remove(key);
  ASSERT_TRUE(resultRemove);
}

}  // namespace CachingProxy
