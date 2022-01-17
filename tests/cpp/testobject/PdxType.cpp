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
  int32_t* Lengtharr;
  int32_t arrLen = 0;
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

bool DynamicFieldsType::operator==(const DynamicFieldsType& other) const {
  return _aptNumber == other._aptNumber && _street == other._street &&
         _city == other._city && !_char == !other._char &&
         (!_char || *_char == *other._char) && !_bool == !other._bool &&
         (!_bool || *_bool == *other._bool) && !_byte == !other._byte &&
         (!_byte || *_byte == *other._byte) && !_short == !other._short &&
         (!_short || *_short == *other._short) && !_int == !other._int &&
         (!_int || *_int == *other._int) && !_long == !other._long &&
         (!_long || *_long == *other._long) && !_float == !other._float &&
         (!_float || *_float == *other._float) && !_double == !other._double &&
         (!_double || *_double == *other._double) && _str == other._str &&
         !_object == !other._object &&
         (!_object || typeid(_object.get()) == typeid(other._object.get())) &&
         !_date == !other._date && (!_date || *_date == *other._date) &&
         _charArray.size() == other._charArray.size() &&
         _boolArray.size() == other._boolArray.size() &&
         _byteArray.size() == other._byteArray.size() &&
         _shortArray.size() == other._shortArray.size() &&
         _intArray.size() == other._intArray.size() &&
         _longArray.size() == other._longArray.size() &&
         _floatArray.size() == other._floatArray.size() &&
         _doubleArray.size() == other._doubleArray.size() &&
         _strArray.size() == other._strArray.size();
}

std::string DynamicFieldsType::toString() const {
  std::string result = "Apt#: " + std::to_string(_aptNumber) + " | " +
                       "Street: " + _street + " | " + "City: " + _street +
                       " | ";

  if (_char) {
    result += "Char: " + std::to_string(*_char) + " | ";
  }

  if (_bool) {
    result += "Boolean: " + std::string{*_bool ? "true" : "false"} + " | ";
  }

  if (_byte) {
    result += "Byte: " + std::to_string(static_cast<int32_t>(*_byte)) + " | ";
  }

  if (_short) {
    result += "Short: " + std::to_string(static_cast<int32_t>(*_short)) + " | ";
  }

  if (_int) {
    result += "Integer: " + std::to_string(*_int) + " | ";
  }

  if (_long) {
    result += "Long: " + std::to_string(*_long) + " | ";
  }

  if (_float) {
    result += "Float: " + std::to_string(*_float) + " | ";
  }

  if (_double) {
    result += "Double: " + std::to_string(*_double) + " | ";
  }

  if (!_str.empty()) {
    result += "String: " + _str + " | ";
  }

  if (_date) {
    result += "Date: " + _date->toString() + " | ";
  }

  if (_object) {
    result += "Object: " + _object->toString() + " | ";
  }

  if (!_charArray.empty()) {
    result += "CharArray[" + std::to_string(_charArray.size()) + "] | ";
  }

  if (!_boolArray.empty()) {
    result += "BoolArray[" + std::to_string(_boolArray.size()) + "] | ";
  }

  if (!_byteArray.empty()) {
    result += "ByteArray[" + std::to_string(_byteArray.size()) + "] | ";
  }

  if (!_shortArray.empty()) {
    result += "ShortArray[" + std::to_string(_shortArray.size()) + "] | ";
  }

  if (!_intArray.empty()) {
    result += "IntArray[" + std::to_string(_intArray.size()) + "] | ";
  }

  if (!_longArray.empty()) {
    result += "LongArray[" + std::to_string(_longArray.size()) + "] | ";
  }

  if (!_floatArray.empty()) {
    result += "FloatArray[" + std::to_string(_floatArray.size()) + "] | ";
  }

  if (!_doubleArray.empty()) {
    result += "DoubleArray[" + std::to_string(_doubleArray.size()) + "] | ";
  }

  if (!_strArray.empty()) {
    result += "StringArray[" + std::to_string(_strArray.size()) + "] | ";
  }

  return result;
}

const std::string& DynamicFieldsType::getClassName() const {
  static std::string className = "PdxTests.DynamicAddress";
  return className;
}

