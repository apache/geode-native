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

#ifndef GEODE_TESTOBJECT_PDXCLASSV1_H_
#define GEODE_TESTOBJECT_PDXCLASSV1_H_

#include <util/Log.hpp>

#include <geode/PdxReader.hpp>
#include <geode/PdxSerializable.hpp>
#include <geode/PdxSerializer.hpp>
#include <geode/PdxWriter.hpp>

#include "testobject_export.h"

namespace PdxTests {

using apache::geode::client::CacheableDate;
using apache::geode::client::PdxReader;
using apache::geode::client::PdxSerializable;
using apache::geode::client::PdxSerializer;
using apache::geode::client::PdxUnreadFields;
using apache::geode::client::PdxWriter;
using apache::geode::client::UserObjectSizer;

/************************************************************
 *  PdxType1V1
 * *********************************************************/
class TESTOBJECT_EXPORT PdxType1V1 : public PdxSerializable {
 private:
  static int32_t m_diffInSameFields;
  static bool m_useWeakHashMap;
  std::shared_ptr<PdxUnreadFields> m_pdxUreadFields;

  int32_t m_i1;  // = 34324;
  int32_t m_i2;  // = 2144;
  int32_t m_i3;  // = 4645734;
  int32_t m_i4;  // = 73567;

 public:
  PdxType1V1();

  ~PdxType1V1() noexcept override;

  static void reset(bool useWeakHashMap);

  int getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  std::string toString() const override;

  using PdxSerializable::toData;

  using PdxSerializable::fromData;

  virtual void fromData(PdxReader& pr) override;

  virtual void toData(PdxWriter& pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests::PdxTypes1V";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PdxType1V1>();
  }
};

/************************************************************
 *  PdxType2V1
 * *********************************************************/
class TESTOBJECT_EXPORT PdxType2V1 : public PdxSerializable {
 private:
  static int m_diffInSameFields;
  static bool m_useWeakHashMap;
  std::shared_ptr<PdxUnreadFields> m_unreadFields;
  int32_t m_i1;
  int32_t m_i2;
  int32_t m_i3;
  int32_t m_i4;

 public:
  PdxType2V1();

  ~PdxType2V1() noexcept override;

  static void reset(bool useWeakHashMap);

  int getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  std::string toString() const override;

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void fromData(PdxReader& pr) override;

  virtual void toData(PdxWriter& pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests::PdxTypes2V";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PdxType2V1>();
  }
};

/************************************************************
 *  PdxType3V1
 * *********************************************************/

class TESTOBJECT_EXPORT PdxType3V1 : public PdxSerializable {
 private:
  static int m_diffInSameFields;
  static int m_diffInExtraFields;
  static bool m_useWeakHashMap;
  std::shared_ptr<PdxUnreadFields> m_unreadFields;
  int32_t m_i1;
  int32_t m_i2;
  std::string m_str1;
  int32_t m_i3;
  int32_t m_i4;
  int32_t m_i5;
  std::string m_str2;

 public:
  PdxType3V1();

  ~PdxType3V1() noexcept override;

  std::string toString() const override;

  static void reset(bool useWeakHashMap);

  int getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void fromData(PdxReader& pr) override;

  virtual void toData(PdxWriter& pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests::PdxTypes3V";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PdxType3V1>();
  }
};

/************************************************************
 *  PdxTypesV1R1
 * *********************************************************/
class TESTOBJECT_EXPORT PdxTypesV1R1 : public PdxSerializable {
 private:
  static int m_diffInSameFields;
  static bool m_useWeakHashMap;
  std::shared_ptr<PdxUnreadFields> m_pdxUreadFields;
  int32_t m_i1;
  int32_t m_i2;
  int32_t m_i3;
  int32_t m_i4;

 public:
  PdxTypesV1R1();

  ~PdxTypesV1R1() noexcept override;

  std::string toString() const override;

  static void reset(bool useWeakHashMap);

  int getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void fromData(PdxReader& pr) override;

