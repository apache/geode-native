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
#include <thread>

#include <geode/CacheFactory.hpp>
#include <geode/CqAttributesFactory.hpp>
#include <geode/PoolManager.hpp>
#include <geode/QueryService.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/Struct.hpp>
#include <geode/TypeRegistry.hpp>

#include "Order.hpp"

using namespace apache::geode::client;
using namespace continuousquery;

class MyCqListener : public CqListener {
public:
  void onEvent(const CqEvent& cqEvent) override {
    auto opStr = "Default";

    auto order = dynamic_cast<Order *>(cqEvent.getNewValue().get());
    auto key(dynamic_cast<CacheableString *>(cqEvent.getKey().get()));

    switch (cqEvent.getQueryOperation()) {
    case CqOperation::OP_TYPE_CREATE:
      opStr = "CREATE";
      break;
    case CqOperation::OP_TYPE_UPDATE:
      opStr = "UPDATE";
      break;
    case CqOperation::OP_TYPE_DESTROY:
      opStr = "DESTROY";
      break;
    default:
      break;
    }

    // On a destroy event, if value dropped out of query but is still in the
    // region, it will be valid, but if it was removed altogether it will be
    // nullptr
    if (order) {
      std::cout << "MyCqListener::OnEvent called with " << opStr << ", key["
                << key->value().c_str() << "], value(" << order->getOrderId()
                << ", " << order->getName().c_str() << ", "
                << order->getQuantity() << ")" << std::endl;
    } else {
      std::cout << "MyCqListener::OnEvent called with " << opStr << ", key["
                << key->value().c_str() << "], value is nullptr" << std::endl;
    }
  }

  void onError(const CqEvent& cqEvent) override {
    std::cout << __FUNCTION__ << " called" << std::endl;
  }

  void close() override { std::cout << __FUNCTION__ << " called" << std::endl; }
};

int main(int argc, char** argv) {
  auto cache = CacheFactory()
      .set("log-level", "none")
      .create();

  auto pool = cache.getPoolManager()
      .createFactory()
      .addLocator("localhost", 10334)
      .setSubscriptionEnabled(true)
      .create("pool");
  
  auto regionFactory = cache.createRegionFactory(RegionShortcut::PROXY);

  auto region = regionFactory.setPoolName("pool").create("custom_orders");

  cache.getTypeRegistry().registerPdxType(Order::createDeserializable);

  auto queryService = pool->getQueryService();

  CqAttributesFactory cqFactory;

  auto cqListener = std::make_shared<MyCqListener>();

  cqFactory.addCqListener(cqListener);
  auto cqAttributes = cqFactory.create();

  auto query = queryService->newCq(
      "MyCq", "SELECT * FROM /custom_orders c WHERE c.quantity > 30",
      cqAttributes);

  std::cout << "Executing continuous query" << std::endl;
  query->execute();

  std::cout << "Create orders" << std::endl;
  auto order1 = std::make_shared<Order>(1, "product x", 23);
  auto order2 = std::make_shared<Order>(2, "product y", 37);
  auto order3 = std::make_shared<Order>(3, "product z", 1);
  auto order4 = std::make_shared<Order>(4, "product z", 102);
  auto order5 = std::make_shared<Order>(5, "product x", 17);
  auto order6 = std::make_shared<Order>(6, "product z", 42);

  std::cout << "Putting and changing Order objects in the region" << std::endl;
  region->put("Order1", order1);
  region->put("Order2", order2);
  region->put("Order3", order3);
  region->put("Order4", order4);
  region->put("Order5", order5);
  region->put("Order6", order6);

  region->put("Order2", std::make_shared<Order>(2, "product y", 45));
  region->put("Order2", std::make_shared<Order>(2, "product y", 29));

  region->remove("Order6");

  std::this_thread::sleep_for(std::chrono::seconds(2));

  query->stop();
  query->close();

  cache.close();
}
