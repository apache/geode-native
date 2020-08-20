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

#include <thread>

#include <geode/CacheFactory.hpp>
#include <geode/CacheTransactionManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

namespace {

using apache::geode::client::Cache;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheTransactionManager;
using apache::geode::client::Pool;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;
using std::chrono::minutes;

const std::string regionName = "my_region";

std::shared_ptr<Cache> createCache() {
  auto cache = CacheFactory()
                   .set("log-level", "debug")
                   .setPdxReadSerialized(true)
                   .create();
  return std::make_shared<Cache>(std::move(cache));
}

std::shared_ptr<Region> createRegion(Cluster& cluster,
                                     std::shared_ptr<Cache> cache) {
  auto poolFactory = cache->getPoolManager().createFactory();
  cluster.applyLocators(poolFactory);
  poolFactory.setPRSingleHopEnabled(true);

  auto pool = poolFactory.create("pool");
  auto regionFactory = cache->createRegionFactory(RegionShortcut::PROXY);
  auto region = regionFactory.setPoolName("pool").create(regionName);

  return region;
}

void doPut(std::shared_ptr<Cache> cache, std::shared_ptr<Region> region) {
  auto factory = cache->createPdxInstanceFactory(
      "name.string,surname.string,email.string,address.string");
  factory.writeObject("name", CacheableString::create("John"));
  factory.writeObject("surname", CacheableString::create("Johnson"));
  factory.writeObject("email", CacheableString::create("john@johnson.mail"));
  factory.writeObject("address", CacheableString::create("Fake St 123"));

  auto pdxInstance = factory.create();
  region->put(100, pdxInstance);
}

TEST(PdxInstanceFactoryTest, testConcurrentCreateCalls) {
  const int NUM_THREADS = 8;

  Cluster cluster{LocatorCount{1}, ServerCount{2}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName(regionName)
      .withType("PARTITION")
      .execute();

  auto cache = createCache();
  auto region = createRegion(cluster, cache);

  std::vector<std::thread> clientThreads;

  for (int i = 0; i < NUM_THREADS; i++) {
    std::thread th(doPut, cache, region);
    clientThreads.push_back(std::move(th));
  }

  for (std::thread& th : clientThreads) {
    if (th.joinable()) {
      th.join();
    }
  }
  cache->close();
}  // test

}  // namespace
