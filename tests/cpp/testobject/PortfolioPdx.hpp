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

namespace testobject {
using apache::geode::client::CacheableDate;
using apache::geode::client::CacheableHashMap;
using apache::geode::client::PdxSerializable;

class TESTOBJECT_EXPORT PortfolioPdx : public PdxSerializable {
 private:
  int32_t id;

  std::string pkid;

  std::shared_ptr<PositionPdx> _position1;
  std::shared_ptr<PositionPdx> _position2;
  std::shared_ptr<CacheableHashMap> _positions;
  std::string _type;
  std::string _status;
  std::vector<std::string> _names;
  static const char* _secIds[];
  std::vector<int8_t> _newVal;
  int32_t _newValSize;
  std::shared_ptr<CacheableDate> _creationDate;
  std::vector<int8_t> _arrayNull;
  std::vector<int8_t> _arrayZeroSize;

 public:
  PortfolioPdx()
      : id(0),
        pkid(),
        _type(),
        _status(),
        _newVal(),
        _creationDate(nullptr),
        _arrayNull(),
        _arrayZeroSize() {}

  explicit PortfolioPdx(int32_t id, int32_t size = 0,
                        std::vector<std::string> nm = {});

  int32_t getID() { return id; }

  std::string getPkid() { return pkid; }

  std::shared_ptr<PositionPdx> getP1() { return _position1; }

  std::shared_ptr<PositionPdx> getP2() { return _position2; }

  std::shared_ptr<CacheableHashMap> getPositions() { return _positions; }

  bool testMethod() { return true; }

  const std::string& getStatus() { return _status; }

  bool isActive() { return _status == "active"; }

  std::vector<int8_t> getNewVal() { return _newVal; }

  int32_t getNewValSize() { return _newValSize; }

  const std::string& getClassName() { return this->_type; }

  std::shared_ptr<CacheableDate> getCreationDate() { return _creationDate; }

  std::vector<int8_t> getArrayNull() { return _arrayNull; }

  std::vector<int8_t> getArrayZeroSize() { return _arrayZeroSize; }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PortfolioPdx>();
  }

  const std::string& getClassName() const override {
    static std::string className = "testobject.PortfolioPdx";
    return className;
  }

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void toData(PdxWriter& pw) const override;
  virtual void fromData(PdxReader& pr) override;

  std::string toString() const override;
};

}  // namespace testobject

#endif  // GEODE_TESTOBJECT_PORTFOLIOPDX_H_
