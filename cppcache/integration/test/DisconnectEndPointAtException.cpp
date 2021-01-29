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
using std::cout;

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

void runClientOperations(std::shared_ptr<Cache> cache,
                         std::shared_ptr<Region> region, int minEntryKey,
                         int maxEntryKey, bool usingPartitionResolver) {
  auto transactionManager = cache->getCacheTransactionManager();
  auto end = std::chrono::system_clock::now() + std::chrono::minutes{2};
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
  } while (std::chrono::system_clock::now() < end);
}

std::string getConcatHostName(ServerAddress address) {
  std::string hostname;
  return hostname.append(address.address.c_str())
      .append(":")
      .append(std::to_string(address.port));
}

bool checkIfEpDisconnected(const std::string epHostname,
                           std::vector<std::string> list) {
  if (std::find(list.begin(), list.end(), epHostname) != list.end()) {
    return true;
  }
  return false;
}

void executeTestCase(bool useSingleHopAndPR) {
  int NUM_THREADS = 4;
  int MAX_ENTRY_KEY = 1000000;
  auto keyRangeSize = (MAX_ENTRY_KEY / NUM_THREADS);

  Cluster cluster{LocatorCount{1}, ServerCount{2}};
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

  // activate hook that will from now on register all servers
  // that are disconnected from client
  auto cacheImpl = CacheRegionHelper::getCacheImpl(cache.get());
  CacheImpl::setDisconnectionTest();

  std::vector<std::thread> clientThreads;
  for (int i = 0; i < NUM_THREADS; i++) {
    auto minKey = (i * keyRangeSize);
    auto maxKey = minKey + keyRangeSize - 1;
    std::thread th(runClientOperations, cache, region, minKey, maxKey,
                   useSingleHopAndPR);
    clientThreads.push_back(std::move(th));
  }
  // Shut down the server
  cluster.getServers()[1].stop();

  for (std::thread& th : clientThreads) {
    if (th.joinable()) {
      th.join();
    }
  }

  auto list = CacheImpl::getListOfDisconnectedEPs();

  // Check that running server remain connected to client
  ASSERT_FALSE(checkIfEpDisconnected(epRunningHostname, list));
}  // executeTestCase

TEST(DisconnectEndPointAtException, useSingleHopAndPR) {
  executeTestCase(true);
}

TEST(DisconnectEndPointAtException, doNotUseSingleHopAndPR) {
  executeTestCase(false);
}
}  // namespace
