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
#include <ace/Condition_T.h>
#include <ace/OS.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Task.h>

#include <geode/DataSerializable.hpp>
#include <geode/Delta.hpp>

#include "TestObject1.hpp"
#include "testobject_export.h"

namespace testobject {

using apache::geode::client::CacheableBytes;
using apache::geode::client::CacheableString;
using apache::geode::client::DataInput;
using apache::geode::client::DataOutput;
using apache::geode::client::DataSerializable;
using apache::geode::client::Delta;
using apache::geode::client::Serializable;

class TESTOBJECT_EXPORT DeltaTestImpl : public DataSerializable, public Delta {
 private:
  static uint8_t INT_MASK;
  static uint8_t STR_MASK;
  static uint8_t DOUBLE_MASK;
  static uint8_t BYTE_ARR_MASK;
  static uint8_t TEST_OBJ_MASK;
  static uint8_t COMPLETE_MASK;

  int32_t intVar;                           // 0000 0001
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
  DeltaTestImpl(const DeltaTestImpl& rhs);
  ~DeltaTestImpl() noexcept override {}

  void fromData(DataInput& input) override;

  void toData(DataOutput& output) const override;

  void fromDelta(DataInput& input) override;

  void toDelta(DataOutput& output) const override;

  bool hasDelta() const override { return m_hasDelta; }

  inline std::shared_ptr<Delta> clone() const override {
    return std::make_shared<DeltaTestImpl>(*this);
  }

  static std::shared_ptr<Serializable> create() {
    return std::make_shared<DeltaTestImpl>();
  }

  int32_t getIntVar() const { return intVar; }
  std::shared_ptr<CacheableString> getStr() const { return str; }
  double getDoubleVar() const { return doubleVar; }
  std::shared_ptr<CacheableBytes> getByteArr() const { return byteArr; }
  std::shared_ptr<TestObject1> getTestObj() const { return testObj; }

  int64_t getFromDeltaCounter() const { return fromDeltaCounter; }
  int64_t getToDeltaCounter() const { return toDeltaCounter; }

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
  std::string toString() const override;
};

}  // namespace testobject

#endif  // GEODE_TESTOBJECT_DELTATESTIMPL_H_
