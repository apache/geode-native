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
#include <geode/PoolManager.hpp>
#include <geode/PdxSerializer.hpp>
#include <geode/PdxWrapper.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/TypeRegistry.hpp>

#include "Order.hpp"
#include "OrderSerializer.hpp"

using namespace apache::geode::client;
using namespace customserializer;

int main(int argc, char **argv) {
  auto cache = CacheFactory()
      .set("log-level", "none")
      .create();

  cache.getPoolManager()
      .createFactory()
      .addLocator("localhost", 10334)
      .create("pool");
  
  auto regionFactory = cache.createRegionFactory(RegionShortcut::PROXY);
  auto region = regionFactory.setPoolName("pool").create("custom_orders");

  auto orderSer = std::make_shared<OrderSerializer>();
  cache.getTypeRegistry().registerPdxSerializer(orderSer);

  auto order1 = std::make_shared<Order>(1, "product x", 42);
  auto pdxobj1 = std::make_shared<PdxWrapper>(
      order1, OrderSerializer::CLASS_NAME_);
  auto order2 = std::make_shared<Order>(2, "product y", 37);
  auto pdxobj2 = std::make_shared<PdxWrapper>(
      order2, OrderSerializer::CLASS_NAME_);

  std::cout << "Storing orders in the region" << std::endl;
  region->put("Customer1", pdxobj1);
  region->put("Customer2", pdxobj2);

  std::cout << "Getting the orders from the region" << std::endl;
  auto wrappedOrder =
      std::dynamic_pointer_cast<PdxWrapper>(region->get("Customer1"));
  auto customer1Order =
      std::static_pointer_cast<Order>(wrappedOrder->getObject());

  customer1Order->print();

  cache.close();
}
