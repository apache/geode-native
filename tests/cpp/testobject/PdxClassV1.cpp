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

#include "PdxClassV1.hpp"

namespace PdxTests {

using apache::geode::client::Exception;

/************************************************************
 *  PdxType1V1
 * *********************************************************/
int32_t PdxType1V1::m_diffInSameFields = 0;
bool PdxType1V1::m_useWeakHashMap = false;

PdxType1V1::PdxType1V1() {
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxType1V1::~PdxType1V1() noexcept {
  // TODO Auto-generated destructor stub
}

void PdxType1V1::reset(bool useWeakHashMap) {
  m_diffInSameFields = 0;
  m_useWeakHashMap = useWeakHashMap;
}

int PdxType1V1::getHashCode() {
  // TODO:Implement It.
  return 1;
}

bool PdxType1V1::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxType1V1>(obj);
  if (pap == nullptr) return false;

  if (pap.get() == this) return true;

  if (m_i1 + m_diffInSameFields <= pap->m_i1 &&
      m_i2 + m_diffInSameFields <= pap->m_i2 &&
      m_i3 + m_diffInSameFields <= pap->m_i3 &&
      m_i4 + m_diffInSameFields <= pap->m_i4) {
    return true;
  }

  return false;
}

void PdxType1V1::toData(PdxWriter& pw) const {
  if (!m_useWeakHashMap) pw.writeUnreadFields(m_pdxUreadFields);

  pw.writeInt("i1", m_i1 + 1);
  pw.markIdentityField("i1");
  pw.writeInt("i2", m_i2 + 1);
  pw.writeInt("i3", m_i3 + 1);
  pw.writeInt("i4", m_i4 + 1);

  m_diffInSameFields++;

  /*
  pw.writeBoolean("m_bool", m_bool);
  pw.writeByte("m_byte", m_byte);
  pw.writeShort("m_int16", m_int16);
  pw.writeInt("m_int32", m_int32);
  pw.writeLong("m_long", m_long);
  pw.writeFloat("m_float", m_float);
  pw.writeDouble("m_double", m_double);
  pw.writeString("m_string", m_string);
  pw.writeBooleanArray("m_boolArray", m_boolArray, 3);
  pw.writeByteArray("m_byteArray",m_byteArray, 2);
  pw.writeShortArray("m_int16Array", m_int16Array, 2);
  pw.writeIntArray("m_int32Array", m_int32Array, 4);
  pw.writeLongArray("m_longArray", m_longArray, 2);
  pw.writeFloatArray("m_floatArray", m_floatArray, 2);
  pw.writeDoubleArray("m_doubleArray", m_doubleArray, 2);
*/
  // LOG_DEBUG("PdxObject::toData() Done......");
}

void PdxType1V1::fromData(PdxReader& pr) {
  // LOG_DEBUG("PdxObject::fromData() start...");

  if (!m_useWeakHashMap) m_pdxUreadFields = pr.readUnreadFields();

  m_i1 = pr.readInt("i1");
  m_i2 = pr.readInt("i2");
  m_i3 = pr.readInt("i3");
  m_i4 = pr.readInt("i4");

  bool hasField = pr.hasField("i7");

  if (hasField) throw Exception("i7 is not an valid field");

  hasField = pr.hasField("i4");

  if (!hasField) throw Exception("i4 is an valid field");

  /*
  m_bool = pr.readBoolean("m_bool");
  //LOG_DEBUG("PdxObject::fromData() -1 m_bool = %d", m_bool);
  m_byte = pr.readByte("m_byte");
  //LOG_DEBUG("PdxObject::fromData() -2 m_byte =%d", m_byte);
  m_int16 = pr.readShort("m_int16");
  //LOG_DEBUG("PdxObject::fromData() -3 m_int16=%d", m_int16);
  m_int32 = pr.readInt("m_int32");
  //LOG_DEBUG("PdxObject::fromData() -4 m_int32=%d", m_int32);
  m_long = pr.readLong("m_long");
  //LOG_DEBUG("PdxObject::fromData() -5 m_long=%lld", m_long);
  m_float = pr.readFloat("m_float");
  //LOG_DEBUG("PdxObject::fromData() -6 m_float = %f", m_float);
  m_double = pr.readDouble("m_double");
  //LOG_DEBUG("PdxObject::fromData() -7  m_double=%llf", m_double);
  m_string = pr.readString("m_string");
  //LOG_DEBUG("PdxObject::fromData() -8  m_string=%s", m_string);
  m_boolArray = pr.readBooleanArray("m_boolArray");
  m_byteArray = pr.readByteArray("m_byteArray");
  m_int16Array = pr.readShortArray("m_int16Array");
  m_int32Array = pr.readIntArray("m_int32Array");
  m_longArray = pr.readLongArray("m_longArray");
  m_floatArray = pr.readFloatArray("m_floatArray");
  m_doubleArray = pr.readDoubleArray("m_doubleArray");
  //LOG_DEBUG("PdxObject::fromData() -8  m_boolArray[0]=%d", m_boolArray[0]);
  //m_int32 = pr.readInt("m_int32");
  */
  LOG_DEBUG("PdxObject::fromData() End...");
}
std::string PdxType1V1::toString() const {
  char idbuf[4096];
  // sprintf(idbuf,"PdxObject: [ m_bool=%d ] [m_byte=%d] [m_int16=%d]
  // [m_int32=%d] [m_long=%lld] [m_float=%f] [ m_string=%s ]",m_bool, m_byte,
  // m_int16, m_int32, m_long, m_float, m_string);
  // sprintf(idbuf,"PdxObject: [ m_bool=%d ] [m_byte=%d] [m_int16=%d]
  // [m_int32=%d] [m_long=%lld] [m_float=%f] [m_double=%Lf] [ m_string=%s
  // ]",m_bool, m_byte, m_int16, m_int32, m_long, m_float, m_double, m_string);
  // sprintf(idbuf,"PdxObject: [ m_bool=%d ] [m_byte=%d] [m_int16=%d]
  // [m_int32=%d] [m_float=%f] [m_double=%lf] [ m_string=%s ]",m_bool, m_byte,
  // m_int16, m_int32, m_float, m_double, m_string);
  sprintf(idbuf, "PdxType1V1:[ m_i1=%d ] [ m_i2=%d ] [ m_i3=%d ] [ m_i4=%d ]",
          m_i1, m_i2, m_i3, m_i4);
  return idbuf;
}

/************************************************************
 *  PdxType2V1
 * *********************************************************/

int PdxType2V1::m_diffInSameFields = 0;
bool PdxType2V1::m_useWeakHashMap = false;

PdxType2V1::PdxType2V1() {
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxType2V1::~PdxType2V1() noexcept {
  // TODO Auto-generated destructor stub
}

void PdxType2V1::reset(bool useWeakHashMap) {
  PdxType2V1::m_diffInSameFields = 0;
  PdxType2V1::m_useWeakHashMap = useWeakHashMap;
}

int PdxType2V1::getHashCode() {
  // TODO:Implement It.
  return 1;
}

bool PdxType2V1::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxType2V1>(obj);
  if (pap == nullptr) return false;

  if (pap.get() == this) return true;

  if (m_i1 + m_diffInSameFields <= pap->m_i1 &&
      m_i2 + m_diffInSameFields <= pap->m_i2 &&
      m_i3 + m_diffInSameFields <= pap->m_i3 &&
      m_i4 + m_diffInSameFields <= pap->m_i4) {
    return true;
  }

  return false;
}

void PdxType2V1::toData(PdxWriter& pw) const {
  if (!m_useWeakHashMap) pw.writeUnreadFields(m_unreadFields);

  pw.writeInt("i1", m_i1 + 1);
  pw.writeInt("i2", m_i2 + 1);
  pw.writeInt("i3", m_i3 + 1);
  pw.writeInt("i4", m_i4 + 1);
}

void PdxType2V1::fromData(PdxReader& pr) {
  // LOG_DEBUG("PdxObject::fromData() start...");

  if (!m_useWeakHashMap) m_unreadFields = pr.readUnreadFields();

  m_i1 = pr.readInt("i1");
  m_i2 = pr.readInt("i2");
  m_i3 = pr.readInt("i3");
  m_i4 = pr.readInt("i4");
}
std::string PdxType2V1::toString() const {
  char idbuf[4096];
  sprintf(idbuf, "PdxType2V1:[ m_i1=%d ] [ m_i2=%d ] [ m_i3=%d ] [ m_i4=%d ]",
          m_i1, m_i2, m_i3, m_i4);
  return idbuf;
}

/************************************************************
 *  PdxType3V1
 * *********************************************************/

int PdxType3V1::m_diffInSameFields = 0;
int PdxType3V1::m_diffInExtraFields = 0;
bool PdxType3V1::m_useWeakHashMap = false;

PdxType3V1::PdxType3V1() {
  m_i1 = 1;
  m_i2 = 21;
  m_str1 = "common";
  m_i3 = 31;
  m_i4 = 41;
  m_i5 = 0;
  m_str2 = "0";
}

PdxType3V1::~PdxType3V1() noexcept {
  // TODO Auto-generated destructor stub
}

void PdxType3V1::reset(bool useWeakHashMap) {
  PdxType3V1::m_diffInSameFields = 0;
  PdxType3V1::m_diffInExtraFields = 0;
  PdxType3V1::m_useWeakHashMap = useWeakHashMap;
}

int PdxType3V1::getHashCode() {
  // TODO:Implement It.
  return 1;
}

bool PdxType3V1::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxType3V1>(obj);
  if (pap == nullptr) return false;

