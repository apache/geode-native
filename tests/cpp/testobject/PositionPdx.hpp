#pragma once

#ifndef GEODE_TESTOBJECT_POSITIONPDX_H_
#define GEODE_TESTOBJECT_POSITIONPDX_H_

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

#include <string>

#include <geode/PdxReader.hpp>
#include <geode/PdxSerializable.hpp>
#include <geode/PdxWriter.hpp>

#include "testobject_export.h"

namespace testobject {

using apache::geode::client::PdxReader;
using apache::geode::client::PdxSerializable;
using apache::geode::client::PdxWriter;

class TESTOBJECT_EXPORT PositionPdx : public PdxSerializable {
 private:
  int64_t avg20DaysVol;
  std::string bondRating;
  double convRatio;
  std::string country;
  double delta;
  int64_t industry;
  int64_t issuer;
  double mktValue;
  double qty;
  std::string secId;
  std::string secLinks;
  // wchar_t* secType;
  // wchar_t* secType;
  std::string secType;
  int32_t sharesOutstanding;
  std::string underlyer;
  int64_t volatility;

  int32_t pid;

 public:
  static int32_t cnt;

  PositionPdx();
  PositionPdx(const char* id, int32_t out);
  // This constructor is just for some internal data validation test
  explicit PositionPdx(int32_t iForExactVal);
  ~PositionPdx() override = default;

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  void toData(PdxWriter& pw) const override;

  void fromData(PdxReader& pr) override;

  std::string toString() const override;

  virtual size_t objectSize() const override {
    auto objectSize = sizeof(PositionPdx);
    return objectSize;
  }

  static void resetCounter() { cnt = 0; }

  std::string getSecId() { return secId; }

  int32_t getId() { return pid; }

  int32_t getSharesOutstanding() { return sharesOutstanding; }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PositionPdx>();
  }

  const std::string& getClassName() const override {
    static std::string className = "testobject.PositionPdx";
    return className;
  }

 private:
  void init();
};

}  // namespace testobject

#endif  // GEODE_TESTOBJECT_POSITIONPDX_H_
