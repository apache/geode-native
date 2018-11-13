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

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/FunctionService.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "framework/Cluster.h"
#include "framework/Gfsh.h"

using apache::geode::client::Cache;
using apache::geode::client::Cacheable;
using apache::geode::client::CacheableVector;
using apache::geode::client::FunctionExecutionException;
using apache::geode::client::FunctionService;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;
using apache::geode::client::ResultCollector;

std::shared_ptr<Region> setupRegion(Cache &cache) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .create("region");

  return region;
}

TEST(FunctionExecutionTest, UnknownFunction) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = cluster.createCache();
  auto region = setupRegion(cache);

  ASSERT_THROW(FunctionService::onServer(region->getRegionService())
                   .execute("I_Don_t_Exist"),
               FunctionExecutionException);
}

class TestResultCollector : public ResultCollector {
  virtual std::shared_ptr<CacheableVector> getResult(
      std::chrono::milliseconds) override {
    return std::shared_ptr<CacheableVector>();
  }

  virtual void addResult(const std::shared_ptr<Cacheable> &) override {}

  virtual void endResults() override {}

  virtual void clearResults() override {}
};

TEST(FunctionExecutionTest, UnknownFunctionAsync) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = cluster.createCache();
  auto region = setupRegion(cache);

  ASSERT_THROW(FunctionService::onServer(region->getRegionService())
                   .withCollector(std::make_shared<TestResultCollector>())
                   .execute("I_Don_t_Exist"),
               FunctionExecutionException);
}