  if (pap.get() == this) return true;

  if (m_i1 + m_diffInSameFields <= pap->m_i1 &&
      m_i2 + m_diffInSameFields <= pap->m_i2 &&
      m_i3 + m_diffInSameFields <= pap->m_i3 &&
      m_i4 + m_diffInSameFields <= pap->m_i4 &&
      m_i5 + m_diffInExtraFields == pap->m_i5) {
    if (m_str1 == pap->m_str1 && m_str2 == pap->m_str2) {
      return true;
    }
  }
  return false;
}

void PdxType3V1::toData(PdxWriter& pw) const {
  if (!m_useWeakHashMap) pw.writeUnreadFields(m_unreadFields);

  pw.writeInt("i1", m_i1 + 1);
  pw.writeInt("i2", m_i2 + 1);
  pw.writeString("m_str1", m_str1);
  pw.writeInt("i3", m_i3 + 1);
  pw.writeInt("i4", m_i4 + 1);
  pw.writeInt("i5", m_i5 + 1);
  pw.writeString("m_str2", m_str2);

  m_diffInSameFields++;
  m_diffInExtraFields++;
}

void PdxType3V1::fromData(PdxReader& pr) {
  // LOG_DEBUG("PdxObject::fromData() start...");

  if (!m_useWeakHashMap) m_unreadFields = pr.readUnreadFields();

  m_i1 = pr.readInt("i1");
  m_i2 = pr.readInt("i2");
  m_str1 = pr.readString("m_str1");
  m_i3 = pr.readInt("i3");
  m_i4 = pr.readInt("i4");
  m_i5 = pr.readInt("i5");
  auto tmp = pr.readString("m_str2");

  if (tmp.empty()) {
    m_str2 = std::to_string(m_diffInExtraFields);
  } else {
    m_str2 = std::to_string(m_diffInExtraFields) + m_str2;
  }
}

