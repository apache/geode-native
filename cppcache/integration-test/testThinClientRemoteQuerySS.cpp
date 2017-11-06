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
#include <geode/GeodeCppCache.hpp>
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

using namespace apache::geode::client;
using namespace test;
using namespace testData;

#define CLIENT1 s1p1
#define LOCATOR s1p2
#define SERVER1 s2p1

bool isLocator = false;
bool isLocalServer = false;

const char* poolNames[] = {"Pool1", "Pool2", "Pool3"};
const char* locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
bool isPoolConfig = false;  // To track if pool case is running
const char* qRegionNames[] = {"Portfolios", "Positions", "Portfolios2",
                              "Portfolios3"};

const char* checkNullString(const char* str) {
  return ((str == nullptr) ? "(null)" : str);
}

const wchar_t* checkNullString(const wchar_t* str) {
  return ((str == nullptr) ? L"(null)" : str);
}

std::string checkNullString(const std::string* str) {
    return ((str == nullptr) ? "(null)" : *str);
}

void _printFields(CacheablePtr field, Struct* ssptr, int32_t& fields) {
 try {
  if (auto portfolio = std::dynamic_pointer_cast<Portfolio>(field)) {
  printf("   pulled %s :- ID %d, pkid %s\n",
           ssptr->getFieldName(fields).c_str(), portfolio->getID(),
           checkNullString(portfolio->getPkid()->asChar()));
    printf("   pulled %s :- ID %d, pkid %s\n",
           ssptr->getFieldName(fields).c_str(), portfolio->getID(),
           checkNullString(portfolio->getPkid()->asChar()));
  } else if (auto position = std::dynamic_pointer_cast<Position>(field)) {
    printf("   pulled %s :- secId %s, shares %d\n",
           ssptr->getFieldName(fields).c_str(),
           checkNullString(position->getSecId()->asChar()),
           position->getSharesOutstanding());
  } else if (auto portfolioPdx =
                 std::dynamic_pointer_cast<PortfolioPdx>(field)) {
    printf("   pulled %s :- ID %d, pkid %s\n",
           ssptr->getFieldName(fields).c_str(),
           portfolioPdx->getID(),
           checkNullString(portfolioPdx->getPkid()));
  } else if (auto positionPdx = std::dynamic_pointer_cast<PositionPdx>(field)) {
    printf("   pulled %s :- secId %s, shares %d\n",
           ssptr->getFieldName(fields).c_str(),
           checkNullString(positionPdx->getSecId()),
           positionPdx->getSharesOutstanding());
  } else {
    if (auto str = std::dynamic_pointer_cast<CacheableString>(field)) {
      if (str->isWideString()) {
        printf("   pulled %s :- %S\n",
               ssptr->getFieldName(fields).c_str(),
               checkNullString(str->asWChar()));
      } else {
        printf("   pulled %s :- %s\n",
               ssptr->getFieldName(fields).c_str(),
               checkNullString(str->asChar()));
      }
    } else if (auto boolptr =
                   std::dynamic_pointer_cast<CacheableBoolean>(field)) {
      printf("   pulled %s :- %s\n",
             ssptr->getFieldName(fields).c_str(),
             boolptr->toString()->asChar());
    } else {
      if (auto ptr = std::dynamic_pointer_cast<CacheableKey>(field)) {
        char buff[1024] = {'\0'};
        ptr->logString(&buff[0], 1024);
        printf("   pulled %s :- %s \n",
               ssptr->getFieldName(fields).c_str(), buff);
      } else if (auto strArr =
                     std::dynamic_pointer_cast<CacheableStringArray>(field)) {
        printf(" string array object printing \n\n");
        for (int stri = 0; stri < strArr->length(); stri++) {
          if (strArr->operator[](stri)->isWideString()) {
            printf("   pulled %s(%d) - %S \n",
                   ssptr->getFieldName(fields).c_str(), stri,
                   checkNullString(strArr->operator[](stri)->asWChar()));
          } else {
            printf("   pulled %s(%d) - %s \n",
                   ssptr->getFieldName(fields).c_str(), stri,
                   checkNullString(strArr->operator[](stri)->asChar()));
          }
        }
      } else if (auto map =
                     std::dynamic_pointer_cast<CacheableHashMap>(field)) {
        int index = 0;
        for (const auto& iter : *map) {
          printf("   hashMap %d of %zd ... \n", ++index, map->size());
          _printFields(iter.first, ssptr, fields);
          _printFields(iter.second, ssptr, fields);
        }
        printf("   end of map \n");
      } else if (auto structimpl = std::dynamic_pointer_cast<Struct>(field)) {
        printf("   structImpl %s {\n",
               ssptr->getFieldName(fields).c_str());
        for (int32_t inner_fields = 0; inner_fields < structimpl->length();
             inner_fields++) {
          SerializablePtr field = (*structimpl)[inner_fields];
          if (field == nullptr) {
            printf("we got null fields here, probably we have nullptr data\n");
            continue;
          }

          _printFields(field, structimpl.get(), inner_fields);

        }  // end of field iterations
        printf("   } //end of %s\n",
               ssptr->getFieldName(fields).c_str());
      } else {
        printf(
            "unknown field data.. couldn't even convert it to Cacheable "
            "variants\n");
      }
    }

  }  // end of else
  } catch (const std::out_of_range& e) {
    printf("Caught a non-fatal out_of_range exception: %s", e.what());
  }
}

