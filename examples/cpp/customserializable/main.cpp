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
#include <geode/PdxWrapper.hpp>
#include <geode/PoolManager.hpp>

#include "Order.hpp"

using namespace apache::geode::client;
using namespace customserializable;

int main(int argc, char **argv) {
  auto cacheFactory = CacheFactory();
  cacheFactory.set("log-level", "none");
  auto cache = cacheFactory.create();

  auto poolFactory = cache.getPoolManager().createFactory();
  poolFactory->addLocator("localhost", 10334);
  auto pool = poolFactory->create("pool");

  auto regionFactory = cache.createRegionFactory(RegionShortcut::PROXY);
  auto region = regionFactory.setPoolName("pool").create("custom_orders");

  cache.getTypeRegistry().registerPdxType(Order::createDeserializable);

  std::cout << "Create orders" << std::endl;
  auto order1 = std::shared_ptr<Order>(new Order(1, "product x", 23));
  auto order2 = std::shared_ptr<Order>(new Order(2, "product y", 37));

  std::cout << "Storing orders in the region" << std::endl;
  region->put("Customer1", order1);
  region->put("Customer2", order2);

  std::cout << "Getting the orders from the region" << std::endl;
  auto order1retrieved = region->get("Customer1");
  auto order2retrieved = region->get("Customer2");

  std::cout << order1retrieved->toString() << std::endl;
  std::cout << order2retrieved->toString() << std::endl;

  cache.close();
}
