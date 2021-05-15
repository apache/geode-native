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
#define LOCATOR s2p2
#define SERVER1 s2p1

using apache::geode::client::CacheableBoolean;
using apache::geode::client::CacheableVector;
using apache::geode::client::ClassCastException;
using apache::geode::client::FunctionService;
using apache::geode::client::IllegalStateException;
using apache::geode::client::UserFunctionExecutionException;

using PdxTests::PdxTypes1;
using PdxTests::PdxTypes10;
using PdxTests::PdxTypes2;
using PdxTests::PdxTypes3;
using PdxTests::PdxTypes4;
using PdxTests::PdxTypes5;
using PdxTests::PdxTypes6;
using PdxTests::PdxTypes7;
using PdxTests::PdxTypes8;
using PdxTests::PdxTypes9;

bool isLocator = false;
bool isLocalServer = false;

const char *poolNames[] = {"Pool1", "Pool2", "Pool3"};
const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
bool isPoolConfig = false;  // To track if pool case is running
// const char * qRegionNames[] = { "Portfolios", "Positions", "Portfolios2",
// "Portfolios3" };

void initClient(const bool isthinClient, bool isPdxIgnoreUnreadFields,
                const std::shared_ptr<Properties> &configPtr = nullptr) {
  LOG_INFO("isPdxIgnoreUnreadFields = %d ", isPdxIgnoreUnreadFields);
  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(isthinClient, isPdxIgnoreUnreadFields, false,
                                  configPtr, false);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

void initClient1(bool isPdxIgnoreUnreadFields = false) {
  // Create just one pool and attach all regions to that.
  initClient(true, isPdxIgnoreUnreadFields);
  isPoolConfig = true;
  createPool(poolNames[0], locHostPort, {}, 0, false);
  createRegionAndAttachPool("DistRegionAck", USE_ACK, poolNames[0],
                            true /*Caching enabled*/);
  LOG("StepOne complete.");
}

void initClient2(bool isPdxIgnoreUnreadFields = false) {
  // Create just one pool and attach all regions to that.
  initClient(true, isPdxIgnoreUnreadFields);
  isPoolConfig = true;
  createPool(poolNames[0], locHostPort, {}, 0,
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

DUNIT_TASK_DEFINITION(SERVER1, CreateServerWithLocator_PdxMetadataTest)
  {
    LOG("Starting SERVER1...");
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserverPdx2.xml", locHostPort);
    }
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOnePoolLoc_PdxMetadataTest)
  {
    LOG("Starting Step One with Pool + Locator lists");
    initClient1();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwoPoolLoc_PdxMetadataTest)
  {
    LOG("Starting Step Two with Pool + Locator");
    initClient2();
  }
END_TASK_DEFINITION

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

DUNIT_MAIN
  {
    CALL_TASK(StartLocator);
    CALL_TASK(CreateServerWithLocator_PdxMetadataTest);
    CALL_TASK(StepOnePoolLoc_PdxMetadataTest);
    CALL_TASK(StepTwoPoolLoc_PdxMetadataTest);

    CALL_TASK(generateJavaPdxType);

    CALL_TASK(putAllPdxTypes);

    CALL_TASK(verifyDotNetPdxTypes);

    CALL_TASK(CloseCache1);
    CALL_TASK(CloseCache2);
    CALL_TASK(CloseServer);

    CALL_TASK(CloseLocator);
  }
END_MAIN
