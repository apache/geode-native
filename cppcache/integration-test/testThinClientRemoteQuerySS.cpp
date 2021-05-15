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
#include "fw_dunit.hpp"
#include <ace/OS.h>
#include <ace/High_Res_Timer.h>
#include <string>

#define ROOT_NAME "testThinClientRemoteQuerySS"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "ThinClientHelper.hpp"

#include "QueryStrings.hpp"
#include "QueryHelper.hpp"

#include <geode/Query.hpp>
#include <geode/QueryService.hpp>

#include "SerializationRegistry.hpp"
#include "CacheRegionHelper.hpp"
#include "CacheImpl.hpp"

#define CLIENT1 s1p1
#define LOCATOR s1p2
#define SERVER1 s2p1

using apache::geode::client::Cacheable;
using apache::geode::client::CacheableBoolean;
using apache::geode::client::CacheableHashMap;
using apache::geode::client::CacheableVector;
using apache::geode::client::HashMapOfCacheable;
using apache::geode::client::IllegalStateException;
using apache::geode::client::QueryService;

using testData::numSSQueryParam;
using testData::queryparamSetSS;
using testData::QueryStrings;
using testData::structsetFieldCounts;
using testData::structsetFieldCountsOPL;
using testData::structsetFieldCountsPQ;
using testData::structsetParamQueries;
using testData::structsetQueries;
using testData::structsetQueriesOPL;
using testData::structsetRowCounts;
using testData::structsetRowCountsOPL;
using testData::structsetRowCountsPQ;
using testData::unsupported;

bool isLocator = false;
bool isLocalServer = false;

const char *poolNames[] = {"Pool1", "Pool2", "Pool3"};
const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
bool isPoolConfig = false;  // To track if pool case is running
const char *qRegionNames[] = {"Portfolios", "Positions", "Portfolios2",
                              "Portfolios3"};

const char *checkNullString(const char *str) {
  return ((str == nullptr) ? "(null)" : str);
}

const wchar_t *checkNullString(const wchar_t *str) {
  return ((str == nullptr) ? L"(null)" : str);
}

std::string checkNullString(const std::string *str) {
  return ((str == nullptr) ? "(null)" : *str);
}

