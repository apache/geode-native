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
 * testThinClientPdxTests.cpp
 *
 *  Created on: Sep 30, 2011
 *      Author: npatel
 */

//#include "ThinClientPdxTests.hpp"
/*DUNIT_MAIN
{
        //LOG("NIL:DUNIT_MAIN:PDXTests function called");
        runPdxTests(true, false);
}
END_MAIN*/
#include <string>

#include "fw_dunit.hpp"

#include <ace/OS.h>
#include <ace/High_Res_Timer.h>

#include <geode/PdxInstance.hpp>
#include <geode/UserFunctionExecutionException.hpp>
#include <geode/FunctionService.hpp>

#define ROOT_NAME "testThinClientPdxTests"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "ThinClientHelper.hpp"
#include "testobject/PdxClassV1.hpp"
#include "testobject/PdxClassV2.hpp"
#include "testobject/VariousPdxTypes.hpp"
#include "testobject/InvalidPdxUsage.hpp"
#include "QueryStrings.hpp"
#include "QueryHelper.hpp"
#include "Utils.hpp"
#include <geode/Query.hpp>
#include <geode/QueryService.hpp>
#include "CachePerfStats.hpp"
#include <LocalRegion.hpp>

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define CLIENT3 s2p2
#define LOCATOR s2p2
#define SERVER1 s2p1

using PdxTests::Address;
using PdxTests::AddressWithInvalidAPIUsage;
using PdxTests::InvalidPdxUsage;
using PdxTests::MixedVersionNestedPdx;
using PdxTests::NestedPdx;
using PdxTests::PdxInsideIGeodeSerializable;
using PdxTests::PdxType1V1;
using PdxTests::PdxType2V1;
using PdxTests::PdxType3V1;
using PdxTests::PdxTypes1;
using PdxTests::PdxTypes10;
using PdxTests::PdxTypes1V2;
using PdxTests::PdxTypes2;
using PdxTests::PdxTypes2V2;
using PdxTests::PdxTypes3;
using PdxTests::PdxTypes3V2;
using PdxTests::PdxTypes4;
using PdxTests::PdxTypes5;
using PdxTests::PdxTypes6;
using PdxTests::PdxTypes7;
using PdxTests::PdxTypes8;
using PdxTests::PdxTypes9;
using PdxTests::PdxTypesIgnoreUnreadFieldsV1;
using PdxTests::PdxTypesIgnoreUnreadFieldsV2;
using PdxTests::PdxTypesR1V2;
using PdxTests::PdxTypesR2V2;
using PdxTests::PdxTypesV1R1;
using PdxTests::PdxTypesV1R2;

using apache::geode::client::CacheableBoolean;
using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableInt64;
using apache::geode::client::CacheableLinkedList;
using apache::geode::client::CacheableObjectArray;
using apache::geode::client::CacheableVector;
using apache::geode::client::ClassCastException;
using apache::geode::client::FunctionService;
using apache::geode::client::IllegalStateException;
using apache::geode::client::LocalRegion;
using apache::geode::client::PdxInstance;
using apache::geode::client::UserFunctionExecutionException;

bool isLocator = false;
bool isLocalServer = false;

const char *poolNames[] = {"Pool1", "Pool2", "Pool3"};
const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
bool isPoolConfig = false;  // To track if pool case is running
// const char * qRegionNames[] = { "Portfolios", "Positions", "Portfolios2",
// "Portfolios3" };
static bool m_useWeakHashMap = false;

template <typename T1, typename T2>
bool genericValCompare(T1 value1, T2 value2) /*const*/
{
  if (value1 != value2) return false;
  return true;
}

