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
#include <framework/Gfsh.h>

#include <memory>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/FunctionService.hpp>
#include <geode/PdxInstanceFactory.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "CacheImpl.hpp"
#include "DeliveryAddress.hpp"
#include "LocalRegion.hpp"
#include "NestedPdxObject.hpp"
#include "PdxType.hpp"
#include "mock/CacheListenerMock.hpp"

namespace {

using apache::geode::client::BooleanArray;
using apache::geode::client::Cache;
using apache::geode::client::CacheableBoolean;
using apache::geode::client::CacheableByte;
using apache::geode::client::CacheableBytes;
using apache::geode::client::CacheableCharacter;
using apache::geode::client::CacheableDouble;
using apache::geode::client::CacheableDoubleArray;
using apache::geode::client::CacheableFloat;
using apache::geode::client::CacheableFloatArray;
using apache::geode::client::CacheableInt16;
using apache::geode::client::CacheableInt16Array;
using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableInt32Array;
using apache::geode::client::CacheableInt64;
using apache::geode::client::CacheableInt64Array;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheableStringArray;
using apache::geode::client::CacheableVector;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheListenerMock;
using apache::geode::client::CacheRegionHelper;
using apache::geode::client::CharArray;
using apache::geode::client::FunctionService;
using apache::geode::client::IllegalStateException;
using apache::geode::client::LocalRegion;
using apache::geode::client::PdxInstance;
using apache::geode::client::PdxInstanceFactory;
using apache::geode::client::PdxSerializable;
using apache::geode::client::PoolFactory;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;
using apache::geode::client::SelectResults;

using PdxTests::Address;
using PdxTests::DeliveryAddress;
using PdxTests::PdxType;

using testobject::ChildPdx;
using testobject::ParentPdx;

using testing::_;
using testing::DoAll;
using testing::Eq;
using testing::InvokeWithoutArgs;
using testing::IsTrue;
using testing::NotNull;
using testing::Return;

const std::string gemfireJsonClassName = "__GEMFIRE_JSON";

Cache createTestCache() {
  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("on-client-disconnect-clear-pdxType-Ids", "true")
                   .set("statistic-sampling-enabled", "false")
                   .create();

  return cache;
}

std::shared_ptr<Region> setupRegion(Cache& cache) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .create("region");

  return region;
}

