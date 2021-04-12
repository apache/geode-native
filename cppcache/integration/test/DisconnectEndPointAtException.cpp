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
#include <framework/Gfsh.h>
#include <framework/TestConfig.h>

#include <CacheImpl.hpp>
#include <CacheRegionHelper.hpp>
#include <TcrConnectionManager.hpp>
#include <TcrEndpoint.hpp>
#include <thread>

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/CacheTransactionManager.hpp>
#include <geode/Region.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

namespace {
using apache::geode::client::Cache;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheImpl;
using apache::geode::client::CacheRegionHelper;
using apache::geode::client::CacheTransactionManager;
using apache::geode::client::Pool;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;
using apache::geode::client::TcrConnectionManager;
using apache::geode::client::TcrEndpoint;

std::shared_ptr<Cache> createCache() {
  auto cache = CacheFactory().create();
  return std::make_shared<Cache>(std::move(cache));
}

std::shared_ptr<Pool> createPool(Cluster& cluster, std::shared_ptr<Cache> cache,
                                 bool useSingleHop) {
  auto poolFactory = cache->getPoolManager().createFactory();
  cluster.applyLocators(poolFactory);
  poolFactory.setPRSingleHopEnabled(useSingleHop);
  return poolFactory.create("default");
}

std::string addKeyPrefix(int key) {
  return std::to_string(key) + "|" + std::to_string(key);
}

void runOperationsUntilServerDisconnects(std::shared_ptr<Cache> cache,
                                         std::shared_ptr<Region> region,
                                         int minEntryKey, int maxEntryKey,
                                         bool usingPartitionResolver,
                                         CacheImpl* cacheImpl,
                                         std::string epShutDownHostname) {
  auto transactionManager = cache->getCacheTransactionManager();

  auto end = std::chrono::system_clock::now() + std::chrono::minutes(2);
  bool isTimeoutUpdated = false;
  do {
    auto theKey = (rand() % (maxEntryKey - minEntryKey)) + minEntryKey;
    std::string theValue = "theValue";
    try {
      transactionManager->begin();
      if (usingPartitionResolver) {
        region->put(addKeyPrefix(theKey), theValue);
      } else {
        region->put(theKey, theValue);
      }
      transactionManager->commit();
    } catch (...) {
      if (transactionManager->exists()) {
        transactionManager->rollback();
      }
    }

    int isShutDown = cacheImpl->getNumberOfTimeEndpointDisconnected(
        epShutDownHostname, "default");

    // After server disconnects then send traffic for 15 more seconds
    if (isShutDown != 0 && !isTimeoutUpdated) {
      end = std::chrono::system_clock::now() + std::chrono::seconds(15);
      isTimeoutUpdated = true;
    }
  } while (std::chrono::system_clock::now() < end);
}

std::string getConcatHostName(ServerAddress address) {
  std::string hostname;
  return hostname.append(address.address)
      .append(":")
      .append(std::to_string(address.port));
}

void executeTestCase(bool useSingleHopAndPR) {
  int NUM_THREADS = 4;
  int MAX_ENTRY_KEY = 1000000;
  auto keyRangeSize = (MAX_ENTRY_KEY / NUM_THREADS);

  std::vector<uint16_t> serverPorts;
  serverPorts.push_back(Framework::getAvailablePort());
  serverPorts.push_back(Framework::getAvailablePort());

  Cluster cluster{LocatorCount{1}, ServerCount{2}, serverPorts};
  cluster.start();
  auto region_cmd =
      cluster.getGfsh().create().region().withName("region").withType(
          "PARTITION");
  if (useSingleHopAndPR) {
    region_cmd
        .withPartitionResolver(
            "org.apache.geode.cache.util.StringPrefixPartitionResolver")
        .execute();
  } else {
    region_cmd.execute();
  }

  auto cache = createCache();
  auto pool = createPool(cluster, cache, useSingleHopAndPR);
  auto region = cache->createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .create("region");

  // Get address of server that will remain running
  auto epRunning = cluster.getServers()[0].getAddress();
  auto epRunningHostname = getConcatHostName(epRunning);

  // Get address of server that will be shutdown
  auto epShutDown = cluster.getServers()[1].getAddress();
  auto epShutDownHostname = getConcatHostName(epShutDown);

  auto cacheImpl = CacheRegionHelper::getCacheImpl(cache.get());

  std::vector<std::thread> clientThreads;
  for (int i = 0; i < NUM_THREADS; i++) {
    auto minKey = (i * keyRangeSize);
    auto maxKey = minKey + keyRangeSize - 1;
    std::thread th(runOperationsUntilServerDisconnects, cache, region, minKey,
                   maxKey, useSingleHopAndPR, cacheImpl, epShutDownHostname);
    clientThreads.push_back(std::move(th));
  }
  // Shut down the server
  cluster.getServers()[1].stop();

  for (std::thread& th : clientThreads) {
    if (th.joinable()) {
      th.join();
    }
  }
  ASSERT_EQ(cacheImpl->getNumberOfTimeEndpointDisconnected(epRunningHostname,
                                                           "default"),
            0);
  ASSERT_EQ(cacheImpl->getNumberOfTimeEndpointDisconnected(epShutDownHostname,
                                                           "default"),
            1);
}  // executeTestCase

TEST(DisconnectEndPointAtException, useSingleHopAndPR) {
  executeTestCase(true);
}

TEST(DisconnectEndPointAtException, doNotUseSingleHopAndPR) {
  executeTestCase(false);
}
}  // namespace
