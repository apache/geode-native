/* Licensed to the Apache Software Foundation (ASF) under one or more *
 * contributor license agreements.  See the NOTICE file distributed with this
 * work for additional information regarding copyright ownership. The ASF
 * licenses this file to You under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/CacheListener.hpp>
#include <geode/PoolManager.hpp>
#include <geode/QueryService.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "framework/Cluster.h"
#include "framework/Framework.h"
#include "gmock_actions.hpp"
#include "mock/CacheListenerMock.hpp"
#include "util/concurrent/binary_semaphore.hpp"

namespace {

using apache::geode::client::binary_semaphore;

using apache::geode::client::Cache;
using apache::geode::client::CacheableInt16;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheListener;
using apache::geode::client::CacheListenerMock;
using apache::geode::client::NotConnectedException;
using apache::geode::client::PdxFieldTypes;
using apache::geode::client::PdxInstance;
using apache::geode::client::Region;
using apache::geode::client::RegionEvent;
using apache::geode::client::RegionShortcut;
using apache::geode::client::SelectResults;

using ::testing::_;
using ::testing::DoAll;
using ::testing::InvokeWithoutArgs;
using ::testing::Return;

Cache createTestCache() {
  CacheFactory cacheFactory;
  return cacheFactory.set("log-level", "none")
      .set("connect-timeout", "2s")
      .set("statistic-sampling-enabled", "false")
      .set("on-client-disconnect-clear-pdxType-Ids", "true")
      .setPdxReadSerialized(true)
      .create();
}

void createTestPool(Cluster& cluster, Cache& cache) {
  auto poolFactory = cache.getPoolManager()
                         .createFactory()
                         .setReadTimeout(std::chrono::seconds{1})
                         .setPingInterval(std::chrono::seconds{5})
                         .setSubscriptionEnabled(true);

  cluster.applyLocators(poolFactory);
  poolFactory.create("pool");
}

std::shared_ptr<Region> createTestRegion(
    Cache& cache, std::shared_ptr<CacheListener> listener) {
  auto regionFactory = cache.createRegionFactory(RegionShortcut::PROXY);
  return regionFactory.setPoolName("pool").setCacheListener(listener).create(
      "region");
}

std::shared_ptr<PdxInstance> createTestPdxInstance(Cache& cache,
                                                   const std::string& entry) {
  auto factory = cache.createPdxInstanceFactory("__GEMFIRE_JSON", false);
  return factory.writeString("entryName", entry)
      .writeInt("int_value", -1)
      .create();
}

/**
 * The purpose of this test case is to verify that PdxTypeRegistry is cleaned up
 * under the following scenario:
 *  1 - Spin up a cluster and create a PARTITION region called 'region'. Note
 *      that this region won't be persisting any data.
 *  2 - A cache with is on-client-disconnect-clear-pdxType-Ids=true, meaning
 *      it should cleanup the PdxTypeRegistry upon cluster disconnection.
 *  3 - A PdxInstance with is put into key 'entry' and region 'region'
 *      Before inserting the entry, its PdxType will be created on the cluster,
 *      as it should not be present.
 *  4 - Restart the cluster. Region 'region' should be empty as it was
 *      persisting no data.
 *  5 - Cache should automatically cleanup PdxTypeRegistry, containing the
 *      cache of PdxType's handled by this cache.
 *  6 - Insert again the entry inserted in step 3. If the registry was correctly
 *      cleaned up as stated in state 5, PdxType will be inserted again before
 *      inserting the entry. If PdxTypeRegistry clean up did not happen, the old
 *      PdxTypeId will be used when inserting this entry.
 *  7 - Fetch all of the 'region' entries using an OQL query, hence, forcing the
 *      members to deserialize the entries contained on the region. The query
 *      should not throw any exception and return exactly one entry, inserted on
 *      step 6. If PdxTypeRegistry cleanup fails, then the old PdxTypeId will be
 *      used when inserting the entry on step 6 and consequently members will
 *      fail to deserialize the entry as there will be no PdxType matching the
 *      given PdxTypeId, consequently leading to an UnknownPdxException being
 *      thrown.
 */
TEST(PdxTypeRegistryTest, cleanupOnClusterRestartAndPut) {
  Cluster cluster{LocatorCount{1}, ServerCount{2}};
  cluster.start();

  auto& gfsh = cluster.getGfsh();
  gfsh.create().region().withName("region").withType("PARTITION").execute();

  binary_semaphore live_sem{0};
  binary_semaphore shut_sem{1};
  auto listener = std::make_shared<CacheListenerMock>();
  EXPECT_CALL(*listener, afterRegionLive(_))
      .WillRepeatedly(DoAll(ReleaseSem(&live_sem), AcquireSem(&shut_sem)));
  EXPECT_CALL(*listener, afterRegionDisconnected(_))
      .WillRepeatedly(DoAll(ReleaseSem(&shut_sem), AcquireSem(&live_sem)));
  EXPECT_CALL(*listener, afterCreate(_)).WillRepeatedly(Return());
  EXPECT_CALL(*listener, afterRegionDestroy(_)).WillRepeatedly(Return());
  EXPECT_CALL(*listener, close(_)).WillRepeatedly(Return());

  auto cache = createTestCache();
  createTestPool(cluster, cache);
  auto qs = cache.getQueryService("pool");
  auto region = createTestRegion(cache, listener);

  std::string key = "entry";
  auto pdx = createTestPdxInstance(cache, key);
  region->put(key, pdx);

  // Shutdown and wait for some time
  gfsh.shutdown().execute();

  shut_sem.acquire();
  shut_sem.release();

  std::this_thread::sleep_for(std::chrono::seconds{15});

  for (auto& server : cluster.getServers()) {
    server.start();
  }

  live_sem.acquire();
  live_sem.release();

  region->put(key, pdx);

  std::shared_ptr<SelectResults> result;
  auto query = qs->newQuery("SELECT * FROM /region WHERE entryName = 'entry'");

  // If PdxTypeRegistry was not correctly cleaned up, this query will throw
  // UnknownPdxTypeException. See the comment at the beginning of the test for
  // additional info.
  EXPECT_NO_THROW(result = query->execute());
  EXPECT_TRUE(result);
  EXPECT_EQ(result->size(), 1);
}

