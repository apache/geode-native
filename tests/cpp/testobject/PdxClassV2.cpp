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

#include "PdxClassV2.hpp"

namespace PdxTests {

using apache::geode::client::Exception;

/************************************************************
 *  PdxType1V2
 * *********************************************************/
int32_t PdxTypes1V2::m_diffInSameFields = 0;
int32_t PdxTypes1V2::m_diffInExtraFields = 0;
bool PdxTypes1V2::m_useWeakHashMap = false;

PdxTypes1V2::PdxTypes1V2() {
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
  m_i5 = 0;
  m_i6 = 0;
}

PdxTypes1V2::~PdxTypes1V2() noexcept {
  // TODO Auto-generated destructor stub
}

void PdxTypes1V2::reset(bool useWeakHashMap) {
  PdxTypes1V2::m_diffInSameFields = 0;
  PdxTypes1V2::m_diffInExtraFields = 0;
  PdxTypes1V2::m_useWeakHashMap = useWeakHashMap;
}

int PdxTypes1V2::getHashCode() {
  // TODO:Implement It.
  return 1;
}

bool PdxTypes1V2::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypes1V2>(obj);
  if (pap == nullptr) return false;

  if (pap.get() == this) return true;

  if (m_i1 + m_diffInSameFields <= pap->m_i1 &&
      m_i2 + m_diffInSameFields <= pap->m_i2 &&
      m_i3 + m_diffInSameFields <= pap->m_i3 &&
      m_i4 + m_diffInSameFields <= pap->m_i4 &&
      m_i5 + m_diffInExtraFields == pap->m_i5 &&
      m_i6 + m_diffInExtraFields == pap->m_i6) {
    return true;
  }

  return false;
}

void PdxTypes1V2::toData(PdxWriter& pw) const {
  //
  if (!m_useWeakHashMap) pw.writeUnreadFields(m_pdxUreadFields);

  pw.writeInt("i1", m_i1 + 1);
  pw.markIdentityField("i1");
  pw.writeInt("i2", m_i2 + 1);
  pw.writeInt("i3", m_i3 + 1);
  pw.writeInt("i4", m_i4 + 1);
  pw.writeInt("i5", m_i5 + 1);
  pw.writeInt("i6", m_i6 + 1);

  m_diffInSameFields++;
  m_diffInExtraFields++;
  // LOG_DEBUG("PdxObject::toData() Done......");
}

void PdxTypes1V2::fromData(PdxReader& pr) {
  // LOG_DEBUG("PdxObject::fromData() start...");
  m_i1 = pr.readInt("i1");
  bool isIdentity = pr.isIdentityField("i2");
  if (isIdentity) throw Exception("i2 is not identity field");
  //
  if (!m_useWeakHashMap) m_pdxUreadFields = pr.readUnreadFields();

  m_i2 = pr.readInt("i2");
  m_i3 = pr.readInt("i3");
  m_i4 = pr.readInt("i4");
  m_i5 = pr.readInt("i5");
  m_i6 = pr.readInt("i6");

  // LOG_DEBUG("PdxType1V2::fromData() End...");
}
std::string PdxTypes1V2::toString() const {
  char idbuf[4096];
  sprintf(idbuf,
          "PdxType1V1:[ m_i1=%d ] [ m_i2=%d ] [ m_i3=%d ] [ m_i4=%d ] [ "
          "m_i5=%d ] [ m_i6=%d ] [ m_diffInExtraFields=%d ]",
          m_i1, m_i2, m_i3, m_i4, m_i5, m_i6, m_diffInExtraFields);
  return idbuf;
}

/************************************************************
 *  PdxTypes2V2
 * *********************************************************/

int PdxTypes2V2::m_diffInSameFields = 0;
int PdxTypes2V2::m_diffInExtraFields = 0;
bool PdxTypes2V2::m_useWeakHashMap = false;

PdxTypes2V2::PdxTypes2V2() {
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
  m_i5 = 0;
  m_i6 = 0;
}

