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

#include <future>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <thread>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/TypeRegistry.hpp>
#include <geode/PdxInstanceFactory.hpp>

#include <PdxType.hpp>
#include <NestedPdxObject.hpp>

#include "framework/Cluster.h"
#include "framework/Gfsh.h"

namespace {

using namespace apache::geode::client;
using namespace std::chrono;
using namespace PdxTests;
using namespace testobject;

std::shared_ptr<Region> setupRegion(Cache& cache) {
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("default")
                    .create("region");

  return region;
}

/**
 * Port of testThinClientPdxInstance::testPdxInstance
 */
TEST(PdxInstanceTest, testPdxInstance) {
  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("REPLICATE")
      .execute();

  auto cache1 = cluster.createCache();
  auto region1 = setupRegion(cache1);

  //  auto cache2 = cluster.createCache();
  //  auto region2 = setupRegion(cache2);

  auto&& typeRegistry = cache1.getTypeRegistry();
  typeRegistry.registerPdxType(Address::createDeserializable);
  typeRegistry.registerPdxType(PdxTests::PdxType::createDeserializable);
  typeRegistry.registerPdxType(ChildPdx::createDeserializable);
  typeRegistry.registerPdxType(ParentPdx::createDeserializable);

  auto pdxobj = std::make_shared<PdxTests::PdxType>();

  auto&& pdxInstanceFactory =
      cache1.createPdxInstanceFactory("PdxTests.PdxType");

  pdxInstanceFactory->writeBoolean("m_bool", pdxobj->getBool());
  ASSERT_THROW(pdxInstanceFactory->writeBoolean("m_bool", pdxobj->getBool()),
               IllegalStateException);

  pdxInstanceFactory->markIdentityField("m_bool");
  pdxInstanceFactory->writeByte("m_byte", pdxobj->getByte());
  pdxInstanceFactory->markIdentityField("m_byte");
  pdxInstanceFactory->writeShort("m_int16", pdxobj->getShort());
  pdxInstanceFactory->markIdentityField("m_int16");
  pdxInstanceFactory->writeInt("m_int32", pdxobj->getInt());
  pdxInstanceFactory->markIdentityField("m_int32");
  pdxInstanceFactory->writeLong("m_long", pdxobj->getLong());
  pdxInstanceFactory->markIdentityField("m_long");
  pdxInstanceFactory->writeFloat("m_float", pdxobj->getFloat());
  pdxInstanceFactory->markIdentityField("m_float");
  pdxInstanceFactory->writeDouble("m_double", pdxobj->getDouble());
  pdxInstanceFactory->markIdentityField("m_double");
  pdxInstanceFactory->writeString("m_string", pdxobj->getString());
  pdxInstanceFactory->markIdentityField("m_string");
  pdxInstanceFactory->writeDate("m_dateTime", pdxobj->getDate());
  pdxInstanceFactory->markIdentityField("m_dateTime");
  pdxInstanceFactory->writeBooleanArray("m_boolArray", pdxobj->getBoolArray());
  pdxInstanceFactory->markIdentityField("m_boolArray");
  pdxInstanceFactory->writeByteArray("m_byteArray", pdxobj->getByteArray());
  pdxInstanceFactory->markIdentityField("m_byteArray");
  pdxInstanceFactory->writeShortArray("m_int16Array", pdxobj->getShortArray());
  pdxInstanceFactory->markIdentityField("m_int16Array");
  pdxInstanceFactory->writeIntArray("m_int32Array", pdxobj->getIntArray());
  pdxInstanceFactory->markIdentityField("m_int32Array");
  pdxInstanceFactory->writeLongArray("m_longArray", pdxobj->getLongArray());
  pdxInstanceFactory->markIdentityField("m_longArray");
  pdxInstanceFactory->writeFloatArray("m_floatArray", pdxobj->getFloatArray());
  pdxInstanceFactory->markIdentityField("m_floatArray");
  pdxInstanceFactory->writeDoubleArray("m_doubleArray",
                                       pdxobj->getDoubleArray());
  pdxInstanceFactory->markIdentityField("m_doubleArray");
  pdxInstanceFactory->writeObject("m_map", pdxobj->getHashMap());
  pdxInstanceFactory->markIdentityField("m_map");
  pdxInstanceFactory->writeStringArray("m_stringArray",
                                       pdxobj->getStringArray());
  pdxInstanceFactory->markIdentityField("m_stringArray");
  pdxInstanceFactory->writeObjectArray("m_objectArray",
                                       pdxobj->getCacheableObjectArray());
  pdxInstanceFactory->writeObject("m_pdxEnum", pdxobj->getEnum());
  pdxInstanceFactory->markIdentityField("m_pdxEnum");
  pdxInstanceFactory->writeObject("m_arraylist", pdxobj->getArrayList());
  pdxInstanceFactory->markIdentityField("m_arraylist");
  pdxInstanceFactory->writeObject("m_linkedlist", pdxobj->getLinkedList());
  pdxInstanceFactory->markIdentityField("m_linkedlist");
  pdxInstanceFactory->writeObject("m_hashtable", pdxobj->getHashTable());
  pdxInstanceFactory->markIdentityField("m_hashtable");
  pdxInstanceFactory->writeObject("m_vector", pdxobj->getVector());
  pdxInstanceFactory->markIdentityField("m_vector");

  auto lengths = new int[2] {1, 2};
  pdxInstanceFactory->writeArrayOfByteArrays(
      "m_byteByteArray", pdxobj->getArrayOfByteArrays(), 2, lengths);

  pdxInstanceFactory->markIdentityField("m_byteByteArray");
  pdxInstanceFactory->writeChar("m_char", pdxobj->getChar());
  pdxInstanceFactory->markIdentityField("m_char");
  pdxInstanceFactory->writeCharArray("m_charArray", pdxobj->getCharArray());
  pdxInstanceFactory->markIdentityField("m_charArray");
  pdxInstanceFactory->writeObject("m_chs", pdxobj->getHashSet());
  pdxInstanceFactory->markIdentityField("m_chs");
  pdxInstanceFactory->writeObject("m_clhs", pdxobj->getLinkedHashSet());
  pdxInstanceFactory->markIdentityField("m_clhs");
  pdxInstanceFactory->writeByte("m_sbyte", pdxobj->getSByte());
  pdxInstanceFactory->markIdentityField("m_sbyte");
  pdxInstanceFactory->writeByteArray("m_sbyteArray", pdxobj->getSByteArray());
  pdxInstanceFactory->markIdentityField("m_sbyteArray");
  pdxInstanceFactory->writeShort("m_uint16", pdxobj->getUint16());
  pdxInstanceFactory->markIdentityField("m_uint16");
  pdxInstanceFactory->writeInt("m_uint32", pdxobj->getUInt());
  pdxInstanceFactory->markIdentityField("m_uint32");
  pdxInstanceFactory->writeLong("m_ulong", pdxobj->getULong());
  pdxInstanceFactory->markIdentityField("m_ulong");
  pdxInstanceFactory->writeShortArray("m_uint16Array",
                                      pdxobj->getUInt16Array());
  pdxInstanceFactory->markIdentityField("m_uint16Array");
  pdxInstanceFactory->writeIntArray("m_uint32Array", pdxobj->getUIntArray());
  pdxInstanceFactory->markIdentityField("m_uint32Array");
  pdxInstanceFactory->writeLongArray("m_ulongArray", pdxobj->getULongArray());
  pdxInstanceFactory->markIdentityField("m_ulongArray");

  pdxInstanceFactory->writeByteArray("m_byte252", pdxobj->getByte252());
  pdxInstanceFactory->markIdentityField("m_byte252");
  pdxInstanceFactory->writeByteArray("m_byte253", pdxobj->getByte253());
  pdxInstanceFactory->markIdentityField("m_byte253");
  pdxInstanceFactory->writeByteArray("m_byte65535", pdxobj->getByte65535());
  pdxInstanceFactory->markIdentityField("m_byte65535");
  pdxInstanceFactory->writeByteArray("m_byte65536", pdxobj->getByte65536());
  pdxInstanceFactory->markIdentityField("m_byte65536");
  pdxInstanceFactory->writeObject("m_address",
                                  pdxobj->getCacheableObjectArray());

  pdxInstanceFactory->writeObjectArray(
      "", pdxobj->getCacheableObjectArrayEmptyPdxFieldName());

  auto&& pdxInstance = pdxInstanceFactory->create();
  ASSERT_NE(nullptr, pdxInstance);

  EXPECT_EQ("PdxTests.PdxType", pdxInstance->getClassName())
      << "pdxInstance.getClassName should return PdxTests.PdxType.";
}

}  // namespace
