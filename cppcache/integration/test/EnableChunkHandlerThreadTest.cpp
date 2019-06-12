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

#include <geode/PdxReader.hpp>
#include <geode/PdxSerializable.hpp>
#include <geode/PdxWriter.hpp>
#include <geode/QueryService.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/TypeRegistry.hpp>

#include "framework/Cluster.h"

namespace {

using apache::geode::client::PdxReader;
using apache::geode::client::PdxSerializable;
using apache::geode::client::PdxWriter;
using apache::geode::client::RegionShortcut;

class SerializableWithThreadId : public PdxSerializable {
 public:
  SerializableWithThreadId() : SerializableWithThreadId(0) {}

  explicit SerializableWithThreadId(uint32_t id) : id_(id) {}

  ~SerializableWithThreadId() noexcept override = default;

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  void fromData(PdxReader& pdxReader) override {
    id_ = static_cast<uint32_t>(pdxReader.readLong("id_"));
    thread_id_ = std::this_thread::get_id();
  }

  void toData(PdxWriter& pdxWriter) const override {
    pdxWriter.writeLong("id_", id_);
    pdxWriter.markIdentityField("id_");
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<SerializableWithThreadId>();
  }

  const std::string& getClassName() const override {
    static const std::string CLASS_NAME =
        "com.example.SerializableWithThreadId";
    return CLASS_NAME;
  }

  std::thread::id getThreadId() { return thread_id_; }

 private:
  uint32_t id_;
  std::thread::id thread_id_;
};

TEST(ChunkHandlerThreadTest, isDisabledUsesSameThread) {
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

  cache.getTypeRegistry().registerPdxType(
      SerializableWithThreadId::createDeserializable);

  auto objectOne = std::make_shared<SerializableWithThreadId>(1);

  region->put("objectOne", objectOne);

  auto queryResults =
      cache.getQueryService()->newQuery("select * from /region")->execute();

  auto returnedObjectOne =
      std::dynamic_pointer_cast<SerializableWithThreadId>((*queryResults)[0]);

  ASSERT_NE(nullptr, returnedObjectOne);

  EXPECT_EQ(std::this_thread::get_id(), returnedObjectOne->getThreadId());
}

TEST(ChunkHandlerThreadTest, isEnabledUsesDifferentThread) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.start();
  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  auto cache = cluster.createCache({{"enable-chunk-handler-thread", "true"}});
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .create("region");

  cache.getTypeRegistry().registerPdxType(
      SerializableWithThreadId::createDeserializable);

  auto objectOne = std::make_shared<SerializableWithThreadId>(1);

  region->put("objectOne", objectOne);

  auto queryResults =
      cache.getQueryService()->newQuery("select * from /region")->execute();

  auto returnedObjectOne =
      std::dynamic_pointer_cast<SerializableWithThreadId>((*queryResults)[0]);

  ASSERT_NE(nullptr, returnedObjectOne);

  EXPECT_NE(std::this_thread::get_id(), returnedObjectOne->getThreadId());
}

}  // namespace