void _verifyStructSet(StructSetPtr& ssptr, int i) {
  printf("query idx %d \n", i);
  for (int32_t rows = 0; rows < ssptr->size(); rows++) {
    if (rows > (int32_t)QueryHelper::getHelper().getPortfolioSetSize()) {
      continue;
    }

    Struct* siptr = dynamic_cast<Struct*>(((*ssptr)[rows]).get());
    if (siptr == nullptr) {
      printf("siptr is nullptr \n\n");
      continue;
    }

    printf("   Row : %d \n", rows);
    for (int32_t fields = 0; fields < siptr->length(); fields++) {
      SerializablePtr field = (*siptr)[fields];
      if (field == nullptr) {
        printf("we got null fields here, probably we have nullptr data\n");
        continue;
      }

      _printFields(field, siptr, fields);

    }  // end of field iterations
  }    // end of row iterations
}

void compareMaps(HashMapOfCacheable& map, HashMapOfCacheable& expectedMap) {
  ASSERT(expectedMap.size() == map.size(),
         "Unexpected number of entries in map");
  LOGINFO("Got expected number of %d entries in map", map.size());
  for (const auto& iter : map) {
    const auto& key = iter.first;
    const auto& val = iter.second;
    const auto& expectedIter = expectedMap.find(key);
    if (expectedIter == expectedMap.end()) {
      FAIL("Could not find expected key in map");
    }
    const auto& expectedVal = expectedIter->second;

    if (std::dynamic_pointer_cast<PositionPdx>(expectedVal)) {
      const PositionPdxPtr& posVal =
          std::dynamic_pointer_cast<PositionPdx>(val);
      const PositionPdxPtr& expectedPosVal =
          std::static_pointer_cast<PositionPdx>(expectedVal);
      ASSERT(*expectedPosVal->getSecId() == *posVal->getSecId(),
             "Expected the secIDs to be equal in PositionPdx");
      ASSERT(expectedPosVal->getSharesOutstanding() ==
                 posVal->getSharesOutstanding(),
             "Expected the sharesOutstanding to be equal in PositionPdx");
    } else {
      const PortfolioPdxPtr& portVal =
          std::dynamic_pointer_cast<PortfolioPdx>(val);
      const PortfolioPdxPtr& expectedPortVal =
          std::dynamic_pointer_cast<PortfolioPdx>(expectedVal);
      ASSERT(expectedPortVal->getID() == portVal->getID(),
             "Expected the IDs to be equal in PortfolioPdx");
      ASSERT(expectedPortVal->getNewValSize() == portVal->getNewValSize(),
             "Expected the sizes to be equal in PortfolioPdx");
    }
  }
}

