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
#include <geode/CqAttributesFactory.hpp>
#include <geode/CqAttributes.hpp>
#include <geode/CqListener.hpp>
#include <geode/CqQuery.hpp>
#include <ace/OS.h>
#include <ace/High_Res_Timer.h>
#include <ace/Task.h>
#include <string>

#define ROOT_NAME "TestThinClientCqHAFailover"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"

#include "QueryStrings.hpp"
#include "QueryHelper.hpp"

#include <geode/Query.hpp>
#include <geode/QueryService.hpp>

#include "ThinClientCQ.hpp"
#include <hacks/range.h>

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2

using apache::geode::client::CqAttributesFactory;
using apache::geode::client::CqEvent;
using apache::geode::client::CqListener;
using apache::geode::client::Exception;
using apache::geode::client::IllegalStateException;
using apache::geode::client::QueryService;

const char *cqName = "MyCq";

const char *regionNamesCq[] = {"Portfolios", "Positions"};

class MyCqListener : public CqListener {
  bool m_failedOver;
  uint32_t m_cnt_before;
  uint32_t m_cnt_after;

 public:
  MyCqListener() : m_failedOver(false), m_cnt_before(0), m_cnt_after(0) {}

  void setFailedOver() { m_failedOver = true; }
  uint32_t getCountBefore() { return m_cnt_before; }
  uint32_t getCountAfter() { return m_cnt_after; }

  void onEvent(const CqEvent &) override {
    if (m_failedOver) {
      // LOG("after:MyCqListener::OnEvent called");
      m_cnt_after++;
    } else {
      // LOG("before:MyCqListener::OnEvent called");
      m_cnt_before++;
    }
  }
  void onError(const CqEvent &) override {
    if (m_failedOver) {
      // LOG("after: MyCqListener::OnError called");
      m_cnt_after++;
    } else {
      // LOG("before: MyCqListener::OnError called");
      m_cnt_before++;
    }
  }
  void close() override { LOG("MyCqListener::close called"); }
};