void DynamicFieldsType::toData(PdxWriter& pw) const {
  pw.writeInt("_aptNumber", _aptNumber);  // 4
  pw.writeString("_street", _street);
  pw.writeString("_city", _city);

  if (_char) {
    pw.writeChar("_char", *_char);
  }

  if (_bool) {
    pw.writeBoolean("_bool", *_bool);
  }

  if (_byte) {
    pw.writeByte("_byte", *_byte);
  }

  if (_short) {
    pw.writeShort("_short", *_short);
  }

  if (_int) {
    pw.writeInt("_int", *_int);
  }

  if (_long) {
    pw.writeLong("_long", *_long);
  }

  if (_float) {
    pw.writeFloat("_float", *_float);
  }

  if (_double) {
    pw.writeDouble("_double", *_double);
  }

  if (!_str.empty()) {
    pw.writeString("_str", _str);
  }

  if (!_str.empty()) {
    pw.writeString("_str", _str);
  }

  if (_date) {
    pw.writeDate("_date", _date);
  }

  if (_object) {
    pw.writeObject("_object", _object);
  }

  if (!_charArray.empty()) {
    pw.writeCharArray("_object", _charArray);
  }

  if (!_boolArray.empty()) {
    pw.writeBooleanArray("_boolArray", _boolArray);
  }

  if (!_byteArray.empty()) {
    pw.writeByteArray("_byteArray", _byteArray);
  }

  if (!_shortArray.empty()) {
    pw.writeShortArray("_shortArray", _shortArray);
  }

  if (!_intArray.empty()) {
    pw.writeIntArray("_intArray", _intArray);
  }

  if (!_longArray.empty()) {
    pw.writeLongArray("_longArray", _longArray);
  }

  if (!_floatArray.empty()) {
    pw.writeFloatArray("_floatArray", _floatArray);
  }

  if (!_doubleArray.empty()) {
    pw.writeDoubleArray("_doubleArray", _doubleArray);
  }

  if (!_strArray.empty()) {
    pw.writeStringArray("_strArray", _strArray);
  }
}

void DynamicFieldsType::fromData(PdxReader& pr) {
  _aptNumber = pr.readInt("_aptNumber");
  _street = pr.readString("_street");
  _city = pr.readString("_city");

  if (pr.hasField("_char")) {
    _char = std::unique_ptr<char16_t>(new char16_t);
    *_char = pr.readChar("_char");
  }

  if (pr.hasField("_bool")) {
    _bool = std::unique_ptr<bool>(new bool);
    *_bool = pr.readBoolean("_bool");
  }

  if (pr.hasField("_byte")) {
    _byte = std::unique_ptr<int8_t>(new int8_t);
    *_byte = pr.readByte("_byte");
  }

  if (pr.hasField("_short")) {
    _short = std::unique_ptr<int16_t>(new int16_t);
    *_short = pr.readShort("_short");
  }

  if (pr.hasField("_int")) {
    _int = std::unique_ptr<int32_t>(new int32_t);
    *_int = pr.readInt("_int");
  }

  if (pr.hasField("_long")) {
    _long = std::unique_ptr<int64_t>(new int64_t);
    *_long = pr.readLong("_long");
  }

  if (pr.hasField("_float")) {
    _float = std::unique_ptr<float>(new float);
    *_float = pr.readFloat("_float");
  }

  if (pr.hasField("_double")) {
    _double = std::unique_ptr<double>(new double);
    *_double = pr.readFloat("_double");
  }

  if (pr.hasField("_str")) {
    _str = pr.readString("_str");
  }

  if (pr.hasField("_date")) {
    _date = pr.readDate("_date");
  }

  if (pr.hasField("_object")) {
    _object = pr.readObject("_object");
  }

  if (pr.hasField("_charArray")) {
    _charArray = pr.readCharArray("_charArray");
  }

  if (pr.hasField("_boolArray")) {
    _boolArray = pr.readBooleanArray("_boolArray");
  }

  if (pr.hasField("_byteArray")) {
    _byteArray = pr.readByteArray("_byteArray");
  }

  if (pr.hasField("_shortArray")) {
    _shortArray = pr.readShortArray("_shortArray");
  }

  if (pr.hasField("_intArray")) {
    _intArray = pr.readIntArray("_intArray");
  }

  if (pr.hasField("_longArray")) {
    _longArray = pr.readLongArray("_longArray");
  }

  if (pr.hasField("_floatArray")) {
    _floatArray = pr.readFloatArray("_floatArray");
  }

  if (pr.hasField("_doubleArray")) {
    _doubleArray = pr.readDoubleArray("_doubleArray");
  }

  if (pr.hasField("_strArray")) {
    _strArray = pr.readStringArray("_strArray");
  }
}

std::ostream& operator<<(std::ostream& os, const DynamicFieldsType& address) {
  os << address.toString();
  return os;
}

}  // namespace PdxTests
