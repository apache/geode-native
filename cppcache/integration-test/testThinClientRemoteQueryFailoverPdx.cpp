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

#include <string>

#define ROOT_NAME "testThinClientRemoteQueryFailoverPdx"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "ThinClientHelper.hpp"

#include <geode/Query.hpp>
#include <geode/QueryService.hpp>
#include <geode/ResultSet.hpp>
#include <geode/StructSet.hpp>

#include "testobject/Portfolio.hpp"
#include "testobject/PortfolioPdx.hpp"
#include "SerializationRegistry.hpp"
#include "CacheRegionHelper.hpp"

#define CLIENT1 s1p1
#define LOCATOR s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2

using testobject::Portfolio;
using testobject::PortfolioPdx;
using testobject::Position;
using testobject::PositionPdx;

using apache::geode::client::Exception;
using apache::geode::client::IllegalStateException;
using apache::geode::client::QueryService;
using apache::geode::client::SelectResults;

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

bool isLocator = false;
bool isLocalServer = false;

const char *qRegionNames[] = {"Portfolios", "Positions"};
KillServerThread *kst = nullptr;
const char *poolNames[] = {"Pool1", "Pool2", "Pool3"};
const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
bool isPoolConfig = false;  // To track if pool case is running

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
      CacheHelper::initServer(1, "cacheserver_remoteoql.xml", locHostPort);
    }
    LOG("SERVER1 started with Locator");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2WithLocator)
  {
    LOG("Starting SERVER2...");
    if (isLocalServer) {
      CacheHelper::initServer(2, "cacheserver_remoteoql2.xml", locHostPort);
    }
    LOG("SERVER2 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, RegisterTypesAndCreatePoolAndRegion)
  {
    LOG("Starting Step One with Pool + Locator lists");

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

    auto rptr = getHelper()->cachePtr->getRegion(qRegionNames[0]);

    auto port1 = std::make_shared<PortfolioPdx>(1, 100);
    auto port2 = std::make_shared<PortfolioPdx>(2, 200);
    auto port3 = std::make_shared<PortfolioPdx>(3, 300);
    auto port4 = std::make_shared<PortfolioPdx>(4, 400);

    rptr->put("1", port1);
    rptr->put("2", port2);
    rptr->put("3", port3);
    rptr->put("4", port4);

    LOG("RegisterTypesAndCreatePoolAndRegion complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, ValidateQueryExecutionAcrossServerFailure)
  {
    try {
      kst = new KillServerThread();

      std::shared_ptr<QueryService> qs = nullptr;
      if (isPoolConfig) {
        auto pool1 = findPool(poolNames[0]);
        qs = pool1->getQueryService();
      } else {
        qs = getHelper()->cachePtr->getQueryService();
      }

      for (int i = 0; i < 10000; i++) {
        auto qry = qs->newQuery("select distinct * from /Portfolios");

        std::shared_ptr<SelectResults> results;
        results = qry->execute();

        if (i == 10) {
          kst->start();
        }

        auto resultsize = results->size();

        if (i % 100 == 0) {
          std::cout << "Iteration upto " << i << " done, result size is "
                    << resultsize << "\n";
        }

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

    LOG("ValidateQueryExecutionAcrossServerFailure complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  {
    LOG("cleanProc 1...");
    isPoolConfig = false;
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

DUNIT_TASK_DEFINITION(LOCATOR, CloseLocator)
  {
    if (isLocator) {
      CacheHelper::closeLocator(1);
      LOG("Locator1 stopped");
    }
  }
END_TASK_DEFINITION

void runRemoteQueryFailoverTest() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServer1WithLocator);
  CALL_TASK(RegisterTypesAndCreatePoolAndRegion);
  CALL_TASK(CreateServer2WithLocator);
  CALL_TASK(ValidateQueryExecutionAcrossServerFailure);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseServer2);
  CALL_TASK(CloseLocator);
}

DUNIT_MAIN
  { runRemoteQueryFailoverTest(); }
END_MAIN
