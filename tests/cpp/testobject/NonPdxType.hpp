#pragma once

#ifndef GEODE_TESTOBJECT_NONPDXTYPE_H_
#define GEODE_TESTOBJECT_NONPDXTYPE_H_

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
/*
 * NonPdxType.hpp
 *
 *  Created on: Apr 30, 2012
 *      Author: vrao
 */

#include <util/Log.hpp>

#include <geode/CacheableEnum.hpp>
#include <geode/CacheableObjectArray.hpp>
#include <geode/PdxWrapper.hpp>

#include "SerializationRegistry.hpp"
#include "testobject_export.h"

namespace PdxTests {

using apache::geode::client::Cacheable;
using apache::geode::client::CacheableArrayList;
using apache::geode::client::CacheableDate;
using apache::geode::client::CacheableEnum;
using apache::geode::client::CacheableHashMap;
using apache::geode::client::CacheableHashSet;
using apache::geode::client::CacheableHashTable;
using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableLinkedHashSet;
using apache::geode::client::CacheableObjectArray;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheableVector;
using apache::geode::client::IllegalStateException;
using apache::geode::client::PdxWrapper;
using apache::geode::client::Serializable;

class TESTOBJECT_EXPORT NonPdxAddress {
 public:
  int32_t _aptNumber;
  std::string _street;
  std::string _city;

 public:
  NonPdxAddress() {}

  NonPdxAddress(int32_t aptN, const char* street, const char* city) {
    _aptNumber = aptN;
    _street = street;
    _city = city;
  }

  bool equals(NonPdxAddress& other) const {
    LOG_DEBUG("Inside NonPdxAddress equals");
    NonPdxAddress* ot = dynamic_cast<NonPdxAddress*>(&other);
    if (!ot) {
      return false;
    }
    if (ot == this) {
      return true;
    }
    if (ot->_aptNumber != _aptNumber) {
      return false;
    }
    if (ot->_street != _street) {
      return false;
    }
    if (ot->_city != _city) {
      return false;
    }

    return true;
  }
};

enum pdxEnumTest { pdx1, pdx2, pdx3 };

class TESTOBJECT_EXPORT NonPdxType {
 public:
  char16_t m_char;
  bool m_bool;
  int8_t m_byte;
  int8_t m_sbyte;  //
  int16_t m_int16;
  int16_t m_uint16;  ///
  int32_t m_int32;
  int32_t m_uint32;  ///
  int64_t m_long;
  int64_t m_ulong;  ///
  float m_float;
  double m_double;

  std::string m_string;

  std::vector<bool> m_boolArray;
  std::vector<int8_t> m_byteArray;
  std::vector<int8_t> m_sbyteArray;  ///

  std::vector<char16_t> m_charArray;

  std::shared_ptr<CacheableDate> m_date;

  std::vector<int16_t> m_int16Array;
  std::vector<int16_t> m_uint16Array;

  std::vector<int32_t> m_int32Array;
  std::vector<int32_t> m_uint32Array;

  std::vector<int64_t> m_longArray;
  std::vector<int64_t> m_ulongArray;

  std::vector<float> m_floatArray;
  std::vector<double> m_doubleArray;

  int8_t** m_byteByteArray;

  std::vector<std::string> m_stringArray;

  std::shared_ptr<Serializable> m_address;

  std::shared_ptr<NonPdxAddress> m_add[10];

  std::shared_ptr<CacheableArrayList> m_arraylist;
  std::shared_ptr<CacheableHashMap> m_map;
  std::shared_ptr<CacheableHashTable> m_hashtable;
  std::shared_ptr<CacheableVector> m_vector;

  std::shared_ptr<CacheableHashSet> m_chs;
  std::shared_ptr<CacheableLinkedHashSet> m_clhs;

  std::vector<int8_t> m_byte252;
  std::vector<int8_t> m_byte253;
  std::vector<int8_t> m_byte65535;
  std::vector<int8_t> m_byte65536;
  std::shared_ptr<Cacheable> m_pdxEnum;

  std::shared_ptr<CacheableObjectArray> m_objectArray;

  int32_t boolArrayLen;
  int32_t charArrayLen;
  int32_t byteArrayLen;
  int32_t shortArrayLen;
  int32_t intArrayLen;
  int32_t longArrayLen;
  int32_t doubleArrayLen;
  int32_t floatArrayLen;
  int32_t strLenArray;
  int32_t m_byte252Len;
  int32_t m_byte253Len;
  int32_t m_byte65535Len;
  int32_t m_byte65536Len;
  int32_t byteByteArrayLen;

