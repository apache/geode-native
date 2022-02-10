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
 * PdxObject.cpp
 *
 *  Created on: Sep 29, 2011
 *      Author: npatel
 */

#include "PdxType.hpp"

namespace PdxTests {

template <typename T1, typename T2>
bool PdxTests::PdxType::genericValCompare(T1 value1, T2 value2) const {
  if (value1 != value2) return false;
  LOGINFO("PdxObject::genericValCompare Line_19");
  return true;
}

template <typename T1, typename T2, typename L>
bool PdxTests::PdxType::genericCompare(T1* value1, T2* value2, L length) const {
  L i = 0;
  while (i < length) {
    if (value1[i] != value2[i]) {
      return false;
    } else {
      i++;
    }
  }
  LOGINFO("PdxObject::genericCompare Line_34");
  return true;
}

template <typename T1, typename T2>
bool PdxTests::PdxType::generic2DCompare(T1** value1, T2** value2, int length,
                                         int* arrLengths) const {
  LOGINFO("generic2DCompare length = %d ", length);
  LOGINFO("generic2DCompare value1 = %d \t value2", value1[0][0], value2[0][0]);
  LOGINFO("generic2DCompare value1 = %d \t value2", value1[1][0], value2[1][0]);
  LOGINFO("generic2DCompare value1 = %d \t value2", value1[1][1], value2[1][1]);
  for (int j = 0; j < length; j++) {
    LOGINFO("generic2DCompare arrlength0 = %d ", arrLengths[j]);
    for (int k = 0; k < arrLengths[j]; k++) {
      LOGINFO("generic2DCompare arrlength = %d ", arrLengths[j]);
      LOGINFO("generic2DCompare value1 = %d \t value2 = %d ", value1[j][k],
              value2[j][k]);
      if (value1[j][k] != value2[j][k]) return false;
    }
  }
  LOGINFO("PdxObject::genericCompare Line_34");
  return true;
}

// PdxType::~PdxObject() {
//}

PdxTests::PdxType::PdxType() { init(); }

PdxTests::PdxType::~PdxType() { deleteByteByteArray(); }

void PdxTests::PdxType::init() {
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
  m_hashtable->emplace(CacheableInt32::create(2),
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

  m_add[0].reset(new Address{1, "street0", "city0"});
  m_add[1].reset(new Address{2, "street1", "city1"});
  m_add[2].reset(new Address{3, "street2", "city2"});
  m_add[3].reset(new Address{4, "street3", "city3"});
  m_add[4].reset(new Address{5, "street4", "city4"});
  m_add[5].reset(new Address{6, "street5", "city5"});
  m_add[6].reset(new Address{7, "street6", "city6"});
  m_add[7].reset(new Address{8, "street7", "city7"});
  m_add[8].reset(new Address{9, "street8", "city8"});
  m_add[9].reset(new Address{10, "street9", "city9"});

  m_objectArray = nullptr;
  m_objectArrayEmptyPdxFieldName = nullptr;

  m_objectArray = CacheableObjectArray::create();
  m_objectArray->emplace_back(std::make_shared<Address>(1, "street0", "city0"));
  m_objectArray->emplace_back(std::make_shared<Address>(2, "street1", "city1"));
  m_objectArray->emplace_back(std::make_shared<Address>(3, "street2", "city2"));
  m_objectArray->emplace_back(std::make_shared<Address>(4, "street3", "city3"));
  m_objectArray->emplace_back(std::make_shared<Address>(5, "street4", "city4"));
  m_objectArray->emplace_back(std::make_shared<Address>(6, "street5", "city5"));
  m_objectArray->emplace_back(std::make_shared<Address>(7, "street6", "city6"));
  m_objectArray->emplace_back(std::make_shared<Address>(8, "street7", "city7"));
  m_objectArray->emplace_back(std::make_shared<Address>(9, "street8", "city8"));
  m_objectArray->emplace_back(
      std::make_shared<Address>(10, "street9", "city9"));

  m_objectArrayEmptyPdxFieldName = CacheableObjectArray::create();
  m_objectArrayEmptyPdxFieldName->emplace_back(
      std::make_shared<Address>(1, "street0", "city0"));
  m_objectArrayEmptyPdxFieldName->emplace_back(
      std::make_shared<Address>(2, "street1", "city1"));
  m_objectArrayEmptyPdxFieldName->emplace_back(
      std::make_shared<Address>(3, "street2", "city2"));
  m_objectArrayEmptyPdxFieldName->emplace_back(
      std::make_shared<Address>(4, "street3", "city3"));
  m_objectArrayEmptyPdxFieldName->emplace_back(
      std::make_shared<Address>(5, "street4", "city4"));
  m_objectArrayEmptyPdxFieldName->emplace_back(
      std::make_shared<Address>(6, "street5", "city5"));
  m_objectArrayEmptyPdxFieldName->emplace_back(
      std::make_shared<Address>(7, "street6", "city6"));
  m_objectArrayEmptyPdxFieldName->emplace_back(
      std::make_shared<Address>(8, "street7", "city7"));
  m_objectArrayEmptyPdxFieldName->emplace_back(
      std::make_shared<Address>(9, "street8", "city8"));
  m_objectArrayEmptyPdxFieldName->emplace_back(
      std::make_shared<Address>(10, "street9", "city9"));

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

void PdxTests::PdxType::toData(PdxWriter& pw) const {
  std::vector<int> lengths(2);

  lengths[0] = 1;
  lengths[1] = 2;
  pw.writeArrayOfByteArrays("m_byteByteArray", m_byteByteArray, 2,
                            lengths.data());
  pw.writeChar("m_char", m_char);
  pw.markIdentityField("m_char");
  pw.writeBoolean("m_bool", m_bool);  // 1
  pw.markIdentityField("m_bool");
  pw.writeBooleanArray("m_boolArray", m_boolArray);
  pw.markIdentityField("m_boolArray");
  pw.writeByte("m_byte", m_byte);
  pw.markIdentityField("m_byte");
  pw.writeByteArray("m_byteArray", m_byteArray);
  pw.markIdentityField("m_byteArray");
  pw.writeCharArray("m_charArray", m_charArray);
  pw.markIdentityField("m_charArray");
  pw.writeObject("m_arraylist", m_arraylist);
  pw.writeObject("m_linkedlist", m_linkedlist);
  pw.markIdentityField("m_arraylist");
  pw.writeObject("m_map", m_map);
  pw.markIdentityField("m_map");
  pw.writeObject("m_hashtable", m_hashtable);
  pw.markIdentityField("m_hashtable");
  pw.writeObject("m_vector", m_vector);
  pw.markIdentityField("m_vector");
  pw.writeObject("m_chs", m_chs);
  pw.markIdentityField("m_chs");
  pw.writeObject("m_clhs", m_clhs);
  pw.markIdentityField("m_clhs");
  pw.writeString("m_string", m_string);
  pw.markIdentityField("m_string");
  pw.writeDate("m_dateTime", m_date);
  pw.markIdentityField("m_dateTime");
  pw.writeDouble("m_double", m_double);
  pw.markIdentityField("m_double");
  pw.writeDoubleArray("m_doubleArray", m_doubleArray);
  pw.markIdentityField("m_doubleArray");
  pw.writeFloat("m_float", m_float);
  pw.markIdentityField("m_float");
  pw.writeFloatArray("m_floatArray", m_floatArray);
  pw.markIdentityField("m_floatArray");
  pw.writeShort("m_int16", m_int16);
  pw.markIdentityField("m_int16");
  pw.writeInt("m_int32", m_int32);
  pw.markIdentityField("m_int32");
  pw.writeLong("m_long", m_long);
  pw.markIdentityField("m_long");
  pw.writeIntArray("m_int32Array", m_int32Array);
  pw.markIdentityField("m_int32Array");
  pw.writeLongArray("m_longArray", m_longArray);
  pw.markIdentityField("m_longArray");
  pw.writeShortArray("m_int16Array", m_int16Array);
  pw.markIdentityField("m_int16Array");
  pw.writeByte("m_sbyte", m_sbyte);
  pw.markIdentityField("m_sbyte");
  pw.writeByteArray("m_sbyteArray", m_sbyteArray);
  pw.markIdentityField("m_sbyteArray");

  // int* strlengthArr = new int[2];

  // strlengthArr[0] = 5;
  // strlengthArr[1] = 5;
  pw.writeStringArray("m_stringArray", m_stringArray);
  pw.markIdentityField("m_stringArray");
  pw.writeShort("m_uint16", m_uint16);
  pw.markIdentityField("m_uint16");
  pw.writeInt("m_uint32", m_uint32);
  pw.markIdentityField("m_uint32");
  pw.writeLong("m_ulong", m_ulong);
  pw.markIdentityField("m_ulong");
  pw.writeIntArray("m_uint32Array", m_uint32Array);
  pw.markIdentityField("m_uint32Array");
  pw.writeLongArray("m_ulongArray", m_ulongArray);
  pw.markIdentityField("m_ulongArray");
  pw.writeShortArray("m_uint16Array", m_uint16Array);
  pw.markIdentityField("m_uint16Array");

  pw.writeByteArray("m_byte252", m_byte252);
  pw.markIdentityField("m_byte252");
  pw.writeByteArray("m_byte253", m_byte253);
  pw.markIdentityField("m_byte253");
  pw.writeByteArray("m_byte65535", m_byte65535);
  pw.markIdentityField("m_byte65535");
  pw.writeByteArray("m_byte65536", m_byte65536);
  pw.markIdentityField("m_byte65536");

  pw.writeObject("m_pdxEnum", m_pdxEnum);
  pw.markIdentityField("m_pdxEnum");
  pw.writeObject("m_address", m_objectArray);

  pw.writeObjectArray("m_objectArray", m_objectArray);
  pw.writeObjectArray("", m_objectArrayEmptyPdxFieldName);

  LOGDEBUG("PdxObject::writeObject() for enum Done......");

  LOGDEBUG("PdxObject::toData() Done......");
}

void PdxTests::PdxType::fromData(PdxReader& pr) {
  int32_t arrLen = 0;
  int32_t* Lengtharr = nullptr;
  deleteByteByteArray();
  m_byteByteArray =
      pr.readArrayOfByteArrays("m_byteByteArray", arrLen, &Lengtharr);
  _GEODE_SAFE_DELETE_ARRAY(Lengtharr);
  // TODO::need to write compareByteByteArray() and check for m_byteByteArray
  // elements

  m_char = pr.readChar("m_char");
  // GenericValCompare

  m_bool = pr.readBoolean("m_bool");
  // GenericValCompare
  m_boolArray = pr.readBooleanArray("m_boolArray");

  m_byte = pr.readByte("m_byte");
  m_byteArray = pr.readByteArray("m_byteArray");
  m_charArray = pr.readCharArray("m_charArray");

  m_arraylist = std::dynamic_pointer_cast<CacheableArrayList>(
      pr.readObject("m_arraylist"));
  m_linkedlist = std::dynamic_pointer_cast<CacheableLinkedList>(
      pr.readObject("m_linkedlist"));
  m_map = std::dynamic_pointer_cast<CacheableHashMap>(pr.readObject("m_map"));
  // TODO:Check for the size

  m_hashtable = std::dynamic_pointer_cast<CacheableHashTable>(
      pr.readObject("m_hashtable"));
  // TODO:Check for the size

  m_vector =
      std::dynamic_pointer_cast<CacheableVector>(pr.readObject("m_vector"));
  // TODO::Check for size

  m_chs = std::dynamic_pointer_cast<CacheableHashSet>(pr.readObject("m_chs"));
  // TODO::Size check

  m_clhs = std::dynamic_pointer_cast<CacheableLinkedHashSet>(
      pr.readObject("m_clhs"));
  // TODO:Size check

  m_string = pr.readString("m_string");  // GenericValCompare
  m_date = pr.readDate("m_dateTime");    // compareData

  m_double = pr.readDouble("m_double");

  m_doubleArray = pr.readDoubleArray("m_doubleArray");
  m_float = pr.readFloat("m_float");
  m_floatArray = pr.readFloatArray("m_floatArray");
  m_int16 = pr.readShort("m_int16");
  m_int32 = pr.readInt("m_int32");
  m_long = pr.readLong("m_long");
  m_int32Array = pr.readIntArray("m_int32Array");
  m_longArray = pr.readLongArray("m_longArray");
  m_int16Array = pr.readShortArray("m_int16Array");
  m_sbyte = pr.readByte("m_sbyte");
  m_sbyteArray = pr.readByteArray("m_sbyteArray");
  m_stringArray = pr.readStringArray("m_stringArray");
  m_uint16 = pr.readShort("m_uint16");
  m_uint32 = pr.readInt("m_uint32");
  m_ulong = pr.readLong("m_ulong");
  m_uint32Array = pr.readIntArray("m_uint32Array");
  m_ulongArray = pr.readLongArray("m_ulongArray");
  m_uint16Array = pr.readShortArray("m_uint16Array");
  // LOGINFO("PdxType::readInt() start...");

  m_byte252 = pr.readByteArray("m_byte252");
  m_byte253 = pr.readByteArray("m_byte253");
  m_byte65535 = pr.readByteArray("m_byte65535");
  m_byte65536 = pr.readByteArray("m_byte65536");
  // TODO:Check for size

  m_pdxEnum = pr.readObject("m_pdxEnum");

  m_address = pr.readObject("m_address");
  // size chaeck

  m_objectArray = pr.readObjectArray("m_objectArray");
  m_objectArrayEmptyPdxFieldName = pr.readObjectArray("");

  // Check for individual elements

  // TODO:temp added delete it later

  LOGINFO("PdxObject::readObject() for enum Done...");
}
std::string PdxTests::PdxType::toString() const {
  return "PdxObject:[m_int32=" + std::to_string(m_int32) + "]";
}

bool PdxTests::PdxType::equals(PdxTests::PdxType& other,
                               bool isPdxReadSerialized) const {
  PdxType* ot = dynamic_cast<PdxType*>(&other);
  if (!ot) {
    return false;
  }
  if (ot == this) {
    return true;
  }
  genericValCompare(ot->m_int32, m_int32);
  genericValCompare(ot->m_bool, m_bool);
  genericValCompare(ot->m_byte, m_byte);
  genericValCompare(ot->m_int16, m_int16);
  genericValCompare(ot->m_long, m_long);
  genericValCompare(ot->m_float, m_float);
  genericValCompare(ot->m_double, m_double);
  genericValCompare(ot->m_sbyte, m_sbyte);
  genericValCompare(ot->m_uint16, m_uint16);
  genericValCompare(ot->m_uint32, m_uint32);
  genericValCompare(ot->m_ulong, m_ulong);
  genericValCompare(ot->m_char, m_char);
  if (ot->m_string != m_string) {
    return false;
  }
  genericCompare(ot->m_byteArray.data(), m_byteArray.data(),
                 m_byteArray.size());
  genericCompare(ot->m_int16Array.data(), m_int16Array.data(),
                 m_int16Array.size());
  genericCompare(ot->m_int32Array.data(), m_int32Array.data(),
                 m_int32Array.size());
  genericCompare(ot->m_longArray.data(), m_longArray.data(),
                 m_longArray.size());
  genericCompare(ot->m_uint32Array.data(), m_uint32Array.data(),
                 m_uint32Array.size());
  genericCompare(ot->m_ulongArray.data(), m_ulongArray.data(),
                 m_ulongArray.size());
  genericCompare(ot->m_uint16Array.data(), m_uint16Array.data(),
                 m_uint16Array.size());
  genericCompare(ot->m_sbyteArray.data(), m_sbyteArray.data(),
                 m_sbyteArray.size());
  genericCompare(ot->m_charArray.data(), m_charArray.data(),
                 m_charArray.size());
  // generic2DCompare(ot->m_byteByteArray, m_byteByteArray, byteByteArrayLen,
  // lengthArr);

  if (!isPdxReadSerialized) {
    for (size_t i = 0; i < m_objectArray->size(); i++) {
      Address* otherAddr1 =
          dynamic_cast<Address*>(ot->m_objectArray->at(i).get());
      Address* myAddr1 = dynamic_cast<Address*>(m_objectArray->at(i).get());
      if (!otherAddr1->equals(*myAddr1)) return false;
    }
    LOGINFO("PdxObject::equals isPdxReadSerialized = %d", isPdxReadSerialized);
  }

  // m_objectArrayEmptyPdxFieldName
  if (!isPdxReadSerialized) {
    for (size_t i = 0; i < m_objectArrayEmptyPdxFieldName->size(); i++) {
      Address* otherAddr1 =
          dynamic_cast<Address*>(ot->m_objectArray->at(i).get());
      Address* myAddr1 = dynamic_cast<Address*>(m_objectArray->at(i).get());
      if (!otherAddr1->equals(*myAddr1)) return false;
    }
    LOGINFO("PdxObject::equals Empty Field Name isPdxReadSerialized = %d",
            isPdxReadSerialized);
  }

  auto myenum = std::dynamic_pointer_cast<CacheableEnum>(m_pdxEnum);
  auto otenum = std::dynamic_pointer_cast<CacheableEnum>(ot->m_pdxEnum);
  if (myenum->getEnumOrdinal() != otenum->getEnumOrdinal()) return false;
  if (myenum->getEnumClassName() != otenum->getEnumClassName()) return false;
  if (myenum->getEnumName() != otenum->getEnumName()) return false;

  genericValCompare(ot->m_arraylist->size(), m_arraylist->size());
  for (size_t k = 0; k < m_arraylist->size(); k++) {
    genericValCompare(ot->m_arraylist->at(k), m_arraylist->at(k));
  }

  LOGINFO("Equals Linked List Starts");
  genericValCompare(ot->m_linkedlist->size(), m_linkedlist->size());
  for (size_t k = 0; k < m_linkedlist->size(); k++) {
    genericValCompare(ot->m_linkedlist->at(k), m_linkedlist->at(k));
  }
  LOGINFO("Equals Linked List Finished");

  genericValCompare(ot->m_vector->size(), m_vector->size());
  for (size_t j = 0; j < m_vector->size(); j++) {
    genericValCompare(ot->m_vector->at(j), m_vector->at(j));
  }

  LOGINFO("PdxObject::equals DOne Line_201");
  return true;
}

}  // namespace PdxTests
