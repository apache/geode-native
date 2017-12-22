#pragma once

#ifndef GEODE_TESTOBJECT_PDXCLASSV2_H_
#define GEODE_TESTOBJECT_PDXCLASSV2_H_

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
 * PdxClassV1.hpp
 *
 *  Created on: Feb 3, 2012
 *      Author: npatel
 */

#include <geode/PdxSerializable.hpp>
#include <geode/PdxSerializer.hpp>
#include <geode/PdxWriter.hpp>
#include <geode/PdxReader.hpp>
#include <util/Log.hpp>
#include <util/Log.hpp>

#ifdef _WIN32
#ifdef BUILD_TESTOBJECT
#define TESTOBJECT_EXPORT LIBEXP
#else
#define TESTOBJECT_EXPORT LIBIMP
#endif
#else
#define TESTOBJECT_EXPORT
#endif

using namespace apache::geode::client;

namespace PdxTests {

/************************************************************
 *  PdxTypes1V2
 * *********************************************************/
class TESTOBJECT_EXPORT PdxTypes1V2 : public PdxSerializable {
 private:
  static int32_t m_diffInSameFields;
  static int32_t m_diffInExtraFields;
  static bool m_useWeakHashMap;
  std::shared_ptr<PdxUnreadFields> m_pdxUreadFields;

  int32_t m_i1;
  int32_t m_i2;
  int32_t m_i3;
  int32_t m_i4;
  int32_t m_i5;
  int32_t m_i6;

 public:
  PdxTypes1V2();

  virtual ~PdxTypes1V2();

  static void reset(bool useWeakHashMap);

  int getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  std::string toString() const override;

  using PdxSerializable::toData;
  using PdxSerializable::fromData;

  virtual void fromData(std::shared_ptr<PdxReader> pr) override;

  virtual void toData(std::shared_ptr<PdxWriter> pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests::PdxTypes1V";
    return className;
  }

  static PdxSerializable* createDeserializable() { return new PdxTypes1V2(); }
};

/************************************************************
 *  PdxTypes2V2
 * *********************************************************/

class TESTOBJECT_EXPORT PdxTypes2V2 : public PdxSerializable {
 private:
  static int m_diffInSameFields;
  static int m_diffInExtraFields;
  static bool m_useWeakHashMap;
  std::shared_ptr<PdxUnreadFields> m_unreadFields;
  int32_t m_i1;
  int32_t m_i2;
  int32_t m_i3;
  int32_t m_i4;
  int32_t m_i5;
  int32_t m_i6;

 public:
  PdxTypes2V2();

  virtual ~PdxTypes2V2();

  static void reset(bool useWeakHashMap);

  int getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  std::string toString() const override;

  using PdxSerializable::toData;
  using PdxSerializable::fromData;

  virtual void fromData(std::shared_ptr<PdxReader> pr) override;

  virtual void toData(std::shared_ptr<PdxWriter> pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests::PdxTypes2V";
    return className;
  }

  static PdxSerializable* createDeserializable() { return new PdxTypes2V2(); }
};

/************************************************************
 *  PdxTypes3V2
 * *********************************************************/

class TESTOBJECT_EXPORT PdxTypes3V2 : public PdxSerializable {
 private:
  static int m_diffInSameFields;
  static int m_diffInExtraFields;
  static bool m_useWeakHashMap;
  std::shared_ptr<PdxUnreadFields> m_unreadFields;
  int32_t m_i1;
  int32_t m_i2;
  char* m_str1;
  int32_t m_i4;
  int32_t m_i3;
  int32_t m_i6;
  char* m_str3;

 public:
  PdxTypes3V2();

  virtual ~PdxTypes3V2();

  std::string toString() const override;

  static void reset(bool useWeakHashMap);

  int getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  using PdxSerializable::toData;
  using PdxSerializable::fromData;

  virtual void fromData(std::shared_ptr<PdxReader> pr) override;

