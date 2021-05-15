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

#include "PdxVersioned1.hpp"

#include <util/Log.hpp>

namespace PdxTests {

using apache::geode::client::CacheableInt32;

template <typename T1, typename T2>
bool PdxTests::PdxVersioned1::genericValCompare(T1 value1, T2 value2) const {
  if (value1 != value2) return false;
  LOG_INFO("PdxObject::genericValCompare Line_19");
  return true;
}

template <typename T1, typename T2, typename L>
bool PdxTests::PdxVersioned1::genericCompare(T1* value1, T2* value2,
                                             L length) const {
  L i = 0;
  while (i < length) {
    if (value1[i] != value2[i]) {
      return false;
    } else {
      i++;
    }
  }
  LOG_INFO("PdxObject::genericCompare Line_34");
  return true;
}

template <typename T1, typename T2>
bool PdxTests::PdxVersioned1::generic2DCompare(T1** value1, T2** value2,
                                               int length,
                                               int* arrLengths) const {
  LOG_INFO("generic2DCompare length = %d ", length);
  LOG_INFO("generic2DCompare value1 = %d \t value2", value1[0][0],
           value2[0][0]);
  LOG_INFO("generic2DCompare value1 = %d \t value2", value1[1][0],
           value2[1][0]);
  LOG_INFO("generic2DCompare value1 = %d \t value2", value1[1][1],
           value2[1][1]);
  for (int j = 0; j < length; j++) {
    LOG_INFO("generic2DCompare arrlength0 = %d ", arrLengths[j]);
    for (int k = 0; k < arrLengths[j]; k++) {
      LOG_INFO("generic2DCompare arrlength = %d ", arrLengths[j]);
      LOG_INFO("generic2DCompare value1 = %d \t value2 = %d ", value1[j][k],
               value2[j][k]);
      if (value1[j][k] != value2[j][k]) return false;
    }
  }
  LOG_INFO("PdxObject::genericCompare Line_34");
  return true;
}
/*void PdxVersioned1::checkNullAndDelete(void *data)
{
        if(data != NULL)
                delete[] data;
}
PdxVersioned1::~PdxVersioned1() {
         LOG_INFO("~PdxVersioned1 - 1");
                checkNullAndDelete( m_string);
                LOG_INFO("~PdxVersioned1 - 2");
                checkNullAndDelete( m_byteArray);
                LOG_INFO("~PdxVersioned1 - 3");
                checkNullAndDelete( m_boolArray);
                LOG_INFO("~PdxVersioned1 - 4");
                checkNullAndDelete( m_sbyteArray);
                LOG_INFO("~PdxVersioned1 - 5");
                checkNullAndDelete( m_charArray);
                LOG_INFO("~PdxVersioned1 - 6");
                checkNullAndDelete( m_uint16Array);
                LOG_INFO("~PdxVersioned1 - 7");
                checkNullAndDelete( m_uint16Array);
                LOG_INFO("~PdxVersioned1 - 8");
                checkNullAndDelete( m_int32Array);
                LOG_INFO("~PdxVersioned1 - 9");
                checkNullAndDelete( m_uint32Array);
                LOG_INFO("~PdxVersioned1 - 10");
                checkNullAndDelete( m_longArray);
                LOG_INFO("~PdxVersioned1 - 11");
                checkNullAndDelete( m_ulongArray);
                LOG_INFO("~PdxVersioned1 - 12");
                checkNullAndDelete( m_floatArray);
                LOG_INFO("~PdxVersioned1 - 13");
                checkNullAndDelete( m_doubleArray);
                LOG_INFO("~PdxVersioned1 - 14");
                checkNullAndDelete( m_byteByteArray);
                LOG_INFO("~PdxVersioned1 - 15");
                checkNullAndDelete( m_stringArray);
                LOG_INFO("~PdxVersioned1 - 16");
                checkNullAndDelete( m_byte252);
                LOG_INFO("~PdxVersioned1 -17 ");
                checkNullAndDelete( m_byte253);
                LOG_INFO("~PdxVersioned1 - 18");
                checkNullAndDelete( m_byte65535);
                LOG_INFO("~PdxVersioned1 - 19");
                checkNullAndDelete( m_byte65536);
                LOG_INFO("~PdxVersioned1 -20 ");
}*/
PdxVersioned1::PdxVersioned1(const char* key) { init(key); }

void PdxVersioned1::init(const char* key) {
  m_char = 'C';
  m_bool = true;
  m_byte = 0x74;
  m_sbyte = 0x67;
  m_int16 = 0xab;
  m_uint16 = 0x2dd5;
  m_int32 = 0x2345abdc;
  // m_uint32 = 0x2a65c434;
  // m_long = 324897980;
  m_ulong = 238749898;
  m_float = 23324.324f;
  m_double = 3243298498.00;
  m_string = std::string("PdxVersioned ") + key;
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

  m_sbyteArray = std::vector<int8_t>{0x34, 0x64};

  m_charArray = std::vector<char16_t>{u'c', u'v'};

  int64_t d = 1310447869154L;
  m_date = CacheableDate::create(CacheableDate::duration(d));

  m_int16Array = std::vector<int16_t>{0x2332, 0x4545};

  m_uint16Array = std::vector<int16_t>{0x3243, 0x3232};

  m_int32Array = std::vector<int32_t>{23, 676868, 34343, 2323};

  m_uint32Array = std::vector<int32_t>{435, 234324, 324324, 23432432};

  m_longArray = std::vector<int64_t>{324324L, 23434545L};

  m_ulongArray = std::vector<int64_t>{3245435, 3425435};

  m_floatArray = std::vector<float>{232.565f, 2343254.67f};

  m_doubleArray = std::vector<double>{23423432, 4324235435.00};

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
  m_pdxEnum = CacheableEnum::create("pdxEnumTest", "pdx2", pdx2);
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
}

void PdxTests::PdxVersioned1::toData(PdxWriter& pw) const {
  // TODO:delete it later

  int* lengths = new int[2];

  lengths[0] = 1;
  lengths[1] = 2;
  pw.writeArrayOfByteArrays("m_byteByteArray", m_byteByteArray, 2, lengths);
  pw.writeChar("m_char", m_char);
  pw.writeBoolean("m_bool", m_bool);  // 1
  pw.writeBooleanArray("m_boolArray", m_boolArray);
  pw.writeByte("m_byte", m_byte);
  pw.writeByteArray("m_byteArray", m_byteArray);
  pw.writeCharArray("m_charArray", m_charArray);
  pw.writeObject("m_arraylist", m_arraylist);
  pw.writeObject("m_map", m_map);
  pw.writeString("m_string", m_string);
  pw.writeDate("m_dateTime", m_date);
  pw.writeDouble("m_double", m_double);
  pw.writeDoubleArray("m_doubleArray", m_doubleArray);
  pw.writeFloat("m_float", m_float);
  pw.writeFloatArray("m_floatArray", m_floatArray);
  pw.writeShort("m_int16", m_int16);
  pw.writeInt("m_int32", m_int32);
  // pw.writeLong("m_long", m_long);
  pw.writeIntArray("m_int32Array", m_int32Array);
  pw.writeLongArray("m_longArray", m_longArray);
  pw.writeShortArray("m_int16Array", m_int16Array);
  pw.writeByte("m_sbyte", m_sbyte);
  pw.writeByteArray("m_sbyteArray", m_sbyteArray);
  // int* strlengthArr = new int[2];

  // strlengthArr[0] = 5;
  // strlengthArr[1] = 5;
  pw.writeStringArray("m_stringArray", m_stringArray);
  pw.writeShort("m_uint16", m_uint16);
  // pw.writeInt("m_uint32", m_uint32);
  pw.writeLong("m_ulong", m_ulong);
  pw.writeIntArray("m_uint32Array", m_uint32Array);
  pw.writeLongArray("m_ulongArray", m_ulongArray);
  pw.writeShortArray("m_uint16Array", m_uint16Array);
  pw.writeByteArray("m_byte252", m_byte252);
  pw.writeByteArray("m_byte253", m_byte253);
  pw.writeByteArray("m_byte65535", m_byte65535);
  pw.writeByteArray("m_byte65536", m_byte65536);
  pw.writeObject("m_pdxEnum", m_pdxEnum);

  // TODO:delete it later
}

void PdxTests::PdxVersioned1::fromData(PdxReader& pr) {
  // TODO:temp added, delete later

  int32_t* Lengtharr;
  _GEODE_NEW(Lengtharr, int32_t[2]);
  int32_t arrLen = 0;
  m_byteByteArray =
      pr.readArrayOfByteArrays("m_byteByteArray", arrLen, &Lengtharr);
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
  m_map = std::dynamic_pointer_cast<CacheableHashMap>(pr.readObject("m_map"));
  // TODO:Check for the size
  m_string = pr.readString("m_string");  // GenericValCompare
  m_date = pr.readDate("m_dateTime");    // compareData
  m_double = pr.readDouble("m_double");
  m_doubleArray = pr.readDoubleArray("m_doubleArray");
  m_float = pr.readFloat("m_float");
  m_floatArray = pr.readFloatArray("m_floatArray");
  m_int16 = pr.readShort("m_int16");
  m_int32 = pr.readInt("m_int32");
  // m_long = pr.readLong("m_long");
  m_int32Array = pr.readIntArray("m_int32Array");
  m_longArray = pr.readLongArray("m_longArray");
  m_int16Array = pr.readShortArray("m_int16Array");
  m_sbyte = pr.readByte("m_sbyte");
  m_sbyteArray = pr.readByteArray("m_sbyteArray");
  m_stringArray = pr.readStringArray("m_stringArray");
  m_uint16 = pr.readShort("m_uint16");
  // m_uint32 = pr.readInt("m_uint32");
  m_ulong = pr.readLong("m_ulong");
  m_uint32Array = pr.readIntArray("m_uint32Array");
  m_ulongArray = pr.readLongArray("m_ulongArray");
  m_uint16Array = pr.readShortArray("m_uint16Array");
  // LOG_INFO("PdxVersioned1::readInt() start...");

  m_byte252 = pr.readByteArray("m_byte252");
  m_byte253 = pr.readByteArray("m_byte253");
  m_byte65535 = pr.readByteArray("m_byte65535");
  m_byte65536 = pr.readByteArray("m_byte65536");
  // TODO:Check for size
  m_pdxEnum = pr.readObject("m_pdxEnum");
}

std::string PdxTests::PdxVersioned1::toString() const {
  return "PdxVersioned 1 : " + m_string;
}

bool PdxTests::PdxVersioned1::equals(PdxTests::PdxVersioned1& other) const {
  PdxVersioned1* ot = dynamic_cast<PdxVersioned1*>(&other);
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
  // genericValCompare(ot->m_long, m_long);
  genericValCompare(ot->m_float, m_float);
  genericValCompare(ot->m_double, m_double);
  genericValCompare(ot->m_sbyte, m_sbyte);
  genericValCompare(ot->m_uint16, m_uint16);
  // genericValCompare(ot->m_uint32, m_uint32);
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

  // auto myenum = std::dynamic_pointer_cast<CacheableEnum>(m_pdxEnum);
  // auto otenum = std::dynamic_pointer_cast<CacheableEnum>(ot->m_pdxEnum);
  // if (myenum->getEnumOrdinal() != otenum->getEnumOrdinal()) return false;
  // if (strcmp(myenum->getEnumClassName(), otenum->getEnumClassName()) != 0)
  // return false;
  // if (strcmp(myenum->getEnumName(), otenum->getEnumName()) != 0) return
  // false;

  genericValCompare(ot->m_arraylist->size(), m_arraylist->size());
  for (size_t k = 0; k < m_arraylist->size(); k++) {
    genericValCompare(ot->m_arraylist->at(k), m_arraylist->at(k));
  }

  return true;
}
}  // namespace PdxTests