void initClient(const bool isthinClient, bool isPdxIgnoreUnreadFields,
                const std::shared_ptr<Properties> &configPtr = nullptr) {
  LOG_INFO("isPdxIgnoreUnreadFields = %d ", isPdxIgnoreUnreadFields);
  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(isthinClient, isPdxIgnoreUnreadFields, false,
                                  configPtr, false);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

//////////

void initClientN(const bool isthinClient, bool isPdxIgnoreUnreadFields,
                 bool isPdxReadSerialized = false,
                 const std::shared_ptr<Properties> &configPtr = nullptr) {
  LOG_INFO("isPdxIgnoreUnreadFields = %d ", isPdxIgnoreUnreadFields);
  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(isthinClient, isPdxIgnoreUnreadFields,
                                  isPdxReadSerialized, configPtr, false);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

void stepOneN(bool isPdxIgnoreUnreadFields = false,
              bool isPdxReadSerialized = false,
              std::shared_ptr<Properties> config = nullptr) {
  try {
    // serializationRegistry->addType(Position::createDeserializable);
    // serializationRegistry->addType(Portfolio::createDeserializable);
  } catch (const IllegalStateException &) {
    // ignore exception
  }
  // Create just one pool and attach all regions to that.
  initClientN(true, isPdxIgnoreUnreadFields, isPdxReadSerialized, config);

  isPoolConfig = true;
  createPool(poolNames[0], locHostPort, nullptr, 0, true);
  createRegionAndAttachPool("DistRegionAck", USE_ACK, poolNames[0],
                            false /*Caching disabled*/);

  LOG("StepOne complete.");
}

DUNIT_TASK_DEFINITION(CLIENT1, StepOnePoolLoc1)
  {
    LOG("Starting Step One with Pool + Locator lists");
    stepOneN(false, true, nullptr);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwoPoolLoc1)
  {
    LOG("Starting Step Two with Pool + Locator");
    stepOneN(false, true, nullptr);
  }
END_TASK_DEFINITION
///////////////

void initClient1WithClientName(
    const bool isthinClient,
    const std::shared_ptr<Properties> &configPtr = nullptr) {
  if (cacheHelper == nullptr) {
    auto config = configPtr;
    if (config == nullptr) {
      config = Properties::create();
    }
    config->insert("name", "Client-1");
    cacheHelper = new CacheHelper(isthinClient, config);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

void initClient2WithClientName(
    const bool isthinClient,
    const std::shared_ptr<Properties> &configPtr = nullptr) {
  if (cacheHelper == nullptr) {
    auto config = configPtr;
    if (config == nullptr) {
      config = Properties::create();
    }
    config->insert("name", "Client-2");
    cacheHelper = new CacheHelper(isthinClient, config);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

void stepOneForClient1() {
  // Create just one pool and attach all regions to that.
  initClient1WithClientName(true);

  isPoolConfig = true;
  createPool(poolNames[0], locHostPort, nullptr, 0, true);
  createRegionAndAttachPool("DistRegionAck", USE_ACK, poolNames[0],
                            false /*Caching disabled*/);
  LOG("StepOne complete.");
}

void stepOneForClient2() {
  // Create just one pool and attach all regions to that.
  initClient2WithClientName(true);

  isPoolConfig = true;
  createPool(poolNames[0], locHostPort, nullptr, 0, true);
  createRegionAndAttachPool("DistRegionAck", USE_ACK, poolNames[0],
                            false /*Caching disabled*/);
  LOG("StepOne complete.");
}
void stepOne(bool isPdxIgnoreUnreadFields = false,
             std::shared_ptr<Properties> config = nullptr) {
  try {
    // serializationRegistry->addType(Position::createDeserializable);
    // serializationRegistry->addType(Portfolio::createDeserializable);
  } catch (const IllegalStateException &) {
    // ignore exception
  }
  // Create just one pool and attach all regions to that.
  initClient(true, isPdxIgnoreUnreadFields, config);
  isPoolConfig = true;
  createPool(poolNames[0], locHostPort, nullptr, 0, true);
  createRegionAndAttachPool("DistRegionAck", USE_ACK, poolNames[0],
                            false /*Caching disabled*/);
  LOG("StepOne complete.");
}

void initClient1(bool isPdxIgnoreUnreadFields = false) {
  // Create just one pool and attach all regions to that.
  initClient(true, isPdxIgnoreUnreadFields);
  isPoolConfig = true;
  createPool(poolNames[0], locHostPort, nullptr, 0, false);
  createRegionAndAttachPool("DistRegionAck", USE_ACK, poolNames[0],
                            true /*Caching enabled*/);
  LOG("StepOne complete.");
}

void initClient2(bool isPdxIgnoreUnreadFields = false) {
  // Create just one pool and attach all regions to that.
  initClient(true, isPdxIgnoreUnreadFields);
  isPoolConfig = true;
  createPool(poolNames[0], locHostPort, nullptr, 0,
             true /*ClientNotification enabled*/);
  createRegionAndAttachPool("DistRegionAck", USE_ACK, poolNames[0],
                            true /*Caching enabled*/);
  LOG("StepOne complete.");
}

void initClient3(bool isPdxIgnoreUnreadFields = false) {
  // Create just one pool and attach all regions to that.
  initClient(true, isPdxIgnoreUnreadFields);
  isPoolConfig = true;
  createPool(poolNames[0], locHostPort, nullptr, 0,
             true /*ClientNotification enabled*/);
  createRegionAndAttachPool("DistRegionAck", USE_ACK, poolNames[0],
                            true /*Caching enabled*/);
  LOG("StepOne complete.");
}

DUNIT_TASK_DEFINITION(SERVER1, StartLocator)
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

DUNIT_TASK_DEFINITION(CLIENT1, StepOnePoolLocSysConfig)
  {
    LOG("Starting Step One with Pool + Locator lists");
    auto config = Properties::create();
    config->insert("on-client-disconnect-clear-pdxType-Ids", "true");
    stepOne(false, config);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwoPoolLocSysConfig)
  {
    LOG("Starting Step One with Pool + Locator lists");
    auto config = Properties::create();
    config->insert("on-client-disconnect-clear-pdxType-Ids", "true");
    stepOne(false, config);
  }
END_TASK_DEFINITION

// StepOnePoolLoc_PdxMetadataTest
DUNIT_TASK_DEFINITION(CLIENT1, StepOnePoolLoc_PdxMetadataTest)
  {
    LOG("Starting Step One with Pool + Locator lists");
    initClient1();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServer)
  {
    LOG("Starting SERVER1...");
    if (isLocalServer) CacheHelper::initServer(1, "cacheserverPdx.xml");
    LOG("SERVER1 started");
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

DUNIT_TASK_DEFINITION(SERVER1, CreateServer_PdxMetadataTest)
  {
    LOG("Starting SERVER1...");
    if (isLocalServer) CacheHelper::initServer(1, "cacheserverPdx2.xml");
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServerWithLocator_PdxMetadataTest)
  {
    LOG("Starting SERVER1...");
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserverPdx2.xml", locHostPort);
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

DUNIT_TASK_DEFINITION(CLIENT1, StepOnePoolLocBug866)
  {
    LOG("Starting Step One with Pool + Locator lists");
    stepOneForClient1();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwoPoolLocBug866)
  {
    LOG("Starting Step Two with Pool + Locator");
    stepOneForClient2();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwoPoolLoc)
  {
    LOG("Starting Step Two with Pool + Locator");
    stepOne();
  }
END_TASK_DEFINITION

// StepTwoPoolLoc_PdxMetadataTest
DUNIT_TASK_DEFINITION(CLIENT2, StepTwoPoolLoc_PdxMetadataTest)
  {
    LOG("Starting Step Two with Pool + Locator");
    initClient2();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT3, StepThreePoolLoc_PdxMetadataTest)
  {
    LOG("Starting Step Two with Pool + Locator");
    initClient3();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwoPoolLoc_PDX)
  {
    LOG("Starting Step Two with Pool + Locator");
    stepOne(true);
  }
END_TASK_DEFINITION

void checkPdxInstanceToStringAtServer(std::shared_ptr<Region> regionPtr) {
  auto keyport = CacheableKey::create("success");
  auto boolPtr =
      std::dynamic_pointer_cast<CacheableBoolean>(regionPtr->get(keyport));
  bool val = boolPtr->value();
  // TODO::Enable asser and disable LOG_INFO
  ASSERT(val == true, "checkPdxInstanceToStringAtServer: Val should be true");
  LOG_INFO("NIL::checkPdxInstanceToStringAtServer:139: val = %d", val);
}

// testPdxWriterAPIsWithInvalidArgs
DUNIT_TASK_DEFINITION(CLIENT1, testPdxWriterAPIsWithInvalidArgs)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    try {
      serializationRegistry->addPdxSerializableType(
          InvalidPdxUsage::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          AddressWithInvalidAPIUsage::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    int expectedExceptionCount = 0;

    // Put operation
    auto keyport = CacheableKey::create(1);
    auto pdxobj = std::make_shared<PdxTests::InvalidPdxUsage>();
    regPtr0->put(keyport, pdxobj);

    // Check the exception count:: expected is 41.
    expectedExceptionCount =
        (std::dynamic_pointer_cast<PdxTests::InvalidPdxUsage>(pdxobj))
            ->gettoDataExceptionCount();
    // LOG_INFO("TASK::testPdxWriterAPIsWithInvalidArgs:: toData ExceptionCount
    // ::
    // %d ", expectedExceptionCount);
    ASSERT(expectedExceptionCount == 41,
           "Task testPdxWriterAPIsWithInvalidArgs:Did not get expected "
           "toDataExceptionCount");

    // Get Operation and check fromDataExceptionCount, Expected is 41.
    auto obj2 = std::dynamic_pointer_cast<PdxTests::InvalidPdxUsage>(
        regPtr0->get(keyport));
    // LOG_INFO("TASK::testPdxWriterAPIsWithInvalidArgs:: fromData
    // ExceptionCOunt
    // :: %d ", obj2->getfromDataExceptionCount());
    expectedExceptionCount = obj2->getfromDataExceptionCount();
    ASSERT(expectedExceptionCount == 41,
           "Task testPdxWriterAPIsWithInvalidArgs:Did not get expected "
           "fromDataExceptionCount");

    LOG_INFO("TASK::testPdxWriterAPIsWithInvalidArgs completed Successfully");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, testPdxReaderAPIsWithInvalidArgs)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    try {
      serializationRegistry->addPdxSerializableType(
          InvalidPdxUsage::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          AddressWithInvalidAPIUsage::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    int expectedExceptionCount = 0;
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    // Get Operation. Check fromDataExceptionCount. Expected is 41.
    auto keyport1 = CacheableKey::create(1);
    auto obj1 = std::dynamic_pointer_cast<PdxTests::InvalidPdxUsage>(
        regPtr0->get(keyport1));

    // Check the exception count:: expected is 41.
    // LOG_INFO("TASK::testPdxReaderAPIsWithInvalidArgs:: fromDataExceptionCount
    // ::
    // %d ", obj1->getfromDataExceptionCount());
    expectedExceptionCount = obj1->getfromDataExceptionCount();
    ASSERT(expectedExceptionCount == 41,
           "Task testPdxReaderAPIsWithInvalidArgs:Did not get expected "
           "fromDataExceptionCount");

    LOG_INFO("TASK::testPdxReaderAPIsWithInvalidArgs completed Successfully");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, testPutWithMultilevelInheritance)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::Child::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    // Put operation
    auto keyport = CacheableKey::create(1);
    auto pdxobj = std::make_shared<PdxTests::Child>();
    regPtr0->put(keyport, pdxobj);
    LOG_INFO("TASK::testPutWithMultilevelInheritance:: Put successful");

    // Get Operation and check fromDataExceptionCount, Expected is 41.
    auto obj2 =
        std::dynamic_pointer_cast<PdxTests::Child>(regPtr0->get(keyport));
    // LOG_INFO("Task: testPutWithMultilevelInheritance: got members :: %d %d %d
    // %d
    // %d %d ", obj2->getMember_a(), obj2->getMember_b(), obj2->getMember_c(),
    // obj2->getMember_d(), obj2->getMember_e(), obj2->getMember_f());
    bool isEqual =
        (std::dynamic_pointer_cast<PdxTests::Child>(pdxobj))->equals(obj2);
    LOG_INFO("testPutWithMultilevelInheritance:.. isEqual = %d", isEqual);
    ASSERT(isEqual == true, "Objects of type class Child should be equal");

    LOG_INFO("TASK::testPutWithMultilevelInheritance:: Get successful");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, testGetWithMultilevelInheritance)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::Child::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport1 = CacheableKey::create(1);
    auto obj1 =
        std::dynamic_pointer_cast<PdxTests::Child>(regPtr0->get(keyport1));

    auto pdxobj = std::make_shared<PdxTests::Child>();
    bool isEqual =
        (std::dynamic_pointer_cast<PdxTests::Child>(pdxobj))->equals(obj1);
    LOG_INFO("testPutWithMultilevelInheritance:.. isEqual = %d", isEqual);
    ASSERT(isEqual == true, "Objects of type class Child should be equal");
    // LOG_INFO("Task: testGetWithMultilevelInheritance: got members :: %d %d %d
    // %d
    // %d %d ", obj1->getMember_a(), obj1->getMember_b(), obj1->getMember_c(),
    // obj1->getMember_d(), obj1->getMember_e(), obj1->getMember_f());
    LOG_INFO(
        "TASK::testGetWithMultilevelInheritance GET completed Successfully");
  }
END_TASK_DEFINITION

// Added for the LinkedList testcase

DUNIT_TASK_DEFINITION(CLIENT1, JavaPutGet1)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport = CacheableKey::create(1);
    auto valPtr = CacheableInt32::create(123);
    regPtr0->put(keyport, valPtr);

    auto getVal =
        std::dynamic_pointer_cast<CacheableInt32>(regPtr0->get(keyport));

    auto boolPtr =
        std::dynamic_pointer_cast<CacheableBoolean>(regPtr0->get("success"));

    bool isEqual = boolPtr->value();
    ASSERT(isEqual == true,
           "Task JavaPutGet:Objects of type PdxType should be equal");

    LOG_INFO("Task:JavaPutGet PDX-ON read-serialized = %d",
             cacheHelper->getCache()->getPdxReadSerialized());
    auto jsonDoc =
        std::dynamic_pointer_cast<PdxInstance>(regPtr0->get("jsondoc1"));
    auto toString = jsonDoc->toString();
    LOG_INFO("Task:JavaPutGet: Result = %s ", toString.c_str());
    /*
    int16_t age = 0;
    jsonDoc->getField("age", age);

    char* stringVal = nullptr;
    jsonDoc->getField("firstName", &stringVal);

    char* stringVal1 = nullptr;
    jsonDoc->getField("lastName", &stringVal1);
    */

    auto object2 = jsonDoc->getCacheableField("kids");
    auto listPtr = std::dynamic_pointer_cast<CacheableLinkedList>(object2);
    LOG_INFO("Task:JavaPutGet: list size = %d", listPtr->size());

    auto m_linkedlist = CacheableLinkedList::create();
    m_linkedlist->push_back(CacheableString::create("Manan"));
    m_linkedlist->push_back(CacheableString::create("Nishka"));

    ASSERT(genericValCompare(m_linkedlist->size(), listPtr->size()) == true,
           "LinkedList size should be equal");
    for (size_t j = 0; j < m_linkedlist->size(); j++) {
      genericValCompare(m_linkedlist->at(j), listPtr->at(j));
    }

    LOG_INFO("Task:JavaPutGet Tese-cases completed successfully!");
  }
END_TASK_DEFINITION

// END

DUNIT_TASK_DEFINITION(CLIENT1, JavaPutGet)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxType::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          Address::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport = CacheableKey::create(1);
    auto pdxobj = std::make_shared<PdxTests::PdxType>();
    regPtr0->put(keyport, pdxobj);

    auto obj2 =
        std::dynamic_pointer_cast<PdxTests::PdxType>(regPtr0->get(keyport));

    auto boolPtr =
        std::dynamic_pointer_cast<CacheableBoolean>(regPtr0->get("success"));
    bool isEqual = boolPtr->value();
    LOG_DEBUG("Task:JavaPutGet: isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Task JavaPutGet:Objects of type PdxType should be equal");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, JavaGet)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxType::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          Address::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    LOG_DEBUG("JavaGet-1 Line_309");
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport1 = CacheableKey::create(1);
    auto pdxobj = std::make_shared<PdxTests::PdxType>();
    LOG_DEBUG("JavaGet-2 Line_314");
    auto obj1 =
        std::dynamic_pointer_cast<PdxTests::PdxType>(regPtr0->get(keyport1));
    LOG_DEBUG("JavaGet-3 Line_316");
    auto keyport2 = CacheableKey::create("putFromjava");
    LOG_DEBUG("JavaGet-4 Line_316");
    auto obj2 =
        std::dynamic_pointer_cast<PdxTests::PdxType>(regPtr0->get(keyport2));
    LOG_DEBUG("JavaGet-5 Line_320");
  }
END_TASK_DEFINITION
/***************************************************************/
DUNIT_TASK_DEFINITION(CLIENT2, putAtVersionTwoR21)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypesR2V2::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport = CacheableKey::create(1);
    auto np = std::make_shared<PdxTypesR2V2>();

    regPtr0->put(keyport, np);

    auto pRet = std::dynamic_pointer_cast<PdxTypesR2V2>(regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("putAtVersionTwoR21:.. isEqual = %d", isEqual);
    ASSERT(
        isEqual == true,
        "Objects of type PdxTypesR2V2 should be equal at putAtVersionTwoR21");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, getPutAtVersionOneR22)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypesV1R2::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport = CacheableKey::create(1);
    auto np = std::make_shared<PdxTypesV1R2>();

    auto pRet = std::dynamic_pointer_cast<PdxTypesV1R2>(regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("getPutAtVersionOneR22:.. isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Objects of type PdxTypesV1R2 should be equal at "
           "getPutAtVersionOneR22");

    regPtr0->put(keyport, pRet);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, getPutAtVersionTwoR23)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport = CacheableKey::create(1);

    auto np = std::make_shared<PdxTypesR2V2>();

    auto pRet = std::dynamic_pointer_cast<PdxTypesR2V2>(regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("getPutAtVersionTwoR23:.. isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Objects of type PdxTypesR2V2 should be equal at "
           "getPutAtVersionTwoR23");

    regPtr0->put(keyport, pRet);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, getPutAtVersionOneR24)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport = CacheableKey::create(1);
    auto np = std::make_shared<PdxTypesV1R2>();

    auto pRet = std::dynamic_pointer_cast<PdxTypesV1R2>(regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("getPutAtVersionOneR24:.. isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Objects of type PdxTypesV1R2 should be equal at "
           "getPutAtVersionOneR24");

    regPtr0->put(keyport, pRet);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, putAtVersionOne31)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxType3V1::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport = CacheableKey::create(1);
    auto np = std::make_shared<PdxType3V1>();

    regPtr0->put(keyport, np);

    auto pRet = std::dynamic_pointer_cast<PdxType3V1>(regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("Task:putAtVersionOne31: isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Objects of type PdxType3V1 should be equal at putAtVersionOne31");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, getPutAtVersionTwo32)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes3V2::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto np = std::make_shared<PdxTypes3V2>();

    auto keyport = CacheableKey::create(1);

    auto pRet = std::dynamic_pointer_cast<PdxTypes3V2>(regPtr0->get(keyport));
    bool isEqual = np->equals(pRet);
    LOG_DEBUG("Task:getPutAtVersionTwo32.. isEqual = %d", isEqual);
    ASSERT(
        isEqual == true,
        "Objects of type PdxTypes3V2 should be equal at getPutAtVersionTwo32");

    regPtr0->put(keyport, pRet);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, getPutAtVersionOne33)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport = CacheableKey::create(1);
    auto np = std::make_shared<PdxType3V1>();

    auto pRet = std::dynamic_pointer_cast<PdxType3V1>(regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("getPutAtVersionOne33:.. isEqual = %d", isEqual);
    ASSERT(
        isEqual == true,
        "Objects of type PdxType3V1 should be equal at getPutAtVersionOne33");

    regPtr0->put(keyport, pRet);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, getPutAtVersionTwo34)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto np = std::make_shared<PdxTypes3V2>();

    auto keyport = CacheableKey::create(1);

    auto pRet = std::dynamic_pointer_cast<PdxTypes3V2>(regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("Task:getPutAtVersionTwo34: isEqual = %d", isEqual);
    ASSERT(
        isEqual == true,
        "Objects of type PdxType3V1 should be equal at getPutAtVersionTwo34");

    regPtr0->put(keyport, pRet);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, putAtVersionOne21)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxType2V1::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport = CacheableKey::create(1);
    auto np = std::make_shared<PdxType2V1>();

    regPtr0->put(keyport, np);

    auto pRet = std::dynamic_pointer_cast<PdxType2V1>(regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("Task:putAtVersionOne21:.. isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Objects of type PdxType2V1 should be equal at putAtVersionOne21");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, getPutAtVersionTwo22)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes2V2::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto np = std::make_shared<PdxTypes2V2>();

    auto keyport = CacheableKey::create(1);

    auto pRet = std::dynamic_pointer_cast<PdxTypes2V2>(regPtr0->get(keyport));
    bool isEqual = np->equals(pRet);
    LOG_DEBUG("Task:getPutAtVersionTwo22.. isEqual = %d", isEqual);
    ASSERT(
        isEqual == true,
        "Objects of type PdxTypes2V2 should be equal at getPutAtVersionTwo22");

    regPtr0->put(keyport, pRet);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, getPutAtVersionOne23)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto np = std::make_shared<PdxType2V1>();

    auto keyport = CacheableKey::create(1);

    auto pRet = std::dynamic_pointer_cast<PdxType2V1>(regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("Task:getPutAtVersionOne23: isEqual = %d", isEqual);
    ASSERT(
        isEqual == true,
        "Objects of type PdxType2V1 should be equal at getPutAtVersionOne23");

    regPtr0->put(keyport, pRet);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, getPutAtVersionTwo24)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto np = std::make_shared<PdxTypes2V2>();
    auto keyport = CacheableKey::create(1);

    auto pRet = std::dynamic_pointer_cast<PdxTypes2V2>(regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("Task:getPutAtVersionTwo24.. isEqual = %d", isEqual);
    ASSERT(
        isEqual == true,
        "Objects of type PdxTypes2V2 should be equal at getPutAtVersionTwo24");

    regPtr0->put(keyport, pRet);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, putAtVersionOne11)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxType1V1::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport = CacheableKey::create(1);
    auto np = std::make_shared<PdxTests::PdxType1V1>();

    regPtr0->put(keyport, np);

    auto pRet =
        std::dynamic_pointer_cast<PdxTests::PdxType1V1>(regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("NIL:putAtVersionOne11:.. isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Objects of type PdxType1V1 should be equal at putAtVersionOne11 "
           "Line_170");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, putAtVersionTwo1)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypesR1V2::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    PdxTests::PdxTypesR1V2::reset(false);

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport = CacheableKey::create(1);
    auto np = std::make_shared<PdxTypesR1V2>();

    regPtr0->put(keyport, np);

    auto pRet = std::dynamic_pointer_cast<PdxTypesR1V2>(regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("NIL:putAtVersionTwo1:.. isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Objects of type PdxTypesR1V2 should be equal at putAtVersionTwo1");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, getPutAtVersionOne2)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypesV1R1::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto np = std::make_shared<PdxTypesV1R1>();

    auto keyport = CacheableKey::create(1);

    auto pRet = std::dynamic_pointer_cast<PdxTypesV1R1>(regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("NIL:getPutAtVersionOne2:.. isEqual = %d", isEqual);
    ASSERT(
        isEqual == true,
        "Objects of type PdxTypesV1R1 should be equal at getPutAtVersionOne2");

    regPtr0->put(keyport, pRet);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, getPutAtVersionTwo3)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto np = std::make_shared<PdxTypesR1V2>();

    auto keyport = CacheableKey::create(1);

    auto pRet = std::dynamic_pointer_cast<PdxTypesR1V2>(regPtr0->get(keyport));
    bool isEqual = np->equals(pRet);
    LOG_DEBUG("NIL:getPutAtVersionTwo3.. isEqual = %d", isEqual);
    ASSERT(
        isEqual == true,
        "Objects of type PdxTypesR1V2 should be equal at getPutAtVersionTwo3");

    regPtr0->put(keyport, pRet);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, getPutAtVersionOne4)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto np = std::make_shared<PdxTypesV1R1>();

    auto keyport = CacheableKey::create(1);
    auto pRet = std::dynamic_pointer_cast<PdxTypesV1R1>(regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("getPutAtVersionOne4: isEqual = %d", isEqual);
    ASSERT(
        isEqual == true,
        "Objects of type PdxTypesV1R1 should be equal at getPutAtVersionOne4");

    regPtr0->put(keyport, pRet);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, getPutAtVersionTwo5)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto np = std::make_shared<PdxTypesR1V2>();

    // GET
    auto keyport = CacheableKey::create(1);

    auto pRet = std::dynamic_pointer_cast<PdxTypesR1V2>(regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("Task:getPutAtVersionTwo5.. isEqual = %d", isEqual);
    ASSERT(
        isEqual == true,
        "Objects of type PdxTypesR1V2 should be equal at getPutAtVersionTwo5");

    regPtr0->put(keyport, pRet);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, getPutAtVersionOne6)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto np = std::make_shared<PdxTypesV1R1>();

    auto keyport = CacheableKey::create(1);
    auto pRet = std::dynamic_pointer_cast<PdxTypesV1R1>(regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("Task getPutAtVersionOne6:.. isEqual = %d", isEqual);
    ASSERT(
        isEqual == true,
        "Objects of type PdxTypesV1R1 should be equal at getPutAtVersionOne6");

    regPtr0->put(keyport, pRet);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, putV2PdxUI)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypesIgnoreUnreadFieldsV2::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    // PdxTests::PdxTypesIgnoreUnreadFieldsV2::reset(false);

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto np = std::make_shared<PdxTypesIgnoreUnreadFieldsV2>();
    auto keyport = CacheableKey::create(1);
    regPtr0->put(keyport, np);

    auto pRet = std::dynamic_pointer_cast<PdxTypesIgnoreUnreadFieldsV2>(
        regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("NIL:putV2PdxUI:.. isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Objects of type PdxTypesIgnoreUnreadFieldsV2 should be equal at "
           "putV2PdxUI ");

    regPtr0->put(keyport, pRet);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, putV1PdxUI)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypesIgnoreUnreadFieldsV1::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    // PdxTests::PdxTypesIgnoreUnreadFieldsV1::reset(false);
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto keyport = CacheableKey::create(1);

    auto pRet = std::dynamic_pointer_cast<PdxTypesIgnoreUnreadFieldsV1>(
        regPtr0->get(keyport));
    regPtr0->put(keyport, pRet);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, getV2PdxUI)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto np = std::make_shared<PdxTypesIgnoreUnreadFieldsV2>();

    auto keyport = CacheableKey::create(1);
    auto pRet = std::dynamic_pointer_cast<PdxTypesIgnoreUnreadFieldsV2>(
        regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("Task:getV2PdxUI:.. isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Objects of type PdxTypesIgnoreUnreadFieldsV2 should be equal at "
           "getV2PdxUI ");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, getPutAtVersionTwo12)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes1V2::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto np = std::make_shared<PdxTypes1V2>();

    auto keyport = CacheableKey::create(1);

    auto pRet = std::dynamic_pointer_cast<PdxTypes1V2>(regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("NIL:getPutAtVersionTwo12:.. isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Objects of type PdxType1V2 should be equal at getPutAtVersionTwo12 "
           "Line_197");

    regPtr0->put(keyport, pRet);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, getPutAtVersionOne13)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto np = std::make_shared<PdxType1V1>();

    auto keyport = CacheableKey::create(1);

    auto pRet = std::dynamic_pointer_cast<PdxType1V1>(regPtr0->get(keyport));
    bool isEqual = np->equals(pRet);

    LOG_DEBUG("NIL:getPutAtVersionOne13:221.. isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Objects of type PdxType1V2 should be equal at getPutAtVersionOne13 "
           "Line_215");

    LOG_DEBUG("NIL:getPutAtVersionOne13: PUT remote object -1");
    regPtr0->put(keyport, pRet);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, getPutAtVersionTwo14)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto np = std::make_shared<PdxTypes1V2>();

    auto keyport = CacheableKey::create(1);
    auto pRet = std::dynamic_pointer_cast<PdxTypes1V2>(regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("NIL:getPutAtVersionTwo14:241.. isEqual = %d", isEqual);
    ASSERT(
        isEqual == true,
        "Objects of type PdxTypes1V2 should be equal at getPutAtVersionTwo14 "
        "Line_242");

    regPtr0->put(keyport, pRet);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, getPutAtVersionOne15)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto np = std::make_shared<PdxType1V1>();

    // GET
    auto keyport = CacheableKey::create(1);

    auto pRet = std::dynamic_pointer_cast<PdxType1V1>(regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("NIL:getPutAtVersionOne15:784.. isEqual = %d", isEqual);
    ASSERT(isEqual == true,
           "Objects of type PdxType1V2 should be equal at getPutAtVersionOne15 "
           "Line_272");

    regPtr0->put(keyport, pRet);
    auto testNumberOfPreservedData =
        TestUtils::testNumberOfPreservedData(*CacheRegionHelper::getCacheImpl(
            CacheHelper::getHelper().getCache().get()));
    LOG_DEBUG(
        "NIL:getPutAtVersionOne15 m_useWeakHashMap = %d and "
        "TestUtils::testNumberOfPreservedData() = %d",
        m_useWeakHashMap, testNumberOfPreservedData);
    if (m_useWeakHashMap == false) {
      ASSERT(testNumberOfPreservedData == 0,
             "testNumberOfPreservedData should be zero at Line_288");
    } else {
      ASSERT(
          testNumberOfPreservedData > 0,
          "testNumberOfPreservedData should be Greater than zero at Line_292");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, getPutAtVersionTwo16)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto np = std::make_shared<PdxTypes1V2>();

    auto keyport = CacheableKey::create(1);
    auto pRet = std::dynamic_pointer_cast<PdxTypes1V2>(regPtr0->get(keyport));

    bool isEqual = np->equals(pRet);
    LOG_DEBUG("NIL:getPutAtVersionTwo14:.. isEqual = %d", isEqual);
    ASSERT(
        isEqual == true,
        "Objects of type PdxTypes1V2 should be equal at getPutAtVersionTwo14");

    regPtr0->put(keyport, pRet);
    auto testNumberOfPreservedData =
        TestUtils::testNumberOfPreservedData(*CacheRegionHelper::getCacheImpl(
            CacheHelper::getHelper().getCache().get()));
    if (m_useWeakHashMap == false) {
      ASSERT(testNumberOfPreservedData == 0,
             "getPutAtVersionTwo16:testNumberOfPreservedData should be zero");
    } else {
      // it has extra fields, so no need to preserve data
      ASSERT(testNumberOfPreservedData == 0,
             "getPutAtVersionTwo16:testNumberOfPreservedData should be zero");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, Puts2)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes1::createDeserializable);
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes2::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport = CacheableKey::create(1);

    auto pdxobj = std::make_shared<PdxTests::PdxTypes1>();

    regPtr0->put(keyport, pdxobj);

    auto keyport2 = CacheableKey::create(2);

    auto pdxobj2 = std::make_shared<PdxTests::PdxTypes2>();

    regPtr0->put(keyport2, pdxobj2);

    // ASSERT(lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializationBytes()
    // ==
    // lregPtr->getCacheImpl()->getCachePerfStats().getPdxDeSerializationBytes(),
    //"Total pdxDeserializationBytes should be equal to Total
    // pdxSerializationsBytes.");

    LOG("Stepone two puts complete.\n");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, forCleanup)
  {
    LOG_INFO("Do put to clean the pdxtype registry");
    try {
      auto regPtr0 = getHelper()->getRegion("DistRegionAck");

      auto keyport = CacheableKey::create(1);

      auto pdxobj = std::make_shared<PdxTests::PdxTypes1>();

      regPtr0->put(keyport, pdxobj);
    } catch (...) {
      // ignore
    }
    LOG_INFO("Wake up");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, Puts22)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport = CacheableKey::create(1);

    auto pdxobj = std::make_shared<PdxTests::PdxTypes1>();

    regPtr0->put(keyport, pdxobj);

    auto keyport2 = CacheableKey::create(2);

    auto pdxobj2 = std::make_shared<PdxTests::PdxTypes2>();

    regPtr0->put(keyport2, pdxobj2);

    // ASSERT(lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializationBytes()
    // ==
    // lregPtr->getCacheImpl()->getCachePerfStats().getPdxDeSerializationBytes(),
    //"Total pdxDeserializationBytes should be equal to Total
    // pdxSerializationsBytes.");

    LOG("Puts22 complete.\n");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, Get2)
  {
    try {
      auto serializationRegistry =
          CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
              ->getSerializationRegistry();

      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes1::createDeserializable);
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes2::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport = CacheableKey::create(2);
    auto obj2 =
        std::dynamic_pointer_cast<PdxTests::PdxTypes2>(regPtr0->get(keyport));

    LOG("Get2 complete.\n");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, PutAndVerifyPdxInGet)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxType::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          Address::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport = CacheableKey::create(1);

    auto pdxobj = std::make_shared<PdxTests::PdxType>();

    regPtr0->put(keyport, pdxobj);

    auto obj2 =
        std::dynamic_pointer_cast<PdxTests::PdxType>(regPtr0->get(keyport));

    checkPdxInstanceToStringAtServer(regPtr0);

    ASSERT(cacheHelper->getCache()->getPdxReadSerialized() == false,
           "Pdx read serialized property should be false.");

    LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
    LOG_INFO(
        "PdxSerializations = %d ",
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
    LOG_INFO(
        "PdxDeSerializations = %d ",
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxDeSerializations());
    LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                 ->getCachePerfStats()
                                                 .getPdxSerializationBytes());
    LOG_INFO("PdxDeSerializationBytes = %ld ",
             lregPtr->getCacheImpl()
                 ->getCachePerfStats()
                 .getPdxDeSerializationBytes());
    ASSERT(
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() ==
            lregPtr->getCacheImpl()
                ->getCachePerfStats()
                .getPdxDeSerializations(),
        "Total pdxDeserializations should be equal to Total "
        "pdxSerializations.");
    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxSerializationBytes() ==
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes(),
           "Total pdxDeserializationBytes should be equal to Total "
           "pdxSerializationsBytes.");

    LOG("StepThree complete.\n");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, PutAndVerifyNestedPdxInGet)
  {
    LOG("PutAndVerifyNestedPdxInGet started.");
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          NestedPdx::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes1::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes2::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto p1 = std::make_shared<NestedPdx>();
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport = CacheableKey::create(1);

    regPtr0->put(keyport, p1);

    auto obj2 = std::dynamic_pointer_cast<NestedPdx>(regPtr0->get(keyport));

    ASSERT(obj2->equals(p1) == true, "Nested pdx objects should be equal");

    LOG("PutAndVerifyNestedPdxInGet complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, PutMixedVersionNestedPdx)
  {
    LOG("PutMixedVersionNestedPdx started.");
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          MixedVersionNestedPdx::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes1::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes2::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    LOG("auto p1 = std::make_shared<MixedVersionNestedPdx>();	");
    auto p1 = std::make_shared<MixedVersionNestedPdx>();
    auto p2 = std::make_shared<MixedVersionNestedPdx>();
    auto p3 = std::make_shared<MixedVersionNestedPdx>();
    LOG("RegionPtr regPtr0 = getHelper()->getRegion(\"DistRegionAck\");");
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    LOG("std::shared_ptr<CacheableKey> keyport1 = CacheableKey::create(1);");
    auto keyport1 = CacheableKey::create(1);
    auto keyport2 = CacheableKey::create(2);
    auto keyport3 = CacheableKey::create(3);

    LOG("regPtr0->put(keyport1, p1 );");
    regPtr0->put(keyport1, p1);
    LOG("regPtr0->put(keyport2, p2 );");
    regPtr0->put(keyport2, p2);
    LOG("regPtr0->put(keyport3, p3 );");
    regPtr0->put(keyport3, p3);
    LOG("PutMixedVersionNestedPdx complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, PutAndVerifyPdxInGFSInGet)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    try {
      serializationRegistry->addDataSerializableType(
          PdxInsideIGeodeSerializable::createDeserializable, 0x10);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          NestedPdx::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes1::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes2::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes3::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes4::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes5::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes6::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes7::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes8::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto np = std::make_shared<PdxInsideIGeodeSerializable>();

    auto keyport = CacheableKey::create(1);
    regPtr0->put(keyport, np);

    // GET
    auto pRet = std::dynamic_pointer_cast<PdxInsideIGeodeSerializable>(
        regPtr0->get(keyport));
    ASSERT(
        pRet->equals(np) == true,
        "TASK PutAndVerifyPdxInIGFSInGet: PdxInsideIGeodeSerializable objects "
        "should be equal");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyPdxInGFSGetOnly)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    try {
      serializationRegistry->addDataSerializableType(
          PdxInsideIGeodeSerializable::createDeserializable, 0x10);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          NestedPdx::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes1::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes2::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes3::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes4::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes5::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes6::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes7::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes8::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto orig = std::make_shared<PdxInsideIGeodeSerializable>();

    // GET
    auto keyport = CacheableKey::create(1);
    auto pRet = std::dynamic_pointer_cast<PdxInsideIGeodeSerializable>(
        regPtr0->get(keyport));
    ASSERT(pRet->equals(orig) == true,
           "TASK:VerifyPdxInIGFSGetOnly, PdxInsideIGeodeSerializable objects "
           "should "
           "be equal");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyMixedVersionNestedGetOnly)
  {
    LOG("VerifyMixedVersionNestedGetOnly started.");

    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    try {
      serializationRegistry->addPdxSerializableType(
          MixedVersionNestedPdx::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes1::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes2::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto p1 = std::make_shared<MixedVersionNestedPdx>();
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport1 = CacheableKey::create(1);
    auto keyport2 = CacheableKey::create(2);
    auto keyport3 = CacheableKey::create(3);

    auto obj1 = std::dynamic_pointer_cast<MixedVersionNestedPdx>(
        regPtr0->get(keyport1));
    auto obj2 = std::dynamic_pointer_cast<MixedVersionNestedPdx>(
        regPtr0->get(keyport2));
    auto obj3 = std::dynamic_pointer_cast<MixedVersionNestedPdx>(
        regPtr0->get(keyport3));

    ASSERT(obj1->equals(p1) == true, "Nested pdx objects should be equal");

    LOG("VerifyMixedVersionNestedGetOnly complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyNestedGetOnly)
  {
    LOG("VerifyNestedGetOnly started.");

    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    try {
      serializationRegistry->addPdxSerializableType(
          NestedPdx::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes1::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes2::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto p1 = std::make_shared<NestedPdx>();
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport = CacheableKey::create(1);

    auto obj2 = std::dynamic_pointer_cast<NestedPdx>(regPtr0->get(keyport));

    ASSERT(obj2->equals(p1) == true, "Nested pdx objects should be equal");

    LOG("VerifyNestedGetOnly complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyGetOnly)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxType::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          Address::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport = CacheableKey::create(1);
    auto obj2 =
        std::dynamic_pointer_cast<PdxTests::PdxType>(regPtr0->get(keyport));

    checkPdxInstanceToStringAtServer(regPtr0);

    LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
    LOG_INFO(
        "PdxSerializations = %d ",
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
    LOG_INFO(
        "PdxDeSerializations = %d ",
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxDeSerializations());
    LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                 ->getCachePerfStats()
                                                 .getPdxSerializationBytes());
    LOG_INFO("PdxDeSerializationBytes = %ld ",
             lregPtr->getCacheImpl()
                 ->getCachePerfStats()
                 .getPdxDeSerializationBytes());
    ASSERT(lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() <
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializations(),
           "Total pdxDeserializations should be less than Total "
           "pdxSerializations.");
    ASSERT(
        lregPtr->getCacheImpl()
                ->getCachePerfStats()
                .getPdxSerializationBytes() < lregPtr->getCacheImpl()
                                                  ->getCachePerfStats()
                                                  .getPdxDeSerializationBytes(),
        "Total pdxDeserializationBytes should be less than Total "
        "pdxSerializationsBytes.");
    LOG("StepFour complete.\n");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, PutAndVerifyVariousPdxTypes)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes1::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes2::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes3::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes4::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes5::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes6::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes7::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes8::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes9::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes10::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    // TODO
    // serializationRegistry->addPdxType(PdxTests.PortfolioPdx.CreateDeserializable);
    // serializationRegistry->addPdxType(PdxTests.PositionPdx.CreateDeserializable);

    // Region region0 = CacheHelper.GetVerifyRegion<object,
    // object>(m_regionNames[0]);
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    bool flag = false;
    {
      auto p1 = std::make_shared<PdxTypes1>();
      auto keyport = CacheableKey::create(11);
      regPtr0->put(keyport, p1);
      auto pRet = std::dynamic_pointer_cast<PdxTypes1>(regPtr0->get(keyport));

      flag = p1->equals(pRet);
      LOG_DEBUG("PutAndVerifyVariousPdxTypes:.. flag = %d", flag);
      ASSERT(flag == true, "Objects of type PdxTypes1 should be equal");
      checkPdxInstanceToStringAtServer(regPtr0);

      LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
      LOG_INFO(
          "PdxSerializations = %d ",
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
      LOG_INFO("PdxDeSerializations = %d ", lregPtr->getCacheImpl()
                                                ->getCachePerfStats()
                                                .getPdxDeSerializations());
      LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                   ->getCachePerfStats()
                                                   .getPdxSerializationBytes());
      LOG_INFO("PdxDeSerializationBytes = %ld ",
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes());
      ASSERT(
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() ==
              lregPtr->getCacheImpl()
                  ->getCachePerfStats()
                  .getPdxDeSerializations(),
          "Total pdxDeserializations should be equal to Total "
          "pdxSerializations.");
      ASSERT(lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxSerializationBytes() ==
                 lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxDeSerializationBytes(),
             "Total pdxDeserializationBytes should be equal to Total "
             "pdxSerializationsBytes.");
    }

    {
      auto p2 = std::make_shared<PdxTypes2>();
      auto keyport2 = CacheableKey::create(12);
      regPtr0->put(keyport2, p2);
      auto pRet2 = std::dynamic_pointer_cast<PdxTypes2>(regPtr0->get(keyport2));

      flag = p2->equals(pRet2);
      LOG_DEBUG("PutAndVerifyVariousPdxTypes:.. flag = %d", flag);
      ASSERT(flag == true, "Objects of type PdxTypes2 should be equal");
      checkPdxInstanceToStringAtServer(regPtr0);

      LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
      LOG_INFO(
          "PdxSerializations = %d ",
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
      LOG_INFO("PdxDeSerializations = %d ", lregPtr->getCacheImpl()
                                                ->getCachePerfStats()
                                                .getPdxDeSerializations());
      LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                   ->getCachePerfStats()
                                                   .getPdxSerializationBytes());
      LOG_INFO("PdxDeSerializationBytes = %ld ",
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes());
      ASSERT(
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() ==
              lregPtr->getCacheImpl()
                  ->getCachePerfStats()
                  .getPdxDeSerializations(),
          "Total pdxDeserializations should be equal to Total "
          "pdxSerializations.");
      ASSERT(lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxSerializationBytes() ==
                 lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxDeSerializationBytes(),
             "Total pdxDeserializationBytes should be equal to Total "
             "pdxSerializationsBytes.");
    }

    {
      auto p3 = std::make_shared<PdxTypes3>();
      auto keyport3 = CacheableKey::create(13);
      regPtr0->put(keyport3, p3);
      auto pRet3 = std::dynamic_pointer_cast<PdxTypes3>(regPtr0->get(keyport3));

      flag = p3->equals(pRet3);
      LOG_DEBUG("PutAndVerifyVariousPdxTypes:.. flag = %d", flag);
      ASSERT(flag == true, "Objects of type PdxTypes3 should be equal");
      checkPdxInstanceToStringAtServer(regPtr0);

      LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
      LOG_INFO(
          "PdxSerializations = %d ",
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
      LOG_INFO("PdxDeSerializations = %d ", lregPtr->getCacheImpl()
                                                ->getCachePerfStats()
                                                .getPdxDeSerializations());
      LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                   ->getCachePerfStats()
                                                   .getPdxSerializationBytes());
      LOG_INFO("PdxDeSerializationBytes = %ld ",
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes());
      ASSERT(
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() ==
              lregPtr->getCacheImpl()
                  ->getCachePerfStats()
                  .getPdxDeSerializations(),
          "Total pdxDeserializations should be equal to Total "
          "pdxSerializations.");
      ASSERT(lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxSerializationBytes() ==
                 lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxDeSerializationBytes(),
             "Total pdxDeserializationBytes should be equal to Total "
             "pdxSerializationsBytes.");
    }

    {
      auto p4 = std::make_shared<PdxTypes4>();
      auto keyport4 = CacheableKey::create(14);
      regPtr0->put(keyport4, p4);
      auto pRet4 = std::dynamic_pointer_cast<PdxTypes4>(regPtr0->get(keyport4));

      flag = p4->equals(pRet4);
      LOG_DEBUG("PutAndVerifyVariousPdxTypes:.. flag = %d", flag);
      ASSERT(flag == true, "Objects of type PdxTypes4 should be equal");
      checkPdxInstanceToStringAtServer(regPtr0);

      LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
      LOG_INFO(
          "PdxSerializations = %d ",
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
      LOG_INFO("PdxDeSerializations = %d ", lregPtr->getCacheImpl()
                                                ->getCachePerfStats()
                                                .getPdxDeSerializations());
      LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                   ->getCachePerfStats()
                                                   .getPdxSerializationBytes());
      LOG_INFO("PdxDeSerializationBytes = %ld ",
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes());
      ASSERT(
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() ==
              lregPtr->getCacheImpl()
                  ->getCachePerfStats()
                  .getPdxDeSerializations(),
          "Total pdxDeserializations should be equal to Total "
          "pdxSerializations.");
      ASSERT(lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxSerializationBytes() ==
                 lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxDeSerializationBytes(),
             "Total pdxDeserializationBytes should be equal to Total "
             "pdxSerializationsBytes.");
    }

    {
      auto p5 = std::make_shared<PdxTypes5>();
      auto keyport5 = CacheableKey::create(15);
      regPtr0->put(keyport5, p5);
      auto pRet5 = std::dynamic_pointer_cast<PdxTypes5>(regPtr0->get(keyport5));

      flag = p5->equals(pRet5);
      LOG_DEBUG("PutAndVerifyVariousPdxTypes:.. flag = %d", flag);
      ASSERT(flag == true, "Objects of type PdxTypes5 should be equal");
      checkPdxInstanceToStringAtServer(regPtr0);

      LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
      LOG_INFO(
          "PdxSerializations = %d ",
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
      LOG_INFO("PdxDeSerializations = %d ", lregPtr->getCacheImpl()
                                                ->getCachePerfStats()
                                                .getPdxDeSerializations());
      LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                   ->getCachePerfStats()
                                                   .getPdxSerializationBytes());
      LOG_INFO("PdxDeSerializationBytes = %ld ",
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes());
      ASSERT(
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() ==
              lregPtr->getCacheImpl()
                  ->getCachePerfStats()
                  .getPdxDeSerializations(),
          "Total pdxDeserializations should be equal to Total "
          "pdxSerializations.");
      ASSERT(lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxSerializationBytes() ==
                 lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxDeSerializationBytes(),
             "Total pdxDeserializationBytes should be equal to Total "
             "pdxSerializationsBytes.");
    }

    {
      auto p6 = std::make_shared<PdxTypes6>();
      auto keyport6 = CacheableKey::create(16);
      regPtr0->put(keyport6, p6);
      auto pRet6 = std::dynamic_pointer_cast<PdxTypes6>(regPtr0->get(keyport6));

      flag = p6->equals(pRet6);
      LOG_DEBUG("PutAndVerifyVariousPdxTypes:.. flag = %d", flag);
      ASSERT(flag == true, "Objects of type PdxTypes6 should be equal");
      checkPdxInstanceToStringAtServer(regPtr0);

      LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
      LOG_INFO(
          "PdxSerializations = %d ",
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
      LOG_INFO("PdxDeSerializations = %d ", lregPtr->getCacheImpl()
                                                ->getCachePerfStats()
                                                .getPdxDeSerializations());
      LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                   ->getCachePerfStats()
                                                   .getPdxSerializationBytes());
      LOG_INFO("PdxDeSerializationBytes = %ld ",
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes());
      ASSERT(
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() ==
              lregPtr->getCacheImpl()
                  ->getCachePerfStats()
                  .getPdxDeSerializations(),
          "Total pdxDeserializations should be equal to Total "
          "pdxSerializations.");
      ASSERT(lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxSerializationBytes() ==
                 lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxDeSerializationBytes(),
             "Total pdxDeserializationBytes should be equal to Total "
             "pdxSerializationsBytes.");
    }

    {
      auto p7 = std::make_shared<PdxTypes7>();
      auto keyport7 = CacheableKey::create(17);
      regPtr0->put(keyport7, p7);
      auto pRet7 = std::dynamic_pointer_cast<PdxTypes7>(regPtr0->get(keyport7));

      flag = p7->equals(pRet7);
      LOG_DEBUG("PutAndVerifyVariousPdxTypes:.. flag = %d", flag);
      ASSERT(flag == true, "Objects of type PdxTypes7 should be equal");
      checkPdxInstanceToStringAtServer(regPtr0);

      LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
      LOG_INFO(
          "PdxSerializations = %d ",
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
      LOG_INFO("PdxDeSerializations = %d ", lregPtr->getCacheImpl()
                                                ->getCachePerfStats()
                                                .getPdxDeSerializations());
      LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                   ->getCachePerfStats()
                                                   .getPdxSerializationBytes());
      LOG_INFO("PdxDeSerializationBytes = %ld ",
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes());
      ASSERT(
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() ==
              lregPtr->getCacheImpl()
                  ->getCachePerfStats()
                  .getPdxDeSerializations(),
          "Total pdxDeserializations should be equal to Total "
          "pdxSerializations.");
      ASSERT(lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxSerializationBytes() ==
                 lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxDeSerializationBytes(),
             "Total pdxDeserializationBytes should be equal to Total "
             "pdxSerializationsBytes.");
    }

    {
      auto p8 = std::make_shared<PdxTypes8>();
      auto keyport8 = CacheableKey::create(18);
      regPtr0->put(keyport8, p8);
      auto pRet8 = std::dynamic_pointer_cast<PdxTypes8>(regPtr0->get(keyport8));

      flag = p8->equals(pRet8);
      LOG_DEBUG("PutAndVerifyVariousPdxTypes:.. flag = %d", flag);
      ASSERT(flag == true, "Objects of type PdxTypes8 should be equal");
      checkPdxInstanceToStringAtServer(regPtr0);

      LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
      LOG_INFO(
          "PdxSerializations = %d ",
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
      LOG_INFO("PdxDeSerializations = %d ", lregPtr->getCacheImpl()
                                                ->getCachePerfStats()
                                                .getPdxDeSerializations());
      LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                   ->getCachePerfStats()
                                                   .getPdxSerializationBytes());
      LOG_INFO("PdxDeSerializationBytes = %ld ",
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes());
      ASSERT(
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() ==
              lregPtr->getCacheImpl()
                  ->getCachePerfStats()
                  .getPdxDeSerializations(),
          "Total pdxDeserializations should be equal to Total "
          "pdxSerializations.");
      ASSERT(lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxSerializationBytes() ==
                 lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxDeSerializationBytes(),
             "Total pdxDeserializationBytes should be equal to Total "
             "pdxSerializationsBytes.");
    }

    {
      auto p9 = std::make_shared<PdxTypes9>();
      auto keyport9 = CacheableKey::create(19);
      regPtr0->put(keyport9, p9);
      auto pRet9 = std::dynamic_pointer_cast<PdxTypes9>(regPtr0->get(keyport9));

      flag = p9->equals(pRet9);
      LOG_DEBUG("PutAndVerifyVariousPdxTypes:.. flag = %d", flag);
      ASSERT(flag == true, "Objects of type PdxTypes9 should be equal");
      checkPdxInstanceToStringAtServer(regPtr0);

      LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
      LOG_INFO(
          "PdxSerializations = %d ",
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
      LOG_INFO("PdxDeSerializations = %d ", lregPtr->getCacheImpl()
                                                ->getCachePerfStats()
                                                .getPdxDeSerializations());
      LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                   ->getCachePerfStats()
                                                   .getPdxSerializationBytes());
      LOG_INFO("PdxDeSerializationBytes = %ld ",
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes());
      ASSERT(
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() ==
              lregPtr->getCacheImpl()
                  ->getCachePerfStats()
                  .getPdxDeSerializations(),
          "Total pdxDeserializations should be equal to Total "
          "pdxSerializations.");
      ASSERT(lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxSerializationBytes() ==
                 lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxDeSerializationBytes(),
             "Total pdxDeserializationBytes should be equal to Total "
             "pdxSerializationsBytes.");
    }

    {
      auto p10 = std::make_shared<PdxTypes10>();
      auto keyport10 = CacheableKey::create(20);
      regPtr0->put(keyport10, p10);
      auto pRet10 =
          std::dynamic_pointer_cast<PdxTypes10>(regPtr0->get(keyport10));

      flag = p10->equals(pRet10);
      LOG_DEBUG("PutAndVerifyVariousPdxTypes:.. flag = %d", flag);
      ASSERT(flag == true, "Objects of type PdxTypes10 should be equal");
      checkPdxInstanceToStringAtServer(regPtr0);

      LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
      LOG_INFO(
          "PdxSerializations = %d ",
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
      LOG_INFO("PdxDeSerializations = %d ", lregPtr->getCacheImpl()
                                                ->getCachePerfStats()
                                                .getPdxDeSerializations());
      LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                   ->getCachePerfStats()
                                                   .getPdxSerializationBytes());
      LOG_INFO("PdxDeSerializationBytes = %ld ",
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes());
      ASSERT(
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() ==
              lregPtr->getCacheImpl()
                  ->getCachePerfStats()
                  .getPdxDeSerializations(),
          "Total pdxDeserializations should be equal to Total "
          "pdxSerializations.");
      ASSERT(lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxSerializationBytes() ==
                 lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxDeSerializationBytes(),
             "Total pdxDeserializationBytes should be equal to Total "
             "pdxSerializationsBytes.");
    }

    LOG("NIL:329:StepFive complete.\n");
  }
END_TASK_DEFINITION

// TestCase-1
// C1.generateJavaPdxType
DUNIT_TASK_DEFINITION(CLIENT1, generateJavaPdxType)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto args = CacheableKey::create("saveAllJavaPdxTypes");
    auto key = CacheableKey::create(1);
    auto routingObj = CacheableVector::create();
    routingObj->push_back(key);

    auto funcExec = FunctionService::onRegion(regPtr0);

    auto collector = funcExec.withArgs(args)
                         .withFilter(routingObj)
                         .execute("ComparePdxTypes");
    ASSERT(collector != nullptr, "onRegion collector nullptr");

    auto result = collector->getResult();
    LOG_INFO("NIL:: testTCPDXTests: result->size = %d ", result->size());
    if (result == nullptr) {
      ASSERT(false, "echo String : result is nullptr");
    } else {
      //
      bool gotResult = false;
      for (size_t i = 0; i < result->size(); i++) {
        try {
          auto boolValue = std::dynamic_pointer_cast<CacheableBoolean>(
              result->operator[](i));
          LOG_INFO("NIL:: boolValue is %d ", boolValue->value());
          bool resultVal = boolValue->value();
          ASSERT(resultVal == true,
                 "Function should return true NIL LINE_1508");
          gotResult = true;
        } catch (ClassCastException &ex) {
          LOG("exFuncNameSendException casting to int for arrayList arguement "
              "exception.");
          std::string logmsg = "";
          logmsg += ex.getName();
          logmsg += ": ";
          logmsg += ex.what();
          LOG(logmsg.c_str());
          LOG(ex.getStackTrace());
          LOG("exFuncNameSendException now casting to "
              "UserFunctionExecutionExceptionPtr for arrayList arguement "
              "exception.");
          auto uFEPtr =
              std::dynamic_pointer_cast<UserFunctionExecutionException>(
                  result->operator[](i));
          ASSERT(uFEPtr != nullptr, "uFEPtr exception is nullptr");
          LOG_INFO("Done casting to uFEPtr");
          LOG_INFO("Read expected uFEPtr exception %s ",
                   uFEPtr->getMessage().c_str());
        } catch (...) {
          FAIL(
              "exFuncNameSendException casting to string for bool arguement "
              "Unknown exception.");
        }
      }
      ASSERT(gotResult == true, "Function should (gotResult) return true ");
      //
    }
  }
END_TASK_DEFINITION
// C1.putAllPdxTypes
DUNIT_TASK_DEFINITION(CLIENT1, putAllPdxTypes)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes1::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes2::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes3::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes4::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes5::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes6::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes7::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes8::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes9::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes10::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    // TODO::Uncomment it once PortfolioPdx/PositionPdx Classes are ready
    // serializationRegistry->addPdxType(PdxTests.PortfolioPdx.CreateDeserializable);
    // serializationRegistry->addPdxType(PdxTests.PositionPdx.CreateDeserializable);

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto p1 = std::make_shared<PdxTypes1>();
    auto keyport1 = CacheableKey::create(p1->getClassName().c_str());
    regPtr0->put(keyport1, p1);

    auto p2 = std::make_shared<PdxTypes2>();
    auto keyport2 = CacheableKey::create(p2->getClassName().c_str());
    regPtr0->put(keyport2, p2);

    auto p3 = std::make_shared<PdxTypes3>();
    auto keyport3 = CacheableKey::create(p3->getClassName().c_str());
    regPtr0->put(keyport3, p3);

    auto p4 = std::make_shared<PdxTypes4>();
    auto keyport4 = CacheableKey::create(p4->getClassName().c_str());
    regPtr0->put(keyport4, p4);

    auto p5 = std::make_shared<PdxTypes5>();
    auto keyport5 = CacheableKey::create(p5->getClassName().c_str());
    regPtr0->put(keyport5, p5);

    auto p6 = std::make_shared<PdxTypes6>();
    auto keyport6 = CacheableKey::create(p6->getClassName().c_str());
    regPtr0->put(keyport6, p6);

    auto p7 = std::make_shared<PdxTypes7>();
    auto keyport7 = CacheableKey::create(p7->getClassName().c_str());
    regPtr0->put(keyport7, p7);

    auto p8 = std::make_shared<PdxTypes8>();
    auto keyport8 = CacheableKey::create(p8->getClassName().c_str());
    regPtr0->put(keyport8, p8);

    auto p9 = std::make_shared<PdxTypes9>();
    auto keyport9 = CacheableKey::create(p9->getClassName().c_str());
    regPtr0->put(keyport9, p9);

    auto p10 = std::make_shared<PdxTypes10>();
    auto keyport10 = CacheableKey::create(p10->getClassName().c_str());
    regPtr0->put(keyport10, p10);

    //
  }
END_TASK_DEFINITION

// C1.verifyDotNetPdxTypes
DUNIT_TASK_DEFINITION(CLIENT1, verifyDotNetPdxTypes)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto args = CacheableKey::create("compareDotNETPdxTypes");
    auto key = CacheableKey::create(1);
    auto routingObj = CacheableVector::create();
    routingObj->push_back(key);

    auto funcExec = FunctionService::onRegion(regPtr0);

    auto collector = funcExec.withArgs(args)
                         .withFilter(routingObj)
                         .execute("ComparePdxTypes");
    ASSERT(collector != nullptr, "onRegion collector nullptr");

    auto result = collector->getResult();
    LOG_INFO("NIL:: testTCPDXTests:verifyDotNetPdxTypes result->size = %d ",
             result->size());
    if (result == nullptr) {
      ASSERT(false, "echo String : result is nullptr");
    } else {
      bool gotResult = false;
      for (size_t i = 0; i < result->size(); i++) {
        try {
          auto boolValue = std::dynamic_pointer_cast<CacheableBoolean>(
              result->operator[](i));
          LOG_INFO("NIL::verifyDotNetPdxTypes boolValue is %d ",
                   boolValue->value());
          bool resultVal = boolValue->value();
          ASSERT(resultVal == true,
                 "Function should return true NIL LINE_1508");
          gotResult = true;
        } catch (ClassCastException &ex) {
          LOG("exFuncNameSendException casting to int for arrayList arguement "
              "exception.");
          std::string logmsg = "";
          logmsg += ex.getName();
          logmsg += ": ";
          logmsg += ex.what();
          LOG(logmsg.c_str());
          LOG(ex.getStackTrace());
          LOG("exFuncNameSendException now casting to "
              "UserFunctionExecutionExceptionPtr for arrayList arguement "
              "exception.");
          auto uFEPtr =
              std::dynamic_pointer_cast<UserFunctionExecutionException>(
                  result->operator[](i));
          ASSERT(uFEPtr != nullptr, "uFEPtr exception is nullptr");
          LOG_INFO("Done casting to uFEPtr");
          LOG_INFO("Read expected uFEPtr exception %s ",
                   uFEPtr->getMessage().c_str());
        } catch (...) {
          FAIL(
              "exFuncNameSendException casting to string for bool arguement "
              "Unknown exception.");
        }
      }
      ASSERT(gotResult == true, "Function should (gotResult) return true ");
    }
  }
END_TASK_DEFINITION
// END TestCase-1

// TestCase-2
// c1.client1PutsV1Object
DUNIT_TASK_DEFINITION(CLIENT1, client1PutsV1Object)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxType3V1::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    PdxTests::PdxType3V1::reset(false);
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto keyport = CacheableKey::create(1);
    auto np = std::make_shared<PdxType3V1>();

    regPtr0->put(keyport, np);
  }
END_TASK_DEFINITION
// c2.client2GetsV1ObjectAndPutsV2Object
DUNIT_TASK_DEFINITION(CLIENT2, client2GetsV1ObjectAndPutsV2Object)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes3V2::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    PdxTests::PdxTypes3V2::reset(false);
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    // get v1 object
    auto keyport = CacheableKey::create(1);
    auto pRet = std::dynamic_pointer_cast<PdxTypes3V2>(regPtr0->get(keyport));

    // now put v2 object
    auto np = std::make_shared<PdxTypes3V2>();
    regPtr0->put(keyport, np);

    LOG_DEBUG("Task:client2GetsV1ObjectAndPutsV2Object Done successfully ");
  }
END_TASK_DEFINITION
// c3.client3GetsV2Object
DUNIT_TASK_DEFINITION(CLIENT3, client3GetsV2Object)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto args = CacheableKey::create("compareDotNETPdxTypes");
    auto key = CacheableKey::create(1);
    auto routingObj = CacheableVector::create();
    routingObj->push_back(key);

    auto funcExec = FunctionService::onRegion(regPtr0);

    auto collector = funcExec.execute("IterateRegion");
    ASSERT(collector != nullptr, "onRegion collector nullptr");

    auto result = collector->getResult();
    LOG_INFO("NIL:: testTCPDXTests:verifyDotNetPdxTypes result->size = %d ",
             result->size());
    if (result == nullptr) {
      ASSERT(false, "echo String : result is nullptr");
    } else {
      bool gotResult = false;
      for (size_t i = 0; i < result->size(); i++) {
        try {
          auto boolValue = std::dynamic_pointer_cast<CacheableBoolean>(
              result->operator[](i));
          LOG_INFO("NIL::verifyDotNetPdxTypes boolValue is %d ",
                   boolValue->value());
          bool resultVal = boolValue->value();
          ASSERT(resultVal == true,
                 "Function should return true NIL LINE_1508");
          gotResult = true;
        } catch (ClassCastException &ex) {
          LOG("exFuncNameSendException casting to int for arrayList arguement "
              "exception.");
          std::string logmsg = "";
          logmsg += ex.getName();
          logmsg += ": ";
          logmsg += ex.what();
          LOG(logmsg.c_str());
          LOG(ex.getStackTrace());
          LOG("exFuncNameSendException now casting to "
              "UserFunctionExecutionExceptionPtr for arrayList arguement "
              "exception.");
          auto uFEPtr =
              std::dynamic_pointer_cast<UserFunctionExecutionException>(
                  result->operator[](i));
          ASSERT(uFEPtr != nullptr, "uFEPtr exception is nullptr");
          LOG_INFO("Done casting to uFEPtr");
          LOG_INFO("Read expected uFEPtr exception %s ",
                   uFEPtr->getMessage().c_str());
        } catch (...) {
          FAIL(
              "exFuncNameSendException casting to string for bool arguement "
              "Unknown exception.");
        }
      }
      ASSERT(gotResult == true, "Function should (gotResult) return true ");
    }
  }
END_TASK_DEFINITION
// END TestCase-2

DUNIT_TASK_DEFINITION(CLIENT2, VerifyVariousPdxGets)
  {
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes1::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes2::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes3::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes4::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes5::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes6::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes7::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes8::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes9::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes10::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }
    // TODO::Uncomment it once PortfolioPdx/PositionPdx Classes are ready
    // serializationRegistry->addPdxType(PdxTests.PortfolioPdx.CreateDeserializable);
    // serializationRegistry->addPdxType(PdxTests.PositionPdx.CreateDeserializable);

    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    bool flag = false;
    {
      auto p1 = std::make_shared<PdxTypes1>();
      auto keyport = CacheableKey::create(11);
      auto pRet = std::dynamic_pointer_cast<PdxTypes1>(regPtr0->get(keyport));

      flag = p1->equals(pRet);
      LOG_DEBUG("VerifyVariousPdxGets:.. flag = %d", flag);
      ASSERT(flag == true,
             "VerifyVariousPdxGets:Objects of type PdxTypes1 should be equal");
      checkPdxInstanceToStringAtServer(regPtr0);

      LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
      LOG_INFO(
          "PdxSerializations = %d ",
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
      LOG_INFO("PdxDeSerializations = %d ", lregPtr->getCacheImpl()
                                                ->getCachePerfStats()
                                                .getPdxDeSerializations());
      LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                   ->getCachePerfStats()
                                                   .getPdxSerializationBytes());
      LOG_INFO("PdxDeSerializationBytes = %ld ",
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes());
      ASSERT(
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() <
              lregPtr->getCacheImpl()
                  ->getCachePerfStats()
                  .getPdxDeSerializations(),
          "Total pdxDeserializations should be less than Total "
          "pdxSerializations.");
      ASSERT(lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxSerializationBytes() <
                 lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxDeSerializationBytes(),
             "Total pdxDeserializationBytes should be less than Total "
             "pdxSerializationsBytes.");
    }

    {
      auto p2 = std::make_shared<PdxTypes2>();
      auto keyport2 = CacheableKey::create(12);
      auto pRet2 = std::dynamic_pointer_cast<PdxTypes2>(regPtr0->get(keyport2));

      flag = p2->equals(pRet2);
      LOG_DEBUG("VerifyVariousPdxGets:. flag = %d", flag);
      ASSERT(flag == true,
             "VerifyVariousPdxGets:Objects of type PdxTypes2 should be equal");
      checkPdxInstanceToStringAtServer(regPtr0);

      LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
      LOG_INFO(
          "PdxSerializations = %d ",
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
      LOG_INFO("PdxDeSerializations = %d ", lregPtr->getCacheImpl()
                                                ->getCachePerfStats()
                                                .getPdxDeSerializations());
      LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                   ->getCachePerfStats()
                                                   .getPdxSerializationBytes());
      LOG_INFO("PdxDeSerializationBytes = %ld ",
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes());
      ASSERT(
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() <
              lregPtr->getCacheImpl()
                  ->getCachePerfStats()
                  .getPdxDeSerializations(),
          "Total pdxDeserializations should be less than Total "
          "pdxSerializations.");
      ASSERT(lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxSerializationBytes() <
                 lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxDeSerializationBytes(),
             "Total pdxDeserializationBytes should be less than Total "
             "pdxSerializationsBytes.");
    }

    {
      auto p3 = std::make_shared<PdxTypes3>();
      auto keyport3 = CacheableKey::create(13);
      auto pRet3 = std::dynamic_pointer_cast<PdxTypes3>(regPtr0->get(keyport3));

      flag = p3->equals(pRet3);
      LOG_DEBUG("VerifyVariousPdxGets:.. flag = %d", flag);
      ASSERT(flag == true,
             "VerifyVariousPdxGets:Objects of type PdxTypes3 should be equal");
      checkPdxInstanceToStringAtServer(regPtr0);

      LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
      LOG_INFO(
          "PdxSerializations = %d ",
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
      LOG_INFO("PdxDeSerializations = %d ", lregPtr->getCacheImpl()
                                                ->getCachePerfStats()
                                                .getPdxDeSerializations());
      LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                   ->getCachePerfStats()
                                                   .getPdxSerializationBytes());
      LOG_INFO("PdxDeSerializationBytes = %ld ",
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes());
      ASSERT(
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() <
              lregPtr->getCacheImpl()
                  ->getCachePerfStats()
                  .getPdxDeSerializations(),
          "Total pdxDeserializations should be less than Total "
          "pdxSerializations.");
      ASSERT(lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxSerializationBytes() <
                 lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxDeSerializationBytes(),
             "Total pdxDeserializationBytes should be less than Total "
             "pdxSerializationsBytes.");
    }

    {
      auto p4 = std::make_shared<PdxTypes4>();
      auto keyport4 = CacheableKey::create(14);
      auto pRet4 = std::dynamic_pointer_cast<PdxTypes4>(regPtr0->get(keyport4));

      flag = p4->equals(pRet4);
      LOG_DEBUG("VerifyVariousPdxGets:.. flag = %d", flag);
      ASSERT(flag == true,
             "VerifyVariousPdxGets:Objects of type PdxTypes4 should be equal");
      checkPdxInstanceToStringAtServer(regPtr0);

      LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
      LOG_INFO(
          "PdxSerializations = %d ",
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
      LOG_INFO("PdxDeSerializations = %d ", lregPtr->getCacheImpl()
                                                ->getCachePerfStats()
                                                .getPdxDeSerializations());
      LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                   ->getCachePerfStats()
                                                   .getPdxSerializationBytes());
      LOG_INFO("PdxDeSerializationBytes = %ld ",
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes());
      ASSERT(
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() <
              lregPtr->getCacheImpl()
                  ->getCachePerfStats()
                  .getPdxDeSerializations(),
          "Total pdxDeserializations should be less than Total "
          "pdxSerializations.");
      ASSERT(lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxSerializationBytes() <
                 lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxDeSerializationBytes(),
             "Total pdxDeserializationBytes should be less than Total "
             "pdxSerializationsBytes.");
    }

    {
      auto p5 = std::make_shared<PdxTypes5>();
      auto keyport5 = CacheableKey::create(15);
      auto pRet5 = std::dynamic_pointer_cast<PdxTypes5>(regPtr0->get(keyport5));

      flag = p5->equals(pRet5);
      LOG_DEBUG("VerifyVariousPdxGets:.. flag = %d", flag);
      ASSERT(flag == true,
             "VerifyVariousPdxGets:Objects of type PdxTypes5 should be equal");
      checkPdxInstanceToStringAtServer(regPtr0);

      LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
      LOG_INFO(
          "PdxSerializations = %d ",
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
      LOG_INFO("PdxDeSerializations = %d ", lregPtr->getCacheImpl()
                                                ->getCachePerfStats()
                                                .getPdxDeSerializations());
      LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                   ->getCachePerfStats()
                                                   .getPdxSerializationBytes());
      LOG_INFO("PdxDeSerializationBytes = %ld ",
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes());
      ASSERT(
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() <
              lregPtr->getCacheImpl()
                  ->getCachePerfStats()
                  .getPdxDeSerializations(),
          "Total pdxDeserializations should be less than Total "
          "pdxSerializations.");
      ASSERT(lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxSerializationBytes() <
                 lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxDeSerializationBytes(),
             "Total pdxDeserializationBytes should be less than Total "
             "pdxSerializationsBytes.");
    }

    {
      auto p6 = std::make_shared<PdxTypes6>();
      auto keyport6 = CacheableKey::create(16);
      auto pRet6 = std::dynamic_pointer_cast<PdxTypes6>(regPtr0->get(keyport6));

      flag = p6->equals(pRet6);
      LOG_DEBUG("VerifyVariousPdxGets:.. flag = %d", flag);
      ASSERT(flag == true,
             "VerifyVariousPdxGets:Objects of type PdxTypes6 should be equal");
      checkPdxInstanceToStringAtServer(regPtr0);

      LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
      LOG_INFO(
          "PdxSerializations = %d ",
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
      LOG_INFO("PdxDeSerializations = %d ", lregPtr->getCacheImpl()
                                                ->getCachePerfStats()
                                                .getPdxDeSerializations());
      LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                   ->getCachePerfStats()
                                                   .getPdxSerializationBytes());
      LOG_INFO("PdxDeSerializationBytes = %ld ",
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes());
      ASSERT(
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() <
              lregPtr->getCacheImpl()
                  ->getCachePerfStats()
                  .getPdxDeSerializations(),
          "Total pdxDeserializations should be less than Total "
          "pdxSerializations.");
      ASSERT(lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxSerializationBytes() <
                 lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxDeSerializationBytes(),
             "Total pdxDeserializationBytes should be less than Total "
             "pdxSerializationsBytes.");
    }

    {
      auto p7 = std::make_shared<PdxTypes7>();
      auto keyport7 = CacheableKey::create(17);
      auto pRet7 = std::dynamic_pointer_cast<PdxTypes7>(regPtr0->get(keyport7));

      flag = p7->equals(pRet7);
      LOG_DEBUG("VerifyVariousPdxGets:.. flag = %d", flag);
      ASSERT(flag == true,
             "VerifyVariousPdxGets:Objects of type PdxTypes7 should be equal");
      checkPdxInstanceToStringAtServer(regPtr0);

      LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
      LOG_INFO(
          "PdxSerializations = %d ",
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
      LOG_INFO("PdxDeSerializations = %d ", lregPtr->getCacheImpl()
                                                ->getCachePerfStats()
                                                .getPdxDeSerializations());
      LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                   ->getCachePerfStats()
                                                   .getPdxSerializationBytes());
      LOG_INFO("PdxDeSerializationBytes = %ld ",
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes());
      ASSERT(
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() <
              lregPtr->getCacheImpl()
                  ->getCachePerfStats()
                  .getPdxDeSerializations(),
          "Total pdxDeserializations should be less than Total "
          "pdxSerializations.");
      ASSERT(lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxSerializationBytes() <
                 lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxDeSerializationBytes(),
             "Total pdxDeserializationBytes should be less than Total "
             "pdxSerializationsBytes.");
    }

    {
      auto p8 = std::make_shared<PdxTypes8>();
      auto keyport8 = CacheableKey::create(18);
      auto pRet8 = std::dynamic_pointer_cast<PdxTypes8>(regPtr0->get(keyport8));

      flag = p8->equals(pRet8);
      LOG_DEBUG("VerifyVariousPdxGets:.. flag = %d", flag);
      ASSERT(flag == true,
             "VerifyVariousPdxGets:Objects of type PdxTypes8 should be equal");
      checkPdxInstanceToStringAtServer(regPtr0);

      LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
      LOG_INFO(
          "PdxSerializations = %d ",
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
      LOG_INFO("PdxDeSerializations = %d ", lregPtr->getCacheImpl()
                                                ->getCachePerfStats()
                                                .getPdxDeSerializations());
      LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                   ->getCachePerfStats()
                                                   .getPdxSerializationBytes());
      LOG_INFO("PdxDeSerializationBytes = %ld ",
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes());
      ASSERT(
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() <
              lregPtr->getCacheImpl()
                  ->getCachePerfStats()
                  .getPdxDeSerializations(),
          "Total pdxDeserializations should be less than Total "
          "pdxSerializations.");
      ASSERT(lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxSerializationBytes() <
                 lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxDeSerializationBytes(),
             "Total pdxDeserializationBytes should be less than Total "
             "pdxSerializationsBytes.");
    }

    {
      auto p9 = std::make_shared<PdxTypes9>();
      auto keyport9 = CacheableKey::create(19);
      auto pRet9 = std::dynamic_pointer_cast<PdxTypes9>(regPtr0->get(keyport9));

      flag = p9->equals(pRet9);
      LOG_DEBUG("VerifyVariousPdxGets:. flag = %d", flag);
      ASSERT(flag == true,
             "VerifyVariousPdxGets:Objects of type PdxTypes9 should be equal");
      checkPdxInstanceToStringAtServer(regPtr0);

      LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
      LOG_INFO(
          "PdxSerializations = %d ",
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
      LOG_INFO("PdxDeSerializations = %d ", lregPtr->getCacheImpl()
                                                ->getCachePerfStats()
                                                .getPdxDeSerializations());
      LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                   ->getCachePerfStats()
                                                   .getPdxSerializationBytes());
      LOG_INFO("PdxDeSerializationBytes = %ld ",
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes());
      ASSERT(
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() <
              lregPtr->getCacheImpl()
                  ->getCachePerfStats()
                  .getPdxDeSerializations(),
          "Total pdxDeserializations should be less than Total "
          "pdxSerializations.");
      ASSERT(lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxSerializationBytes() <
                 lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxDeSerializationBytes(),
             "Total pdxDeserializationBytes should be less than Total "
             "pdxSerializationsBytes.");
    }

    {
      auto p10 = std::make_shared<PdxTypes10>();
      auto keyport10 = CacheableKey::create(20);
      auto pRet10 =
          std::dynamic_pointer_cast<PdxTypes10>(regPtr0->get(keyport10));

      flag = p10->equals(pRet10);
      LOG_DEBUG("VerifyVariousPdxGets:.. flag = %d", flag);
      ASSERT(flag == true,
             "VerifyVariousPdxGets:Objects of type PdxTypes10 should be equal");
      checkPdxInstanceToStringAtServer(regPtr0);

      LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
      LOG_INFO(
          "PdxSerializations = %d ",
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
      LOG_INFO("PdxDeSerializations = %d ", lregPtr->getCacheImpl()
                                                ->getCachePerfStats()
                                                .getPdxDeSerializations());
      LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                   ->getCachePerfStats()
                                                   .getPdxSerializationBytes());
      LOG_INFO("PdxDeSerializationBytes = %ld ",
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes());
      ASSERT(
          lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() <
              lregPtr->getCacheImpl()
                  ->getCachePerfStats()
                  .getPdxDeSerializations(),
          "Total pdxDeserializations should be less than Total "
          "pdxSerializations.");
      ASSERT(lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxSerializationBytes() <
                 lregPtr->getCacheImpl()
                     ->getCachePerfStats()
                     .getPdxDeSerializationBytes(),
             "Total pdxDeserializationBytes should be less than Total "
             "pdxSerializationsBytes.");
    }
    LOG("NIL:436:StepSix complete.\n");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, putOperation)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    regPtr0->put(1, 1);

    // Verify the CLientName.::putOperation
    // auto testReg = getHelper()->getRegion("testregion");
    auto valuePtr1 = regPtr0->get("clientName1");
    const char *clientName1 =
        (std::dynamic_pointer_cast<CacheableString>(valuePtr1))
            ->value()
            .c_str();
    LOG_INFO(" C1.putOperation Got ClientName1 = %s ", clientName1);
    ASSERT(strcmp(clientName1, "Client-1") == 0,
           "ClientName for Client-1 is not set");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, getOperation)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    auto keyport = CacheableKey::create(1);
    auto value = regPtr0->get(keyport);

    // Verify Client Name for C2
    auto valuePtr2 = regPtr0->get("clientName2");
    const char *clientName2 =
        (std::dynamic_pointer_cast<CacheableString>(valuePtr2))
            ->value()
            .c_str();
    LOG_INFO(" C2.getOperation Got ClientName2 = %s ", clientName2);
    ASSERT(strcmp(clientName2, "Client-2") == 0,
           "ClientName for Client-2 is not set");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, putCharTypes)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::CharTypes::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    LOG("PdxTests::CharTypes Registered Successfully....");

    LOG("Trying to populate PDX objects.....\n");
    auto pdxobj = std::make_shared<PdxTests::CharTypes>();
    auto keyport = CacheableKey::create(1);

    // PUT Operation
    regPtr0->put(keyport, pdxobj);
    LOG("PdxTests::CharTypes: PUT Done successfully....");

    // locally destroy PdxTests::PdxType
    regPtr0->localDestroy(keyport);
    LOG("localDestroy() operation....Done");

    LOG("Done populating PDX objects.....Success\n");
    LOG("STEP putCharTypes complete.\n");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, getCharTypes)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    LOG("Trying to GET PDX objects.....\n");
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::CharTypes::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    auto localPdxptr = std::make_shared<PdxTests::CharTypes>();

    auto keyport = CacheableKey::create(1);
    LOG("Client-2 PdxTests::CharTypes GET OP Start....");
    auto remotePdxptr =
        std::dynamic_pointer_cast<PdxTests::CharTypes>(regPtr0->get(keyport));
    LOG("Client-2 PdxTests::CharTypes GET OP Done....");

    PdxTests::CharTypes *localPdx = localPdxptr.get();
    PdxTests::CharTypes *remotePdx =
        dynamic_cast<PdxTests::CharTypes *>(remotePdxptr.get());

    LOG_INFO("testThinClientPdxTests:StepFour before equal() check");
    ASSERT(remotePdx->equals(*localPdx) == true,
           "PdxTests::PdxTypes should be equal.");

    LOG_INFO("testThinClientPdxTests:StepFour equal check done successfully");

    // LOG_INFO("GET OP Result: Char Val=%c", remotePdx->getChar());
    // LOG_INFO("NIL GET OP Result: Char[0] val=%c",
    // remotePdx->getCharArray()[0]);
    // LOG_INFO("NIL GET OP Result: Char[1] val=%c",
    // remotePdx->getCharArray()[1]);

    LOG("STEP: getCharTypes complete.\n");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    // QueryHelper * qh = &QueryHelper::getHelper();
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxType::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          Address::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    LOG("PdxClassV1 Registered Successfully....");

    LOG("Trying to populate PDX objects.....\n");
    auto pdxobj = std::make_shared<PdxTests::PdxType>();
    auto keyport = CacheableKey::create(1);

    // PUT Operation
    regPtr0->put(keyport, pdxobj);
    LOG("PdxTests::PdxType: PUT Done successfully....");

    // PUT CacheableObjectArray as a Value
    auto keyport2 = CacheableKey::create(2);
    std::shared_ptr<CacheableObjectArray> m_objectArray;

    m_objectArray = CacheableObjectArray::create();
    m_objectArray->push_back(
        std::shared_ptr<Address>(new Address(1, "street0", "city0")));
    m_objectArray->push_back(
        std::shared_ptr<Address>(new Address(2, "street1", "city1")));
    m_objectArray->push_back(
        std::shared_ptr<Address>(new Address(3, "street2", "city2")));
    m_objectArray->push_back(
        std::shared_ptr<Address>(new Address(4, "street3", "city3")));
    m_objectArray->push_back(
        std::shared_ptr<Address>(new Address(5, "street4", "city4")));
    m_objectArray->push_back(
        std::shared_ptr<Address>(new Address(6, "street5", "city5")));
    m_objectArray->push_back(
        std::shared_ptr<Address>(new Address(7, "street6", "city6")));
    m_objectArray->push_back(
        std::shared_ptr<Address>(new Address(8, "street7", "city7")));
    m_objectArray->push_back(
        std::shared_ptr<Address>(new Address(9, "street8", "city8")));
    m_objectArray->push_back(
        std::shared_ptr<Address>(new Address(10, "street9", "city9")));

    // PUT Operation
    regPtr0->put(keyport2, m_objectArray);

    // locally destroy PdxTests::PdxType
    regPtr0->localDestroy(keyport);
    regPtr0->localDestroy(keyport2);

    LOG("localDestroy() operation....Done");

    // This is merely for asserting statistics
    regPtr0->get(keyport);
    regPtr0->get(keyport2);

    LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(regPtr0.get()));
    LOG_INFO(
        "PdxSerializations = %d ",
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
    LOG_INFO(
        "PdxDeSerializations = %d ",
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxDeSerializations());
    LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                 ->getCachePerfStats()
                                                 .getPdxSerializationBytes());
    LOG_INFO("PdxDeSerializationBytes = %ld ",
             lregPtr->getCacheImpl()
                 ->getCachePerfStats()
                 .getPdxDeSerializationBytes());
    ASSERT(
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations() ==
            lregPtr->getCacheImpl()
                ->getCachePerfStats()
                .getPdxDeSerializations(),
        "Total pdxDeserializations should be equal to Total "
        "pdxSerializations.");
    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxSerializationBytes() ==
               lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxDeSerializationBytes(),
           "Total pdxDeserializationBytes should be equal to Total "
           "pdxSerializationsBytes.");

    // Now update new keys with updated stats values, so that other client can
    // verify these values with its stats.
    auto keyport3 = CacheableKey::create(3);
    auto keyport4 = CacheableKey::create(4);
    regPtr0->put(keyport3,
                 CacheableInt32::create(lregPtr->getCacheImpl()
                                            ->getCachePerfStats()
                                            .getPdxDeSerializations()));
    regPtr0->put(keyport4,
                 CacheableInt64::create(lregPtr->getCacheImpl()
                                            ->getCachePerfStats()
                                            .getPdxDeSerializationBytes()));

    LOG("Done populating PDX objects.....Success\n");
    LOG("StepThree complete.\n");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepFour)
  {
    // initClient(true);
    auto regPtr0 = getHelper()->getRegion("DistRegionAck");

    // QueryHelper * qh = &QueryHelper::getHelper();
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();

    LOG("Trying to GET PDX objects.....\n");
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxType::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          Address::createDeserializable);
    } catch (const IllegalStateException &) {
      // ignore exception
    }

    // Create local CacheableObjectArray
    std::shared_ptr<CacheableObjectArray> m_localObjectArray;
    m_localObjectArray = CacheableObjectArray::create();
    m_localObjectArray->push_back(
        std::shared_ptr<Address>(new Address(1, "street0", "city0")));
    m_localObjectArray->push_back(
        std::shared_ptr<Address>(new Address(2, "street1", "city1")));
    m_localObjectArray->push_back(
        std::shared_ptr<Address>(new Address(3, "street2", "city2")));
    m_localObjectArray->push_back(
        std::shared_ptr<Address>(new Address(4, "street3", "city3")));
    m_localObjectArray->push_back(
        std::shared_ptr<Address>(new Address(5, "street4", "city4")));
    m_localObjectArray->push_back(
        std::shared_ptr<Address>(new Address(6, "street5", "city5")));
    m_localObjectArray->push_back(
        std::shared_ptr<Address>(new Address(7, "street6", "city6")));
    m_localObjectArray->push_back(
        std::shared_ptr<Address>(new Address(8, "street7", "city7")));
    m_localObjectArray->push_back(
        std::shared_ptr<Address>(new Address(9, "street8", "city8")));
    m_localObjectArray->push_back(
        std::shared_ptr<Address>(new Address(10, "street9", "city9")));

    // Get remote CacheableObjectArray on key 2
    auto keyport2 = CacheableKey::create(2);
    LOG_INFO("Client-2 PdxTests::PdxType GET OP Start....");
    auto remoteCObjArray =
        std::dynamic_pointer_cast<CacheableObjectArray>(regPtr0->get(keyport2));

    LOG_INFO(
        "Client-2 PdxTests::PdxType GET OP Done.. Received CObjeArray Size = "
        "%d",
        remoteCObjArray->size());
    ASSERT(
        remoteCObjArray->size() == 10,
        "PdxTests StepFour: CacheableObjectArray Size should be equal to 10");

    // Compare local vs remote CacheableObjectArray elements.
    bool isEqual = true;
    for (size_t i = 0; i < remoteCObjArray->size(); i++) {
      auto rAddr1 = std::dynamic_pointer_cast<Address>(remoteCObjArray->at(i));
      auto lAddr1 =
          std::dynamic_pointer_cast<Address>(m_localObjectArray->at(i));
      LOG_INFO("Remote Address:: %d th element  AptNum=%d  street=%s  city=%s ",
               i, rAddr1->getAptNum(), rAddr1->getStreet().c_str(),
               rAddr1->getCity().c_str());
      if (!rAddr1->equals(*lAddr1)) {
        isEqual = false;
        break;
      }
    }
    ASSERT(isEqual == true,
           "PdxTests StepFour: CacheableObjectArray elements are not matched");

    auto localPdxptr = std::make_shared<PdxTests::PdxType>();

    auto keyport = CacheableKey::create(1);
    LOG("Client-2 PdxTests::PdxType GET OP Start....");
    auto remotePdxptr =
        std::dynamic_pointer_cast<PdxTests::PdxType>(regPtr0->get(keyport));
    LOG("Client-2 PdxTests::PdxType GET OP Done....");

    //
    PdxTests::PdxType *localPdx = localPdxptr.get();
    PdxTests::PdxType *remotePdx =
        dynamic_cast<PdxTests::PdxType *>(remotePdxptr.get());

    // ToDo open this equals check
    LOG_INFO("testThinClientPdxTests:StepFour before equal() check");
    ASSERT(remotePdx->equals(*localPdx, false) == true,
           "PdxTests::PdxTypes should be equal.");
    LOG_INFO("testThinClientPdxTests:StepFour equal check done successfully");
    LOG_INFO("GET OP Result: Char Val={}",
             static_cast<int32_t>(remotePdx->getChar()));
    LOG_INFO("NIL GET OP Result: Char[0] val={}",
             static_cast<int32_t>(remotePdx->getCharArray()[0]));
    LOG_INFO("NIL GET OP Result: Char[1] val={}",
             static_cast<int32_t>(remotePdx->getCharArray()[1]));
    LOG_INFO("GET OP Result: Array of byte arrays [0]={:#04x}",
             remotePdx->getArrayOfByteArrays()[0][0]);
    LOG_INFO("GET OP Result: Array of byte arrays [1]={:#04x}",
             remotePdx->getArrayOfByteArrays()[1][0]);
    LOG_INFO("GET OP Result: Array of byte arrays [2]={:#04x}",
             remotePdx->getArrayOfByteArrays()[1][1]);

    CacheableInt32 *element =
        dynamic_cast<CacheableInt32 *>(remotePdx->getArrayList()->at(0).get());
    LOG_INFO("GET OP Result_1233: Array List element Value =%d",
             element->value());

    for (const auto &iter : *(remotePdx->getHashTable())) {
      const auto remoteKey =
          std::dynamic_pointer_cast<CacheableInt32>(iter.first);
      const auto remoteVal =
          std::dynamic_pointer_cast<CacheableString>(iter.second);
      LOG_INFO("HashTable Key Val = %d", remoteKey->value());
      LOG_INFO("HashTable Val = %s", remoteVal->value().c_str());
      //(*iter1).first.value();
      // output.writeObject( *iter );
    }

    // Now get values for key3 and 4 to asset against stats of this client
    const auto lregPtr = std::dynamic_pointer_cast<LocalRegion>(regPtr0);
    LOG_INFO(
        "PdxSerializations = %d ",
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxSerializations());
    LOG_INFO(
        "PdxDeSerializations = %d ",
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxDeSerializations());
    LOG_INFO("PdxSerializationBytes = %ld ", lregPtr->getCacheImpl()
                                                 ->getCachePerfStats()
                                                 .getPdxSerializationBytes());
    LOG_INFO("PdxDeSerializationBytes = %ld ",
             lregPtr->getCacheImpl()
                 ->getCachePerfStats()
                 .getPdxDeSerializationBytes());

    auto keyport3 = CacheableKey::create(3);
    auto keyport4 = CacheableKey::create(4);
    auto int32Ptr =
        std::dynamic_pointer_cast<CacheableInt32>(regPtr0->get(keyport3));
    auto int64Ptr =
        std::dynamic_pointer_cast<CacheableInt64>(regPtr0->get(keyport4));
    ASSERT(int32Ptr->value() == lregPtr->getCacheImpl()
                                    ->getCachePerfStats()
                                    .getPdxDeSerializations(),
           "Total pdxDeserializations should be equal to Total "
           "pdxSerializations.");
    ASSERT(int64Ptr->value() == lregPtr->getCacheImpl()
                                    ->getCachePerfStats()
                                    .getPdxDeSerializationBytes(),
           "Total pdxDeserializationBytes should be equal to Total "
           "pdxSerializationsBytes.");

    // LOG_INFO("GET OP Result: IntVal1=%d", obj2->getInt1());
    // LOG_INFO("GET OP Result: IntVal2=%d", obj2->getInt2());
    // LOG_INFO("GET OP Result: IntVal3=%d", obj2->getInt3());
    // LOG_INFO("GET OP Result: IntVal4=%d", obj2->getInt4());
    // LOG_INFO("GET OP Result: IntVal5=%d", obj2->getInt5());
    // LOG_INFO("GET OP Result: IntVal6=%d", obj2->getInt6());

    // LOG_INFO("GET OP Result: BoolVal=%d", obj2->getBool());
    // LOG_INFO("GET OP Result: ByteVal=%d", obj2->getByte());
    // LOG_INFO("GET OP Result: ShortVal=%d", obj2->getShort());

    // LOG_INFO("GET OP Result: IntVal=%d", obj2->getInt());

    // LOG_INFO("GET OP Result: LongVal=%ld", obj2->getLong());
    // LOG_INFO("GET OP Result: FloatVal=%f", obj2->getFloat());
    // LOG_INFO("GET OP Result: DoubleVal=%lf", obj2->getDouble());
    // LOG_INFO("GET OP Result: StringVal=%s", obj2->getString());
    // LOG_INFO("GET OP Result: BoolArray[0]=%d", obj2->getBoolArray()[0]);
    // LOG_INFO("GET OP Result: BoolArray[1]=%d", obj2->getBoolArray()[1]);
    // LOG_INFO("GET OP Result: BoolArray[2]=%d", obj2->getBoolArray()[2]);

    // LOG_INFO("GET OP Result: ByteArray[0]=%d", obj2->getByteArray()[0]);
    // LOG_INFO("GET OP Result: ByteArray[1]=%d", obj2->getByteArray()[1]);

    // LOG_INFO("GET OP Result: ShortArray[0]=%d", obj2->getShortArray()[0]);
    // LOG_INFO("GET OP Result: IntArray[0]=%d", obj2->getIntArray()[0]);
    // LOG_INFO("GET OP Result: LongArray[1]=%lld", obj2->getLongArray()[1]);
    // LOG_INFO("GET OP Result: FloatArray[0]=%f", obj2->getFloatArray()[0]);
    // LOG_INFO("GET OP Result: DoubleArray[1]=%lf", obj2->getDoubleArray()[1]);

    LOG("Done Getting PDX objects.....Success\n");

    LOG("StepFour complete.\n");
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