  virtual void toData(PdxWriter& pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests::PdxTypesR1";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PdxTypesV1R1>();
  }
};

/************************************************************
 *  PdxTypesV1R2
 * *********************************************************/
class TESTOBJECT_EXPORT PdxTypesV1R2 : public PdxSerializable {
 private:
  static int m_diffInSameFields;
  static bool m_useWeakHashMap;
  std::shared_ptr<PdxUnreadFields> m_pdxUreadFields;
  int32_t m_i1;
  int32_t m_i2;
  int32_t m_i3;
  int32_t m_i4;

 public:
  PdxTypesV1R2();

  ~PdxTypesV1R2() noexcept override;

  static void reset(bool useWeakHashMap);

  int getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  std::string toString() const override;

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void fromData(PdxReader& pr) override;

  virtual void toData(PdxWriter& pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests::PdxTypesR2";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PdxTypesV1R2>();
  }
};

/************************************************************
 *  PdxTypesIgnoreUnreadFieldsV1
 * *********************************************************/
class TESTOBJECT_EXPORT PdxTypesIgnoreUnreadFieldsV1 : public PdxSerializable {
 private:
  static int m_diffInSameFields;
  static bool m_useWeakHashMap;
  std::shared_ptr<PdxUnreadFields> m_unreadFields;
  int32_t m_i1;
  int32_t m_i2;
  int32_t m_i3;
  int32_t m_i4;

 public:
  PdxTypesIgnoreUnreadFieldsV1();

  ~PdxTypesIgnoreUnreadFieldsV1() noexcept override;

  std::string toString() const override;

  static void reset(bool useWeakHashMap);

  int getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void fromData(PdxReader& pr) override;

  virtual void toData(PdxWriter& pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTypesIgnoreUnreadFields";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PdxTypesIgnoreUnreadFieldsV1>();
  }
};

/************************************************************
 *  PdxVersionedV1
 * *********************************************************/
class TESTOBJECT_EXPORT PdxVersionedV1 : public PdxSerializable {
 private:
  char m_char;
  bool m_bool;
  int8_t m_byte;
  int16_t m_int16;
  int32_t m_int32;
  int64_t m_long;
  float m_float;
  double m_double;
  std::string m_string;
  std::vector<bool> m_boolArray;
  char* m_charArray;
  std::shared_ptr<CacheableDate> m_dateTime;
  std::vector<int16_t> m_int16Array;
  std::vector<int32_t> m_int32Array;
  std::vector<int64_t> m_longArray;
  std::vector<float> m_floatArray;
  std::vector<double> m_doubleArray;

  // IDictionary<object, object> m_map;
  // List<object> m_list;

  int32_t boolArrayLen;
  int32_t byteArrayLen;
  int32_t shortArrayLen;
  int32_t intArrayLen;
  int32_t longArrayLen;
  int32_t floatArrayLen;
  int32_t strLenArray;

 public:
  PdxVersionedV1();

  explicit PdxVersionedV1(int32_t size);

  ~PdxVersionedV1() noexcept override;

  void init(int32_t size);

  std::string toString() const override;

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void fromData(PdxReader& pr) override;

  virtual void toData(PdxWriter& pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests::PdxVersioned";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PdxVersionedV1>();
  }
};

/************************************************************
 *  TestKey
 * *********************************************************/

class TESTOBJECT_EXPORT TestKeyV1 {
 public:
  std::string _id;

 public:
  TestKeyV1();

  explicit TestKeyV1(char* id);
};

/************************************************************
 *  TestKey
 * *********************************************************/

class TESTOBJECT_EXPORT TestDiffTypePdxSV1 {
 public:
  std::string _id;
  std::string _name;

 public:
  TestDiffTypePdxSV1();

  explicit TestDiffTypePdxSV1(bool init);

  bool equals(const TestDiffTypePdxSV1& obj);
};

/************************************************************
 *  TestPdxSerializerForV1
 * *********************************************************/
static const char* V1CLASSNAME1 = "PdxTests.TestKey";
static const char* V1CLASSNAME2 = "PdxTests.TestDiffTypePdxS";

class TestPdxSerializerForV1 : public PdxSerializer {
 public:
  static void deallocate(void* testObject, const std::string& className) {
    // ASSERT(strcmp(className, V1CLASSNAME1) == 0 || strcmp(className,
    // V1CLASSNAME2) == 0 , "Unexpected classname in deallocate()");
    LOG_INFO("TestPdxSerializer::deallocate called");
    if (className == V1CLASSNAME1) {
      PdxTests::TestKeyV1* tkv1 =
          reinterpret_cast<PdxTests::TestKeyV1*>(testObject);
      delete tkv1;
    } else if (className == V1CLASSNAME2) {
      PdxTests::TestDiffTypePdxSV1* dtpv1 =
          reinterpret_cast<PdxTests::TestDiffTypePdxSV1*>(testObject);
      delete dtpv1;
    } else {
      LOG_INFO("TestPdxSerializerForV1::deallocate Invalid Class Name");
    }
  }

