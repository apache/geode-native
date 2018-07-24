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
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/TypeRegistry.hpp>

#include "Order.hpp"

using namespace apache::geode::client;
using namespace customserializable;

int main(int argc, char** argv) {
  auto cacheFactory = CacheFactory();
  cacheFactory.set("log-level", "none");
  auto cache = cacheFactory.create();

  auto poolFactory = cache.getPoolManager().createFactory();
  poolFactory.addLocator("localhost", 10334);
  auto pool = poolFactory.create("pool");

  auto regionFactory = cache.createRegionFactory(RegionShortcut::PROXY);
  auto region = regionFactory.setPoolName("pool").create("custom_orders");

  cache.getTypeRegistry().registerType(Order::createDeserializable);

  std::cout << "Create orders" << std::endl;
  auto order1 = std::make_shared<Order>(1, "product x", 23);
  auto order2 = std::make_shared<Order>(2, "product y", 37);

  std::cout << "Storing orders in the region" << std::endl;
  region->put("Customer1", order1);
  region->put("Customer2", order2);

  std::cout << "Getting the orders from the region" << std::endl;

  if (auto order1retrieved =
          std::dynamic_pointer_cast<Order>(region->get("Customer1"))) {
    std::cout << "OrderID: " << order1retrieved->getOrderId() << std::endl;
    std::cout << "Product Name: " << order1retrieved->getName() << std::endl;
    std::cout << "Quantity: " << order1retrieved->getQuantity() << std::endl;
  } else {
    std::cout << "Order 1 not found." << std::endl;
  }

  if (auto order2retrieved = region->get("Customer2")) {
    std::cout << order2retrieved->toString() << std::endl;
  } else {
    std::cout << "Order 2 not found." << std::endl;
  }

  cache.close();
}
