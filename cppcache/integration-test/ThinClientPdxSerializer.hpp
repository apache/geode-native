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

#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTPDXSERIALIZER_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTPDXSERIALIZER_H_

#include <string>

#include <ace/OS.h>
#include <ace/High_Res_Timer.h>

#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"
#include "Utils.hpp"
#include "testobject/PdxClassV1.hpp"
#include "testobject/PdxClassV2.hpp"
#include "testobject/NonPdxType.hpp"
#include "ThinClientPdxSerializers.hpp"
#include "CacheRegionHelper.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define LOCATOR s2p2
#define SERVER1 s2p1

namespace { // NOLINT(google-build-namespaces)

using apache::geode::client::CacheableBoolean;
using apache::geode::client::UserObjectSizer;

using PdxTests::PdxWrapper;
using PdxTests::TestPdxSerializerForV1;

bool isLocator = false;
bool isLocalServer = false;

const char* poolNames[] = {"Pool1", "Pool2", "Pool3"};
const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
bool isPoolConfig = false;  // To track if pool case is running

void initClient(const bool isthinClient, bool isPdxIgnoreUnreadFields) {
  LOG_INFO("initClient: isPdxIgnoreUnreadFields = %d ", isPdxIgnoreUnreadFields);
  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(isthinClient, isPdxIgnoreUnreadFields, false,
                                  nullptr, false);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

void stepOne(bool isPdxIgnoreUnreadFields = false) {
  // Create just one pool and attach all regions to that.
  initClient(true, isPdxIgnoreUnreadFields);
  isPoolConfig = true;
  createPool(poolNames[0], locHostPort, {}, 0, true);
  createRegionAndAttachPool("DistRegionAck", USE_ACK, poolNames[0],
                            false /*Caching disabled*/);
  LOG("StepOne complete.");
}

DUNIT_TASK_DEFINITION(LOCATOR, StartLocator)
  {
    // starting locator 1 2
    if (isLocator) {
      CacheHelper::initLocator(1);
    }
    LOG("Locator started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOnePoolLoc_PDX)
  {
    LOG("Starting Step One with Pool + Locator lists");
    stepOne(true);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOnePoolLoc)
  {
    LOG("Starting Step One with Pool + Locator lists");
    stepOne();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServerWithLocator)
  {
    LOG("Starting SERVER1...");
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserverPdx.xml", locHostPort);
    }
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServerWithLocator1)
  {
    LOG("Starting SERVER1...");
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver.xml", locHostPort);
    }
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServerWithLocator2)
  {
    LOG("Starting SERVER1...");
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserverForPdx.xml", locHostPort);
    }
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServerWithLocator3)
  {
    LOG("Starting SERVER1...");
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserverPdxSerializer.xml", locHostPort);
    }
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwoPoolLoc)
  {
    LOG("Starting Step Two with Pool + Locator");
    stepOne();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwoPoolLoc_PDX)
  {
    LOG("Starting Step Two with Pool + Locator");
    stepOne(true);
  }
END_TASK_DEFINITION

void checkPdxInstanceToStringAtServer(std::shared_ptr<Region> region) {
  auto keyPort = CacheableKey::create("success");
  auto checkPdxInstanceToStringAtServer =
      std::dynamic_pointer_cast<CacheableBoolean>(region->get(keyPort));
  bool actualValue = checkPdxInstanceToStringAtServer->value();
  ASSERT(actualValue == true,
         "checkPdxInstanceToStringAtServer: Val should be true");
}

