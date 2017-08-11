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
#include <geode/CqAttributesFactory.hpp>
#include <geode/CqAttributes.hpp>
#include <geode/CqListener.hpp>
#include <geode/CqQuery.hpp>
#include <geode/Struct.hpp>
#include <geode/CqResults.hpp>
#define ROOT_NAME "TestThinClientCqWithIR"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"

#include "QueryStrings.hpp"
#include "QueryHelper.hpp"

#include <geode/Query.hpp>
#include <geode/QueryService.hpp>

#include "ThinClientCQ.hpp"

using namespace apache::geode::client;
using namespace test;
using namespace testData;

#define CLIENT1 s1p1
#define SERVER1 s2p1
#define CLIENT2 s1p2

const char* cqName = "MyCq";

void initClientCq(const bool isthinClient) {
  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(isthinClient);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");

  try {
    SerializationRegistryPtr serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    serializationRegistry->addType(Position::createDeserializable);
    serializationRegistry->addType(Portfolio::createDeserializable);

    serializationRegistry->addPdxType(PositionPdx::createDeserializable);
    serializationRegistry->addPdxType(PortfolioPdx::createDeserializable);
  } catch (const IllegalStateException&) {
    // ignore exception
  }
}
const char* regionNamesCq[] = {"Portfolios", "Positions", "Portfolios2",
                               "Portfolios3"};

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
                            locator ? locatorsG : nullptr);
  }
  LOG("SERVER1 started");
}

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1_Locator)
  { createServer(true); }
END_TASK_DEFINITION

void stepOne(bool pool = false, bool locator = false) {}

