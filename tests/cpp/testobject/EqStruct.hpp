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

#ifndef GEODE_TESTOBJECT_EQSTRUCT_H_
#define GEODE_TESTOBJECT_EQSTRUCT_H_

#include <cinttypes>
#include <fwklib/FwkException.hpp>
#include <string>

#include "TimestampedObject.hpp"
#include "testobject_export.h"

namespace testobject {

using apache::geode::client::testframework::FwkException;

/**
 * @brief User class for testing the put functionality for object.
 */
class TESTOBJECT_EXPORT EqStruct : public TimestampedObject {
  int myIndex;
  std::string state;
  uint64_t timestamp;
  double executedPriceSum;
  int cxlQty;
  int isSyntheticOrder;
  int64_t availQty;
  double positionQty;
  int isRestricted;
  std::string demandInd;
  std::string side;
  int orderQty;
  double price;
  std::string ordType;
  double stopPx;
  std::string senderCompID;
  std::string tarCompID;
  std::string tarSubID;
  std::string handlInst;
  std::string orderID;
  std::string timeInForce;
  std::string clOrdID;
  std::string orderCapacity;
  int cumQty;
  std::string symbol;
  std::string symbolSfx;
  std::string execInst;
  std::string oldClOrdID;
  double pegDifference;
  std::string discretionInst;
  double discretionOffset;
  std::string financeInd;
  std::string securityID;
  std::string targetCompID;
  std::string targetSubID;
  int isDoneForDay;
  int revisionSeqNum;
  int replaceQty;
  int64_t usedClientAvailability;
  std::string clientAvailabilityKey;
  int isIrregularSettlmnt;

  std::string var1;
  std::string var2;
  std::string var3;
  std::string var4;
  std::string var5;
  std::string var6;
  std::string var7;
  std::string var8;
  std::string var9;

  inline size_t getObjectSize(const std::shared_ptr<Serializable>& obj) const {
    return (obj == nullptr ? 0 : obj->objectSize());
  }

 public:
  EqStruct() {}
  explicit EqStruct(int index);
  ~EqStruct() override = default;
  virtual void toData(apache::geode::client::DataOutput& output) const override;
  virtual void fromData(apache::geode::client::DataInput& input) override;
  std::string toString() const override;

  virtual size_t objectSize() const override {
    auto objectSize = sizeof(EqStruct);
    return objectSize;
  }

  int getIndex() { return myIndex; }
  void validate(int index) {
    int encodedIndex = myIndex;
    if (encodedIndex != index) {
      char logmsg[2048];
      sprintf(logmsg, "Expected index %d , got %d.\n", index, encodedIndex);
      throw FwkException(logmsg);
    }
  }
  void update() {
    var1 = "abcdefghi";
    cumQty = 39;
    usedClientAvailability = 1310447848683LL;
    discretionOffset = 12.3456789;
    resetTimestamp();
  }

  uint64_t getTimestamp() override { return timestamp; }
  void resetTimestamp() override {
    timestamp = std::chrono::system_clock::now().time_since_epoch().count();
  }

  static apache::geode::client::Serializable* createDeserializable() {
    return new EqStruct();
  }
};

}  // namespace testobject

#endif  // GEODE_TESTOBJECT_EQSTRUCT_H_