void clonePdxInstance(PdxType& source, PdxInstanceFactory& destination) {
  destination.writeBoolean("m_bool", source.getBool());
  destination.markIdentityField("m_bool");
  destination.writeByte("m_byte", source.getByte());
  destination.markIdentityField("m_byte");
  destination.writeShort("m_int16", source.getShort());
  destination.markIdentityField("m_int16");
  destination.writeInt("m_int32", source.getInt());
  destination.markIdentityField("m_int32");
  destination.writeLong("m_long", source.getLong());
  destination.markIdentityField("m_long");
  destination.writeFloat("m_float", source.getFloat());
  destination.markIdentityField("m_float");
  destination.writeDouble("m_double", source.getDouble());
  destination.markIdentityField("m_double");
  destination.writeString("m_string", source.getString());
  destination.markIdentityField("m_string");
  destination.writeDate("m_dateTime", source.getDate());
  destination.markIdentityField("m_dateTime");
  destination.writeBooleanArray("m_boolArray", source.getBoolArray());
  destination.markIdentityField("m_boolArray");
  destination.writeByteArray("m_byteArray", source.getByteArray());
  destination.markIdentityField("m_byteArray");
  destination.writeShortArray("m_int16Array", source.getShortArray());
  destination.markIdentityField("m_int16Array");
  destination.writeIntArray("m_int32Array", source.getIntArray());
  destination.markIdentityField("m_int32Array");
  destination.writeLongArray("m_longArray", source.getLongArray());
  destination.markIdentityField("m_longArray");
  destination.writeFloatArray("m_floatArray", source.getFloatArray());
  destination.markIdentityField("m_floatArray");
  destination.writeDoubleArray("m_doubleArray", source.getDoubleArray());
  destination.markIdentityField("m_doubleArray");
  destination.writeObject("m_map", source.getHashMap());
  destination.markIdentityField("m_map");
  destination.writeStringArray("m_stringArray", source.getStringArray());
  destination.markIdentityField("m_stringArray");
  destination.writeObjectArray("m_objectArray",
                               source.getCacheableObjectArray());
  destination.writeObject("m_pdxEnum", source.getEnum());
  destination.writeObject("m_arraylist", source.getArrayList());
  destination.markIdentityField("m_arraylist");
  destination.writeObject("m_linkedlist", source.getLinkedList());
  destination.markIdentityField("m_linkedlist");
  destination.writeObject("m_hashtable", source.getHashTable());
  destination.markIdentityField("m_hashtable");
  destination.writeObject("m_vector", source.getVector());
  destination.markIdentityField("m_vector");

  int lengths[2] = {1, 2};
  destination.writeArrayOfByteArrays("m_byteByteArray",
                                     source.getArrayOfByteArrays(), 2, lengths);

  destination.markIdentityField("m_byteByteArray");
  destination.writeChar("m_char", source.getChar());
  destination.markIdentityField("m_char");
  destination.writeCharArray("m_charArray", source.getCharArray());
  destination.markIdentityField("m_charArray");
  destination.writeObject("m_chs", source.getHashSet());
  destination.markIdentityField("m_chs");
  destination.writeObject("m_clhs", source.getLinkedHashSet());
  destination.markIdentityField("m_clhs");
  destination.writeByte("m_sbyte", source.getSByte());
  destination.markIdentityField("m_sbyte");
  destination.writeByteArray("m_sbyteArray", source.getSByteArray());
  destination.markIdentityField("m_sbyteArray");
  destination.writeShort("m_uint16", source.getUint16());
  destination.markIdentityField("m_uint16");
  destination.writeInt("m_uint32", source.getUInt());
  destination.markIdentityField("m_uint32");
  destination.writeLong("m_ulong", source.getULong());
  destination.markIdentityField("m_ulong");
  destination.writeShortArray("m_uint16Array", source.getUInt16Array());
  destination.markIdentityField("m_uint16Array");
  destination.writeIntArray("m_uint32Array", source.getUIntArray());
  destination.markIdentityField("m_uint32Array");
  destination.writeLongArray("m_ulongArray", source.getULongArray());
  destination.markIdentityField("m_ulongArray");

  destination.writeByteArray("m_byte252", source.getByte252());
  destination.markIdentityField("m_byte252");
  destination.writeByteArray("m_byte253", source.getByte253());
  destination.markIdentityField("m_byte253");
  destination.writeByteArray("m_byte65535", source.getByte65535());
  destination.markIdentityField("m_byte65535");
  destination.writeByteArray("m_byte65536", source.getByte65536());
  destination.markIdentityField("m_byte65536");
  destination.writeObject("m_address", source.getCacheableObjectArray());

  destination.writeObjectArray(
      "", source.getCacheableObjectArrayEmptyPdxFieldName());
}

void clonePdxInstance(ParentPdx& source, PdxInstanceFactory& destination) {
  destination.writeInt("m_parentId", source.getParentId());
  destination.writeObject("m_enum", source.getEnum());
  destination.writeString("m_parentName", source.getParentName());
  destination.writeObject("m_childPdx", source.getChildPdx());
  destination.writeChar("m_char", source.getChar());
  destination.writeChar("m_wideChar", source.getChar());
  destination.writeCharArray("m_charArray", source.getCharArray());
}

/**
 * Port of testThinClientPdxInstance::testPdxInstance
 */
