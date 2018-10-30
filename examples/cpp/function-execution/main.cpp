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

/*
 * This example takes the following steps:
 *
 * 1. Create a Geode Cache, Pool, and example Region Programmatically.
 * 3. Populate some objects on the Region.
 * 4. Create Execute Object
 * 5. Execute Function
 */
#include <iostream>
#include <memory>
#include <sstream>

#include <geode/CacheFactory.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/FunctionService.hpp>
#include <geode/CacheableString.hpp>

using apache::geode::client::Cache;
using apache::geode::client::CacheableArrayList;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheableVector;
using apache::geode::client::CacheFactory;
using apache::geode::client::Exception;
using apache::geode::client::FunctionService;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

const auto getFuncIName = std::string("MultiGetFunctionI");

const int EXAMPLE_ITEM_COUNT  = 6;
const int EXAMPLE_SERVER_PORT = 50505;

Cache setupCache() {
  return CacheFactory()
      .set("log-level", "none")
      .create();
}

void createPool(const Cache& cache) {
  auto pool = cache.getPoolManager()
      .createFactory()
      .setSubscriptionEnabled(true)
      .addServer("localhost", EXAMPLE_SERVER_PORT)
      .create("pool");
}

std::shared_ptr<Region> createRegion(Cache& cache) {
  auto regionFactory = cache.createRegionFactory(RegionShortcut::PROXY);
  auto regPtr0 = regionFactory.setPoolName("pool").create("partition_region");

  return regPtr0;
}

void populateRegion(const std::shared_ptr<Region>& regionPtr) {
  for (int i = 0; i < EXAMPLE_ITEM_COUNT; i++) {
    std::stringstream keyStream;
    std::stringstream valueStream;
    keyStream << "KEY--" << i;
    valueStream << "VALUE--" << i;
    auto value(CacheableString::create(valueStream.str().c_str()));

    auto key = CacheableKey::create(keyStream.str().c_str());
    regionPtr->put(key, value);
  }
}

std::shared_ptr<CacheableVector> populateQueryObject() {
  auto routingObj = CacheableVector::create();
  for (int i = 1; i < EXAMPLE_ITEM_COUNT; i+=2) {
    std::stringstream keyStream;
    keyStream << "KEY--" << i;
    auto key = CacheableKey::create(keyStream.str().c_str());
    routingObj->push_back(key);
  }
  return routingObj;
}

std::vector<std::string> executeFunctionOnServer(const std::shared_ptr<Region> regionPtr,
    const std::shared_ptr<CacheableVector> queryObject) {
  std::vector<std::string> resultList;

  auto functionService = FunctionService::onServer(regionPtr->getRegionService());
  if(auto executeFunctionResult = functionService.withArgs(queryObject).execute(getFuncIName)->getResult()) {
    for (auto &arrayList: *executeFunctionResult) {
//        auto newList = std::dynamic_pointer_cast<CacheableArrayList>(arrayList);
//        std::for_each(newList->begin(), newList->end(), [&resultList](std::shared_ptr<Cacheable> val) {
//          resultList.push_back(std::dynamic_pointer_cast<CacheableString>(val)->value());
//        });
      for (auto &cachedString: *std::dynamic_pointer_cast<CacheableArrayList>(arrayList)) {
        resultList.push_back(std::dynamic_pointer_cast<CacheableString>(cachedString)->value());
      }
    }
  } else {
    std::cout << "get executeFunctionResult is NULL\n";
  }

  return resultList;
}

void printResults(const std::vector<std::string>& resultList) {
  std::cout << "Result count = " << resultList.size() << std::endl << std::endl;
  int i = 0;
  for (auto &cachedString: resultList) {
    std::cout << "\tResult[" << i << "]=" << cachedString << std::endl;
    ++i;
  }
}

int main(int argc, char** argv) {
  try {
    auto cache = setupCache();

    createPool(cache);

    auto regionPtr = createRegion(cache);

    populateRegion(regionPtr);

    auto queryObject = populateQueryObject();

    auto resultList = executeFunctionOnServer(regionPtr, queryObject);

    printResults(resultList);
  }
  catch (const Exception& geodeExcp) {
    std::cerr << "Function Execution Geode Exception: " << geodeExcp.getMessage() << '\n';
    return 1;
  }
}

