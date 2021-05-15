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

#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTPDXSERIALIZERS_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTPDXSERIALIZERS_H_

namespace { // NOLINT(google-build-namespaces)

using apache::geode::client::CacheableArrayList;
using apache::geode::client::CacheableHashMap;
using apache::geode::client::CacheableHashSet;
using apache::geode::client::CacheableHashTable;
using apache::geode::client::CacheableLinkedHashSet;
using apache::geode::client::CacheableVector;
using apache::geode::client::PdxReader;
using apache::geode::client::PdxSerializer;
using apache::geode::client::PdxWriter;
using apache::geode::client::UserObjectSizer;

using PdxTests::TestPdxSerializerForV2;
using PdxTests::V1CLASSNAME2;
using PdxTests::V2CLASSNAME4;

static const char* CLASSNAME1 = "PdxTests.PdxType";
static const char* CLASSNAME2 = "PdxTests.Address";

class TestPdxSerializer : public PdxSerializer {
 public:
  static void deallocate(void* testObject, const std::string& className) {
    ASSERT(className == CLASSNAME1 || className == CLASSNAME2,
           "Unexpected classname in deallocate()");
    LOG("TestPdxSerializer::deallocate called");
    if (className == CLASSNAME1) {
      PdxTests::NonPdxType* nonPdxType =
          reinterpret_cast<PdxTests::NonPdxType*>(testObject);
      delete nonPdxType;
    } else {
      PdxTests::NonPdxAddress* nonPdxAddress =
          reinterpret_cast<PdxTests::NonPdxAddress*>(testObject);
      delete nonPdxAddress;
    }
  }

  static size_t objectSize(const std::shared_ptr<const void>&,
                           const std::string& className) {
    ASSERT(className == CLASSNAME1 || className == CLASSNAME2,
           "Unexpected classname in objectSize()");
    LOG("TestPdxSerializer::objectSize called");
    return 12345;  // dummy value
  }

  UserObjectSizer getObjectSizer(const std::string& className) override {
    ASSERT(className == CLASSNAME1 || className == CLASSNAME2,
           "Unexpected classname in getObjectSizer");
    return objectSize;
  }

  std::shared_ptr<void> fromDataForAddress(PdxReader& pr) {
    try {
      auto nonPdxAddress = std::make_shared<PdxTests::NonPdxAddress>();
      nonPdxAddress->_aptNumber = pr.readInt("_aptNumber");
      nonPdxAddress->_street = pr.readString("_street");
      nonPdxAddress->_city = pr.readString("_city");
      return std::move(nonPdxAddress);
    } catch (...) {
      return nullptr;
    }
  }

