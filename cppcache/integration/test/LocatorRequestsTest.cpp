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

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "framework/Cluster.h"
#include "framework/Gfsh.h"

using apache::geode::client::Cache;
using apache::geode::client::CacheableInt16;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::IllegalStateException;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

Cache createTestCache() {
  CacheFactory cacheFactory;
  return cacheFactory.set("log-level", "none")
      .set("connect-timeout", "5s")
      .set("statistic-sampling-enabled", "false")
      .create();
}

std::shared_ptr<Region> setupCachingProxyRegion(Cache& cache) {
  auto region = cache.createRegionFactory(RegionShortcut::CACHING_PROXY)
                    .setPoolName("default")
                    .create("region");

  return region;
}

std::shared_ptr<Region> setupProxyRegion(Cache& cache) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .create("region");

  return region;
}

TEST(LocatorRequestsTest, verifyNoInterlock) {
  const auto N = 64U;
  const auto ERR_MARGIN = 0.10;
  const std::chrono::seconds TIMEOUT{10};

  std::mutex mutex;
  std::thread runners[N];
  std::condition_variable cv;
  std::atomic_size_t completed_requests;
  const auto threshold = std::size_t(N * (0.5 - ERR_MARGIN));

  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.start();

  const auto& locator = cluster.getLocators().front();
  auto cache = createTestCache();
  auto poolFactory = cache.getPoolManager()
                         .createFactory()
                         .setSubscriptionEnabled(true)
                         .setUpdateLocatorListInterval(std::chrono::seconds(0))
                         .addLocator("0.1.2.3", 10334);
  cluster.applyLocators(poolFactory);
  auto pool = poolFactory.create("default");

  completed_requests = 0;
  std::unique_lock<std::mutex> lk(mutex);

  for (auto i = 0U; i < N;) {
    runners[i++] = std::thread{[&completed_requests, &cv, &pool] {
      auto servers = pool->getServers();
      EXPECT_TRUE(servers);
      ++completed_requests;
      cv.notify_one();
    }};
  }

  auto before = std::chrono::steady_clock::now();
  auto res = cv.wait_for(lk, TIMEOUT, [&completed_requests, &threshold] {
    return completed_requests >= threshold;
  });

  auto after = std::chrono::steady_clock::now();
  auto elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::duration{(after - before).count()})
          .count();

  std::clog << completed_requests.load() << " out of " << N << " in " << elapsed
            << " ms." << std::endl;

  for (auto i = 0U; i < N;) {
    runners[i++].join();
  }

  EXPECT_TRUE(res);
}