TEST(PdxInstanceTest, testPdxInstance) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = cluster.createCache();
  auto region = setupRegion(cache);
  auto&& typeRegistry = cache.getTypeRegistry();
  auto&& cachePerfStats = std::dynamic_pointer_cast<LocalRegion>(region)
                              ->getCacheImpl()
                              ->getCachePerfStats();

  typeRegistry.registerPdxType(Address::createDeserializable);
  typeRegistry.registerPdxType(PdxTests::PdxType::createDeserializable);

  PdxTests::PdxType pdxTypeOriginal;
  auto&& pdxTypeInstanceFactory =
      cache.createPdxInstanceFactory("PdxTests.PdxType");

  clonePdxInstance(pdxTypeOriginal, pdxTypeInstanceFactory);

  ASSERT_THROW(
      pdxTypeInstanceFactory.writeBoolean("m_bool", pdxTypeOriginal.getBool()),
      IllegalStateException);

  auto&& pdxTypeInstance = pdxTypeInstanceFactory.create();
  ASSERT_NE(nullptr, pdxTypeInstance);

  EXPECT_EQ("PdxTests.PdxType", pdxTypeInstance->getClassName())
      << "pdxTypeInstance.getClassName should return PdxTests.PdxType.";

  auto&& objectFromPdxTypeInstance = pdxTypeInstance->getObject();
  ASSERT_NE(nullptr, objectFromPdxTypeInstance);

  EXPECT_EQ(1, cachePerfStats.getPdxInstanceDeserializations())
      << "pdxInstanceDeserialization should be equal to 1.";

  EXPECT_EQ(1, cachePerfStats.getPdxInstanceCreations())
      << "pdxInstanceCreations should be equal to 1";

  EXPECT_EQ(0, cachePerfStats.getPdxInstanceDeserializationTime())
      << "pdxInstanceDeserializationTime should be equal to 0.";

  auto pdxTypeFromPdxTypeInstance =
      std::dynamic_pointer_cast<PdxTests::PdxType>(objectFromPdxTypeInstance);
  EXPECT_TRUE(pdxTypeOriginal.equals(*pdxTypeFromPdxTypeInstance, false))
      << "PdxObjects should be equal.";

  auto pdxInstanceKey = CacheableKey::create("pdxTypeInstance");
  region->put(pdxInstanceKey, pdxTypeInstance);

  auto objectFromPdxTypeInstanceGet =
      std::dynamic_pointer_cast<PdxSerializable>(region->get(pdxInstanceKey));
  ASSERT_NE(nullptr, objectFromPdxTypeInstanceGet);

  EXPECT_EQ(1, cachePerfStats.getPdxInstanceDeserializations())
      << "pdxInstanceDeserialization should be equal to 1.";

  EXPECT_EQ(1, cachePerfStats.getPdxInstanceCreations())
      << "pdxInstanceCreations should be equal to 1.";

  EXPECT_LT(0, cachePerfStats.getPdxInstanceDeserializationTime())
      << "pdxInstanceDeserializationTime should be less than 0.";

  auto pdxTypeFromPdxTypeInstanceGet =
      std::dynamic_pointer_cast<PdxTests::PdxType>(
          objectFromPdxTypeInstanceGet);
  ASSERT_THAT(
      pdxTypeFromPdxTypeInstance->equals(*pdxTypeFromPdxTypeInstanceGet, false),
      IsTrue())
      << "PdxObjects should be equal.";
}

TEST(PdxInstanceTest, testNestedPdxInstance) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = cluster.createCache();
  auto region = setupRegion(cache);
  auto&& typeRegistry = cache.getTypeRegistry();
  auto&& cachePerfStats = std::dynamic_pointer_cast<LocalRegion>(region)
                              ->getCacheImpl()
                              ->getCachePerfStats();

  typeRegistry.registerPdxType(ChildPdx::createDeserializable);
  typeRegistry.registerPdxType(ParentPdx::createDeserializable);

  ParentPdx original{10};
  auto factory = cache.createPdxInstanceFactory(original.getClassName());
  clonePdxInstance(original, factory);
  auto pdxInstance = factory.create();

  auto keyport = CacheableKey::create("pdxParentOriginal");
  region->put(keyport, pdxInstance);
  auto object =
      std::dynamic_pointer_cast<PdxSerializable>(region->get(keyport));
  EXPECT_TRUE(object);

  EXPECT_EQ(0, cachePerfStats.getPdxInstanceDeserializations())
      << "pdxInstanceDeserialization should be equal to 0";
  EXPECT_EQ(1, cachePerfStats.getPdxInstanceCreations())
      << "pdxInstanceCreations should be equal to 1";
  EXPECT_LT(0, cachePerfStats.getPdxInstanceDeserializationTime())
      << "pdxInstanceDeserializationTime should be less than 0";

  auto parentPdx = std::dynamic_pointer_cast<ParentPdx>(object);
  EXPECT_TRUE(parentPdx);
  EXPECT_TRUE(original.equals(*parentPdx, false))
      << "ParentPdx objects should be equal";
}