PdxTypes2V2::~PdxTypes2V2() noexcept {
  // TODO Auto-generated destructor stub
}

void PdxTypes2V2::reset(bool useWeakHashMap) {
  PdxTypes2V2::m_diffInSameFields = 0;
  PdxTypes2V2::m_diffInExtraFields = 0;
  PdxTypes2V2::m_useWeakHashMap = useWeakHashMap;
}

int PdxTypes2V2::getHashCode() {
  // TODO:Implement It.
  return 1;
}

bool PdxTypes2V2::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypes2V2>(obj);
  if (pap == nullptr) return false;

  if (pap.get() == this) return true;

  if (m_i1 + m_diffInSameFields <= pap->m_i1 &&
      m_i2 + m_diffInSameFields <= pap->m_i2 &&
      m_i3 + m_diffInSameFields <= pap->m_i3 &&
      m_i4 + m_diffInSameFields <= pap->m_i4 &&
      m_i5 + m_diffInExtraFields == pap->m_i5 &&
      m_i6 + m_diffInExtraFields == pap->m_i6) {
    return true;
  }

  return false;
}

void PdxTypes2V2::toData(PdxWriter& pw) const {
  if (!m_useWeakHashMap) pw.writeUnreadFields(m_unreadFields);

  pw.writeInt("i1", m_i1 + 1);
  pw.writeInt("i2", m_i2 + 1);
  pw.writeInt("i5", m_i5 + 1);
  pw.writeInt("i6", m_i6 + 1);
  pw.writeInt("i3", m_i3 + 1);
  pw.writeInt("i4", m_i4 + 1);

  m_diffInExtraFields++;
  m_diffInSameFields++;
}

void PdxTypes2V2::fromData(PdxReader& pr) {
  // LOG_DEBUG("PdxObject::fromData() start...");
  //
  if (!m_useWeakHashMap) m_unreadFields = pr.readUnreadFields();

  m_i1 = pr.readInt("i1");
  m_i2 = pr.readInt("i2");
  m_i5 = pr.readInt("i5");
  m_i6 = pr.readInt("i6");
  m_i3 = pr.readInt("i3");
  m_i4 = pr.readInt("i4");
}
std::string PdxTypes2V2::toString() const {
  char idbuf[4096];
  sprintf(idbuf,
          "PdxTypes2V2:[ m_i1=%d ] [ m_i2=%d ] [ m_i3=%d ] [ m_i4=%d ] [ "
          "m_i5=%d ] [ m_i6=%d ]",
          m_i1, m_i2, m_i3, m_i4, m_i5, m_i6);
  return idbuf;
}

/************************************************************
 *  PdxTypes3V2
 * *********************************************************/

int PdxTypes3V2::m_diffInSameFields = 0;
int PdxTypes3V2::m_diffInExtraFields = 0;
bool PdxTypes3V2::m_useWeakHashMap = false;

PdxTypes3V2::PdxTypes3V2() {
  m_i1 = 1;
  m_i2 = 21;
  m_str1 = "common";
  m_i4 = 41;
  m_i3 = 31;
  m_i6 = 0;
  m_str3 = "0";
}

PdxTypes3V2::~PdxTypes3V2() noexcept {
  // TODO Auto-generated destructor stub
}

void PdxTypes3V2::reset(bool useWeakHashMap) {
  PdxTypes3V2::m_diffInSameFields = 0;
  PdxTypes3V2::m_diffInExtraFields = 0;
  PdxTypes3V2::m_useWeakHashMap = useWeakHashMap;
}

int PdxTypes3V2::getHashCode() {
  // TODO:Implement It.
  return 1;
}

