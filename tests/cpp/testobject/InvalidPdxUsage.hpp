#pragma once

#ifndef GEODE_TESTOBJECT_INVALIDPDXUSAGE_H_
#define GEODE_TESTOBJECT_INVALIDPDXUSAGE_H_

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
 * InvalidPdxUsage.hpp
 *
 *  Created on: Sep 29, 2011
 *      Author: npatel
 */

#include <util/Log.hpp>

#include <geode/CacheableEnum.hpp>
#include <geode/CacheableObjectArray.hpp>
#include <geode/PdxReader.hpp>
#include <geode/PdxSerializable.hpp>
#include <geode/PdxWriter.hpp>

#include "testobject_export.h"

namespace PdxTests {

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
using apache::geode::client::PdxReader;
using apache::geode::client::PdxSerializable;
using apache::geode::client::PdxWriter;

class TESTOBJECT_EXPORT CharTypesWithInvalidUsage : public PdxSerializable {
 private:
  char16_t m_ch;
  std::vector<char16_t> m_chArray;

  int32_t m_charArrayLen;

 public:
  inline void init() {
    m_ch = 'C';

    m_chArray = std::vector<char16_t>(2);
    m_chArray[0] = 'X';
    m_chArray[1] = 'Y';

    m_charArrayLen = 0;
  }

  CharTypesWithInvalidUsage() { init(); }

  std::string toString() const override {
    char idbuf[1024];
    sprintf(idbuf, "%c %c %c", m_ch, m_chArray[0], m_chArray[1]);
    return idbuf;
  }

  bool equals(CharTypesWithInvalidUsage& other) const {
    LOG_DEBUG("Inside CharTypesWithInvalidUsage equals");
    CharTypesWithInvalidUsage* ot =
        dynamic_cast<CharTypesWithInvalidUsage*>(&other);
    if (!ot) {
      return false;
    }
    if (ot == this) {
      return true;
    }
    LOG_INFO("CharTypesWithInvalidUsage::equals ot->m_ch = {} m_ch = {}",
             static_cast<int32_t>(ot->m_ch), static_cast<int32_t>(m_ch));
    if (ot->m_ch != m_ch) {
      return false;
    }

    int i = 0;
    while (i < 2) {
      LOG_INFO(
          "CharTypesWithInvalidUsage::equals Normal char array values "
          "ot->m_chArray[{}] = {} m_chArray[{}] = {}",
          i, static_cast<int32_t>(ot->m_chArray[i]), i,
          static_cast<int32_t>(m_chArray[i]));
      if (ot->m_chArray[i] != m_chArray[i]) {
        return false;
      } else {
        i++;
      }
    }

    return true;
  }

  const std::string& getClassName() const override {
    static std::string className = "PdxTests.CharTypesWithInvalidUsage";
    return className;
  }

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  void toData(PdxWriter& pw) const override {
    pw.writeChar("m_ch", m_ch);
    pw.writeCharArray("m_chArray", m_chArray);
  }

  void fromData(PdxReader& pr) override {
    m_ch = pr.readChar("m_ch");
    m_chArray = pr.readCharArray("m_chArray");
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<CharTypesWithInvalidUsage>();
  }
};

class TESTOBJECT_EXPORT AddressWithInvalidAPIUsage : public PdxSerializable {
 private:
  int32_t _aptNumber;
  std::string _street;
  std::string _city;

 public:
  AddressWithInvalidAPIUsage() {}

  std::string toString() const override {
    return std::to_string(_aptNumber) + " " + _street + " " + _city;
  }

  AddressWithInvalidAPIUsage(int32_t aptN, std::string street,
                             std::string city) {
    _aptNumber = aptN;
    _street = street;
    _city = city;
  }

  bool equals(AddressWithInvalidAPIUsage& other) const {
    LOG_DEBUG("Inside AddressWithInvalidAPIUsage equals");
    AddressWithInvalidAPIUsage* ot =
        dynamic_cast<AddressWithInvalidAPIUsage*>(&other);
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

  const std::string& getClassName() const override {
    static std::string className = "PdxTests.AddressWithInvalidAPIUsage";
    return className;
  }

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  void toData(PdxWriter& pw) const override {
    pw.writeInt("_aptNumber", _aptNumber);  // 4
    pw.writeString("_street", _street);
    pw.writeString("_city", _city);
  }

  void fromData(PdxReader& pr) override {
    _aptNumber = pr.readInt("_aptNumber");
    _street = pr.readString("_street");
    _city = pr.readString("_city");
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<AddressWithInvalidAPIUsage>();
  }

  int32_t getAptNum() { return _aptNumber; }

  const std::string& getStreet() { return _street; }

  const std::string& getCity() { return _city; }
};

enum pdxEnumTestWithInvalidAPIUsage { mypdx1, mypdx2, mypdx3 };

class TESTOBJECT_EXPORT InvalidPdxUsage : public PdxSerializable {
 private:
  char16_t m_char;
  bool m_bool;
  int8_t m_byte;
  int8_t m_sbyte;
  int16_t m_int16;
  int16_t m_uint16;
  int32_t m_int32;
  int32_t m_uint32;
  int64_t m_long;
  int64_t m_ulong;
  float m_float;
  double m_double;

  std::string m_string;

  std::vector<bool> m_boolArray;
  std::vector<int8_t> m_byteArray;
  std::vector<int8_t> m_sbyteArray;

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
  AddressWithInvalidAPIUsage* m_add[10];

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
  std::shared_ptr<CacheableEnum> m_pdxEnum;
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
  int32_t byteByteArrayLen;

  int* lengthArr;
  mutable int toDataexceptionCounter;
  int fromDataexceptionCounter;