DUNIT_TASK_DEFINITION(CLIENT3, CloseCache3)
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

DUNIT_TASK_DEFINITION(CLIENT1, SetWeakHashMapToTrueC1)
  {
    m_useWeakHashMap = true;
    PdxTests::PdxTypesIgnoreUnreadFieldsV1::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, SetWeakHashMapToTrueC2)
  {
    m_useWeakHashMap = true;
    PdxTests::PdxTypesIgnoreUnreadFieldsV2::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, setWeakHashMapToFlaseC1)
  {
    m_useWeakHashMap = false;
    PdxTests::PdxTypesIgnoreUnreadFieldsV1::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, SetWeakHashMapToFalseC2)
  {
    m_useWeakHashMap = false;
    PdxTests::PdxTypesIgnoreUnreadFieldsV2::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION
///
DUNIT_TASK_DEFINITION(CLIENT1, SetWeakHashMapToTrueC1BM)
  {
    m_useWeakHashMap = true;
    PdxTests::PdxType1V1::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, SetWeakHashMapToTrueC2BM)
  {
    m_useWeakHashMap = true;
    PdxTests::PdxTypes1V2::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, setWeakHashMapToFlaseC1BM)
  {
    m_useWeakHashMap = false;
    PdxTests::PdxType1V1::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, SetWeakHashMapToFalseC2BM)
  {
    m_useWeakHashMap = false;
    PdxTests::PdxTypes1V2::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION
///
DUNIT_TASK_DEFINITION(CLIENT1, SetWeakHashMapToTrueC1BM2)
  {
    m_useWeakHashMap = true;
    PdxTests::PdxType2V1::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, SetWeakHashMapToTrueC2BM2)
  {
    m_useWeakHashMap = true;
    PdxTests::PdxTypes2V2::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, setWeakHashMapToFlaseC1BM2)
  {
    m_useWeakHashMap = false;
    PdxTests::PdxType2V1::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, SetWeakHashMapToFalseC2BM2)
  {
    m_useWeakHashMap = false;
    PdxTests::PdxTypes2V2::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION
///
DUNIT_TASK_DEFINITION(CLIENT1, SetWeakHashMapToTrueC1BM3)
  {
    m_useWeakHashMap = true;
    PdxTests::PdxType3V1::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, SetWeakHashMapToTrueC2BM3)
  {
    m_useWeakHashMap = true;
    PdxTests::PdxTypes3V2::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, setWeakHashMapToFlaseC1BM3)
  {
    m_useWeakHashMap = false;
    PdxTests::PdxType3V1::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, SetWeakHashMapToFalseC2BM3)
  {
    m_useWeakHashMap = false;
    PdxTests::PdxTypes3V2::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION
///
DUNIT_TASK_DEFINITION(CLIENT1, SetWeakHashMapToTrueC1BMR1)
  {
    m_useWeakHashMap = true;
    PdxTests::PdxTypesV1R1::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, SetWeakHashMapToTrueC2BMR1)
  {
    m_useWeakHashMap = true;
    PdxTests::PdxTypesR1V2::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, setWeakHashMapToFlaseC1BMR1)
  {
    m_useWeakHashMap = false;
    PdxTests::PdxTypesV1R1::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, SetWeakHashMapToFalseC2BMR1)
  {
    m_useWeakHashMap = false;
    PdxTests::PdxTypesR1V2::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION
///
DUNIT_TASK_DEFINITION(CLIENT1, SetWeakHashMapToTrueC1BMR2)
  {
    m_useWeakHashMap = true;
    PdxTests::PdxTypesV1R2::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, SetWeakHashMapToTrueC2BMR2)
  {
    m_useWeakHashMap = true;
    PdxTests::PdxTypesR2V2::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, setWeakHashMapToFlaseC1BMR2)
  {
    m_useWeakHashMap = false;
    PdxTests::PdxTypesV1R2::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, SetWeakHashMapToFalseC2BMR2)
  {
    m_useWeakHashMap = false;
    PdxTests::PdxTypesR2V2::reset(m_useWeakHashMap);
  }
END_TASK_DEFINITION
///

void runPdxLongRunningClientTest() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator);
  CALL_TASK(StepOnePoolLocSysConfig);
  CALL_TASK(StepTwoPoolLocSysConfig);

  // StepThree: Put some portfolio/Position objects
  CALL_TASK(Puts2);

  // now close server
  CALL_TASK(CloseServer);

  CALL_TASK(forCleanup);
  // now start server
  CALL_TASK(CreateServerWithLocator);

  // do put again
  CALL_TASK(Puts22);

  CALL_TASK(Get2);

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer);

  CALL_TASK(CloseLocator);
}

void runPdxDistOps() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator);
  CALL_TASK(StepOnePoolLoc);
  CALL_TASK(StepTwoPoolLoc);

  // StepThree: Put some portfolio/Position objects
  CALL_TASK(PutAndVerifyPdxInGet);
  CALL_TASK(VerifyGetOnly);
  CALL_TASK(PutAndVerifyVariousPdxTypes);
  CALL_TASK(VerifyVariousPdxGets);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer);

  CALL_TASK(CloseLocator);
}