TEST(PdxInstanceTest, testHashCode) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start([&]() {
    cluster.getGfsh()
        .deploy()
        .jar(getFrameworkString(FrameworkVariable::JavaObjectJarPath))
        .execute();
  });

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = cluster.createCache();
  auto region = setupRegion(cache);
  auto&& typeRegistry = cache.getTypeRegistry();
  auto&& cachePerfStats = std::dynamic_pointer_cast<LocalRegion>(region)
                              ->getCacheImpl()
                              ->getCachePerfStats();

  {
    PdxType serialized;
    auto&& factory = cache.createPdxInstanceFactory("Test", false);

    clonePdxInstance(serialized, factory);
    auto&& instance = factory.create();

    auto collector =
        FunctionService::onRegion(region).withArgs(instance).execute(
            "HashCodeFunction");
    ASSERT_THAT(collector, NotNull());

    auto resultSet = collector->getResult();
    ASSERT_THAT(resultSet, NotNull());

    ASSERT_THAT(resultSet->size(), Eq(1));

    auto hashCode = std::dynamic_pointer_cast<CacheableInt32>((*resultSet)[0]);
    ASSERT_THAT(hashCode, NotNull());

    auto clientHashCode = instance->hashcode();
    auto serverHashCode = hashCode->value();
    ASSERT_THAT(clientHashCode, Eq(serverHashCode));
  }
}

TEST(PdxInstanceTest, testHashCodeDefaultBytes) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start([&]() {
    cluster.getGfsh()
        .deploy()
        .jar(getFrameworkString(FrameworkVariable::JavaObjectJarPath))
        .execute();
  });

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = cluster.createCache();
  auto region = setupRegion(cache);
  auto&& typeRegistry = cache.getTypeRegistry();
  auto&& cachePerfStats = std::dynamic_pointer_cast<LocalRegion>(region)
                              ->getCacheImpl()
                              ->getCachePerfStats();

  {
    PdxType serialized;
    auto&& factory = cache.createPdxInstanceFactory("Test", false);

    factory.writeBoolean("m_defaultBoolean", false);
    factory.writeByte("m_defaultByte", 0);
    factory.writeShort("m_defaultShort", 0);
    factory.writeInt("m_defaultInt", 0);
    factory.writeLong("m_defaultLong", 0);
    factory.writeChar("m_defaultChar", u'\0');
    factory.writeFloat("m_defaultFloat", 0.0f);
    factory.writeDouble("m_defaultDouble", 0.0);
    factory.writeString("m_defaultString", std::string{});
    factory.writeDate("m_defaultDate", nullptr);
    factory.writeByteArray("m_defaultByteArray", std::vector<int8_t>{});
    factory.writeShortArray("m_defaultShortArray", std::vector<int16_t>{});
    factory.writeCharArray("m_defaultCharArray", std::vector<char16_t>{});
    factory.writeIntArray("m_defaultIntArray", std::vector<int32_t>{});
    factory.writeLongArray("m_defaultLongArray", std::vector<int64_t>{});
    factory.writeFloatArray("m_defaultFloatArray", std::vector<float>{});
    factory.writeDoubleArray("m_defaultDoubleArray", std::vector<double>{});
    factory.writeStringArray("m_defaultStringArray",
                             std::vector<std::string>{});
    factory.writeObjectArray("m_defaultObjectArray", nullptr);
    factory.writeObject("m_defaultObject", nullptr);

    auto&& instance = factory.create();

    auto collector =
        FunctionService::onRegion(region).withArgs(instance).execute(
            "HashCodeFunction");
    ASSERT_THAT(collector, NotNull());

    auto resultSet = collector->getResult();
    ASSERT_THAT(resultSet, NotNull());

    ASSERT_THAT(resultSet->size(), Eq(1));

    auto hashCode = std::dynamic_pointer_cast<CacheableInt32>((*resultSet)[0]);
    ASSERT_THAT(hashCode, NotNull());

    auto clientHashCode = instance->hashcode();
    auto serverHashCode = hashCode->value();
    ASSERT_THAT(clientHashCode, Eq(serverHashCode));
  }
}