DUNIT_TASK_DEFINITION(CLIENT1, JavaPutGet)
  {
    CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
        ->getTypeRegistry()
        .registerPdxSerializer(std::make_shared<TestPdxSerializer>());

    auto region0 = getHelper()->getRegion("DistRegionAck");
    auto keyPort = CacheableKey::create(1);

    auto nonPdxType1 = std::make_shared<PdxTests::NonPdxType>();
    auto pdxWrapper1 = std::make_shared<PdxWrapper>(nonPdxType1, CLASSNAME1);
    region0->put(keyPort, pdxWrapper1);

    auto pdxWrapper2 =
        std::dynamic_pointer_cast<PdxWrapper>(region0->get(keyPort));

    auto regionGetSuccess =
        std::dynamic_pointer_cast<CacheableBoolean>(region0->get("success"));
    bool actualValue = regionGetSuccess->value();

    ASSERT(actualValue == true,
           "Task JavaPutGet:Objects of type NonPdxType should be equal");

    auto nonPdxType2 = std::static_pointer_cast<PdxTests::NonPdxType>(
        pdxWrapper2->getObject());

    ASSERT(nonPdxType1->equals(*nonPdxType2, false), "NonPdxType compare");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, JavaGet)
  {
    CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
        ->getTypeRegistry()
        .registerPdxSerializer(std::make_shared<TestPdxSerializer>());

    LOG_DEBUG("JavaGet-1 Line_309");
    auto region0 = getHelper()->getRegion("DistRegionAck");

    auto keyPort1 = CacheableKey::create(1);

    LOG_DEBUG("JavaGet-2 Line_314");
    auto pdxWrapper1 =
        std::dynamic_pointer_cast<PdxWrapper>(region0->get(keyPort1));
    auto nonPdxType1 = std::static_pointer_cast<PdxTests::NonPdxType>(
        pdxWrapper1->getObject());

    LOG_DEBUG("JavaGet-3 Line_316");
    auto keyPort2 = CacheableKey::create("putFromjava");

    LOG_DEBUG("JavaGet-4 Line_316");
    auto pdxWrapper2 =
        std::dynamic_pointer_cast<PdxWrapper>(region0->get(keyPort2));
    auto nonPdxWrapper2 = std::static_pointer_cast<PdxTests::NonPdxType>(
        pdxWrapper2->getObject());

    LOG_DEBUG("JavaGet-5 Line_320");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, putFromVersion1_PS)
  {
    auto region0 = getHelper()->getRegion("DistRegionAck");
    auto key = CacheableKey::create(1);

    CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
        ->getTypeRegistry()
        .registerPdxSerializer(std::make_shared<TestPdxSerializerForV1>());

    // Create New object and wrap it in PdxWrapper (owner)
    auto nonPdxType1 = std::make_shared<PdxTests::TestDiffTypePdxSV1>(true);
    auto pdxWrapper = std::make_shared<PdxWrapper>(nonPdxType1, V1CLASSNAME2);

    // PUT
    region0->put(key, pdxWrapper);

    // GET
    auto pdxWrapper2 = std::dynamic_pointer_cast<PdxWrapper>(region0->get(key));
    auto nonPdxType2 = std::static_pointer_cast<PdxTests::TestDiffTypePdxSV1>(
        pdxWrapper2->getObject());

    // Equal check
    bool isEqual = nonPdxType1->equals(*nonPdxType2);
    LOG_DEBUG("putFromVersion1_PS isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Task putFromVersion1_PS:Objects of type TestPdxSerializerForV1 "
           "should be equal");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, putFromVersion2_PS)
  {
    auto region0 = getHelper()->getRegion("DistRegionAck");
    auto key = CacheableKey::create(1);

    CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
        ->getTypeRegistry()
        .registerPdxSerializer(
            std::shared_ptr<PdxSerializer>(new TestPdxSerializerForV2));

    // Create New object and wrap it in PdxWrapper (owner)
    auto nonPdxType1 = std::make_shared<PdxTests::TestDiffTypePdxSV2>(true);
    auto pdxWrapper = std::make_shared<PdxWrapper>(nonPdxType1, V2CLASSNAME4);

    // PUT
    region0->put(key, pdxWrapper);

    // GET
    auto pdxWrapper2 = std::dynamic_pointer_cast<PdxWrapper>(region0->get(key));
    auto nonPdxType2 = std::static_pointer_cast<PdxTests::TestDiffTypePdxSV2>(
        pdxWrapper2->getObject());

    // Equal check
    bool isEqual = nonPdxType1->equals(*nonPdxType2);
    LOG_DEBUG("putFromVersion2_PS isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Task putFromVersion2_PS:Objects of type TestPdxSerializerForV2 "
           "should be equal");

    auto key2 = CacheableKey::create(2);
    region0->put(key2, pdxWrapper);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, getputFromVersion1_PS)
  {
    auto region0 = getHelper()->getRegion("DistRegionAck");
    auto key = CacheableKey::create(1);

    // GET
    auto pdxWrapper2 = std::dynamic_pointer_cast<PdxWrapper>(region0->get(key));
    auto nonPdxType2 = std::static_pointer_cast<PdxTests::TestDiffTypePdxSV1>(
        pdxWrapper2->getObject());

    // Create New object and Compare
    auto nonPdxType1 = std::make_shared<PdxTests::TestDiffTypePdxSV1>(true);
    bool isEqual = nonPdxType1->equals(*nonPdxType2);
    LOG_DEBUG("getputFromVersion1_PS-1 isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Task getputFromVersion1_PS:Objects of type TestPdxSerializerForV1 "
           "should be equal");

    // PUT
    region0->put(key, pdxWrapper2);

    auto key2 = CacheableKey::create(2);
    pdxWrapper2 = std::dynamic_pointer_cast<PdxWrapper>(region0->get(key2));
    auto pRet = std::static_pointer_cast<PdxTests::TestDiffTypePdxSV1>(
        pdxWrapper2->getObject());
    isEqual = nonPdxType1->equals(*pRet);
    LOG_DEBUG("getputFromVersion1_PS-2 isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Task getputFromVersion1_PS:Objects of type TestPdxSerializerForV1 "
           "should be equal");

    // Get then Put.. this should Not merge data back
    auto pdxWrapper = std::make_shared<PdxWrapper>(nonPdxType1, V1CLASSNAME2);
    region0->put(key2, pdxWrapper);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, getAtVersion2_PS)
  {
    auto region0 = getHelper()->getRegion("DistRegionAck");
    auto key = CacheableKey::create(1);

    // New object
    auto testDiffTpePdxSV2 = std::unique_ptr<PdxTests::TestDiffTypePdxSV2>(new PdxTests::TestDiffTypePdxSV2(true));

    // GET
    auto pdxWrapper2 = std::dynamic_pointer_cast<PdxWrapper>(region0->get(key));
    auto pRet = std::static_pointer_cast<PdxTests::TestDiffTypePdxSV2>(
        pdxWrapper2->getObject());

    bool isEqual = testDiffTpePdxSV2->equals(*pRet);
    LOG_DEBUG("getAtVersion2_PS-1 isEqual = %d", isEqual);
    ASSERT(
        isEqual == true,
        "Task getAtVersion2_PS:Objects of type TestPdxSerializerForV2 should "
        "be equal");

    auto key2 = CacheableKey::create(2);
    testDiffTpePdxSV2.reset(new PdxTests::TestDiffTypePdxSV2(true));

    pdxWrapper2 = std::dynamic_pointer_cast<PdxWrapper>(region0->get(key2));
    pRet = std::static_pointer_cast<PdxTests::TestDiffTypePdxSV2>(
        pdxWrapper2->getObject());
    isEqual = testDiffTpePdxSV2->equals(*pRet);

    LOG_DEBUG("getAtVersion2_PS-2 isEqual = %d", isEqual);
    ASSERT(
        isEqual == false,
        "Task getAtVersion2_PS:Objects of type TestPdxSerializerForV2 should "
        "be equal");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  {
    LOG("cleanProc 1...");
    isPoolConfig = false;
    cleanProc();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CloseCache2)
  {
    LOG("cleanProc 2...");
    isPoolConfig = false;
    cleanProc();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer)
  {
    LOG("closing Server1...");
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
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

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTPDXSERIALIZER_H_