void runPdxTestForCharTypes() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator);
  CALL_TASK(StepOnePoolLoc);
  CALL_TASK(StepTwoPoolLoc);

  // StepThree: Put some portfolio/Position objects
  CALL_TASK(putCharTypes);
  CALL_TASK(getCharTypes);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer);

  CALL_TASK(CloseLocator);
}

void testBug866() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator);
  CALL_TASK(StepOnePoolLocBug866);
  CALL_TASK(StepTwoPoolLocBug866);

  // StepThree: Put some portfolio/Position objects
  CALL_TASK(putOperation);
  CALL_TASK(getOperation);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer);

  CALL_TASK(CloseLocator);
}

void runPdxPutGetTest() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator);
  CALL_TASK(StepOnePoolLoc);
  CALL_TASK(StepTwoPoolLoc);

  // StepThree: Put some portfolio/Position objects
  CALL_TASK(StepThree);
  CALL_TASK(StepFour);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer);

  CALL_TASK(CloseLocator);
}

void runBasicMergeOpsR2() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator1);
  CALL_TASK(StepOnePoolLoc);
  CALL_TASK(StepTwoPoolLoc);

  CALL_TASK(putAtVersionTwoR21);

  CALL_TASK(getPutAtVersionOneR22);

  for (int i = 0; i < 10; i++) {
    CALL_TASK(getPutAtVersionTwoR23);
    CALL_TASK(getPutAtVersionOneR24);
  }

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer);

  CALL_TASK(CloseLocator);
}

