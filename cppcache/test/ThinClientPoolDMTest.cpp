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

#include <gmock/gmock.h>

#include <gtest/gtest.h>

#include "CacheImpl.hpp"
#include "TcrConnectionManager.hpp"
#include "ThinClientPoolDM.hpp"

using apache::geode::client::PoolAttributes;
using apache::geode::client::TcrConnectionManager;
using apache::geode::client::ThinClientPoolDM;

class ThinClientPoolMock : public ThinClientPoolDM {
 public:
  ThinClientPoolMock(std::shared_ptr<PoolAttributes> attr,
                     TcrConnectionManager& manager)
      : ThinClientPoolDM("TestPool", attr, manager) {}

  MOCK_METHOD0(clearPdxTypeRegistry, void());

  using ThinClientPoolDM::startClearPdxTypeRegistryThread;
  using ThinClientPoolDM::stopClearPdxTypeRegistryThread;

  void baseClearPdxTypeRegistry() { ThinClientPoolDM::clearPdxTypeRegistry(); }
};

namespace {

using apache::geode::client::AuthInitialize;
using apache::geode::client::CacheImpl;
using apache::geode::client::LogLevel;
using apache::geode::client::PdxType;
using apache::geode::client::Properties;
using ::testing::InvokeWithoutArgs;

TEST(ThinClientPoolDMTest, testClearPdxTypeRegistry) {
  const auto PDX_TYPE_ID = 1337;

  std::mutex cv_mutex;
  std::condition_variable cv;

  auto properties = Properties::create();
  properties->insert("log-level", "none");

  auto pool_attr = std::make_shared<PoolAttributes>();
  pool_attr->addLocator("198.18.0.0", 10334);
  auto cache_impl =
      std::make_shared<CacheImpl>(nullptr, properties, false, false, nullptr);
  auto& sys_prop = cache_impl->getSystemProperties();
  auto& pdx_type_registry = *cache_impl->getPdxTypeRegistry();
  TcrConnectionManager manager{cache_impl.get()};
  ThinClientPoolMock pool{pool_attr, manager};
  pool.startClearPdxTypeRegistryThread();

  auto expected_pdx_type =
      std::make_shared<PdxType>(pdx_type_registry, "TestPdx", false);
  pdx_type_registry.addPdxType(PDX_TYPE_ID, expected_pdx_type);

  auto pdx_type = pdx_type_registry.getPdxType(PDX_TYPE_ID);
  EXPECT_EQ(pdx_type, expected_pdx_type);

  EXPECT_CALL(pool, clearPdxTypeRegistry())
      .Times(1)
      .WillOnce(InvokeWithoutArgs([&]() {
        pool.baseClearPdxTypeRegistry();
        cv.notify_one();
      }));

  pool.reducePoolSize(0);

  {
    std::unique_lock<std::mutex> lock(cv_mutex);
    EXPECT_EQ(cv.wait_for(lock, std::chrono::seconds(60)),
              std::cv_status::no_timeout);
  }

  pdx_type = pdx_type_registry.getPdxType(PDX_TYPE_ID);
  EXPECT_FALSE(pdx_type);

  pool.stopClearPdxTypeRegistryThread();
}

}  // namespace
