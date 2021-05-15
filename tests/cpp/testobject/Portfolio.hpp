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
  int32_t ID;
  std::shared_ptr<CacheableString> pkid;
  std::shared_ptr<Position> position1;
  std::shared_ptr<Position> position2;
  std::shared_ptr<CacheableHashMap> positions;
  std::shared_ptr<CacheableString> type;
  std::string status;
  std::shared_ptr<CacheableStringArray> names;
  static const char* secIds[];
  uint8_t* newVal;
  int32_t newValSize;
  std::shared_ptr<CacheableDate> creationDate;
  uint8_t* arrayNull;
  uint8_t* arrayZeroSize;

  inline size_t getObjectSize(const std::shared_ptr<Serializable>& obj) const {
    return (obj == nullptr ? 0 : obj->objectSize());
  }

 public:
  Portfolio()
      : ID(0),
        pkid(nullptr),
        type(nullptr),
        status(),
        newVal(nullptr),
        creationDate(nullptr),
        arrayNull(nullptr),
        arrayZeroSize(nullptr) {}
  explicit Portfolio(int32_t id, uint32_t size = 0,
                     std::shared_ptr<CacheableStringArray> nm = nullptr);
  ~Portfolio() noexcept override;

  size_t objectSize() const override {
    auto objectSize = sizeof(Portfolio);
    objectSize += getObjectSize(pkid);
    objectSize += getObjectSize(position1);
    objectSize += getObjectSize(position2);
    objectSize += getObjectSize(positions);
    objectSize += getObjectSize(type);
    objectSize += sizeof(decltype(status)::value_type) * status.length();
    objectSize += getObjectSize(names);
    objectSize += sizeof(uint8_t) * newValSize;
    objectSize += getObjectSize(creationDate);
    return objectSize;
  }

  int32_t getID() { return ID; }
  void showNames(const char* label) {
    LOG_INFO(label);
    if (!names) {
      LOG_INFO("names is NULL");
      return;
    }
    for (int i = 0; i < names->length(); i++) {
      LOG_INFO("names[%d]=%s", i, names->operator[](i)->value().c_str());
    }
  }

  std::shared_ptr<CacheableString> getPkid() const { return pkid; }

  std::shared_ptr<Position> getP1() const { return position1; }

  std::shared_ptr<Position> getP2() const { return position2; }

  std::shared_ptr<CacheableHashMap> getPositions() const { return positions; }

  bool testMethod() const { return true; }

  const std::string& getStatus() const { return status; }

  bool isActive() const { return status == "active"; }

  uint8_t* getNewVal() const { return newVal; }

  int32_t getNewValSize() const { return newValSize; }

  std::shared_ptr<CacheableDate> getCreationDate() const {
    return creationDate;
  }

  uint8_t* getArrayNull() const { return arrayNull; }

  uint8_t* getArrayZeroSize() const { return arrayZeroSize; }

  static std::shared_ptr<Serializable> createDeserializable() {
    return std::make_shared<Portfolio>();
  }

  void toData(DataOutput& output) const override;
  void fromData(DataInput& input) override;
  std::string toString() const override;
};

}  // namespace testobject

#endif  // GEODE_TESTOBJECT_PORTFOLIO_H_
