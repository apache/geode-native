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
#include <sstream>

#include <geode/CacheFactory.hpp>
#include <geode/PoolManager.hpp>
#include <geode/QueryService.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/TypeRegistry.hpp>

#include "Order.hpp"

using namespace apache::geode::client;
using namespace remotequery;

int main(int argc, char** argv) {
  auto cache = CacheFactory()
      .set("log-level", "none")
      .create();

  auto pool = cache.getPoolManager()
      .createFactory()
      .addLocator("localhost", 10334)
      .create("pool");
  
  auto regionFactory = cache.createRegionFactory(RegionShortcut::PROXY);
  auto region = regionFactory.setPoolName("pool").create("custom_orders");

  cache.getTypeRegistry().registerPdxType(Order::createDeserializable);

  std::cout << "Create orders" << std::endl;
  auto order1 = std::make_shared<Order>(1, "product x", 23);
  auto order2 = std::make_shared<Order>(2, "product y", 37);
  auto order3 = std::make_shared<Order>(3, "product z", 1);
  auto order4 = std::make_shared<Order>(4, "product z", 102);
  auto order5 = std::make_shared<Order>(5, "product x", 17);
  auto order6 = std::make_shared<Order>(6, "product z", 42);

  std::cout << "Storing orders in the region" << std::endl;
  region->put("Order1", order1);
  region->put("Order2", order2);
  region->put("Order3", order3);
  region->put("Order4", order4);
  region->put("Order5", order5);
  region->put("Order6", order6);

  std::shared_ptr<QueryService> queryService = nullptr;
  queryService = pool->getQueryService();

  std::cout << "Getting the orders from the region" << std::endl;
  auto query = queryService->newQuery("SELECT * FROM /custom_orders WHERE quantity > 30");
  auto queryResults = query->execute();

  std::cout << "The following orders have a quantity greater than 30:" << std::endl;

  for (auto&& value : *queryResults) {
    auto&& order = std::dynamic_pointer_cast<Order>(value);
    std::cout << order->toString() << std::endl;
  }

  cache.close();
}