  std::shared_ptr<void> fromData(const std::string& className,
                                 PdxReader& pdxReader) override {
    ASSERT(className == CLASSNAME1 || className == CLASSNAME2,
           "Unexpected classname in fromData");

    if (className == CLASSNAME2) {
      return fromDataForAddress(pdxReader);
    }

    auto nonPdxType = std::make_shared<PdxTests::NonPdxType>();

    try {
      int32_t *Lengtharr;
      int32_t arrLen = 0;
      nonPdxType->deleteByteByteArray();
      nonPdxType->m_byteByteArray = pdxReader.readArrayOfByteArrays(
          "m_byteByteArray", arrLen, &Lengtharr);
      _GEODE_SAFE_DELETE_ARRAY(Lengtharr);

      // TODO::need to write compareByteByteArray() and check for
      // m_byteByteArray elements

      nonPdxType->m_char = pdxReader.readChar("m_char");
      // GenericValCompare

      nonPdxType->m_bool = pdxReader.readBoolean("m_bool");
      // GenericValCompare
      nonPdxType->m_boolArray = pdxReader.readBooleanArray("m_boolArray");

      nonPdxType->m_byte = pdxReader.readByte("m_byte");
      nonPdxType->m_byteArray = pdxReader.readByteArray("m_byteArray");
      nonPdxType->m_charArray = pdxReader.readCharArray("m_charArray");

      nonPdxType->m_arraylist = std::dynamic_pointer_cast<CacheableArrayList>(
          pdxReader.readObject("m_arraylist"));

      nonPdxType->m_map = std::dynamic_pointer_cast<CacheableHashMap>(
          pdxReader.readObject("m_map"));
      // TODO:Check for the size

      nonPdxType->m_hashtable = std::dynamic_pointer_cast<CacheableHashTable>(
          pdxReader.readObject("m_hashtable"));
      // TODO:Check for the size

      nonPdxType->m_vector = std::dynamic_pointer_cast<CacheableVector>(
          pdxReader.readObject("m_vector"));
      // TODO::Check for size

      nonPdxType->m_chs = std::dynamic_pointer_cast<CacheableHashSet>(
          pdxReader.readObject("m_chs"));
      // TODO::Size check

      nonPdxType->m_clhs = std::dynamic_pointer_cast<CacheableLinkedHashSet>(
          pdxReader.readObject("m_clhs"));
      // TODO:Size check

      nonPdxType->m_string =
          pdxReader.readString("m_string");  // GenericValCompare
      nonPdxType->m_date = pdxReader.readDate("m_dateTime");  // compareData

      nonPdxType->m_double = pdxReader.readDouble("m_double");

      nonPdxType->m_doubleArray = pdxReader.readDoubleArray("m_doubleArray");
      nonPdxType->m_float = pdxReader.readFloat("m_float");
      nonPdxType->m_floatArray = pdxReader.readFloatArray("m_floatArray");
      nonPdxType->m_int16 = pdxReader.readShort("m_int16");
      nonPdxType->m_int32 = pdxReader.readInt("m_int32");
      nonPdxType->m_long = pdxReader.readLong("m_long");
      nonPdxType->m_int32Array = pdxReader.readIntArray("m_int32Array");
      nonPdxType->m_longArray = pdxReader.readLongArray("m_longArray");
      nonPdxType->m_int16Array = pdxReader.readShortArray("m_int16Array");
      nonPdxType->m_sbyte = pdxReader.readByte("m_sbyte");
      nonPdxType->m_sbyteArray = pdxReader.readByteArray("m_sbyteArray");
      nonPdxType->m_stringArray = pdxReader.readStringArray("m_stringArray");
      nonPdxType->m_uint16 = pdxReader.readShort("m_uint16");
      nonPdxType->m_uint32 = pdxReader.readInt("m_uint32");
      nonPdxType->m_ulong = pdxReader.readLong("m_ulong");
      nonPdxType->m_uint32Array = pdxReader.readIntArray("m_uint32Array");
      nonPdxType->m_ulongArray = pdxReader.readLongArray("m_ulongArray");
      nonPdxType->m_uint16Array = pdxReader.readShortArray("m_uint16Array");

      nonPdxType->m_byte252 = pdxReader.readByteArray("m_byte252");
      nonPdxType->m_byte253 = pdxReader.readByteArray("m_byte253");
      nonPdxType->m_byte65535 = pdxReader.readByteArray("m_byte65535");
      nonPdxType->m_byte65536 = pdxReader.readByteArray("m_byte65536");

      nonPdxType->m_pdxEnum = pdxReader.readObject("m_pdxEnum");

      nonPdxType->m_address = pdxReader.readObject("m_address");

      nonPdxType->m_objectArray = pdxReader.readObjectArray("m_objectArray");

      LOG_INFO("TestPdxSerializer: NonPdxType fromData() Done.");
    } catch (...) {
      return nullptr;
    }
    return std::move(nonPdxType);
  }

  bool toDataForAddress(const std::shared_ptr<const void>& testObject,
                        PdxWriter& pdxWriter) {
    try {
      auto nonPdxAddress =
          std::static_pointer_cast<const PdxTests::NonPdxAddress>(testObject);
      pdxWriter.writeInt("_aptNumber", nonPdxAddress->_aptNumber);
      pdxWriter.writeString("_street", nonPdxAddress->_street);
      pdxWriter.writeString("_city", nonPdxAddress->_city);
      return true;
    } catch (...) {
      return false;
    }
  }

