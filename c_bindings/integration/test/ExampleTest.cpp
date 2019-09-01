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

#include "geode/cache.h"
#include "geode/cache/factory.h"
#include "geode/client.h"
#include "geode/pool.h"
#include "geode/pool/factory.h"
#include "geode/pool/manager.h"
#include "geode/region.h"
#include "geode/region/factory.h"
#include "geode/region/shortcut.h"

/**
 * Example test using single server and waiting for async put and update
 * operations to synchronize using promises.
 */
TEST(ExampleTest, putGetAndUpdateWith1Server) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto client = apache_geode_ClientInitialize();
  auto cache_factory = apache_geode_CreateCacheFactory(client);

  apache_geode_CacheFactory_SetProperty(cache_factory, "log-level", "none");
  apache_geode_CacheFactory_SetProperty(cache_factory,
                                        "statistic-sampling-enabled", "false");

  auto cache = apache_geode_CacheFactory_CreateCache(cache_factory);

  auto pool_manager = apache_geode_Cache_GetPoolManager(cache);
  auto pool_factory = apache_geode_PoolManager_CreateFactory(pool_manager);
  apache_geode_PoolFactory_AddLocator(pool_factory, "localhost",
                                      cluster.getLocatorPort());
  auto pool = apache_geode_PoolFactory_CreatePool(pool_factory, "myPool");
  auto region_factory = apache_geode_Cache_CreateRegionFactory(cache, PROXY);
  apache_geode_RegionFactory_SetPoolName(region_factory, "myPool");
  auto region =
      apache_geode_RegionFactory_CreateRegion(region_factory, "region");

  apache_geode_Region_PutString(region, "key", "value");

  auto value = apache_geode_Region_GetString(region, "key");

  ASSERT_STREQ(value, "value");

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