TEST(PdxInstanceTest, testHashCodeObject) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start([&]() {
    cluster.getGfsh()
        .deploy()
        .jar(getFrameworkString(FrameworkVariable::JavaObjectJarPath))
        .execute();
  });

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = cluster.createCache();
  auto region = setupRegion(cache);
  auto&& typeRegistry = cache.getTypeRegistry();
  auto&& cachePerfStats = std::dynamic_pointer_cast<LocalRegion>(region)
                              ->getCacheImpl()
                              ->getCachePerfStats();

  {
    PdxType serialized;
    auto&& factory = cache.createPdxInstanceFactory("Test", false);

    factory.writeObject("m_boolean",
                        CacheableBoolean::create(serialized.getBool()));
    factory.writeObject("m_char",
                        CacheableCharacter::create(serialized.getChar()));
    factory.writeObject("m_byte", CacheableByte::create(serialized.getByte()));
    factory.writeObject("m_short",
                        CacheableInt16::create(serialized.getShort()));
    factory.writeObject("m_int", CacheableInt32::create(serialized.getInt()));
    factory.writeObject("m_long", CacheableInt64::create(serialized.getLong()));
    factory.writeObject("m_float",
                        CacheableFloat::create(serialized.getFloat()));
    factory.writeObject("m_double",
                        CacheableDouble::create(serialized.getDouble()));
    auto&& instance = factory.create();

    auto collector =
        FunctionService::onRegion(region).withArgs(instance).execute(
            "HashCodeFunction");
    ASSERT_THAT(collector, NotNull());

    auto resultSet = collector->getResult();
    ASSERT_THAT(resultSet, NotNull());

    ASSERT_THAT(resultSet->size(), Eq(1));

    auto hashCode = std::dynamic_pointer_cast<CacheableInt32>((*resultSet)[0]);
    ASSERT_THAT(hashCode, NotNull());

    auto clientHashCode = instance->hashcode();
    auto serverHashCode = hashCode->value();
    ASSERT_THAT(clientHashCode, Eq(serverHashCode));
  }
}

/**
 * This test cover some of the edge cases for hash calculation
 */
TEST(PdxInstanceTest, testHashCodeEdgeCases) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start([&]() {
    cluster.getGfsh()
        .deploy()
        .jar(getFrameworkString(FrameworkVariable::JavaObjectJarPath))
        .execute();
  });

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = cluster.createCache();
  auto region = setupRegion(cache);
  auto&& typeRegistry = cache.getTypeRegistry();
  auto&& cachePerfStats = std::dynamic_pointer_cast<LocalRegion>(region)
                              ->getCacheImpl()
                              ->getCachePerfStats();

  {
    auto&& factory = cache.createPdxInstanceFactory("Test", false);

    // This string hashCode should be 0
    factory.writeObject("m_string_1", CacheableString::create("("));
    factory.writeObject("m_string_2", CacheableString::create("0"));
    factory.writeObject("m_string_3", CacheableString::create("&"));
    factory.writeObject("m_string_4", CacheableString::create("P"));
    factory.writeObject("m_string_5", CacheableString::create("I"));
    factory.writeObject("m_string_6", CacheableString::create("2"));
    factory.writeObject("m_string_7", CacheableString::create("<"));
    auto&& instance = factory.create();

    auto collector =
        FunctionService::onRegion(region).withArgs(instance).execute(
            "HashCodeFunction");
    ASSERT_THAT(collector, NotNull());

    auto resultSet = collector->getResult();
    ASSERT_THAT(resultSet, NotNull());

    ASSERT_THAT(resultSet->size(), Eq(1));

    auto hashCode = std::dynamic_pointer_cast<CacheableInt32>((*resultSet)[0]);
    ASSERT_THAT(hashCode, NotNull());

    auto clientHashCode = instance->hashcode();
    auto serverHashCode = hashCode->value();
    ASSERT_THAT(clientHashCode, Eq(serverHashCode));
  }
}

