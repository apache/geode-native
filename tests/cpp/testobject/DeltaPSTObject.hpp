#pragma once

#ifndef GEODE_TESTOBJECT_DELTAPSTOBJECT_H_
#define GEODE_TESTOBJECT_DELTAPSTOBJECT_H_

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
 * @brief User class for testing the put functionality for object.
 */

#include <geode/GeodeCppCache.hpp>
#include <string>
#include "fwklib/Timer.hpp"
#include "fwklib/FrameworkTest.hpp"
#include "TimestampedObject.hpp"
#include "testobject/PSTObject.hpp"
#include <ace/ACE.h>
#include <ace/OS.h>
#include <ace/Time_Value.h>

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
using namespace testframework;
namespace testobject {
class TESTOBJECT_EXPORT DeltaPSTObject : public Cacheable, public Delta {
 private:
  uint64_t timestamp;
  int32_t field1;
  int8_t field2;
  CacheableBytesPtr valueData;
  std::shared_ptr<DeltaPSTObject> shared_from_this() {
    return std::static_pointer_cast<DeltaPSTObject>(
        Serializable::shared_from_this());
  }

 public:
  DeltaPSTObject() : Delta(nullptr), timestamp(0), valueData(nullptr) {}
  DeltaPSTObject(int size, bool encodeKey, bool encodeTimestamp);
  virtual ~DeltaPSTObject() {}
  void toData(apache::geode::client::DataOutput& output) const;
  apache::geode::client::Serializable* fromData(
      apache::geode::client::DataInput& input);
  void fromDelta(DataInput& input);
  void toDelta(DataOutput& output) const;
  CacheableStringPtr toString() const;
  bool hasDelta() { return true; }
  int32_t classId() const { return 42; }

  uint32_t objectSize() const {
    uint32_t objectSize = sizeof(DeltaPSTObject);
    return objectSize;
  }
  void incrementField1() { ++field1; }

  void update() {
    incrementField1();
    resetTimestamp();
  }
  uint64_t getTimestamp() { return timestamp; }
  void resetTimestamp() {
    ACE_Time_Value startTime;
    startTime = ACE_OS::gettimeofday();
    ACE_UINT64 tusec;
    startTime.to_usec(tusec);
    timestamp = tusec * 1000;
  }
  DeltaPtr clone() {
    // TODO shared_ptr - this isn't actually cloning.
    return shared_from_this();
  }

  static Serializable* createDeserializable() { return new DeltaPSTObject(); }
};
typedef std::shared_ptr<DeltaPSTObject> DeltaPSTObjectPtr;
}  // namespace testobject

#endif  // GEODE_TESTOBJECT_DELTAPSTOBJECT_H_
