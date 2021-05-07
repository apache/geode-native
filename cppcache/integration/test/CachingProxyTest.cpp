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

#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/Serializable.hpp>

#include "framework/Cluster.h"

using namespace apache::geode::client;

namespace CachingProxy {

using apache::geode::client::RegionShortcut;
using apache::geode::client::Serializable;

TEST(CachingProxyTest, InvalidateAndRemove) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  auto cache = cluster.createCache();
  auto region = cache.createRegionFactory(RegionShortcut::CACHING_PROXY)
                    .setPoolName("default")
                    .create("region");

  std::string key("scharles");
  std::string value("Sylvia Charles");

  region->put(key, value);

  auto user = region->get(key);

  //**********************************************************************
  // Test localRemove after localInvalidate
  //**********************************************************************

  region->localInvalidate(key); // Invalidating a key sets its value to null

  // Local value is now null, so can't do localRemove of original value
  bool resultLocalRemove = region->localRemove(key, value);
  ASSERT_FALSE(resultLocalRemove);

  // Can local remove by nullptr
  resultLocalRemove = region->localRemove(
      key, static_cast<std::shared_ptr<Cacheable>>(nullptr));
  ASSERT_TRUE(resultLocalRemove);

  // Still in the server, so get it again
  user = region->get(key);
  ASSERT_EQ(std::dynamic_pointer_cast<CacheableString>(user)->value(), value);

  //**********************************************************************
  // Test remove after invalidate
  //**********************************************************************

  region->invalidate(key);  // Invalidating a key sets its value to null

  // Local and remote value is now null, so can't do remove of original value
  bool resultRemove = region->remove(key, value);
  ASSERT_FALSE(resultRemove);

  // Can remove by nullptr value
  resultRemove =
      region->remove(key, static_cast<std::shared_ptr<Cacheable>>(nullptr));
  ASSERT_TRUE(resultRemove);

  //**********************************************************************
  // Test remove after localInvalidate
  //**********************************************************************

  // First, need to put it back
  region->put(key, value);

  region->localInvalidate(key);  // Invalidating a key sets its value to null

  // Only Local value is now null, so shouldn't be able to remove nullptr value
  resultRemove =
      region->remove(key, static_cast<std::shared_ptr<Cacheable>>(nullptr));
  ASSERT_FALSE(resultRemove);

  // Remove should fail, so should still be in the server
  user = region->get(key);
  ASSERT_EQ(std::dynamic_pointer_cast<CacheableString>(user)->value(), value);

  cache.close();
}
}  // namespace CachingProxy