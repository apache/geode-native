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

#include <iostream>
#include <random>

#include <geode/CacheFactory.hpp>
#include <geode/CacheTransactionManager.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

using apache::geode::client::Cache;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheTransactionManager;
using apache::geode::client::RegionShortcut;

auto keys = {
    "Key1",
    "Key2",
    "Key3",
    "Key4",
    "Key5",
    "Key6",
    "Key7",
    "Key8",
    "Key9",
    "Key10"
};


int getValueFromExternalSystem() {
  static thread_local std::default_random_engine generator(std::random_device{}());
  auto value = std::uniform_int_distribution<int32_t>{0, 9}(generator);

  if (!value) {
    throw "failed to get from external system";
  }

  return value;
}

int main(int argc, char** argv) {
  auto cache = CacheFactory()
      .set("log-level", "none")
      .create();

  std::cout << "Created cache" << std::endl;

  cache.getPoolManager()
      .createFactory()
      .addLocator("localhost", 10334)
      .create("pool");
  
  auto regionFactory = cache.createRegionFactory(RegionShortcut::PROXY);
  auto region = regionFactory.setPoolName("pool").create("exampleRegion");

  std::cout << "Created region 'exampleRegion'" << std::endl;

  auto transactionManager = cache.getCacheTransactionManager();

  auto retries = 5;
  while (retries--) {
    try {
      transactionManager->begin();
      for (auto& key : keys) {
        auto value = getValueFromExternalSystem();
        region->put(key, value);
      }
      transactionManager->commit();
      std::cout << "Committed transaction - exiting" << std::endl;
      break;
    } catch ( ... ) {
      if (transactionManager->exists()){
        transactionManager->rollback();
      }
      std::cout << "Rolled back transaction - retrying(" << retries << ")" << std::endl;
    }
  }
}

