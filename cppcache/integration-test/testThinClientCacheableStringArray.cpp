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
#include <hacks/range.h>

#define CLIENT1 s1p1
#define SERVER1 s2p1

using apache::geode::client::Exception;
using apache::geode::client::IllegalStateException;

static int numberOfLocators = 1;
bool isLocalServer = true;
bool isLocator = true;
const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);

const char *_regionNames[] = {"Portfolios", "Positions"};

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

    serializationRegistry->addDataSerializableType(
        Position::createDeserializable, 2);
    serializationRegistry->addDataSerializableType(
        Portfolio::createDeserializable, 3);

    auto regptr = getHelper()->createPooledRegion(
        _regionNames[0], USE_ACK, locHostPort, "__TEST_POOL1__", true, true);
    auto subregPtr =
        regptr->createSubregion(_regionNames[1], regptr->getAttributes());

    auto &&qh = &QueryHelper::getHelper();
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
      auto &&qs = getHelper()->cachePtr->getQueryService("__TEST_POOL1__");
      auto qryStr = "select * from /Portfolios p where p.ID < 3";
      auto &&qry = qs->newQuery(qryStr);
      auto &&results = qry->execute();

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
