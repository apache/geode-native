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

#include <util/Log.hpp>

#include <geode/CacheableEnum.hpp>

namespace PdxTests {

using apache::geode::client::CacheableEnum;

/************************************************************
 *  PdxTypes1
 * *********************************************************/

PdxTypes1::PdxTypes1() {
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxTypes1::~PdxTypes1() noexcept {
  // TODO Auto-generated destructor stub
}

int32_t PdxTypes1::getHashCode() { return 1; }

bool PdxTypes1::equals(std::shared_ptr<PdxSerializable> obj) {
  // LOG_DEBUG("NIL:PdxTypes1::==::33");
  if (obj == nullptr) {
    // LOG_DEBUG("NIL:PdxTypes1::==::35");
    return false;
  }
  auto pap = std::dynamic_pointer_cast<PdxTypes1>(obj);
  if (pap == nullptr) {
    // LOG_DEBUG("NIL:PdxTypes1::==::40");
    return false;
  }
  if (pap.get() == this) {
    // LOG_DEBUG("NIL:PdxTypes1::==::44");
    return true;
  }
  LOG_INFO("PdxTypes1:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d]", m_i1, m_i2,
           m_i3, m_i4);
  if (m_i1 == pap->m_i1 && m_i2 == pap->m_i2 && m_i3 == pap->m_i3 &&
      m_i4 == pap->m_i4) {
    // LOG_DEBUG("NIL:PdxTypes1::==::48");
    return true;
  }
  LOG_DEBUG("NIL:PdxTypes1::==::51");
  return false;
}
std::string PdxTypes1::toString() const {
  char idbuf[4096];
  sprintf(idbuf, "PdxTypes1:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d]", m_i1,
          m_i2, m_i3, m_i4);
  return idbuf;
}

void PdxTypes1::toData(PdxWriter &pw) const {
  pw.writeInt("i1", m_i1);
  pw.writeInt("i2", m_i2);
  pw.writeInt("i3", m_i3);
  pw.writeInt("i4", m_i4);
}

void PdxTypes1::fromData(PdxReader &pr) {
  m_i1 = pr.readInt("i1");
  m_i2 = pr.readInt("i2");
  m_i3 = pr.readInt("i3");
  m_i4 = pr.readInt("i4");
}

/************************************************************
 *  PdxTypes2
 * *********************************************************/
PdxTypes2::PdxTypes2() {
  m_s1 = "one";
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxTypes2::~PdxTypes2() noexcept {
  // TODO Auto-generated destructor stub
}

int32_t PdxTypes2::getHashCode() { return 1; }

bool PdxTypes2::equals(std::shared_ptr<PdxSerializable> obj) {
  // LOG_DEBUG("NIL:96:this::PdxType2 = %s", this->toString());

  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypes2>(obj);
  // LOG_DEBUG("NIl:102:pap::PdxType2 = %s", pap->toString());
  if (pap == nullptr) return false;

  // if (pap.get() == this)
  //	return true;

  if (m_i1 == pap->m_i1 && m_i2 == pap->m_i2 && m_i3 == pap->m_i3 &&
      m_i4 == pap->m_i4 && m_s1 == pap->m_s1) {
    return true;
  }

  return false;
}
std::string PdxTypes2::toString() const {
  char idbuf[4096];
  sprintf(idbuf, "PdxTypes2:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] [m_s1=%s]",
          m_i1, m_i2, m_i3, m_i4, m_s1.c_str());
  return idbuf;
}

void PdxTypes2::toData(PdxWriter &pw) const {
  pw.writeString("s1", m_s1);
  pw.writeInt("i1", m_i1);
  pw.writeInt("i2", m_i2);
  pw.writeInt("i3", m_i3);
  pw.writeInt("i4", m_i4);
}

void PdxTypes2::fromData(PdxReader &pr) {
  m_s1 = pr.readString("s1");
  m_i1 = pr.readInt("i1");
  m_i2 = pr.readInt("i2");
  m_i3 = pr.readInt("i3");
  m_i4 = pr.readInt("i4");
}

/************************************************************
 *  PdxTypes3
 * *********************************************************/
PdxTypes3::PdxTypes3() {
  m_s1 = "one";
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxTypes3::~PdxTypes3() noexcept {
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
      m_i4 == pap->m_i4 && m_s1 == pap->m_s1) {
    return true;
  }

  return false;
}
std::string PdxTypes3::toString() const {
  char idbuf[4096];
  sprintf(idbuf, "PdxTypes3:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] [m_s1=%s]",
          m_i1, m_i2, m_i3, m_i4, m_s1.c_str());
  return idbuf;
}

void PdxTypes3::toData(PdxWriter &pw) const {
  pw.writeInt("i1", m_i1);
  pw.writeInt("i2", m_i2);
  pw.writeInt("i3", m_i3);
  pw.writeInt("i4", m_i4);
  pw.writeString("s1", m_s1);
}

void PdxTypes3::fromData(PdxReader &pr) {
  m_i1 = pr.readInt("i1");
  m_i2 = pr.readInt("i2");
  m_i3 = pr.readInt("i3");
  m_i4 = pr.readInt("i4");
  m_s1 = pr.readString("s1");
}

/************************************************************
 *  PdxTypes4
 * *********************************************************/
PdxTypes4::PdxTypes4() {
  m_s1 = "one";
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxTypes4::~PdxTypes4() noexcept {
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
      m_i4 == pap->m_i4 && m_s1 == pap->m_s1) {
    return true;
  }

  return false;
}
std::string PdxTypes4::toString() const {
  char idbuf[4096];
  sprintf(idbuf, "PdxTypes4:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] [m_s1=%s]",
          m_i1, m_i2, m_i3, m_i4, m_s1.c_str());
  return idbuf;
}

void PdxTypes4::toData(PdxWriter &pw) const {
  pw.writeInt("i1", m_i1);
  pw.writeInt("i2", m_i2);
  pw.writeString("s1", m_s1);
  pw.writeInt("i3", m_i3);
  pw.writeInt("i4", m_i4);
}

void PdxTypes4::fromData(PdxReader &pr) {
  m_i1 = pr.readInt("i1");
  m_i2 = pr.readInt("i2");
  m_s1 = pr.readString("s1");
  m_i3 = pr.readInt("i3");
  m_i4 = pr.readInt("i4");
}

/************************************************************
 *  PdxTypes5
 * *********************************************************/
PdxTypes5::PdxTypes5() {
  m_s1 = "one";
  m_s2 = "two";
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxTypes5::~PdxTypes5() noexcept {
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
      m_i4 == pap->m_i4 && m_s1 == pap->m_s1 && m_s2 == pap->m_s2) {
    return true;
  }

  return false;
}
std::string PdxTypes5::toString() const {
  char idbuf[4096];
  sprintf(
      idbuf,
      "PdxTypes4:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] [m_s1=%s] [m_s2=%s]",
      m_i1, m_i2, m_i3, m_i4, m_s1.c_str(), m_s2.c_str());
  return idbuf;
}

void PdxTypes5::toData(PdxWriter &pw) const {
  pw.writeString("s1", m_s1);
  pw.writeString("s2", m_s2);
  pw.writeInt("i1", m_i1);
  pw.writeInt("i2", m_i2);
  pw.writeInt("i3", m_i3);
  pw.writeInt("i4", m_i4);
}

void PdxTypes5::fromData(PdxReader &pr) {
  m_s1 = pr.readString("s1");
  m_s2 = pr.readString("s2");
  m_i1 = pr.readInt("i1");
  m_i2 = pr.readInt("i2");
  m_i3 = pr.readInt("i3");
  m_i4 = pr.readInt("i4");
}

/************************************************************
 *  PdxTypes6
 * *********************************************************/
PdxTypes6::PdxTypes6() {
  m_s1 = "one";
  m_s2 = "two";
  bytes128 = std::vector<int8_t>(2);
  bytes128[0] = 0x34;
  bytes128[1] = 0x64;
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxTypes6::~PdxTypes6() noexcept {
  // TODO Auto-generated destructor stub
}

int32_t PdxTypes6::getHashCode() { return 1; }

bool PdxTypes6::equals(std::shared_ptr<PdxSerializable> obj) {
  LOG_DEBUG("PdxTypes6::equals -1");
  if (obj == nullptr) return false;

  LOG_DEBUG("PdxTypes6::equals -2");
  auto pap = std::dynamic_pointer_cast<PdxTypes6>(obj);
  if (pap == nullptr) return false;

  LOG_DEBUG("PdxTypes6::equals -3 m_i1 = %d", m_i1);
  LOG_DEBUG("PdxTypes6::equals -4 m_i2 = %d", m_i2);
  LOG_DEBUG("PdxTypes6::equals -5 m_i3 = %d", m_i3);
  LOG_DEBUG("PdxTypes6::equals -6 m_i4 = %d", m_i4);
  LOG_DEBUG("PdxTypes6::equals -7 m_s1 = %s", m_s1.c_str());
  LOG_DEBUG("PdxTypes6::equals -8 m_s2 = %s", m_s2.c_str());

  LOG_DEBUG("PdxTypes6::equals -9 pap->m_i1 = %d", pap->m_i1);
  LOG_DEBUG("PdxTypes6::equals -10 pap->m_i2 = %d", pap->m_i2);
  LOG_DEBUG("PdxTypes6::equals -11 pap->m_i3 = %d", pap->m_i3);
  LOG_DEBUG("PdxTypes6::equals -12 pap->m_i4 = %d", pap->m_i4);
  LOG_DEBUG("PdxTypes6::equals -13 pap->m_s1 = %s", pap->m_s1.c_str());
  LOG_DEBUG("PdxTypes6::equals -14 pap->m_s2 = %s", pap->m_s2.c_str());
  // if (pap.get() == this)
  //	return true;

  if (m_i1 == pap->m_i1 && m_i2 == pap->m_i2 && m_i3 == pap->m_i3 &&
      m_i4 == pap->m_i4 && m_s1 == pap->m_s1 && m_s2 == pap->m_s2) {
    // Check byte[] length.
    // if(bytes128.Length == pap.bytes128.Length)
    return true;
  }

  return false;
}
std::string PdxTypes6::toString() const {
  char idbuf[4096];
  sprintf(
      idbuf,
      "PdxTypes4:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] [m_s1=%s] [m_s2=%s]",
      m_i1, m_i2, m_i3, m_i4, m_s1.c_str(), m_s2.c_str());
  return idbuf;
}

void PdxTypes6::toData(PdxWriter &pw) const {
  pw.writeString("s1", m_s1);
  pw.writeInt("i1", m_i1);
  pw.writeByteArray("bytes128", bytes128);
  pw.writeInt("i2", m_i2);
  pw.writeInt("i3", m_i3);
  pw.writeInt("i4", m_i4);
  pw.writeString("s2", m_s2);
}

void PdxTypes6::fromData(PdxReader &pr) {
  m_s1 = pr.readString("s1");
  // LOG_DEBUG("PdxTypes6::fromData m_s1 = %s", m_s1);

  m_i1 = pr.readInt("i1");
  // LOG_DEBUG("PdxTypes6::fromData m_i1 = %d", m_i1);
  bytes128 = pr.readByteArray("bytes128");
  m_i2 = pr.readInt("i2");
  // LOG_DEBUG("PdxTypes6::fromData m_i2 = %d", m_i2);

  m_i3 = pr.readInt("i3");
  // LOG_DEBUG("PdxTypes6::fromData m_i3 = %d", m_i3);

  m_i4 = pr.readInt("i4");
  // LOG_DEBUG("PdxTypes6::fromData m_i4 = %d", m_i4);

  m_s2 = pr.readString("s2");
  // LOG_DEBUG("PdxTypes6::fromData m_s2 = %s", m_s2);
}

/************************************************************
 *  PdxTypes7
 * *********************************************************/
PdxTypes7::PdxTypes7() {
  m_s1 = "one";
  m_s2 = "two";
  m_i1 = 34324;
  bytes38000 = std::vector<int8_t>(38000);
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxTypes7::~PdxTypes7() noexcept {
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
      m_i4 == pap->m_i4 && m_s1 == pap->m_s1 && m_s2 == pap->m_s2) {
    // Check byte[] length.
    // if(bytes38000.Length == pap.bytes38000.Length)
    return true;
  }

  return false;
}
std::string PdxTypes7::toString() const {
  char idbuf[4096];
  sprintf(
      idbuf,
      "PdxTypes7:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] [m_s1=%s] [m_s2=%s]",
      m_i1, m_i2, m_i3, m_i4, m_s1.c_str(), m_s2.c_str());
  return idbuf;
}

void PdxTypes7::toData(PdxWriter &pw) const {
  pw.writeInt("i1", m_i1);
  pw.writeInt("i2", m_i2);
  pw.writeString("s1", m_s1);
  pw.writeByteArray("bytes38000", bytes38000);
  pw.writeInt("i3", m_i3);
  pw.writeInt("i4", m_i4);
  pw.writeString("s2", m_s2);
}

void PdxTypes7::fromData(PdxReader &pr) {
  m_i1 = pr.readInt("i1");
  m_i2 = pr.readInt("i2");
  m_s1 = pr.readString("s1");
  bytes38000 = pr.readByteArray("bytes38000");
  m_i3 = pr.readInt("i3");
  m_i4 = pr.readInt("i4");
  m_s2 = pr.readString("s2");
}

/************************************************************
 *  PdxTypes8
 * *********************************************************/
PdxTypes8::PdxTypes8() {
  enum pdxEnumTest { pdx1, pdx2, pdx3 };
  m_s1 = "one";
  m_s2 = "two";
  m_i1 = 34324;
  bytes300 = std::vector<int8_t>(300);
  _enum = CacheableEnum::create("PdxTests.pdxEnumTest", "pdx2", pdx2);
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

int32_t PdxTypes8::getHashCode() { return 1; }

bool PdxTypes8::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypes8>(obj);
  if (pap == nullptr) return false;

  // if (pap.get() == this)
  //	return true;

  if (m_i1 == pap->m_i1 && m_i2 == pap->m_i2 && m_i3 == pap->m_i3 &&
      m_i4 == pap->m_i4 && m_s1 == pap->m_s1 && m_s2 == pap->m_s2) {
    // Check byte[] length.
    // if(bytes300.Length == pap.bytes300.Length)
    return true;
  }

  return false;
}
std::string PdxTypes8::toString() const {
  char idbuf[4096];
  sprintf(
      idbuf,
      "PdxTypes8:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] [m_s1=%s] [m_s2=%s]",
      m_i1, m_i2, m_i3, m_i4, m_s1.c_str(), m_s2.c_str());
  return idbuf;
}

void PdxTypes8::toData(PdxWriter &pw) const {
  pw.writeInt("i1", m_i1);
  pw.writeInt("i2", m_i2);
  pw.writeString("s1", m_s1);
  pw.writeByteArray("bytes300", bytes300);
  pw.writeObject("_enum", _enum);
  pw.writeString("s2", m_s2);
  pw.writeInt("i3", m_i3);
  pw.writeInt("i4", m_i4);
}

void PdxTypes8::fromData(PdxReader &pr) {
  m_i1 = pr.readInt("i1");
  m_i2 = pr.readInt("i2");
  m_s1 = pr.readString("s1");
  bytes300 = pr.readByteArray("bytes300");
  _enum = pr.readObject("_enum");
  m_s2 = pr.readString("s2");
  m_i3 = pr.readInt("i3");
  m_i4 = pr.readInt("i4");
}

/************************************************************
 *  PdxTypes9
 * *********************************************************/
PdxTypes9::PdxTypes9() {
  m_s1 = "one";
  m_s2 = "two";
  m_s3 = "three";
  m_bytes66000 = std::vector<int8_t>(66000);
  m_s4 = "four";
  m_s5 = "five";
}

int32_t PdxTypes9::getHashCode() { return 1; }

bool PdxTypes9::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypes9>(obj);
  if (pap == nullptr) return false;

  // if (pap.get() == this)
  //	return true;

  if (m_s1 == pap->m_s1 && m_s2 == pap->m_s2 && m_s3 == pap->m_s3 &&
      m_s4 == pap->m_s4 && m_s5 == pap->m_s5) {
    // Check byte[] length.
    // if(m_bytes66000.Length == pap.m_bytes66000.Length)
    return true;
  }

  return false;
}
std::string PdxTypes9::toString() const {
  char idbuf[4096];
  sprintf(idbuf, "PdxTypes9:[m_s1=%s] [m_s2=%s] [m_s3=%s] [m_s4=%s] [m_s5=%s] ",
          m_s1.c_str(), m_s2.c_str(), m_s3.c_str(), m_s4.c_str(), m_s5.c_str());
  return idbuf;
}

void PdxTypes9::toData(PdxWriter &pw) const {
  pw.writeString("s1", m_s1);
  pw.writeString("s2", m_s2);
  pw.writeByteArray("bytes66000", m_bytes66000);
  pw.writeString("s3", m_s3);
  pw.writeString("s4", m_s4);
  pw.writeString("s5", m_s5);
}

void PdxTypes9::fromData(PdxReader &pr) {
  m_s1 = pr.readString("s1");
  m_s2 = pr.readString("s2");
  m_bytes66000 = pr.readByteArray("bytes66000");
  m_s3 = pr.readString("s3");
  m_s4 = pr.readString("s4");
  m_s5 = pr.readString("s5");
}

/************************************************************
 *  PdxTypes10
 * *********************************************************/
PdxTypes10::PdxTypes10() {
  m_s1 = "one";
  m_s2 = "two";
  m_s3 = "three";
  m_bytes66000 = std::vector<int8_t>(66000);
  m_s4 = "four";
  m_s5 = "five";
}

int32_t PdxTypes10::getHashCode() { return 1; }

bool PdxTypes10::equals(std::shared_ptr<PdxSerializable> obj) {
  if (obj == nullptr) return false;

  auto pap = std::dynamic_pointer_cast<PdxTypes10>(obj);
  if (pap == nullptr) return false;

  // if (pap.get() == this)
  //	return true;

  if (m_s1 == pap->m_s1 && m_s2 == pap->m_s2 && m_s3 == pap->m_s3 &&
      m_s4 == pap->m_s4 && m_s5 == pap->m_s5) {
    // Check byte[] length.
    // if(m_bytes66000.Length == pap.m_bytes66000.Length)
    return true;
  }

  return false;
}
std::string PdxTypes10::toString() const {
  char idbuf[4096];
  sprintf(idbuf, "PdxTypes9:[m_s1=%s] [m_s2=%s] [m_s3=%s] [m_s4=%s] [m_s5=%s] ",
          m_s1.c_str(), m_s2.c_str(), m_s3.c_str(), m_s4.c_str(), m_s5.c_str());
  return idbuf;
}

void PdxTypes10::toData(PdxWriter &pw) const {
  pw.writeString("s1", m_s1);
  pw.writeString("s2", m_s2);
  pw.writeByteArray("bytes66000", m_bytes66000);
  pw.writeString("s3", m_s3);
  pw.writeString("s4", m_s4);
  pw.writeString("s5", m_s5);
}

void PdxTypes10::fromData(PdxReader &pr) {
  m_s1 = pr.readString("s1");
  m_s2 = pr.readString("s2");
  m_bytes66000 = pr.readByteArray("bytes66000");
  m_s3 = pr.readString("s3");
  m_s4 = pr.readString("s4");
  m_s5 = pr.readString("s5");
}

/************************************************************
 *  NestedPdx
 * *********************************************************/

NestedPdx::NestedPdx() {
  m_pd1 = std::make_shared<PdxTypes1>();
  m_pd2 = std::make_shared<PdxTypes2>();
  m_s1 = "one";
  m_s2 = "two";
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

NestedPdx::NestedPdx(char *key) {
  m_pd1 = std::make_shared<PdxTypes1>();
  m_pd2 = std::make_shared<PdxTypes2>();
  m_s1 = std::string("NestedPdx ") + key;
  m_s2 = "two";
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}
NestedPdx::~NestedPdx() noexcept {
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
      m_i4 == pap->m_i4 && m_s1 == pap->m_s1 && m_s2 == pap->m_s2 &&
      m_pd1->equals(pap->m_pd1) && m_pd2->equals(pap->m_pd2)) {
    return true;
  }

  return false;
}
std::string NestedPdx::toString() const {
  char idbuf[4096];
  sprintf(
      idbuf,
      "NestedPdx:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] [m_s1=%s] [m_s2=%s]",
      m_i1, m_i2, m_i3, m_i4, m_s1.c_str(), m_s2.c_str());
  return idbuf;
}

void NestedPdx::toData(PdxWriter &pw) const {
  pw.writeInt("i1", m_i1);
  pw.writeObject("pd1", m_pd1);
  pw.writeInt("i2", m_i2);
  pw.writeString("s1", m_s1);
  pw.writeString("s2", m_s2);
  pw.writeObject("pd2", m_pd2);
  pw.writeInt("i3", m_i3);
  pw.writeInt("i4", m_i4);
}

void NestedPdx::fromData(PdxReader &pr) {
  m_i1 = pr.readInt("i1");
  m_pd1 = std::dynamic_pointer_cast<PdxTypes1>(pr.readObject("pd1"));
  m_i2 = pr.readInt("i2");
  m_s1 = pr.readString("s1");
  m_s2 = pr.readString("s2");
  m_pd2 = std::dynamic_pointer_cast<PdxTypes2>(pr.readObject("pd2"));
  m_i3 = pr.readInt("i3");
  m_i4 = pr.readInt("i4");
}

/************************************************************
 *  MixedVersionNestedPdx
 * *********************************************************/

MixedVersionNestedPdx::MixedVersionNestedPdx() {
  m_pd1 = std::make_shared<PdxTypes1>();
  m_pd2 = std::make_shared<PdxTypes2>();
  m_s1 = "one";
  m_s2 = "two";
  m_s3 = "three";
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

MixedVersionNestedPdx::MixedVersionNestedPdx(char *key) {
  m_pd1 = std::make_shared<PdxTypes1>();
  m_pd2 = std::make_shared<PdxTypes2>();
  m_s1 = std::string("MixedVersionNestedPdx ") + key;
  m_s2 = "two";
  m_s3 = "three";
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}
MixedVersionNestedPdx::~MixedVersionNestedPdx() noexcept {
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
      m_i4 == pap->m_i4 && m_s1 == pap->m_s1 && m_s2 == pap->m_s2 &&
      m_pd1->equals(pap->m_pd1) && m_pd2->equals(pap->m_pd2)) {
    return true;
  }

  return false;
}
std::string MixedVersionNestedPdx::toString() const {
  char idbuf[4096];
  sprintf(idbuf,
          "MixedVersionNestedPdx:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] "
          "[m_s1=%s] [m_s2=%s]",
          m_i1, m_i2, m_i3, m_i4, m_s1.c_str(), m_s2.c_str());
  return idbuf;
}

void MixedVersionNestedPdx::toData(PdxWriter &pw) const {
  pw.writeInt("i1", m_i1);
  pw.writeObject("pd1", m_pd1);
  pw.writeInt("i2", m_i2);
  pw.writeString("s1", m_s1);
  pw.writeString("s2", m_s2);
  pw.writeString("s3", m_s3);
  pw.writeObject("pd2", m_pd2);
  pw.writeInt("i3", m_i3);
  pw.writeInt("i4", m_i4);
}

void MixedVersionNestedPdx::fromData(PdxReader &pr) {
  m_i1 = pr.readInt("i1");
  m_pd1 = std::dynamic_pointer_cast<PdxTypes1>(pr.readObject("pd1"));
  m_i2 = pr.readInt("i2");
  m_s1 = pr.readString("s1");
  m_s2 = pr.readString("s2");
  // Mixed version missing: m_s3=pr.readString("m_s3")
  m_pd2 = std::dynamic_pointer_cast<PdxTypes2>(pr.readObject("pd2"));
  m_i3 = pr.readInt("i3");
  m_i4 = pr.readInt("i4");
}

/************************************************************
 *  PdxInsideIGeodeSerializable
 * *********************************************************/
PdxInsideIGeodeSerializable::PdxInsideIGeodeSerializable() {
  m_npdx = std::make_shared<NestedPdx>();
  m_pdx3 = std::make_shared<PdxTypes3>();
  m_s1 = "one";
  m_s2 = "two";
  m_i1 = 34324;
  m_i2 = 2144;
  m_i3 = 4645734;
  m_i4 = 73567;
}

PdxInsideIGeodeSerializable::~PdxInsideIGeodeSerializable() noexcept {
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
      m_s1 == pap->m_s1 && m_s2 == pap->m_s2 && m_npdx->equals(pap->m_npdx) &&
      m_pdx3->equals(pap->m_pdx3)) {
    return true;
  }

  return false;
}
std::string PdxInsideIGeodeSerializable::toString() const {
  char idbuf[4096];
  sprintf(idbuf,
          "PdxInsideIGeodeSerializable:[m_i1=%d] [m_i2=%d] [m_i3=%d] [m_i4=%d] "
          "[m_s1=%s] [m_s2=%s]",
          m_i1, m_i2, m_i3, m_i4, m_s1.c_str(), m_s2.c_str());
  return idbuf;
}

void PdxInsideIGeodeSerializable::toData(DataOutput &output) const {
  output.writeInt(m_i1);
  output.writeObject(m_npdx);
  output.writeInt(m_i2);
  output.writeString(m_s1);
  output.writeString(m_s2);
  output.writeObject(m_pdx3);
  output.writeInt(m_i3);
  output.writeInt(m_i4);
}

void PdxInsideIGeodeSerializable::fromData(DataInput &input) {
  m_i1 = input.readInt32();
  m_npdx = std::dynamic_pointer_cast<NestedPdx>(input.readObject());
  m_i2 = input.readInt32();
  m_s1 = input.readString();
  m_s2 = input.readString();
  m_pdx3 = std::dynamic_pointer_cast<PdxTypes3>(input.readObject());
  m_i3 = input.readInt32();
  m_i4 = input.readInt32();
}

} /* namespace PdxTests */