std::string PdxType3V1::toString() const {
  return "PdxType2V1:[ m_i1=" + std::to_string(m_i1) +
         " ] [ m_i2=" + std::to_string(m_i2) + " ] [m_str1=" + m_str1 +
         "] [ m_i3=" + std::to_string(m_i3) +
         " ] [  m_i4=" + std::to_string(m_i4) +
         " ] [ m_i5=" + std::to_string(m_i5) + " ] [m_str2=" + m_str2 + "]";
}

/************************************************************
 *  PdxTypesV1R1
 * *********************************************************/
int PdxTypesV1R1::m_diffInSameFields = 0;
bool PdxTypesV1R1::m_useWeakHashMap = false;

PdxTypesV1R1::PdxTypesV1R1() {
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxTypesV1R1::~PdxTypesV1R1() noexcept {
  // TODO Auto-generated destructor stub
}

void PdxTypesV1R1::reset(bool useWeakHashMap) {
  PdxTypesV1R1::m_diffInSameFields = 0;
  PdxTypesV1R1::m_useWeakHashMap = useWeakHashMap;
}

int PdxTypesV1R1::getHashCode() {
  // TODO:Implement It.
  return 1;
}

bool PdxTypesV1R1::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypesV1R1>(obj);
  if (pap == nullptr) return false;

  if (pap.get() == this) return true;

  if (m_i1 + m_diffInSameFields <= pap->m_i1 &&
      m_i2 + m_diffInSameFields <= pap->m_i2 &&
      m_i3 + m_diffInSameFields <= pap->m_i3 &&
      m_i4 + m_diffInSameFields <= pap->m_i4) {
    return true;
  }
  return false;
}