  int lengthArr[2];

 public:
  bool selfCheck();

  inline void init() {
    m_char = 'C';
    m_bool = true;
    m_byte = 0x74;
    m_sbyte = 0x67;
    m_int16 = 0xab;
    m_uint16 = 0x2dd5;
    m_int32 = 0x2345abdc;
    m_uint32 = 0x2a65c434;
    m_long = 324897980;
    m_ulong = 238749898;
    m_float = 23324.324f;
    m_double = 3243298498.00;

    m_string = "gfestring";

    m_boolArray = std::vector<bool>(3);
    m_boolArray[0] = true;
    m_boolArray[1] = false;
    m_boolArray[2] = true;

    m_byteArray = std::vector<int8_t>(2);
    m_byteArray[0] = 0x34;
    m_byteArray[1] = 0x64;

    m_sbyteArray = std::vector<int8_t>(2);
    m_sbyteArray[0] = 0x34;
    m_sbyteArray[1] = 0x64;

    m_charArray = std::vector<char16_t>(2);
    m_charArray[0] = 'c';
    m_charArray[1] = 'v';

    int64_t d = 1310447869154L;
    m_date = CacheableDate::create(CacheableDate::duration(d));

    m_int16Array = std::vector<int16_t>(2);
    m_int16Array[0] = 0x2332;
    m_int16Array[1] = 0x4545;

    m_uint16Array = std::vector<int16_t>(2);
    m_uint16Array[0] = 0x3243;
    m_uint16Array[1] = 0x3232;

    m_int32Array = std::vector<int32_t>(4);
    m_int32Array[0] = 23;
    m_int32Array[1] = 676868;
    m_int32Array[2] = 34343;
    m_int32Array[3] = 2323;

    m_uint32Array = std::vector<int32_t>(4);
    m_uint32Array[0] = 435;
    m_uint32Array[1] = 234324;
    m_uint32Array[2] = 324324;
    m_uint32Array[3] = 23432432;

    m_longArray = std::vector<int64_t>(2);
    m_longArray[0] = 324324L;
    m_longArray[1] = 23434545L;

    m_ulongArray = std::vector<int64_t>(2);
    m_ulongArray[0] = 3245435;
    m_ulongArray[1] = 3425435;

    m_floatArray = std::vector<float>(2);
    m_floatArray[0] = 232.565f;
    m_floatArray[1] = 2343254.67f;

    m_doubleArray = std::vector<double>(2);
    m_doubleArray[0] = 23423432;
    m_doubleArray[1] = 4324235435.00;

    m_byteByteArray = new int8_t*[2];
    m_byteByteArray[0] = new int8_t[1];
    m_byteByteArray[1] = new int8_t[2];
    m_byteByteArray[0][0] = 0x23;
    m_byteByteArray[1][0] = 0x34;
    m_byteByteArray[1][1] = 0x55;

    m_stringArray = {"one", "two"};

    m_arraylist = CacheableArrayList::create();
    m_arraylist->push_back(CacheableInt32::create(1));
    m_arraylist->push_back(CacheableInt32::create(2));

    m_map = CacheableHashMap::create();
    m_map->emplace(CacheableInt32::create(1), CacheableInt32::create(1));
    m_map->emplace(CacheableInt32::create(2), CacheableInt32::create(2));

    m_hashtable = CacheableHashTable::create();
    m_hashtable->emplace(CacheableInt32::create(1),
                         CacheableString::create("1111111111111111"));
    m_hashtable->emplace(
        CacheableInt32::create(2),
        CacheableString::create("2222222222221111111111111111"));

    m_vector = CacheableVector::create();
    m_vector->push_back(CacheableInt32::create(1));
    m_vector->push_back(CacheableInt32::create(2));
    m_vector->push_back(CacheableInt32::create(3));

    m_chs = CacheableHashSet::create();
    m_chs->insert(CacheableInt32::create(1));

    m_clhs = CacheableLinkedHashSet::create();
    m_clhs->insert(CacheableInt32::create(1));
    m_clhs->insert(CacheableInt32::create(2));

    m_pdxEnum = CacheableEnum::create("PdxTests.pdxEnumTest", "pdx2", pdx2);

    m_add[0] = std::make_shared<NonPdxAddress>(1, "street0", "city0");
    m_add[1] = std::make_shared<NonPdxAddress>(2, "street1", "city1");
    m_add[2] = std::make_shared<NonPdxAddress>(3, "street2", "city2");
    m_add[3] = std::make_shared<NonPdxAddress>(4, "street3", "city3");
    m_add[4] = std::make_shared<NonPdxAddress>(5, "street4", "city4");
    m_add[5] = std::make_shared<NonPdxAddress>(6, "street5", "city5");
    m_add[6] = std::make_shared<NonPdxAddress>(7, "street6", "city6");
    m_add[7] = std::make_shared<NonPdxAddress>(8, "street7", "city7");
    m_add[8] = std::make_shared<NonPdxAddress>(9, "street8", "city8");
    m_add[9] = std::make_shared<NonPdxAddress>(10, "street9", "city9");

    m_objectArray = CacheableObjectArray::create();
    m_objectArray->push_back(std::shared_ptr<PdxWrapper>(
        new PdxWrapper(std::make_shared<NonPdxAddress>(1, "street0", "city0"),
                       "PdxTests.Address")));
    m_objectArray->push_back(std::shared_ptr<PdxWrapper>(
        new PdxWrapper(std::make_shared<NonPdxAddress>(2, "street1", "city1"),
                       "PdxTests.Address")));
    m_objectArray->push_back(std::shared_ptr<PdxWrapper>(
        new PdxWrapper(std::make_shared<NonPdxAddress>(3, "street2", "city2"),
                       "PdxTests.Address")));
    m_objectArray->push_back(std::shared_ptr<PdxWrapper>(
        new PdxWrapper(std::make_shared<NonPdxAddress>(4, "street3", "city3"),
                       "PdxTests.Address")));
    m_objectArray->push_back(std::shared_ptr<PdxWrapper>(
        new PdxWrapper(std::make_shared<NonPdxAddress>(5, "street4", "city4"),
                       "PdxTests.Address")));
    m_objectArray->push_back(std::shared_ptr<PdxWrapper>(
        new PdxWrapper(std::make_shared<NonPdxAddress>(6, "street5", "city5"),
                       "PdxTests.Address")));
    m_objectArray->push_back(std::shared_ptr<PdxWrapper>(
        new PdxWrapper(std::make_shared<NonPdxAddress>(7, "street6", "city6"),
                       "PdxTests.Address")));
    m_objectArray->push_back(std::shared_ptr<PdxWrapper>(
        new PdxWrapper(std::make_shared<NonPdxAddress>(8, "street7", "city7"),
                       "PdxTests.Address")));
    m_objectArray->push_back(std::shared_ptr<PdxWrapper>(
        new PdxWrapper(std::make_shared<NonPdxAddress>(9, "street8", "city8"),
                       "PdxTests.Address")));
    m_objectArray->push_back(std::shared_ptr<PdxWrapper>(
        new PdxWrapper(std::make_shared<NonPdxAddress>(10, "street9", "city9"),
                       "PdxTests.Address")));

    m_byte252 = std::vector<int8_t>(252);
    for (int i = 0; i < 252; i++) {
      m_byte252[i] = 0;
    }

    m_byte253 = std::vector<int8_t>(253);
    for (int i = 0; i < 253; i++) {
      m_byte253[i] = 0;
    }

    m_byte65535 = std::vector<int8_t>(65535);
    for (int i = 0; i < 65535; i++) {
      m_byte65535[i] = 0;
    }

    m_byte65536 = std::vector<int8_t>(65536);
    for (int i = 0; i < 65536; i++) {
      m_byte65536[i] = 0;
    }

    boolArrayLen = 3;
    byteArrayLen = 2;
    shortArrayLen = 2;
    intArrayLen = 4;
    longArrayLen = 2;
    doubleArrayLen = 2;
    floatArrayLen = 2;
    strLenArray = 2;
    charArrayLen = 2;
    byteByteArrayLen = 2;

    lengthArr[0] = 1;
    lengthArr[1] = 2;
  }

