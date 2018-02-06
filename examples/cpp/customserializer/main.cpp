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
#include "Order.hpp"
#include "OrderSerializer.hpp"

using namespace apache::geode::client;
using namespace customserializer;

int main(int argc, char **argv) {
  auto cacheFactory = CacheFactory();
  cacheFactory.set("log-level", "none");
  auto cache = cacheFactory.create();
  auto poolFactory = cache.getPoolManager().createFactory();

  poolFactory->addLocator("localhost", 10334);
  auto pool = poolFactory->create("pool");
  auto regionFactory = cache.createRegionFactory(RegionShortcut::PROXY);
  auto region = regionFactory.setPoolName("pool").create("custom_orders");

  std::shared_ptr<PdxSerializer> orderSer =
      std::shared_ptr<PdxSerializer>(new OrderSerializer());
  cache.getTypeRegistry().registerPdxSerializer(orderSer);

  auto order1 = new Order(1, "product x", 42);
  std::shared_ptr<PdxWrapper> pdxobj1(
      new PdxWrapper(order1, Order::CLASS_NAME_, orderSer));
  auto order2 = new Order(2, "product y", 37);
  std::shared_ptr<PdxWrapper> pdxobj2(
      new PdxWrapper(order2, Order::CLASS_NAME_, orderSer));

  std::cout << "Storing orders in the region" << std::endl;
  region->put("Customer1", pdxobj1);
  region->put("Customer2", pdxobj2);

  std::cout << "Getting the orders from the region" << std::endl;
  auto wrappedOrder =
      std::dynamic_pointer_cast<PdxWrapper>(region->get("Customer1"));
  auto customer1Order = reinterpret_cast<Order *>(wrappedOrder->getObject());

  customer1Order->print();

  cache.close();
}