void runBasicMergeOpsR1() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator1);
  CALL_TASK(StepOnePoolLoc);
  CALL_TASK(StepTwoPoolLoc);

  CALL_TASK(putAtVersionTwo1);

  CALL_TASK(getPutAtVersionOne2);

  CALL_TASK(getPutAtVersionTwo3);

  CALL_TASK(getPutAtVersionOne4);

  for (int i = 0; i < 10; i++) {
    CALL_TASK(getPutAtVersionTwo5);
    CALL_TASK(getPutAtVersionOne6);
  }

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer);

  CALL_TASK(CloseLocator);
}

void runBasicMergeOps() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator1);
  CALL_TASK(StepOnePoolLoc);
  CALL_TASK(StepTwoPoolLoc);

  CALL_TASK(putAtVersionOne11);

  CALL_TASK(getPutAtVersionTwo12);

  CALL_TASK(getPutAtVersionOne13);

  CALL_TASK(getPutAtVersionTwo14);

  for (int i = 0; i < 10; i++) {
    CALL_TASK(getPutAtVersionOne15);
    CALL_TASK(getPutAtVersionTwo16);
  }
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer);

  CALL_TASK(CloseLocator);
}

void runBasicMergeOps2() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator1);
  CALL_TASK(StepOnePoolLoc);
  CALL_TASK(StepTwoPoolLoc);

  CALL_TASK(putAtVersionOne21);

  CALL_TASK(getPutAtVersionTwo22);

  for (int i = 0; i < 10; i++) {
    CALL_TASK(getPutAtVersionOne23);
    CALL_TASK(getPutAtVersionTwo24);
  }
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer);

  CALL_TASK(CloseLocator);
}

