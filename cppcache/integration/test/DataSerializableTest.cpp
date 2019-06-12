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

#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>
#include <geode/DataSerializable.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/TypeRegistry.hpp>

#include "framework/Cluster.h"

namespace {

using apache::geode::client::CacheableString;
using apache::geode::client::CacheableStringArray;
using apache::geode::client::DataInput;
using apache::geode::client::DataOutput;
using apache::geode::client::DataSerializable;
using apache::geode::client::RegionShortcut;

class Simple : public DataSerializable {
 public:
  inline Simple() : Simple("TestSimple", 30) {}

  inline Simple(std::string name, int age)
      : name_(std::move(name)), age_(age) {}

  ~Simple() noexcept override = default;

  using DataSerializable::fromData;
  using DataSerializable::toData;

  void fromData(DataInput& dataInput) override {
    name_ = dataInput.readString();
    age_ = dataInput.readInt32();
  }

  void toData(DataOutput& dataOutput) const override {
    dataOutput.writeString(name_);
    dataOutput.writeInt(age_);
  }

  static std::shared_ptr<DataSerializable> createDeserializable() {
    return std::make_shared<Simple>();
  }

  std::string getName() { return name_; }
  int getAge() { return age_; }

 private:
  std::string name_;
  int age_;
};

class DataSerializableObject : public DataSerializable {
 public:
  inline DataSerializableObject()
      : DataSerializableObject("TestDataSerializableObject", nullptr, nullptr) {
  }

  inline DataSerializableObject(std::string name,
                                std::shared_ptr<CacheableStringArray> csArray,
                                std::shared_ptr<Simple> simple)
      : name_(std::move(name)), csArray_(csArray), simple_(simple) {}

  ~DataSerializableObject() noexcept override = default;

  using DataSerializable::fromData;
  using DataSerializable::toData;

  void fromData(DataInput& dataInput) override {
    name_ = dataInput.readString();
    csArray_ =
        std::dynamic_pointer_cast<CacheableStringArray>(dataInput.readObject());

    simple_ = std::dynamic_pointer_cast<Simple>(dataInput.readObject());
  }

  void toData(DataOutput& dataOutput) const override {
    dataOutput.writeString(name_);
    dataOutput.writeObject(csArray_);
    dataOutput.writeObject(simple_);
  }

  static std::shared_ptr<DataSerializable> createDeserializable() {
    return std::make_shared<DataSerializableObject>();
  }

  std::string getName() { return name_; }
  std::shared_ptr<CacheableStringArray> getCSArray() { return csArray_; }
  std::shared_ptr<Simple> getSimple() { return simple_; }

 private:
  std::string name_;
  std::shared_ptr<CacheableStringArray> csArray_;
  std::shared_ptr<Simple> simple_;
};

TEST(DataSerializableTest, isSerializableAndDeserializable) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.start();
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

  std::vector<std::shared_ptr<CacheableString>> cstr{
      CacheableString::create("Taaa"), CacheableString::create("Tbbb"),
      CacheableString::create("Tccc"), CacheableString::create("Tddd")};
  auto cacheStrArray = CacheableStringArray::create(cstr);

  auto dsObject = std::make_shared<DataSerializableObject>(
      "name", cacheStrArray, std::make_shared<Simple>("simple", 10));

  cache.getTypeRegistry().registerType(
      DataSerializableObject::createDeserializable, 77);
  cache.getTypeRegistry().registerType(Simple::createDeserializable, 78);

  region->put("objectOne", dsObject);

  auto returnedObject = std::dynamic_pointer_cast<DataSerializableObject>(
      region->get("objectOne"));

  ASSERT_NE(nullptr, returnedObject);
  EXPECT_EQ(dsObject->getName(), returnedObject->getName());
  EXPECT_EQ(dsObject->getSimple()->getName(),
            returnedObject->getSimple()->getName());
  EXPECT_EQ(dsObject->getSimple()->getAge(),
            returnedObject->getSimple()->getAge());
  auto originalArray = dsObject->getCSArray();
  auto returnedArray = returnedObject->getCSArray();
  for (uint32_t index = 0; index < 4; ++index) {
    EXPECT_EQ(originalArray->operator[](index)->toString(),
              returnedArray->operator[](index)->toString());
  }
}
}  // namespace
