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

#ifndef GEODE_TESTOBJECT_DELTAPSTOBJECT_H_
#define GEODE_TESTOBJECT_DELTAPSTOBJECT_H_

#include <cinttypes>
#include <string>

#include <geode/Delta.hpp>

#include "TimestampedObject.hpp"
#include "testobject/PSTObject.hpp"
#include "testobject_export.h"

namespace testobject {

using apache::geode::client::Delta;

class TESTOBJECT_EXPORT DeltaPSTObject : public DataSerializable, public Delta {
 private:
  uint64_t timestamp;
  int32_t field1;
  int8_t field2;
  std::shared_ptr<CacheableBytes> valueData;

 public:
  DeltaPSTObject() : Delta(), timestamp(0), valueData(nullptr) {}
  DeltaPSTObject(int size, bool encodeKey);
  ~DeltaPSTObject() noexcept override {}
  void toData(apache::geode::client::DataOutput& output) const override;
  void fromData(apache::geode::client::DataInput& input) override;
  void fromDelta(DataInput& input) override;
  void toDelta(DataOutput& output) const override;
  std::string toString() const override;
  bool hasDelta() const override { return true; }

  size_t objectSize() const override {
    auto objectSize = sizeof(DeltaPSTObject);
    return objectSize;
  }
  void incrementField1() { ++field1; }

  void update() {
    incrementField1();
    resetTimestamp();
  }

  uint64_t getTimestamp() { return timestamp; }

  void resetTimestamp() {
    timestamp = std::chrono::system_clock::now().time_since_epoch().count();
  }

  inline std::shared_ptr<Delta> clone() const override { return nullptr; }

  static Serializable* createDeserializable() { return new DeltaPSTObject(); }
};
}  // namespace testobject

#endif  // GEODE_TESTOBJECT_DELTAPSTOBJECT_H_
