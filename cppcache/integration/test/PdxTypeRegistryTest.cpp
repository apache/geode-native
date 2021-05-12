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
#include "framework/Gfsh.h"

namespace {

using apache::geode::client::Cache;
using apache::geode::client::CacheableInt16;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheListener;
using apache::geode::client::NotConnectedException;
using apache::geode::client::PdxInstance;
using apache::geode::client::Region;
using apache::geode::client::RegionEvent;
using apache::geode::client::RegionShortcut;
using apache::geode::client::SelectResults;

static bool isDisconnected = false;

class RegionListener : public CacheListener {
 public:
  void waitConnected() {
    std::unique_lock<decltype(mutex_)> lock{mutex_};
    status_cv_.wait(lock, [this] { return status_; });
  }

  void waitDisconnected() {
    std::unique_lock<decltype(mutex_)> lock{mutex_};
    status_cv_.wait(lock, [this] { return !status_; });
  }

 protected:
  void afterRegionDisconnected(Region&) override {
    std::unique_lock<decltype(mutex_)> lock{mutex_};

    status_ = false;
    status_cv_.notify_all();
  }

  void afterRegionLive(const RegionEvent&) override {
    std::unique_lock<decltype(mutex_)> lock{mutex_};

    status_ = true;
    status_cv_.notify_all();
  }

 protected:
  bool status_;
  std::mutex mutex_;
  std::condition_variable status_cv_;
};

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
    Cache& cache, std::shared_ptr<RegionListener> listener) {
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

TEST(PdxTypeRegistryTest, cleanupOnClusterRestartAndPut) {
  Cluster cluster{LocatorCount{1}, ServerCount{2}};
  cluster.start();

  auto& gfsh = cluster.getGfsh();
  gfsh.create().region().withName("region").withType("PARTITION").execute();

  auto listener = std::make_shared<RegionListener>();

  auto cache = createTestCache();
  createTestPool(cluster, cache);
  auto qs = cache.getQueryService("pool");
  auto region = createTestRegion(cache, listener);

  std::string key = "entry";
  auto pdx = createTestPdxInstance(cache, key);
  region->put(key, pdx);

  // Shutdown and wait for some time
  gfsh.shutdown().execute();
  listener->waitDisconnected();
  std::this_thread::sleep_for(std::chrono::seconds{15});

  for (auto& server : cluster.getServers()) {
    server.start();
  }

  listener->waitConnected();

  region->put(key, pdx);

  // If PdxTypeRegistry was cleaned up, then the PdxType should have been
  // registered in the new cluster

  std::shared_ptr<SelectResults> result;
  auto query = qs->newQuery("SELECT * FROM /region WHERE int_value = -1");

  EXPECT_NO_THROW(result = query->execute());
  EXPECT_TRUE(result);
  EXPECT_EQ(result->size(), 1);
}

TEST(PdxTypeRegistryTest, cleanupOnClusterRestartAndFetchFields) {
  Cluster cluster{LocatorCount{1}, ServerCount{2}};
  cluster.start();

  auto& gfsh = cluster.getGfsh();
  gfsh.create().region().withName("region").withType("PARTITION").execute();

  auto listener = std::make_shared<RegionListener>();

  auto cache = createTestCache();
  createTestPool(cluster, cache);
  auto qs = cache.getQueryService("pool");
  auto region = createTestRegion(cache, listener);

  std::string key = "before-shutdown";
  region->put(key, createTestPdxInstance(cache, key));
  auto object = region->get(key);
  EXPECT_TRUE(object);

  auto pdx = std::dynamic_pointer_cast<PdxInstance>(object);
  EXPECT_TRUE(pdx);

  // Shutdown and wait for some time
  gfsh.shutdown().execute();
  listener->waitDisconnected();
  std::this_thread::sleep_for(std::chrono::seconds{15});

  for (auto& server : cluster.getServers()) {
    server.start();
  }

  listener->waitConnected();
  auto fields = pdx->getFieldNames();
  EXPECT_TRUE(fields);

  std::set<std::string> fields_set;
  for (auto field : fields->value()) {
    fields_set.insert(field->toString());
  }

  EXPECT_EQ(fields_set.count("entryName"), 1);
  EXPECT_EQ(fields_set.count("int_value"), 1);
}
}  // namespace
