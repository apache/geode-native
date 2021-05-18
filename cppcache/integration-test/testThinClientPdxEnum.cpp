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

#include "QueryHelper.hpp"
#include "QueryStrings.hpp"
#include "ThinClientHelper.hpp"
#include "fw_dunit.hpp"
#include "testobject/NestedPdxObject.hpp"
#include <geode/Query.hpp>
#include <geode/QueryService.hpp>
#include <hacks/range.h>

using testobject::PdxEnumTestClass;

bool isLocalServer = false;

#define CLIENT1 s1p1
#define SERVER1 s2p1
static bool isLocator = false;

const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);

DUNIT_TASK_DEFINITION(CLIENT1, SetupClientPoolLoc)
  {
    LOG("Starting Step One with Pool + Locator lists");
    initClient(true);

    createPool("__TEST_POOL1__", locatorsG, {}, 0, true);
    createRegionAndAttachPool("DistRegionAck", USE_ACK, "__TEST_POOL1__");

    LOG("SetupClient complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateLocator1)
  {
    // starting locator
    CacheHelper::initLocator(1);
    LOG("Locator1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserverPdxSerializer.xml");
    }
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1_With_Locator)
  {
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserverPdxSerializer.xml", locatorsG);
    }
    LOG("SERVER1 with locator started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, putPdxWithEnum)
  {
    LOG("putPdxWithEnum started ");

    // Creating objects of type PdxEnumTestClass
    auto pdxobj1 = std::make_shared<PdxEnumTestClass>(0);
    auto pdxobj2 = std::make_shared<PdxEnumTestClass>(1);
    auto pdxobj3 = std::make_shared<PdxEnumTestClass>(2);

    auto rptr = getHelper()->getRegion("DistRegionAck");

    // PUT Operations
    rptr->put(CacheableInt32::create(0), pdxobj1);
    LOG("pdxPut 1 completed ");

    rptr->put(CacheableInt32::create(1), pdxobj2);
    LOG("pdxPut 2 completed ");

    rptr->put(CacheableInt32::create(2), pdxobj3);
    LOG("pdxPut 3 completed ");

    LOG("putPdxWithEnum complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, pdxEnumQuery)
  {
    LOG("pdxEnumQuery started ");

    try {
      auto serializationRegistry =
          CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
              ->getSerializationRegistry();
      serializationRegistry->addPdxSerializableType(
          PdxEnumTestClass::createDeserializable);
      LOG("PdxEnumTestClass Registered Successfully....");
    } catch (apache::geode::client::IllegalStateException & /* ex*/) {
      LOG("PdxEnumTestClass IllegalStateException");
    }

    auto &&rptr = getHelper()->getRegion("DistRegionAck");
    auto &&results = rptr->query("m_enumid.name = 'id2'");
    ASSERT(results->size() == 1, "query result should have one item");
    auto rsptr = std::dynamic_pointer_cast<ResultSet>(results);
    for (auto &&row : hacks::range(*rsptr)) {
      auto re = std::dynamic_pointer_cast<PdxEnumTestClass>(row);
      ASSERT(re->getID() == 1, "query should have return id 1");
    }

    QueryHelper::getHelper();
    auto &&pool1 = findPool("__TEST_POOL1__");
    auto &&qs = pool1->getQueryService();
    auto &&qry = qs->newQuery(
        "select distinct * from /DistRegionAck this where m_enumid.name = "
        "'id3'");
    results = qry->execute();
    rsptr = std::dynamic_pointer_cast<ResultSet>(results);
    for (auto &&row : hacks::range(*rsptr)) {
      auto re = std::dynamic_pointer_cast<PdxEnumTestClass>(row);
      ASSERT(re->getID() == 2, "query should have return id 0");
    }

    LOG("pdxEnumQuery complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer1)
  {
    CacheHelper::closeServer(1);
    LOG("SERVER1 stopped");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseLocator1)
  {
    // stop locator
    CacheHelper::closeLocator(1);
    LOG("Locator1 stopped");
  }
END_TASK_DEFINITION

DUNIT_MAIN
  {
    CALL_TASK(CreateLocator1);
    CALL_TASK(CreateServer1_With_Locator);

    CALL_TASK(SetupClientPoolLoc);
    CALL_TASK(putPdxWithEnum);
    CALL_TASK(pdxEnumQuery);

    CALL_TASK(CloseCache1);
    CALL_TASK(CloseServer1);

    CALL_TASK(CloseLocator1);
  }
END_MAIN