  bool toData(const std::shared_ptr<const void>& testObject,
              const std::string& className, PdxWriter& pdxWriter) override {
    ASSERT(className == CLASSNAME1 || className == CLASSNAME2,
           "Unexpected classname in toData");

    if (className == CLASSNAME2) {
      return toDataForAddress(testObject, pdxWriter);
    }

    auto nonPdxType =
        std::static_pointer_cast<const PdxTests::NonPdxType>(testObject);

    try {
      int lengthArr[2];

      lengthArr[0] = 1;
      lengthArr[1] = 2;
      pdxWriter.writeArrayOfByteArrays(
          "m_byteByteArray", nonPdxType->m_byteByteArray, 2, lengthArr);
      pdxWriter.writeChar("m_char", nonPdxType->m_char);
      pdxWriter.markIdentityField("m_char");
      pdxWriter.writeBoolean("m_bool", nonPdxType->m_bool);  // 1
      pdxWriter.markIdentityField("m_bool");
      pdxWriter.writeBooleanArray("m_boolArray", nonPdxType->m_boolArray);
      pdxWriter.markIdentityField("m_boolArray");
      pdxWriter.writeByte("m_byte", nonPdxType->m_byte);
      pdxWriter.markIdentityField("m_byte");
      pdxWriter.writeByteArray("m_byteArray", nonPdxType->m_byteArray);
      pdxWriter.markIdentityField("m_byteArray");
      pdxWriter.writeCharArray("m_charArray", nonPdxType->m_charArray);
      pdxWriter.markIdentityField("m_charArray");
      pdxWriter.writeObject("m_arraylist", nonPdxType->m_arraylist);
      pdxWriter.markIdentityField("m_arraylist");
      pdxWriter.writeObject("m_map", nonPdxType->m_map);
      pdxWriter.markIdentityField("m_map");
      pdxWriter.writeObject("m_hashtable", nonPdxType->m_hashtable);
      pdxWriter.markIdentityField("m_hashtable");
      pdxWriter.writeObject("m_vector", nonPdxType->m_vector);
      pdxWriter.markIdentityField("m_vector");
      pdxWriter.writeObject("m_chs", nonPdxType->m_chs);
      pdxWriter.markIdentityField("m_chs");
      pdxWriter.writeObject("m_clhs", nonPdxType->m_clhs);
      pdxWriter.markIdentityField("m_clhs");
      pdxWriter.writeString("m_string", nonPdxType->m_string);
      pdxWriter.markIdentityField("m_string");
      pdxWriter.writeDate("m_dateTime", nonPdxType->m_date);
      pdxWriter.markIdentityField("m_dateTime");
      pdxWriter.writeDouble("m_double", nonPdxType->m_double);
      pdxWriter.markIdentityField("m_double");
      pdxWriter.writeDoubleArray("m_doubleArray", nonPdxType->m_doubleArray);
      pdxWriter.markIdentityField("m_doubleArray");
      pdxWriter.writeFloat("m_float", nonPdxType->m_float);
      pdxWriter.markIdentityField("m_float");
      pdxWriter.writeFloatArray("m_floatArray", nonPdxType->m_floatArray);
      pdxWriter.markIdentityField("m_floatArray");
      pdxWriter.writeShort("m_int16", nonPdxType->m_int16);
      pdxWriter.markIdentityField("m_int16");
      pdxWriter.writeInt("m_int32", nonPdxType->m_int32);
      pdxWriter.markIdentityField("m_int32");
      pdxWriter.writeLong("m_long", nonPdxType->m_long);
      pdxWriter.markIdentityField("m_long");
      pdxWriter.writeIntArray("m_int32Array", nonPdxType->m_int32Array);
      pdxWriter.markIdentityField("m_int32Array");
      pdxWriter.writeLongArray("m_longArray", nonPdxType->m_longArray);
      pdxWriter.markIdentityField("m_longArray");
      pdxWriter.writeShortArray("m_int16Array", nonPdxType->m_int16Array);
      pdxWriter.markIdentityField("m_int16Array");
      pdxWriter.writeByte("m_sbyte", nonPdxType->m_sbyte);
      pdxWriter.markIdentityField("m_sbyte");
      pdxWriter.writeByteArray("m_sbyteArray", nonPdxType->m_sbyteArray);
      pdxWriter.markIdentityField("m_sbyteArray");

      pdxWriter.writeStringArray("m_stringArray", nonPdxType->m_stringArray);
      pdxWriter.markIdentityField("m_stringArray");
      pdxWriter.writeShort("m_uint16", nonPdxType->m_uint16);
      pdxWriter.markIdentityField("m_uint16");
      pdxWriter.writeInt("m_uint32", nonPdxType->m_uint32);
      pdxWriter.markIdentityField("m_uint32");
      pdxWriter.writeLong("m_ulong", nonPdxType->m_ulong);
      pdxWriter.markIdentityField("m_ulong");
      pdxWriter.writeIntArray("m_uint32Array", nonPdxType->m_uint32Array);
      pdxWriter.markIdentityField("m_uint32Array");
      pdxWriter.writeLongArray("m_ulongArray", nonPdxType->m_ulongArray);
      pdxWriter.markIdentityField("m_ulongArray");
      pdxWriter.writeShortArray("m_uint16Array", nonPdxType->m_uint16Array);
      pdxWriter.markIdentityField("m_uint16Array");

      pdxWriter.writeByteArray("m_byte252", nonPdxType->m_byte252);
      pdxWriter.markIdentityField("m_byte252");
      pdxWriter.writeByteArray("m_byte253", nonPdxType->m_byte253);
      pdxWriter.markIdentityField("m_byte253");
      pdxWriter.writeByteArray("m_byte65535", nonPdxType->m_byte65535);
      pdxWriter.markIdentityField("m_byte65535");
      pdxWriter.writeByteArray("m_byte65536", nonPdxType->m_byte65536);
      pdxWriter.markIdentityField("m_byte65536");

      pdxWriter.writeObject("m_pdxEnum", nonPdxType->m_pdxEnum);
      pdxWriter.markIdentityField("m_pdxEnum");

      pdxWriter.writeObject("m_address", nonPdxType->m_objectArray);
      pdxWriter.writeObjectArray("m_objectArray", nonPdxType->m_objectArray);

      LOG("TestPdxSerializer: NonPdxType toData() Done......");
    } catch (...) {
      return false;
    }
    return true;
  }
};

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTPDXSERIALIZERS_H_
