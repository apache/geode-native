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

#include <chrono>
#include <thread>

#include <geode/Cache.hpp>
#include <geode/EntryEvent.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/TypeRegistry.hpp>

#include "Order.hpp"

namespace {

using apache::geode::client::Cache;
using apache::geode::client::Cacheable;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheListener;
using apache::geode::client::EntryEvent;
using apache::geode::client::Pool;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;
using std::chrono::minutes;
using WanDeserialization::Order;

class GeodeCacheListener : public CacheListener {
 public:
  void afterCreate(const EntryEvent& event) override {
    numEvents++;
    auto region = event.getRegion();
    std::cout << "GeodeCacheListener::afterCreate  " << region->getName()
              << std::endl;
  }

  int getNumEvents() { return numEvents; }

 private:
  int numEvents = 0;
};  // class GeodeCacheListener

const std::string regionName = "region";
const std::chrono::seconds fiveSeconds(5);

Cache createCache(std::string durableClientId) {
  using apache::geode::client::CacheFactory;

  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("statistic-sampling-enabled", "false")
                   .setPdxReadSerialized(true)
                   .set("durable-client-id", durableClientId)
                   .set("durable-timeout", "300s")
                   .create();

  return cache;
}

std::shared_ptr<Pool> createPool(Cluster& cluster, Cache& cache,
                                 std::string poolName) {
  auto poolFactory = cache.getPoolManager().createFactory();
  cluster.applyLocators(poolFactory);
  poolFactory.setPRSingleHopEnabled(true);
  poolFactory.setSubscriptionEnabled(true);
  return poolFactory.create(poolName);
}

std::shared_ptr<Region> setupRegion(
    Cache& cache, const std::shared_ptr<Pool>& pool,
    std::shared_ptr<GeodeCacheListener> cacheListener) {
  auto region = cache.createRegionFactory(RegionShortcut::CACHING_PROXY)
                    .setPoolName(pool->getName())
                    .setCacheListener(cacheListener)
                    .create(regionName);
  region->registerAllKeys(true, true, true);
  return region;
}

TEST(WanDeserializationTest, testEventsAreDeserializedCorrectly) {
  uint16_t portSiteA = Framework::getAvailablePort();
  uint16_t portSiteB = Framework::getAvailablePort();

  std::vector<uint16_t> locatorPortsSiteA = {portSiteA};
  std::vector<uint16_t> locatorPortsSiteB = {portSiteB};

  std::vector<uint16_t> remoteLocatorPortsA = {portSiteB};
  std::vector<uint16_t> remoteLocatorPortsB = {portSiteA};

  Cluster clusterA{LocatorCount{1}, ServerCount{1}, locatorPortsSiteA,
                   remoteLocatorPortsA, 1};
  clusterA.start();

  Cluster clusterB{LocatorCount{1}, ServerCount{1}, locatorPortsSiteB,
                   remoteLocatorPortsB, 2};
  clusterB.start();

  // Create gw receivers
  clusterA.getGfsh().create().gatewayReceiver().execute();
  clusterB.getGfsh().create().gatewayReceiver().execute();

  // Create gw senders
  clusterA.getGfsh()
      .create()
      .gatewaySender()
      .withId("A-to-B")
      .withRemoteDSId("2")
      .execute();
  clusterB.getGfsh()
      .create()
      .gatewaySender()
      .withId("B-to-A")
      .withRemoteDSId("1")
      .execute();

  // Create regions
  clusterA.getGfsh()
      .create()
      .region()
      .withName(regionName)
      .withType("PARTITION")
      .withGatewaySenderId("A-to-B")
      .execute();
  clusterB.getGfsh()
      .create()
      .region()
      .withName(regionName)
      .withType("PARTITION")
      .withGatewaySenderId("B-to-A")
      .execute();

  auto cacheA = createCache("clientA");
  auto poolA = createPool(clusterA, cacheA, "poolSiteA");
  std::shared_ptr<GeodeCacheListener> cacheListenerA =
      std::make_shared<GeodeCacheListener>();
  auto regionA = setupRegion(cacheA, poolA, cacheListenerA);

  auto cacheB = createCache("clientB");
  auto poolB = createPool(clusterB, cacheB, "poolSiteB");
  std::shared_ptr<GeodeCacheListener> cacheListenerB =
      std::make_shared<GeodeCacheListener>();
  auto regionB = setupRegion(cacheB, poolB, cacheListenerB);

  cacheA.getTypeRegistry().registerPdxType(Order::createDeserializable);
  cacheA.readyForEvents();

  cacheB.getTypeRegistry().registerPdxType(Order::createDeserializable);
  cacheB.readyForEvents();

  auto order = std::make_shared<Order>(2, "product y", 37);
  regionA->put("order", order);

  std::this_thread::sleep_for(fiveSeconds);  // wait for the event to be sent
  ASSERT_EQ(cacheListenerA->getNumEvents(), 1);
  ASSERT_EQ(cacheListenerB->getNumEvents(), 1);

}  // TEST

}  // namespace
