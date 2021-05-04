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

#ifndef GEODE_TESTOBJECT_PDXTYPE_H_
#define GEODE_TESTOBJECT_PDXTYPE_H_

#include <util/Log.hpp>

#include <geode/CacheableEnum.hpp>
#include <geode/CacheableObjectArray.hpp>
#include <geode/PdxReader.hpp>
#include <geode/PdxSerializable.hpp>
#include <geode/PdxWriter.hpp>

#include "testobject_export.h"

namespace PdxTests {

using apache::geode::client::Cache;
using apache::geode::client::Cacheable;
using apache::geode::client::CacheableArrayList;
using apache::geode::client::CacheableDate;
using apache::geode::client::CacheableEnum;
using apache::geode::client::CacheableHashMap;
using apache::geode::client::CacheableHashSet;
using apache::geode::client::CacheableHashTable;
using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableLinkedHashSet;
using apache::geode::client::CacheableLinkedList;
using apache::geode::client::CacheableObjectArray;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheableVector;
using apache::geode::client::IllegalStateException;
using apache::geode::client::PdxReader;
using apache::geode::client::PdxSerializable;
using apache::geode::client::PdxWriter;
using apache::geode::client::Serializable;

class TESTOBJECT_EXPORT GrandParent {
 public:
  int32_t m_a;
  int32_t m_b;

  inline void init() {
    this->m_a = 1;
    this->m_b = 2;
  }

  GrandParent() { init(); }

  int32_t getMember_a() { return m_a; }

  int32_t getMember_b() { return m_b; }
};

class TESTOBJECT_EXPORT Parent : public GrandParent {
 public:
  int32_t m_c;
  int32_t m_d;

  inline void init() {
    this->m_c = 3;
    this->m_d = 4;
  }

  int32_t getMember_c() { return m_c; }

  int32_t getMember_d() { return m_d; }

  Parent() { init(); }
};

class Child;

class TESTOBJECT_EXPORT Child : public Parent, public PdxSerializable {
 private:
  int32_t m_e;
  int32_t m_f;

 public:
  inline void init() {
    this->m_e = 5;
    this->m_f = 6;
  }

  Child() { init(); }

  int32_t getMember_e() { return m_e; }

  int32_t getMember_f() { return m_f; }

  std::string toString() const override {
    char idbuf[512];
    sprintf(idbuf, " Child:: %d %d %d %d %d %d ", m_a, m_b, m_c, m_d, m_e, m_f);
    return idbuf;
  }

  const std::string& getClassName() const override {
    static std::string className = "PdxTests.Child";
    return className;
  }

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  void toData(PdxWriter& pw) const override {
    pw.writeInt("m_a", m_a);
    pw.writeInt("m_b", m_b);
    pw.writeInt("m_c", m_c);
    pw.writeInt("m_d", m_d);
    pw.writeInt("m_e", m_e);
    pw.writeInt("m_f", m_f);
  }

  void fromData(PdxReader& pr) override {
    m_a = pr.readInt("m_a");
    m_b = pr.readInt("m_b");
    m_c = pr.readInt("m_c");
    m_d = pr.readInt("m_d");
    m_e = pr.readInt("m_e");
    m_f = pr.readInt("m_f");
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<Child>();
  }

  bool equals(std::shared_ptr<PdxSerializable> obj) {
    if (obj == nullptr) return false;

    auto pap = std::dynamic_pointer_cast<Child>(obj);
    if (pap == nullptr) return false;

    if (pap.get() == this) return true;

    if (m_a == pap->m_a && m_b == pap->m_b && m_c == pap->m_c &&
        m_d == pap->m_d && m_e == pap->m_e && m_f == pap->m_f) {
      return true;
    }

    return false;
  }
};

class TESTOBJECT_EXPORT CharTypes : public PdxSerializable {
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

  CharTypes() { init(); }

  std::string toString() const override {
    char idbuf[1024];
    sprintf(idbuf, "%c %c %c", m_ch, m_chArray[0], m_chArray[1]);
    return idbuf;
  }

