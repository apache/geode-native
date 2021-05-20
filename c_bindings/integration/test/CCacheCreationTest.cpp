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
TEST(CCacheCreationTest, setPdxIgnoreUnreadFieldsAndCreateCache) {
  auto client = apache_geode_ClientInitialize();
  auto cache_factory = apache_geode_CreateCacheFactory(client);

  apache_geode_CacheFactory_SetProperty(cache_factory,
                                        "statistic-sampling-enabled", "false");

  apache_geode_CacheFactory_SetPdxIgnoreUnreadFields(cache_factory, true);
  auto cache = apache_geode_CacheFactory_CreateCache(cache_factory);
  ASSERT_TRUE(apache_geode_Cache_GetPdxIgnoreUnreadFields(cache));

  apache_geode_CacheFactory_SetPdxIgnoreUnreadFields(cache_factory, false);
  auto cache2 = apache_geode_CacheFactory_CreateCache(cache_factory);
  ASSERT_FALSE(apache_geode_Cache_GetPdxIgnoreUnreadFields(cache2));

  apache_geode_DestroyCache(cache);
  apache_geode_DestroyCache(cache2);

  apache_geode_DestroyCacheFactory(cache_factory);
  auto leaks = apache_geode_ClientUninitialize(client);

  ASSERT_EQ(leaks, 0);
}