void PdxTypesV1R1::toData(PdxWriter& pw) const {
  if (!m_useWeakHashMap) pw.writeUnreadFields(m_pdxUreadFields);

  pw.writeInt("i1", m_i1 + 1);
  pw.writeInt("i2", m_i2 + 1);
  pw.writeInt("i3", m_i3 + 1);
  pw.writeInt("i4", m_i4 + 1);

  m_diffInSameFields++;
}

void PdxTypesV1R1::fromData(PdxReader& pr) {
  if (!m_useWeakHashMap) m_pdxUreadFields = pr.readUnreadFields();

  m_i1 = pr.readInt("i1");
  m_i2 = pr.readInt("i2");
  m_i3 = pr.readInt("i3");
  m_i4 = pr.readInt("i4");
}
std::string PdxTypesV1R1::toString() const {
  char idbuf[4096];
  sprintf(idbuf, "PdxTypesV1R1:[ m_i1=%d ] [ m_i2=%d ] [ m_i3=%d ] [ m_i4=%d ]",
          m_i1, m_i2, m_i3, m_i4);
  return idbuf;
}

/************************************************************
 *  PdxTypesV1R2
 * *********************************************************/
int PdxTypesV1R2::m_diffInSameFields = 0;
bool PdxTypesV1R2::m_useWeakHashMap = false;

PdxTypesV1R2::PdxTypesV1R2() {
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxTypesV1R2::~PdxTypesV1R2() noexcept {
  // TODO Auto-generated destructor stub
}

void PdxTypesV1R2::reset(bool useWeakHashMap) {
  PdxTypesV1R2::m_diffInSameFields = 0;
  PdxTypesV1R2::m_useWeakHashMap = useWeakHashMap;
}

int PdxTypesV1R2::getHashCode() {
  // TODO:Implement It.
  return 1;
}

bool PdxTypesV1R2::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypesV1R2>(obj);
  if (pap == nullptr) return false;

  if (pap.get() == this) return true;

  if (m_i1 + m_diffInSameFields <= pap->m_i1 &&
      m_i2 + m_diffInSameFields <= pap->m_i2 &&
      m_i3 + m_diffInSameFields <= pap->m_i3 &&
      m_i4 + m_diffInSameFields <= pap->m_i4) {
    return true;
  }
  return false;
}

void PdxTypesV1R2::toData(PdxWriter& pw) const {
  if (!m_useWeakHashMap) pw.writeUnreadFields(m_pdxUreadFields);

  pw.writeInt("i1", m_i1 + 1);
  pw.writeInt("i2", m_i2 + 1);
  pw.writeInt("i3", m_i3 + 1);
  pw.writeInt("i4", m_i4 + 1);
}

void PdxTypesV1R2::fromData(PdxReader& pr) {
  // LOG_DEBUG("PdxObject::fromData() start...");
  if (!m_useWeakHashMap) m_pdxUreadFields = pr.readUnreadFields();

  m_i1 = pr.readInt("i1");
  m_i2 = pr.readInt("i2");
  m_i3 = pr.readInt("i3");
  m_i4 = pr.readInt("i4");
}
std::string PdxTypesV1R2::toString() const {
  char idbuf[4096];
  sprintf(idbuf, "PdxTypesV1R1:[ m_i1=%d ] [ m_i2=%d ] [ m_i3=%d ] [ m_i4=%d ]",
          m_i1, m_i2, m_i3, m_i4);
  return idbuf;
}

/************************************************************
 *  PdxTypesIgnoreUnreadFieldsV1
 * *********************************************************/
int PdxTypesIgnoreUnreadFieldsV1::m_diffInSameFields = 0;
bool PdxTypesIgnoreUnreadFieldsV1::m_useWeakHashMap = false;

