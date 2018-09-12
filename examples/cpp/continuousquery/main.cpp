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

//TODO: plug in real logging
void log(const char* fmt, ...) {

}

#define LOGINFO log

class MyCqListener : public CqListener {
 public:
  void onEvent(const CqEvent& cqe)
  {
    auto opStr = "Default";

    auto order = dynamic_cast<Order*>(cqe.getNewValue().get());
    auto key( dynamic_cast<CacheableString*> (cqe.getKey().get() ));

    switch (cqe.getQueryOperation())
    {
      case CqOperation::OP_TYPE_CREATE:
      {
        opStr = "CREATE";
        break;
      }
      case CqOperation::OP_TYPE_UPDATE:
      {
        opStr = "UPDATE";
        break;
      }
      case CqOperation::OP_TYPE_DESTROY:
      {
        opStr = "UPDATE";
        break;
      }
      default:
        break;
    }
    std::cout << "MyCqListener::OnEvent called with " << opStr << ", key[" <<
      key->value().c_str() << "], value(" << order->getOrderId() << ", " <<
      order->getName().c_str() << ", " << order->getQuantity() << ")" << std::endl;
  }

  void onError(const CqEvent& cqe){
    std::cout << "MyCqListener::OnError called" << std::endl;
  }

  void close(){
    std::cout << "MyCqListener::close called" << std::endl;
  }
};

// The CqQuery QuickStart example.
int main(int argc, char ** argv) {
  try {
    auto cacheFactory = CacheFactory();
    cacheFactory.set("log-level", "none");
    auto cache = cacheFactory.create();
    auto poolFactory = cache.getPoolManager().createFactory();
    auto pool = poolFactory.addLocator("localhost", 10334)
        .setSubscriptionEnabled(true)
        .create("pool");

    LOGINFO("Created the GemFire Cache");

    auto regionFactory = cache.createRegionFactory(RegionShortcut::PROXY);

    LOGINFO("Created the RegionFactory");

    auto region = regionFactory.setPoolName("pool").create("custom_orders");

    // Register our Serializable/Cacheable Query objects, viz. Portfolio and Position.
    cache.getTypeRegistry().registerPdxType(Order::createDeserializable);

    LOGINFO("Registered Serializable Query Objects");

    // Populate the Region with some Portfolio objects.
    std::cout << "Create orders" << std::endl;
    auto order1 = std::make_shared<Order>(1, "product x", 23);
    auto order2 = std::make_shared<Order>(2, "product y", 37);
    auto order3 = std::make_shared<Order>(3, "product z", 1);
    auto order4 = std::make_shared<Order>(4, "product z", 102);
    auto order5 = std::make_shared<Order>(5, "product x", 17);
    auto order6 = std::make_shared<Order>(6, "product z", 42);

    LOGINFO("Populated some Portfolio Objects");

    // Get the QueryService from the Cache.
    std::shared_ptr<QueryService> queryService = pool->getQueryService();

    std::cout << "Getting the orders from the region" << std::endl;

    CqAttributesFactory cqFac;

    //Create CqAttributes and Install Listener
    std::shared_ptr<MyCqListener> cqLstner = std::make_shared<MyCqListener>();

    cqFac.addCqListener(cqLstner);
    std::shared_ptr<CqAttributes> cqAttr = cqFac.create();

    //create a new Cq Query
    auto qry = queryService->newCq("MyCq", "SELECT * FROM /custom_orders c WHERE c.quantity > 30", cqAttr);

    //execute Cq Query with initial Results
    auto resultsPtr  = qry->executeWithInitialResults();

    //make change to generate cq events
    std::cout << "Storing orders in the region" << std::endl;
    region->put("Order1", order1);
    region->put("Order2", order2);
    region->put("Order3", order3);
    region->put("Order4", order4);
    region->put("Order5", order5);
    region->put("Order6", order6);

    std::cout << "ResultSet Query returned " << resultsPtr->size() << " rows" << std::endl;

    // Iterate through the rows of the query result.
    for (auto&& item : *resultsPtr) {
      std::cout << "query pulled object " << item->toString() << std::endl;

      auto stPtr = dynamic_cast<Struct*>(item.get());
      if (stPtr) {
        LOGINFO(" got struct ptr ");
        auto serKey = (*stPtr)["key"];
        if (serKey) {
          std::cout << "got struct key " << serKey->toString() << std::endl;
        }

        auto serVal = (*stPtr)["value"];

        if (serVal) {
          std::cout << "got struct value " << serVal->toString() << std::endl;
        }
      }
    }

    qry->stop();
    qry->close();

    cache.close();

    std::cout << "Closed the GemFire Cache" << std::endl;
  }
  catch(const Exception & gemfireExcp) {
    std::cout << "ERROR: CqQuery GemFire Exception: " << gemfireExcp.getMessage() << std::endl;
  }
}