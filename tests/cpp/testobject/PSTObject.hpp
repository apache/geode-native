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

#ifndef GEODE_TESTOBJECT_PSTOBJECT_H_
#define GEODE_TESTOBJECT_PSTOBJECT_H_

#include <cinttypes>
#include <string>

#include <geode/CacheableBuiltins.hpp>

#include "TimestampedObject.hpp"
#include "testobject_export.h"

namespace testobject {

using apache::geode::client::CacheableBytes;
using apache::geode::client::DataInput;
using apache::geode::client::DataOutput;

/**
 * @brief User class for testing the put functionality for object.
 */
class TESTOBJECT_EXPORT PSTObject : public TimestampedObject {
 protected:
  uint64_t timestamp;
  int32_t field1;
  int8_t field2;
  std::shared_ptr<CacheableBytes> valueData;

  inline size_t getObjectSize(const std::shared_ptr<Serializable>& obj) const {
    return (obj == nullptr ? 0 : obj->objectSize());
  }

 public:
  PSTObject() : timestamp(0), valueData(nullptr) {}
  PSTObject(int size, bool encodeKey);
  ~PSTObject() override = default;
  void toData(DataOutput& output) const override;
  void fromData(DataInput& input) override;
  std::string toString() const override;

  size_t objectSize() const override {
    auto objectSize = sizeof(PSTObject);
    objectSize += getObjectSize(valueData);
    return objectSize;
  }

  uint64_t getTimestamp() override { return timestamp; }
  void resetTimestamp() override {
    timestamp = std::chrono::system_clock::now().time_since_epoch().count();
  }

  static apache::geode::client::Serializable* createDeserializable() {
    return new PSTObject();
  }
};

}  // namespace testobject

#endif  // GEODE_TESTOBJECT_PSTOBJECT_H_