bool PdxTypes3V2::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypes3V2>(obj);
  if (pap == nullptr) return false;

  if (pap.get() == this) return true;

  if (m_i1 + m_diffInSameFields <= pap->m_i1 &&
      m_i2 + m_diffInSameFields <= pap->m_i2 &&
      m_i3 + m_diffInSameFields <= pap->m_i3 &&
      m_i4 + m_diffInSameFields <= pap->m_i4 &&
      m_i6 + m_diffInExtraFields == pap->m_i6) {
    if (m_str1 == pap->m_str1 && m_str3 <= pap->m_str3) {
      return true;
    }
  }
  return false;
}

void PdxTypes3V2::toData(PdxWriter& pw) const {
  //
  if (!m_useWeakHashMap) pw.writeUnreadFields(m_unreadFields);

  pw.writeInt("i1", m_i1 + 1);
  pw.writeInt("i2", m_i2 + 1);
  pw.writeString("m_str1", m_str1);
  pw.writeInt("i4", m_i4 + 1);
  pw.writeInt("i3", m_i3 + 1);
  pw.writeInt("i6", m_i6 + 1);
  pw.writeString("m_str3", m_str3);

  m_diffInExtraFields++;
  m_diffInSameFields++;
}

void PdxTypes3V2::fromData(PdxReader& pr) {
  // LOG_DEBUG("PdxObject::fromData() start...");
  //
  if (!m_useWeakHashMap) m_unreadFields = pr.readUnreadFields();

  m_i1 = pr.readInt("i1");
  m_i2 = pr.readInt("i2");
  m_str1 = pr.readString("m_str1");
  m_i4 = pr.readInt("i4");
  m_i3 = pr.readInt("i3");
  m_i6 = pr.readInt("i6");
  auto tmp = pr.readString("m_str3");

  if (tmp.empty()) {
    m_str3 = std::to_string(m_diffInExtraFields);
  } else {
    m_str3 = std::to_string(m_diffInExtraFields) + m_str3;
  }
}

std::string PdxTypes3V2::toString() const {
  return "PdxTypes3V2:[ m_i1=" + std::to_string(m_i1) +
         " ] [ m_i2=" + std::to_string(m_i2) + " ] [m_str1=" + m_str1 +
         "] [ m_i3=" + std::to_string(m_i3) +
         " ] [  m_i4=" + std::to_string(m_i4) + " ] [m_str3=" + m_str3 + "]";
}

/************************************************************
 *  PdxTypesR1V2
 * *********************************************************/

int32_t PdxTypesR1V2::m_diffInSameFields = 0;
int32_t PdxTypesR1V2::m_diffInExtraFields = 0;
bool PdxTypesR1V2::m_useWeakHashMap = false;

PdxTypesR1V2::PdxTypesR1V2() {
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;

  m_i5 = 721398;
  m_i6 = 45987;
}

PdxTypesR1V2::~PdxTypesR1V2() noexcept {
  // TODO Auto-generated destructor stub
}

void PdxTypesR1V2::reset(bool useWeakHashMap) {
  PdxTypesR1V2::m_diffInSameFields = 0;
  PdxTypesR1V2::m_diffInExtraFields = 0;
  PdxTypesR1V2::m_useWeakHashMap = useWeakHashMap;
}

int PdxTypesR1V2::getHashCode() {
  // TODO:Implement It.
  return 1;
}

bool PdxTypesR1V2::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypesR1V2>(obj);
  if (pap == nullptr) return false;

  if (pap.get() == this) return true;

  if (m_i1 + m_diffInSameFields <= pap->m_i1 &&
      m_i2 + m_diffInSameFields <= pap->m_i2 &&
      m_i3 + m_diffInSameFields <= pap->m_i3 &&
      m_i4 + m_diffInSameFields <= pap->m_i4) {
    if (((m_i5 + PdxTypesR1V2::m_diffInExtraFields == pap->m_i5) &&
         (m_i6 + PdxTypesR1V2::m_diffInExtraFields == pap->m_i6)) ||
        ((PdxTypesR1V2::m_diffInExtraFields == pap->m_i5) &&
         (PdxTypesR1V2::m_diffInExtraFields == pap->m_i6))) {
      return true;
    }
  }
  return false;
}

