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

#include <geode/Cache.hpp>
#include <geode/CacheableString.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

namespace {

using apache::geode::client::Cache;
using apache::geode::client::CacheableString;
using apache::geode::client::HashMapOfCacheable;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

class RegisterInterestBM : public benchmark::Fixture {
 public:
  RegisterInterestBM() {
    boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                        boost::log::trivial::debug);

    BOOST_LOG_TRIVIAL(info) << "constructed";
  }

  ~RegisterInterestBM() noexcept override {
    BOOST_LOG_TRIVIAL(info) << "destructed";
  }

  using benchmark::Fixture::SetUp;
  void SetUp(benchmark::State& state) override {
    BOOST_LOG_TRIVIAL(info) << "starting cluster";
    cluster = std::unique_ptr<Cluster>(
        new Cluster(::Name{name_}, LocatorCount{1}, ServerCount{4}));
    cluster->start();
    cluster->getGfsh()
        .create()
        .region()
        .withName("region")
        .withType("PARTITION")
        .execute();

    cache = std::unique_ptr<Cache>(new Cache(cluster->createCache(
        {{"log-level", "finer"}}, Cluster::SubscriptionState::Enabled)));
    region = cache->createRegionFactory(RegionShortcut::PROXY)
                 .setPoolName("default")
                 .setCachingEnabled(true)
                 .create("region");

    BOOST_LOG_TRIVIAL(info)
        << "filling region with " << state.range(0) << " keys";
    HashMapOfCacheable map;
    const auto batchSize = 10000;
    map.reserve(batchSize);
    for (auto i = 0; i < state.range(0); ++i) {
      map.emplace(
          std::make_shared<CacheableString>("key" + std::to_string(i)),
          std::make_shared<CacheableString>("value" + std::to_string(i)));
      if (0 == i % batchSize) {
        region->putAll(map);
        map.clear();
      }
    }
    if (!map.empty()) {
      region->putAll(map);
      map.clear();
    }
    BOOST_LOG_TRIVIAL(info) << "region ready";
  }

  using benchmark::Fixture::TearDown;
  void TearDown(benchmark::State&) override {
    BOOST_LOG_TRIVIAL(info) << "stopping cluster";
    region = nullptr;
    cache = nullptr;
    cluster = nullptr;
  }

 protected:
  void SetName(const char* name) {
    name_ = name;

    Benchmark::SetName(name);
  }

  void unregisterInterestAllKeys(benchmark::State& state) {
    state.PauseTiming();
    region->unregisterAllKeys();
    state.ResumeTiming();
  }

  std::unique_ptr<Cluster> cluster;
  std::unique_ptr<Cache> cache;
  std::shared_ptr<Region> region;

 private:
  std::string name_;
};

BENCHMARK_DEFINE_F(RegisterInterestBM, registerInterestAllKeys)
(benchmark::State& state) {
  for (auto _ : state) {
    region->registerAllKeys();
    unregisterInterestAllKeys(state);
  }
}
BENCHMARK_REGISTER_F(RegisterInterestBM, registerInterestAllKeys)
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(1)
    ->Iterations(10)
    ->Arg(1000000);

BENCHMARK_DEFINE_F(RegisterInterestBM, registerInterestAllKeysInitialValues)
(benchmark::State& state) {
  for (auto _ : state) {
    region->registerAllKeys(false, true);
    unregisterInterestAllKeys(state);
  }
}
BENCHMARK_REGISTER_F(RegisterInterestBM, registerInterestAllKeysInitialValues)
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(1)
    ->Iterations(10)
    ->Arg(1000000);

}  // namespace
