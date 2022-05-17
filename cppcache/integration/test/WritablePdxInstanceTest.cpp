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
#include <gmock/gmock.h>

#include <memory>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/FunctionService.hpp>
#include <geode/PdxInstanceFactory.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/WritablePdxInstance.hpp>

#include "CacheImpl.hpp"
#include "PdxType.hpp"

namespace {

using apache::geode::client::Cache;
using apache::geode::client::CacheableBoolean;
using apache::geode::client::CacheableByte;
using apache::geode::client::CacheableBytes;
using apache::geode::client::CacheableCharacter;
using apache::geode::client::CacheableDate;
using apache::geode::client::IllegalStateException;
using apache::geode::client::PdxInstance;
using apache::geode::client::PdxInstanceFactory;
using apache::geode::client::PoolFactory;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;
using apache::geode::client::SelectResults;
using apache::geode::client::Serializable;

using PdxTests::PdxType;

using ::testing::Eq;
using ::testing::IsTrue;
using ::testing::NotNull;

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

/**
 * Port of testThinClientPdxInstance::testPdxInstance
 */
TEST(WriteablePdxInstanceTest, testWriteablePdxInstance) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  PdxType serializable;
  auto cache = cluster.createCache();
  auto region = setupRegion(cache);
  {
    auto&& factory = cache.createPdxInstanceFactory("Test");
    clonePdxInstance(serializable, factory);
    region->put("entry.v1", factory.create());
  }
  {
    auto&& entry = region->get("entry.v1");
    ASSERT_THAT(entry, NotNull());

    auto&& instance = std::dynamic_pointer_cast<PdxInstance>(entry);
    ASSERT_THAT(instance, NotNull());

    auto&& writer = instance->createWriter();
    writer->setField("m_bool", false);
    writer->setField("m_byte", static_cast<int8_t>(0x70));
    writer->setField("m_char", static_cast<char16_t>(u'λ'));
    writer->setField("m_int16", static_cast<int16_t>(0x5aa1));
    writer->setField("m_int32", static_cast<int32_t>(0x123456));
    writer->setField("m_long", static_cast<int64_t>(0x987654321));
    writer->setField("m_float", static_cast<float>(3.1f));
    writer->setField("m_double", static_cast<double>(6.2));
    writer->setField("m_string", static_cast<std::string>("Some other string"));
    writer->setField("m_dateTime", CacheableDate::create(127001241));

    region->put("entry.v2", writer);
  }
  {
    auto&& entry = region->get("entry.v2");
    ASSERT_THAT(entry, NotNull());

    auto&& instance = std::dynamic_pointer_cast<PdxInstance>(entry);
    ASSERT_THAT(instance, NotNull());

    ASSERT_THAT(instance->getBooleanField("m_bool"), Eq(false));
    ASSERT_THAT(instance->getByteField("m_byte"), Eq(0x70));
    ASSERT_THAT(instance->getCharField("m_char"), Eq(u'λ'));
    ASSERT_THAT(instance->getShortField("m_int16"), Eq(0x5aa1));
    ASSERT_THAT(instance->getIntField("m_int32"), Eq(0x123456));
    ASSERT_THAT(instance->getLongField("m_long"), Eq(0x987654321));
    ASSERT_THAT(instance->getFloatField("m_float"), Eq(3.1f));
    ASSERT_THAT(instance->getDoubleField("m_double"), Eq(6.2));
    ASSERT_THAT(instance->getStringField("m_string"), Eq("Some other string"));
    {
      auto&& date = instance->getCacheableDateField("m_dateTime");
      ASSERT_THAT(date, NotNull());
      ASSERT_THAT(date->milliseconds(), Eq(127001241000));
    }
  }
}

}  // namespace