  virtual void toData(std::shared_ptr<PdxWriter> pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests::PdxTypes3V";
    return className;
  }

  static PdxSerializable* createDeserializable() { return new PdxTypes3V2(); }
};

/************************************************************
 *  PdxTypesR1V2
 **********************************************************/

class TESTOBJECT_EXPORT PdxTypesR1V2 : public PdxSerializable {
 private:
  static int32_t m_diffInSameFields;
  static int32_t m_diffInExtraFields;
  static bool m_useWeakHashMap;
  std::shared_ptr<PdxUnreadFields> m_pdxUnreadFields;
  int32_t m_i1;
  int32_t m_i2;
  int32_t m_i3;
  int32_t m_i4;

  // Fields not in V1.
  int32_t m_i5;
  int32_t m_i6;

 public:
  PdxTypesR1V2();

  virtual ~PdxTypesR1V2();

  std::string toString() const override;

  static void reset(bool useWeakHashMap);

  int getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  using PdxSerializable::toData;
  using PdxSerializable::fromData;

  virtual void fromData(std::shared_ptr<PdxReader> pr) override;

  virtual void toData(std::shared_ptr<PdxWriter> pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests::PdxTypesR1";
    return className;
  }

  static PdxSerializable* createDeserializable() { return new PdxTypesR1V2(); }
};

/************************************************************
 *  PdxTypesR2V2
 * *********************************************************/

class TESTOBJECT_EXPORT PdxTypesR2V2 : public PdxSerializable {
 private:
  static int m_diffInSameFields;
  static int m_diffInExtraFields;
  static bool m_useWeakHashMap;
  std::shared_ptr<PdxUnreadFields> m_pdxunreadFields;
  int32_t m_i1;
  int32_t m_i2;
  int32_t m_i3;
  int32_t m_i4;
  int32_t m_i5;
  int32_t m_i6;

  char* m_str1;

 public:
  PdxTypesR2V2();

  virtual ~PdxTypesR2V2();

  static void reset(bool useWeakHashMap);

  int getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  std::string toString() const override;

  using PdxSerializable::toData;
  using PdxSerializable::fromData;

  virtual void fromData(std::shared_ptr<PdxReader> pr) override;

  virtual void toData(std::shared_ptr<PdxWriter> pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests::PdxTypesR2";
    return className;
  }

  static PdxSerializable* createDeserializable() { return new PdxTypesR2V2(); }
};

/************************************************************
 *  PdxTypesIgnoreUnreadFieldsV2
 * *********************************************************/

class TESTOBJECT_EXPORT PdxTypesIgnoreUnreadFieldsV2 : public PdxSerializable {
 private:
  static int m_diffInSameFields;
  static int m_diffInExtraFields;
  static bool m_useWeakHashMap;
  std::shared_ptr<PdxUnreadFields> m_unreadFields;
  int32_t m_i1;
  int32_t m_i2;
  int32_t m_i3;
  int32_t m_i4;
  mutable int32_t m_i5;
  mutable int32_t m_i6;

 public:
  PdxTypesIgnoreUnreadFieldsV2();

  virtual ~PdxTypesIgnoreUnreadFieldsV2();

  std::string toString() const override;

  static void reset(bool useWeakHashMap);

  void updateMembers();

  int getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  using PdxSerializable::toData;
  using PdxSerializable::fromData;

  virtual void fromData(std::shared_ptr<PdxReader> pr) override;

  virtual void toData(std::shared_ptr<PdxWriter> pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests::PdxTypesIgnoreUnreadFields";
    return className;
  }

  static PdxSerializable* createDeserializable() {
    return new PdxTypesIgnoreUnreadFieldsV2();
  }
};


/************************************************************
 *  PdxVersionedV2
 * *********************************************************/

class TESTOBJECT_EXPORT PdxVersionedV2 : public PdxSerializable {
 private:
  char m_char;
  bool m_bool;
  int8_t m_byte;
  int16_t m_int16;
  int32_t m_int32;
  int64_t m_long;
  float m_float;
  double m_double;
  char* m_string;
  bool* m_boolArray;
  char* m_charArray;
  std::shared_ptr<CacheableDate> m_dateTime;
  int16_t* m_int16Array;
  int32_t* m_int32Array;
  int64_t* m_longArray;
  float* m_floatArray;
  double* m_doubleArray;

