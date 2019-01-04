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

#include <geode/CacheFactory.hpp>
#include <geode/CacheTransactionManager.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

using namespace apache::geode::client;

int main(int argc, char** argv) {
  try {
    auto cacheFactory = CacheFactory();
    cacheFactory.set("log-level", "none");
    auto cache = cacheFactory.create();
    auto poolFactory = cache.getPoolManager().createFactory();

    std::cout << "Created cache" << std::endl;

    poolFactory.addLocator("localhost", 10334);
    auto pool = poolFactory.create("pool");
    auto regionFactory = cache.createRegionFactory(RegionShortcut::PROXY);
    auto region = regionFactory.setPoolName("pool").create("exampleRegion");

    std::cout << "Created region 'exampleRegion'" << std::endl;

    auto txManager = cache.getCacheTransactionManager();

    txManager->begin();

    std::cout << "Began transaction #1" << std::endl;

    region->put("Key1", "Value1");
    region->put("Key2", "Value2");

    try {
      txManager->commit();
      std::cout << "Committed transaction #1" << std::endl;
    } catch (const CommitConflictException&) {
      std::cout << "Transaction #1 CommitConflictException!" << std::endl;
      return 1;
    }

    if (region->containsKeyOnServer(CacheableKey::create("Key1"))) {
      std::cout << "Obtained the first entry from the Region" << std::endl;
    }
    else {
      std::cout << "ERROR: First entry not found" << std::endl;
    }

    if (region->containsKeyOnServer(CacheableKey::create("Key2"))) {
      std::cout << "Obtained the second entry from the Region" << std::endl;
    }
    else {
      std::cout << "ERROR: Second entry not found" << std::endl;
    }

    txManager->begin();

    std::cout << "Began transaction #2" << std::endl;

    region->put("Key3", "Value3");

    region->destroy("Key1");

    txManager->rollback();

    std::cout << "Rolled back transaction #2" << std::endl;

    if (region->containsKeyOnServer(CacheableKey::create("Key1"))) {
      std::cout << "Obtained the first entry from the Region" << std::endl;
    }
    else {
      std::cout << "ERROR: second entry not found!" << std::endl;
    }

    if (region->containsKeyOnServer(CacheableKey::create("Key2"))) {
      std::cout << "Obtained the second entry from the Region" << std::endl;
    }
    else {
      std::cout << "ERROR: second entry not found!" << std::endl;
    }

    if (region->containsKeyOnServer(CacheableKey::create("Key3"))) {
      std::cout << "ERROR: Obtained the third entry from the Region" << std::endl;
    }
    else {
      std::cout << "Third entry not found" << std::endl;
    }

    cache.close();
  }
  catch (const Exception& ex) {
    std::cout << "Transaction Geode Exception: " << ex.getMessage() << std::endl;
  }
}