void PdxTypesR1V2::toData(PdxWriter& pw) const {
  if (!m_useWeakHashMap) pw.writeUnreadFields(m_pdxUnreadFields);

  pw.writeInt("i1", m_i1 + 1);
  pw.writeInt("i2", m_i2 + 1);
  pw.writeInt("i3", m_i3 + 1);
  pw.writeInt("i4", m_i4 + 1);

  pw.writeInt("i5", m_i5 + 1);
  pw.writeInt("i6", m_i6 + 1);

  m_diffInSameFields++;
  m_diffInExtraFields++;
}

void PdxTypesR1V2::fromData(PdxReader& pr) {
  if (!m_useWeakHashMap) m_pdxUnreadFields = pr.readUnreadFields();

  m_i1 = pr.readInt("i1");
  m_i2 = pr.readInt("i2");
  m_i3 = pr.readInt("i3");
  m_i4 = pr.readInt("i4");

  m_i5 = pr.readInt("i5");
  m_i6 = pr.readInt("i6");
}
std::string PdxTypesR1V2::toString() const {
  char idbuf[4096];
  sprintf(idbuf,
          "PdxTypesR1V2:[ m_i1=%d ] [ m_i2=%d ] [ m_i3=%d ] [ m_i4=%d ] [ "
          "m_i5=%d ] [ m_i6=%d ]",
          m_i1, m_i2, m_i3, m_i4, m_i5, m_i6);
  return idbuf;
}

/************************************************************
 *  PdxTypesR2V2
 * *********************************************************/

int PdxTypesR2V2::m_diffInSameFields = 0;
int PdxTypesR2V2::m_diffInExtraFields = 0;
bool PdxTypesR2V2::m_useWeakHashMap = false;

PdxTypesR2V2::PdxTypesR2V2() {
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
  m_i5 = 798243;
  m_i6 = 9900;

  m_str1 = "0";
}

PdxTypesR2V2::~PdxTypesR2V2() noexcept {
  // TODO Auto-generated destructor stub
}

void PdxTypesR2V2::reset(bool useWeakHashMap) {
  PdxTypesR2V2::m_diffInSameFields = 0;
  PdxTypesR2V2::m_diffInExtraFields = 0;
  PdxTypesR2V2::m_useWeakHashMap = useWeakHashMap;
}

int PdxTypesR2V2::getHashCode() {
  // TODO:Implement It.
  return 1;
}

bool PdxTypesR2V2::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypesR2V2>(obj);
  if (pap == nullptr) return false;

  if (pap.get() == this) return true;

  if (m_i1 + m_diffInSameFields <= pap->m_i1 &&
      m_i2 + m_diffInSameFields <= pap->m_i2 &&
      m_i3 + m_diffInSameFields <= pap->m_i3 &&
      m_i4 + m_diffInSameFields <= pap->m_i4) {
    if (pap->m_str1 == std::to_string(m_diffInExtraFields)) {
      if (m_i5 + m_diffInExtraFields == pap->m_i5 &&
          m_i6 + m_diffInExtraFields == pap->m_i6) {
        return true;
      }
    }
  }
  return false;
}

void PdxTypesR2V2::toData(PdxWriter& pw) const {
  if (!m_useWeakHashMap) pw.writeUnreadFields(m_pdxunreadFields);

  pw.writeInt("i1", m_i1 + 1);
  pw.writeInt("i2", m_i2 + 1);
  pw.writeInt("i5", m_i5 + 1);
  pw.writeInt("i6", m_i6 + 1);
  pw.writeInt("i3", m_i3 + 1);
  pw.writeInt("i4", m_i4 + 1);

  m_diffInExtraFields++;
  m_diffInSameFields++;

  char tmpBuf[512] = {0};
  sprintf(tmpBuf, "%d", m_diffInExtraFields);
  pw.writeString("m_str1", tmpBuf);
}