 public:
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
    /*for(int i=0; i<3; i++){
      m_boolArray[i] = true;
    };*/

    m_byteArray = std::vector<int8_t>(2);
    m_byteArray[0] = 0x34;
    m_byteArray[1] = 0x64;

    m_sbyteArray = std::vector<int8_t>(2);
    m_sbyteArray[0] = 0x34;
    m_sbyteArray[1] = 0x64;

    m_charArray = std::vector<char16_t>(2);
    m_charArray[0] = L'c';
    m_charArray[1] = L'v';

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
    // for(int i=0; i<2; i++){
    //  m_byteByteArray[i] = new int8_t[1];
    //}
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

    m_pdxEnum = CacheableEnum::create("PdxTests.pdxEnumTestWithInvalidAPIUsage",
                                      "mypdx2", mypdx2);

    // std::shared_ptr<AddressWithInvalidAPIUsage>* addPtr = NULL;
    // m_add = new AddressWithInvalidAPIUsage[10];
    // addPtr[i] = AddressWithInvalidAPIUsage::create();

    m_add[0] = new AddressWithInvalidAPIUsage(1, "street0", "city0");
    m_add[1] = new AddressWithInvalidAPIUsage(2, "street1", "city1");
    m_add[2] = new AddressWithInvalidAPIUsage(3, "street2", "city2");
    m_add[3] = new AddressWithInvalidAPIUsage(4, "street3", "city3");
    m_add[4] = new AddressWithInvalidAPIUsage(5, "street4", "city4");
    m_add[5] = new AddressWithInvalidAPIUsage(6, "street5", "city5");
    m_add[6] = new AddressWithInvalidAPIUsage(7, "street6", "city6");
    m_add[7] = new AddressWithInvalidAPIUsage(8, "street7", "city7");
    m_add[8] = new AddressWithInvalidAPIUsage(9, "street8", "city8");
    m_add[9] = new AddressWithInvalidAPIUsage(10, "street9", "city9");

    m_objectArray = nullptr;

    m_objectArray = CacheableObjectArray::create();
    m_objectArray->push_back(std::shared_ptr<AddressWithInvalidAPIUsage>(
        new AddressWithInvalidAPIUsage(1, "street0", "city0")));
    m_objectArray->push_back(std::shared_ptr<AddressWithInvalidAPIUsage>(
        new AddressWithInvalidAPIUsage(2, "street1", "city1")));
    m_objectArray->push_back(std::shared_ptr<AddressWithInvalidAPIUsage>(
        new AddressWithInvalidAPIUsage(3, "street2", "city2")));
    m_objectArray->push_back(std::shared_ptr<AddressWithInvalidAPIUsage>(
        new AddressWithInvalidAPIUsage(4, "street3", "city3")));
    m_objectArray->push_back(std::shared_ptr<AddressWithInvalidAPIUsage>(
        new AddressWithInvalidAPIUsage(5, "street4", "city4")));
    m_objectArray->push_back(std::shared_ptr<AddressWithInvalidAPIUsage>(
        new AddressWithInvalidAPIUsage(6, "street5", "city5")));
    m_objectArray->push_back(std::shared_ptr<AddressWithInvalidAPIUsage>(
        new AddressWithInvalidAPIUsage(7, "street6", "city6")));
    m_objectArray->push_back(std::shared_ptr<AddressWithInvalidAPIUsage>(
        new AddressWithInvalidAPIUsage(8, "street7", "city7")));
    m_objectArray->push_back(std::shared_ptr<AddressWithInvalidAPIUsage>(
        new AddressWithInvalidAPIUsage(9, "street8", "city8")));
    m_objectArray->push_back(std::shared_ptr<AddressWithInvalidAPIUsage>(
        new AddressWithInvalidAPIUsage(10, "street9", "city9")));

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

    /*for (int32_t index = 0; index <3; ++index) {
      m_objectArray->push_back(objectArray[index]);
    }*/
    /*
    if (keys.size() > 0) {
      m_objectArray = CacheableObjectArray::create();
      for (int32_t index = 0; index < keys.size(); ++index) {
        m_objectArray->push_back(keys.operator[](index));
      }
    }*/

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

    lengthArr = new int[2];

    lengthArr[0] = 1;
    lengthArr[1] = 2;

    toDataexceptionCounter = 0;
    fromDataexceptionCounter = 0;
  }

  InvalidPdxUsage() { init(); }

  inline bool compareBool(bool b, bool b2) {
    if (b == b2) return b;
    throw IllegalStateException("Not got expected value for bool type: ");
  }

  ~InvalidPdxUsage() override = default;

  virtual size_t objectSize() const override {
    auto objectSize = sizeof(InvalidPdxUsage);
    return objectSize;
  }

  int gettoDataExceptionCount() { return toDataexceptionCounter; }

  int getfromDataExceptionCount() { return fromDataexceptionCounter; }

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

  std::shared_ptr<CacheableEnum> getEnum() { return m_pdxEnum; }

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

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void toData(PdxWriter& pw) const override;

  virtual void fromData(PdxReader& pr) override;

  std::string toString() const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests.InvalidPdxUsage";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<InvalidPdxUsage>();
  }

  bool equals(PdxTests::InvalidPdxUsage& other, bool isPdxReadSerialized) const;

  template <typename T1, typename T2>
  bool genericValCompare(T1 value1, T2 value2) const;

  template <typename T1, typename T2, typename L>
  bool genericCompare(T1* value1, T2* value2, L length) const;

  template <typename T1, typename T2>
  bool generic2DCompare(T1** value1, T2** value2, int length,
                        int* arrLengths) const;
};
}  // namespace PdxTests

#endif  // GEODE_TESTOBJECT_INVALIDPDXUSAGE_H_
