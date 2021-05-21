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

#include "testobject/Portfolio.hpp"

#define CLIENT1 s1p1
#define SERVER1 s2p1
#define SERVER2 s2p2

using apache::geode::client::CacheHelper;
using apache::geode::client::CacheRegionHelper;
using apache::geode::client::Exception;
using apache::geode::client::IllegalStateException;
using apache::geode::client::QueryService;
using apache::geode::client::SelectResults;

using testobject::Portfolio;
using testobject::Position;

CacheHelper *cacheHelper = nullptr;
static bool isLocalServer = false;
static bool isLocator = false;
static int numberOfLocators = 1;

const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);

class KillServerThread {
 public:
  void start() {
    thread_ = std::thread{[]() {
      CacheHelper::closeServer(1);
      LOG("THREAD CLOSED SERVER 1");
    }};
  }

  void stop() {
    if (thread_.joinable()) {
      thread_.join();
    }
  }

 protected:
  std::thread thread_;
};

void initClient() {
  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(true);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
  try {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    serializationRegistry->addDataSerializableType(
        Portfolio::createDeserializable, 2);
    serializationRegistry->addDataSerializableType(
        Position::createDeserializable, 3);
  } catch (const IllegalStateException &) {
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

CacheHelper *getHelper() {
  ASSERT(cacheHelper != nullptr, "No cacheHelper initialized.");
  return cacheHelper;
}

void createRegion(const char *name, bool ackMode,
                  bool clientNotificationEnabled = true) {
  LOG("createRegion() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  char *endpoints = nullptr;
  auto regPtr = getHelper()->createRegion(name, ackMode, false, nullptr,
                                          endpoints, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}

const char *regionNames[] = {"Portfolios", "Positions"};

const bool USE_ACK = true;
const bool NO_ACK = false;

KillServerThread *kst = nullptr;

void initClientAndRegion(int redundancy) {
  // std::shared_ptr<Properties> pp  = Properties::create();
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

    auto rptr = getHelper()->cachePtr->getRegion(regionNames[0]);

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

      std::shared_ptr<QueryService> qs = nullptr;

      auto pool =
          getHelper()->getCache()->getPoolManager().find("__TESTPOOL1_");
      qs = pool->getQueryService();
      LOG("Got query service from pool");

      for (int i = 0; i < 10000; i++) {
        auto qry = qs->newQuery("select distinct * from /Portfolios");

        std::shared_ptr<SelectResults> results;

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

        auto resultsize = results->size();

        if (i % 100 == 0) {
          printf("Iteration upto %d done, result size is %zd\n", i, resultsize);
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
    } catch (IllegalStateException &ise) {
      std::string excpmsg = "IllegalStateException: " + std::string{ise.what()};

      LOG(excpmsg);
      FAIL(excpmsg);
    } catch (Exception &excp) {
      std::string excpmsg = "Exception: " + std::string{excp.what()};

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

  CALL_TASK(StepOne);

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
