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

#define ROOT_NAME "testThinClientRemoteQueryRS"
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

using testData::noofQueryParam;
using testData::queryparamSet;
using testData::QueryStrings;
using testData::resultsetparamQueries;
using testData::resultsetQueries;
using testData::resultsetQueriesOPL;
using testData::resultsetRowCounts;
using testData::resultsetRowCountsOPL;
using testData::resultsetRowCountsPQ;
using testData::unsupported;

using apache::geode::client::Cacheable;
using apache::geode::client::CacheableVector;
using apache::geode::client::IllegalStateException;
using apache::geode::client::QueryService;

bool isLocator = false;
bool isLocalServer = false;

const char *poolNames[] = {"Pool1", "Pool2", "Pool3"};
const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
bool isPoolConfig = false;  // To track if pool case is running
const char *qRegionNames[] = {"Portfolios", "Positions", "Portfolios2",
                              "Portfolios3"};
static bool m_isPdx = false;
void stepOne() {
  // Create just one pool and attach all regions to that.
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

DUNIT_TASK_DEFINITION(CLIENT1, SetPortfolioTypeToPdx)
  { m_isPdx = true; }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, UnsetPortfolioTypeToPdx)
  { m_isPdx = false; }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    auto regPtr0 = getHelper()->getRegion(qRegionNames[0]);
    auto subregPtr0 = regPtr0->getSubregion("Positions");
    auto regPtr1 = getHelper()->getRegion(qRegionNames[1]);

    auto regPtr2 = getHelper()->getRegion(qRegionNames[2]);
    auto regPtr3 = getHelper()->getRegion(qRegionNames[3]);

    QueryHelper *qh = &QueryHelper::getHelper();

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
      LOG("StepThree complete:Done populating Portfolio/Position object\n");
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
    LOG("StepThree complete.\n");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFour)
  {
    SLEEP(500);
    bool doAnyErrorOccured = false;
    QueryHelper *qh = &QueryHelper::getHelper();

    std::shared_ptr<QueryService> qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }
    for (int i = 0; i < QueryStrings::RSOPLsize(); i++) {
      auto qry = qs->newQuery(resultsetQueriesOPL[i].query());
      auto results = qry->execute();
      if (!qh->verifyRS(results, resultsetRowCountsOPL[i])) {
        ASSERT(false,
               "Query verify failed for query index " + std::to_string(i));
      }

      auto rsptr = std::dynamic_pointer_cast<ResultSet>(results);
      for (size_t rows = 0; rows < rsptr->size(); rows++) {
        if (rows > QueryHelper::getHelper().getPortfolioSetSize()) {
          continue;
        }

        if (!m_isPdx) {
          auto ser = (*rsptr)[rows];
          if (auto portfolio = std::dynamic_pointer_cast<Portfolio>(ser)) {
            printf(
                "   query idx %d pulled portfolio object ID %d, pkid  :: %s\n",
                i, portfolio->getID(), portfolio->getPkid()->value().c_str());
          } else if (auto position = std::dynamic_pointer_cast<Position>(ser)) {
            printf(
                "   query idx %d pulled position object secId %s, shares  :: "
                "%d\n",
                i, position->getSecId()->value().c_str(),
                position->getSharesOutstanding());
          } else {
            if (ser != nullptr) {
              printf(" query idx %d pulled object %s \n", i,
                     ser->toString().c_str());
            } else {
              printf("   query idx %d pulled bad object \n", i);
              FAIL("Unexpected object received in query");
            }
          }
        } else {
          auto pdxser = (*rsptr)[rows];
          if (auto portfoliopdx =
                  std::dynamic_pointer_cast<PortfolioPdx>(pdxser)) {
            printf(
                "   query idx %d pulled portfolioPdx object ID %d, pkid %s  :: "
                "\n",
                i, portfoliopdx->getID(), portfoliopdx->getPkid().c_str());
          } else if (auto positionpdx =
                         std::dynamic_pointer_cast<PositionPdx>(pdxser)) {
            printf(
                "   query idx %d pulled positionPdx object secId %s, shares %d "
                " "
                ":: \n",
                i, positionpdx->getSecId().c_str(),
                positionpdx->getSharesOutstanding());
          } else {
            if (pdxser != nullptr) {
              printf(" query idx %d pulled object %s  :: \n", i,
                     pdxser->toString().c_str());
            } else {
              printf("   query idx %d pulled bad object  :: \n", i);
              FAIL("Unexpected object received in query");
            }
          }
        }
      }
    }

    if (!doAnyErrorOccured) {
      printf("HURRAY !! StepFour PASSED \n\n");
    } else {
      FAIL("Failed in StepFour verification");
    }

    LOG("StepFour complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFive)
  {
    SLEEP(500);
    bool doAnyErrorOccured = false;
    QueryHelper *qh = &QueryHelper::getHelper();

    std::shared_ptr<QueryService> qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }
    for (int i = 0; i < QueryStrings::RSsize(); i++) {
      if (m_isPdx == true) {
        if (i == 2 || i == 3 || i == 4) {
          LOG_INFO(
              "Skipping query index %d for Pdx because it is function type.",
              i);
          continue;
        }
      }

      if (resultsetQueries[i].category != unsupported) {
        auto qry = qs->newQuery(resultsetQueries[i].query());
        auto results = qry->execute();
        if (!qh->verifyRS(results, (qh->isExpectedRowsConstantRS(i)
                                        ? resultsetRowCounts[i]
                                        : resultsetRowCounts[i] *
                                              qh->getPortfolioNumSets()))) {
          std::string failmsg =
              "Query verify failed for query index " + std::to_string(i);
          ASSERT(false, failmsg);
        }

        auto rsptr = std::dynamic_pointer_cast<ResultSet>(results);
        for (size_t rows = 0; rows < rsptr->size(); rows++) {
          if (rows > QueryHelper::getHelper().getPortfolioSetSize()) {
            continue;
          }

          if (!m_isPdx) {
            auto ser = (*rsptr)[rows];
            if (auto portfolio = std::dynamic_pointer_cast<Portfolio>(ser)) {
              printf(
                  "   query idx %d pulled portfolio object ID %d, pkid  :: "
                  "%s\n",
                  i, portfolio->getID(), portfolio->getPkid()->value().c_str());
            } else if (auto position =
                           std::dynamic_pointer_cast<Position>(ser)) {
              printf(
                  "   query idx %d pulled position object secId %s, shares  :: "
                  "%d\n",
                  i, position->getSecId()->value().c_str(),
                  position->getSharesOutstanding());
            } else {
              if (ser != nullptr) {
                printf(" query idx %d pulled object %s \n", i,
                       ser->toString().c_str());
              } else {
                printf("   query idx %d pulled bad object \n", i);
                FAIL("Unexpected object received in query");
              }
            }
          } else {
            auto pdxser = (*rsptr)[rows];
            if (auto portfoliopdx =
                    std::dynamic_pointer_cast<PortfolioPdx>(pdxser)) {
              printf(
                  "   query idx %d pulled portfolioPdx object ID %d, pkid %s  "
                  ":: "
                  "\n",
                  i, portfoliopdx->getID(), portfoliopdx->getPkid().c_str());
            } else if (auto positionpdx =
                           std::dynamic_pointer_cast<PositionPdx>(pdxser)) {
              printf(
                  "   query idx %d pulled positionPdx object secId %s, shares "
                  "%d "
                  " :: \n",
                  i, positionpdx->getSecId().c_str(),
                  positionpdx->getSharesOutstanding());
            } else {
              if (pdxser != nullptr) {
                printf(" query idx %d pulled object %s  :: \n", i,
                       pdxser->toString().c_str());
              } else {
                printf("   query idx %d pulled bad object  :: \n", i);
                FAIL("Unexpected object received in query");
              }
            }
          }
        }
      }
    }

    if (!doAnyErrorOccured) {
      printf("HURRAY !! We PASSED \n\n");
    } else {
      FAIL("Failed in StepFive verification");
    }

    LOG("StepFive complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepSix)
  {
    SLEEP(500);
    bool doAnyErrorOccured = false;
    QueryHelper *qh = &QueryHelper::getHelper();

    std::shared_ptr<QueryService> qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }

    for (int i = 0; i < QueryStrings::RSPsize(); i++) {
      if (resultsetparamQueries[i].category != unsupported) {
        auto qry = qs->newQuery(resultsetparamQueries[i].query());
        // LOG_INFO("NIL::229:Retrieved QueryString = %s",
        // qry->getQueryString());

        auto paramList = CacheableVector::create();
        for (int j = 0; j < noofQueryParam[i]; j++) {
          // LOG_INFO("NIL::260: queryparamSet[%d][%d] = %s", i, j,
          // queryparamSet[i][j]);
          if (atoi(queryparamSet[i][j]) != 0) {
            paramList->push_back(Cacheable::create(atoi(queryparamSet[i][j])));
          } else {
            paramList->push_back(Cacheable::create(queryparamSet[i][j]));
          }
        }

        auto results = qry->execute(paramList);
        if (!qh->verifyRS(results, (qh->isExpectedRowsConstantPQRS(i)
                                        ? resultsetRowCountsPQ[i]
                                        : resultsetRowCountsPQ[i] *
                                              qh->getPortfolioNumSets()))) {
          std::string failmsg =
              "Query verify failed for query index " + std::to_string(i);
          ASSERT(false, failmsg);
        }

        auto rsptr = std::dynamic_pointer_cast<ResultSet>(results);
        for (size_t rows = 0; rows < rsptr->size(); rows++) {
          if (rows > QueryHelper::getHelper().getPortfolioSetSize()) {
            continue;
          }

          if (!m_isPdx) {
            auto ser = (*rsptr)[rows];
            if (auto portfolio = std::dynamic_pointer_cast<Portfolio>(ser)) {
              printf(
                  "   query idx %d pulled portfolio object ID %d, pkid %s : \n",
                  i, portfolio->getID(), portfolio->getPkid()->value().c_str());
            } else if (auto position =
                           std::dynamic_pointer_cast<Position>(ser)) {
              printf(
                  "   query idx %d pulled position object secId %s, shares %d  "
                  ": "
                  "\n",
                  i, position->getSecId()->value().c_str(),
                  position->getSharesOutstanding());
            } else {
              if (ser != nullptr) {
                printf(" query idx %d pulled object %s  : \n", i,
                       ser->toString().c_str());
              } else {
                printf("   query idx %d pulled bad object  \n", i);
                FAIL("Unexpected object received in query");
              }
            }
          } else {
            auto ser = (*rsptr)[rows];
            if (auto portfoliopdx =
                    std::dynamic_pointer_cast<PortfolioPdx>(ser)) {
              printf(
                  "   query idx %d pulled portfolioPdx object ID %d, pkid %s  "
                  ": "
                  "\n",
                  i, portfoliopdx->getID(), portfoliopdx->getPkid().c_str());
            } else if (auto positionpdx =
                           std::dynamic_pointer_cast<PositionPdx>(ser)) {
              printf(
                  "   query idx %d pulled positionPdx object secId %s, shares "
                  "%d "
                  " : \n",
                  i, positionpdx->getSecId().c_str(),
                  positionpdx->getSharesOutstanding());
            } else {
              if (ser != nullptr) {
                printf(" query idx %d pulled object %s : \n", i,
                       ser->toString().c_str());
              } else {
                printf("   query idx %d pulled bad object\n", i);
                FAIL("Unexpected object received in query");
              }
            }
          }
        }
      }
    }

    if (!doAnyErrorOccured) {
      printf("HURRAY !! We PASSED \n\n");
    } else {
      FAIL("Failed in StepSix verification");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, DoQueryRSError)
  {
    QueryHelper::getHelper();

    std::shared_ptr<QueryService> qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }

    for (int i = 0; i < QueryStrings::RSsize(); i++) {
      if (resultsetQueries[i].category == unsupported) {
        auto qry = qs->newQuery(resultsetQueries[i].query());

        try {
          auto results = qry->execute();
          std::string failmsg =
              "Query exception didnt occur for index " + std::to_string(i);

          LOG(failmsg);
          FAIL(failmsg);
        } catch (apache::geode::client::QueryException &) {
          // ok, expecting an exception, do nothing
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

DUNIT_TASK_DEFINITION(SERVER1, CloseServer)
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

void runRemoteQueryRSTest() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator);
  CALL_TASK(StepOnePoolLoc);

  CALL_TASK(StepThree);
  CALL_TASK(StepFour);
  CALL_TASK(StepFive);
  CALL_TASK(StepSix);
  CALL_TASK(DoQueryRSError);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseServer);

  CALL_TASK(CloseLocator);
}

void setPortfolioPdxType() { CALL_TASK(SetPortfolioTypeToPdx); }

void UnsetPortfolioType() { CALL_TASK(UnsetPortfolioTypeToPdx); }

DUNIT_MAIN
  {
    for (int i = 0; i < 2; i++) {
      runRemoteQueryRSTest();
      setPortfolioPdxType();
    }
  }
END_MAIN
