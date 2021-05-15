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

#include <string>
#include <thread>
#include <chrono>

#include <ace/OS.h>
#include <ace/High_Res_Timer.h>

#include <geode/CqAttributesFactory.hpp>
#include <geode/CqAttributes.hpp>
#include <geode/CqListener.hpp>
#include <geode/CqQuery.hpp>
#include <geode/RegionFactory.hpp>

#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"

#define ROOT_SCOPE DISTRIBUTED_ACK

#include <geode/Query.hpp>
#include <geode/QueryService.hpp>
#include <geode/RegionShortcut.hpp>

#include "QueryStrings.hpp"
#include "QueryHelper.hpp"
#include "ThinClientCQ.hpp"

#define CLIENT1 s1p1
#define SERVER1 s2p1
#define CLIENT2 s1p2

using apache::geode::client::Cacheable;
using apache::geode::client::CacheFactory;
using apache::geode::client::CqAttributesFactory;
using apache::geode::client::CqEvent;
using apache::geode::client::CqListener;
using apache::geode::client::CqOperation;
using apache::geode::client::Exception;
using apache::geode::client::IllegalStateException;
using apache::geode::client::QueryService;
using apache::geode::client::RegionShortcut;

const char *durableIds[] = {"DurableId1", "DurableId2"};

const char *cqName = "MyCq";
const char *durableCQNamesClient1[] = {
    "durableCQ1Client1", "durableCQ2Client1", "durableCQ3Client1",
    "durableCQ4Client1", "durableCQ5Client1", "durableCQ6Client1",
    "durableCQ7Client1", "durableCQ8Client1"};

const char *durableCQNamesClient2[] = {
    "durableCQ1Client2", "durableCQ2Client2", "durableCQ3Client2",
    "durableCQ4Client2", "durableCQ5Client2", "durableCQ6Client2",
    "durableCQ7Client2", "durableCQ8Client2"};

static bool m_isPdx = false;

void initClientWithId(int ClientIdx, bool typeRegistered = false) {
  auto pp = Properties::create();
  pp->insert("durable-client-id", durableIds[ClientIdx]);
  pp->insert("durable-timeout", std::chrono::seconds(60));
  pp->insert("notify-ack-interval", std::chrono::seconds(1));

  initClient(true, pp);

  if (typeRegistered == false) {
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
  }
}

class MyCqListener1 : public CqListener {
 public:
  static int m_cntEvents;
  void onEvent(const CqEvent &cqe) override {
    m_cntEvents++;
    const char *opStr = "Default";
    std::shared_ptr<CacheableInt32> value(
        std::dynamic_pointer_cast<CacheableInt32>(cqe.getNewValue()));
    std::shared_ptr<CacheableInt32> key(
        std::dynamic_pointer_cast<CacheableInt32>(cqe.getKey()));
    switch (cqe.getQueryOperation()) {
      case CqOperation::OP_TYPE_CREATE: {
        opStr = "CREATE";
        break;
      }
      case CqOperation::OP_TYPE_UPDATE: {
        opStr = "UPDATE";
        break;
      }
      case CqOperation::OP_TYPE_DESTROY: {
        opStr = "UPDATE";
        break;
      }
      case CqOperation::OP_TYPE_INVALID:
      case CqOperation::OP_TYPE_INVALIDATE:
      case CqOperation::OP_TYPE_REGION_CLEAR:
      case CqOperation::OP_TYPE_MARKER:
        break;
    }
    LOG_INFO("MyCqListener1::OnEvent called with %s, key[%s], value=(%s)",
             opStr, key->toString().c_str(), value->toString().c_str());
  }

  void onError(const CqEvent &) override {
    LOG_INFO("MyCqListener1::OnError called");
  }

  void close() override { LOG_INFO("MyCqListener1::close called"); }
};
int MyCqListener1::m_cntEvents = 0;

const char *regionNamesCq[] = {"Portfolios", "Positions", "Portfolios2",
                               "Portfolios3"};

