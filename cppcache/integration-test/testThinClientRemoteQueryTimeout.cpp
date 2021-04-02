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

#define ROOT_NAME "testThinClientRemoteQueryTimeout"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "ThinClientHelper.hpp"

#include "QueryStrings.hpp"
#include "QueryHelper.hpp"
#include "SerializationRegistry.hpp"
#include "CacheRegionHelper.hpp"

#include <geode/Query.hpp>
#include <geode/QueryService.hpp>

#include "SerializationRegistry.hpp"
#include "CacheRegionHelper.hpp"

#define CLIENT1 s1p1
#define LOCATOR s1p2
#define SERVER1 s2p1

using testData::numSSQueryParam;
using testData::queryparamSetSS;
using testData::resultsetQueries;
using testData::structsetParamQueries;
using testData::structsetQueries;

using apache::geode::client::Cacheable;
using apache::geode::client::CacheableVector;
using apache::geode::client::Exception;
using apache::geode::client::IllegalArgumentException;
using apache::geode::client::IllegalStateException;
using apache::geode::client::QueryService;
using apache::geode::client::TimeoutException;

bool isLocalServer = false;
bool isLocator = false;
const char *poolNames[] = {"Pool1", "Pool2", "Pool3"};
const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);

const char *qRegionNames[] = {"Portfolios", "Positions", "Portfolios2",
                              "Portfolios3"};

bool isPoolConfig = false;  // To track if pool case is running
static bool m_isPdx = false;
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
  }
  isPoolConfig = true;
  createPool(poolNames[0], locHostPort, {}, 0, true);
  createRegionAndAttachPool(qRegionNames[0], USE_ACK, poolNames[0]);

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

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
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

