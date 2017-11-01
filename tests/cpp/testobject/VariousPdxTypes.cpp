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
 * VariousPdxTypes.cpp
 *
 *  Created on: Feb 10, 2012
 *      Author: npatel
 */

#include "VariousPdxTypes.hpp"
#include <geode/CacheableEnum.hpp>
namespace PdxTests {

/************************************************************
 *  PdxTypes1
 * *********************************************************/

PdxTypes1::PdxTypes1() {
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxTypes1::~PdxTypes1() {
  // TODO Auto-generated destructor stub
}

int32_t PdxTypes1::getHashCode() { return 1; }

bool PdxTypes1::equals(std::shared_ptr<PdxSerializable> obj) {
  // LOGDEBUG("NIL:PdxTypes1::==::33");
  if (obj == nullptr) {
    // LOGDEBUG("NIL:PdxTypes1::==::35");
    return false;
  }
  auto pap = std::dynamic_pointer_cast<PdxTypes1>(obj);
  if (pap == nullptr) {
    // LOGDEBUG("NIL:PdxTypes1::==::40");
    return false;
  }
  if (pap.get() == this) {
    // LOGDEBUG("NIL:PdxTypes1::==::44");
    return true;
  }
  LOGINFO("PdxTypes1:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d]", m_i1, m_i2, m_i3,
          m_i4);
  if (m_i1 == pap->m_i1 && m_i2 == pap->m_i2 && m_i3 == pap->m_i3 &&
      m_i4 == pap->m_i4) {
    // LOGDEBUG("NIL:PdxTypes1::==::48");
    return true;
  }
  LOGDEBUG("NIL:PdxTypes1::==::51");
  return false;
}

std::shared_ptr<CacheableString> PdxTypes1::toString() const {
  char idbuf[4096];
  sprintf(idbuf, "PdxTypes1:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d]", m_i1,
          m_i2, m_i3, m_i4);
  return CacheableString::create(idbuf);
}

void PdxTypes1::toData(std::shared_ptr<PdxWriter> pw) {
  pw->writeInt("i1", m_i1);
  pw->writeInt("i2", m_i2);
  pw->writeInt("i3", m_i3);
  pw->writeInt("i4", m_i4);
}

void PdxTypes1::fromData(std::shared_ptr<PdxReader> pr) {
  m_i1 = pr->readInt("i1");
  m_i2 = pr->readInt("i2");
  m_i3 = pr->readInt("i3");
  m_i4 = pr->readInt("i4");
}

/************************************************************
 *  PdxTypes2
 * *********************************************************/
PdxTypes2::PdxTypes2() {
  m_s1 = (char *)"one";
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxTypes2::~PdxTypes2() {
  // TODO Auto-generated destructor stub
}

int32_t PdxTypes2::getHashCode() { return 1; }

bool PdxTypes2::equals(std::shared_ptr<PdxSerializable> obj) {
  // LOGDEBUG("NIL:96:this::PdxType2 = %s", this->toString());

  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypes2>(obj);
  // LOGDEBUG("NIl:102:pap::PdxType2 = %s", pap->toString());
  if (pap == nullptr) return false;

  // if (pap.get() == this)
  //	return true;

  if (m_i1 == pap->m_i1 && m_i2 == pap->m_i2 && m_i3 == pap->m_i3 &&
      m_i4 == pap->m_i4 && (strcmp(m_s1, pap->m_s1) == 0)) {
    return true;
  }

  return false;
}

std::shared_ptr<CacheableString> PdxTypes2::toString() const {
  char idbuf[4096];
  sprintf(idbuf, "PdxTypes2:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] [m_s1=%s]",
          m_i1, m_i2, m_i3, m_i4, m_s1);
  return CacheableString::create(idbuf);
}

void PdxTypes2::toData(std::shared_ptr<PdxWriter> pw) {
  pw->writeString("s1", m_s1);
  pw->writeInt("i1", m_i1);
  pw->writeInt("i2", m_i2);
  pw->writeInt("i3", m_i3);
  pw->writeInt("i4", m_i4);
}

void PdxTypes2::fromData(std::shared_ptr<PdxReader> pr) {
  m_s1 = pr->readString("s1");
  m_i1 = pr->readInt("i1");
  m_i2 = pr->readInt("i2");
  m_i3 = pr->readInt("i3");
  m_i4 = pr->readInt("i4");
}

/************************************************************
 *  PdxTypes3
 * *********************************************************/
PdxTypes3::PdxTypes3() {
  m_s1 = (char *)"one";
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxTypes3::~PdxTypes3() {
  // TODO Auto-generated destructor stub
}

int32_t PdxTypes3::getHashCode() { return 1; }

bool PdxTypes3::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypes3>(obj);
  if (pap == nullptr) return false;

  // if (pap.get() == this)
  //	return true;

  if (m_i1 == pap->m_i1 && m_i2 == pap->m_i2 && m_i3 == pap->m_i3 &&
      m_i4 == pap->m_i4 && (strcmp(m_s1, pap->m_s1) == 0)) {
    return true;
  }

  return false;
}

std::shared_ptr<CacheableString> PdxTypes3::toString() const {
  char idbuf[4096];
  sprintf(idbuf, "PdxTypes3:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] [m_s1=%s]",
          m_i1, m_i2, m_i3, m_i4, m_s1);
  return CacheableString::create(idbuf);
}

void PdxTypes3::toData(std::shared_ptr<PdxWriter> pw) {
  pw->writeInt("i1", m_i1);
  pw->writeInt("i2", m_i2);
  pw->writeInt("i3", m_i3);
  pw->writeInt("i4", m_i4);
  pw->writeString("s1", m_s1);
}

void PdxTypes3::fromData(std::shared_ptr<PdxReader> pr) {
  m_i1 = pr->readInt("i1");
  m_i2 = pr->readInt("i2");
  m_i3 = pr->readInt("i3");
  m_i4 = pr->readInt("i4");
  m_s1 = pr->readString("s1");
}

/************************************************************
 *  PdxTypes4
 * *********************************************************/
PdxTypes4::PdxTypes4() {
  m_s1 = (char *)"one";
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxTypes4::~PdxTypes4() {
  // TODO Auto-generated destructor stub
}

int32_t PdxTypes4::getHashCode() { return 1; }

bool PdxTypes4::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypes4>(obj);
  if (pap == nullptr) return false;

  // if (pap.get() == this)
  //	return true;

  if (m_i1 == pap->m_i1 && m_i2 == pap->m_i2 && m_i3 == pap->m_i3 &&
      m_i4 == pap->m_i4 && (strcmp(m_s1, pap->m_s1) == 0)) {
    return true;
  }

  return false;
}

std::shared_ptr<CacheableString> PdxTypes4::toString() const {
  char idbuf[4096];
  sprintf(idbuf, "PdxTypes4:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] [m_s1=%s]",
          m_i1, m_i2, m_i3, m_i4, m_s1);
  return CacheableString::create(idbuf);
}

void PdxTypes4::toData(std::shared_ptr<PdxWriter> pw) {
  pw->writeInt("i1", m_i1);
  pw->writeInt("i2", m_i2);
  pw->writeString("s1", m_s1);
  pw->writeInt("i3", m_i3);
  pw->writeInt("i4", m_i4);
}

void PdxTypes4::fromData(std::shared_ptr<PdxReader> pr) {
  m_i1 = pr->readInt("i1");
  m_i2 = pr->readInt("i2");
  m_s1 = pr->readString("s1");
  m_i3 = pr->readInt("i3");
  m_i4 = pr->readInt("i4");
}

/************************************************************
 *  PdxTypes5
 * *********************************************************/
PdxTypes5::PdxTypes5() {
  m_s1 = (char *)"one";
  m_s2 = (char *)"two";
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxTypes5::~PdxTypes5() {
  // TODO Auto-generated destructor stub
}

int32_t PdxTypes5::getHashCode() { return 1; }

bool PdxTypes5::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypes5>(obj);
  if (pap == nullptr) return false;

  // if (pap.get() == this)
  //	return true;

  if (m_i1 == pap->m_i1 && m_i2 == pap->m_i2 && m_i3 == pap->m_i3 &&
      m_i4 == pap->m_i4 && (strcmp(m_s1, pap->m_s1) == 0) &&
      (strcmp(m_s2, pap->m_s2) == 0)) {
    return true;
  }

  return false;
}

std::shared_ptr<CacheableString> PdxTypes5::toString() const {
  char idbuf[4096];
  sprintf(
      idbuf,
      "PdxTypes4:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] [m_s1=%s] [m_s2=%s]",
      m_i1, m_i2, m_i3, m_i4, m_s1, m_s2);
  return CacheableString::create(idbuf);
}

void PdxTypes5::toData(std::shared_ptr<PdxWriter> pw) {
  pw->writeString("s1", m_s1);
  pw->writeString("s2", m_s2);
  pw->writeInt("i1", m_i1);
  pw->writeInt("i2", m_i2);
  pw->writeInt("i3", m_i3);
  pw->writeInt("i4", m_i4);
}

void PdxTypes5::fromData(std::shared_ptr<PdxReader> pr) {
  m_s1 = pr->readString("s1");
  m_s2 = pr->readString("s2");
  m_i1 = pr->readInt("i1");
  m_i2 = pr->readInt("i2");
  m_i3 = pr->readInt("i3");
  m_i4 = pr->readInt("i4");
}

/************************************************************
 *  PdxTypes6
 * *********************************************************/
PdxTypes6::PdxTypes6() {
  m_s1 = (char *)"one";
  m_s2 = (char *)"two";
  bytes128 = new int8_t[2];
  bytes128[0] = 0x34;
  ;
  bytes128[1] = 0x64;
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxTypes6::~PdxTypes6() {
  // TODO Auto-generated destructor stub
}

int32_t PdxTypes6::getHashCode() { return 1; }

bool PdxTypes6::equals(std::shared_ptr<PdxSerializable> obj) {
  LOGDEBUG("PdxTypes6::equals -1");
  if (obj == nullptr) return false;

  LOGDEBUG("PdxTypes6::equals -2");
  auto pap = std::dynamic_pointer_cast<PdxTypes6>(obj);
  if (pap == nullptr) return false;

  LOGDEBUG("PdxTypes6::equals -3 m_i1 = %d", m_i1);
  LOGDEBUG("PdxTypes6::equals -4 m_i2 = %d", m_i2);
  LOGDEBUG("PdxTypes6::equals -5 m_i3 = %d", m_i3);
  LOGDEBUG("PdxTypes6::equals -6 m_i4 = %d", m_i4);
  LOGDEBUG("PdxTypes6::equals -7 m_s1 = %s", m_s1);
  LOGDEBUG("PdxTypes6::equals -8 m_s2 = %s", m_s2);

  LOGDEBUG("PdxTypes6::equals -9 pap->m_i1 = %d", pap->m_i1);
  LOGDEBUG("PdxTypes6::equals -10 pap->m_i2 = %d", pap->m_i2);
  LOGDEBUG("PdxTypes6::equals -11 pap->m_i3 = %d", pap->m_i3);
  LOGDEBUG("PdxTypes6::equals -12 pap->m_i4 = %d", pap->m_i4);
  LOGDEBUG("PdxTypes6::equals -13 pap->m_s1 = %s", pap->m_s1);
  LOGDEBUG("PdxTypes6::equals -14 pap->m_s2 = %s", pap->m_s2);
  // if (pap.get() == this)
  //	return true;

  if (m_i1 == pap->m_i1 && m_i2 == pap->m_i2 && m_i3 == pap->m_i3 &&
      m_i4 == pap->m_i4 && (strcmp(m_s1, pap->m_s1) == 0) &&
      (strcmp(m_s2, pap->m_s2) == 0)) {
    // Check byte[] length.
    // if(bytes128.Length == pap.bytes128.Length)
    return true;
  }

  return false;
}

std::shared_ptr<CacheableString> PdxTypes6::toString() const {
  char idbuf[4096];
  sprintf(
      idbuf,
      "PdxTypes4:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] [m_s1=%s] [m_s2=%s]",
      m_i1, m_i2, m_i3, m_i4, m_s1, m_s2);
  return CacheableString::create(idbuf);
}

void PdxTypes6::toData(std::shared_ptr<PdxWriter> pw) {
  pw->writeString("s1", m_s1);
  pw->writeInt("i1", m_i1);
  pw->writeByteArray("bytes128", bytes128, 2);
  pw->writeInt("i2", m_i2);
  pw->writeInt("i3", m_i3);
  pw->writeInt("i4", m_i4);
  pw->writeString("s2", m_s2);
}

void PdxTypes6::fromData(std::shared_ptr<PdxReader> pr) {
  m_s1 = pr->readString("s1");
  // LOGDEBUG("PdxTypes6::fromData m_s1 = %s", m_s1);

  m_i1 = pr->readInt("i1");
  // LOGDEBUG("PdxTypes6::fromData m_i1 = %d", m_i1);
  int32_t byteArrLen = 0;
  bytes128 = pr->readByteArray("bytes128", byteArrLen);
  m_i2 = pr->readInt("i2");
  // LOGDEBUG("PdxTypes6::fromData m_i2 = %d", m_i2);

  m_i3 = pr->readInt("i3");
  // LOGDEBUG("PdxTypes6::fromData m_i3 = %d", m_i3);

  m_i4 = pr->readInt("i4");
  // LOGDEBUG("PdxTypes6::fromData m_i4 = %d", m_i4);

  m_s2 = pr->readString("s2");
  // LOGDEBUG("PdxTypes6::fromData m_s2 = %s", m_s2);
}

/************************************************************
 *  PdxTypes7
 * *********************************************************/
PdxTypes7::PdxTypes7() {
  m_s1 = (char *)"one";
  m_s2 = (char *)"two";
  m_i1 = 34324;
  bytes38000 = new int8_t[38000];
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxTypes7::~PdxTypes7() {
  // TODO Auto-generated destructor stub
}

int32_t PdxTypes7::getHashCode() { return 1; }

bool PdxTypes7::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypes7>(obj);
  if (pap == nullptr) return false;

  // if (pap.get() == this)
  //	return true;

  if (m_i1 == pap->m_i1 && m_i2 == pap->m_i2 && m_i3 == pap->m_i3 &&
      m_i4 == pap->m_i4 && (strcmp(m_s1, pap->m_s1) == 0) &&
      (strcmp(m_s2, pap->m_s2) == 0)) {
    // Check byte[] length.
    // if(bytes38000.Length == pap.bytes38000.Length)
    return true;
  }

  return false;
}

std::shared_ptr<CacheableString> PdxTypes7::toString() const {
  char idbuf[4096];
  sprintf(
      idbuf,
      "PdxTypes7:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] [m_s1=%s] [m_s2=%s]",
      m_i1, m_i2, m_i3, m_i4, m_s1, m_s2);
  return CacheableString::create(idbuf);
}

void PdxTypes7::toData(std::shared_ptr<PdxWriter> pw) {
  pw->writeInt("i1", m_i1);
  pw->writeInt("i2", m_i2);
  pw->writeString("s1", m_s1);
  pw->writeByteArray("bytes38000", bytes38000, 2);
  pw->writeInt("i3", m_i3);
  pw->writeInt("i4", m_i4);
  pw->writeString("s2", m_s2);
}

void PdxTypes7::fromData(std::shared_ptr<PdxReader> pr) {
  m_i1 = pr->readInt("i1");
  m_i2 = pr->readInt("i2");
  m_s1 = pr->readString("s1");
  int32_t byteArrLen = 0;
  bytes38000 = pr->readByteArray("bytes38000", byteArrLen);
  m_i3 = pr->readInt("i3");
  m_i4 = pr->readInt("i4");
  m_s2 = pr->readString("s2");
}

/************************************************************
 *  PdxTypes8
 * *********************************************************/
PdxTypes8::PdxTypes8() {
  enum pdxEnumTest { pdx1, pdx2, pdx3 };
  m_s1 = (char *)"one";
  m_s2 = (char *)"two";
  m_i1 = 34324;
  bytes300 = new int8_t[300];
  _enum = CacheableEnum::create("PdxTests.pdxEnumTest", "pdx2", pdx2);
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxTypes8::~PdxTypes8() { delete[] bytes300; }

int32_t PdxTypes8::getHashCode() { return 1; }

bool PdxTypes8::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypes8>(obj);
  if (pap == nullptr) return false;

  // if (pap.get() == this)
  //	return true;

  if (m_i1 == pap->m_i1 && m_i2 == pap->m_i2 && m_i3 == pap->m_i3 &&
      m_i4 == pap->m_i4 && (strcmp(m_s1, pap->m_s1) == 0) &&
      (strcmp(m_s2, pap->m_s2) == 0)) {
    // Check byte[] length.
    // if(bytes300.Length == pap.bytes300.Length)
    return true;
  }

  return false;
}

std::shared_ptr<CacheableString> PdxTypes8::toString() const {
  char idbuf[4096];
  sprintf(
      idbuf,
      "PdxTypes8:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] [m_s1=%s] [m_s2=%s]",
      m_i1, m_i2, m_i3, m_i4, m_s1, m_s2);
  return CacheableString::create(idbuf);
}

void PdxTypes8::toData(std::shared_ptr<PdxWriter> pw) {
  pw->writeInt("i1", m_i1);
  pw->writeInt("i2", m_i2);
  pw->writeString("s1", m_s1);
  pw->writeByteArray("bytes300", bytes300, 2);
  pw->writeObject("_enum", _enum);
  pw->writeString("s2", m_s2);
  pw->writeInt("i3", m_i3);
  pw->writeInt("i4", m_i4);
}

void PdxTypes8::fromData(std::shared_ptr<PdxReader> pr) {
  m_i1 = pr->readInt("i1");
  m_i2 = pr->readInt("i2");
  m_s1 = pr->readString("s1");
  int32_t byteArrLen = 0;
  bytes300 = pr->readByteArray("bytes300", byteArrLen);
  _enum = pr->readObject("_enum");
  m_s2 = pr->readString("s2");
  m_i3 = pr->readInt("i3");
  m_i4 = pr->readInt("i4");
}

/************************************************************
 *  PdxTypes9
 * *********************************************************/
PdxTypes9::PdxTypes9() {
  m_s1 = (char *)"one";
  m_s2 = (char *)"two";
  m_s3 = (char *)"three";
  m_bytes66000 = new int8_t[66000];
  m_s4 = (char *)"four";
  m_s5 = (char *)"five";
}

PdxTypes9::~PdxTypes9() { delete[] m_bytes66000; }

int32_t PdxTypes9::getHashCode() { return 1; }

bool PdxTypes9::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypes9>(obj);
  if (pap == nullptr) return false;

  // if (pap.get() == this)
  //	return true;

  if ((strcmp(m_s1, pap->m_s1) == 0) && (strcmp(m_s2, pap->m_s2) == 0) &&
      (strcmp(m_s3, pap->m_s3) == 0) && (strcmp(m_s4, pap->m_s4) == 0) &&
      (strcmp(m_s5, pap->m_s5) == 0)) {
    // Check byte[] length.
    // if(m_bytes66000.Length == pap.m_bytes66000.Length)
    return true;
  }

  return false;
}

std::shared_ptr<CacheableString> PdxTypes9::toString() const {
  char idbuf[4096];
  sprintf(idbuf, "PdxTypes9:[m_s1=%s] [m_s2=%s] [m_s3=%s] [m_s4=%s] [m_s5=%s] ",
          m_s1, m_s2, m_s3, m_s4, m_s5);
  return CacheableString::create(idbuf);
}

void PdxTypes9::toData(std::shared_ptr<PdxWriter> pw) {
  pw->writeString("s1", m_s1);
  pw->writeString("s2", m_s2);
  pw->writeByteArray("bytes66000", m_bytes66000, 2);
  pw->writeString("s3", m_s3);
  pw->writeString("s4", m_s4);
  pw->writeString("s5", m_s5);
}

void PdxTypes9::fromData(std::shared_ptr<PdxReader> pr) {
  m_s1 = pr->readString("s1");
  m_s2 = pr->readString("s2");
  int32_t byteArrLen = 0;
  m_bytes66000 = pr->readByteArray("bytes66000", byteArrLen);
  m_s3 = pr->readString("s3");
  m_s4 = pr->readString("s4");
  m_s5 = pr->readString("s5");
}

/************************************************************
 *  PdxTypes10
 * *********************************************************/
PdxTypes10::PdxTypes10() {
  m_s1 = (char *)"one";
  m_s2 = (char *)"two";
  m_s3 = (char *)"three";
  m_bytes66000 = new int8_t[66000];
  m_s4 = (char *)"four";
  m_s5 = (char *)"five";
}

PdxTypes10::~PdxTypes10() { delete[] m_bytes66000; }

int32_t PdxTypes10::getHashCode() { return 1; }

bool PdxTypes10::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypes10>(obj);
  if (pap == nullptr) return false;

  // if (pap.get() == this)
  //	return true;

  if ((strcmp(m_s1, pap->m_s1) == 0) && (strcmp(m_s2, pap->m_s2) == 0) &&
      (strcmp(m_s3, pap->m_s3) == 0) && (strcmp(m_s4, pap->m_s4) == 0) &&
      (strcmp(m_s5, pap->m_s5) == 0)) {
    // Check byte[] length.
    // if(m_bytes66000.Length == pap.m_bytes66000.Length)
    return true;
  }

  return false;
}

std::shared_ptr<CacheableString> PdxTypes10::toString() const {
  char idbuf[4096];
  sprintf(idbuf, "PdxTypes9:[m_s1=%s] [m_s2=%s] [m_s3=%s] [m_s4=%s] [m_s5=%s] ",
          m_s1, m_s2, m_s3, m_s4, m_s5);
  return CacheableString::create(idbuf);
}

void PdxTypes10::toData(std::shared_ptr<PdxWriter> pw) {
  pw->writeString("s1", m_s1);
  pw->writeString("s2", m_s2);
  pw->writeByteArray("bytes66000", m_bytes66000, 2);
  pw->writeString("s3", m_s3);
  pw->writeString("s4", m_s4);
  pw->writeString("s5", m_s5);
}

void PdxTypes10::fromData(std::shared_ptr<PdxReader> pr) {
  m_s1 = pr->readString("s1");
  m_s2 = pr->readString("s2");
  int32_t byteArrLen = 0;
  m_bytes66000 = pr->readByteArray("bytes66000", byteArrLen);
  m_s3 = pr->readString("s3");
  m_s4 = pr->readString("s4");
  m_s5 = pr->readString("s5");
}

/************************************************************
 *  NestedPdx
 * *********************************************************/

NestedPdx::NestedPdx() {
  m_pd1 = std::make_shared<PdxTypes1>();
  m_pd2 = std::make_shared<PdxTypes2>();
  m_s1 = (char *)"one";
  m_s2 = (char *)"two";
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

NestedPdx::NestedPdx(char *key) {
  m_pd1 = std::make_shared<PdxTypes1>();
  m_pd2 = std::make_shared<PdxTypes2>();
  size_t len = strlen("NestedPdx ") + strlen(key) + 1;
  m_s1 = new char[len];
  strcpy(m_s1, "NestedPdx ");
  strcat(m_s1, key);
  m_s2 = (char *)"two";
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}
NestedPdx::~NestedPdx() {
  // TODO Auto-generated destructor stub
}

int32_t NestedPdx::getHashCode() { return 1; }

bool NestedPdx::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<NestedPdx>(obj);
  if (pap == nullptr) return false;

  // if (pap.get() == this)
  //	return true;

  if (m_i1 == pap->m_i1 && m_i2 == pap->m_i2 && m_i3 == pap->m_i3 &&
      m_i4 == pap->m_i4 && (strcmp(m_s1, pap->m_s1) == 0) &&
      (strcmp(m_s2, pap->m_s2) == 0) && (m_pd1->equals(pap->m_pd1) == true) &&
      (m_pd2->equals(pap->m_pd2) == true)) {
    return true;
  }

  return false;
}

std::shared_ptr<CacheableString> NestedPdx::toString() const {
  char idbuf[4096];
  sprintf(
      idbuf,
      "NestedPdx:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] [m_s1=%s] [m_s2=%s]",
      m_i1, m_i2, m_i3, m_i4, m_s1, m_s2);
  return CacheableString::create(idbuf);
}

void NestedPdx::toData(std::shared_ptr<PdxWriter> pw) {
  pw->writeInt("i1", m_i1);
  pw->writeObject("pd1", m_pd1);
  pw->writeInt("i2", m_i2);
  pw->writeString("s1", m_s1);
  pw->writeString("s2", m_s2);
  pw->writeObject("pd2", m_pd2);
  pw->writeInt("i3", m_i3);
  pw->writeInt("i4", m_i4);
}

void NestedPdx::fromData(std::shared_ptr<PdxReader> pr) {
  m_i1 = pr->readInt("i1");
  m_pd1 = std::static_pointer_cast<PdxTypes1>(pr->readObject("pd1"));
  m_i2 = pr->readInt("i2");
  m_s1 = pr->readString("s1");
  m_s2 = pr->readString("s2");
  m_pd2 = std::static_pointer_cast<PdxTypes2>(pr->readObject("pd2"));
  m_i3 = pr->readInt("i3");
  m_i4 = pr->readInt("i4");
}

/************************************************************
 *  MixedVersionNestedPdx
 * *********************************************************/

MixedVersionNestedPdx::MixedVersionNestedPdx() {
  m_pd1 = std::make_shared<PdxTypes1>();
  m_pd2 = std::make_shared<PdxTypes2>();
  m_s1 = (char *)"one";
  m_s2 = (char *)"two";
  m_s3 = (char *)"three";
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

MixedVersionNestedPdx::MixedVersionNestedPdx(char *key) {
  m_pd1 = std::make_shared<PdxTypes1>();
  m_pd2 = std::make_shared<PdxTypes2>();
  size_t len = strlen("MixedVersionNestedPdx ") + strlen(key) + 1;
  m_s1 = new char[len];
  strcpy(m_s1, "MixedVersionNestedPdx ");
  strcat(m_s1, key);
  m_s2 = (char *)"two";
  m_s3 = (char *)"three";
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}
MixedVersionNestedPdx::~MixedVersionNestedPdx() {
  // TODO Auto-generated destructor stub
}

int32_t MixedVersionNestedPdx::getHashCode() { return 1; }

bool MixedVersionNestedPdx::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<MixedVersionNestedPdx>(obj);
  if (pap == nullptr) return false;

  // if (pap.get() == this)
  //	return true;

  if (m_i1 == pap->m_i1 && m_i2 == pap->m_i2 && m_i3 == pap->m_i3 &&
      m_i4 == pap->m_i4 && (strcmp(m_s1, pap->m_s1) == 0) &&
      (strcmp(m_s2, pap->m_s2) == 0) && (m_pd1->equals(pap->m_pd1) == true) &&
      (m_pd2->equals(pap->m_pd2) == true)) {
    return true;
  }

  return false;
}

std::shared_ptr<CacheableString> MixedVersionNestedPdx::toString() const {
  char idbuf[4096];
  sprintf(idbuf,
          "MixedVersionNestedPdx:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] "
          "[m_s1=%s] [m_s2=%s]",
          m_i1, m_i2, m_i3, m_i4, m_s1, m_s2);
  return CacheableString::create(idbuf);
}

void MixedVersionNestedPdx::toData(std::shared_ptr<PdxWriter> pw) {
  pw->writeInt("i1", m_i1);
  pw->writeObject("pd1", m_pd1);
  pw->writeInt("i2", m_i2);
  pw->writeString("s1", m_s1);
  pw->writeString("s2", m_s2);
  pw->writeString("s3", m_s3);
  pw->writeObject("pd2", m_pd2);
  pw->writeInt("i3", m_i3);
  pw->writeInt("i4", m_i4);
}

void MixedVersionNestedPdx::fromData(std::shared_ptr<PdxReader> pr) {
  m_i1 = pr->readInt("i1");
  m_pd1 = std::static_pointer_cast<PdxTypes1>(pr->readObject("pd1"));
  m_i2 = pr->readInt("i2");
  m_s1 = pr->readString("s1");
  m_s2 = pr->readString("s2");
  // Mixed version missing: m_s3=pr->readString("m_s3")
  m_pd2 = std::static_pointer_cast<PdxTypes2>(pr->readObject("pd2"));
  m_i3 = pr->readInt("i3");
  m_i4 = pr->readInt("i4");
}

/************************************************************
 *  PdxInsideIGeodeSerializable
 * *********************************************************/
PdxInsideIGeodeSerializable::PdxInsideIGeodeSerializable() {
  m_npdx = std::make_shared<NestedPdx>();
  m_pdx3 = std::make_shared<PdxTypes3>();
  m_s1 = (char *)"one";
  m_s2 = (char *)"two";
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxInsideIGeodeSerializable::~PdxInsideIGeodeSerializable() {
  // TODO Auto-generated destructor stub
}

int32_t PdxInsideIGeodeSerializable::getHashCode() { return 1; }

bool PdxInsideIGeodeSerializable::equals(std::shared_ptr<Serializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxInsideIGeodeSerializable>(obj);
  if (pap == nullptr) return false;

  // if (pap.get() == this)
  //	return true;

  if (m_i1 == pap->m_i1 && m_i2 == pap->m_i2 && m_i3 == pap->m_i3 &&
      (strcmp(m_s1, pap->m_s1) == 0) && (strcmp(m_s2, pap->m_s2) == 0) &&
      m_npdx->equals(pap->m_npdx) && m_pdx3->equals(pap->m_pdx3)) {
    return true;
  }

  return false;
}

std::shared_ptr<CacheableString> PdxInsideIGeodeSerializable::toString() const {
  char idbuf[4096];
  sprintf(idbuf,
          "PdxInsideIGeodeSerializable:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] "
          "[m_s1=%s] [m_s2=%s]",
          m_i1, m_i2, m_i3, m_i4, m_s1, m_s2);
  return CacheableString::create(idbuf);
}

void PdxInsideIGeodeSerializable::toData(DataOutput &output) const {
  output.writeInt(m_i1);
  output.writeObject(m_npdx);
  output.writeInt(m_i2);
  output.writeUTF(m_s1);
  output.writeUTF(m_s2);
  output.writeObject(m_pdx3);
  output.writeInt(m_i3);
  output.writeInt(m_i4);
}

void PdxInsideIGeodeSerializable::fromData(DataInput &input) {
  m_i1 = input.readInt32();
  m_npdx = input.readObject<NestedPdx>();
  m_i2 = input.readInt32();
  input.readUTF(&m_s1);
  input.readUTF(&m_s2);
  m_pdx3 = input.readObject<PdxTypes3>();
  m_i3 = input.readInt32();
  m_i4 = input.readInt32();
}

} /* namespace PdxTests */
