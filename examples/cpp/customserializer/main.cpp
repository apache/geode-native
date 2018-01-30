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

using namespace apache::geode::client;

static const std::string CLASSNAME = "com.example.Order";

class Order {
 public:
  Order(unsigned long inOrderID, const std::string& inName,
        unsigned int inQuantity)
      : orderID(inOrderID), name(inName), quantity(inQuantity){};
  ~Order(){};
  unsigned long getOrderID() { return orderID; }
  const std::string& getName() const { return name; }
  unsigned int getQuantity() { return quantity; }
  size_t getSize() const {
    size_t size = name.length();
    size += sizeof(orderID);
    size += sizeof(quantity);
    return size;
  }
  void print() {
    std::cout << " OrderID: " << orderID << std::endl
              << " Product Name: " << name << std::endl
              << " Quantity: " << quantity << std::endl;
  }

 private:
  unsigned long orderID;
  std::string name;
  unsigned int quantity;
};

class OrderSerializer : public PdxSerializer {
 public:
  OrderSerializer() {}

  virtual ~OrderSerializer() {}

  virtual void* fromData(const std::string& className, PdxReader& pdxReader) {
    if (className != CLASSNAME) {
      return nullptr;
    }

    try {
      auto myOrder = new Order((unsigned long)pdxReader.readLong(orderid_key),
                               pdxReader.readString(name_key),
                               (unsigned int)pdxReader.readInt(quantity_key));
      return myOrder;
    } catch (std::exception& e) {
      std::cout << "Caught exception: " << e.what() << std::endl;
      return nullptr;
    }
  }

  static void deallocate(void* testObject, const std::string& className) {
    if (className == CLASSNAME) {
      Order* order = reinterpret_cast<Order*>(testObject);
      delete order;
    }
  }

  static size_t objectSize(const void* testObject,
                           const std::string& className) {
    size_t size = 0;
    if (className == CLASSNAME) {
      auto order = reinterpret_cast<const Order*>(testObject);
      size = order->getSize();
    }
    return size;
  }

  virtual bool toData(void* userObject, const std::string& className,
                      PdxWriter& pdxWriter) {
    if (className != CLASSNAME) {
      return false;
    }

    try {
      auto myOrder = static_cast<Order*>(userObject);
      pdxWriter.writeLong(orderid_key, myOrder->getOrderID());
      pdxWriter.writeString(name_key, myOrder->getName());
      pdxWriter.writeInt(quantity_key, myOrder->getQuantity());
    } catch (std::exception& e) {
      std::cout << "Caught exception: " << e.what() << std::endl;
      return false;
    }
    return true;
  }

  UserDeallocator getDeallocator(const std::string& className) {
    if (className == CLASSNAME) {
      return deallocate;
    }
    return nullptr;
  }

  UserObjectSizer getObjectSizer(const std::string& className) {
    if (className == CLASSNAME) {
      return objectSize;
    }
    return nullptr;
  }

 private:
  const std::string orderid_key = "orderid";
  const std::string name_key = "name";
  const std::string quantity_key = "quantity";
};

int main(int argc, char** argv) {
  auto cacheFactory = CacheFactory();
  cacheFactory.set("log-level", "none");
  auto cache = cacheFactory.create();
  auto poolFactory = cache.getPoolManager().createFactory();

  poolFactory->addLocator("localhost", 10334);
  auto pool = poolFactory->create("pool");
  auto regionFactory = cache.createRegionFactory(PROXY);
  auto region = regionFactory.setPoolName("pool").create("custom_orders");

  std::shared_ptr<PdxSerializer> orderSer =
      std::shared_ptr<PdxSerializer>(new OrderSerializer());
  cache.getTypeRegistry().registerPdxSerializer(orderSer);

  auto order1 = new Order(1, "product x", 42);
  std::shared_ptr<PdxWrapper> pdxobj1(
      new PdxWrapper(order1, CLASSNAME, orderSer));
  auto order2 = new Order(2, "product y", 37);
  std::shared_ptr<PdxWrapper> pdxobj2(
      new PdxWrapper(order2, CLASSNAME, orderSer));

  std::cout << "Storing orders in the region" << std::endl;
  region->put("Customer1", pdxobj1);
  region->put("Customer2", pdxobj2);

  std::cout << "Getting the orders from the region" << std::endl;
  auto wrappedOrder =
      std::dynamic_pointer_cast<PdxWrapper>(region->get("Customer1"));
  auto customer1Order = reinterpret_cast<Order*>(wrappedOrder->getObject());

  customer1Order->print();

  cache.close();
}
