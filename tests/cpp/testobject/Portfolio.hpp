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

#include <geode/CacheableBuiltins.hpp>
#include <geode/CacheableDate.hpp>
#include "Position.hpp"
#include <util/Log.hpp>

using namespace apache::geode::client;

namespace testobject {

class TESTOBJECT_EXPORT Portfolio : public Serializable {
 private:
  int32_t ID;
  std::shared_ptr<CacheableString> pkid;
  std::shared_ptr<Position> position1;
  std::shared_ptr<Position> position2;
  std::shared_ptr<CacheableHashMap> positions;
  std::shared_ptr<CacheableString> type;
  char* status;
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
        status(NULL),
        newVal(NULL),
        creationDate(nullptr),
        arrayNull(NULL),
        arrayZeroSize(NULL) {}
  Portfolio(int32_t id, uint32_t size = 0,
            std::shared_ptr<CacheableStringArray> nm = nullptr);
  virtual ~Portfolio();

  virtual size_t objectSize() const {
    auto objectSize = sizeof(Portfolio);
    objectSize += getObjectSize(pkid);
    objectSize += getObjectSize(position1);
    objectSize += getObjectSize(position2);
    objectSize += getObjectSize(positions);
    objectSize += getObjectSize(type);
    objectSize +=
        (status == NULL ? 0
                        : sizeof(char) * static_cast<uint32_t>(strlen(status)));
    objectSize += getObjectSize(names);
    objectSize += sizeof(uint8_t) * newValSize;
    objectSize += getObjectSize(creationDate);
    return objectSize;
  }

  int32_t getID() { return ID; }
  void showNames(const char* label) {
    LOGINFO(label);
    if (names == nullptr) {
      LOGINFO("names is NULL");
      return;
    }
    for (int i = 0; i < names->length(); i++) {
      LOGINFO("names[%d]=%s", i, names->operator[](i)->value().c_str());
    }
  }

  std::shared_ptr<CacheableString> getPkid() { return pkid; }

  std::shared_ptr<Position> getP1() { return position1; }

  std::shared_ptr<Position> getP2() { return position2; }

  std::shared_ptr<CacheableHashMap> getPositions() { return positions; }

  bool testMethod(bool booleanArg) { return true; }

  char* getStatus() { return status; }

  bool isActive() { return (strcmp(status, "active") == 0) ? true : false; }

  uint8_t* getNewVal() { return newVal; }

  int32_t getNewValSize() { return newValSize; }

  std::shared_ptr<CacheableString> getType() { return this->type; }

  std::shared_ptr<CacheableDate> getCreationDate() { return creationDate; }

  uint8_t* getArrayNull() { return arrayNull; }

  uint8_t* getArrayZeroSize() { return arrayZeroSize; }

  static Serializable* createDeserializable() { return new Portfolio(); }

  virtual void toData(DataOutput& output) const;
  virtual void fromData(DataInput& input);
  virtual int32_t classId() const { return 0x03; }
  std::string toString() const;
};

}  // namespace testobject

#endif  // GEODE_TESTOBJECT_PORTFOLIO_H_
