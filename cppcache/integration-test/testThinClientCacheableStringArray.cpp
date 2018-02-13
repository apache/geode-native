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

#define ROOT_NAME "TestThinClientCacheableStringArray"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"
#include "ThinClientHelper.hpp"

#include "QueryStrings.hpp"
#include "QueryHelper.hpp"

#include <geode/Query.hpp>
#include <geode/QueryService.hpp>

#include "SerializationRegistry.hpp"
#include "CacheRegionHelper.hpp"
#include "CacheImpl.hpp"

using namespace apache::geode::client;
using namespace test;
using namespace testobject;

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2

static int numberOfLocators = 1;
bool isLocalServer = true;
bool isLocator = true;
const char* locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);

const char* _regionNames[] = {"Portfolios", "Positions"};

DUNIT_TASK(SERVER1, CreateServer1)
  {
    LOG("Starting SERVER1...");
    CacheHelper::initLocator(1);
    if (isLocalServer) {
      CacheHelper::initServer(1, "remotequery.xml", locHostPort);
    }
    LOG("SERVER1 started");
  }
END_TASK(CreateServer1)

DUNIT_TASK(CLIENT1, StepOne)
  {
    initClientWithPool(true, "__TEST_POOL1__", locHostPort, nullptr, nullptr, 0,
                       true);
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    serializationRegistry->addType(Position::createDeserializable);
    serializationRegistry->addType(Portfolio::createDeserializable);

    auto regptr = getHelper()->createPooledRegion(
        _regionNames[0], USE_ACK, locHostPort, "__TEST_POOL1__", true, true);
    std::shared_ptr<RegionAttributes> lattribPtr = regptr->getAttributes();
    auto subregPtr = regptr->createSubregion(_regionNames[1], lattribPtr);

    auto&& qh = &QueryHelper::getHelper();
    std::vector<std::shared_ptr<CacheableString>> cstr{
        CacheableString::create("Taaa"), CacheableString::create("Tbbb"),
        CacheableString::create("Tccc"), CacheableString::create("Tddd")};
    auto nm = CacheableStringArray::create(cstr);
    qh->populatePortfolioData(regptr, 4, 3, 2, nm);
    qh->populatePositionData(subregPtr, 4, 3);

    LOG("StepOne complete.");
  }
END_TASK(StepOne)

DUNIT_TASK(CLIENT1, StepThree)
  {
    try {
      auto qs = getHelper()->cachePtr->getQueryService("__TEST_POOL1__");

      char* qryStr = (char*)"select * from /Portfolios p where p.ID < 3";
      auto qry = qs->newQuery(qryStr);
      std::shared_ptr<SelectResults> results;
      results = qry->execute();

      SelectResultsIterator iter = results->getIterator();
      char buf[100];
      int count = results->size();
      sprintf(buf, "results size=%d", count);
      LOG(buf);
      while (iter.hasNext()) {
        count--;
        auto ser = iter.next();
        auto portfolio = std::dynamic_pointer_cast<Portfolio>(ser);
        auto position = std::dynamic_pointer_cast<Position>(ser);

        if (portfolio != nullptr) {
          printf("   query pulled portfolio object ID %d, pkid %s\n",
                 portfolio->getID(), portfolio->getPkid()->value().c_str());
        }

        else if (position != nullptr) {
          printf("   query  pulled position object secId %s, shares %d\n",
                 position->getSecId()->value().c_str(),
                 position->getSharesOutstanding());
        }

        else {
          if (ser != nullptr) {
            printf(" query pulled object %s\n", ser->toString().c_str());
          } else {
            printf("   query pulled bad object\n");
          }
        }
      }
      sprintf(buf, "results last count=%d", count);
      LOG(buf);
    } catch (IllegalStateException& ise) {
      char isemsg[500] = {0};
      ACE_OS::snprintf(isemsg, 499, "IllegalStateException: %s",
                       ise.what());
      LOG(isemsg);
      FAIL(isemsg);
    } catch (Exception& excp) {
      char excpmsg[500] = {0};
      ACE_OS::snprintf(excpmsg, 499, "Exception: %s", excp.what());
      LOG(excpmsg);
      FAIL(excpmsg);
    } catch (...) {
      LOG("Got an exception!");
      FAIL("Got an exception!");
    }

    LOG("StepThree complete.");
  }
END_TASK(StepThree)

DUNIT_TASK(CLIENT1, CloseCache1)
  {
    LOG("cleanProc 1...");
    cleanProc();
  }
END_TASK(CloseCache1)

DUNIT_TASK(SERVER1, CloseServer1)
  {
    LOG("closing Server1...");
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
  }
END_TASK(CloseServer1)