void runBasicMergeOps3() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator1);
  CALL_TASK(StepOnePoolLoc);
  CALL_TASK(StepTwoPoolLoc);

  CALL_TASK(putAtVersionOne31);

  CALL_TASK(getPutAtVersionTwo32);

  for (int i = 0; i < 10; i++) {
    CALL_TASK(getPutAtVersionOne33);
    CALL_TASK(getPutAtVersionTwo34);
  }
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer);

  CALL_TASK(CloseLocator);
}

void runJavaInteroperableOps() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator2);
  CALL_TASK(StepOnePoolLoc);
  CALL_TASK(StepTwoPoolLoc);

  CALL_TASK(JavaPutGet);  // c1
  CALL_TASK(JavaGet);     // c2

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer);

  CALL_TASK(CloseLocator);
}

// runJavaInterOpsUsingLinkedList
void runJavaInterOpsUsingLinkedList() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator2);
  CALL_TASK(StepOnePoolLoc1);
  CALL_TASK(StepTwoPoolLoc1);

  CALL_TASK(JavaPutGet1);  // c1

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer);

  CALL_TASK(CloseLocator);
}

// test case that checks for Invalid Usage and corr. IllegalStatException for
// PDXReader And PDXWriter APIs.
void _disable_see_bug_999_testReaderWriterInvalidUsage() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator2);
  CALL_TASK(StepOnePoolLoc);
  CALL_TASK(StepTwoPoolLoc);

  CALL_TASK(testPdxWriterAPIsWithInvalidArgs);
  CALL_TASK(testPdxReaderAPIsWithInvalidArgs);

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer);

  CALL_TASK(CloseLocator);
}

