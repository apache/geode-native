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

#ifndef GEODE_TESTOBJECT_BATCHOBJECT_H_
#define GEODE_TESTOBJECT_BATCHOBJECT_H_

#include <cinttypes>
#include <string>

#include <geode/CacheableBuiltins.hpp>

#include "TimestampedObject.hpp"
#include "testobject_export.h"

namespace testobject {

using apache::geode::client::CacheableBytes;

/**
 * @brief User class for testing the cq functionality.
 */
class TESTOBJECT_EXPORT BatchObject : public TimestampedObject {
 private:
  int32_t index;
  uint64_t timestamp;
  int32_t batch;
  std::shared_ptr<CacheableBytes> byteArray;

  inline size_t getObjectSize(const std::shared_ptr<Serializable>& obj) const {
    return (obj == nullptr ? 0 : obj->objectSize());
  }

 public:
  BatchObject() : index(0), timestamp(0), batch(0), byteArray(nullptr) {}
  BatchObject(int32_t anIndex, int32_t batchSize, int32_t size);
  ~BatchObject() override = default;
  virtual void toData(apache::geode::client::DataOutput& output) const override;
  virtual void fromData(apache::geode::client::DataInput& input) override;
  std::string toString() const override;

  virtual size_t objectSize() const override {
    auto objectSize = sizeof(BatchObject);
    return objectSize;
  }

  uint64_t getTimestamp() override { return timestamp; }
  int getIndex() { return index; }
  int getBatch() { return batch; }
  void resetTimestamp() override {
    timestamp = std::chrono::system_clock::now().time_since_epoch().count();
  }

  static apache::geode::client::Serializable* createDeserializable() {
    return new BatchObject();
  }
};

}  // namespace testobject

#endif  // GEODE_TESTOBJECT_BATCHOBJECT_H_