PdxTypesIgnoreUnreadFieldsV1::PdxTypesIgnoreUnreadFieldsV1() {
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxTypesIgnoreUnreadFieldsV1::~PdxTypesIgnoreUnreadFieldsV1() noexcept {
  // TODO Auto-generated destructor stub
}

void PdxTypesIgnoreUnreadFieldsV1::reset(bool useWeakHashMap) {
  PdxTypesIgnoreUnreadFieldsV1::m_diffInSameFields = 0;
  PdxTypesIgnoreUnreadFieldsV1::m_useWeakHashMap = useWeakHashMap;
}

int PdxTypesIgnoreUnreadFieldsV1::getHashCode() {
  // TODO:Implement It.
  return 1;
}

bool PdxTypesIgnoreUnreadFieldsV1::equals(
    std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypesIgnoreUnreadFieldsV1>(obj);
  if (pap == nullptr) return false;

  if (pap.get() == this) return true;

  if (m_i1 + m_diffInSameFields <= pap->m_i1 &&
      m_i2 + m_diffInSameFields <= pap->m_i2 &&
      m_i3 + m_diffInSameFields <= pap->m_i3 &&
      m_i4 + m_diffInSameFields <= pap->m_i4) {
    return true;
  }
  return false;
}

void PdxTypesIgnoreUnreadFieldsV1::toData(PdxWriter& pw) const {
  if (!m_useWeakHashMap) pw.writeUnreadFields(m_unreadFields);

  pw.writeInt("i1", m_i1 + 1);
  pw.markIdentityField("i1");
  pw.writeInt("i2", m_i2 + 1);
  pw.writeInt("i3", m_i3 + 1);
  pw.writeInt("i4", m_i4 + 1);

  m_diffInSameFields++;
}

void PdxTypesIgnoreUnreadFieldsV1::fromData(PdxReader& pr) {
  // LOG_DEBUG("PdxObject::fromData() start...");

  if (!m_useWeakHashMap) m_unreadFields = pr.readUnreadFields();

  m_i1 = pr.readInt("i1");
  m_i2 = pr.readInt("i2");
  m_i3 = pr.readInt("i3");
  m_i4 = pr.readInt("i4");

  bool hasField = pr.hasField("i7");
  if (hasField) throw Exception("i7 is not an valid field");

  hasField = pr.hasField("i4");
  if (!hasField) throw Exception("i4 is an valid field");
}
std::string PdxTypesIgnoreUnreadFieldsV1::toString() const {
  char idbuf[4096];
  sprintf(idbuf, "PdxTypesV1R1:[ m_i1=%d ] [ m_i2=%d ] [ m_i3=%d ] [ m_i4=%d ]",
          m_i1, m_i2, m_i3, m_i4);
  return idbuf;
}

/************************************************************
 *  PdxVersionedV1
 * *********************************************************/

PdxVersionedV1::PdxVersionedV1() {}

void PdxVersionedV1::init(int32_t size) {
  m_char = 'C';
  m_bool = true;
  m_byte = 0x74;
  m_int16 = 0xab;
  m_int32 = 0x2345abdc;
  m_long = 324897980;
  m_float = 23324.324f;
  m_double = 3243298498;

  m_string = "gfestring";

  m_boolArray = std::vector<bool>(3);
  m_boolArray[0] = true;
  m_boolArray[1] = false;
  m_boolArray[2] = true;

  m_charArray = new char[2];
  m_charArray[0] = 'c';
  m_charArray[1] = 'v';

  m_dateTime = nullptr;

  m_int16Array = std::vector<int16_t>(size);
  m_int32Array = std::vector<int32_t>(size);
  m_longArray = std::vector<int64_t>(size);
  m_floatArray = std::vector<float>(size);
  m_doubleArray = std::vector<double>(size);

  for (int i = 0; i < size; i++) {
    m_int16Array[i] = 0x2332;
    m_int32Array[i] = 34343 + i;
    m_longArray[i] = 324324L + i;
    m_floatArray[i] = 232.565f + i;
    m_doubleArray[i] = 23423432 + i;
    // m_stringArray[i] = String.Format("one{0}", i);
  }

  boolArrayLen = 0;
  byteArrayLen = 0;
  shortArrayLen = 0;
  intArrayLen = 0;
  longArrayLen = 0;
  floatArrayLen = 0;
  strLenArray = 0;
}

PdxVersionedV1::PdxVersionedV1(int32_t size) {
  init(size);
  LOG_DEBUG("PdxVersioned 1");
}

PdxVersionedV1::~PdxVersionedV1() noexcept {
  // TODO Auto-generated destructor stub
}

void PdxVersionedV1::toData(PdxWriter& pw) const {
  // pw.writeChar("m_char", m_char);
  pw.writeBoolean("m_bool", m_bool);
  pw.writeByte("m_byte", m_byte);
  pw.writeShort("m_int16", m_int16);
  pw.writeInt("m_int32", m_int32);
  pw.writeLong("m_long", m_long);
  pw.writeFloat("m_float", m_float);
  pw.writeDouble("m_double", m_double);
  pw.writeString("m_string", m_string);
  pw.writeBooleanArray("m_boolArray", m_boolArray);
  // pw.writeCharArray("m_charArray", m_charArray, 2);
  pw.writeDate("m_dateTime", m_dateTime);
  pw.writeShortArray("m_int16Array", m_int16Array);
  pw.writeIntArray("m_int32Array", m_int32Array);
  pw.writeLongArray("m_longArray", m_longArray);
  pw.writeFloatArray("m_floatArray", m_floatArray);
  pw.writeDoubleArray("m_doubleArray", m_doubleArray);
}

void PdxVersionedV1::fromData(PdxReader& pr) {
  // m_char = pr.readChar("m_char");
  m_bool = pr.readBoolean("m_bool");
  m_byte = pr.readByte("m_byte");
  m_int16 = pr.readShort("m_int16");
  m_int32 = pr.readInt("m_int32");
  m_long = pr.readLong("m_long");
  m_float = pr.readFloat("m_float");
  m_double = pr.readDouble("m_double");
  m_string = pr.readString("m_string");
  m_boolArray = pr.readBooleanArray("m_boolArray");
  // m_charArray = pr.readCharArray("m_charArray");
  m_dateTime = pr.readDate("m_dateTime");
  m_int16Array = pr.readShortArray("m_int16Array");
  m_int32Array = pr.readIntArray("m_int32Array");
  m_longArray = pr.readLongArray("m_longArray");
  m_floatArray = pr.readFloatArray("m_floatArray");
  m_doubleArray = pr.readDoubleArray("m_doubleArray");
}
std::string PdxVersionedV1::toString() const {
  char idbuf[4096];
  // sprintf(idbuf,"PdxTypesV1R1:[ m_i1=%d ] [ m_i2=%d ] [ m_i3=%d ] [ m_i4=%d
  // ]", m_i1, m_i2, m_i3, m_i4 );
  return idbuf;
}

/************************************************************
 *  TestKey
 * *********************************************************/

TestKeyV1::TestKeyV1() {}

TestKeyV1::TestKeyV1(char* id) { _id = id; }

/************************************************************
 *  TestDiffTypePdxSV1
 * *********************************************************/

TestDiffTypePdxSV1::TestDiffTypePdxSV1() {}

TestDiffTypePdxSV1::TestDiffTypePdxSV1(bool init) {
  if (init) {
    _id = "id:100";
    _name = "HK";
  }
}

bool TestDiffTypePdxSV1::equals(const TestDiffTypePdxSV1& obj) {
  if (obj._id == _id && obj._name == _name) {
    return true;
  }

  return false;
}

/************************************************************
 *  TestEqualsV1
 * *********************************************************/

TestEqualsV1::TestEqualsV1() {
  i1 = 1;
  i2 = 0;
  s1 = "s1";
  // TODO: Uncomment it.
  // sArr = ;
  // intArr = ;
}

void TestEqualsV1::toData(PdxWriter& pw) const {
  pw.writeInt("i1", i1);
  pw.writeInt("i2", i2);
  pw.writeString("s1", s1);
  // pw.writeStringArray("sArr", sArr, 2);
  // pw.writeIntArray("intArr", intArr, 2);
  // pw.writeObject("intArrObject", intArr);
  // pw.writeObject("sArrObject", sArr);
}

void TestEqualsV1::fromData(PdxReader& pr) {
  i1 = pr.readInt("i1");
  i2 = pr.readInt("i2");
  s1 = pr.readString("s1");
  // sArr = pr.readStringArray("sArr");
  // intArr = pr.readIntArray("intArr");
  // intArr = (int[]) reader.ReadObject("intArrObject");
  // sArr = (string[]) reader.ReadObject("sArrObject");
}
std::string TestEqualsV1::toString() const {
  char idbuf[1024];
  sprintf(idbuf, "TestEqualsV1:[i1=%d ] [i2=%d] ", i1, i2);
  return idbuf;
}

}  // namespace PdxTests
