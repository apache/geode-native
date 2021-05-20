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
#include <framework/Gfsh.h>

#include <gtest/gtest.h>

#include "geode/auth_initialize.h"
#include "geode/cache.h"
#include "geode/cache/factory.h"
#include "geode/client.h"
#include "geode/pool.h"
#include "geode/pool/factory.h"
#include "geode/pool/manager.h"
#include "geode/region.h"
#include "geode/region/factory.h"
#include "geode/region/shortcut.h"

auto credentialsRequested_ = 0;

void simpleGetCredentials(apache_geode_properties_t* props) {
  apache_geode_AuthInitialize_AddProperty(props, "security-username", "root");
  apache_geode_AuthInitialize_AddProperty(props, "security-password",
                                          "root-password");
  credentialsRequested_++;
}

void simpleClose() {}

TEST(CAuthInitializeTest, putGetWithBasicAuth) {
  Cluster cluster(
      Name(std::string(::testing::UnitTest::GetInstance()
                           ->current_test_info()
                           ->test_suite_name()) +
           "/" +
           ::testing::UnitTest::GetInstance()->current_test_info()->name()),
      Classpath{getFrameworkString(FrameworkVariable::JavaObjectJarPath)},
      SecurityManager{"javaobject.SimpleSecurityManager"}, User{"root"},
      Password{"root-password"}, LocatorCount{1}, ServerCount{1});

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  auto client = apache_geode_ClientInitialize();
  auto cache_factory = apache_geode_CreateCacheFactory(client);

  apache_geode_CacheFactory_SetProperty(cache_factory,
                                        "statistic-sampling-enabled", "false");
  apache_geode_CacheFactory_SetAuthInitialize(
      cache_factory, simpleGetCredentials, simpleClose);

  auto cache = apache_geode_CacheFactory_CreateCache(cache_factory);

  auto pool_manager = apache_geode_Cache_GetPoolManager(cache);
  auto pool_factory = apache_geode_PoolManager_CreateFactory(pool_manager);

  for (const auto& locator : cluster.getLocators()) {
    apache_geode_PoolFactory_AddLocator(pool_factory,
                                        locator.getAddress().address.c_str(),
                                        locator.getAddress().port);
  }

  auto pool = apache_geode_PoolFactory_CreatePool(pool_factory, "default");

  auto region_factory = apache_geode_Cache_CreateRegionFactory(cache, PROXY);

  apache_geode_RegionFactory_SetPoolName(region_factory, "default");
  auto region =
      apache_geode_RegionFactory_CreateRegion(region_factory, "region");

  apache_geode_Region_PutString(region, "foo", "bar");
  apache_geode_Region_PutString(region, "baz", "qux");

  auto foo_value = apache_geode_Region_GetString(region, "foo");
  ASSERT_STREQ(foo_value, "bar");

  auto baz_value = apache_geode_Region_GetString(region, "baz");
  ASSERT_STREQ(baz_value, "qux");

  ASSERT_GT(credentialsRequested_, 0);

  apache_geode_DestroyRegion(region);
  apache_geode_DestroyRegionFactory(region_factory);
  apache_geode_DestroyPool(pool);
  apache_geode_DestroyPoolFactory(pool_factory);
  apache_geode_DestroyPoolManager(pool_manager);
  apache_geode_DestroyCache(cache);

  apache_geode_DestroyCacheFactory(cache_factory);
  auto leaks = apache_geode_ClientUninitialize(client);
  ASSERT_EQ(leaks, 0);
}