int onEventCount = 0;
int onErrorCount = 0;
int onEventCountBefore = 0;
class MyCqListener : public CqListener {
  void onEvent(const CqEvent &) override {
    //    LOG("MyCqListener::OnEvent called");
    onEventCount++;
  }

  void onError(const CqEvent &) override {
    //   LOG("MyCqListener::OnError called");
    onErrorCount++;
  }

  void close() override { LOG("MyCqListener::close called"); }
};

DUNIT_TASK_DEFINITION(SERVER1, CreateLocator)
  {
    if (isLocator) CacheHelper::initLocator(1);
    LOG("Locator1 started");
  }
END_TASK_DEFINITION

void createServer(bool locator = false) {
  LOG("Starting SERVER1...");
  if (isLocalServer) {
    CacheHelper::initServer(1, "remotequery.xml",
                            locator ? locatorsG : std::string{});
  }
  LOG("SERVER1 started");
}

void createServer_XML() {
  LOG("Starting SERVER...");
  if (isLocalServer) {
    CacheHelper::initServer(1, "serverDurableClient.xml");
  }
  LOG("SERVER started");
}

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  { createServer(false); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServer)
  { createServer_XML(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1_Locator)
  { createServer(true); }
END_TASK_DEFINITION

void stepOne() {
  initClientWithId(0);
  createRegionForCQ(regionNamesCq[0], USE_ACK, true);
  auto regptr = getHelper()->getRegion(regionNamesCq[0]);
  auto subregPtr =
      regptr->createSubregion(regionNamesCq[1], regptr->getAttributes());

  LOG("StepOne complete.");
}

void RunDurableCqClient() {
  // Create durable client's properties using api.
  auto pp = Properties::create();
  pp->insert("durable-client-id", "DurableClientId");
  pp->insert("durable-timeout", std::chrono::seconds(3600));

  // Create a Geode Cache Programmatically.
  auto cacheFactory = CacheFactory(pp);
  auto cachePtr = std::make_shared<Cache>(cacheFactory.create());
  auto poolFactory = cachePtr->getPoolManager().createFactory();
  poolFactory.setSubscriptionEnabled(true)
      .setSubscriptionAckInterval(std::chrono::milliseconds(5000))
      .setSubscriptionMessageTrackingTimeout(std::chrono::milliseconds(50000))
      .create("");

  LOG_INFO("Created the Geode Cache Programmatically");

  auto regionFactory =
      cachePtr->createRegionFactory(RegionShortcut::CACHING_PROXY);

  // Create the Region Programmatically.
  auto regionPtr = regionFactory.create("DistRegionAck");

  LOG_INFO("Created the Region Programmatically");

  // Get the QueryService from the Cache.
  auto qrySvcPtr = cachePtr->getQueryService();

  // Create CqAttributes and Install Listener
  CqAttributesFactory cqFac;
  auto cqLstner = std::make_shared<MyCqListener1>();
  cqFac.addCqListener(cqLstner);
  auto cqAttr = cqFac.create();

  LOG_INFO("Attached CqListener");

  // create a new Cq Query
  const char *qryStr = "select * from /DistRegionAck ";
  auto qry = qrySvcPtr->newCq("MyCq", qryStr, cqAttr, true);

  LOG_INFO("Created new CqQuery");

  // execute Cq Query
  qry->execute();
  std::this_thread::sleep_for(std::chrono::seconds(10));

  LOG_INFO("Executed new CqQuery");

  // Send ready for Event message to Server( only for Durable Clients ).
  // Server will send queued events to client after recieving this.
  cachePtr->readyForEvents();

  LOG_INFO("Sent ReadyForEvents message to server");

  // wait for some time to recieve events
  std::this_thread::sleep_for(std::chrono::seconds(10));

  // Close the Geode Cache with keepalive = true.  Server will queue events
  // for
  // durable registered keys and will deliver all events when client will
  // reconnect
  // within timeout period and send "readyForEvents()"
  cachePtr->close(true);

  LOG_INFO("Closed the Geode Cache with keepalive as true");
}

void RunFeederClient() {
  auto cacheFactory = CacheFactory();
  LOG_INFO("Feeder connected to the Geode Distributed System");

  auto cachePtr = std::make_shared<Cache>(cacheFactory.create());

  LOG_INFO("Created the Geode Cache");

  auto regionFactory = cachePtr->createRegionFactory(RegionShortcut::PROXY);

  LOG_INFO("Created the RegionFactory");

  // Create the Region Programmatically.
  auto regionPtr = regionFactory.create("DistRegionAck");

  LOG_INFO("Created the Region Programmatically.");

  for (int i = 0; i < 10; i++) {
    auto keyPtr = CacheableInt32::create(i);
    auto valPtr = CacheableInt32::create(i);

    regionPtr->put(keyPtr, valPtr);
  }
  std::this_thread::sleep_for(std::chrono::seconds(10));
  LOG_INFO("put on 0-10 keys done.");

  // Close the Geode Cache
  cachePtr->close();

  LOG_INFO("Closed the Geode Cache");
}

void RunFeederClient1() {
  auto cacheFactory = CacheFactory();
  LOG_INFO("Feeder connected to the Geode Distributed System");

  auto cachePtr = std::make_shared<Cache>(cacheFactory.create());

  LOG_INFO("Created the Geode Cache");

  auto regionFactory = cachePtr->createRegionFactory(RegionShortcut::PROXY);

  LOG_INFO("Created the RegionFactory");

  // Create the Region Programmatically.
  auto regionPtr = regionFactory.create("DistRegionAck");

  LOG_INFO("Created the Region Programmatically.");

  for (int i = 10; i < 20; i++) {
    auto keyPtr = CacheableInt32::create(i);
    auto valPtr = CacheableInt32::create(i);

    regionPtr->put(keyPtr, valPtr);
  }
  std::this_thread::sleep_for(std::chrono::seconds(10));
  LOG_INFO("put on 0-10 keys done.");

  // Close the Geode Cache
  cachePtr->close();

  LOG_INFO("Closed the Geode Cache");
}

DUNIT_TASK_DEFINITION(CLIENT1, RunDurableClient)
  { RunDurableCqClient(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, RunFeeder)
  { RunFeederClient(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, RunFeeder1)
  { RunFeederClient1(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_PoolLocator)
  { stepOne(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, VerifyEvents)
  {
    LOG_INFO("MyCqListener1::m_cntEvents = %d ", MyCqListener1::m_cntEvents);
    ASSERT(MyCqListener1::m_cntEvents == 20, "Incorrect events, expected 20");
  }
END_TASK_DEFINITION

void stepOne2() {
  initClientWithId(1);
  createRegionForCQ(regionNamesCq[0], USE_ACK, true);
  auto regptr = getHelper()->getRegion(regionNamesCq[0]);
  auto subregPtr =
      regptr->createSubregion(regionNamesCq[1], regptr->getAttributes());

  LOG("StepOne2 complete.");
}

DUNIT_TASK_DEFINITION(CLIENT2, StepOne2_PoolLocator)
  { stepOne2(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepTwo)
  {
    auto regPtr0 = getHelper()->getRegion(regionNamesCq[0]);
    auto subregPtr0 = regPtr0->getSubregion(regionNamesCq[1]);

    QueryHelper *qh = &QueryHelper::getHelper();
    if (!m_isPdx) {
      qh->populatePortfolioData(regPtr0, 130, 20, 20);
      qh->populatePositionData(subregPtr0, 130, 20);
    } else {
      qh->populatePortfolioPdxData(regPtr0, 130, 20, 20);
      qh->populatePositionPdxData(subregPtr0, 130, 20);
    }

    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    QueryHelper::getHelper();

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

    auto qryStr = "select * from /Portfolios p where p.ID < 3";
    auto &&qry = qs->newCq(cqName, qryStr, cqAttr);

    try {
      LOG("EXECUTE 1 START");

      auto &&results = qry->executeWithInitialResults();

      LOG("EXECUTE 1 STOP");
      auto count = results->size();
      char buf[100];
      sprintf(buf, "results size=%zd", count);
      LOG(buf);
    } catch (const Exception &excp) {
      std::string logmsg = "";
      logmsg += excp.getName();
      logmsg += ": ";
      logmsg += excp.what();
      LOG(logmsg.c_str());
      LOG(excp.getStackTrace());
    }

    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo2)
  {
    auto regPtr0 = getHelper()->getRegion(regionNamesCq[0]);
    auto subregPtr0 = regPtr0->getSubregion(regionNamesCq[1]);

    QueryHelper *qh = &QueryHelper::getHelper();

    qh->populatePortfolioData(regPtr0, 140, 30, 20);
    qh->populatePositionData(subregPtr0, 140, 30);
    std::shared_ptr<Cacheable> port = nullptr;
    for (int i = 1; i < 140; i++) {
      if (!m_isPdx) {
        port = std::shared_ptr<Cacheable>(new Portfolio(i, 20));
      } else {
        port = std::shared_ptr<Cacheable>(new PortfolioPdx(i, 20));
      }
      auto keyport = CacheableKey::create("port1-1");
      regPtr0->put(keyport, port);
      SLEEP(10);  // sleep a while to allow server query to complete
    }

    LOG("StepTwo2 complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, Client1Down)
  {
    getHelper()->disconnect(true);
    cleanProc();
    LOG("Clnt1Down complete: Keepalive = True");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, Client2Down)
  {
    getHelper()->disconnect(true);
    cleanProc();
    LOG("Clnt2Down complete: Keepalive = True");
  }
END_TASK_DEFINITION

void client1Up() {
  // No RegisterIntrest again
  initClientWithId(0, true);
  createRegionForCQ(regionNamesCq[0], USE_ACK, true);

  LOG("Client1Up complete.");
  QueryHelper::getHelper();

  std::shared_ptr<QueryService> qs = getHelper()
                                         ->getCache()
                                         ->getPoolManager()
                                         .find(regionNamesCq[0])
                                         ->getQueryService();
  CqAttributesFactory cqFac;
  auto cqLstner = std::make_shared<MyCqListener>();
  cqFac.addCqListener(cqLstner);
  auto cqAttr = cqFac.create();

  const char *qryStr = "select * from /Portfolios p where p.ID < 3";
  auto qry = qs->newCq(cqName, qryStr, cqAttr);

  try {
    LOG("EXECUTE 1 START");

    qry->execute();

    LOG("EXECUTE 1 STOP");
  } catch (const Exception &excp) {
    std::string logmsg = "";
    logmsg += excp.getName();
    logmsg += ": ";
    logmsg += excp.what();
    LOG(logmsg.c_str());
    LOG(excp.getStackTrace());
  }
  try {
    getHelper()->cachePtr->readyForEvents();
  } catch (...) {
    LOG("Exception occured while sending readyForEvents");
  }
}

void client1UpDurableCQList() {
  // No RegisterIntrest again
  initClientWithId(0, true);
  createRegionForCQ(regionNamesCq[0], USE_ACK, true);

  LOG("Client1Up complete.");
  QueryHelper::getHelper();
}

void client2UpDurableCQList() {
  // No RegisterIntrest again
  initClientWithId(1, true);
  createRegionForCQ(regionNamesCq[0], USE_ACK, true);
  LOG("Client2Up complete.");
  QueryHelper::getHelper();
}

DUNIT_TASK_DEFINITION(CLIENT1, Client1Up_Pool)
  { client1Up(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, Client1UpDurableCQList_Pool)
  { client1UpDurableCQList(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, Client2UpDurableCQList_Pool)
  { client2UpDurableCQList(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFour)
  {
    QueryHelper::getHelper();

    auto pool =
        getHelper()->getCache()->getPoolManager().find(regionNamesCq[0]);
    std::shared_ptr<QueryService> qs;
    if (pool != nullptr) {
      qs = pool->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }
    char buf[1024];
    try {
      // TEST_COVERAGE
      LOG_INFO(
          "CLIENT-1 StepFour: verifying getCq() behaviour for the invalid CQ "
          "Name");
      auto invalidCqptr = qs->getCq("InValidCQ");
      ASSERT(
          invalidCqptr == nullptr,
          "Cqptr must be nullptr, as it getCq() is invoked on invalid CQ name");
      /*if(invalidCqptr == nullptr){
        LOG_INFO("Testing getCq(InvalidName) :: invalidCqptr is nullptr");
      }else{
         LOG_INFO("Testing getCq(InvalidName) :: invalidCqptr is NOT nullptr");
      }*/

      auto cqy = qs->getCq(cqName);
      cqy->stop();
      SLEEP(1500);  // sleep 0.025 min to allow server stop query to complete
      auto cqStats = cqy->getStatistics();
      sprintf(buf,
              "numInserts[%d], numDeletes[%d], numUpdates[%d], numEvents[%d]",
              cqStats->numInserts(), cqStats->numDeletes(),
              cqStats->numUpdates(), cqStats->numEvents());
      LOG(buf);
      sprintf(buf, "MyCount:onEventCount=%d, onErrorCount=%d", onEventCount,
              onErrorCount);
      LOG(buf);
      ASSERT(cqStats->numEvents() > 0, "stats incorrect!");
      cqy->close();
    } catch (Exception &excp) {
      std::string failmsg = "";
      failmsg += excp.getName();
      failmsg += ": ";
      failmsg += excp.what();
      LOG(failmsg.c_str());
      FAIL(failmsg.c_str());
      LOG(excp.getStackTrace());
    }

    LOG("StepFour complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  {
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

DUNIT_TASK_DEFINITION(SERVER1, CloseServer1)
  {
    LOG("closing Server1...");
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
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

DUNIT_TASK_DEFINITION(CLIENT1, SetPortfolioTypeToPdx)
  { m_isPdx = true; }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, UnsetPortfolioTypeToPdx)
  { m_isPdx = false; }
END_TASK_DEFINITION

bool isDurableCQName(const char *continuousQueryName, int clientID,
                     bool isRecycled) {
  bool bRetVal = false;
  int i = 0;
  if (clientID == 1) {
    if (!isRecycled) {
      for (i = 0; i < 4; i++) {
        if (strcmp(continuousQueryName, durableCQNamesClient1[i]) == 0) break;
      }
      if (i < 4) bRetVal = true;
    } else {
      for (i = 0; i < 8; i++) {
        if (strcmp(continuousQueryName, durableCQNamesClient1[i]) == 0) break;
      }
      if (i < 8) bRetVal = true;
    }
  } else if (clientID == 2) {
    if (!isRecycled) {
      for (i = 0; i < 4; i++) {
        if (strcmp(continuousQueryName, durableCQNamesClient2[i]) == 0) break;
      }
      if (i < 4) bRetVal = true;
    } else {
      for (i = 0; i < 8; i++) {
        if (strcmp(continuousQueryName, durableCQNamesClient2[i]) == 0) break;
      }
      if (i < 8) bRetVal = true;
    }
  }
  return bRetVal;
}

void doThinClientCqDurable() {
  CALL_TASK(CreateLocator);
  CALL_TASK(CreateServer1_Locator);

  CALL_TASK(StepOne_PoolLocator);
  CALL_TASK(StepOne2_PoolLocator);

  CALL_TASK(StepTwo);
  CALL_TASK(StepThree);
  CALL_TASK(StepTwo2);
  CALL_TASK(Client1Down);

  CALL_TASK(Client1Up_Pool);

  CALL_TASK(StepFour);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer1);

  CALL_TASK(CloseLocator);
}

DUNIT_TASK_DEFINITION(CLIENT1, RegisterCqs1)
  {
    auto pool =
        getHelper()->getCache()->getPoolManager().find(regionNamesCq[0]);
    std::shared_ptr<QueryService> qs;
    if (pool != nullptr) {
      qs = pool->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }
    CqAttributesFactory cqFac;
    auto cqAttr = cqFac.create();

    qs->newCq(durableCQNamesClient1[0],
              "select * from /Portfolios p where p.ID < 3", cqAttr, true)
        ->executeWithInitialResults();
    qs->newCq(durableCQNamesClient1[1],
              "select * from /Portfolios p where p.ID > 5", cqAttr, true)
        ->executeWithInitialResults();
    qs->newCq(durableCQNamesClient1[2],
              "select * from /Portfolios p where p.ID > 10", cqAttr, false)
        ->executeWithInitialResults();
    qs->newCq(durableCQNamesClient1[3],
              "select * from /Portfolios p where p.ID = 0", cqAttr, false)
        ->executeWithInitialResults();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, RegisterCqsAfterClientup1)
  {
    auto pool =
        getHelper()->getCache()->getPoolManager().find(regionNamesCq[0]);
    std::shared_ptr<QueryService> qs;
    if (pool != nullptr) {
      qs = pool->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }
    CqAttributesFactory cqFac;
    auto cqAttr = cqFac.create();

    qs->newCq(durableCQNamesClient1[4],
              "select * from /Portfolios p where p.ID < 3", cqAttr, true)
        ->executeWithInitialResults();
    qs->newCq(durableCQNamesClient1[5],
              "select * from /Portfolios p where p.ID > 5", cqAttr, true)
        ->executeWithInitialResults();
    qs->newCq(durableCQNamesClient1[6],
              "select * from /Portfolios p where p.ID > 10", cqAttr, false)
        ->executeWithInitialResults();
    qs->newCq(durableCQNamesClient1[7],
              "select * from /Portfolios p where p.ID = 0", cqAttr, false)
        ->executeWithInitialResults();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, RegisterCqs2)
  {
    auto pool =
        getHelper()->getCache()->getPoolManager().find(regionNamesCq[0]);
    std::shared_ptr<QueryService> qs;
    if (pool != nullptr) {
      qs = pool->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }
    CqAttributesFactory cqFac;
    auto cqAttr = cqFac.create();

    qs->newCq(durableCQNamesClient2[0],
              "select * from /Portfolios p where p.ID < 3", cqAttr, true)
        ->executeWithInitialResults();
    qs->newCq(durableCQNamesClient2[1],
              "select * from /Portfolios p where p.ID > 5", cqAttr, true)
        ->executeWithInitialResults();
    qs->newCq(durableCQNamesClient2[2],
              "select * from /Portfolios p where p.ID > 10", cqAttr, true)
        ->executeWithInitialResults();
    qs->newCq(durableCQNamesClient2[3],
              "select * from /Portfolios p where p.ID = 5", cqAttr, true)
        ->executeWithInitialResults();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, RegisterCqsAfterClientup2)
  {
    auto pool =
        getHelper()->getCache()->getPoolManager().find(regionNamesCq[0]);
    std::shared_ptr<QueryService> qs;
    if (pool != nullptr) {
      qs = pool->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }
    CqAttributesFactory cqFac;
    auto cqAttr = cqFac.create();

    qs->newCq(durableCQNamesClient2[4],
              "select * from /Portfolios p where p.ID < 3", cqAttr, true)
        ->executeWithInitialResults();
    qs->newCq(durableCQNamesClient2[5],
              "select * from /Portfolios p where p.ID > 5", cqAttr, true)
        ->executeWithInitialResults();
    qs->newCq(durableCQNamesClient2[6],
              "select * from /Portfolios p where p.ID > 10", cqAttr, true)
        ->executeWithInitialResults();
    qs->newCq(durableCQNamesClient2[7],
              "select * from /Portfolios p where p.ID = 5", cqAttr, true)
        ->executeWithInitialResults();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, VerifyCqs1)
  {
    auto pool =
        getHelper()->getCache()->getPoolManager().find(regionNamesCq[0]);
    std::shared_ptr<QueryService> qs;
    if (pool != nullptr) {
      qs = pool->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }

    auto durableCqListPtr = qs->getAllDurableCqsFromServer();

    ASSERT(durableCqListPtr != nullptr, "Durable CQ List should not be null");
    ASSERT(durableCqListPtr->size() == 2, "Durable CQ List lenght should be 2");
    ASSERT(
        isDurableCQName(durableCqListPtr->at(0)->toString().c_str(), 1, false),
        "Durable CQ name should be in the durable cq list");
    ASSERT(
        isDurableCQName(durableCqListPtr->at(1)->toString().c_str(), 1, false),
        "Durable CQ name should be in the durable cq list");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, VerifyCqsAfterClientup1)
  {
    auto pool =
        getHelper()->getCache()->getPoolManager().find(regionNamesCq[0]);
    std::shared_ptr<QueryService> qs;
    if (pool != nullptr) {
      qs = pool->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }

    auto durableCqListPtr = qs->getAllDurableCqsFromServer();
    ASSERT(durableCqListPtr != nullptr, "Durable CQ List should not be null");
    ASSERT(durableCqListPtr->size() == 4, "Durable CQ List length should be 4");
    ASSERT(
        isDurableCQName(durableCqListPtr->at(0)->toString().c_str(), 1, true),
        "Durable CQ name should be in the durable cq list");
    ASSERT(
        isDurableCQName(durableCqListPtr->at(1)->toString().c_str(), 1, true),
        "Durable CQ name should be in the durable cq list");
    ASSERT(
        isDurableCQName(durableCqListPtr->at(2)->toString().c_str(), 1, true),
        "Durable CQ name should be in the durable cq list");
    ASSERT(
        isDurableCQName(durableCqListPtr->at(3)->toString().c_str(), 1, true),
        "Durable CQ name should be in the durable cq list");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyCqs2)
  {
    auto pool =
        getHelper()->getCache()->getPoolManager().find(regionNamesCq[0]);
    std::shared_ptr<QueryService> qs;
    if (pool != nullptr) {
      qs = pool->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }
    auto durableCqListPtr = qs->getAllDurableCqsFromServer();
    ASSERT(durableCqListPtr != nullptr, "Durable CQ List should not be null");
    ASSERT(durableCqListPtr->size() == 4, "Durable CQ List lenght should be 4");
    ASSERT(
        isDurableCQName(durableCqListPtr->at(0)->toString().c_str(), 2, false),
        "Durable CQ name should be in the durable cq list");
    ASSERT(
        isDurableCQName(durableCqListPtr->at(1)->toString().c_str(), 2, false),
        "Durable CQ name should be in the durable cq list");
    ASSERT(
        isDurableCQName(durableCqListPtr->at(2)->toString().c_str(), 2, false),
        "Durable CQ name should be in the durable cq list");
    ASSERT(
        isDurableCQName(durableCqListPtr->at(3)->toString().c_str(), 2, false),
        "Durable CQ name should be in the durable cq list");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyCqsAfterClientup2)
  {
    auto pool =
        getHelper()->getCache()->getPoolManager().find(regionNamesCq[0]);
    std::shared_ptr<QueryService> qs;
    if (pool != nullptr) {
      qs = pool->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }
    auto durableCqListPtr = qs->getAllDurableCqsFromServer();
    ASSERT(durableCqListPtr != nullptr, "Durable CQ List should not be null");
    ASSERT(durableCqListPtr->size() == 8, "Durable CQ List lenght should be 8");
    ASSERT(
        isDurableCQName(durableCqListPtr->at(0)->toString().c_str(), 2, true),
        "Durable CQ name should be in the durable cq list");
    ASSERT(
        isDurableCQName(durableCqListPtr->at(1)->toString().c_str(), 2, true),
        "Durable CQ name should be in the durable cq list");
    ASSERT(
        isDurableCQName(durableCqListPtr->at(2)->toString().c_str(), 2, true),
        "Durable CQ name should be in the durable cq list");
    ASSERT(
        isDurableCQName(durableCqListPtr->at(3)->toString().c_str(), 2, true),
        "Durable CQ name should be in the durable cq list");
    ASSERT(
        isDurableCQName(durableCqListPtr->at(4)->toString().c_str(), 2, true),
        "Durable CQ name should be in the durable cq list");
    ASSERT(
        isDurableCQName(durableCqListPtr->at(5)->toString().c_str(), 2, true),
        "Durable CQ name should be in the durable cq list");
    ASSERT(
        isDurableCQName(durableCqListPtr->at(6)->toString().c_str(), 2, true),
        "Durable CQ name should be in the durable cq list");
    ASSERT(
        isDurableCQName(durableCqListPtr->at(7)->toString().c_str(), 2, true),
        "Durable CQ name should be in the durable cq list");
  }
END_TASK_DEFINITION

void verifyEmptyDurableCQList() {
  auto pool = getHelper()->getCache()->getPoolManager().find(regionNamesCq[0]);
  std::shared_ptr<QueryService> qs;
  if (pool != nullptr) {
    qs = pool->getQueryService();
  } else {
    qs = getHelper()->cachePtr->getQueryService();
  }

  auto durableCqListPtr = qs->getAllDurableCqsFromServer();
  ASSERT(durableCqListPtr == nullptr, "Durable CQ List should be null");
}

DUNIT_TASK_DEFINITION(CLIENT1, VerifyEmptyDurableCQList1)
  { verifyEmptyDurableCQList(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyEmptyDurableCQList2)
  { verifyEmptyDurableCQList(); }
END_TASK_DEFINITION

void getDurableCQsFromServerEmptyList() {
  CALL_TASK(CreateLocator);
  CALL_TASK(CreateServer1_Locator);

  CALL_TASK(StepOne_PoolLocator);
  CALL_TASK(StepOne2_PoolLocator);

  CALL_TASK(VerifyEmptyDurableCQList1);
  CALL_TASK(VerifyEmptyDurableCQList2);

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer1);

  CALL_TASK(CloseLocator);
}

void getDurableCQsFromServer() {
  CALL_TASK(CreateLocator);
  CALL_TASK(CreateServer1_Locator);

  CALL_TASK(StepOne_PoolLocator);
  CALL_TASK(StepOne2_PoolLocator);

  CALL_TASK(RegisterCqs1);
  CALL_TASK(RegisterCqs2);
  CALL_TASK(VerifyCqs1);
  CALL_TASK(VerifyCqs2);

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer1);

  CALL_TASK(CloseLocator);
}

void getDurableCQsFromServerWithCyclicClients() {
  CALL_TASK(CreateLocator);
  CALL_TASK(CreateServer1_Locator);

  CALL_TASK(StepOne_PoolLocator);
  CALL_TASK(StepOne2_PoolLocator);

  CALL_TASK(RegisterCqs1);
  CALL_TASK(RegisterCqs2);
  CALL_TASK(VerifyCqs1);
  CALL_TASK(VerifyCqs2);

  CALL_TASK(Client1Down);
  CALL_TASK(Client2Down);

  CALL_TASK(Client1UpDurableCQList_Pool);
  CALL_TASK(Client2UpDurableCQList_Pool);

  CALL_TASK(RegisterCqsAfterClientup1);
  CALL_TASK(RegisterCqsAfterClientup2);
  CALL_TASK(VerifyCqsAfterClientup1);
  CALL_TASK(VerifyCqsAfterClientup2);

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer1);

  CALL_TASK(CloseLocator);
}

void setPortfolioPdxType() { CALL_TASK(SetPortfolioTypeToPdx); }

void UnsetPortfolioType() { CALL_TASK(UnsetPortfolioTypeToPdx); }

void doThinClientCqDurable1() {
  CALL_TASK(CreateServer);
  // First Run of Durable Client
  CALL_TASK(RunDurableClient);
  // Intermediate Feeder, feeding events
  CALL_TASK(RunFeeder);
  // Reconnect Durable Client
  CALL_TASK(RunDurableClient);
  // Intermediate Feeder, feeding events again
  CALL_TASK(RunFeeder1);
  // Reconnect Durable Client again
  CALL_TASK(RunDurableClient);
  // Verify we get 20 events
  CALL_TASK(VerifyEvents);
  CALL_TASK(CloseServer1);
}

DUNIT_MAIN
  {
    UnsetPortfolioType();
    for (int runIdx = 1; runIdx <= 2; ++runIdx) {
      doThinClientCqDurable();

      setPortfolioPdxType();

      getDurableCQsFromServerEmptyList();

      getDurableCQsFromServer();

      getDurableCQsFromServerWithCyclicClients();
    }

    doThinClientCqDurable1();
  }
END_MAIN