//
void testPolymorphicUseCase() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator2);
  CALL_TASK(StepOnePoolLoc);
  CALL_TASK(StepTwoPoolLoc);

  CALL_TASK(testPutWithMultilevelInheritance);
  CALL_TASK(testGetWithMultilevelInheritance);

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer);

  CALL_TASK(CloseLocator);
}

void runNestedPdxOps() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator1);
  CALL_TASK(StepOnePoolLoc);
  CALL_TASK(StepTwoPoolLoc);

  CALL_TASK(PutAndVerifyNestedPdxInGet);

  CALL_TASK(VerifyNestedGetOnly);

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer);

  CALL_TASK(CloseLocator);
}

void runNestedPdxOpsWithVersioning() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator1);
  CALL_TASK(StepOnePoolLoc);
  CALL_TASK(StepTwoPoolLoc);

  CALL_TASK(PutMixedVersionNestedPdx);

  CALL_TASK(VerifyMixedVersionNestedGetOnly);

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer);

  CALL_TASK(CloseLocator);
}

void runPdxInGFSOps() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator1);
  CALL_TASK(StepOnePoolLoc);
  CALL_TASK(StepTwoPoolLoc);

  CALL_TASK(PutAndVerifyPdxInGFSInGet);

  CALL_TASK(VerifyPdxInGFSGetOnly);

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer);

  CALL_TASK(CloseLocator);
}

