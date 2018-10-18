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
 * The Execute Function QuickStart Example.
 *
 * This example takes the following steps:
 *
 * 1. Create a Geode Cache.
 * 2. Create the example Region Programmatically.
 * 3. Populate some objects on the Region.
 * 4. Create Execute Objects
 * 5. Execute Functions
 * 6. Close the Cache.
 *
 */

// Include the Geode library.
#include <iostream>
#include <memory>

#include <geode/CacheFactory.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/FunctionService.hpp>
#include <geode/CacheableString.hpp>

// Use the "geode" namespace.
using namespace apache::geode::client;

const auto getFuncIName = std::string("MultiGetFunctionI");
const auto putFuncIName = std::string("MultiPutFunctionI");
const auto getFuncName = std::string("MultiGetFunction");
const auto putFuncName = std::string("MultiPutFunction");

// The Execute Function QuickStart example.
int main(int argc, char** argv) {
  try {
    // Create CacheFactory using the settings from the geode.properties file by
    // default.
    auto cacheFactory = CacheFactory();
    cacheFactory.set("log-level", "none");
    auto cache = cacheFactory.create();

    std::cout << "Created CacheFactory\n";

    auto poolFactory = cache.getPoolManager().createFactory();
    poolFactory.setSubscriptionEnabled(true)
      .addServer("localhost", 50505)
      .addServer("localhost", 40404);
    auto pool = poolFactory.create("pool");


    // Create the example Region Programmatically
    auto regionFactory = cache.createRegionFactory(RegionShortcut::PROXY);
    auto regPtr0 = regionFactory.setPoolName("pool").create("partition_region");

    std::cout << "Created the Region\n";

    regPtr0->registerAllKeys();
    char buf[128];

    auto resultList = CacheableVector::create();
    for (int i = 0; i < 34; i++) {
      sprintf(buf, "VALUE--%d", i);
      auto value(CacheableString::create(buf));

      sprintf(buf, "KEY--%d", i);
      auto key = CacheableKey::create(buf);
      regPtr0->put(key, value);
    }

    auto routingObj = CacheableVector::create();
    for (int i = 0; i < 34; i++) {
      if (i % 2 == 0) continue;
      sprintf(buf, "KEY--%d", i);
      auto key = CacheableKey::create(buf);
      routingObj->push_back(key);
    }

    std::cout << "test data independent function with result on one server\n";
    auto args = routingObj;
    auto exc = FunctionService::onServer(regPtr0->getRegionService());
    auto executeFunctionResult = exc.withArgs(args).execute(getFuncIName)->getResult();
    if (!executeFunctionResult) {
      std::cout << "get executeFunctionResult is NULL\n";
    } else {
      for (int32_t item = 0; item < executeFunctionResult->size(); item++) {
        auto arrayList = std::dynamic_pointer_cast<CacheableArrayList>(executeFunctionResult->operator[](item));
        for (int32_t pos = 0; pos < arrayList->size(); pos++) {
          resultList->push_back(arrayList->operator[](pos));
        }
      }
      sprintf(buf, "get: result count = %lu\n", resultList->size());
      std::cout << buf;
      for (int32_t i = 0; i < executeFunctionResult->size(); i++) {
        sprintf(
            buf, "get result[%d]=%s\n", i,
            std::dynamic_pointer_cast<CacheableString>(resultList->operator[](i))->toString().c_str());
        std::cout << buf;
      }
    }

    std::cout << "test data independent function without result on one server\n";

    exc.withArgs(args).execute(putFuncIName, std::chrono::milliseconds(15));

    std::cout << "test data independent function with result on all servers\n";

    exc = FunctionService::onServers(regPtr0->getRegionService());
    executeFunctionResult =
        exc.withArgs(args).execute(getFuncIName)->getResult();
    if (!executeFunctionResult) {
      std::cout << "get executeFunctionResult is NULL\n";
    } else {
      resultList->clear();
      for (int32_t item = 0; item < executeFunctionResult->size(); item++) {
        auto arrayList = std::dynamic_pointer_cast<CacheableArrayList>(executeFunctionResult->operator[](item));
        for (int32_t pos = 0; pos < arrayList->size(); pos++) {
          resultList->push_back(arrayList->operator[](pos));
        }
      }
      sprintf(buf, "get: result count = %lu\n", resultList->size());
      std::cout << buf;
      for (int32_t i = 0; i < executeFunctionResult->size(); i++) {
        sprintf(
            buf, "get result[%d]=%s\n", i,
            std::dynamic_pointer_cast<CacheableString>(resultList->operator[](i))->toString().c_str());
        std::cout << buf;
      }
    }


    std::cout << "test data independent function without result on all servers\n";
    exc.withArgs(args).execute(putFuncIName, std::chrono::milliseconds(15));
    std::cout << "test data dependent function with result\n";

    auto args2 = CacheableBoolean::create(1);
    exc = FunctionService::onRegion(regPtr0);
    executeFunctionResult = exc.withFilter(routingObj)
                                .withArgs(args)
                                .execute(getFuncName)
                                ->getResult();
    if (!executeFunctionResult) {
      std::cout << "execute on region: executeFunctionResult is NULL\n";
    } else {
      resultList->clear();
      std::cout << "Execute on Region: result count = " << executeFunctionResult->size() << '\n';
      for (int32_t i = 0; i < executeFunctionResult->size(); i++) {
        auto arrayList = std::dynamic_pointer_cast<CacheableArrayList>(executeFunctionResult->operator[](i));
        for (int32_t pos = 0; pos < arrayList->size(); pos++) {
          resultList->push_back(arrayList->operator[](pos));
        }
      }
      sprintf(buf, "Execute on Region: result count = %lu\n", resultList->size());
      std::cout << buf;

      for (int32_t i = 0; i < resultList->size(); i++) {
        sprintf(
            buf, "Execute on Region: result[%d]=%s\n", i,
            std::dynamic_pointer_cast<CacheableString>(resultList->operator[](i))->toString().c_str());
        std::cout << buf;
      }
    }

    std::cout << "test data dependent function without result\n";

    exc.withFilter(routingObj).withArgs(args).execute(putFuncName, std::chrono::milliseconds(15));

    // Close the Geode Cache.
    cache.close();
    std::cout << "Closed the Geode Cache\n";

    return 0;
  }
  // An exception should not occur
  catch (const Exception& geodeExcp) {
    std::cerr << "Function Execution Geode Exception: " << geodeExcp.getMessage() << '\n';

    return 1;
  }
}
