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

#ifndef __PORTFOLIOPDX_AUTO_HPP__
#define __PORTFOLIOPDX_AUTO_HPP__

#include "PositionPdxAuto.hpp"

using namespace apache::geode::client;

namespace testobject {
#define GFARRAYSIZE(X)
#define GFID
class PortfolioPdxAuto : public apache::geode::client::PdxSerializable {
 private:
  GFID int32_t id;

  GFID char* pkid;

  GFID std::shared_ptr<PositionPdx> position1;
  GFID std::shared_ptr<PositionPdx> position2;
  GFID std::shared_ptr<CacheableHashMap> positions;
  GFID char* type;
  GFID char* status;
  // char** names;
  // static const char* secIds[];
  GFID int8_t* newVal;
  GFARRAYSIZE(newVal) int32_t newValSize;
  GFID std::shared_ptr<CacheableDate> creationDate;
  GFID int8_t* arrayNull;
  GFARRAYSIZE(arrayNull) int32_t arrayNullSize;

  GFID int8_t* arrayZeroSize;
  GFARRAYSIZE(arrayZeroSize) int32_t arrayZeroSizeSize;

 public:
  PortfolioPdxAuto()
      : id(0),
        pkid(NULL),
        type(NULL),
        status(NULL),
        newVal(NULL),
        creationDate(nullptr),
        arrayNull(NULL),
        arrayNullSize(0),
        arrayZeroSize(NULL),
        arrayZeroSizeSize(0) {}

  PortfolioPdxAuto(int32_t id, int32_t size = 0, char** nm = NULL);

  virtual ~PortfolioPdxAuto();

  int32_t getID() { return id; }

  char* getPkid() { return pkid; }

  std::shared_ptr<PositionPdx> getP1() { return position1; }

  std::shared_ptr<PositionPdx> getP2() { return position2; }

  std::shared_ptr<CacheableHashMap> getPositions() { return positions; }

  bool testMethod(bool booleanArg) { return true; }

  char* getStatus() { return status; }

  bool isActive() { return (strcmp(status, "active") == 0) ? true : false; }

  int8_t* getNewVal() { return newVal; }

  int32_t getNewValSize() { return newValSize; }

  const char* getClassName() { return this->type; }

  std::shared_ptr<CacheableDate> getCreationDate() { return creationDate; }

  int8_t* getArrayNull() { return arrayNull; }

  int8_t* getArrayZeroSize() { return arrayZeroSize; }

  const char* getClassName() const;

  virtual void toData(PdxWriter& pw);
  virtual void fromData(PdxReader& pr);

  static PdxSerializable* createDeserializable();

  std::string toString() const;
};
}
#endif
