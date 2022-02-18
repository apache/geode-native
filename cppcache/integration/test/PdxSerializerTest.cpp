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

#include <framework/Cluster.h>

#include <initializer_list>
#include <memory>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/PdxWrapper.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/TypeRegistry.hpp>

namespace {

using apache::geode::client::Cache;
using apache::geode::client::Cacheable;
using apache::geode::client::PdxReader;
using apache::geode::client::PdxSerializable;
using apache::geode::client::PdxSerializer;
using apache::geode::client::PdxWrapper;
using apache::geode::client::PdxWriter;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;
using apache::geode::client::UserObjectSizer;

static const char* CLASSNAME1 = "PdxTests.PdxType";

class TestPdxSerializer;

class NonPdxType {
 private:
  int64_t longValue;

 public:
  NonPdxType() : longValue(1) {}

  bool operator==(const NonPdxType& rhs) const {
    return longValue == rhs.longValue;
  }

  void setLongValue(int64_t value) { this->longValue = value; }

  friend TestPdxSerializer;
};

class TestPdxSerializer : public PdxSerializer {
 public:
  static size_t objectSize(const std::shared_ptr<const void>&,
                           const std::string& className) {
    EXPECT_EQ(CLASSNAME1, className);
    auto nonPdxType = std::make_shared<NonPdxType>();
    return sizeof(nonPdxType->longValue);
  }

  UserObjectSizer getObjectSizer(const std::string& className) override {
    EXPECT_EQ(CLASSNAME1, className);
    return objectSize;
  }

  std::shared_ptr<void> fromData(const std::string& className,
                                 PdxReader& pdxReader) override {
    EXPECT_EQ(CLASSNAME1, className);
    auto nonPdxType = std::make_shared<NonPdxType>();
    try {
      nonPdxType->longValue = pdxReader.readLong("longValue");
    } catch (...) {
      return nullptr;
    }

    return std::move(nonPdxType);
  }

  bool toData(const std::shared_ptr<const void>& testObject,
              const std::string& className, PdxWriter& pdxWriter) override {
    EXPECT_EQ(CLASSNAME1, className);

    auto nonPdxType = std::static_pointer_cast<const NonPdxType>(testObject);

    try {
      pdxWriter.writeLong("longValue", nonPdxType->longValue);
      pdxWriter.markIdentityField("longValue");
    } catch (...) {
      return false;
    }
    return true;
  }
};

std::shared_ptr<Region> setupRegion(Cache& cache) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .create("region");

  return region;
}

void expectNonPdxTypeEquals(const std::shared_ptr<NonPdxType>& expected,
                            const std::shared_ptr<Cacheable>& actual) {
  EXPECT_TRUE(actual);

  auto wrapper = std::dynamic_pointer_cast<PdxWrapper>(actual);
  EXPECT_TRUE(wrapper);

  auto object = wrapper->getObject();
  EXPECT_TRUE(object);

  auto nonPdxType = std::static_pointer_cast<NonPdxType>(object);
  EXPECT_TRUE(nonPdxType);

  EXPECT_EQ(*expected, *nonPdxType);
}

TEST(PdxSerializerTest, canSerializeNonPdxSerializableType) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.start();
  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto nonPdxType = std::make_shared<NonPdxType>();
  nonPdxType->setLongValue(2);

  {
    auto cache = cluster.createCache();
    auto region = setupRegion(cache);
    cache.getTypeRegistry().registerPdxSerializer(
        std::make_shared<TestPdxSerializer>());

    region->put("2", std::make_shared<PdxWrapper>(nonPdxType, CLASSNAME1));
    expectNonPdxTypeEquals(nonPdxType, region->get("2"));
  }

  {
    auto cache = cluster.createCache();
    auto region = setupRegion(cache);
    cache.getTypeRegistry().registerPdxSerializer(
        std::make_shared<TestPdxSerializer>());

    expectNonPdxTypeEquals(nonPdxType, region->get("2"));
  }
}

}  // namespace
