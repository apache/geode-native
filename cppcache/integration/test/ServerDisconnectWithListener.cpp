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

#include <util/concurrent/binary_semaphore.hpp>

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/CacheListener.hpp>
#include <geode/EntryEvent.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionEvent.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "framework/Cluster.h"
#include "framework/Gfsh.h"
#include "gmock_actions.hpp"
#include "mock/CacheListenerMock.hpp"

namespace {

using apache::geode::client::binary_semaphore;
using apache::geode::client::Cache;
using apache::geode::client::CacheableInt16;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheListener;
using apache::geode::client::CacheListenerMock;
using apache::geode::client::NotConnectedException;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

using ::testing::_;
using ::testing::NiceMock;

Cache createTestCache() {
  CacheFactory cacheFactory;
  return cacheFactory.set("log-level", "none")
      .set("statistic-sampling-enabled", "false")
      .create();
}

TEST(ServerDisconnect, WithRegionDisconnectedListener) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  auto cache = createTestCache();

  auto poolFactory =
      cache.getPoolManager().createFactory().setSubscriptionEnabled(true);

  cluster.applyLocators(poolFactory);

  auto pool = poolFactory.create("pool");
  auto regionFactory = cache.createRegionFactory(RegionShortcut::CACHING_PROXY);

  binary_semaphore live_sem{0};
  binary_semaphore shut_sem{1};
  auto listener = std::make_shared<NiceMock<CacheListenerMock>>();
  EXPECT_CALL(*listener, afterRegionLive(_))
      .WillRepeatedly(AcquireSem(&shut_sem));
  EXPECT_CALL(*listener, afterRegionDisconnected(_))
      .WillRepeatedly(ReleaseSem(&shut_sem));
  auto region =
      regionFactory.setPoolName("pool").setCacheListener(listener).create(
          "region");

  region->put("one", std::make_shared<CacheableInt16>(1));

  auto& servers = cluster.getServers();
  servers[0].stop();
  shut_sem.acquire();
  shut_sem.release();

  EXPECT_THROW(region->put("two", std::make_shared<CacheableInt16>(2)),
               NotConnectedException);
}
}  // namespace