  // IDictionary<object, object> m_map;
  // List<object> m_list;

  int32_t boolArrayLen;
  int32_t byteArrayLen;
  int32_t shortArrayLen;
  int32_t intArrayLen;
  int32_t longArrayLen;
  int32_t doubleArrayLen;
  int32_t floatArrayLen;
  int32_t strLenArray;

 public:
  PdxVersionedV2();

  PdxVersionedV2(int32_t size);

  virtual ~PdxVersionedV2();

  void init(int32_t size);

  std::string toString() const override;

  using PdxSerializable::toData;
  using PdxSerializable::fromData;

  virtual void fromData(std::shared_ptr<PdxReader> pr) override;

  virtual void toData(std::shared_ptr<PdxWriter> pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests::PdxVersioned";
    return className;
  }

  static PdxSerializable* createDeserializable() {
    return new PdxVersionedV2();
  }
};

/************************************************************
 *  TestKey
 * *********************************************************/

class TESTOBJECT_EXPORT TestKeyV2 {
 public:
  char* _id;

 public:
  TestKeyV2();

  TestKeyV2(char* id);
};

/************************************************************
 *  TestDiffTypePdxSV2
 * *********************************************************/
// TODO: Enable it once the PdxSerializer is done

class TESTOBJECT_EXPORT TestDiffTypePdxSV2 {
 public:
  char* _id;
  char* _name;
  int _count;

 public:
  TestDiffTypePdxSV2();

  TestDiffTypePdxSV2(bool init);

  bool equals(TestDiffTypePdxSV2* obj);
};

/************************************************************
 *  TestPdxSerializerForV2
 * *********************************************************/
static const char* V2CLASSNAME3 = "PdxTests.TestKey";
static const char* V2CLASSNAME4 = "PdxTests.TestDiffTypePdxS";

class TestPdxSerializerForV2 : public PdxSerializer {
 public:
  static void deallocate(void* testObject, const std::string& className) {
    // ASSERT(strcmp(className, V2CLASSNAME3) == 0 || strcmp(className,
    // V2CLASSNAME4) == 0 , "Unexpected classname in deallocate()");
    LOGINFO("TestPdxSerializerForV2::deallocate called");
    if (className == V2CLASSNAME3) {
      PdxTests::TestKeyV2* tkv1 =
          reinterpret_cast<PdxTests::TestKeyV2*>(testObject);
      delete tkv1;
    } else if (className == V2CLASSNAME4) {
      PdxTests::TestDiffTypePdxSV2* dtpv1 =
          reinterpret_cast<PdxTests::TestDiffTypePdxSV2*>(testObject);
      delete dtpv1;
    } else {
      LOGINFO("TestPdxSerializerForV1::deallocate Invalid Class Name");
    }
  }

  static uint32_t objectSize(void* testObject, const std::string& className) {
    // ASSERT(strcmp(className, V2CLASSNAME3) == 0 || strcmp(className,
    // V2CLASSNAME4) == 0, "Unexpected classname in objectSize()");
    LOGINFO("TestPdxSerializer::objectSize called");
    return 12345;  // dummy value
  }

  virtual UserDeallocator getDeallocator(
      const std::string& className) override {
    // ASSERT(strcmp(className, V2CLASSNAME3) == 0 || strcmp(className,
    // V2CLASSNAME4) == 0, "Unexpected classname in getDeallocator");
    return deallocate;
  }

  virtual UserObjectSizer getObjectSizer(
      const std::string& className) override {
    // ASSERT(strcmp(className, V2CLASSNAME3) == 0 || strcmp(className,
    // V2CLASSNAME4) == 0, "Unexpected classname in getObjectSizer");
    return objectSize;
  }