void PdxTypesR2V2::fromData(PdxReader& pr) {
  LOG_DEBUG("PdxObject::fromData() start...");

  if (!m_useWeakHashMap) m_pdxunreadFields = pr.readUnreadFields();

  m_i1 = pr.readInt("i1");
  m_i2 = pr.readInt("i2");
  m_i5 = pr.readInt("i5");
  m_i6 = pr.readInt("i6");
  m_i3 = pr.readInt("i3");
  m_i4 = pr.readInt("i4");
  m_str1 = pr.readString("m_str1");
}
std::string PdxTypesR2V2::toString() const {
  return "PdxTypesR2V2:[ m_i1=" + std::to_string(m_i1) +
         " ] [ m_i2=" + std::to_string(m_i2) +
         " ] [ m_i3=" + std::to_string(m_i3) +
         " ] [ m_i4=" + std::to_string(m_i4) +
         " ] [ m_i4=" + std::to_string(m_i5) +
         " ] [ m_i4=" + std::to_string(m_i6) + " ] [ m_str1=" + m_str1 + " ]";
}

/************************************************************
 *  PdxTypesIgnoreUnreadFieldsV2
 * *********************************************************/

int PdxTypesIgnoreUnreadFieldsV2::m_diffInSameFields = 0;
int PdxTypesIgnoreUnreadFieldsV2::m_diffInExtraFields = 0;
bool PdxTypesIgnoreUnreadFieldsV2::m_useWeakHashMap = false;

PdxTypesIgnoreUnreadFieldsV2::PdxTypesIgnoreUnreadFieldsV2() {
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
  m_i5 = 0;
  m_i6 = 0;
}

PdxTypesIgnoreUnreadFieldsV2::~PdxTypesIgnoreUnreadFieldsV2() noexcept {
  // TODO Auto-generated destructor stub
}

void PdxTypesIgnoreUnreadFieldsV2::reset(bool useWeakHashMap) {
  PdxTypesIgnoreUnreadFieldsV2::m_diffInSameFields = 0;
  PdxTypesIgnoreUnreadFieldsV2::m_diffInExtraFields = 0;
  PdxTypesIgnoreUnreadFieldsV2::m_useWeakHashMap = useWeakHashMap;
}

int PdxTypesIgnoreUnreadFieldsV2::getHashCode() {
  // TODO:Implement It.
  return 1;
}

bool PdxTypesIgnoreUnreadFieldsV2::equals(
    std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypesIgnoreUnreadFieldsV2>(obj);
  if (pap == nullptr) return false;

  if (pap.get() == this) return true;

  LOG_INFO("PdxTypesIgnoreUnreadFieldsV2::equals");
  LOG_INFO("m_i1 =%d m_diffInSameFields=%d pap->m_i1=%d", m_i1,
           m_diffInSameFields, pap->m_i1);
  LOG_INFO("m_i2 =%d m_diffInSameFields=%d pap->m_i2=%d", m_i2,
           m_diffInSameFields, pap->m_i2);
  LOG_INFO("m_i3 =%d m_diffInSameFields=%d pap->m_i3=%d", m_i3,
           m_diffInSameFields, pap->m_i3);
  LOG_INFO("m_i4 =%d m_diffInSameFields=%d pap->m_i4=%d", m_i4,
           m_diffInSameFields, pap->m_i4);
  LOG_INFO("m_i5 =%d  pap->m_i5=%d", m_i5, pap->m_i5);
  LOG_INFO("m_i6 =%d  pap->m_i6=%d", m_i6, pap->m_i6);

  if (m_i1 + m_diffInSameFields <= pap->m_i1 &&
      m_i2 + m_diffInSameFields <= pap->m_i2 &&
      m_i3 + m_diffInSameFields <= pap->m_i3 &&
      m_i4 + m_diffInSameFields <= pap->m_i4 && m_i5 == pap->m_i5 &&
      m_i6 == pap->m_i6) {
    return true;
  }

  return false;
}

void PdxTypesIgnoreUnreadFieldsV2::updateMembers() {
  m_i5 = static_cast<int32_t>(m_diffInExtraFields);
  m_i6 = static_cast<int32_t>(m_diffInExtraFields);
}

