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

#ifndef GEODE_INTEGRATION_TEST_QUERYHELPER_H_
#define GEODE_INTEGRATION_TEST_QUERYHELPER_H_

#include <cstdlib>

#include <ace/OS.h>

#include <geode/Region.hpp>
#include <geode/ResultSet.hpp>
#include <geode/StructSet.hpp>
#include <geode/SystemProperties.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "DistributedSystemImpl.hpp"
#include "SerializationRegistry.hpp"
#include "testobject/Portfolio.hpp"
#include "testobject/Position.hpp"
#include "testobject/PdxType.hpp"
#include "testobject/PortfolioPdx.hpp"
#include "testobject/PositionPdx.hpp"
#include <hacks/range.h>
#include "QueryStrings.hpp"
#include "fw_dunit.hpp"

//#include <geode/Struct.hpp>

//#ifndef ROOT_NAME
// ROOT_NAME+++ DEFINE ROOT_NAME before including QueryHelper.hpp
//#endif

#ifndef ROOT_SCOPE
#define ROOT_SCOPE LOCAL
#endif

namespace {  // NOLINT(google-build-namespaces)

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableStringArray;
using apache::geode::client::CacheRegionHelper;
using apache::geode::client::Region;
using apache::geode::client::ResultSet;
using apache::geode::client::SelectResults;
using apache::geode::client::Struct;
using apache::geode::client::StructSet;
using testData::constantExpectedRowsPQRS;
using testData::constantExpectedRowsRS;
using testData::constantExpectedRowsSS;
using testData::constantExpectedRowsSSPQ;
using testobject::Portfolio;
using testobject::PortfolioPdx;
using testobject::Position;
using testobject::PositionPdx;

class QueryHelper {
 public:
  static QueryHelper* singleton;

  static QueryHelper& getHelper() {
    if (singleton == nullptr) {
      singleton = new QueryHelper();
    }
    return *singleton;
  }

  QueryHelper() {
    portfolioSetSize = 20;
    portfolioNumSets = 1;
    positionSetSize = 20;
    positionNumSets = 1;
  }

  virtual ~QueryHelper() {}

  virtual void populatePortfolioData(
      std::shared_ptr<Region>& pregion, size_t setSize, size_t numSets,
      int32_t objSize = 1, std::shared_ptr<CacheableStringArray> nm = nullptr);
  virtual void populatePositionData(std::shared_ptr<Region>& pregion,
                                    size_t setSize, size_t numSets);
  virtual void populatePortfolioPdxData(std::shared_ptr<Region>& pregion,
                                        size_t setSize, size_t numSets,
                                        int32_t objSize = 1,
                                        char** nm = nullptr);
  virtual void populatePositionPdxData(std::shared_ptr<Region>& pregion,
                                       size_t setSize, size_t numSets);
  virtual void populatePDXObject(std::shared_ptr<Region>& pregion);
  virtual void getPDXObject(std::shared_ptr<Region>& pregion);

  virtual bool verifyRS(std::shared_ptr<SelectResults>& resultset,
                        size_t rowCount);
  virtual bool verifySS(std::shared_ptr<SelectResults>& structset,
                        size_t rowCount, int32_t fieldCount);

  // utility methods
  virtual size_t getPortfolioSetSize() { return portfolioSetSize; }
  virtual size_t getPortfolioNumSets() { return portfolioNumSets; }
  virtual size_t getPositionSetSize() { return positionSetSize; }
  virtual size_t getPositionNumSets() { return positionNumSets; }

  bool isExpectedRowsConstantRS(int queryindex) {
    for (int i = (sizeof(constantExpectedRowsRS) / sizeof(int)) - 1; i > -1;
         i--) {
      if (constantExpectedRowsRS[i] == queryindex) {
        printf("index %d is having constant rows \n",
               constantExpectedRowsRS[i]);
        return true;
      }
    }

    return false;
  }

  bool isExpectedRowsConstantPQRS(int queryindex) {
    for (int i = (sizeof(constantExpectedRowsPQRS) / sizeof(int)) - 1; i > -1;
         i--) {
      if (constantExpectedRowsPQRS[i] == queryindex) {
        printf("index %d is having constant rows \n",
               constantExpectedRowsPQRS[i]);
        return true;
      }
    }

    return false;
  }