class KillServerThread : public ACE_Task_Base {
 public:
  bool m_running;
  MyCqListener *m_listener;
  explicit KillServerThread(MyCqListener *listener)
      : m_running(false), m_listener(listener) {}
  int svc(void) override {
    while (m_running == true) {
      CacheHelper::closeServer(1);
      LOG("THREAD CLOSED SERVER 1");
      // m_listener->setFailedOver();
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

void initClientCq() {
  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(true);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");

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

KillServerThread *kst = nullptr;

DUNIT_TASK_DEFINITION(SERVER1, CreateLocator)
  {
    if (isLocator) CacheHelper::initLocator(1);
    LOG("Locator1 started");
  }
END_TASK_DEFINITION

void createServer() {
  LOG("Starting SERVER1...");
  if (isLocalServer) {
    CacheHelper::initServer(1, "cqqueryfailover.xml", locatorsG);
  }
  LOG("SERVER1 started");
}

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1_Locator)
  { createServer(); }
END_TASK_DEFINITION

void stepTwo() {
  LOG("Starting SERVER2...");
  // if ( isLocalServer ) CacheHelper::initServer( 2, "cqqueryfailover.xml");
  if (isLocalServer) {
    CacheHelper::initServer(2, "remotequery.xml", locatorsG);
  }
  LOG("SERVER2 started");

  LOG("StepTwo complete.");
}

DUNIT_TASK_DEFINITION(SERVER2, StepTwo_Locator)
  { stepTwo(); }
END_TASK_DEFINITION

void stepOne() {
  initClientCq();

  createRegionForCQ(regionNamesCq[0], USE_ACK, true, 1);

  auto regptr = getHelper()->getRegion(regionNamesCq[0]);
  auto subregPtr =
      regptr->createSubregion(regionNamesCq[1], regptr->getAttributes());

  QueryHelper *qh = &QueryHelper::getHelper();

  qh->populatePortfolioData(regptr, 100, 20, 10);
  qh->populatePositionData(subregPtr, 100, 20);

  LOG("StepOne complete.");
}

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_PoolLocator)
  { stepOne(); }
END_TASK_DEFINITION

void stepOne2() {
  initClientCq();
  createRegionForCQ(regionNamesCq[0], USE_ACK, true, 1);
  auto regptr = getHelper()->getRegion(regionNamesCq[0]);
  auto subregPtr =
      regptr->createSubregion(regionNamesCq[1], regptr->getAttributes());

  LOG("StepOne2 complete.");
}

DUNIT_TASK_DEFINITION(CLIENT2, StepOne2_PoolLocator)
  { stepOne2(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    try {
      auto pool =
          getHelper()->getCache()->getPoolManager().find(regionNamesCq[0]);
      std::shared_ptr<QueryService> qs;
      if (pool != nullptr) {
        qs = pool->getQueryService();
      } else {
        qs = getHelper()->cachePtr->getQueryService();
      }
      CqAttributesFactory cqFac;
      auto cqLstner = std::make_shared<MyCqListener>();
      cqFac.addCqListener(cqLstner);
      auto cqAttr = cqFac.create();

      auto qryStr = "select * from /Portfolios p where p.ID != 1";
      // char* qryStr = (char*)"select * from /Portfolios p where p.ID != 2";
      // char* qryStr = (char*)"select * from /Portfolios p where p.ID < 3";
      auto &&qry = qs->newCq(cqName, qryStr, cqAttr);
      auto &&results = qry->executeWithInitialResults();

      auto count = results->size();
      LOG(std::string("results size=") + std::to_string(count));
      for (auto &&ser : hacks::range(*results)) {
        count--;
        if (auto portfolio = std::dynamic_pointer_cast<Portfolio>(ser)) {
          printf("   query pulled portfolio object ID %d, pkid %s\n",
                 portfolio->getID(), portfolio->getPkid()->value().c_str());
        } else if (auto position = std::dynamic_pointer_cast<Position>(ser)) {
          printf("   query  pulled position object secId %s, shares %d\n",
                 position->getSecId()->value().c_str(),
                 position->getSharesOutstanding());
        } else if (ser) {
          printf(" query pulled object %s\n", ser->toString().c_str());
        } else {
          printf("   query pulled nullptr object\n");
        }
      }
      LOG(std::string("results last count=") + std::to_string(count));
      //  ASSERT( count==0, "results traversal count incorrect!" );
      SLEEP(15000);
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

DUNIT_TASK_DEFINITION(CLIENT2, StepThree2)
  {
    auto regPtr0 = getHelper()->getRegion(regionNamesCq[0]);
    auto subregPtr0 = regPtr0->getSubregion(regionNamesCq[1]);

    QueryHelper *qh = &QueryHelper::getHelper();

    qh->populatePortfolioData(regPtr0, 150, 40, 10);
    qh->populatePositionData(subregPtr0, 150, 40);
    for (int i = 1; i < 150; i++) {
      auto port = std::make_shared<Portfolio>(i, 20);

      auto keyport = CacheableKey::create("port1-1");
      regPtr0->put(keyport, port);
      SLEEP(100);  // sleep a while to allow server query to complete
    }

    LOG("StepTwo2 complete.");
    SLEEP(15000);  // sleep 0.25 min to allow server query to complete
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree3)
  {
    // using region name as pool name
    auto pool =
        getHelper()->getCache()->getPoolManager().find(regionNamesCq[0]);
    std::shared_ptr<QueryService> qs;
    if (pool != nullptr) {
      qs = pool->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }
    auto qry = qs->getCq(cqName);
    ASSERT(qry != nullptr, "failed to get CqQuery");
    auto cqAttr = qry->getCqAttributes();
    ASSERT(cqAttr != nullptr, "failed to get CqAttributes");
    std::shared_ptr<CqListener> cqLstner = nullptr;
    try {
      auto vl = cqAttr->getCqListeners();
      cqLstner = vl[0];
    } catch (Exception &excp) {
      std::string excpmsg = "Exception: " + std::string{excp.what()};

      LOG(excpmsg);
      ASSERT(false, "get listener failed");
    }
    ASSERT(cqLstner != nullptr, "listener is nullptr");
    MyCqListener *myListener = dynamic_cast<MyCqListener *>(cqLstner.get());
    ASSERT(myListener != nullptr, "my listener is nullptr<cast failed>");
    kst = new KillServerThread(myListener);
    char buf[1024];
    LOG(std::string("before kill server 1, before=") +
        std::to_string(myListener->getCountBefore()) +
        ", after=" + std::to_string(myListener->getCountAfter()));
    ASSERT(myListener->getCountAfter() == 0,
           "cq after failover should be zero");
    ASSERT(myListener->getCountBefore() == 6108,
           "check cq event count before failover");
    kst->start();
    SLEEP(1500);
    kst->stop();
    myListener->setFailedOver();
    /*
   auto regPtr0 = getHelper()->getRegion(regionNamesCq[0]);
   auto subregPtr0 = regPtr0->getSubregion(regionNamesCq[1]);
    for(int i=1; i < 150; i++)
    {
        auto port = std::make_shared<Portfolio>(i, 20);

       auto keyport = CacheableKey::create("port1-1");
        try {
          regPtr0->put(keyport, port);
        } catch (...)
        {
          LOG("Failover in progress sleep for 100 ms");
           SLEEP(100); // waiting for failover to complete
           continue;
        }
        LOG("Failover completed");
        myListener->setFailedOver();
        break;
    }
    */
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepThree4)
  {
    auto regPtr0 = getHelper()->getRegion(regionNamesCq[0]);
    auto subregPtr0 = regPtr0->getSubregion(regionNamesCq[1]);

    QueryHelper *qh = &QueryHelper::getHelper();

    qh->populatePortfolioData(regPtr0, 150, 40, 10);
    qh->populatePositionData(subregPtr0, 150, 40);
    for (int i = 1; i < 150; i++) {
      auto port = std::make_shared<Portfolio>(i, 20);

      auto keyport = CacheableKey::create("port1-1");
      regPtr0->put(keyport, port);
      SLEEP(100);  // sleep a while to allow server query to complete
    }

    LOG("StepTwo2 complete.");
    SLEEP(15000);  // sleep 0.25 min to allow server query to complete
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  {
    // using region name as pool name
    auto pool =
        getHelper()->getCache()->getPoolManager().find(regionNamesCq[0]);
    std::shared_ptr<QueryService> qs;
    if (pool != nullptr) {
      qs = pool->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }
    auto qry = qs->getCq(cqName);
    ASSERT(qry != nullptr, "failed to get CqQuery");
    auto cqAttr = qry->getCqAttributes();
    ASSERT(cqAttr != nullptr, "failed to get CqAttributes");
    std::shared_ptr<CqListener> cqLstner = nullptr;
    try {
      auto vl = cqAttr->getCqListeners();
      cqLstner = vl[0];
    } catch (Exception &excp) {
      std::string excpmsg = "Exception: " + std::string{excp.what()};

      LOG(excpmsg);
      ASSERT(false, "get listener failed");
    }
    ASSERT(cqLstner != nullptr, "listener is nullptr");
    MyCqListener *myListener = dynamic_cast<MyCqListener *>(cqLstner.get());
    ASSERT(myListener != nullptr, "my listener is nullptr<cast failed>");
    LOG(std::string("after failed over: before=") +
        std::to_string(myListener->getCountBefore()) +
        ", after=" + std::to_string(myListener->getCountAfter()));
    ASSERT(myListener->getCountBefore() == 6108,
           "check cq event count before failover");
    ASSERT(myListener->getCountAfter() == 6109,
           "check cq event count after failover");
    // ASSERT(myListener->getCountAfter()>0, "no cq after failover");

    LOG("cleanProc 1...");
    cleanProc();
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT2, CloseCache2)
  {
    LOG("cleanProc 2...");
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

DUNIT_TASK_DEFINITION(SERVER1, CloseLocator)
  {
    if (isLocator) {
      CacheHelper::closeLocator(1);
      LOG("Locator1 stopped");
    }
  }
END_TASK_DEFINITION

/*
DUNIT_TASK(SERVER1,CloseServer1)
{
  LOG("closing Server1...");
  if ( isLocalServer ) {
    CacheHelper::closeServer( 1 );
    LOG("SERVER1 stopped");
  }
}
END_TASK(CloseServer1)
*/

void doThinClientCqHAFailover() {
  CALL_TASK(CreateLocator);
  CALL_TASK(CreateServer1_Locator);

  CALL_TASK(StepTwo_Locator);
  CALL_TASK(StepOne_PoolLocator);
  CALL_TASK(StepOne2_PoolLocator);

  CALL_TASK(StepThree);
  CALL_TASK(StepThree2);
  CALL_TASK(StepThree3);
  CALL_TASK(StepThree4);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer2);

  CALL_TASK(CloseLocator);
}

DUNIT_MAIN
  { doThinClientCqHAFailover(); }
END_MAIN
