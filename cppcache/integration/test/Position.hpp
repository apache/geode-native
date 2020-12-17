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

#ifndef POSITION_H
#define POSITION_H

/*
 * @brief User class for testing the put functionality for object.
 */

#include <string>

#include <geode/CacheableString.hpp>
#include <geode/DataSerializable.hpp>

namespace DataSerializableTest {

using apache::geode::client::CacheableString;
using apache::geode::client::DataInput;
using apache::geode::client::DataOutput;
using apache::geode::client::DataSerializable;

class Position : public DataSerializable {
 private:
  int64_t avg20DaysVol;
  std::string bondRating;
  double convRatio;
  std::string country;
  double valueGain;
  int64_t industry;
  int64_t issuer;
  double mktValue;
  double qty;
  std::string secId;
  std::string secLinks;
  std::wstring secType;
  int32_t sharesOutstanding;
  std::string underlyer;
  int64_t volatility;
  int32_t pid;

 public:
  static int32_t cnt;

  Position();
  Position(std::string id, int32_t out);
  ~Position() override = default;
  void toData(DataOutput& output) const override;
  void fromData(DataInput& input) override;

  static void resetCounter() { cnt = 0; }
  std::string getSecId() { return secId; }
  int32_t getId() { return pid; }
  int32_t getSharesOutstanding() { return sharesOutstanding; }
  static std::shared_ptr<Serializable> createDeserializable() {
    return std::make_shared<Position>();
  }
};

}  // namespace DataSerializableTest

#endif  // POSITION_H