  static size_t objectSize(const std::shared_ptr<const void>&,
                           const std::string&) {
    return 12345;  // dummy value
  }

  UserObjectSizer getObjectSizer(const std::string&) override {
    return objectSize;
  }

  std::shared_ptr<void> fromDataForTestKeyV1(PdxReader& pr) {
    try {
      auto tkv1 = std::make_shared<PdxTests::TestKeyV1>();
      tkv1->_id = pr.readString("_id");
      return std::move(tkv1);
    } catch (...) {
      return nullptr;
    }
  }

  bool toDataForTestKeyV1(const std::shared_ptr<const void>& testObject,
                          PdxWriter& pw) {
    try {
      auto tkv1 =
          std::static_pointer_cast<const PdxTests::TestKeyV1>(testObject);
      pw.writeString("_id", tkv1->_id);

      return true;
    } catch (...) {
      return false;
    }
  }

  std::shared_ptr<void> fromDataForTestDiffTypePdxSV1(PdxReader& pr) {
    try {
      auto dtpv1 = std::make_shared<PdxTests::TestDiffTypePdxSV1>();
      dtpv1->_id = pr.readString("_id");
      dtpv1->_name = pr.readString("_name");
      return std::move(dtpv1);
    } catch (...) {
      return nullptr;
    }
  }

  bool toDataForTestDiffTypePdxSV1(
      const std::shared_ptr<const void>& testObject, PdxWriter& pw) {
    try {
      auto dtpv1 = std::static_pointer_cast<const PdxTests::TestDiffTypePdxSV1>(
          testObject);
      pw.writeString("_id", dtpv1->_id);
      pw.writeString("_name", dtpv1->_name);

      return true;
    } catch (...) {
      return false;
    }
  }

  std::shared_ptr<void> fromData(const std::string& className,
                                 PdxReader& pr) override {
    // ASSERT(strcmp(className, V1CLASSNAME1) == 0 || strcmp(className,
    // V1CLASSNAME2) == 0, "Unexpected classname in fromData");

    if (className == V1CLASSNAME2) {
      return fromDataForTestDiffTypePdxSV1(pr);

    } else if (className == V1CLASSNAME1) {
      return fromDataForTestKeyV1(pr);

    } else {
      LOG_INFO("TestPdxSerializerForV1::fromdata() Invalid Class Name");
      return nullptr;
    }
  }

  virtual bool toData(const std::shared_ptr<const void>& testObject,
                      const std::string& className, PdxWriter& pw) override {
    // ASSERT(strcmp(className, V1CLASSNAME1) == 0 || strcmp(className,
    // V1CLASSNAME2) == 0, "Unexpected classname in toData");

    if (className == V1CLASSNAME2) {
      return toDataForTestDiffTypePdxSV1(testObject, pw);

    } else if (className == V1CLASSNAME1) {
      return toDataForTestKeyV1(testObject, pw);

    } else {
      LOG_INFO("TestPdxSerializerForV1::fromdata() Invalid Class Name");
      return false;
    }
  }
};

/************************************************************
 *  TestEqualsV1
 * *********************************************************/

class TESTOBJECT_EXPORT TestEqualsV1 : public PdxSerializable {
 private:
  int32_t i1;
  int32_t i2;
  std::string s1;

 public:
  TestEqualsV1();

  std::string toString() const override;

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void fromData(PdxReader& pr) override;

  virtual void toData(PdxWriter& pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests::TestEquals";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<TestEqualsV1>();
  }
};

} /* namespace PdxTests */

#endif  // GEODE_TESTOBJECT_PDXCLASSV1_H_
