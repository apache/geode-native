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
#include <ace/Task.h>
#include <string>

#define ROOT_NAME "testThinClientRegionQueryExclusiveness"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "ThinClientHelper.hpp"

#include "testobject/Portfolio.hpp"

#include <geode/Query.hpp>
#include <geode/QueryService.hpp>
#include <geode/ResultSet.hpp>
#include <geode/StructSet.hpp>
#include <geode/SelectResultsIterator.hpp>

#include "SerializationRegistry.hpp"
#include "CacheRegionHelper.hpp"
#include "CacheImpl.hpp"

using namespace apache::geode::client;
using namespace test;
using namespace testobject;

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER s2p1
#define LOCATOR s2p2

bool isLocator = false;
bool isLocalServer = false;
const char* poolNames[] = {"Pool1", "Pool2", "Pool3"};
const char* locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);

const char* qRegionNames[] = {"Portfolios", "Positions"};

void clientOperations() {
  initClient(true);
  try {
    SerializationRegistryPtr serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    serializationRegistry->addType(Position::createDeserializable);
    serializationRegistry->addType(Portfolio::createDeserializable);
  } catch (const IllegalStateException&) {
    // ignore exception
  }

  PoolPtr pool1 = nullptr;
  pool1 = createPool(poolNames[0], locHostPort, nullptr, 0, true);
  createRegionAndAttachPool(qRegionNames[0], USE_ACK, poolNames[0]);

  RegionPtr rptr = getHelper()->cachePtr->getRegion(qRegionNames[0]);
  auto p1 = std::make_shared<Portfolio>(1, 100);
  auto p2 = std::make_shared<Portfolio>(2, 100);
  auto p3 = std::make_shared<Portfolio>(3, 100);
  auto p4 = std::make_shared<Portfolio>(4, 100);

  rptr->put("1", p1);
  rptr->put("2", p2);
  rptr->put("3", p3);
  rptr->put("4", p4);

  QueryServicePtr qs = nullptr;
  qs = pool1->getQueryService();

  QueryPtr qry1 = qs->newQuery("select distinct * from /Portfolios");
  SelectResultsPtr results1 = qry1->execute();
  ASSERT(results1->size() == 4,
         "Expected 4 as number of portfolio objects put were 4");

  // Bring down the region
  rptr->localDestroyRegion();
  LOG("StepOne complete.");

  try {
    LOG("Going to execute the query");
    QueryPtr qry2 = qs->newQuery("select distinct * from /Portfolios");
    SelectResultsPtr results2 = qry2->execute();
    ASSERT(results2->size() == 4, "Failed verification");
  } catch (...) {
    FAIL("Got an exception!");
  }
  LOG("StepTwo complete.");
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

DUNIT_TASK_DEFINITION(SERVER, CreateServer)
  {
    LOG("Starting SERVER...");
    if (isLocalServer) CacheHelper::initServer(1, "cacheserver_remoteoql.xml");
    LOG("SERVER started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER, CreateServerWithLocator)
  {
    LOG("Starting SERVER1...");

    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_remoteoql.xml", locHostPort);
    }

    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, ClientOpPoolLocator)
  { clientOperations(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  {
    LOG("cleanProc 1...");
    cleanProc();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER, CloseServer)
  {
    LOG("closing Server1...");
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER stopped");
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
     CALL_TASK(ClientOpPoolLocator) CALL_TASK(CloseCache1)
         CALL_TASK(CloseServer) CALL_TASK(CloseLocator)} END_MAIN