void runPdxIgnoreUnreadFieldTest() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator1);
  CALL_TASK(StepOnePoolLoc_PDX);
  CALL_TASK(StepTwoPoolLoc_PDX);

  CALL_TASK(putV2PdxUI);

  CALL_TASK(putV1PdxUI);

  CALL_TASK(getV2PdxUI);

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer);

  CALL_TASK(CloseLocator);
}

// runPdxBankTest
void runPdxBankTest() {
  CALL_TASK(StartLocator);
  CALL_TASK(CreateServerWithLocator_PdxMetadataTest);
  CALL_TASK(StepOnePoolLoc_PdxMetadataTest);
  CALL_TASK(StepTwoPoolLoc_PdxMetadataTest);
  CALL_TASK(StepThreePoolLoc_PdxMetadataTest);

  CALL_TASK(client1PutsV1Object);  // c1

  CALL_TASK(client2GetsV1ObjectAndPutsV2Object);  // c2

  CALL_TASK(client3GetsV2Object);  // c3

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseCache3);  //

  CALL_TASK(CloseServer);

  CALL_TASK(CloseLocator);
}

void enableWeakHashMapC1() { CALL_TASK(SetWeakHashMapToTrueC1); }
void enableWeakHashMapC2() { CALL_TASK(SetWeakHashMapToTrueC2); }

void disableWeakHashMapC1() { CALL_TASK(setWeakHashMapToFlaseC1); }
void disableWeakHashMapC2() { CALL_TASK(SetWeakHashMapToFalseC2); }
/////
void enableWeakHashMapC1BM() { CALL_TASK(SetWeakHashMapToTrueC1BM); }
void enableWeakHashMapC2BM() { CALL_TASK(SetWeakHashMapToTrueC2BM); }

void disableWeakHashMapC1BM() { CALL_TASK(setWeakHashMapToFlaseC1BM); }
void disableWeakHashMapC2BM() { CALL_TASK(SetWeakHashMapToFalseC2BM); }
////
void enableWeakHashMapC1BM2() { CALL_TASK(SetWeakHashMapToTrueC1BM2); }
void enableWeakHashMapC2BM2() { CALL_TASK(SetWeakHashMapToTrueC2BM2); }

void disableWeakHashMapC1BM2() { CALL_TASK(setWeakHashMapToFlaseC1BM2); }
void disableWeakHashMapC2BM2() { CALL_TASK(SetWeakHashMapToFalseC2BM2); }
////
void enableWeakHashMapC1BM3() { CALL_TASK(SetWeakHashMapToTrueC1BM3); }
void enableWeakHashMapC2BM3() { CALL_TASK(SetWeakHashMapToTrueC2BM3); }

void disableWeakHashMapC1BM3() { CALL_TASK(setWeakHashMapToFlaseC1BM3); }
void disableWeakHashMapC2BM3() { CALL_TASK(SetWeakHashMapToFalseC2BM3); }
/////
void enableWeakHashMapC1BMR1() { CALL_TASK(SetWeakHashMapToTrueC1BMR1); }
void enableWeakHashMapC2BMR1() { CALL_TASK(SetWeakHashMapToTrueC2BMR1); }

void disableWeakHashMapC1BMR1() { CALL_TASK(setWeakHashMapToFlaseC1BMR1); }
void disableWeakHashMapC2BMR1() { CALL_TASK(SetWeakHashMapToFalseC2BMR1); }
///////
void enableWeakHashMapC1BMR2() { CALL_TASK(SetWeakHashMapToTrueC1BMR2); }
void enableWeakHashMapC2BMR2() { CALL_TASK(SetWeakHashMapToTrueC2BMR2); }

void disableWeakHashMapC1BMR2() { CALL_TASK(setWeakHashMapToFlaseC1BMR2); }
void disableWeakHashMapC2BMR2() { CALL_TASK(SetWeakHashMapToFalseC2BMR2); }

DUNIT_MAIN
  {
    { runPdxLongRunningClientTest(); }
    // NON PDX UnitTest for Ticket#866 on NC OR SR#13306117704. Set client name
    // via native client API
    testBug866();

    runPdxTestForCharTypes();

    // PUT-GET Test with values of type CacheableObjectArray and PdxType object
    runPdxPutGetTest();

    // PdxDistOps-PdxTests::PdxType PUT/GET Test across clients
    { runPdxDistOps(); }

    // BasicMergeOps
    {
      enableWeakHashMapC1BM();
      enableWeakHashMapC2BM();
      runBasicMergeOps();
    }

    // BasicMergeOps2
    {
      enableWeakHashMapC1BM2();
      enableWeakHashMapC2BM2();
      runBasicMergeOps2();
    }

    // BasicMergeOps3
    {
      enableWeakHashMapC1BM3();
      enableWeakHashMapC2BM3();
      runBasicMergeOps3();
    }

    // BasicMergeOpsR1
    {
      enableWeakHashMapC1BMR1();
      enableWeakHashMapC2BMR1();
      runBasicMergeOpsR1();
    }

    // BasicMergeOpsR2
    {
      enableWeakHashMapC1BMR2();
      enableWeakHashMapC2BMR2();
      runBasicMergeOpsR2();
    }

    // JavaInteroperableOps
    { runJavaInteroperableOps(); }

    // PDXReaderWriterInvalidUsage
    {
        // disable see bug 999 for more details.
        // testReaderWriterInvalidUsage();
    }

    // Test LinkedList
    {
      runJavaInterOpsUsingLinkedList();
    }

    // NestedPdxOps
    { runNestedPdxOps(); }

    // MixedVersionNestedPdxOps
    { runNestedPdxOpsWithVersioning(); }

    // Pdxobject In Geode Serializable Ops
    //{
    //  runPdxInGFSOps();
    //}

    {
      enableWeakHashMapC1();
      enableWeakHashMapC2();
      runPdxIgnoreUnreadFieldTest();
    }

    // PdxBankTest
    { runPdxBankTest(); }

    // Polymorphic-multilevel inheritance
    { testPolymorphicUseCase(); }
  }
END_MAIN