  void* fromDataForTestKeyV2(std::shared_ptr<PdxReader> pr) {
    try {
      PdxTests::TestKeyV2* tkv1 = new PdxTests::TestKeyV2;
      tkv1->_id = pr->readString("_id");
      return (void*)tkv1;
    } catch (...) {
      return NULL;
    }
  }

  bool toDataForTestKeyV2(void* testObject, std::shared_ptr<PdxWriter> pw) {
    try {
      PdxTests::TestKeyV2* tkv1 =
          reinterpret_cast<PdxTests::TestKeyV2*>(testObject);
      pw->writeString("_id", tkv1->_id);

      return true;
    } catch (...) {
      return false;
    }
  }

  void* fromDataForTestDiffTypePdxSV2(std::shared_ptr<PdxReader> pr) {
    try {
      PdxTests::TestDiffTypePdxSV2* dtpv1 = new PdxTests::TestDiffTypePdxSV2;
      dtpv1->_id = pr->readString("_id");
      dtpv1->_name = pr->readString("_name");
      dtpv1->_count = pr->readInt("_count");
      return (void*)dtpv1;
    } catch (...) {
      return NULL;
    }
  }

  bool toDataForTestDiffTypePdxSV2(void* testObject, std::shared_ptr<PdxWriter> pw) {
    try {
      PdxTests::TestDiffTypePdxSV2* dtpv1 =
          reinterpret_cast<PdxTests::TestDiffTypePdxSV2*>(testObject);
      pw->writeString("_id", dtpv1->_id);
      pw->markIdentityField("_id");
      pw->writeString("_name", dtpv1->_name);
      pw->writeInt("_count", dtpv1->_count);

      return true;
    } catch (...) {
      return false;
    }
  }

  void* fromData(const std::string& className,
                 std::shared_ptr<PdxReader> pr) override {
    // ASSERT(strcmp(className, V2CLASSNAME3) == 0 || strcmp(className,
    // V2CLASSNAME4) == 0, "Unexpected classname in fromData");

    if (className == V2CLASSNAME4) {
      return fromDataForTestDiffTypePdxSV2(pr);

    } else if (className == V2CLASSNAME3) {
      return fromDataForTestKeyV2(pr);

    } else {
      LOGINFO("TestPdxSerializerForV2::fromdata() Invalid Class Name");
      return NULL;
    }
  }

  bool toData(void* testObject, const std::string& className,
              std::shared_ptr<PdxWriter> pw) override {
    // ASSERT(strcmp(className, V2CLASSNAME3) == 0 || strcmp(className,
    // V2CLASSNAME4) == 0, "Unexpected classname in toData");

    if (className == V2CLASSNAME4) {
      return toDataForTestDiffTypePdxSV2(testObject, pw);

    } else if (className == V2CLASSNAME3) {
      return toDataForTestKeyV2(testObject, pw);

    } else {
      LOGINFO("TestPdxSerializerForV1::fromdata() Invalid Class Name");
      return false;
    }
  }
};

/************************************************************
 *  TestEqualsV1
 * *********************************************************/
/*
class TESTOBJECT_EXPORT TestEqualsV1 : public PdxSerializable {

private:

        int32_t i1;
        int32_t i2;
        char* s1;
        char** sArr;
        int32_t* intArr;

public:

        TestEqualsV1();

        std::string toString() const;

    using PdxSerializable::toData;
    using PdxSerializable::fromData;

  virtual void fromData(std::shared_ptr<PdxReader> pr);

  virtual void toData(std::shared_ptr<PdxWriter> pw) ;

  const char* getClassName()const {
    return "PdxTests::TestEquals";
  }

  static PdxSerializable* createDeserializable() {
    return new TestEqualsV1();
  }
};
*/

} /* namespace PdxTests */

#endif  // GEODE_TESTOBJECT_PDXCLASSV2_H_