TEST(PdxInstanceTest, testSerializationCycle) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start([&]() {
    cluster.getGfsh()
        .deploy()
        .jar(getFrameworkString(FrameworkVariable::JavaObjectJarPath))
        .execute();
  });

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  std::shared_ptr<PdxInstance> pdxInstance;
  {
    auto cache = cluster.createCache();
    auto region = setupRegion(cache);
    auto&& typeRegistry = cache.getTypeRegistry();
    auto&& cachePerfStats = std::dynamic_pointer_cast<LocalRegion>(region)
                                ->getCacheImpl()
                                ->getCachePerfStats();

    {
      PdxTests::PdxType serializable;
      auto&& factory =
          cache.createPdxInstanceFactory("PdxTests.PdxType", false);

      clonePdxInstance(serializable, factory);

      pdxInstance = factory.create();
    }
    ASSERT_THAT(pdxInstance, NotNull());

    region->put("entry-1", pdxInstance);
    {
      auto filter = CacheableVector::create();
      filter->push_back(CacheableKey::create("entry-2"));

      auto collector = FunctionService::onRegion(region)
                           .withFilter(filter)
                           .withArgs(pdxInstance)
                           .execute("PutKeyFunction");
      ASSERT_THAT(collector, NotNull());

      auto resultSet = collector->getResult();
      ASSERT_THAT(resultSet, NotNull());
      ASSERT_THAT(resultSet->size(), Eq(1));

      auto result =
          std::dynamic_pointer_cast<CacheableBoolean>((*resultSet)[0]);
      ASSERT_THAT(result, NotNull());
      ASSERT_THAT(result->value(), Eq(true));
    }
  }

  {
    auto cache = cluster.createCache();
    auto region = setupRegion(cache);
    auto&& typeRegistry = cache.getTypeRegistry();
    auto&& cachePerfStats = std::dynamic_pointer_cast<LocalRegion>(region)
                                ->getCacheImpl()
                                ->getCachePerfStats();

    typeRegistry.registerPdxType(Address::createDeserializable);
    typeRegistry.registerPdxType(PdxTests::PdxType::createDeserializable);

    auto&& entry = region->get("entry-2");
    ASSERT_THAT(entry, NotNull());

    auto instance = std::dynamic_pointer_cast<PdxInstance>(entry);
    ASSERT_THAT(instance, NotNull());

    ASSERT_THAT(*instance, Eq(std::ref(*pdxInstance)));
  }
}

TEST(PdxInstanceTest, testCreateJsonInstance) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.start();
  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache = cluster.createCache();
  auto region = setupRegion(cache);
  auto pdxInstanceFactory =
      cache.createPdxInstanceFactory(gemfireJsonClassName);

  pdxInstanceFactory.writeObject("foo",
                                 CacheableString::create(std::string("bar")));
  auto pdxInstance = pdxInstanceFactory.create();

  region->put("simpleObject", pdxInstance);

  auto retrievedValue = region->get("simpleObject");
}