  NonPdxType() { init(); }

  inline bool compareBool(bool b, bool b2) {
    if (b == b2) return b;
    throw IllegalStateException("Not got expected value for bool type: ");
  }

  void deleteByteByteArray() {
    if (m_byteByteArray == nullptr) {
      return;
    }
    _GEODE_SAFE_DELETE_ARRAY(m_byteByteArray[0]);
    _GEODE_SAFE_DELETE_ARRAY(m_byteByteArray[1]);
    _GEODE_SAFE_DELETE_ARRAY(m_byteByteArray);
  }

  virtual ~NonPdxType() { deleteByteByteArray(); }

  char16_t getChar() { return m_char; }

  std::vector<char16_t> getCharArray() { return m_charArray; }

  int8_t** getArrayOfByteArrays() { return m_byteByteArray; }

  bool getBool() { return m_bool; }

  std::shared_ptr<CacheableHashMap> getHashMap() { return m_map; }

  int8_t getSByte() { return m_sbyte; }

  int16_t getUint16() { return m_uint16; }

  int32_t getUInt() { return m_uint32; }

  int64_t getULong() { return m_ulong; }

  std::vector<int16_t> getUInt16Array() { return m_uint16Array; }

  std::vector<int32_t> getUIntArray() { return m_uint32Array; }

