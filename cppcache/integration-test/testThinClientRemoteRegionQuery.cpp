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

#define ROOT_NAME "testThinClientRemoteRegionQuery"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "ThinClientHelper.hpp"

#include "QueryStrings.hpp"
#include "QueryHelper.hpp"

#include <geode/Query.hpp>
#include <geode/QueryService.hpp>

#include "SerializationRegistry.hpp"
#include "CacheRegionHelper.hpp"

#define CLIENT1 s1p1
#define LOCATOR s1p2
#define SERVER1 s2p1

using testData::QueryStrings;
using testData::regionQueries;
using testData::regionQueryRowCounts;
using testData::unsupported;

using apache::geode::client::IllegalStateException;
using apache::geode::client::QueryException;

bool isLocalServer = false;
bool isLocator = false;

const char* poolNames[] = {"Pool1", "Pool2", "Pool3"};
const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
static bool m_isPdx = false;
const char* qRegionNames[] = {"Portfolios", "Positions", "Portfolios2",
                              "Portfolios3"};

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

DUNIT_TASK_DEFINITION(CLIENT1, StepOnePoolLocator)
  {
    initClient(true);
    LOG("StepOnePoolLocator");
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
    } catch (const IllegalStateException&) {
      // ignore exception
    }
    createPool(poolNames[0], locHostPort, {}, 0, true);
    createRegionAndAttachPool(qRegionNames[0], USE_ACK, poolNames[0]);
    createRegionAndAttachPool(qRegionNames[1], USE_ACK, poolNames[0]);

    createRegionAndAttachPool(qRegionNames[2], USE_ACK, poolNames[0]);
    createPool(poolNames[1], locHostPort, {}, 0, true);
    createRegionAndAttachPool(qRegionNames[3], USE_ACK, poolNames[1]);

    auto regptr = getHelper()->getRegion(qRegionNames[0]);
    auto subregPtr =
        regptr->createSubregion(qRegionNames[1], regptr->getAttributes());

    LOG("StepOne complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepTwo)
  {
    auto regPtr0 = getHelper()->getRegion(qRegionNames[0]);
    auto subregPtr0 = regPtr0->getSubregion(qRegionNames[1]);
    auto regPtr1 = getHelper()->getRegion(qRegionNames[1]);

    auto regPtr2 = getHelper()->getRegion(qRegionNames[2]);
    auto regPtr3 = getHelper()->getRegion(qRegionNames[3]);

    QueryHelper* qh = &QueryHelper::getHelper();

    char buf[100];
    sprintf(buf, "SetSize %zd, NumSets %zd", qh->getPortfolioSetSize(),
            qh->getPortfolioNumSets());
    LOG(buf);

    if (!m_isPdx) {
      qh->populatePortfolioData(regPtr0, qh->getPortfolioSetSize(),
                                qh->getPortfolioNumSets());
      qh->populatePositionData(subregPtr0, qh->getPositionSetSize(),
                               qh->getPositionNumSets());
      qh->populatePositionData(regPtr1, qh->getPositionSetSize(),
                               qh->getPositionNumSets());

      qh->populatePortfolioData(regPtr2, qh->getPortfolioSetSize(),
                                qh->getPortfolioNumSets());
      qh->populatePortfolioData(regPtr3, qh->getPortfolioSetSize(),
                                qh->getPortfolioNumSets());
    } else {
      qh->populatePortfolioPdxData(regPtr0, qh->getPortfolioSetSize(),
                                   qh->getPortfolioNumSets());
      qh->populatePositionPdxData(subregPtr0, qh->getPositionSetSize(),
                                  qh->getPositionNumSets());
      qh->populatePositionPdxData(regPtr1, qh->getPositionSetSize(),
                                  qh->getPositionNumSets());

      qh->populatePortfolioPdxData(regPtr2, qh->getPortfolioSetSize(),
                                   qh->getPortfolioNumSets());
      qh->populatePortfolioPdxData(regPtr3, qh->getPortfolioSetSize(),
                                   qh->getPortfolioNumSets());
    }
    LOG("StepTwo complete.\n");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    bool doAnyErrorOccured = false;
    auto region = getHelper()->getRegion(qRegionNames[0]);

    for (int i = 0; i < QueryStrings::RQsize(); i++) {
      if (m_isPdx) {
        if (i == 18) {
          LOG_INFO(
              "Skipping query index %d because it is unsupported for pdx type.",
              i);
          continue;
        }
      }

      if (regionQueries[i].category == unsupported) {
        continue;
      }

      auto results = region->query(regionQueries[i].query());

      if (results->size() != regionQueryRowCounts[i]) {
        std::string failmsg = "FAIL: Query #" + std::to_string(i) +
                              " expected result size is " +
                              std::to_string(regionQueryRowCounts[i]) +
                              ", actual is " + std::to_string(results->size());
        doAnyErrorOccured = true;
        LOG(failmsg);
        continue;
      }
    }

    try {
      auto results = region->query("");
      FAIL("Expected IllegalArgumentException exception for empty predicate");
    } catch (apache::geode::client::IllegalArgumentException& ex) {
      LOG("got expected IllegalArgumentException exception for empty "
          "predicate:");
      LOG(ex.what());
    }

    try {
      auto results = region->query((regionQueries[0].query()),
                                   std::chrono::seconds(2200000));
      FAIL("Expected IllegalArgumentException exception for invalid timeout");
    } catch (apache::geode::client::IllegalArgumentException& ex) {
      LOG("got expected IllegalArgumentException exception for invalid "
          "timeout:");
      LOG(ex.what());
    }

    try {
      auto results =
          region->query((regionQueries[0].query()), std::chrono::seconds(-1));
      FAIL("Expected IllegalArgumentException exception for invalid timeout");
    } catch (apache::geode::client::IllegalArgumentException& ex) {
      LOG("got expected IllegalArgumentException exception for invalid "
          "timeout:");
      LOG(ex.what());
    }
    try {
      auto results = region->query("bad predicate");
      FAIL("Expected QueryException exception for wrong predicate");
    } catch (QueryException& ex) {
      LOG("got expected QueryException exception for wrong predicate:");
      LOG(ex.what());
    }

    if (!doAnyErrorOccured) {
      LOG("ALL QUERIES PASSED");
    } else {
      LOG("QUERY ERROR(S) OCCURED");
      FAIL("QUERY ERROR(S) OCCURED");
    }

    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFour)
  {
    auto region = getHelper()->getRegion(qRegionNames[0]);

    for (int i = 0; i < QueryStrings::RQsize(); i++) {
      if (regionQueries[i].category == unsupported) {
        continue;
      }

      bool existsValue = region->existsValue((regionQueries[i].query()));

      bool expectedResult = regionQueryRowCounts[i] > 0 ? true : false;

      if (existsValue != expectedResult) {
        std::string failmsg = "FAIL: Query #" + std::to_string(i) +
                              " existsValue expected is " +
                              (expectedResult ? "true" : "false") +
                              ", actual is " + (existsValue ? "true" : "false");
        ASSERT(false, failmsg);
      }
    }

    try {
      region->existsValue("");
      FAIL("Expected IllegalArgumentException exception for empty predicate");
    } catch (apache::geode::client::IllegalArgumentException& ex) {
      LOG("got expected IllegalArgumentException exception for empty "
          "predicate:");
      LOG(ex.what());
    }

    try {
      region->existsValue((regionQueries[0].query()),
                          std::chrono::seconds(2200000));
      FAIL("Expected IllegalArgumentException exception for invalid timeout");
    } catch (apache::geode::client::IllegalArgumentException& ex) {
      LOG("got expected IllegalArgumentException exception for invalid "
          "timeout:");
      LOG(ex.what());
    }

    try {
      region->existsValue((regionQueries[0].query()), std::chrono::seconds(-1));
      FAIL("Expected IllegalArgumentException exception for invalid timeout");
    } catch (apache::geode::client::IllegalArgumentException& ex) {
      LOG("got expected IllegalArgumentException exception for invalid "
          "timeout:");
      LOG(ex.what());
    }
    try {
      region->existsValue("bad predicate");
      FAIL("Expected QueryException exception for wrong predicate");
    } catch (QueryException& ex) {
      LOG("got expected QueryException exception for wrong predicate:");
      LOG(ex.what());
    }

    LOG("StepFour complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFive)
  {
    bool doAnyErrorOccured = false;

    auto region = getHelper()->getRegion(qRegionNames[0]);

    for (int i = 0; i < QueryStrings::RQsize(); i++) {
      if (regionQueries[i].category == unsupported) {
        continue;
      }

      try {
        auto result = region->selectValue(regionQueries[i].query());
        if (!(regionQueryRowCounts[i] == 0 || regionQueryRowCounts[i] == 1)) {
          std::string logmsg = "FAIL: Query #" + std::to_string(i) +
                               " expected query exception did not occur";

          LOG(logmsg);
          doAnyErrorOccured = true;
        }
      } catch (const QueryException&) {
        if (regionQueryRowCounts[i] == 0 || regionQueryRowCounts[i] == 1) {
          std::string logmsg = "FAIL: Query #" + std::to_string(i) +
                               " unexpected query exception occurred";

          LOG(logmsg);
          doAnyErrorOccured = true;
        }
      } catch (...) {
        std::string logmsg = "FAIL: Query #" + std::to_string(i) +
                             " unexpected exception occurred";

        LOG(logmsg);
        FAIL(logmsg);
      }
    }

    try {
      auto results =
          std::dynamic_pointer_cast<SelectResults>(region->selectValue(""));
      FAIL("Expected IllegalArgumentException exception for empty predicate");
    } catch (apache::geode::client::IllegalArgumentException& ex) {
      LOG("got expected IllegalArgumentException exception for empty "
          "predicate:");
      LOG(ex.what());
    }

    try {
      auto results =
          std::dynamic_pointer_cast<SelectResults>(region->selectValue(
              (regionQueries[0].query()), std::chrono::seconds(2200000)));
      FAIL("Expected IllegalArgumentException exception for invalid timeout");
    } catch (apache::geode::client::IllegalArgumentException& ex) {
      LOG("got expected IllegalArgumentException exception for invalid "
          "timeout:");
      LOG(ex.what());
    }

    try {
      auto results =
          std::dynamic_pointer_cast<SelectResults>(region->selectValue(
              (regionQueries[0].query()), std::chrono::seconds(-1)));
      FAIL("Expected IllegalArgumentException exception for invalid timeout");
    } catch (apache::geode::client::IllegalArgumentException& ex) {
      LOG("got expected IllegalArgumentException exception for invalid "
          "timeout:");
      LOG(ex.what());
    }
    try {
      auto results = std::dynamic_pointer_cast<SelectResults>(
          region->selectValue("bad predicate"));
      FAIL("Expected IllegalArgumentException exception for wrong predicate");
    } catch (QueryException& ex) {
      LOG("got expected QueryException for wrong predicate:");
      LOG(ex.what());
    }
    if (!doAnyErrorOccured) {
      LOG("ALL QUERIES PASSED");
    } else {
      LOG("QUERY ERROR(S) OCCURED");
      FAIL("QUERY ERROR(S) OCCURED");
    }

    LOG("StepFive complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, QueryError)
  {
    auto region = getHelper()->getRegion(qRegionNames[0]);

    for (int i = 0; i < QueryStrings::RQsize(); i++) {
      if ((regionQueries[i].category != unsupported) ||
          (i == 3))  // UNDEFINED case
      {
        continue;
      }

      try {
        auto results = region->query(regionQueries[i].query());
        std::string failmsg =
            "Query exception didnt occur for index " + std::to_string(i);

        LOG(failmsg);
        FAIL(failmsg);
      } catch (apache::geode::client::QueryException&) {
        // ok, expecting an exception, do nothing
      } catch (...) {
        LOG("Got unexpected exception");
        FAIL("Got unexpected exception");
      }
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  {
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

void runRemoteRegionQueryTest() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator);
  CALL_TASK(StepOnePoolLocator);

  CALL_TASK(StepTwo);
  CALL_TASK(StepThree);
  CALL_TASK(StepFour);
  CALL_TASK(StepFive);
  CALL_TASK(QueryError);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseServer1);

  CALL_TASK(CloseLocator);
}

void setPortfolioPdxType() { CALL_TASK(SetPortfolioTypeToPdx); }

void UnsetPortfolioType() { CALL_TASK(UnsetPortfolioTypeToPdx); }

DUNIT_MAIN
  {
    // Basic Old Test
    // runRemoteRegionQueryTest();

    UnsetPortfolioType();
    for (int runIdx = 1; runIdx <= 2; ++runIdx) {
      runRemoteRegionQueryTest();
      setPortfolioPdxType();
    }
  }
END_MAIN
