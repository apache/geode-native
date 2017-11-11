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
static bool m_isPdx = false;
void stepOne() {
  // Create just one pool and attach all regions to that.
  initClient(true);
  try {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    serializationRegistry->addType(Position::createDeserializable);
    serializationRegistry->addType(Portfolio::createDeserializable);
    serializationRegistry->addPdxType(PositionPdx::createDeserializable);
    serializationRegistry->addPdxType(PortfolioPdx::createDeserializable);
  }
  catch (const IllegalStateException&) {
    // ignore exception
  }
  isPoolConfig = true;
  createPool(poolNames[0], locHostPort, nullptr, 0, true);
  createRegionAndAttachPool(qRegionNames[0], USE_ACK, poolNames[0]);
  createRegionAndAttachPool(qRegionNames[1], USE_ACK, poolNames[0]);
  createRegionAndAttachPool(qRegionNames[2], USE_ACK, poolNames[0]);
  createRegionAndAttachPool(qRegionNames[3], USE_ACK, poolNames[0]);

  auto regptr = getHelper()->getRegion(qRegionNames[0]);
  auto lattribPtr = regptr->getAttributes();
  auto subregPtr = regptr->createSubregion(qRegionNames[1], lattribPtr);

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

    QueryHelper* qh = &QueryHelper::getHelper();

    char buf[100];
    sprintf(buf, "SetSize %d, NumSets %d", qh->getPortfolioSetSize(),
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
    QueryHelper* qh = &QueryHelper::getHelper();

    std::shared_ptr<QueryService> qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }
    for (int i = 0; i < QueryStrings::RSOPLsize(); i++) {
      auto qry =
          qs->newQuery(const_cast<char*>(resultsetQueriesOPL[i].query()));
      std::shared_ptr<SelectResults> results = qry->execute();
      if (!qh->verifyRS(results, resultsetRowCountsOPL[i])) {
        char failmsg[100] = {0};
        ACE_OS::sprintf(failmsg, "Query verify failed for query index %d", i);
        doAnyErrorOccured = true;
        ASSERT(false, failmsg);
        continue;
      }

      auto rsptr = std::dynamic_pointer_cast<ResultSet>(results);
      SelectResultsIterator iter = rsptr->getIterator();
      for (int32_t rows = 0; rows < rsptr->size(); rows++) {
        if (rows > (int32_t)QueryHelper::getHelper().getPortfolioSetSize()) {
          continue;
        }

        if (!m_isPdx) {
          std::shared_ptr<Serializable> ser = (*rsptr)[rows];
          if (std::dynamic_pointer_cast<Portfolio>(ser)) {
            auto portfolio = std::static_pointer_cast<Portfolio>(ser);
            printf(
                "   query idx %d pulled portfolio object ID %d, pkid  :: %s\n",
                i, portfolio->getID(), portfolio->getPkid()->asChar());
          } else if (std::dynamic_pointer_cast<Position>(ser)) {
            auto position = std::static_pointer_cast<Position>(ser);
            printf(
                "   query idx %d pulled position object secId %s, shares  :: "
                "%d\n",
                i, position->getSecId()->asChar(),
                position->getSharesOutstanding());
          } else {
            if (ser != nullptr) {
              printf(" query idx %d pulled object %s \n", i,
                     ser->toString()->asChar());
            } else {
              printf("   query idx %d pulled bad object \n", i);
              FAIL("Unexpected object received in query");
            }
          }
        } else {
          std::shared_ptr<Serializable> pdxser = (*rsptr)[rows];
          if (std::dynamic_pointer_cast<PortfolioPdx>(pdxser)) {
            auto portfoliopdx = std::static_pointer_cast<PortfolioPdx>(pdxser);
            printf(
                "   query idx %d pulled portfolioPdx object ID %d, pkid %s  :: "
                "\n",
                i, portfoliopdx->getID(), portfoliopdx->getPkid());
          } else if (std::dynamic_pointer_cast<PositionPdx>(pdxser)) {
            auto positionpdx = std::static_pointer_cast<PositionPdx>(pdxser);
            printf(
                "   query idx %d pulled positionPdx object secId %s, shares %d "
                " "
                ":: \n",
                i, positionpdx->getSecId(),
                positionpdx->getSharesOutstanding());
          } else {
            if (pdxser != nullptr) {
              if (pdxser->toString()->isWideString()) {
                printf(" query idx %d pulled object %S  :: \n", i,
                       pdxser->toString()->asWChar());
              } else {
                printf(" query idx %d pulled object %s  :: \n", i,
                       pdxser->toString()->asChar());
              }
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
    QueryHelper* qh = &QueryHelper::getHelper();

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
          LOGINFO(
              "Skipping query index %d for Pdx because it is function type.",
              i);
          continue;
        }
      }

      if (resultsetQueries[i].category != unsupported) {
        auto qry = qs->newQuery(const_cast<char*>(resultsetQueries[i].query()));
        std::shared_ptr<SelectResults> results = qry->execute();
        if (!qh->verifyRS(results, (qh->isExpectedRowsConstantRS(i)
                                        ? resultsetRowCounts[i]
                                        : resultsetRowCounts[i] *
                                              qh->getPortfolioNumSets()))) {
          char failmsg[100] = {0};
          ACE_OS::sprintf(failmsg, "Query verify failed for query index %d", i);
          doAnyErrorOccured = true;
          ASSERT(false, failmsg);
          continue;
        }

        auto rsptr = std::dynamic_pointer_cast<ResultSet>(results);
        SelectResultsIterator iter = rsptr->getIterator();
        for (int32_t rows = 0; rows < rsptr->size(); rows++) {
          if (rows > (int32_t)QueryHelper::getHelper().getPortfolioSetSize()) {
            continue;
          }

          if (!m_isPdx) {
            std::shared_ptr<Serializable> ser = (*rsptr)[rows];
            if (std::dynamic_pointer_cast<Portfolio>(ser)) {
              auto portfolio = std::static_pointer_cast<Portfolio>(ser);
              printf(
                  "   query idx %d pulled portfolio object ID %d, pkid  :: "
                  "%s\n",
                  i, portfolio->getID(), portfolio->getPkid()->asChar());
            } else if (std::dynamic_pointer_cast<Position>(ser)) {
              auto position = std::static_pointer_cast<Position>(ser);
              printf(
                  "   query idx %d pulled position object secId %s, shares  :: "
                  "%d\n",
                  i, position->getSecId()->asChar(),
                  position->getSharesOutstanding());
            } else {
              if (ser != nullptr) {
                printf(" query idx %d pulled object %s \n", i,
                       ser->toString()->asChar());
              } else {
                printf("   query idx %d pulled bad object \n", i);
                FAIL("Unexpected object received in query");
              }
            }
          } else {
            std::shared_ptr<Serializable> pdxser = (*rsptr)[rows];
            if (std::dynamic_pointer_cast<PortfolioPdx>(pdxser)) {
              auto portfoliopdx =
                  std::static_pointer_cast<PortfolioPdx>(pdxser);
              printf(
                  "   query idx %d pulled portfolioPdx object ID %d, pkid %s  "
                  ":: "
                  "\n",
                  i, portfoliopdx->getID(), portfoliopdx->getPkid());
            } else if (std::dynamic_pointer_cast<PositionPdx>(pdxser)) {
              auto positionpdx = std::static_pointer_cast<PositionPdx>(pdxser);
              printf(
                  "   query idx %d pulled positionPdx object secId %s, shares "
                  "%d "
                  " :: \n",
                  i, positionpdx->getSecId(),
                  positionpdx->getSharesOutstanding());
            } else {
              if (pdxser != nullptr) {
                if (pdxser->toString()->isWideString()) {
                  printf(" query idx %d pulled object %S  :: \n", i,
                         pdxser->toString()->asWChar());
                } else {
                  printf(" query idx %d pulled object %s  :: \n", i,
                         pdxser->toString()->asChar());
                }
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
    QueryHelper* qh = &QueryHelper::getHelper();

    std::shared_ptr<QueryService> qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }

    for (int i = 0; i < QueryStrings::RSPsize(); i++) {
      if (resultsetparamQueries[i].category != unsupported) {
        auto qry =
            qs->newQuery(const_cast<char*>(resultsetparamQueries[i].query()));
        // LOGINFO("NIL::229:Retrieved QueryString = %s",
        // qry->getQueryString());

        std::shared_ptr<CacheableVector> paramList = CacheableVector::create();
        for (int j = 0; j < noofQueryParam[i]; j++) {
          // LOGINFO("NIL::260: queryparamSet[%d][%d] = %s", i, j,
          // queryparamSet[i][j]);
          if (atoi(queryparamSet[i][j]) != 0) {
            paramList->push_back(Cacheable::create(atoi(queryparamSet[i][j])));
          } else {
            paramList->push_back(Cacheable::create(queryparamSet[i][j]));
          }
        }

        std::shared_ptr<SelectResults> results = qry->execute(paramList);
        if (!qh->verifyRS(results, (qh->isExpectedRowsConstantPQRS(i)
                                        ? resultsetRowCountsPQ[i]
                                        : resultsetRowCountsPQ[i] *
                                              qh->getPortfolioNumSets()))) {
          char failmsg[100] = {0};
          ACE_OS::sprintf(failmsg, "Query verify failed for query index %d", i);
          doAnyErrorOccured = true;
          ASSERT(false, failmsg);
          continue;
        }

        auto rsptr = std::dynamic_pointer_cast<ResultSet>(results);
        SelectResultsIterator iter = rsptr->getIterator();
        for (int32_t rows = 0; rows < rsptr->size(); rows++) {
          if (rows > (int32_t)QueryHelper::getHelper().getPortfolioSetSize()) {
            continue;
          }

          if (!m_isPdx) {
            std::shared_ptr<Serializable> ser = (*rsptr)[rows];
            if (std::dynamic_pointer_cast<Portfolio>(ser)) {
              auto portfolio = std::static_pointer_cast<Portfolio>(ser);
              printf(
                  "   query idx %d pulled portfolio object ID %d, pkid %s : \n",
                  i, portfolio->getID(), portfolio->getPkid()->asChar());
            } else if (std::dynamic_pointer_cast<Position>(ser)) {
              auto position = std::static_pointer_cast<Position>(ser);
              printf(
                  "   query idx %d pulled position object secId %s, shares %d  "
                  ": "
                  "\n",
                  i, position->getSecId()->asChar(),
                  position->getSharesOutstanding());
            } else {
              if (ser != nullptr) {
                printf(" query idx %d pulled object %s  : \n", i,
                       ser->toString()->asChar());
              } else {
                printf("   query idx %d pulled bad object  \n", i);
                FAIL("Unexpected object received in query");
              }
            }
          } else {
            std::shared_ptr<Serializable> ser = (*rsptr)[rows];
            if (std::dynamic_pointer_cast<PortfolioPdx>(ser)) {
              auto portfoliopdx = std::static_pointer_cast<PortfolioPdx>(ser);
              printf(
                  "   query idx %d pulled portfolioPdx object ID %d, pkid %s  "
                  ": "
                  "\n",
                  i, portfoliopdx->getID(), portfoliopdx->getPkid());
            } else if (std::dynamic_pointer_cast<PositionPdx>(ser)) {
              auto positionpdx = std::static_pointer_cast<PositionPdx>(ser);
              printf(
                  "   query idx %d pulled positionPdx object secId %s, shares "
                  "%d "
                  " : \n",
                  i, positionpdx->getSecId(),
                  positionpdx->getSharesOutstanding());
            } else {
              if (ser != nullptr) {
                printf(" query idx %d pulled object %s : \n", i,
                       ser->toString()->asChar());
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
    QueryHelper* qh ATTR_UNUSED = &QueryHelper::getHelper();

    std::shared_ptr<QueryService> qs = nullptr;
    if (isPoolConfig) {
      auto pool1 = findPool(poolNames[0]);
      qs = pool1->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }

    for (int i = 0; i < QueryStrings::RSsize(); i++) {
      if (resultsetQueries[i].category == unsupported) {
        auto qry = qs->newQuery(const_cast<char*>(resultsetQueries[i].query()));

        try {
          std::shared_ptr<SelectResults> results = qry->execute();

          char failmsg[100] = {0};
          ACE_OS::sprintf(failmsg, "Query exception didnt occur for index %d",
                          i);
          LOG(failmsg);
          FAIL(failmsg);
        } catch (apache::geode::client::QueryException&) {
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
  CALL_TASK(StartLocator)
  CALL_TASK(CreateServerWithLocator)
  CALL_TASK(StepOnePoolLoc)

  CALL_TASK(StepThree)
  CALL_TASK(StepFour)
  CALL_TASK(StepFive)
  CALL_TASK(StepSix)
  CALL_TASK(DoQueryRSError)
  CALL_TASK(CloseCache1)
  CALL_TASK(CloseServer)

  CALL_TASK(CloseLocator)
}

void setPortfolioPdxType() { CALL_TASK(SetPortfolioTypeToPdx) }

void UnsetPortfolioType(){CALL_TASK(UnsetPortfolioTypeToPdx)}

DUNIT_MAIN {
  for (int i = 0; i < 2; i++) {
    runRemoteQueryRSTest();
    setPortfolioPdxType();
  }
}
END_MAIN