  std::vector<int64_t> getULongArray() { return m_ulongArray; }

  std::vector<int8_t> getByte252() { return m_byte252; }

  std::vector<int8_t> getByte253() { return m_byte253; }

  std::vector<int8_t> getByte65535() { return m_byte65535; }

  std::vector<int8_t> getByte65536() { return m_byte65536; }

  std::vector<int8_t> getSByteArray() { return m_sbyteArray; }

  std::shared_ptr<CacheableHashSet> getHashSet() { return m_chs; }

  std::shared_ptr<CacheableLinkedHashSet> getLinkedHashSet() { return m_clhs; }

  std::shared_ptr<CacheableArrayList> getArrayList() { return m_arraylist; }

  std::shared_ptr<CacheableHashTable> getHashTable() { return m_hashtable; }

  std::shared_ptr<CacheableVector> getVector() { return m_vector; }

  int8_t getByte() { return m_byte; }

  int16_t getShort() { return m_int16; }

  int32_t getInt() { return m_int32; }

  int64_t getLong() { return m_long; }

  float getFloat() { return m_float; }

  double getDouble() { return m_double; }

  const std::string& getString() { return m_string; }

  std::vector<bool> getBoolArray() { return m_boolArray; }

  std::vector<int8_t> getByteArray() { return m_byteArray; }

  std::vector<int16_t> getShortArray() { return m_int16Array; }

  std::vector<int32_t> getIntArray() { return m_int32Array; }

  std::vector<int64_t> getLongArray() { return m_longArray; }

  std::vector<double> getDoubleArray() { return m_doubleArray; }

  std::vector<float> getFloatArray() { return m_floatArray; }

  const std::vector<std::string>& getStringArray() { return m_stringArray; }

  std::shared_ptr<CacheableDate> getDate() { return m_date; }

  std::shared_ptr<CacheableObjectArray> getCacheableObjectArray() {
    return m_objectArray;
  }

  std::shared_ptr<CacheableEnum> getEnum() {
    return std::dynamic_pointer_cast<CacheableEnum>(m_pdxEnum);
  }

  int32_t getByteArrayLength() { return byteArrayLen; }

  int32_t getBoolArrayLength() { return boolArrayLen; }

  int32_t getShortArrayLength() { return shortArrayLen; }

  int32_t getStringArrayLength() { return strLenArray; }

  int32_t getIntArrayLength() { return intArrayLen; }

  int32_t getLongArrayLength() { return longArrayLen; }

  int32_t getFloatArrayLength() { return floatArrayLen; }

  int32_t getDoubleArrayLength() { return doubleArrayLen; }

  int32_t getbyteByteArrayLength() { return byteByteArrayLen; }

  int32_t getCharArrayLength() { return charArrayLen; }

  bool equals(PdxTests::NonPdxType& other, bool isPdxReadSerialized) const;

  template <typename T1, typename T2>
  bool genericValCompare(T1 value1, T2 value2) const;

  template <typename T1, typename T2, typename L>
  bool genericCompare(T1* value1, T2* value2, L length) const;

  template <typename T1, typename T2>
  bool generic2DCompare(T1** value1, T2** value2, int length,
                        int* arrLengths) const;
};
}  // namespace PdxTests

#endif  // GEODE_TESTOBJECT_NONPDXTYPE_H_