DUNIT_TASK_DEFINITION(CLIENT1, CreateClient1Regions)
  {
    initClientCq(true);
    createRegionForCQ(regionNamesCq[0], USE_ACK, true);
    createRegionForCQ(regionNamesCq[2], USE_ACK, true);
    RegionPtr regptr = getHelper()->getRegion(regionNamesCq[0]);
    RegionAttributesPtr lattribPtr = regptr->getAttributes();
    RegionPtr subregPtr = regptr->createSubregion(regionNamesCq[1], lattribPtr);

    LOG("CreateClient1Regions complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CreateClient2Regions)
  {
    initClientCq(true);
    createRegionForCQ(regionNamesCq[0], USE_ACK, true);
    RegionPtr regptr = getHelper()->getRegion(regionNamesCq[0]);
    RegionAttributesPtr lattribPtr = regptr->getAttributes();
    RegionPtr subregPtr = regptr->createSubregion(regionNamesCq[1], lattribPtr);

    LOG("CreateClient2Regions complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, PopulateData)
  {
    RegionPtr regPtr0 = getHelper()->getRegion(regionNamesCq[0]);
    RegionPtr subregPtr0 = regPtr0->getSubregion(regionNamesCq[1]);
    RegionPtr regPtr1 = getHelper()->getRegion(regionNamesCq[2]);

    QueryHelper* qh = &QueryHelper::getHelper();

    qh->populatePortfolioPdxData(regPtr0, 30, 20, 20);
    qh->populatePortfolioPdxData(regPtr1, 30, 20, 20);
    qh->populatePositionPdxData(subregPtr0, 30, 20);

    LOG("PopulateData complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT2, PutData)
  {
    RegionPtr regPtr0 = getHelper()->getRegion(regionNamesCq[0]);
    RegionPtr subregPtr0 = regPtr0->getSubregion(regionNamesCq[1]);

    QueryHelper* qh = &QueryHelper::getHelper();
    qh->populatePortfolioPdxData(regPtr0, 150, 40, 150);
    qh->populatePositionPdxData(subregPtr0, 150, 40);

    CacheablePtr port = nullptr;
    for (int i = 1; i < 150; i++) {
      port = CacheablePtr(new PortfolioPdx(i, 150));

      CacheableKeyPtr keyport = CacheableKey::create((char*)"port1-1");
      regPtr0->put(keyport, port);
      SLEEP(100);  // sleep a while to allow server query to complete
    }

    LOG("PopulateData2 complete.");
    SLEEP(15000);  // sleep 0.25 min to allow server query to complete
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, QueryData)
  {
    auto& qh ATTR_UNUSED = QueryHelper::getHelper();

    // using region name as pool name
    auto pool =
        getHelper()->getCache()->getPoolManager().find(regionNamesCq[0]);
    QueryServicePtr qs;
    if (pool != nullptr) {
      qs = pool->getQueryService();
    } else {
      qs = getHelper()->cachePtr->getQueryService();
    }

    auto cqAttr = CqAttributesFactory().create();

    // char* qryStr = (char*)"select * from /Portfolios p where p.ID != 2";
    // qry->execute();

    auto qryStr = "select * from /Portfolios where ID != 2";
    auto qry = qs->newCq(cqName, qryStr, cqAttr);

    try {
      LOG("before executing executeWithInitialResults.");
      auto results = qry->executeWithInitialResults();
      LOG("before executing executeWithInitialResults done.");

      auto iter = results->getIterator();
      char buf[100];
      int count = results->size();
      sprintf(buf, "results size=%d", count);
      LOG(buf);
      ASSERT(count > 0, "count should be > 0");
      while (iter.hasNext()) {
        count--;
        auto ser = iter.next();

        if (ser != nullptr) {
          printf(" query pulled object %s\n", ser->toString()->asChar());

          auto stPtr = std::dynamic_pointer_cast<Struct>(ser);
          ASSERT(stPtr != nullptr, "Failed to get struct in CQ result.");

          if (stPtr != nullptr) {
            LOG(" got struct ptr ");
            auto serKey = (*stPtr)["key"];
            ASSERT(serKey != nullptr, "Failed to get KEY in CQ result.");
            if (serKey != nullptr) {
              LOG("got struct key ");
              printf("  got struct key %s\n", serKey->toString()->asChar());
            }

            auto serVal = (*stPtr)["value"];
            ASSERT(serVal != nullptr, "Failed to get VALUE in CQ result.");

            if (serVal != nullptr) {
              LOG("got struct value ");
              printf("  got struct value %s\n", serVal->toString()->asChar());
            }
          }
        } else {
          printf("   query pulled bad object\n");
        }
      }
      sprintf(buf, "results last count=%d", count);
      LOG(buf);

      qry = qs->newCq("MyCq2", "select * from /Portfolios2", cqAttr);

      LOG("before executing executeWithInitialResults2.");
      results = qry->executeWithInitialResults();
      LOG("before executing executeWithInitialResults2 done.");

      auto iter2 = results->getIterator();

      count = results->size();
      sprintf(buf, "results2 size=%d", count);
      LOG(buf);
      ASSERT(count > 0, "count should be > 0");
      while (iter2.hasNext()) {
        count--;
        auto ser = iter2.next();

        if (ser != nullptr) {
          printf(" query pulled object %s\n", ser->toString()->asChar());

          auto stPtr = std::dynamic_pointer_cast<Struct>(ser);
          ASSERT(stPtr != nullptr, "Failed to get struct in CQ result.");

          if (stPtr != nullptr) {
            LOG(" got struct ptr ");
            auto serKey = (*stPtr)["key"];
            ASSERT(serKey != nullptr, "Failed to get KEY in CQ result.");
            if (serKey != nullptr) {
              LOG("got struct key ");
              printf("  got struct key %s\n", serKey->toString()->asChar());
            }

            auto serVal = (*stPtr)["value"];
            ASSERT(serVal != nullptr, "Failed to get VALUE in CQ result.");

            if (serVal != nullptr) {
              LOG("got struct value ");
              printf("  got struct value %s\n", serVal->toString()->asChar());
            }
          }
        } else {
          printf("   query pulled bad object\n");
        }
      }
      sprintf(buf, "results last count=%d", count);
      LOG(buf);

      {
        auto regPtr0 = getHelper()->getRegion(regionNamesCq[0]);
        regPtr0->destroyRegion();
      }
      SLEEP(20000);
      qry = qs->getCq(cqName);
      sprintf(buf, "cq[%s] should have been removed after close!", cqName);
      ASSERT(qry == nullptr, buf);
    } catch (const Exception& excp) {
      std::string logmsg = "";
      logmsg += excp.getName();
      logmsg += ": ";
      logmsg += excp.getMessage();
      LOG(logmsg.c_str());
      excp.printStackTrace();
      ASSERT(false, logmsg.c_str());
    }

    LOG("QueryData complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CheckRegionDestroy)
  {
    LOG("check region destory");
    try {
      RegionPtr regPtr0 = getHelper()->getRegion(regionNamesCq[0]);
      if (regPtr0 == nullptr) {
        LOG("regPtr0==nullptr");
      } else {
        LOG("regPtr0!=nullptr");
        ASSERT(regPtr0->isDestroyed(), "should have been distroyed");
      }
    } catch (...) {
      LOG("exception in getting region");
    }
    LOG("region has been destoryed");
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

DUNIT_MAIN
  {
    CALL_TASK(CreateLocator);
    CALL_TASK(CreateServer1_Locator);

    CALL_TASK(CreateClient1Regions);
    CALL_TASK(CreateClient2Regions);
    CALL_TASK(PopulateData);
    CALL_TASK(PutData);
    CALL_TASK(QueryData);

    CALL_TASK(CloseCache1);
    CALL_TASK(CloseCache2);
    CALL_TASK(CloseServer1);
    CALL_TASK(CloseLocator);
  }
END_MAIN
