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

#ifndef GEODE_TESTOBJECT_PORTFOLIO_H_
#define GEODE_TESTOBJECT_PORTFOLIO_H_

/*
 * @brief User class for testing the put functionality for object.
 */

#include <util/Log.hpp>

#include <geode/CacheableBuiltins.hpp>
#include <geode/CacheableDate.hpp>

#include "Position.hpp"

namespace testobject {

using apache::geode::client::CacheableDate;
using apache::geode::client::CacheableHashMap;
using apache::geode::client::CacheableStringArray;

class TESTOBJECT_EXPORT Portfolio : public DataSerializable {
 private:
  int32_t _ID;
  std::shared_ptr<CacheableString> _pkid;
  std::shared_ptr<Position> _position1;
  std::shared_ptr<Position> _position2;
  std::shared_ptr<CacheableHashMap> _positions;
  std::shared_ptr<CacheableString> _type;
  std::string _status;
  std::shared_ptr<CacheableStringArray> _names;
  static const char* _secIds[];
  uint8_t* _newVal;
  int32_t _newValSize;
  std::shared_ptr<CacheableDate> _creationDate;
  uint8_t* _arrayNull;
  uint8_t* _arrayZeroSize;

  inline size_t getObjectSize(const std::shared_ptr<Serializable>& obj) const {
    return (obj == nullptr ? 0 : obj->objectSize());
  }

 public:
  Portfolio()
      : _ID(0),
        _pkid(nullptr),
        _type(nullptr),
        _status(),
        _newVal(nullptr),
        _creationDate(nullptr),
        _arrayNull(nullptr),
        _arrayZeroSize(nullptr) {}
  explicit Portfolio(int32_t id, uint32_t size = 0,
                     std::shared_ptr<CacheableStringArray> nm = nullptr);
  ~Portfolio() noexcept override;

  size_t objectSize() const override {
    auto objectSize = sizeof(Portfolio);
    objectSize += getObjectSize(_pkid);
    objectSize += getObjectSize(_position1);
    objectSize += getObjectSize(_position2);
    objectSize += getObjectSize(_positions);
    objectSize += getObjectSize(_type);
    objectSize += sizeof(decltype(_status)::value_type) * _status.length();
    objectSize += getObjectSize(_names);
    objectSize += sizeof(uint8_t) * _newValSize;
    objectSize += getObjectSize(_creationDate);
    return objectSize;
  }

  int32_t getID() { return _ID; }
  void showNames(const char* label) {
    LOGINFO(label);
    if (!_names) {
      LOGINFO("names is NULL");
      return;
    }
    for (int i = 0; i < _names->length(); i++) {
      LOGINFO("names[%d]=%s", i, _names->operator[](i)->value().c_str());
    }
  }

  std::shared_ptr<CacheableString> getPkid() const { return _pkid; }

  std::shared_ptr<Position> getP1() const { return _position1; }

  std::shared_ptr<Position> getP2() const { return _position2; }

  std::shared_ptr<CacheableHashMap> getPositions() const { return _positions; }

  bool testMethod() const { return true; }

  const std::string& getStatus() const { return _status; }

  bool isActive() const { return _status == "active"; }

  uint8_t* getNewVal() const { return _newVal; }

  int32_t getNewValSize() const { return _newValSize; }

  std::shared_ptr<CacheableDate> getCreationDate() const {
    return _creationDate;
  }

  uint8_t* getArrayNull() const { return _arrayNull; }

  uint8_t* getArrayZeroSize() const { return _arrayZeroSize; }

  static std::shared_ptr<Serializable> createDeserializable() {
    return std::make_shared<Portfolio>();
  }

  void toData(DataOutput& output) const override;
  void fromData(DataInput& input) override;
  std::string toString() const override;
};

}  // namespace testobject

#endif  // GEODE_TESTOBJECT_PORTFOLIO_H_
