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
#include <geode/PdxWrapper.hpp>

using namespace apache::geode::client;

class Order : public PdxSerializable {
 public:
  Order() {
    orderID = (unsigned long)0;
    quantity = (unsigned int)0;
    name = "";
  };

  Order(unsigned long inOrderID, const std::string& inName,
        unsigned int inQuantity)
      : orderID(inOrderID), name(inName), quantity(inQuantity){};

  virtual ~Order(){};

  unsigned long getOrderID() { return orderID; }
  const std::string& getName() const { return name; }
  unsigned int getQuantity() { return quantity; }

  virtual void fromData(PdxReader& pdxReader) override {
    try {
      orderID = (unsigned long)pdxReader.readLong(orderid_key);
      name = pdxReader.readString(name_key);
      quantity = (unsigned int)pdxReader.readInt(quantity_key);
    } catch (std::exception& e) {
      std::cout << "Caught exception: " << e.what() << std::endl;
      return;
    }
  }

  virtual void toData(PdxWriter& pdxWriter) const override {
    try {
      pdxWriter.writeLong(orderid_key, orderID);
      pdxWriter.markIdentityField(orderid_key);

      pdxWriter.writeString(name_key, name);
      pdxWriter.markIdentityField(name_key);

      pdxWriter.writeInt(quantity_key, quantity);
      pdxWriter.markIdentityField(quantity_key);

    } catch (std::exception& e) {
      std::cout << "Caught exception: " << e.what() << std::endl;
      return;
    }
    return;
  }

  static PdxSerializable* createDeserializable() { return new Order(); }

  virtual std::string toString() const override {
    std::stringstream ss("");
    ss << " OrderID: " << orderID << " Product Name: " << name
       << " Quantity: " << quantity;
    return ss.str();
  }

  virtual size_t objectSize() const override {
    auto objectSize = sizeof(Order);
    return objectSize;
  }

  const std::string& getClassName() const override {
    static std::string className = "com.example.Order";
    return className;
  }

 private:
  unsigned long orderID;
  std::string name;
  unsigned int quantity;

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

  cache.getTypeRegistry().registerPdxType(Order::createDeserializable);

  std::cout << "Create orders" << std::endl;
  auto order1 = std::shared_ptr<Order>(new Order(1, "product x", 23));
  auto order2 = std::shared_ptr<Order>(new Order(2, "product y", 37));

  std::cout << "Storing orders in the region" << std::endl;
  auto keyptr = CacheableKey::create("Customer1");
  region->put(keyptr, order1);
  region->put("Customer2", order2);

  std::cout << "Getting the orders from the region" << std::endl;
  auto keyptr = CacheableKey::create("Customer1");
  auto order = region->get(keyptr);

  std::cout << order->toString() << std::endl;

  cache.close();
}