  bool isExpectedRowsConstantSS(int queryindex) {
    for (int i = (sizeof(constantExpectedRowsSS) / sizeof(int)) - 1; i > -1;
         i--) {
      if (constantExpectedRowsSS[i] == queryindex) {
        printf("index %d is having constant rows \n",
               constantExpectedRowsSS[i]);
        return true;
      }
    }

    return false;
  }

  bool isExpectedRowsConstantSSPQ(int queryindex) {
    for (int i = (sizeof(constantExpectedRowsSSPQ) / sizeof(int)) - 1; i > -1;
         i--) {
      if (constantExpectedRowsSSPQ[i] == queryindex) {
        printf("index %d is having constant rows \n",
               constantExpectedRowsSSPQ[i]);
        return true;
      }
    }
    return false;
  }

 private:
  size_t portfolioSetSize;
  size_t portfolioNumSets;
  size_t positionSetSize;
  size_t positionNumSets;
};

QueryHelper* QueryHelper::singleton = nullptr;

//===========================================================================================

void QueryHelper::populatePortfolioData(
    std::shared_ptr<Region>& rptr, size_t setSize, size_t numSets,
    int32_t objSize, std::shared_ptr<CacheableStringArray> nm) {
  // lets reset the counter for uniform population of position objects
  Position::resetCounter();

  for (size_t set = 1; set <= numSets; set++) {
    for (size_t current = 1; current <= setSize; current++) {
      auto port = std::make_shared<Portfolio>(static_cast<int32_t>(current),
                                              objSize, nm);

      std::string key =
          "port" + std::to_string(set) + '-' + std::to_string(current);
      auto keyport = CacheableKey::create(key);
      // printf(" QueryHelper::populatePortfolioData creating key = %s and
      // puting data \n",portname);
      rptr->put(keyport, port);
    }
  }
  // portfolioSetSize = setSize; portfolioNumSets = numSets; objectSize =
  // objSize;

  printf("all puts done \n");
}

const char* secIds[] = {"SUN", "IBM",  "YHOO", "GOOG", "MSFT",
                        "AOL", "APPL", "ORCL", "SAP",  "DELL"};

void QueryHelper::populatePositionData(std::shared_ptr<Region>& rptr,
                                       size_t setSize, size_t numSets) {
  int numSecIds = sizeof(secIds) / sizeof(char*);

  for (size_t set = 1; set <= numSets; set++) {
    for (size_t current = 1; current <= setSize; current++) {
      auto pos = std::make_shared<Position>(
          secIds[current % numSecIds], static_cast<int32_t>(current * 100));

      std::string key =
          "pos" + std::to_string(set) + '-' + std::to_string(current);
      auto keypos = CacheableKey::create(key);
      rptr->put(keypos, pos);
    }
  }
  // positionSetSize = setSize; positionNumSets = numSets;
}

void QueryHelper::populatePortfolioPdxData(std::shared_ptr<Region>& rptr,
                                           size_t setSize, size_t numSets,
                                           int32_t objSize, char**) {
  // lets reset the counter for uniform population of position objects
  PositionPdx::resetCounter();

  for (size_t set = 1; set <= numSets; set++) {
    for (size_t current = 1; current <= setSize; current++) {
      auto port = std::make_shared<PortfolioPdx>(static_cast<int32_t>(current),
                                                 objSize);
      std::string key =
          "port" + std::to_string(set) + '-' + std::to_string(current);

      auto keyport = CacheableKey::create(key);
      rptr->put(keyport, port);

      LOG_DEBUG("populatePortfolioPdxData:: Put for iteration current = %d done",
               current);
    }
  }
  // portfolioSetSize = setSize; portfolioNumSets = numSets; objectSize =
  // objSize;

  printf("all puts done \n");
}

void QueryHelper::populatePositionPdxData(std::shared_ptr<Region>& rptr,
                                          size_t setSize, size_t numSets) {
  auto numSecIds = sizeof(secIds) / sizeof(char*);

  for (size_t set = 1; set <= numSets; set++) {
    for (size_t current = 1; current <= setSize; current++) {
      auto pos = std::make_shared<PositionPdx>(
          secIds[current % numSecIds], static_cast<int32_t>(current * 100));

      std::string key =
          "pos" + std::to_string(set) + '-' + std::to_string(current);

      auto keypos = CacheableKey::create(key);
      rptr->put(keypos, pos);
    }
  }
  // positionSetSize = setSize; positionNumSets = numSets;
}

void QueryHelper::populatePDXObject(std::shared_ptr<Region>& rptr) {
  // Register PdxType Object

  auto cacheImpl = CacheRegionHelper::getCacheImpl(&rptr->getCache());
  cacheImpl->getSerializationRegistry()->addPdxSerializableType(
      PdxTests::PdxType::createDeserializable);
  LOG("PdxObject Registered Successfully....");

  // Creating object of type PdxObject
  auto pdxobj = std::make_shared<PdxTests::PdxType>();
  auto keyport = CacheableKey::create("ABC");

  // PUT Operation
  rptr->put(keyport, pdxobj);

  // locally destroy PdxObject
  rptr->localDestroy(keyport);
  LOG("localDestroy() operation....Done");

  // Remote GET for PdxObject
  auto obj2 = std::dynamic_pointer_cast<PdxTests::PdxType>(rptr->get(keyport));

  LOG_INFO("get... Result-1: Returned float=%f, String val = %s double=%lf",
          obj2->getFloat(), obj2->getString().c_str(), obj2->getDouble());
  // LOG_INFO("get.. Result-2: Returned BOOL = %d and BYTE = %s SHORT=%d INT=%d",
  // obj2->getBool(), obj2->getByte(), obj2->getShort(), obj2->getInt());

  // TODO
  /*
  ASSERT(obj2->getID1() == 101, "ID1 = 101 expected");
  ASSERT(obj2->getID2() == 201, "ID2 = 201 expected");
  ASSERT(obj2->getID3() == 301, "ID3 = 301 expected");
  */

  LOG("NIL:200:PUT Operation successfully Done....End");
}

void QueryHelper::getPDXObject(std::shared_ptr<Region>& rptr) {
  // Remote GET for PdxObject
  // PdxObject *obj2 = dynamic_cast<PdxObject *> ((rptr->get(keyport)).get());

  auto keyport = CacheableKey::create("ABC");
  LOG("Client-2 PdxObject GET OP Start....");
  auto obj2 = std::dynamic_pointer_cast<PdxTests::PdxType>(rptr->get(keyport));
  LOG("Client-2 PdxObject GET OP Done....");

  /*
  LOG_INFO("GET OP Result: BoolVal=%d", obj2->getBool());
  LOG_INFO("GET OP Result: ByteVal=%d", obj2->getByte());
  LOG_INFO("GET OP Result: ShortVal=%d", obj2->getShort());*/

  // LOG_INFO("GET OP Result: IntVal=%d", obj2->getInt());
  /*
  LOG_INFO("GET OP Result: LongVal=%ld", obj2->getLong());
  LOG_INFO("GET OP Result: FloatVal=%f", obj2->getFloat());
  LOG_INFO("GET OP Result: DoubleVal=%lf", obj2->getDouble());
  LOG_INFO("GET OP Result: StringVal=%s", obj2->getString());
  */
}

bool QueryHelper::verifyRS(std::shared_ptr<SelectResults>& resultSet,
                           size_t expectedRows) {
  if (auto rsptr = std::static_pointer_cast<ResultSet>(resultSet)) {
    size_t foundRows = 0;
    for (auto&& row : hacks::range(*rsptr)) {
      foundRows++;
    }

    printf("found rows %zd, expected %zd \n", foundRows, expectedRows);
    if (foundRows == expectedRows) return true;
  }
  return false;
}

bool QueryHelper::verifySS(std::shared_ptr<SelectResults>& structSet,
                           size_t expectedRows, int32_t expectedFields) {
  if (auto ssptr = std::static_pointer_cast<StructSet>(structSet)) {
    size_t foundRows = 0;
    for (auto&& ser : hacks::range(*ssptr)) {
      foundRows++;

      auto siptr = std::dynamic_pointer_cast<Struct>(ser);

      if (siptr == nullptr) {
        printf("siptr is nullptr \n\n");
        return false;
      }

      int32_t foundFields = 0;
      for (auto&& field : *siptr) {
        foundFields++;
      }

      if (foundFields != expectedFields) {
        char buffer[1024] = {'\0'};
        sprintf(buffer, "found fields %d, expected fields %d \n", foundFields,
                expectedFields);
        LOG(buffer);
        return false;
      }
    }

    if (foundRows == expectedRows) return true;

    // lets log and return in case of error only situation
    char buffer[1024] = {'\0'};
    sprintf(buffer, "found rows %zd, expected rows %zd\n", foundRows,
            expectedRows);
    LOG(buffer);
  } else {
    if (expectedRows == 0 && expectedFields == 0) {
      return true;  // quite possible we got a null set back.
    }
  }
  return false;
}

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_QUERYHELPER_H_
