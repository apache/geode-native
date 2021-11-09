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

#include <benchmark/benchmark.h>
#include <framework/Cluster.h>
#include <framework/Gfsh.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4596)
#endif
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>

#include <geode/Cache.hpp>
#include <geode/CacheableString.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

using apache::geode::client::Cache;
using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableString;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

namespace {

class RegionBM : public benchmark::Fixture {
 public:
  RegionBM() {
    boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                        boost::log::trivial::warning);
  }

  using benchmark::Fixture::SetUp;
  void SetUp(benchmark::State&) override {
    if (!cluster) {
      cluster = std::unique_ptr<Cluster>(
          new Cluster(::Name{name_}, LocatorCount{1}, ServerCount{1}));
      cluster->getGfsh()
          .create()
          .region()
          .withName("region")
          .withType("REPLICATE")
          .execute();

      cache = std::unique_ptr<Cache>(new Cache(cluster->createCache()));
      region = cache->createRegionFactory(RegionShortcut::PROXY)
                   .setPoolName("default")
                   .create("region");
    }
  }

  using benchmark::Fixture::TearDown;
  void TearDown(benchmark::State&) override {
    if (cluster) {
      region = nullptr;
      cache = nullptr;
      cluster = nullptr;
    }
  }

 protected:
  void SetName(const char* name) {
    name_ = name;

    Benchmark::SetName(name);
  }

  std::unique_ptr<Cluster> cluster;
  std::unique_ptr<Cache> cache;
  std::shared_ptr<Region> region;

 private:
  std::string name_;
};

BENCHMARK_F(RegionBM, put_string)(benchmark::State& state) {
  auto key = CacheableString::create("key");
  auto value = CacheableString::create("value");

  for (auto _ : state) {
    region->put(key, value);
  }
}

BENCHMARK_F(RegionBM, put_int)(benchmark::State& state) {
  auto key = CacheableInt32::create(1);
  auto value = CacheableInt32::create(1);

  for (auto _ : state) {
    region->put(key, value);
  }
}

BENCHMARK_F(RegionBM, get_string)(benchmark::State& state) {
  auto key = CacheableString::create("key");
  auto value = CacheableString::create("value");

  region->put(key, value);

  for (auto _ : state) {
    region->get(key);
  }
}

BENCHMARK_F(RegionBM, get_int)(benchmark::State& state) {
  auto key = CacheableInt32::create(1);
  auto value = CacheableInt32::create(1);

  region->put(key, value);

  for (auto _ : state) {
    region->get(key);
  }
}

}  // namespace
