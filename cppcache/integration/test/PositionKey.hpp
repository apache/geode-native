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

#ifndef POSITIONKEY_H_
#define POSITIONKEY_H_

#include <string>

#include <geode/CacheableString.hpp>
#include <geode/DataSerializable.hpp>

namespace DataSerializableTest {

using apache::geode::client::CacheableKey;
using apache::geode::client::DataInput;
using apache::geode::client::DataOutput;
using apache::geode::client::DataSerializable;

class PositionKey : public DataSerializable, public CacheableKey {
 private:
  int64_t positionId_;

 public:
  PositionKey() = default;
  explicit PositionKey(int64_t positionId) : positionId_(positionId) {}
  ~PositionKey() override = default;

  bool operator==(const CacheableKey& other) const override;
  int32_t hashcode() const override;

  void toData(DataOutput& output) const override;
  void fromData(DataInput& input) override;

  int64_t getPositionId() const { return positionId_; }
  static std::shared_ptr<Serializable> createDeserializable() {
    return std::make_shared<PositionKey>();
  }
};

}  // namespace DataSerializableTest

#endif  // POSITIONKEY_H_