void PdxTypesIgnoreUnreadFieldsV2::toData(PdxWriter& pw) const {
  if (!m_useWeakHashMap) pw.writeUnreadFields(m_unreadFields);

  pw.writeInt("i1", m_i1 + 1);
  pw.markIdentityField("i1");
  pw.writeInt("i2", m_i2 + 1);
  pw.writeInt("i3", m_i3 + 1);
  pw.writeInt("i4", m_i4 + 1);
  pw.writeInt("i5", m_diffInExtraFields);
  pw.writeInt("i6", m_diffInExtraFields);

  m_i5 = static_cast<int32_t>(m_diffInExtraFields);
  m_i6 = static_cast<int32_t>(m_diffInExtraFields);

  m_diffInSameFields++;
  m_diffInExtraFields++;
}

void PdxTypesIgnoreUnreadFieldsV2::fromData(PdxReader& pr) {
  m_i1 = pr.readInt("i1");
  bool isIdentity = pr.isIdentityField("i2");

  if (isIdentity) {
    throw Exception("i2 is not identity field");
  }

  if (!m_useWeakHashMap) m_unreadFields = pr.readUnreadFields();

  m_i2 = pr.readInt("i2");
  m_i3 = pr.readInt("i3");
  m_i4 = pr.readInt("i4");
  m_i5 = pr.readInt("i5");
  m_i6 = pr.readInt("i6");
}
std::string PdxTypesIgnoreUnreadFieldsV2::toString() const {
  char idbuf[4096];
  sprintf(idbuf,
          "PdxTypesV1R1:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] [m_i5=%d] "
          "[m_i6=%d]",
          m_i1, m_i2, m_i3, m_i4, m_i5, m_i6);
  return idbuf;
}

/************************************************************
 *  PdxVersionedV2
 * *********************************************************/
PdxVersionedV2::PdxVersionedV2() {}

void PdxVersionedV2::init(int32_t size) {
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

PdxVersionedV2::PdxVersionedV2(int32_t size) {
  init(size);
  LOG_DEBUG("PdxVersioned 1");
}

PdxVersionedV2::~PdxVersionedV2() noexcept {
  // TODO Auto-generated destructor stub
}

void PdxVersionedV2::toData(PdxWriter& pw) const {
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

void PdxVersionedV2::fromData(PdxReader& pr) {
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
std::string PdxVersionedV2::toString() const {
  char idbuf[4096];
  // sprintf(idbuf,"PdxTypesV1R1:[ m_i1=%d ] [ m_i2=%d ] [ m_i3=%d ] [ m_i4=%d
  // ]", m_i1, m_i2, m_i3, m_i4 );
  return idbuf;
}

/************************************************************
 *  TestKeyV2
 * *********************************************************/

TestKeyV2::TestKeyV2() {}

TestKeyV2::TestKeyV2(char* id) { _id = id; }

/************************************************************
 *  TestDiffTypePdxSV2
 * *********************************************************/

TestDiffTypePdxSV2::TestDiffTypePdxSV2() {}

TestDiffTypePdxSV2::TestDiffTypePdxSV2(bool init) {
  if (init) {
    _id = "id:100";
    _name = "HK";
    _count = 100;
  }
}

bool TestDiffTypePdxSV2::equals(const TestDiffTypePdxSV2& other) {
  LOG_INFO("TestDiffTypePdxSV2 other->_coun = %d and _count = %d ",
           other._count, _count);
  LOG_INFO("TestDiffTypePdxSV2 other->_id = %s and _id = %s ",
           other._id.c_str(), _id.c_str());
  LOG_INFO("TestDiffTypePdxSV2 other->_name = %s and _name = %s ",
           other._name.c_str(), _name.c_str());

  if (other._count == _count && other._id == _id && other._name == _name) {
    return true;
  }

  return false;
}

}  // namespace PdxTests
