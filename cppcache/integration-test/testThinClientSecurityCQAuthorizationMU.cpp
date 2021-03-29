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
#include <geode/CqServiceStatistics.hpp>
#include <geode/AuthenticatedView.hpp>
#include <ace/OS.h>
#include <ace/High_Res_Timer.h>
#include <string>

#define ROOT_NAME "testThinClientSecurityCQAuthorizationMU"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"

#include "QueryStrings.hpp"
#include "QueryHelper.hpp"

#include <geode/Query.hpp>
#include <geode/QueryService.hpp>

#include "ThinClientCQ.hpp"

#include "CacheHelper.hpp"
#include "ThinClientHelper.hpp"
#include <ace/Process.h>

using apache::geode::client::testframework::security::CredentialGenerator;

using apache::geode::client::AuthenticatedView;
using apache::geode::client::CqAttributesFactory;
using apache::geode::client::CqEvent;
using apache::geode::client::CqListener;
using apache::geode::client::CqOperation;
using apache::geode::client::Exception;
using apache::geode::client::IllegalStateException;
using apache::geode::client::QueryService;

const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
std::shared_ptr<CredentialGenerator> credentialGeneratorHandler;
#define CLIENT1 s1p1
#define SERVER1 s2p1
#define CLIENT2 s1p2

#define MAX_LISTNER 8

const char *cqNames[MAX_LISTNER] = {"MyCq_0", "MyCq_1", "MyCq_2", "MyCq_3",
                                    "MyCq_4", "MyCq_5", "MyCq_6", "MyCq_7"};

const char *queryStrings[MAX_LISTNER] = {
    "select * from /Portfolios p where p.ID < 1",
    "select * from /Portfolios p where p.ID < 2",
    "select * from /Portfolios p where p.ID = 2",
    "select * from /Portfolios p where p.ID >= 3",
    "select * from /Portfolios p where p.ID = 4",
    "select * from /Portfolios p where p.ID = 5",
    "select * from /Portfolios p where p.ID = 6",
    "select * from /Portfolios p where p.ID = 7"};

const char *regionNamesCq[] = {"Portfolios", "Positions", "Portfolios2",
                               "Portfolios3"};

class MyCqListener : public CqListener {
  uint8_t m_id;
  uint32_t m_numInserts;
  uint32_t m_numUpdates;
  uint32_t m_numDeletes;
  uint32_t m_numEvents;

 public:
  uint8_t getId() { return m_id; }
  uint32_t getNumInserts() { return m_numInserts; }
  uint32_t getNumUpdates() { return m_numUpdates; }
  uint32_t getNumDeletes() { return m_numDeletes; }
  uint32_t getNumEvents() { return m_numEvents; }
  explicit MyCqListener(uint8_t id)
      : m_id(id),
        m_numInserts(0),
        m_numUpdates(0),
        m_numDeletes(0),
        m_numEvents(0) {}
  ~MyCqListener() noexcept override = default;
  inline void updateCount(const CqEvent &cqEvent) {
    printf(" in cqEvent.getQueryOperation() %d id = %d\n",
           static_cast<int>(cqEvent.getQueryOperation()), m_id);
    printf(" in update key = %s \n",
           (dynamic_cast<CacheableString *>(cqEvent.getKey().get()))
               ->value()
               .c_str());
    m_numEvents++;
    switch (cqEvent.getQueryOperation()) {
      case CqOperation::OP_TYPE_CREATE:
        m_numInserts++;
        break;
      case CqOperation::OP_TYPE_UPDATE:
        m_numUpdates++;
        break;
      case CqOperation::OP_TYPE_DESTROY:
        m_numDeletes++;
        break;
      case CqOperation::OP_TYPE_INVALID:
      case CqOperation::OP_TYPE_INVALIDATE:
      case CqOperation::OP_TYPE_REGION_CLEAR:
      case CqOperation::OP_TYPE_MARKER:
        break;
    }
    printf(" in create = %d, update = %d , delete = %d ", m_numInserts,
           m_numUpdates, m_numDeletes);
  }

  void onEvent(const CqEvent &cqe) override {
    LOG("MyCqListener::OnEvent called");
    updateCount(cqe);
  }
  void onError(const CqEvent &cqe) override {
    updateCount(cqe);
    LOG("MyCqListener::OnError called");
  }
  void close() override { LOG("MyCqListener::close called"); }
};

std::string getXmlPath() {
  char xmlPath[1000] = {'\0'};
  const char *path = std::getenv("TESTSRC");
  ASSERT(path != nullptr,
         "Environment variable TESTSRC for test source directory is not set.");
  strncpy(xmlPath, path, strlen(path) - strlen("cppcache"));
  strncat(xmlPath, "xml/Security/", sizeof(xmlPath) - strlen(xmlPath) - 1);
  return std::string(xmlPath);
}