TEST(PdxInstanceTest, testInstancePutAfterRestart) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.start();

  auto& gfsh = cluster.getGfsh();
  gfsh.create().region().withName("region").withType("REPLICATE").execute();

  auto cache = createTestCache();
  auto poolFactory = cache.getPoolManager()
                         .createFactory()
                         .setSubscriptionEnabled(true)
                         .setPingInterval(std::chrono::seconds{2});
  cluster.applyLocators(poolFactory);
  poolFactory.create("default");

  bool status = false;
  std::mutex mutex_status;
  std::condition_variable cv_status;
  auto listener = std::make_shared<CacheListenerMock>();
  EXPECT_CALL(*listener, afterCreate(_)).WillRepeatedly(Return());
  EXPECT_CALL(*listener, afterRegionLive(_))
      .WillRepeatedly(InvokeWithoutArgs([&status, &cv_status] {
        status = true;
        cv_status.notify_one();
      }));
  EXPECT_CALL(*listener, afterRegionDisconnected(_))
      .WillRepeatedly(InvokeWithoutArgs([&status, &cv_status] {
        status = false;
        cv_status.notify_one();
      }));
  EXPECT_CALL(*listener, afterRegionDestroy(_)).WillRepeatedly(Return());
  EXPECT_CALL(*listener, close(_)).WillRepeatedly(Return());

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .setCacheListener(listener)
                    .create("region");

  std::shared_ptr<PdxInstance> first_instance;
  std::shared_ptr<PdxInstance> second_instance;

  {
    auto pdxInstanceFactory =
        cache.createPdxInstanceFactory(gemfireJsonClassName, false);

    pdxInstanceFactory.writeObject("foo",
                                   CacheableString::create(std::string("bar")));
    first_instance = pdxInstanceFactory.create();
  }

  {
    auto pdxInstanceFactory =
        cache.createPdxInstanceFactory(gemfireJsonClassName, false);

    pdxInstanceFactory.writeObject("random",
                                   CacheableString::create(std::string("bar")));

    pdxInstanceFactory.writeInt("bar", -1);
    second_instance = pdxInstanceFactory.create();
  }

  region->put("first_instance", first_instance);
  region->put("second_instance", second_instance);

  gfsh.shutdown().execute();

  {
    std::unique_lock<std::mutex> lock(mutex_status);
    cv_status.wait(lock, [&status] { return !status; });
  }

  std::this_thread::sleep_for(std::chrono::seconds{30});

  for (auto& server : cluster.getServers()) {
    server.start();
  }

  {
    std::unique_lock<std::mutex> lock(mutex_status);
    cv_status.wait(lock, [&status] { return status; });
  }

  EXPECT_NO_THROW(region->put("first_instance", first_instance));
  EXPECT_NO_THROW(region->put("second_instance", second_instance));

  auto qs = cache.getQueryService();
  auto q = qs->newQuery("SELECT * FROM /region WHERE bar = -1");

  decltype(q->execute()) result;
  EXPECT_NO_THROW(result = q->execute());
  EXPECT_TRUE(result);
  EXPECT_EQ(result->size(), 1UL);
}

TEST(PdxInstanceTest, testReadFromPdxSerializable) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.start();
  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  std::shared_ptr<DeliveryAddress> entryV2;

  {
    auto cache = cluster.createCache();
    auto region = setupRegion(cache);
    cache.getTypeRegistry().registerPdxType(
        DeliveryAddress::createDeserializable);

    DeliveryAddress::setSerializationVersion(DeliveryAddress::VERSION_2);
    entryV2 = std::make_shared<DeliveryAddress>(
        "Some address line", "Some city", "Some country", "Some instructions",
        std::shared_ptr<CacheableVector>{});

    region->put("entry.v2", entryV2);
  }

  {
    auto cache = cluster.createCache();
    auto region = setupRegion(cache);
    auto entry = region->get("entry.v2");
    ASSERT_THAT(entry, NotNull());

    auto pdxInstance = std::dynamic_pointer_cast<PdxInstance>(entry);
    ASSERT_THAT(pdxInstance, NotNull());

    ASSERT_THAT(pdxInstance->getClassName(), Eq(entryV2->getClassName()));
    ASSERT_THAT(pdxInstance->getStringField("address"),
                Eq(entryV2->getAddressLine()));
    ASSERT_THAT(pdxInstance->getStringField("city"), Eq(entryV2->getCity()));
    ASSERT_THAT(pdxInstance->getStringField("country"),
                Eq(entryV2->getCountry()));
    ASSERT_THAT(pdxInstance->getStringField("instructions"),
                Eq(entryV2->getInstructions()));
  }
}

}  // namespace
