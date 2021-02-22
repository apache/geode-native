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
#include <geode/PdxInstanceFactory.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "LocalRegion.hpp"
#include "NestedPdxObject.hpp"
#include "PdxType.hpp"
#include "mock/CacheListenerMock.hpp"

namespace {

using apache::geode::client::Cache;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheListenerMock;
using apache::geode::client::CacheRegionHelper;
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
using PdxTests::PdxType;

using testobject::ChildPdx;
using testobject::ParentPdx;

using testing::_;
using testing::DoAll;
using testing::InvokeWithoutArgs;
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
  destination.markIdentityField("m_pdxEnum");
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
  typeRegistry.registerPdxType(Address::createDeserializable);
  typeRegistry.registerPdxType(PdxTests::PdxType::createDeserializable);
  typeRegistry.registerPdxType(ChildPdx::createDeserializable);
  typeRegistry.registerPdxType(ParentPdx::createDeserializable);

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

  auto&& cachePerfStats = std::dynamic_pointer_cast<LocalRegion>(region)
                              ->getCacheImpl()
                              ->getCachePerfStats();

  EXPECT_EQ(1, cachePerfStats.getPdxInstanceDeserializations())
      << "pdxInstanceDeserialization should be equal to 1.";

  EXPECT_EQ(0, cachePerfStats.getPdxInstanceCreations())
      << "pdxInstanceCreations should be equal to 0.";

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

  EXPECT_EQ(0, cachePerfStats.getPdxInstanceCreations())
      << "pdxInstanceCreations should be equal to 0.";

  EXPECT_LT(0, cachePerfStats.getPdxInstanceDeserializationTime())
      << "pdxInstanceDeserializationTime should be greater than 0.";

  auto pdxTypeFromPdxTypeInstanceGet =
      std::dynamic_pointer_cast<PdxTests::PdxType>(
          objectFromPdxTypeInstanceGet);
  EXPECT_TRUE(
      pdxTypeFromPdxTypeInstance->equals(*pdxTypeFromPdxTypeInstanceGet, false))
      << "PdxObjects should be equal.";

  EXPECT_EQ(-960665662, pdxTypeInstance->hashcode())
      << "Pdxhashcode hashcode not matched with java pdx hash code.";

  // TODO split into separate test for nested pdx object test.
  ParentPdx pdxParentOriginal(10);
  auto pdxParentInstanceFactory =
      cache.createPdxInstanceFactory("testobject.ParentPdx");
  clonePdxInstance(pdxParentOriginal, pdxParentInstanceFactory);
  auto pdxParentInstance = pdxParentInstanceFactory.create();
  EXPECT_EQ("testobject.ParentPdx", pdxParentInstance->getClassName())
      << "pdxTypeInstance.getClassName should return testobject.ParentPdx.";

  auto keyport = CacheableKey::create("pdxParentOriginal");
  region->put(keyport, pdxParentInstance);
  auto objectFromPdxParentInstanceGet =
      std::dynamic_pointer_cast<PdxSerializable>(region->get(keyport));

  EXPECT_EQ(1, cachePerfStats.getPdxInstanceDeserializations())
      << "pdxInstanceDeserialization should be equal to 1.";
  EXPECT_EQ(0, cachePerfStats.getPdxInstanceCreations())
      << "pdxInstanceCreations should be equal to 0.";
  EXPECT_LT(0, cachePerfStats.getPdxInstanceDeserializationTime())
      << "pdxInstanceDeserializationTime should be greater than 0.";

  auto pdxParentFromPdxParentInstnaceGet =
      std::dynamic_pointer_cast<ParentPdx>(objectFromPdxParentInstanceGet);
  EXPECT_TRUE(
      pdxParentOriginal.equals(*pdxParentFromPdxParentInstnaceGet, false))
      << "ParentPdx objects should be equal.";
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

}  // namespace