void _printFields(std::shared_ptr<Cacheable> field, Struct *ssptr,
                  int32_t &fields) {
  try {
    if (auto portfolio = std::dynamic_pointer_cast<Portfolio>(field)) {
      printf("   pulled %s :- ID %d, pkid %s\n",
             ssptr->getFieldName(fields).c_str(), portfolio->getID(),
             checkNullString(portfolio->getPkid()->value().c_str()));
      printf("   pulled %s :- ID %d, pkid %s\n",
             ssptr->getFieldName(fields).c_str(), portfolio->getID(),
             checkNullString(portfolio->getPkid()->value().c_str()));
    } else if (auto position = std::dynamic_pointer_cast<Position>(field)) {
      printf("   pulled %s :- secId %s, shares %d\n",
             ssptr->getFieldName(fields).c_str(),
             checkNullString(position->getSecId()->value().c_str()),
             position->getSharesOutstanding());
    } else if (auto portfolioPdx =
                   std::dynamic_pointer_cast<PortfolioPdx>(field)) {
      printf("   pulled %s :- ID %d, pkid %s\n",
             ssptr->getFieldName(fields).c_str(), portfolioPdx->getID(),
             portfolioPdx->getPkid().c_str());
    } else if (auto positionPdx =
                   std::dynamic_pointer_cast<PositionPdx>(field)) {
      printf("   pulled %s :- secId %s, shares %d\n",
             ssptr->getFieldName(fields).c_str(),
             positionPdx->getSecId().c_str(),
             positionPdx->getSharesOutstanding());
    } else {
      if (auto str = std::dynamic_pointer_cast<CacheableString>(field)) {
        printf("   pulled %s :- %s\n", ssptr->getFieldName(fields).c_str(),
               checkNullString(str->value().c_str()));
      } else if (auto boolptr =
                     std::dynamic_pointer_cast<CacheableBoolean>(field)) {
        printf("   pulled %s :- %s\n", ssptr->getFieldName(fields).c_str(),
               boolptr->toString().c_str());
      } else {
        if (auto ptr = std::dynamic_pointer_cast<CacheableKey>(field)) {
          printf("   pulled %s :- %s \n", ssptr->getFieldName(fields).c_str(),
                 ptr->toString().c_str());
        } else if (auto strArr =
                       std::dynamic_pointer_cast<CacheableStringArray>(field)) {
          printf(" string array object printing \n\n");
          for (int stri = 0; stri < strArr->length(); stri++) {
            printf("   pulled %s(%d) - %s \n",
                   ssptr->getFieldName(fields).c_str(), stri,
                   strArr->operator[](stri)->value().c_str());
          }
        } else if (auto map =
                       std::dynamic_pointer_cast<CacheableHashMap>(field)) {
          int index = 0;
          for (const auto &iter : *map) {
            printf("   hashMap %d of %zd ... \n", ++index, map->size());
            _printFields(iter.first, ssptr, fields);
            _printFields(iter.second, ssptr, fields);
          }
          printf("   end of map \n");
        } else if (auto structimpl = std::dynamic_pointer_cast<Struct>(field)) {
          printf("   structImpl %s {\n", ssptr->getFieldName(fields).c_str());
          for (int32_t inner_fields = 0; inner_fields < structimpl->size();
               inner_fields++) {
            auto innerField = (*structimpl)[inner_fields];
            if (innerField == nullptr) {
              printf(
                  "we got null fields here, probably we have nullptr data\n");
              continue;
            }

            _printFields(innerField, structimpl.get(), inner_fields);

          }  // end of field iterations
          printf("   } //end of %s\n", ssptr->getFieldName(fields).c_str());
        } else {
          printf(
              "unknown field data.. couldn't even convert it to Cacheable "
              "variants\n");
        }
      }

    }  // end of else
  } catch (const std::out_of_range &e) {
    printf("Caught a non-fatal out_of_range exception: %s", e.what());
  }
}

void _verifyStructSet(std::shared_ptr<StructSet> &ssptr, int i) {
  printf("query idx %d \n", i);
  for (size_t rows = 0; rows < ssptr->size(); rows++) {
    if (rows > QueryHelper::getHelper().getPortfolioSetSize()) {
      continue;
    }

    Struct *siptr = dynamic_cast<Struct *>(((*ssptr)[rows]).get());
    if (siptr == nullptr) {
      printf("siptr is nullptr \n\n");
      continue;
    }

    printf("   Row : %zd \n", rows);
    for (int32_t fields = 0; fields < siptr->size(); fields++) {
      auto field = (*siptr)[fields];
      if (field == nullptr) {
        printf("we got null fields here, probably we have nullptr data\n");
        continue;
      }

      _printFields(field, siptr, fields);

    }  // end of field iterations
  }    // end of row iterations
}

void compareMaps(HashMapOfCacheable &map, HashMapOfCacheable &expectedMap) {
  ASSERT(expectedMap.size() == map.size(),
         "Unexpected number of entries in map");
  LOG_INFO("Got expected number of %d entries in map", map.size());
  for (const auto &iter : map) {
    const auto &key = iter.first;
    const auto &val = iter.second;
    const auto &expectedIter = expectedMap.find(key);
    if (expectedIter == expectedMap.end()) {
      FAIL("Could not find expected key in map");
    }
    const auto &expectedVal = expectedIter->second;

    if (std::dynamic_pointer_cast<PositionPdx>(expectedVal)) {
      auto posVal = std::dynamic_pointer_cast<PositionPdx>(val);
      auto expectedPosVal = std::dynamic_pointer_cast<PositionPdx>(expectedVal);
      ASSERT(expectedPosVal->getSecId() == posVal->getSecId(),
             "Expected the secIDs to be equal in PositionPdx");
      ASSERT(expectedPosVal->getSharesOutstanding() ==
                 posVal->getSharesOutstanding(),
             "Expected the sharesOutstanding to be equal in PositionPdx");
    } else {
      auto portVal = std::dynamic_pointer_cast<PortfolioPdx>(val);
      auto expectedPortVal =
          std::dynamic_pointer_cast<PortfolioPdx>(expectedVal);
      ASSERT(expectedPortVal->getID() == portVal->getID(),
             "Expected the IDs to be equal in PortfolioPdx");
      ASSERT(expectedPortVal->getNewValSize() == portVal->getNewValSize(),
             "Expected the sizes to be equal in PortfolioPdx");
    }
  }
}

