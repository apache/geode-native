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

#include <thread>

#include <gtest/gtest.h>

#include <geode/DataSerializable.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/TypeRegistry.hpp>

#include "framework/Cluster.h"

namespace {

using namespace apache::geode::client;

class DataSerializableObject : public DataSerializable {
 public:
  inline DataSerializableObject()
      : DataSerializableObject("TestDataSerializableObject") {}

  inline DataSerializableObject(std::string name) : name_(std::move(name)) {}

  ~DataSerializableObject() noexcept override = default;

  using DataSerializable::fromData;
  using DataSerializable::toData;

  void fromData(DataInput& dataInput) override {
    name_ = dataInput.readString();
  }

  void toData(DataOutput& dataOutput) const override {
    dataOutput.writeString(name_);
  }

  static std::shared_ptr<DataSerializable> createDeserializable() {
    return std::make_shared<DataSerializableObject>();
  }

  std::string getName() { return name_; }

 private:
  std::string name_;
};

TEST(DataSerializableTest, isSerializableAndDeserializable) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  auto cache = cluster.createCache();
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .create("region");

  cache.getTypeRegistry().registerType(
      DataSerializableObject::createDeserializable, 0x04);

  auto objectOne = std::make_shared<DataSerializableObject>("one");

  region->put("objectOne", objectOne);

  auto returnedObjectOne = std::dynamic_pointer_cast<DataSerializableObject>(
      region->get("objectOne"));

  ASSERT_NE(nullptr, returnedObjectOne);

  EXPECT_EQ("one", returnedObjectOne->getName());
}
}  // namespace
