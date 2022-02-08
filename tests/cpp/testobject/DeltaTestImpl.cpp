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

#include "DeltaTestImpl.hpp"

#include <sstream>

namespace testobject {

uint8_t DeltaTestImpl::INT_MASK = 0x1;
uint8_t DeltaTestImpl::STR_MASK = 0x2;
uint8_t DeltaTestImpl::DOUBLE_MASK = 0x4;
uint8_t DeltaTestImpl::BYTE_ARR_MASK = 0x8;
uint8_t DeltaTestImpl::TEST_OBJ_MASK = 0x10;
uint8_t DeltaTestImpl::COMPLETE_MASK = 0x1F;

DeltaTestImpl::DeltaTestImpl() : Delta() {
  intVar = 1;
  str = CacheableString::create("test");
  doubleVar = 1.1;
  uint8_t byte = 'A';
  byteArr = CacheableBytes::create(std::vector<int8_t>(&byte, &byte + 1));
  testObj = nullptr;
  m_hasDelta = false;
  deltaBits = 0;
  toDeltaCounter = 0;
  fromDeltaCounter = 0;
}

DeltaTestImpl::DeltaTestImpl(int intValue,
                             std::shared_ptr<CacheableString> strptr)
    : Delta(),
      intVar(intValue),
      str(strptr),
      doubleVar(0),
      toDeltaCounter(0),
      fromDeltaCounter(0) {}

DeltaTestImpl::DeltaTestImpl(const DeltaTestImpl& rhs) : Delta() {
  intVar = rhs.intVar;
  str = CacheableString::create(rhs.str->value().c_str());
  doubleVar = rhs.doubleVar;
  byteArr =
      (rhs.byteArr == nullptr ? nullptr
                              : CacheableBytes::create(rhs.byteArr->value()));
  testObj = (rhs.testObj == nullptr
                 ? nullptr
                 : std::shared_ptr<TestObject1>(new TestObject1(*rhs.testObj)));
  toDeltaCounter = rhs.getToDeltaCounter();
  fromDeltaCounter = rhs.getFromDeltaCounter();
}

void DeltaTestImpl::fromData(DataInput& input) {
  intVar = input.readInt32();
  str = std::dynamic_pointer_cast<CacheableString>(input.readObject());
  doubleVar = input.readDouble();
  byteArr = std::dynamic_pointer_cast<CacheableBytes>(input.readObject());
  testObj = std::dynamic_pointer_cast<TestObject1>(input.readObject());
}

void DeltaTestImpl::toData(DataOutput& output) const {
  output.writeInt(intVar);
  output.writeObject(str);
  output.writeDouble(doubleVar);
  output.writeObject(byteArr);
  output.writeObject(testObj);
}

void DeltaTestImpl::toDelta(DataOutput& output) const {
  ++toDeltaCounter;

  output.write(deltaBits);
  if ((deltaBits & INT_MASK) == INT_MASK) {
    output.writeInt(intVar);
  }
  if ((deltaBits & STR_MASK) == STR_MASK) {
    output.writeObject(str);
  }
  if ((deltaBits & DOUBLE_MASK) == DOUBLE_MASK) {
    output.writeDouble(doubleVar);
  }
  if ((deltaBits & BYTE_ARR_MASK) == BYTE_ARR_MASK) {
    output.writeObject(byteArr);
  }
  if ((deltaBits & TEST_OBJ_MASK) == TEST_OBJ_MASK) {
    output.writeObject(testObj);
  }
}

void DeltaTestImpl::fromDelta(DataInput& input) {
  ++fromDeltaCounter;
  deltaBits = input.read();

  if ((deltaBits & INT_MASK) == INT_MASK) {
    intVar = input.readInt32();
  }
  if ((deltaBits & STR_MASK) == STR_MASK) {
    str = std::dynamic_pointer_cast<CacheableString>(input.readObject());
  }
  if ((deltaBits & DOUBLE_MASK) == DOUBLE_MASK) {
    doubleVar = input.readDouble();
  }
  if ((deltaBits & BYTE_ARR_MASK) == BYTE_ARR_MASK) {
    byteArr = std::dynamic_pointer_cast<CacheableBytes>(input.readObject());
  }
  if ((deltaBits & TEST_OBJ_MASK) == TEST_OBJ_MASK) {
    testObj = std::dynamic_pointer_cast<TestObject1>(input.readObject());
  }
}

std::string DeltaTestImpl::toString() const {
  std::stringstream strm;

  strm << "DeltaTestImpl[hasDelta=" << m_hasDelta << " int = " << intVar
       << " double = " << doubleVar << " str = " << str->toString() << "\n";
  return strm.str();
}

}  // namespace testobject