void stepOne() {
  initClient(true);
  try {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    serializationRegistry->addDataSerializableType(
        Position::createDeserializable, 2);
    serializationRegistry->addDataSerializableType(
        Portfolio::createDeserializable, 3);

    serializationRegistry->addPdxSerializableType(
        PositionPdx::createDeserializable);
    serializationRegistry->addPdxSerializableType(
        PortfolioPdx::createDeserializable);
  } catch (const IllegalStateException &) {
    // ignore exception
    LOG("testThinClientRemoteQuerySS stepOne caught exception using "
        "serializationRegistry");
  }

  isPoolConfig = true;
  createPool(poolNames[0], locHostPort, {}, 0, true);
  createRegionAndAttachPool(qRegionNames[0], USE_ACK, poolNames[0]);
  createRegionAndAttachPool(qRegionNames[1], USE_ACK, poolNames[0]);
  createRegionAndAttachPool(qRegionNames[2], USE_ACK, poolNames[0]);
  createRegionAndAttachPool(qRegionNames[3], USE_ACK, poolNames[0]);

  auto regptr = getHelper()->getRegion(qRegionNames[0]);
  auto subregPtr =
      regptr->createSubregion(qRegionNames[1], regptr->getAttributes());

  LOG("StepOne complete.");
}

DUNIT_TASK_DEFINITION(LOCATOR, StartLocator)
  {
    // starting locator 1 2
    if (isLocator) {
      CacheHelper::initLocator(1);
    }
    LOG("Locator started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServer)
  {
    LOG("Starting SERVER1...");

    if (isLocalServer) CacheHelper::initServer(1, "remotequery.xml");

    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServerWithLocator)
  {
    LOG("Starting SERVER1...");
    if (isLocalServer) {
      CacheHelper::initServer(1, "remotequery.xml", locHostPort);
    }
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOnePoolLoc)
  {
    LOG("Starting Step One with Pool + Locator lists");
    stepOne();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    auto regPtr0 = getHelper()->getRegion(qRegionNames[0]);
    auto regPtr1 = regPtr0->getSubregion("Positions");
    auto regPtr2 = getHelper()->getRegion(qRegionNames[1]);

    auto regPtr3 = getHelper()->getRegion(qRegionNames[2]);
    auto regPtr4 = getHelper()->getRegion(qRegionNames[3]);

    auto *qh = &QueryHelper::getHelper();

    qh->populatePortfolioPdxData(regPtr0, qh->getPortfolioSetSize(),
                                 qh->getPortfolioNumSets());
    qh->populatePositionPdxData(regPtr1, qh->getPositionSetSize(),
                                qh->getPositionNumSets());
    qh->populatePositionPdxData(regPtr2, qh->getPositionSetSize(),
                                qh->getPositionNumSets());

    qh->populatePortfolioPdxData(regPtr3, qh->getPortfolioSetSize(),
                                 qh->getPortfolioNumSets());
    qh->populatePortfolioPdxData(regPtr4, qh->getPortfolioSetSize(),
                                 qh->getPortfolioNumSets());

    char buf[100];
    sprintf(buf, "SetSize %zd, NumSets %zd", qh->getPortfolioSetSize(),
            qh->getPortfolioNumSets());
    LOG(buf);

    LOG("StepThree complete.\n");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFour)
  {
    SLEEP(100);
    bool doAnyErrorOccured = false;
    auto *qh = &QueryHelper::getHelper();

    std::shared_ptr<QueryService> qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }

    for (int i = 0; i < QueryStrings::SSOPLsize(); i++) {
      auto qry = qs->newQuery(structsetQueriesOPL[i].query());
      auto results = qry->execute();
      if (!qh->verifySS(results, structsetRowCountsOPL[i],
                        structsetFieldCountsOPL[i])) {
        std::string failmsg =
            "Query verify failed for query index " + std::to_string(i);
        ASSERT(false, failmsg);
      }

      auto ssptr = std::dynamic_pointer_cast<StructSet>(results);
      if ((ssptr) == nullptr) {
        LOG("Zero records were expected and found. Moving onto next. ");
        continue;
      }

      _verifyStructSet(ssptr, i);
    }

    if (!doAnyErrorOccured) printf("HURRAY !! StepFour PASSED \n\n");

    LOG("StepFour complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFive)
  {
    SLEEP(100);
    auto qh = &QueryHelper::getHelper();

    std::shared_ptr<QueryService> qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }

    for (int i = 0; i < QueryStrings::SSsize(); i++) {
      if (i == 12 || i == 4 || i == 7 || i == 22 || i == 30 || i == 34) {
        LOG_DEBUG("Skipping query index %d for pdx because it has function.",
                  i);
        continue;
      }

      if (structsetQueries[i].category != unsupported) {
        auto qry = qs->newQuery(structsetQueries[i].query());
        auto results = qry->execute();
        if (!qh->verifySS(
                results,
                (qh->isExpectedRowsConstantSS(i)
                     ? structsetRowCounts[i]
                     : structsetRowCounts[i] * qh->getPortfolioNumSets()),
                structsetFieldCounts[i])) {
          std::string failmsg =
              "Query verify failed for query index " + std::to_string(i);
          ASSERT(false, failmsg);
        }

        auto ssptr = std::dynamic_pointer_cast<StructSet>(results);
        if (ssptr == nullptr) {
          LOG("Zero records were expected and found. Moving onto next. ");
          continue;
        }

        _verifyStructSet(ssptr, i);
      }
    }

    printf("HURRAY !! We PASSED \n\n");
    LOG("StepFive complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepSix)
  {
    SLEEP(100);
    bool doAnyErrorOccured = false;
    auto *qh = &QueryHelper::getHelper();

    std::shared_ptr<QueryService> qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }

    for (int i = 0; i < QueryStrings::SSPsize(); i++) {
      if (i == 16) {
        LOG_DEBUG("Skipping query index %d for pdx because it has function.",
                  i);
        continue;
      }

      if (structsetParamQueries[i].category != unsupported) {
        auto qry = qs->newQuery(structsetParamQueries[i].query());
        auto paramList = CacheableVector::create();

        for (int j = 0; j < numSSQueryParam[i]; j++) {
          // LOG_INFO("NIL::SSPQ::328: queryparamSetSS[%d][%d] = %s", i, j,
          // queryparamSetSS[i][j]);
          if (atoi(queryparamSetSS[i][j]) != 0) {
            paramList->push_back(
                Cacheable::create(atoi(queryparamSetSS[i][j])));
          } else {
            paramList->push_back(Cacheable::create(queryparamSetSS[i][j]));
          }
        }

        auto results = qry->execute(paramList);
        if (!qh->verifySS(
                results,
                (qh->isExpectedRowsConstantSSPQ(i)
                     ? structsetRowCountsPQ[i]
                     : structsetRowCountsPQ[i] * qh->getPortfolioNumSets()),
                structsetFieldCountsPQ[i])) {
          std::string failmsg =
              "Query verify failed for query index " + std::to_string(i);
          ASSERT(false, failmsg);
        }

        auto ssptr = std::dynamic_pointer_cast<StructSet>(results);
        if ((ssptr) == nullptr) {
          LOG("Zero records were expected and found. Moving onto next. ");
          continue;
        }
        _verifyStructSet(ssptr, i);
      }
    }
    if (!doAnyErrorOccured) printf("HURRAY !! We PASSED \n\n");
    LOG("StepSix complete.");
  }
END_TASK_DEFINITION

// test for getAll with complex objects after they have been deserialized
// on the server
DUNIT_TASK_DEFINITION(CLIENT1, GetAll)
  {
    auto regPtr0 = getHelper()->getRegion(qRegionNames[0]);
    auto regPtr1 = regPtr0->getSubregion("Positions");
    auto regPtr2 = getHelper()->getRegion(qRegionNames[1]);
    auto regPtr3 = getHelper()->getRegion(qRegionNames[2]);
    auto regPtr4 = getHelper()->getRegion(qRegionNames[3]);

    // reset the counter for uniform population of position objects
    PositionPdx::resetCounter();

    int numSecIds = sizeof(secIds) / sizeof(char *);

    std::vector<std::shared_ptr<CacheableKey>> posKeys;
    std::vector<std::shared_ptr<CacheableKey>> portKeys;
    HashMapOfCacheable expectedPosMap;
    HashMapOfCacheable expectedPortMap;

    auto &qh = QueryHelper::getHelper();
    auto setSize = qh.getPositionSetSize();
    auto numSets = qh.getPositionNumSets();

    for (size_t set = 1; set <= numSets; ++set) {
      for (size_t current = 1; current <= setSize; ++current) {
        std::string key =
            "pos" + std::to_string(set) + '-' + std::to_string(current);

        auto posKey = CacheableKey::create(key);
        auto pos = std::make_shared<PositionPdx>(
            secIds[current % numSecIds], static_cast<int32_t>(current * 100));

        posKeys.push_back(posKey);
        expectedPosMap.emplace(posKey, pos);
      }
    }

    // reset the counter for uniform population of position objects
    PositionPdx::resetCounter();

    setSize = qh.getPortfolioSetSize();
    numSets = qh.getPortfolioNumSets();
    for (size_t set = 1; set <= numSets; ++set) {
      for (size_t current = 1; current <= setSize; ++current) {
        std::string key =
            "port" + std::to_string(set) + '-' + std::to_string(current);

        auto portKey = CacheableKey::create(key);
        auto port =
            std::make_shared<PortfolioPdx>(static_cast<int32_t>(current), 1);

        portKeys.push_back(portKey);
        expectedPortMap.emplace(portKey, port);
      }
    }

    // execute getAll for different regions and verify results
    {
      auto resMap = regPtr0->getAll(portKeys);
      compareMaps(resMap, expectedPortMap);
    }

    {
      auto resMap = regPtr1->getAll(posKeys);
      compareMaps(resMap, expectedPosMap);
    }

    {
      auto resMap = regPtr2->getAll(posKeys);
      compareMaps(resMap, expectedPosMap);
    }

    {
      auto resMap = regPtr3->getAll(portKeys);
      compareMaps(resMap, expectedPortMap);
    }

    {
      auto resMap = regPtr4->getAll(portKeys);
      compareMaps(resMap, expectedPortMap);
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, DoQuerySSError)
  {
    QueryHelper::getHelper();

    std::shared_ptr<QueryService> qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }

    for (int i = 0; i < QueryStrings::SSsize(); i++) {
      if (structsetQueries[i].category == unsupported) {
        auto qry = qs->newQuery(structsetQueries[i].query());

        try {
          auto results = qry->execute();
          std::string failmsg =
              "Query exception didnt occur for index " + std::to_string(i);

          LOG(failmsg);
          FAIL(failmsg);
        } catch (apache::geode::client::QueryException &ex) {
          // ok, expecting an exception, do nothing
          fprintf(stdout, "Got expected exception: %s", ex.what());
        } catch (...) {
          ASSERT(false, "Got unexpected exception");
        }
      }
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  {
    LOG("cleanProc 1...");
    isPoolConfig = false;
    cleanProc();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer1)
  {
    LOG("closing Server1...");
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(LOCATOR, CloseLocator)
  {
    if (isLocator) {
      CacheHelper::closeLocator(1);
      LOG("Locator1 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_MAIN
  {
    CALL_TASK(StartLocator);
    CALL_TASK(CreateServerWithLocator);
    CALL_TASK(StepOnePoolLoc);
    CALL_TASK(StepThree);
    CALL_TASK(StepFour);
    CALL_TASK(StepFive);
    CALL_TASK(StepSix);
    CALL_TASK(GetAll);
    CALL_TASK(DoQuerySSError);
    CALL_TASK(CloseCache1);
    CALL_TASK(CloseServer1);
    CALL_TASK(CloseLocator);
  }
END_MAIN