  bool equals(CharTypes& other) const {
    LOG_DEBUG("Inside CharTypes equals");
    CharTypes* ot = dynamic_cast<CharTypes*>(&other);
    if (!ot) {
      return false;
    }
    if (ot == this) {
      return true;
    }
    LOG_INFO("CharTypes::equals ot->m_ch = {} m_ch = {}",
             static_cast<int32_t>(ot->m_ch), static_cast<int32_t>(m_ch));
    if (ot->m_ch != m_ch) {
      return false;
    }

    int i = 0;
    while (i < 2) {
      LOG_INFO(
          "CharTypes::equals Normal char array values ot->m_chArray[{}] = {} "
          "m_chArray[{}] = {}",
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
    static std::string className = "PdxTests.CharTypes";
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
    return std::make_shared<CharTypes>();
  }
};

/**********/
class TESTOBJECT_EXPORT Address : public PdxSerializable {
 private:
  int32_t _aptNumber;
  std::string _street;
  std::string _city;

 public:
  Address() {}

  std::string toString() const override {
    return std::to_string(_aptNumber) + " " + _street + " " + _city;
  }

  Address(int32_t aptN, const char* street, const char* city) {
    _aptNumber = aptN;
    _street = street;
    _city = city;
  }

  bool equals(Address& other) const {
    LOG_DEBUG("Inside Address equals");
    Address* ot = dynamic_cast<Address*>(&other);
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
    static std::string className = "PdxTests.Address";
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
    return std::make_shared<Address>();
  }

  int32_t getAptNum() { return _aptNumber; }

  const std::string& getStreet() { return _street; }

  const std::string& getCity() { return _city; }

  /*static std::shared_ptr<Address> create(int32_t aptN, char* street, char*
  city)
  {
        std::shared_ptr<Address> str = nullptr;
    if (value != NULL) {
      str = new Address();
    }
    return str;
  }*/
};
enum pdxEnumTest { pdx1, pdx2, pdx3 };
class TESTOBJECT_EXPORT PdxType : public PdxSerializable {
 private:
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
  Address* m_add[10];

  std::shared_ptr<CacheableArrayList> m_arraylist;
  std::shared_ptr<CacheableLinkedList> m_linkedlist;
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
  std::shared_ptr<CacheableObjectArray> m_objectArrayEmptyPdxFieldName;

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

  int lengthArr[2];

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
    m_byteByteArray[0] = new int8_t[1];
    m_byteByteArray[1] = new int8_t[2];
    m_byteByteArray[0][0] = 0x23;
    m_byteByteArray[1][0] = 0x34;
    m_byteByteArray[1][1] = 0x55;

    m_stringArray = {"one", "two"};

    m_arraylist = CacheableArrayList::create();
    m_arraylist->push_back(CacheableInt32::create(1));
    m_arraylist->push_back(CacheableInt32::create(2));

    m_linkedlist = CacheableLinkedList::create();
    m_linkedlist->push_back(CacheableInt32::create(1));
    m_linkedlist->push_back(CacheableInt32::create(2));

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

    // std::shared_ptr<Address>* addPtr = NULL;
    // m_add = new Address[10];
    // addPtr[i] = Address::create();

    m_add[0] = new Address(1, "street0", "city0");
    m_add[1] = new Address(2, "street1", "city1");
    m_add[2] = new Address(3, "street2", "city2");
    m_add[3] = new Address(4, "street3", "city3");
    m_add[4] = new Address(5, "street4", "city4");
    m_add[5] = new Address(6, "street5", "city5");
    m_add[6] = new Address(7, "street6", "city6");
    m_add[7] = new Address(8, "street7", "city7");
    m_add[8] = new Address(9, "street8", "city8");
    m_add[9] = new Address(10, "street9", "city9");

    m_objectArray = nullptr;
    m_objectArrayEmptyPdxFieldName = nullptr;

    m_objectArray = CacheableObjectArray::create();
    m_objectArray->push_back(
        std::shared_ptr<Address>(new Address(1, "street0", "city0")));
    m_objectArray->push_back(
        std::shared_ptr<Address>(new Address(2, "street1", "city1")));
    m_objectArray->push_back(
        std::shared_ptr<Address>(new Address(3, "street2", "city2")));
    m_objectArray->push_back(
        std::shared_ptr<Address>(new Address(4, "street3", "city3")));
    m_objectArray->push_back(
        std::shared_ptr<Address>(new Address(5, "street4", "city4")));
    m_objectArray->push_back(
        std::shared_ptr<Address>(new Address(6, "street5", "city5")));
    m_objectArray->push_back(
        std::shared_ptr<Address>(new Address(7, "street6", "city6")));
    m_objectArray->push_back(
        std::shared_ptr<Address>(new Address(8, "street7", "city7")));
    m_objectArray->push_back(
        std::shared_ptr<Address>(new Address(9, "street8", "city8")));
    m_objectArray->push_back(
        std::shared_ptr<Address>(new Address(10, "street9", "city9")));

    m_objectArrayEmptyPdxFieldName = CacheableObjectArray::create();
    m_objectArrayEmptyPdxFieldName->push_back(
        std::shared_ptr<Address>(new Address(1, "street0", "city0")));
    m_objectArrayEmptyPdxFieldName->push_back(
        std::shared_ptr<Address>(new Address(2, "street1", "city1")));
    m_objectArrayEmptyPdxFieldName->push_back(
        std::shared_ptr<Address>(new Address(3, "street2", "city2")));
    m_objectArrayEmptyPdxFieldName->push_back(
        std::shared_ptr<Address>(new Address(4, "street3", "city3")));
    m_objectArrayEmptyPdxFieldName->push_back(
        std::shared_ptr<Address>(new Address(5, "street4", "city4")));
    m_objectArrayEmptyPdxFieldName->push_back(
        std::shared_ptr<Address>(new Address(6, "street5", "city5")));
    m_objectArrayEmptyPdxFieldName->push_back(
        std::shared_ptr<Address>(new Address(7, "street6", "city6")));
    m_objectArrayEmptyPdxFieldName->push_back(
        std::shared_ptr<Address>(new Address(8, "street7", "city7")));
    m_objectArrayEmptyPdxFieldName->push_back(
        std::shared_ptr<Address>(new Address(9, "street8", "city8")));
    m_objectArrayEmptyPdxFieldName->push_back(
        std::shared_ptr<Address>(new Address(10, "street9", "city9")));

    m_byte252 = std::vector<int8_t>(252, 0);

    m_byte253 = std::vector<int8_t>(253, 0);

    m_byte65535 = std::vector<int8_t>(65535, 0);

    m_byte65536 = std::vector<int8_t>(65536, 0);

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

    lengthArr[0] = 1;
    lengthArr[1] = 2;
  }

  PdxType() { init(); }

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

  ~PdxType() override {
    deleteByteByteArray();
    for (auto i = 0; i <= 9; i++) {
      _GEODE_SAFE_DELETE(m_add[i]);
    }
  }

  virtual size_t objectSize() const override {
    auto objectSize = sizeof(PdxType);
    return objectSize;
  }

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

  std::shared_ptr<CacheableLinkedList> getLinkedList() { return m_linkedlist; }

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

  std::shared_ptr<CacheableObjectArray>
  getCacheableObjectArrayEmptyPdxFieldName() {
    return m_objectArrayEmptyPdxFieldName;
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

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void toData(PdxWriter& pw) const override;

  virtual void fromData(PdxReader& pr) override;

  std::string toString() const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests.PdxType";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PdxType>();
  }

  bool equals(PdxTests::PdxType& other, bool isPdxReadSerialized) const;

  template <typename T1, typename T2>
  bool genericValCompare(T1 value1, T2 value2) const;

  template <typename T1, typename T2, typename L>
  bool genericCompare(T1* value1, T2* value2, L length) const;

  template <typename T1, typename T2>
  bool generic2DCompare(T1** value1, T2** value2, int length,
                        int* arrLengths) const;
};

}  // namespace PdxTests

#endif  // GEODE_TESTOBJECT_PDXTYPE_H_
