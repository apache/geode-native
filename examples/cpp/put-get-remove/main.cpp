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

// Include the Geode libraries.
#include <geode/CacheFactory.hpp>
#include <geode/PoolManager.hpp>

// Use the "geode" namespace.
using namespace apache::geode::client;


int main(int argc, char** argv) {

  auto cacheFactory = CacheFactory();
  cacheFactory.set("log-level", "none");
  auto cache = cacheFactory.create();

  auto poolFactory = cache.getPoolManager().createFactory();
  poolFactory->addLocator("localhost", 10334);
  auto pool = poolFactory->create("pool");

  auto regionFactory = cache.createRegionFactory(PROXY);

  auto region = regionFactory.setPoolName("pool").create("orders");


  std::cout << "Storing orders in the region" << std::endl;
  region->put("a123",5);
  region->put("a124",7);


  std::cout << "Getting the orders from the region" << std::endl;
  auto a123 = region->get("a123");
  auto a124 = region->get("a124");
  std::cout << "a123 = " << std::dynamic_pointer_cast<CacheableInt32>(a123)->value() << std::endl;
  std::cout << "a124 = " << std::dynamic_pointer_cast<CacheableInt32>(a124)->value() << std::endl;


  std::cout << "Removing order a123's info from the region" << std::endl;
  region->remove("a123");
  if(region->existsValue("a123")) {
    std::cout << "a123's info not deleted" << std::endl;
  }
  else {
    std::cout << "a123's info successfully deleted" << std::endl;
  }

  cache.close();
}
