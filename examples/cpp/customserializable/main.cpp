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
  Order() : order_id_(0), name_(), quantity_(0) {};

  Order(uint32_t order_id, std::string&& name, uint16_t quantity)
      : order_id_(order_id), name_(std::move(name)), quantity_(quantity) {}

  ~Order(){};

  virtual void fromData(PdxReader& pdxReader) override {
      order_id_ = static_cast<uint32_t>(pdxReader.readLong(ORDER_ID_KEY_));
      name_ = pdxReader.readString(NAME_KEY_);
      quantity_ = static_cast<uint16_t>(pdxReader.readInt(QUANTITY_KEY_));
  }

  virtual void toData(PdxWriter& pdxWriter) const override {
      pdxWriter.writeLong(ORDER_ID_KEY_, order_id_);
      pdxWriter.markIdentityField(ORDER_ID_KEY_);

      pdxWriter.writeString(NAME_KEY_, name_);
      pdxWriter.markIdentityField(NAME_KEY_);

      pdxWriter.writeInt(QUANTITY_KEY_, quantity_);
      pdxWriter.markIdentityField(QUANTITY_KEY_);
  }

  static PdxSerializable* createDeserializable() { return new Order(); }

  virtual std::string toString() const override {
    return "OrderID: " + std::to_string(order_id_) + " Product Name: " + name_ + " Quantity: " + std::to_string(quantity_);
  }

  virtual size_t objectSize() const override {
    auto objectSize = sizeof(Order);
    objectSize += name_.capacity();
    return objectSize;
  }

  const std::string& getClassName() const override {
    static const std::string class_name = "com.example.Order";
    return class_name;
  }

 private:
  uint32_t order_id_;
  std::string name_;
  uint16_t quantity_;

  const std::string ORDER_ID_KEY_ = "order_id";
  const std::string NAME_KEY_ = "name";
  const std::string QUANTITY_KEY_ = "quantity";
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
  region->put("Customer1", order1);
  region->put("Customer2", order2);

  std::cout << "Getting the orders from the region" << std::endl;
  auto order1retrieved = region->get("Customer1");
  auto order2retrieved = region->get("Customer2");

  std::cout << order1retrieved->toString() << std::endl;
  std::cout << order2retrieved->toString() << std::endl;

  cache.close();
}
