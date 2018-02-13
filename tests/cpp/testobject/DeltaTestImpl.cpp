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
 *DeltaTestImpl.cpp
 *
 *Created on: Jul 14, 2009
 *Author: abhaware
 */
#include "DeltaTestImpl.hpp"
#include <ace/Guard_T.h>
using namespace testobject;

uint8_t DeltaTestImpl::INT_MASK = 0x1;
uint8_t DeltaTestImpl::STR_MASK = 0x2;
uint8_t DeltaTestImpl::DOUBLE_MASK = 0x4;
uint8_t DeltaTestImpl::BYTE_ARR_MASK = 0x8;
uint8_t DeltaTestImpl::TEST_OBJ_MASK = 0x10;
uint8_t DeltaTestImpl::COMPLETE_MASK = 0x1F;

DeltaTestImpl::DeltaTestImpl() : Delta(nullptr) {
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
    : Delta(nullptr),
      intVar(intValue),
      str(strptr),
      doubleVar(0),
      toDeltaCounter(0),
      fromDeltaCounter(0) {}
DeltaTestImpl::DeltaTestImpl(const DeltaTestImpl& rhs) : Delta(nullptr) {
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
  str = std::static_pointer_cast<CacheableString>(input.readObject());
  doubleVar = input.readDouble();
  byteArr = std::static_pointer_cast<CacheableBytes>(input.readObject());
  testObj = std::static_pointer_cast<TestObject1>(input.readObject());
}

void DeltaTestImpl::toData(DataOutput& output) const {
  output.writeInt(intVar);
  output.writeObject(str);
  output.writeDouble(doubleVar);
  output.writeObject(byteArr);
  output.writeObject(testObj);
}

void DeltaTestImpl::toDelta(DataOutput& output) const {
  {
    ACE_Recursive_Thread_Mutex* lock =
        const_cast<ACE_Recursive_Thread_Mutex*>(&m_lock);
    ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(*lock);
    toDeltaCounter++;
  }
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
  {
    ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_lock);
    fromDeltaCounter++;
  }
  deltaBits = input.read();
  if ((deltaBits & INT_MASK) == INT_MASK) {
    intVar = input.readInt32();
  }
  if ((deltaBits & STR_MASK) == STR_MASK) {
    str = std::static_pointer_cast<CacheableString>(input.readObject());
  }
  if ((deltaBits & DOUBLE_MASK) == DOUBLE_MASK) {
    doubleVar = input.readDouble();
  }
  if ((deltaBits & BYTE_ARR_MASK) == BYTE_ARR_MASK) {
    byteArr = std::static_pointer_cast<CacheableBytes>(input.readObject());
    /*
        uint8_t* bytes;
        int32_t len;
        input.readBytes( &bytes, &len );
        byteArr = CacheableBytes::create( bytes, len );
        delete bytes;
    */
  }
  if ((deltaBits & TEST_OBJ_MASK) == TEST_OBJ_MASK) {
    testObj = std::static_pointer_cast<TestObject1>(input.readObject());
  }
}

std::string DeltaTestImpl::toString() const {
  char buf[102500];
  sprintf(buf, "DeltaTestImpl[hasDelta=%d int=%d double=%f str=%s \n",
          m_hasDelta, intVar, doubleVar, str->toString().c_str());
  return buf;
}
