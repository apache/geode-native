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

#define ROOT_NAME "testThinClientHAQueryFailover"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"

#include "SerializationRegistry.hpp"
#include "CacheRegionHelper.hpp"
#include "CacheImpl.hpp"

#include <geode/Query.hpp>
#include <geode/QueryService.hpp>
#include <geode/ResultSet.hpp>
#include <geode/StructSet.hpp>
#include <geode/SelectResultsIterator.hpp>

#include "testobject/Portfolio.hpp"

using namespace apache::geode::client;
using namespace test;
using namespace testobject;

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2

CacheHelper* cacheHelper = nullptr;
static bool isLocalServer = false;
static bool isLocator = false;
static int numberOfLocators = 1;

const char* locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);

class KillServerThread : public ACE_Task_Base {
 public:
  bool m_running;
  KillServerThread() : m_running(false) {}
  int svc(void) {
    while (m_running == true) {
      // CacheHelper::initServer( 2, "cacheserver_remoteoql2.xml");
      // LOG("THREAD STARTED SERVER 2");
      CacheHelper::closeServer(1);
      LOG("THREAD CLOSED SERVER 1");
      m_running = false;
    }
    return 0;
  }
  void start() {
    m_running = true;
    activate();
  }
  void stop() {
    m_running = false;
    wait();
  }
};

void initClient() {
  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(true);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
  try {
    SerializationRegistryPtr serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    serializationRegistry->addType(Portfolio::createDeserializable);
    serializationRegistry->addType(Position::createDeserializable);
  } catch (const IllegalStateException&) {
    // ignore reregistration exception
  }
}

/*
void initClient( const bool isthinClient )
{
  serializationRegistry->addType( Portfolio::createDeserializable);
  serializationRegistry->addType( Position::createDeserializable);

  if ( cacheHelper == nullptr ) {
    cacheHelper = new CacheHelper(isthinClient);
  }
  ASSERT( cacheHelper, "Failed to create a CacheHelper client instance." );
}
*/

void cleanProc() {
  if (cacheHelper != nullptr) {
    delete cacheHelper;
    cacheHelper = nullptr;
  }
}

CacheHelper* getHelper() {
  ASSERT(cacheHelper != nullptr, "No cacheHelper initialized.");
  return cacheHelper;
}

void createRegion(const char* name, bool ackMode,
                  bool clientNotificationEnabled = true) {
  LOG("createRegion() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  char* endpoints = nullptr;
  RegionPtr regPtr = getHelper()->createRegion(
      name, ackMode, false, nullptr, endpoints, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}

const char* regionNames[] = {"Portfolios", "Positions"};

const bool USE_ACK = true;
const bool NO_ACK ATTR_UNUSED = false;

KillServerThread* kst = nullptr;

void initClientAndRegion(int redundancy) {
  // PropertiesPtr pp  = Properties::create();
  getHelper()->createPoolWithLocators("__TESTPOOL1_", locatorsG, true,
                                      redundancy);
  getHelper()->createRegionAndAttachPool(regionNames[0], USE_ACK,
                                         "__TESTPOOL1_", true);
}

#include "LocatorHelper.hpp"
#include "ThinClientTasks_C2S2.hpp"

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    LOG("Starting SERVER1...");
    if (isLocalServer) CacheHelper::initServer(1, "cacheserver_remoteoql.xml");
    LOG("SERVER1 started");

    // LOG("Starting SERVER2...");
    // if ( isLocalServer ) CacheHelper::initServer( 2,
    // "cacheserver_remoteoql2.xml");
    // LOG("SERVER2 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne)
  {
    initClient();
    initClientAndRegion(1);

    RegionPtr rptr = getHelper()->cachePtr->getRegion(regionNames[0]);

    auto port1 = std::make_shared<Portfolio>(1, 100);
    auto port2 = std::make_shared<Portfolio>(2, 200);
    auto port3 = std::make_shared<Portfolio>(3, 300);
    auto port4 = std::make_shared<Portfolio>(4, 400);

    rptr->put("1", port1);
    rptr->put("2", port2);
    rptr->put("3", port3);
    rptr->put("4", port4);

    LOG("StepOne complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, StepTwo)
  {
    LOG("Starting SERVER2...");
    if (isLocalServer) CacheHelper::initServer(2, "cacheserver_remoteoql2.xml");
    LOG("SERVER2 started");
    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    try {
      kst = new KillServerThread();

      QueryServicePtr qs = nullptr;

      PoolPtr pool =
          getHelper()->getCache()->getPoolManager().find("__TESTPOOL1_");
      qs = pool->getQueryService();
      LOG("Got query service from pool");

      for (int i = 0; i < 10000; i++) {
        QueryPtr qry = qs->newQuery("select distinct * from /Portfolios");

        SelectResultsPtr results;

        // try
        //{
        results = qry->execute();
        //}
        /*
        catch(IllegalStateException &)
        {
          printf("IllegalStateException occurred at iteration %d\n", i);
          //SLEEP(1000);
          continue;
        }
        catch(Exception &)
        {
          printf("Exception occurred at iteration %d\n", i);
          //SLEEP(1000);
          continue;
        }
        */

        if (i == 10) {
          kst->start();
          // SLEEP(15000);
        }

        int resultsize = results->size();

        if (i % 100 == 0) {
          printf("Iteration upto %d done, result size is %d\n", i, resultsize);
        }

        // ASSERT(resultsize==4, "Failed verification");

        if (resultsize != 4)  // the XMLs for server 1 and server 2 have 1 and 2
                              // entries respectively
        {
          LOG("Result size is not 4!");
          FAIL("Result size is not 4!");
        }
      }

      kst->stop();
    } catch (IllegalStateException& ise) {
      char isemsg[500] = {0};
      ACE_OS::snprintf(isemsg, 499, "IllegalStateException: %s",
                       ise.getMessage());
      LOG(isemsg);
      FAIL(isemsg);
    } catch (Exception& excp) {
      char excpmsg[500] = {0};
      ACE_OS::snprintf(excpmsg, 499, "Exception: %s", excp.getMessage());
      LOG(excpmsg);
      FAIL(excpmsg);
    } catch (...) {
      LOG("Got an exception!");
      FAIL("Got an exception!");
    }

    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  {
    LOG("cleanProc 1...");
    cleanProc();
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

void runQueryFailover() {
  CALL_TASK(CreateLocator1);
  CALL_TASK(CreateServer1_With_Locator_OQL);

  CALL_TASK(StepOne)

  CALL_TASK(CreateServer2_With_Locator_OQL);

  SLEEP(15000);
  CALL_TASK(StepThree);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseServer2);

  CALL_TASK(CloseLocator1);
}
DUNIT_MAIN
  { runQueryFailover(); }
END_MAIN
