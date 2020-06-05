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
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "framework/Cluster.h"
#include "framework/Framework.h"
#include "framework/Gfsh.h"

namespace {

using apache::geode::client::Cache;
using apache::geode::client::CacheableInt16;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheListener;
using apache::geode::client::NotConnectedException;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

static bool isDisconnected = false;

class RegionDisconnectedListener : public CacheListener {
  void afterRegionDisconnected(Region&) override { isDisconnected = true; }
};

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

  auto regionDisconnectedListener =
      std::make_shared<RegionDisconnectedListener>();
  auto region = regionFactory.setPoolName("pool")
                    .setCacheListener(regionDisconnectedListener)
                    .create("region");

  region->put("one", std::make_shared<CacheableInt16>(1));

  auto& servers = cluster.getServers();
  servers[0].stop();

  try {
    region->put("two", std::make_shared<CacheableInt16>(2));
  } catch (const NotConnectedException&) {
  }

  ASSERT_EQ(isDisconnected, true);
}
}  // namespace
