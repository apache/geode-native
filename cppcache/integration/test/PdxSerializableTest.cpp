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
#include <gmock/gmock.h>

#include <memory>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/EntryEvent.hpp>
#include <geode/PdxWrapper.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionEvent.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/TypeRegistry.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "Order.hpp"
#include "PdxRemoteWriter.hpp"
#include "PdxRemoteWriterFactoryImpl.hpp"
#include "gmock_actions.hpp"
#include "mock/CacheListenerMock.hpp"
#include "util/concurrent/binary_semaphore.hpp"
#include "util/cxx_extensions.hpp"

namespace {

using apache::geode::client::binary_semaphore;

using apache::geode::client::Cache;
using apache::geode::client::Cacheable;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheListenerMock;
using apache::geode::client::CacheRegionHelper;
using apache::geode::client::DataOutput;
using apache::geode::client::EntryEvent;
using apache::geode::client::PdxReader;
using apache::geode::client::PdxRemoteWriter;
using apache::geode::client::PdxRemoteWriterFactoryImpl;
using apache::geode::client::PdxSerializable;
using apache::geode::client::PdxSerializer;
using apache::geode::client::PdxType;
using apache::geode::client::PdxTypeRegistry;
using apache::geode::client::PdxWrapper;
using apache::geode::client::PdxWriter;
using apache::geode::client::Region;
using apache::geode::client::RegionEvent;
using apache::geode::client::RegionShortcut;
using apache::geode::client::UserObjectSizer;

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;

using PdxTests::Order;

namespace {
class PdxRemoteWriterConcurrentFactory
    : public virtual PdxRemoteWriterFactoryImpl {
 public:
  ~PdxRemoteWriterConcurrentFactory() override = default;

  std::unique_ptr<PdxRemoteWriter> create(
      DataOutput& output, const std::shared_ptr<PdxSerializable>& object,
      const std::shared_ptr<PdxTypeRegistry>& pdxTypeRegistry,
      const std::shared_ptr<PdxType>& localType) override {
    std::lock_guard<decltype(m_Mutex)> guard_{m_Mutex};
    return PdxRemoteWriterFactoryImpl::create(output, object, pdxTypeRegistry,
                                              localType);
  }

  std::mutex& getMutex() { return m_Mutex; }

 protected:
  std::mutex m_Mutex;
};

}  // namespace

Cache createTestCache() {
  CacheFactory cacheFactory;
  return cacheFactory.set("log-level", "none")
      .set("on-client-disconnect-clear-pdxType-Ids", "true")
      .set("statistic-sampling-enabled", "false")
      .create();
}

TEST(PdxSerializableTest, serializeWhileClusterRestarting) {
  binary_semaphore live_sem{0};
  binary_semaphore shut_sem{1};

  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.start();

  auto& gfsh = cluster.getGfsh();
  gfsh.create().region().withName("region").withType("REPLICATE").execute();

  auto cache = createTestCache();
  auto cacheImpl = CacheRegionHelper::getCacheImpl(&cache);

  auto factory = cxx::make_unique<PdxRemoteWriterConcurrentFactory>();
  auto& mutex = factory->getMutex();

  cacheImpl->setPdxRemoteWriterFactory(std::move(factory));

  auto listener = std::make_shared<CacheListenerMock>();
  EXPECT_CALL(*listener, afterCreate(_)).WillRepeatedly(Return());
  EXPECT_CALL(*listener, afterRegionDestroy(_)).WillRepeatedly(Return());
  EXPECT_CALL(*listener, close(_)).WillRepeatedly(Return());
  EXPECT_CALL(*listener, afterRegionLive(_))
      .WillRepeatedly(DoAll(ReleaseSem(&live_sem), AcquireSem(&shut_sem)));
  EXPECT_CALL(*listener, afterRegionDisconnected(_))
      .WillRepeatedly(DoAll(ReleaseSem(&shut_sem), AcquireSem(&live_sem)));

  {
    auto poolFactory = cache.getPoolManager()
                           .createFactory()
                           .setReadTimeout(std::chrono::seconds{1})
                           .setPingInterval(std::chrono::seconds{1})
                           .setSubscriptionEnabled(true);
    cluster.applyLocators(poolFactory);
    poolFactory.create("default");
  }

  auto region = cache.createRegionFactory(RegionShortcut::CACHING_PROXY)
                    .setPoolName("default")
                    .setCacheListener(listener)
                    .create("region");

  cache.getTypeRegistry().registerPdxType(Order::createDeserializable);

  // Create local PDX type in the registry
  region->put("2", std::make_shared<Order>(2, "product x", 37));

  std::thread thread;

  {
    // Lock the mutex so we ensure that cluster is restarted just before
    // PdxRemoteWriter is created. This way localPdxType != nullptr, but
    // the pdxTypeRegistry is cleaned up. If PdxType were instead fetched
    // from PdxTypeRegistry, this would result in a coredump.

    std::lock_guard<std::mutex> guard_{mutex};
    thread = std::thread([&region] {
      region->put("3", std::make_shared<Order>(3, "product y", 37));
    });

    std::this_thread::sleep_for(std::chrono::seconds{5});
    gfsh.shutdown().execute();

    shut_sem.acquire();
    shut_sem.release();

    std::this_thread::sleep_for(std::chrono::seconds{5});
    for (auto& server : cluster.getServers()) {
      server.start();
    }

    live_sem.acquire();
    live_sem.release();
  }
  thread.join();
}

}  // namespace