DUNIT_TASK_DEFINITION(CLIENT1, StepTwo)
  {
    auto regPtr0 = getHelper()->getRegion(qRegionNames[0]);
    auto subregPtr0 = regPtr0->getSubregion(qRegionNames[1]);

    QueryHelper *qh = &QueryHelper::getHelper();

    if (!m_isPdx) {
      qh->populatePortfolioData(regPtr0, 100, 20, 100);
      qh->populatePositionData(subregPtr0, 100, 20);
    } else {
      qh->populatePortfolioPdxData(regPtr0, 100, 20, 100);
      qh->populatePositionPdxData(subregPtr0, 100, 20);
    }

    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    QueryHelper::getHelper();

    std::shared_ptr<QueryService> qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }
    auto qry = qs->newQuery(resultsetQueries[34].query());

    std::shared_ptr<SelectResults> results;

    try {
      LOG("EXECUTE 1 START");

      results = qry->execute(std::chrono::milliseconds(100));

      LOG("EXECUTE 1 STOP");
      std::string logmsg = "Result size is " + std::to_string(results->size());

      LOG(logmsg);

      LOG("Didnt get expected timeout exception for first execute");
      FAIL("Didnt get expected timeout exception for first execute");
    } catch (const TimeoutException &excp) {
      std::string logmsg = "";
      logmsg += "First execute expected exception ";
      logmsg += excp.getName();
      logmsg += ": ";
      logmsg += excp.what();
      LOG(logmsg.c_str());
    }

    SLEEP(15000);

    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFour)
  {
    QueryHelper::getHelper();

    std::shared_ptr<QueryService> qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }
    auto qry = qs->newQuery((resultsetQueries[34].query()));

    std::shared_ptr<SelectResults> results;

    try {
      LOG("EXECUTE 2 START");

      results = qry->execute(std::chrono::seconds(850));

      LOG("EXECUTE 2 STOP");

      std::string logmsg = "Result size is " + std::to_string(results->size());
      LOG(logmsg);
    } catch (Exception &excp) {
      std::string failmsg = "";
      failmsg += "Second execute unwanted exception ";
      failmsg += excp.getName();
      failmsg += ": ";
      failmsg += excp.what();
      LOG(failmsg.c_str());
      FAIL(failmsg.c_str());
    }

    LOG("StepFour complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFive)
  {
    QueryHelper::getHelper();

    std::shared_ptr<QueryService> qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }
    auto qry = qs->newQuery(structsetQueries[17].query());

    std::shared_ptr<SelectResults> results;

    try {
      LOG("EXECUTE 3 START");

      results = qry->execute(std::chrono::milliseconds(100));

      LOG("EXECUTE 3 STOP");
      std::string logmsg = "Result size is " + std::to_string(results->size());

      LOG(logmsg);
      LOG("Didnt get expected timeout exception for third execute");
      FAIL("Didnt get expected timeout exception for third execute");
    } catch (const TimeoutException &excp) {
      std::string logmsg = "";
      logmsg += "Third execute expected exception ";
      logmsg += excp.getName();
      logmsg += ": ";
      logmsg += excp.what();
      LOG(logmsg);
    }

    SLEEP(40000);  // sleep to allow server query to complete

    LOG("StepFive complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepSix)
  {
    QueryHelper::getHelper();

    std::shared_ptr<QueryService> qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }
    auto qry = qs->newQuery(structsetQueries[17].query());

    std::shared_ptr<SelectResults> results;

    try {
      LOG("EXECUTE 4 START");

      results = qry->execute(std::chrono::seconds(850));

      LOG("EXECUTE 4 STOP");
      std::string logmsg = "Result size is " + std::to_string(results->size());

      LOG(logmsg);
    } catch (Exception &excp) {
      std::string failmsg = "";
      failmsg += "Fourth execute unwanted exception ";
      failmsg += excp.getName();
      failmsg += ": ";
      failmsg += excp.what();
      LOG(failmsg.c_str());
      FAIL(failmsg.c_str());
    }

    LOG("StepSix complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepSeven)
  {
    QueryHelper::getHelper();

    std::shared_ptr<QueryService> qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }
    auto qry = qs->newQuery(structsetParamQueries[5].query());

    std::shared_ptr<SelectResults> results;

    try {
      LOG("EXECUTE 5 START");

      auto paramList = CacheableVector::create();

      for (int j = 0; j < numSSQueryParam[5]; j++) {
        if (atoi(queryparamSetSS[5][j]) != 0) {
          paramList->push_back(Cacheable::create(atoi(queryparamSetSS[5][j])));
        } else {
          paramList->push_back(Cacheable::create(queryparamSetSS[5][j]));
        }
      }
      results = qry->execute(paramList, std::chrono::milliseconds(2000));

      LOG("EXECUTE Five STOP");
      std::string logmsg = "Result size is " + std::to_string(results->size());

      LOG(logmsg);

      LOG("Didnt get expected timeout exception for fifth execute");
      FAIL("Didnt get expected timeout exception for fifth execute");
    } catch (const TimeoutException &excp) {
      std::string logmsg = "";
      logmsg += "Fifth execute expected exception ";
      logmsg += excp.getName();
      logmsg += ": ";
      logmsg += excp.what();
      LOG(logmsg.c_str());
    }

    SLEEP(40000);  // sleep to allow server query to complete

    LOG("StepSeven complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepEight)
  {
    QueryHelper::getHelper();

    std::shared_ptr<QueryService> qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }
    auto qry = qs->newQuery((structsetParamQueries[5].query()));

    std::shared_ptr<SelectResults> results;

    try {
      LOG("EXECUTE 6 START");

      auto paramList = CacheableVector::create();

      for (int j = 0; j < numSSQueryParam[5]; j++) {
        if (atoi(queryparamSetSS[5][j]) != 0) {
          paramList->push_back(Cacheable::create(atoi(queryparamSetSS[5][j])));
        } else {
          paramList->push_back(Cacheable::create(queryparamSetSS[5][j]));
        }
      }

      results = qry->execute(paramList, std::chrono::seconds(850));

      LOG("EXECUTE 6 STOP");
      std::string logmsg = "Result size is " + std::to_string(results->size());

      LOG(logmsg);
    } catch (Exception &excp) {
      std::string failmsg = "";
      failmsg += "Sixth execute unwanted exception ";
      failmsg += excp.getName();
      failmsg += ": ";
      failmsg += excp.what();
      LOG(failmsg.c_str());
      FAIL(failmsg.c_str());
    }

    LOG("StepEight complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, verifyNegativeValueTimeout)
  {
    QueryHelper::getHelper();

    std::shared_ptr<QueryService> qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }
    auto qry = qs->newQuery((resultsetQueries[34].query()));

    std::shared_ptr<SelectResults> results;

    try {
      LOG("Task::verifyNegativeValueTimeout - EXECUTE 1 START");

      results = qry->execute(std::chrono::seconds(-3));

      LOG("Task::verifyNegativeValueTimeout - EXECUTE 1 STOP");
      std::string logmsg = "Result size is " + std::to_string(results->size());

      LOG(logmsg);

      LOG("Didnt get expected timeout exception for first execute");
      FAIL("Didnt get expected timeout exception for first execute");
    }

    catch (const IllegalArgumentException &excp) {
      std::string logmsg = "";
      logmsg += "execute expected exception ";
      logmsg += excp.getName();
      logmsg += ": ";
      logmsg += excp.what();
      LOG(logmsg.c_str());
    }

    SLEEP(15000);

    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, verifyLargeValueTimeout)
  {
    QueryHelper::getHelper();

    std::shared_ptr<QueryService> qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }
    auto qry = qs->newQuery((resultsetQueries[34].query()));

    std::shared_ptr<SelectResults> results;

    try {
      LOG("Task:: verifyLargeValueTimeout - EXECUTE 1 START");

      results = qry->execute(std::chrono::seconds(2147500));

      LOG("Task:: verifyLargeValueTimeout - EXECUTE 1 STOP");
      std::string logmsg = "Result size is " + std::to_string(results->size());

      LOG(logmsg);

      LOG("Didnt get expected timeout exception for first execute");
      FAIL("Didnt get expected timeout exception for first execute");
    }

    catch (const IllegalArgumentException &excp) {
      std::string logmsg = "";
      logmsg += "execute expected exception ";
      logmsg += excp.getName();
      logmsg += ": ";
      logmsg += excp.what();
      LOG(logmsg.c_str());
    }

    SLEEP(15000);

    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  {
    isPoolConfig = false;
    LOG("cleanProc 1...");
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

DUNIT_TASK_DEFINITION(CLIENT1, SetPortfolioTypeToPdx)
  { m_isPdx = true; }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, UnsetPortfolioTypeToPdx)
  { m_isPdx = false; }
END_TASK_DEFINITION

void runRemoteQueryTimeoutTest() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator);
  CALL_TASK(StepOnePoolLoc);

  CALL_TASK(StepTwo);
  CALL_TASK(StepThree);
  CALL_TASK(StepFour);
  CALL_TASK(StepFive);
  CALL_TASK(StepSix);
  CALL_TASK(StepSeven);
  CALL_TASK(StepEight);
  CALL_TASK(verifyNegativeValueTimeout);
  CALL_TASK(verifyLargeValueTimeout);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseServer1);

  CALL_TASK(CloseLocator);
}

void setPortfolioPdxType() { CALL_TASK(SetPortfolioTypeToPdx); }

void UnsetPortfolioType() { CALL_TASK(UnsetPortfolioTypeToPdx); }

DUNIT_MAIN
  {
    UnsetPortfolioType();

    for (int runIdx = 1; runIdx <= 2; ++runIdx) {
      // New Test with Pool + EP
      runRemoteQueryTimeoutTest();
      setPortfolioPdxType();
    }
  }
END_MAIN
