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

#ifndef GEODE_TESTOBJECT_DELTATESTIMPL_H_
#define GEODE_TESTOBJECT_DELTATESTIMPL_H_

#include <ace/ACE.h>
#include <ace/OS.h>
#include <ace/Task.h>
#include <ace/Condition_T.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Time_Value.h>
#include "TestObject1.hpp"
#include <geode/Delta.hpp>

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

namespace testobject {

class TESTOBJECT_EXPORT DeltaTestImpl : public Cacheable, public Delta {
 private:
  static uint8_t INT_MASK;
  static uint8_t STR_MASK;
  static uint8_t DOUBLE_MASK;
  static uint8_t BYTE_ARR_MASK;
  static uint8_t TEST_OBJ_MASK;
  static uint8_t COMPLETE_MASK;

  int32_t intVar;             // 0000 0001
  std::shared_ptr<CacheableString> str;     // 0000 0010
  double doubleVar;                         // 0000 0100
  std::shared_ptr<CacheableBytes> byteArr;  // 0000 1000
  std::shared_ptr<TestObject1> testObj;     // 0001 0000

  bool m_hasDelta;
  uint8_t deltaBits;
  mutable int64_t toDeltaCounter;
  int64_t fromDeltaCounter;
  ACE_Recursive_Thread_Mutex m_lock;

 public:
  DeltaTestImpl();
  DeltaTestImpl(int intValue, std::shared_ptr<CacheableString> strptr);
  DeltaTestImpl(std::shared_ptr<DeltaTestImpl> rhs);
  void fromData(DataInput& input);
  void toData(DataOutput& output) const;

  void fromDelta(DataInput& input);
  void toDelta(DataOutput& output) const;
  bool hasDelta() { return m_hasDelta; }

  int32_t classId() const { return 30; }

  uint32_t objectSize() const { return 0; }

  std::shared_ptr<Delta> clone() {
    return std::shared_ptr<Delta>(
        std::dynamic_pointer_cast<DeltaTestImpl>(shared_from_this()));
  }

  static Serializable* create() { return new DeltaTestImpl(); }

  int32_t getIntVar() { return intVar; }
  std::shared_ptr<CacheableString> getStr() { return str; }
  double getDoubleVar() { return doubleVar; }
  std::shared_ptr<CacheableBytes> getByteArr() { return byteArr; }
  std::shared_ptr<TestObject1> getTestObj() { return testObj; }

  int64_t getFromDeltaCounter() { return fromDeltaCounter; }
  int64_t getToDeltaCounter() { return toDeltaCounter; }

  void setIntVar(int32_t value) {
    intVar = value;
    deltaBits |= INT_MASK;
    m_hasDelta = true;
  }
  void setStr(char* str1) { str = CacheableString::create(str1); }
  void setDoubleVar(double value) { doubleVar = value; }
  void setByteArr(std::shared_ptr<CacheableBytes> value) { byteArr = value; }
  void setTestObj(std::shared_ptr<TestObject1> value) { testObj = value; }

  void setDelta(bool value) { m_hasDelta = value; }

  void setIntVarFlag() { deltaBits = deltaBits | INT_MASK; }
  void setStrFlag() { deltaBits = deltaBits | STR_MASK; }
  void setDoubleVarFlag() { deltaBits = deltaBits | DOUBLE_MASK; }
  void setByteArrFlag() { deltaBits = deltaBits | BYTE_ARR_MASK; }
  void setTestObjFlag() { deltaBits = deltaBits | TEST_OBJ_MASK; }
  std::shared_ptr<CacheableString> toString() const;
};

}  // namespace testobject

#endif  // GEODE_TESTOBJECT_DELTATESTIMPL_H_
