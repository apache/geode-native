#pragma once

#ifndef GEODE_TESTOBJECT_PORTFOLIOPDX_H_
#define GEODE_TESTOBJECT_PORTFOLIOPDX_H_

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

#include "PositionPdx.hpp"

using namespace apache::geode::client;

namespace testobject {

class TESTOBJECT_EXPORT PortfolioPdx : public PdxSerializable {
 private:
  int32_t id;

  std::string pkid;

  std::shared_ptr<PositionPdx> position1;
  std::shared_ptr<PositionPdx> position2;
  std::shared_ptr<CacheableHashMap> positions;
  std::string type;
  std::string status;
  std::vector<std::string> names;
  static const char* secIds[];
  std::vector<int8_t> newVal;
  int32_t newValSize;
  std::shared_ptr<CacheableDate> creationDate;
  std::vector<int8_t> arrayNull;
  std::vector<int8_t> arrayZeroSize;

 public:
  PortfolioPdx()
      : id(0),
        pkid(),
        type(),
        status(),
        newVal(),
        creationDate(nullptr),
        arrayNull(),
        arrayZeroSize() {}

  PortfolioPdx(int32_t id, int32_t size = 0, std::vector<std::string> nm = {});

  int32_t getID() { return id; }

  std::string getPkid() { return pkid; }

  std::shared_ptr<PositionPdx> getP1() { return position1; }

  std::shared_ptr<PositionPdx> getP2() { return position2; }

  std::shared_ptr<CacheableHashMap> getPositions() { return positions; }

  bool testMethod(bool booleanArg) { return true; }

  const std::string& getStatus() { return status; }

  bool isActive() { return status == "active"; }

  std::vector<int8_t> getNewVal() { return newVal; }

  int32_t getNewValSize() { return newValSize; }

  const std::string& getClassName() { return this->type; }

  std::shared_ptr<CacheableDate> getCreationDate() { return creationDate; }

  std::vector<int8_t> getArrayNull() { return arrayNull; }

  std::vector<int8_t> getArrayZeroSize() { return arrayZeroSize; }

  static PdxSerializable* createDeserializable() { return new PortfolioPdx(); }

  const std::string& getClassName() const override {
    static std::string className = "testobject.PortfolioPdx";
    return className;
  }

  using PdxSerializable::toData;
  using PdxSerializable::fromData;

  virtual void toData(PdxWriter& pw) const override;
  virtual void fromData(PdxReader& pr) override;

  std::string toString() const override;
};

}  // namespace testobject

#endif  // GEODE_TESTOBJECT_PORTFOLIOPDX_H_
