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

#ifndef GEODE_TESTOBJECT_FASTASSETACCOUNT_H_
#define GEODE_TESTOBJECT_FASTASSETACCOUNT_H_

#include <string>

#include <geode/CacheableBuiltins.hpp>

#include "TimestampedObject.hpp"
#include "testobject_export.h"

namespace testobject {

using apache::geode::client::CacheableHashMap;
using apache::geode::client::CacheableString;
using apache::geode::client::DataInput;
using apache::geode::client::DataOutput;

/**
 * @brief User class for testing the query functionality.
 */
class TESTOBJECT_EXPORT FastAssetAccount : public TimestampedObject {
 protected:
  bool encodeTimestamp;
  int32_t acctId;
  std::shared_ptr<CacheableString> customerName;
  double netWorth;
  std::shared_ptr<CacheableHashMap> assets;
  uint64_t timestamp;

  inline size_t getObjectSize(const std::shared_ptr<Serializable>& obj) const {
    return (obj == nullptr ? 0 : obj->objectSize());
  }

 public:
  FastAssetAccount()
      : encodeTimestamp(0),
        acctId(0),
        customerName(nullptr),
        netWorth(0.0),
        assets(nullptr),
        timestamp(0) {}
  FastAssetAccount(int index, bool encodeTimestp, int maxVal, int asstSize = 0);
  ~FastAssetAccount() override = default;
  void toData(DataOutput& output) const override;
  void fromData(DataInput& input) override;
  std::string toString() const override;

  size_t objectSize() const override {
    auto objectSize = sizeof(FastAssetAccount);
    return objectSize;
  }

  int getAcctId() { return acctId; }

  std::shared_ptr<CacheableString> getCustomerName() { return customerName; }

  double getNetWorth() { return netWorth; }

  void incrementNetWorth() { ++netWorth; }

  std::shared_ptr<CacheableHashMap> getAssets() { return assets; }
  int getIndex() { return acctId; }
  uint64_t getTimestamp() override {
    if (encodeTimestamp) {
      return timestamp;
    } else {
      return 0;
    }
  }

  void resetTimestamp() override {
    timestamp =
        encodeTimestamp
            ? std::chrono::system_clock::now().time_since_epoch().count()
            : 0;
  }

  void update() {
    incrementNetWorth();
    if (encodeTimestamp) {
      resetTimestamp();
    }
  }
  static apache::geode::client::Serializable* createDeserializable() {
    return new FastAssetAccount();
  }
};

}  // namespace testobject

#endif  // GEODE_TESTOBJECT_FASTASSETACCOUNT_H_
