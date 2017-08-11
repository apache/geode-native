#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTPDXSERIALIZER_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTPDXSERIALIZER_H_

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
/*
 * testThinClientPdxSerializer.cpp
 *
 *  Created on: Apr 19, 2012
 *      Author: vrao
 */

#include "fw_dunit.hpp"
#include <geode/GeodeCppCache.hpp>
#include <ace/OS.h>
#include <ace/High_Res_Timer.h>
#include <string>

#include "ThinClientHelper.hpp"
#include <Utils.hpp>
#include "testobject/PdxClassV1.hpp"
#include "testobject/PdxClassV2.hpp"
#include "testobject/NonPdxType.hpp"
#include "ThinClientPdxSerializers.hpp"
#include "CacheRegionHelper.hpp"
using namespace apache::geode::client;
using namespace test;
using namespace PdxTests;

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define LOCATOR s2p2
#define SERVER1 s2p1

bool isLocator = false;
bool isLocalServer = false;

const char* poolNames[] = {"Pool1", "Pool2", "Pool3"};
const char* locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
bool isPoolConfig = false;  // To track if pool case is running
static bool m_useWeakHashMap ATTR_UNUSED = false;

void initClient(const bool isthinClient, bool isPdxIgnoreUnreadFields) {
  LOGINFO("initClient: isPdxIgnoreUnreadFields = %d ", isPdxIgnoreUnreadFields);
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
  createPool(poolNames[0], locHostPort, nullptr, 0, true);
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

void checkPdxInstanceToStringAtServer(RegionPtr regionPtr) {
  auto keyport = CacheableKey::create("success");
  auto boolPtr =
      std::dynamic_pointer_cast<CacheableBoolean>(regionPtr->get(keyport));
  bool val = boolPtr->value();
  ASSERT(val == true, "checkPdxInstanceToStringAtServer: Val should be true");
}

DUNIT_TASK_DEFINITION(CLIENT1, JavaPutGet)
  {
    SerializationRegistryPtr serializationRegistry = CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())->getSerializationRegistry();
    serializationRegistry->setPdxSerializer(std::make_shared<TestPdxSerializer>());

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto keyport = CacheableKey::create(1);

    auto npt1 = new PdxTests::NonPdxType(
        CacheRegionHelper::getCacheImpl(getHelper()->getCache().get())
            ->getSerializationRegistry()
            ->getPdxSerializer());
    auto pdxobj = std::make_shared<PdxWrapper>(
        npt1, CLASSNAME1,
        CacheRegionHelper::getCacheImpl(getHelper()->getCache().get())
            ->getSerializationRegistry()
            ->getPdxSerializer());
    regPtr0->put(keyport, pdxobj);

    auto obj2 = std::dynamic_pointer_cast<PdxWrapper>(regPtr0->get(keyport));

    auto boolPtr =
        std::dynamic_pointer_cast<CacheableBoolean>(regPtr0->get("success"));
    bool isEqual = boolPtr.get()->value();
    ASSERT(isEqual == true,
           "Task JavaPutGet:Objects of type NonPdxType should be equal");

    auto npt2 = reinterpret_cast<PdxTests::NonPdxType*>(obj2->getObject());
    ASSERT(npt1->equals(*npt2, false), "NonPdxType compare");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, JavaGet)
  {
    SerializationRegistryPtr serializationRegistry = CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())->getSerializationRegistry();
    serializationRegistry->setPdxSerializer(std::make_shared<TestPdxSerializer>());

    LOGDEBUG("JavaGet-1 Line_309");
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport1 = CacheableKey::create(1);
    LOGDEBUG("JavaGet-2 Line_314");
    auto obj1 = std::dynamic_pointer_cast<PdxWrapper>(regPtr0->get(keyport1));
    auto npt1 ATTR_UNUSED =
        reinterpret_cast<PdxTests::NonPdxType*>(obj1->getObject());
    LOGDEBUG("JavaGet-3 Line_316");
    auto keyport2 = CacheableKey::create("putFromjava");
    LOGDEBUG("JavaGet-4 Line_316");
    auto obj2 = std::dynamic_pointer_cast<PdxWrapper>(regPtr0->get(keyport2));
    auto npt2 ATTR_UNUSED =
        reinterpret_cast<PdxTests::NonPdxType*>(obj2->getObject());
    LOGDEBUG("JavaGet-5 Line_320");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, putFromVersion1_PS)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto key = CacheableKey::create(1);

    SerializationRegistryPtr serializationRegistry = CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())->getSerializationRegistry();
    serializationRegistry->setPdxSerializer(std::make_shared<TestPdxSerializerForV1>());

    // Create New object and wrap it in PdxWrapper (owner)
    auto npt1 = new PdxTests::TestDiffTypePdxSV1(true);
    auto pdxobj = std::make_shared<PdxWrapper>(
        npt1, V1CLASSNAME2,
        CacheRegionHelper::getCacheImpl(getHelper()->getCache().get())
            ->getSerializationRegistry()
            ->getPdxSerializer());

    // PUT
    regPtr0->put(key, pdxobj);

    // GET
    auto obj2 = std::dynamic_pointer_cast<PdxWrapper>(regPtr0->get(key));
    auto npt2 =
        reinterpret_cast<PdxTests::TestDiffTypePdxSV1*>(obj2->getObject());

    // Equal check
    bool isEqual = npt1->equals(npt2);
    LOGDEBUG("putFromVersion1_PS isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Task putFromVersion1_PS:Objects of type TestPdxSerializerForV1 "
           "should be equal");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, putFromVersion2_PS)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto key = CacheableKey::create(1);


    SerializationRegistryPtr serializationRegistry = CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())->getSerializationRegistry();
    serializationRegistry->setPdxSerializer(PdxSerializerPtr(new TestPdxSerializerForV2));

    // Create New object and wrap it in PdxWrapper (owner)
    auto npt1 = new PdxTests::TestDiffTypePdxSV2(true);
    auto pdxobj = std::make_shared<PdxWrapper>(
        npt1, V2CLASSNAME4,
        CacheRegionHelper::getCacheImpl(getHelper()->getCache().get())
            ->getSerializationRegistry()
            ->getPdxSerializer());

    // PUT
    regPtr0->put(key, pdxobj);

    // GET
    auto obj2 = std::dynamic_pointer_cast<PdxWrapper>(regPtr0->get(key));
    auto npt2 =
        reinterpret_cast<PdxTests::TestDiffTypePdxSV2*>(obj2->getObject());

    // Equal check
    bool isEqual = npt1->equals(npt2);
    LOGDEBUG("putFromVersion2_PS isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Task putFromVersion2_PS:Objects of type TestPdxSerializerForV2 "
           "should be equal");

    auto key2 = CacheableKey::create(2);
    regPtr0->put(key2, pdxobj);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, getputFromVersion1_PS)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto key = CacheableKey::create(1);

    // GET
    auto obj2 = std::dynamic_pointer_cast<PdxWrapper>(regPtr0->get(key));
    auto npt2 =
        reinterpret_cast<PdxTests::TestDiffTypePdxSV1*>(obj2->getObject());

    // Create New object and Compare
    auto npt1 = new PdxTests::TestDiffTypePdxSV1(true);
    bool isEqual = npt1->equals(npt2);
    LOGDEBUG("getputFromVersion1_PS-1 isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Task getputFromVersion1_PS:Objects of type TestPdxSerializerForV1 "
           "should be equal");

    // PUT
    regPtr0->put(key, obj2);

    auto key2 = CacheableKey::create(2);
    obj2 = std::dynamic_pointer_cast<PdxWrapper>(regPtr0->get(key2));
    auto pRet =
        reinterpret_cast<PdxTests::TestDiffTypePdxSV1*>(obj2->getObject());
    isEqual = npt1->equals(pRet);
    LOGDEBUG("getputFromVersion1_PS-2 isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Task getputFromVersion1_PS:Objects of type TestPdxSerializerForV1 "
           "should be equal");

    // Get then Put.. this should Not merge data back
    auto pdxobj = std::make_shared<PdxWrapper>(
        npt1, V1CLASSNAME2,
        CacheRegionHelper::getCacheImpl(getHelper()->getCache().get())
            ->getSerializationRegistry()
            ->getPdxSerializer());
    regPtr0->put(key2, pdxobj);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, getAtVersion2_PS)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto key = CacheableKey::create(1);

    // New object
    auto np = new PdxTests::TestDiffTypePdxSV2(true);

    // GET
    auto obj2 = std::dynamic_pointer_cast<PdxWrapper>(regPtr0->get(key));
    auto pRet =
        reinterpret_cast<PdxTests::TestDiffTypePdxSV2*>(obj2->getObject());

    bool isEqual = np->equals(pRet);
    LOGDEBUG("getAtVersion2_PS-1 isEqual = %d", isEqual);
    ASSERT(
        isEqual == true,
        "Task getAtVersion2_PS:Objects of type TestPdxSerializerForV2 should "
        "be equal");

    auto key2 = CacheableKey::create(2);
    np = new PdxTests::TestDiffTypePdxSV2(true);

    obj2 = std::dynamic_pointer_cast<PdxWrapper>(regPtr0->get(key2));
    pRet = reinterpret_cast<PdxTests::TestDiffTypePdxSV2*>(obj2->getObject());
    isEqual = np->equals(pRet);

    LOGDEBUG("getAtVersion2_PS-2 isEqual = %d", isEqual);
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

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTPDXSERIALIZER_H_