/**
 * The purpose of this test case is to verify that there are no coredumps when
 * calling several PdxInstance's methods due to PdxTypeRegistry cleanup under
 * the following scenario:
 *  1 - Spin up a cluster and create a PARTITION region called 'region'. Note
 *      that this region won't be persisting any data.
 *  2 - A cache with is on-client-disconnect-clear-pdxType-Ids=true, meaning
 *      it should cleanup the PdxTypeRegistry upon cluster disconnection.
 *  3 - Create and put PdxInstance with is put into key 'entry' and region
 *     'region'.
 *  4 - Restart the cluster. Region 'region' should be empty as it was
 *      persisting no data.
 *  5 - Cache should automatically cleanup PdxTypeRegistry, containing the
 *      cache of PdxType's handled by this cache.
 *  6 - If PdxInstance's PdxType object is stored inside the PdxInstance rather
 *      than its PdxTypeId, all of the accessors could be called for the
 *      PdxInstance even if this PdxType was not added to the PdxTypeRegistry.
 *      If instead the PdxTypeId is stored, this will cause an coredump given
 *      that the cache won't be able to find the PdxType for the above mentioned
 *      PdxTypeId.
 */
TEST(PdxTypeRegistryTest, cleanupOnClusterRestartAndCallAccessors) {
  Cluster cluster{LocatorCount{1}, ServerCount{2}};
  cluster.start();

  auto& gfsh = cluster.getGfsh();
  gfsh.create().region().withName("region").withType("PARTITION").execute();

  binary_semaphore live_sem{0};
  binary_semaphore shut_sem{1};
  auto listener = std::make_shared<CacheListenerMock>();
  EXPECT_CALL(*listener, afterRegionLive(_))
      .WillRepeatedly(DoAll(ReleaseSem(&live_sem), AcquireSem(&shut_sem)));
  EXPECT_CALL(*listener, afterRegionDisconnected(_))
      .WillRepeatedly(DoAll(ReleaseSem(&shut_sem), AcquireSem(&live_sem)));
  EXPECT_CALL(*listener, afterCreate(_)).WillRepeatedly(Return());
  EXPECT_CALL(*listener, afterRegionDestroy(_)).WillRepeatedly(Return());
  EXPECT_CALL(*listener, close(_)).WillRepeatedly(Return());

  auto cache = createTestCache();
  createTestPool(cluster, cache);
  auto qs = cache.getQueryService("pool");
  auto region = createTestRegion(cache, listener);

  std::string key = "entry";
  region->put(key, createTestPdxInstance(cache, key));
  auto object = region->get(key);
  EXPECT_TRUE(object);

  auto pdx = std::dynamic_pointer_cast<PdxInstance>(object);
  EXPECT_TRUE(pdx);

  // Shutdown and wait for some time
  gfsh.shutdown().execute();

  shut_sem.acquire();
  shut_sem.release();

  std::this_thread::sleep_for(std::chrono::seconds{15});

  for (auto& server : cluster.getServers()) {
    server.start();
  }

  live_sem.acquire();
  live_sem.release();

  // Checking no coredump is happening when calling isIdentityField
  EXPECT_FALSE(pdx->isIdentityField("entryName"));
  EXPECT_FALSE(pdx->isIdentityField("int_value"));

  // Checking no coredump is happening when calling hasField
  EXPECT_TRUE(pdx->hasField("entryName"));
  EXPECT_TRUE(pdx->hasField("int_value"));

  // Checking no coredump is happening when calling getFieldNames
  auto fields = pdx->getFieldNames();
  EXPECT_TRUE(fields);

  std::set<std::string> fieldsSet;
  for (auto field : fields->value()) {
    fieldsSet.insert(field->toString());
  }

  EXPECT_EQ(fieldsSet.count("entryName"), 1);
  EXPECT_EQ(fieldsSet.count("int_value"), 1);

  // Checking no coredump is happening when calling getFieldType
  EXPECT_EQ(pdx->getFieldType("entryName"), PdxFieldTypes::STRING);
  EXPECT_EQ(pdx->getFieldType("int_value"), PdxFieldTypes::INT);

  // Checking no coredump is happening when calling getStringField
  EXPECT_EQ(pdx->getStringField("entryName"), key);

  // Checking no coredump is happening when calling getIntField
  EXPECT_EQ(pdx->getIntField("int_value"), -1);

  // Checking no coredump is happening when calling objectSize
  EXPECT_NE(pdx->objectSize(), 0);

  // Checking no coredump is happening when calling hashcode
  EXPECT_NE(pdx->hashcode(), 0);

  // Checking no coredump is happening when calling getClassName
  EXPECT_EQ(pdx->getClassName(), "__GEMFIRE_JSON");

  // Checking no coredump is happening when calling getClassName
  EXPECT_EQ(pdx->getClassName(), "__GEMFIRE_JSON");

  // Checking no coredump is happening when calling toString
  EXPECT_FALSE(pdx->toString().empty());
}
}  // namespace