void initCredentialGenerator() {
  credentialGeneratorHandler = CredentialGenerator::create("DUMMY3");

  if (credentialGeneratorHandler == nullptr) {
    FAIL("credentialGeneratorHandler is nullptr");
  }
}
std::shared_ptr<Properties> userCreds;
void initClientCq(const bool isthinClient) {
  userCreds = Properties::create();
  auto config = Properties::create();
  // credentialGeneratorHandler->getAuthInit(config);
  credentialGeneratorHandler->getValidCredentials(userCreds);

  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(isthinClient, config);
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

DUNIT_TASK_DEFINITION(CLIENT1, CreateServer1)
  {
    initCredentialGenerator();
    std::string cmdServerAuthenticator;

    if (isLocalServer) {
      cmdServerAuthenticator = credentialGeneratorHandler->getServerCmdParams(
          "authenticator:authorizer:authorizerPP", getXmlPath());
      printf("string %s", cmdServerAuthenticator.c_str());
      CacheHelper::initServer(
          1, "remotequery.xml", {},
          const_cast<char *>(cmdServerAuthenticator.c_str()));
      LOG("Server1 started");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CreateServer2)
  {
    std::string cmdServerAuthenticator;

    if (isLocalServer) {
      cmdServerAuthenticator = credentialGeneratorHandler->getServerCmdParams(
          "authenticator:authorizer:authorizerPP", getXmlPath());
      printf("string %s", cmdServerAuthenticator.c_str());
      CacheHelper::initServer(
          2, "remotequery2.xml", nullptr,
          const_cast<char *>(cmdServerAuthenticator.c_str()));
      LOG("Server2 started");
    }
    SLEEP(20000);
  }
END_TASK_DEFINITION

void stepOne() {
  LOG("StepOne1 complete. 1");
  initClientCq(true);
  LOG("StepOne1 complete. 2");
  createRegionForCQMU(regionNamesCq[0], USE_ACK, false);
  LOG("StepOne1 complete. 3");
  auto regptr = getHelper()->getRegion(regionNamesCq[0]);
  LOG("StepOne1 complete. 4");
  LOG("StepOne1 complete. 5");
  auto subregPtr =
      regptr->createSubregion(regionNamesCq[1], regptr->getAttributes());

  LOG("StepOne complete.");
}

void stepOne2() {
  LOG("StepOne2 complete. 1");
  initClientCq(true);
  LOG("StepOne2 complete. 2");
  createRegionForCQMU(regionNamesCq[0], USE_ACK, false);
  LOG("StepOne2 complete. 3");
  auto regptr = getHelper()->getRegion(regionNamesCq[0]);
  LOG("StepOne2 complete. 4");
  LOG("StepOne2 complete. 5");
  auto subregPtr =
      regptr->createSubregion(regionNamesCq[1], regptr->getAttributes());

  LOG("StepOne2 complete.");
}

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_PoolEP)
  { stepOne(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepOne2_PoolEP)
  {
    initCredentialGenerator();
    stepOne2();
  }
END_TASK_DEFINITION
std::shared_ptr<Pool> getPool(const char *name) {
  return getHelper()->getCache()->getPoolManager().find(name);
}

static std::shared_ptr<QueryService> userQueryService;

AuthenticatedView setUpAuthenticatedView(const int userId) {
  auto creds = Properties::create();
  char tmp[25] = {'\0'};
  sprintf(tmp, "user%d", userId);

  creds->insert("security-username", tmp);
  creds->insert("security-password", tmp);
  return getHelper()->getCache()->createAuthenticatedView(creds,
                                                          regionNamesCq[0]);
}

DUNIT_TASK_DEFINITION(CLIENT1, StepTwo)
  {
    auto authenticatedView = setUpAuthenticatedView(4);

    auto regPtr0 = authenticatedView.getRegion(regionNamesCq[0]);
    auto subregPtr0 = regPtr0->getSubregion(regionNamesCq[1]);

    authenticatedView.close();
    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    uint8_t i = 0;
    // QueryHelper * qh = &QueryHelper::getHelper();
    // userCache = getVirtualCache(userCreds, regionNamesCq[0]);
    auto authenticatedView = setUpAuthenticatedView(4);
    userQueryService = authenticatedView.getQueryService();
    std::shared_ptr<QueryService> qs;

    qs = userQueryService;

    try {
      for (i = 0; i < MAX_LISTNER; i++) {
        auto cqLstner = std::make_shared<MyCqListener>(i);
        CqAttributesFactory cqFac;
        cqFac.addCqListener(cqLstner);
        auto cqAttr = cqFac.create();

        auto qry = qs->newCq(cqNames[i], queryStrings[i], cqAttr);
        qry->execute();
      }

      LOG("EXECUTE 1 START");

      // qs->executeCqs();

      LOG("EXECUTE 1 STOP");
    } catch (const Exception &excp) {
      std::string logmsg = "";
      logmsg += excp.getName();
      logmsg += ": ";
      logmsg += excp.what();
      LOG(logmsg.c_str());
      LOG(excp.getStackTrace());
    }

    authenticatedView.close();
    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo2)
  {
    auto authenticatedView = setUpAuthenticatedView(3);
    auto regPtr0 = authenticatedView.getRegion(regionNamesCq[0]);
    auto subregPtr0 = regPtr0->getSubregion(regionNamesCq[1]);

    QueryHelper *qh = &QueryHelper::getHelper();

    qh->populatePortfolioData(regPtr0, 3, 2, 1);
    qh->populatePositionData(subregPtr0, 3, 2);
    for (int i = 1; i <= 4; i++) {
      auto port = std::make_shared<Portfolio>(i, 2);

      char tmp[25] = {'\0'};
      sprintf(tmp, "port1-%d", i);
      auto keyport = CacheableKey::create(tmp);
      regPtr0->put(keyport, port);
      SLEEP(10);  // sleep a while to allow server query to complete
    }

    LOG("StepTwo2 complete. Sleeping .25 min for server query to complete...");
    SLEEP(15000);  // sleep .25 min to allow server query to complete
    authenticatedView.close();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFour)
  {
    // auto userCache = getVirtualCache(userCreds, regionNamesCq[0]);
    std::shared_ptr<QueryService> qs;

    qs = userQueryService;

    char buf[1024];

    uint8_t i = 0;

    CqAttributesFactory cqFac;
    for (i = 0; i < MAX_LISTNER; i++) {
      sprintf(buf, "get info for cq[%s]:", cqNames[i]);
      LOG(buf);
      auto cqy = qs->getCq(cqNames[i]);
      // auto cqStats = cqy->getStatistics();
    }

    // if key port1-4 then only query 3 and 4 will satisfied
    auto cqy = qs->getCq(cqNames[3]);
    auto cqAttr = cqy->getCqAttributes();
    auto vl = cqAttr->getCqListeners();

    auto cqListener_3 = static_cast<MyCqListener *>(vl[0].get());
    printf(" cqListener_3 should have one create event = %d \n",
           cqListener_3->getNumInserts());
    ASSERT(cqListener_3->getNumInserts() == 1,
           "incorrect number of events got listener 3");

    cqy = qs->getCq(cqNames[4]);
    cqAttr = cqy->getCqAttributes();
    vl = cqAttr->getCqListeners();

    auto cqListener_4 = static_cast<MyCqListener *>(vl[0].get());
    printf(" cqListener_4 should have one create event = %d \n",
           cqListener_4->getNumInserts());
    ASSERT(cqListener_4->getNumInserts() == 1,
           "incorrect number of events got listener 4");

    /*for(i=0; i < MAX_LISTNER; i++)
    {
      sprintf(buf, "get info for cq[%s]:", cqNames[i]);
      LOG(buf);
     auto cqy = qs->getCq(cqNames[i]);
      cqy->stop();
      cqy->close();
     //auto cqStats = cqy->getStatistics();
    }*/

    // userCache->close();
    LOG("StepFour complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFour2)
  {
    auto qs = userQueryService;

    char buf[1024];

    uint8_t i = 0;

    for (i = 0; i < MAX_LISTNER; i++) {
      sprintf(buf, "get info for cq[%s]:", cqNames[i]);
      LOG(buf);
      auto cqy = qs->getCq(cqNames[i]);
      // auto cqStats = cqy->getStatistics();
    }

    // if key port1-4 then only query 3 and 4 will satisfied
    auto cqy = qs->getCq(cqNames[3]);
    auto cqAttr = cqy->getCqAttributes();
    auto vl = cqAttr->getCqListeners();

    MyCqListener *cqListener_3 = static_cast<MyCqListener *>(vl[0].get());
    printf(" cqListener_3 should have one update event = %d \n",
           cqListener_3->getNumUpdates());
    ASSERT(cqListener_3->getNumUpdates() == 1,
           "incorrect number of events got listener 3");

    cqy = qs->getCq(cqNames[4]);
    cqAttr = cqy->getCqAttributes();
    vl = cqAttr->getCqListeners();

    auto cqListener_4 = static_cast<MyCqListener *>(vl[0].get());
    printf(" cqListener_4 should have one update event = %d \n",
           cqListener_4->getNumUpdates());
    ASSERT(cqListener_4->getNumUpdates() == 1,
           "incorrect number of events got listener 4");

    /*for(i=0; i < MAX_LISTNER; i++)
    {
      sprintf(buf, "get info for cq[%s]:", cqNames[i]);
      LOG(buf);
     auto cqy = qs->getCq(cqNames[i]);
      cqy->stop();
      cqy->close();
     //auto cqStats = cqy->getStatistics();
    }*/

    LOG("StepFour2 complete.");
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
    SLEEP(5000);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer2)
  {
    LOG("closing Server2...");
    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
    }
  }
END_TASK_DEFINITION

void doThinClientCq() {
  //  CALL_TASK(CreateLocator);
  //  CALL_TASK(CreateServer1_Locator);
  //
  //  CALL_TASK(StepOne_PoolLocator);
  //  CALL_TASK(StepOne2_PoolLocator);

  CALL_TASK(StepTwo);
  CALL_TASK(StepThree);
  CALL_TASK(StepTwo2);
  CALL_TASK(StepFour);  // validates listener events
  CALL_TASK(CreateServer2);
  CALL_TASK(CloseServer1);
  CALL_TASK(StepTwo2);  // again put data
  CALL_TASK(StepFour2);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer2);

  //  CALL_TASK(CloseLocator);
}

DUNIT_MAIN
  { doThinClientCq(); }
END_MAIN