void stepOne() {
  initGridClient(true);
  try {
    SerializationRegistryPtr serializationRegistry = CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())->getSerializationRegistry();

    serializationRegistry->addType(Position::createDeserializable);
    serializationRegistry->addType(Portfolio::createDeserializable);

    serializationRegistry->addPdxType(PositionPdx::createDeserializable);
    serializationRegistry->addPdxType(PortfolioPdx::createDeserializable);
  } catch (const IllegalStateException&) {
    // ignore exception
  }

  isPoolConfig = true;
  createPool(poolNames[0], locHostPort, nullptr, 0, true);
  createRegionAndAttachPool(qRegionNames[0], USE_ACK, poolNames[0]);
  createRegionAndAttachPool(qRegionNames[1], USE_ACK, poolNames[0]);
  createRegionAndAttachPool(qRegionNames[2], USE_ACK, poolNames[0]);
  createRegionAndAttachPool(qRegionNames[3], USE_ACK, poolNames[0]);

  RegionPtr regptr = getHelper()->getRegion(qRegionNames[0]);
  RegionAttributesPtr lattribPtr = regptr->getAttributes();
  RegionPtr subregPtr = regptr->createSubregion(qRegionNames[1], lattribPtr);

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

    auto* qh = &QueryHelper::getHelper();

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
    sprintf(buf, "SetSize %d, NumSets %d", qh->getPortfolioSetSize(),
            qh->getPortfolioNumSets());
    LOG(buf);

    LOG("StepThree complete.\n");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFour)
  {
    SLEEP(100);
    bool doAnyErrorOccured = false;
    auto* qh = &QueryHelper::getHelper();

    QueryServicePtr qs = nullptr;
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
        char failmsg[100] = {0};
        ACE_OS::sprintf(failmsg, "Query verify failed for query index %d", i);
        ASSERT(false, failmsg);
        continue;
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
    bool doAnyErrorOccured = false;
    auto qh = &QueryHelper::getHelper();

    QueryServicePtr qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }

    for (int i = 0; i < QueryStrings::SSsize(); i++) {
      if (i == 12 || i == 4 || i == 7 || i == 22 || i == 30 || i == 34) {
        LOGDEBUG("Skipping query index %d for pdx because it has function.", i);
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
          char failmsg[100] = {0};
          ACE_OS::sprintf(failmsg, "Query verify failed for query index %d", i);
          ASSERT(false, failmsg);
          continue;
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

    LOG("StepFive complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepSix)
  {
    SLEEP(100);
    bool doAnyErrorOccured = false;
    auto* qh = &QueryHelper::getHelper();

    QueryServicePtr qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }

    for (int i = 0; i < QueryStrings::SSPsize(); i++) {
      if (i == 16) {
        LOGDEBUG("Skipping query index %d for pdx because it has function.", i);
        continue;
      }

      if (structsetParamQueries[i].category != unsupported) {
        auto qry = qs->newQuery(structsetParamQueries[i].query());
        auto paramList = CacheableVector::create();

        for (int j = 0; j < numSSQueryParam[i]; j++) {
          // LOGINFO("NIL::SSPQ::328: queryparamSetSS[%d][%d] = %s", i, j,
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
          char failmsg[100] = {0};
          ACE_OS::sprintf(failmsg, "Query verify failed for query index %d", i);
          ASSERT(false, failmsg);
          continue;
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

    int numSecIds = sizeof(secIds) / sizeof(char*);

    VectorOfCacheableKey posKeys;
    VectorOfCacheableKey portKeys;
    HashMapOfCacheable expectedPosMap;
    HashMapOfCacheable expectedPortMap;

    auto& qh = QueryHelper::getHelper();
    int setSize = qh.getPositionSetSize();
    int numSets = qh.getPositionNumSets();

    for (int set = 1; set <= numSets; ++set) {
      for (int current = 1; current <= setSize; ++current) {
        char posname[100] = {0};
        ACE_OS::sprintf(posname, "pos%d-%d", set, current);

        auto posKey(CacheableKey::create(posname));
        auto pos = std::make_shared<PositionPdx>(secIds[current % numSecIds],
                                                 current * 100);

        posKeys.push_back(posKey);
        expectedPosMap.emplace(posKey, pos);
      }
    }

    // reset the counter for uniform population of position objects
    PositionPdx::resetCounter();

    setSize = qh.getPortfolioSetSize();
    numSets = qh.getPortfolioNumSets();
    for (int set = 1; set <= numSets; ++set) {
      for (int current = 1; current <= setSize; ++current) {
        char portname[100] = {0};
        ACE_OS::sprintf(portname, "port%d-%d", set, current);

        auto portKey = CacheableKey::create(portname);
        auto port = std::make_shared<PortfolioPdx>(current, 1);

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
    auto* qh ATTR_UNUSED = &QueryHelper::getHelper();

    QueryServicePtr qs = nullptr;
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

          char failmsg[100] = {0};
          ACE_OS::sprintf(failmsg, "Query exception didnt occur for index %d",
                          i);
          LOG(failmsg);
          FAIL(failmsg);
        } catch (apache::geode::client::QueryException& ex) {
          // ok, expecting an exception, do nothing
          fprintf(stdout, "Got expected exception: %s", ex.getMessage());
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
{CALL_TASK(StartLocator) CALL_TASK(CreateServerWithLocator)
     CALL_TASK(StepOnePoolLoc) CALL_TASK(StepThree) CALL_TASK(StepFour)
         CALL_TASK(StepFive) CALL_TASK(StepSix) CALL_TASK(GetAll)
             CALL_TASK(DoQuerySSError) CALL_TASK(CloseCache1)
                 CALL_TASK(CloseServer1) CALL_TASK(CloseLocator)} END_MAIN
