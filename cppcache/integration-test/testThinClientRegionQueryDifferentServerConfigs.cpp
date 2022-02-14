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
/**
 * @file testThinClientRegionQueryDifferentServerConfigs.cpp
 *
 * @brief This tests that Region::query makes use of only region-level
 *        endpoints, and not all endpoints.
 *
 *
 */

#include "fw_dunit.hpp"
#include <string>
#include "QueryStrings.hpp"
#include "QueryHelper.hpp"
#include "ThinClientHelper.hpp"
#include "SerializationRegistry.hpp"
#include "CacheRegionHelper.hpp"

#define CLIENT1 s1p1
#define LOCATOR s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2

using apache::geode::client::IllegalStateException;
using apache::geode::client::Query;
using apache::geode::client::QueryException;
using apache::geode::client::QueryService;

bool isLocalServer = false;
bool isLocator = false;
const char *poolNames[] = {"Pool1", "Pool2", "Pool3"};
const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);

const char *qRegionNames[] = {"Portfolios", "Positions"};
const char *sGNames[] = {"ServerGroup1", "ServerGroup2"};

void initClient() {
  initClient(true);
  ASSERT(getHelper() != nullptr, "null CacheHelper");
  try {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    serializationRegistry->addDataSerializableType(
        Position::createDeserializable, 2);
    serializationRegistry->addDataSerializableType(
        Portfolio::createDeserializable, 3);
  } catch (const IllegalStateException &) {
    // ignore exception
  }
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

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1WithLocator)
  {
    LOG("Starting SERVER1...");

    if (isLocalServer) {
      CacheHelper::initServer(1, "regionquery_diffconfig_SG.xml", locHostPort);
    }

    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, InitClientCreateRegionAndRunQueries)
  {
    LOG("Starting Step One with Pool + Locator lists");
    initClient();
    std::shared_ptr<Pool> pool1 = nullptr;
    pool1 = createPool(poolNames[0], locHostPort, sGNames[0], 0, true);
    createRegionAndAttachPool(qRegionNames[0], USE_ACK, poolNames[0]);

    // populate the region
    auto reg = getHelper()->getRegion(qRegionNames[0]);
    QueryHelper &qh = QueryHelper::getHelper();
    qh.populatePortfolioData(reg, qh.getPortfolioSetSize(),
                             qh.getPortfolioNumSets());

    std::string qry1Str = std::string("select * from /") + qRegionNames[0];
    std::string qry2Str = std::string("select * from /") + qRegionNames[1];

    std::shared_ptr<QueryService> qs = nullptr;
    qs = pool1->getQueryService();

    std::shared_ptr<SelectResults> results;
    auto qry = qs->newQuery(qry1Str.c_str());
    results = qry->execute();
    ASSERT(results->size() == static_cast<size_t>(qh.getPortfolioSetSize() *
                                                  qh.getPortfolioNumSets()),
           "unexpected number of results");
    try {
      qry = qs->newQuery(qry2Str.c_str());
      results = qry->execute();
      FAIL("Expected a QueryException");
    } catch (const QueryException &ex) {
      std::cout << "Good expected exception : " << ex.what() << "\n";
    }

    // now region queries
    results = reg->query(qry1Str.c_str());
    ASSERT(results->size() == static_cast<size_t>(qh.getPortfolioSetSize() *
                                                  qh.getPortfolioNumSets()),
           "unexpected number of results");
    try {
      results = reg->query(qry2Str.c_str());
      FAIL("Expected a QueryException");
    } catch (const QueryException &ex) {
      std::cout << "Good expected exception : " << ex.what() << "\n";
    }

    LOG("StepOne complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2WithLocator)
  {
    LOG("Starting SERVER2...");

    if (isLocalServer) {
      CacheHelper::initServer(2, "regionquery_diffconfig2_SG.xml", locHostPort);
    }
    LOG("SERVER2 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CreateRegionAndRunQueries)
  {
    LOG("Starting Step Two with Pool + Locator list");
    // Create pool2
    std::shared_ptr<Pool> pool2 = nullptr;

    pool2 = createPool(poolNames[1], locHostPort, sGNames[1], 0, true);
    createRegionAndAttachPool(qRegionNames[1], USE_ACK, poolNames[1]);

    // populate the region
    auto reg = getHelper()->getRegion(qRegionNames[1]);
    QueryHelper &qh = QueryHelper::getHelper();
    qh.populatePositionData(reg, qh.getPositionSetSize(),
                            qh.getPositionNumSets());

    std::string qry1Str = std::string("select * from /") + qRegionNames[0];
    std::string qry2Str = std::string("select * from /") + qRegionNames[1];

    std::shared_ptr<QueryService> qs = nullptr;
    qs = pool2->getQueryService();
    std::shared_ptr<SelectResults> results;
    std::shared_ptr<Query> qry;

    // now region queries
    try {
      results = reg->query(qry1Str.c_str());
      FAIL("Expected a QueryException");
    } catch (const QueryException &ex) {
      std::cout << "Good expected exception : " << ex.what() << "\n";
    }
    results = reg->query(qry2Str.c_str());
    ASSERT(results->size() == static_cast<size_t>(qh.getPositionSetSize() *
                                                  qh.getPositionNumSets()),
           "unexpected number of results");

    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  { cleanProc(); }
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

DUNIT_TASK_DEFINITION(SERVER2, CloseServer2)
  {
    LOG("closing Server2...");
    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
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
    CALL_TASK(CreateServer1WithLocator);
    CALL_TASK(InitClientCreateRegionAndRunQueries);
    CALL_TASK(CreateServer2WithLocator);
    CALL_TASK(CreateRegionAndRunQueries);
    CALL_TASK(CloseCache1);
    CALL_TASK(CloseServer1);
    CALL_TASK(CloseServer2);
    CALL_TASK(CloseLocator);
  }
END_MAIN
