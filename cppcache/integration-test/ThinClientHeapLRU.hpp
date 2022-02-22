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

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTHEAPLRU_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTHEAPLRU_H_

#include "fw_dunit.hpp"
#include "BuiltinCacheableWrappers.hpp"

#include <string>

#define ROOT_NAME "ThinClientHeapLRU"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1

CacheHelper* cacheHelper = nullptr;
static bool isLocator = false;
static bool isLocalServer = false;
static int numberOfLocators = 0;
const char* poolName = "__TESTPOOL1_";
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);

const char* keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char* vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char* nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const char* regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

const bool USE_ACK = true;
const bool NO_ACK = false;

#include "LocatorHelper.hpp"

void initThinClientWithClientTypeAsCLIENT(const bool isthinClient) {
  if (cacheHelper == nullptr) {
    auto pp = Properties::create();
    pp->insert("heap-lru-limit", 1);
    pp->insert("heap-lru-delta", 10);

    cacheHelper = new CacheHelper(isthinClient, pp);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

// void initClient( const bool isthinClient )
//{
//  if ( cacheHelper == nullptr ) {
//    cacheHelper = new CacheHelper(isthinClient);
//  }
//  ASSERT( cacheHelper, "Failed to create a CacheHelper client instance." );
//}
void cleanProc() {
  if (cacheHelper != nullptr) {
    delete cacheHelper;
    cacheHelper = nullptr;
  }
}

CacheHelper* getHelper() {
  ASSERT(cacheHelper != nullptr, "No cacheHelper initialized.");
  return cacheHelper;
}

void createRegion(const char* name, bool ackMode, const char* endpoints,
                  bool clientNotificationEnabled = false) {
  LOG("createRegion() entered.");
  std::cout << "Creating region --  " << name << " ackMode is " << ackMode
            << "\n"
            << std::flush;
  auto regPtr = getHelper()->createRegion(name, ackMode, true, nullptr,
                                          endpoints, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}

void createPooledRegion(const char* name, bool ackMode, const char* locators,
                        const char* poolname,
                        bool clientNotificationEnabled = false,
                        bool cachingEnable = true) {
  LOG("createRegion_Pool() entered.");
  std::cout << "Creating region --  " << name << " ackMode is " << ackMode
            << "\n"
            << std::flush;
  auto regPtr =
      getHelper()->createPooledRegion(name, ackMode, locators, poolname,
                                      cachingEnable, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Pooled Region created.");
}

void createOnekEntries() {
  CacheableHelper::registerBuiltins();
  auto dataReg = getHelper()->getRegion(regionNames[0]);
  for (int i = 0; i < 2048; i++) {
    CacheableWrapper* tmpkey =
        CacheableWrapperFactory::createInstance(GeodeTypeIds::CacheableInt32);
    CacheableWrapper* tmpval =
        CacheableWrapperFactory::createInstance(GeodeTypeIds::CacheableBytes);
    tmpkey->initKey(i, 32);
    tmpval->initRandomValue(1024);
    ASSERT(tmpkey->getCacheable() != nullptr,
           "tmpkey->getCacheable() is nullptr");
    ASSERT(tmpval->getCacheable() != nullptr,
           "tmpval->getCacheable() is nullptr");
    dataReg->put(
        std::dynamic_pointer_cast<CacheableKey>(tmpkey->getCacheable()),
        tmpval->getCacheable());
    // delete tmpkey;
    //  delete tmpval;
  }
  dunit::sleep(10000);
  std::vector<std::shared_ptr<RegionEntry>> me;
  dataReg->entries(me, false);
  LOG("Verifying size outside loop");
  LOG(std::string("region size is ") + std::to_string(me.size()));

  ASSERT(me.size() <= 1024, "Should have evicted anything over 1024 entries");
}

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    if (isLocalServer) CacheHelper::initServer(1);
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne)
  {
    initThinClientWithClientTypeAsCLIENT(true);
    createRegion(regionNames[0], USE_ACK, endPoints);
    createRegion(regionNames[1], NO_ACK, endPoints);
    LOG("StepOne complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pooled_Locator)
  {
    initThinClientWithClientTypeAsCLIENT(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, poolName);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, poolName);
    LOG("StepOne_Pooled_Locators complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pooled_EndPoint)
  {
    initThinClientWithClientTypeAsCLIENT(true);
    createPooledRegion(regionNames[0], USE_ACK, endPoints, nullptr, poolName);
    createPooledRegion(regionNames[1], NO_ACK, endPoints, nullptr, poolName);
    LOG("StepOne complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo)
  {
    initThinClientWithClientTypeAsCLIENT(true);
    createRegion(regionNames[0], USE_ACK, endPoints);
    createRegion(regionNames[1], NO_ACK, endPoints);
    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo_Pooled_Locator)
  {
    initThinClientWithClientTypeAsCLIENT(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, poolName);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, poolName);
    LOG("StepTwo_Pooled_Locator complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo_Pooled_EndPoint)
  {
    initThinClientWithClientTypeAsCLIENT(true);
    createPooledRegion(regionNames[0], USE_ACK, endPoints, nullptr, poolName);
    createPooledRegion(regionNames[1], NO_ACK, endPoints, nullptr, poolName);
    LOG("StepTwo_Pooled_EndPoint complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    // verfy that eviction works
    createOnekEntries();
    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CloseCache2)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer1)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
  }
END_TASK_DEFINITION

void runHeapLRU(bool poolConfig = true, bool isLocator = true) {
  if (poolConfig && isLocator) {
    CALL_TASK(CreateLocator1);
    CALL_TASK(CreateServer1_With_Locator);
  } else {
    CALL_TASK(CreateServer1);
  }
  if (!poolConfig) {
    CALL_TASK(StepOne);
    CALL_TASK(StepTwo);
  } else if (isLocator) {
    CALL_TASK(StepOne_Pooled_Locator);
    CALL_TASK(StepTwo_Pooled_EndPoint);
  } else {
    CALL_TASK(StepOne_Pooled_EndPoint);
    CALL_TASK(StepTwo_Pooled_EndPoint);
  }
  CALL_TASK(StepThree);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer1);
  if (poolConfig && isLocator) {
    CALL_TASK(CloseLocator1);
  }
}

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTHEAPLRU_H_
