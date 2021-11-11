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

#define ROOT_NAME "testThinClientPutWithDelta"

#include "DeltaEx.hpp"
#include "fw_dunit.hpp"
#include <string>
#include "SerializationRegistry.hpp"
#include "CacheRegionHelper.hpp"
#include "CacheImpl.hpp"

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheHelper;
using apache::geode::client::CacheRegionHelper;
using apache::geode::client::IllegalStateException;

CacheHelper *cacheHelper = nullptr;
bool isLocalServer = false;

static bool isLocator = false;
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#include "LocatorHelper.hpp"

int DeltaEx::toDeltaCount = 0;
int DeltaEx::toDataCount = 0;
int DeltaEx::fromDeltaCount = 0;
int DeltaEx::fromDataCount = 0;
int DeltaEx::cloneCount = 0;

int PdxDeltaEx::m_toDeltaCount = 0;
int PdxDeltaEx::m_toDataCount = 0;
int PdxDeltaEx::m_fromDeltaCount = 0;
int PdxDeltaEx::m_fromDataCount = 0;
int PdxDeltaEx::m_cloneCount = 0;

void initClient(const bool isthinClient) {
  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(isthinClient);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

void initClientNoPools() {
  cacheHelper = new CacheHelper(0);
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

void cleanProc() {
  if (cacheHelper != nullptr) {
    delete cacheHelper;
    cacheHelper = nullptr;
  }
}

CacheHelper *getHelper() {
  ASSERT(cacheHelper != nullptr, "No cacheHelper initialized.");
  return cacheHelper;
}

void createPooledRegion(const std::string &name, bool ackMode,
                        const std::string &locators,
                        const std::string &poolname,
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

void createRegion(const char *name, bool ackMode, const char *endpoints,
                  bool clientNotificationEnabled = false) {
  LOG("createRegion() entered.");
  std::cout << "Creating region --  " << name << " ackMode is " << ackMode
            << "\n"
            << std::flush;
  // ack, caching
  auto regPtr = getHelper()->createRegion(name, ackMode, true, nullptr,
                                          endpoints, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}

const char *keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};

const char *regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

const bool USE_ACK = true;
const bool NO_ACK = false;

DUNIT_TASK_DEFINITION(CLIENT1, CreateClient1_UsePools)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TESTPOOL1_",
                       true);
    LOG("CreateRegions1_PoolLocators complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne)
  {
    LOG("Step one entered");
    try {
      auto serializationRegistry =
          CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
              ->getSerializationRegistry();
      serializationRegistry->addDataSerializableType(DeltaEx::create, 1);
    } catch (IllegalStateException &) {
      //  ignore exception caused by type reregistration.
    }
    DeltaEx::toDeltaCount = 0;
    DeltaEx::toDataCount = 0;

    auto keyPtr = CacheableKey::create(keys[0]);
    auto valPtr = std::make_shared<DeltaEx>();
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    regPtr->put(keyPtr, valPtr);
    valPtr->setDelta(true);
    regPtr->put(keyPtr, valPtr);

    // Test create with delta - toData() should be invoked instead of toDelta()
    regPtr->destroy(keyPtr);
    valPtr->setDelta(true);
    regPtr->create(keyPtr, valPtr);

    auto valPtr1 = std::make_shared<DeltaEx>(0);
    regPtr->put(keyPtr, valPtr1);
    valPtr1->setDelta(true);
    regPtr->put(keyPtr, valPtr1);
    ASSERT(DeltaEx::toDeltaCount == 2, " Delta count should have been 2 ");
    ASSERT(DeltaEx::toDataCount == 4, " Data count should have been 3 ");
    LOG("Step one exited");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_DisableDelta)
  {
    LOG("Step one (disable delta on server) entered");
    DeltaEx::toDeltaCount = 0;
    DeltaEx::toDataCount = 0;
    try {
      auto serializationRegistry =
          CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
              ->getSerializationRegistry();

      serializationRegistry->addDataSerializableType(DeltaEx::create, 1);
    } catch (IllegalStateException &) {
      //  Ignore the exception caused by re-registration of DeltaEx.
    }
    auto keyPtr = CacheableKey::create(keys[0]);
    auto valPtr = std::make_shared<DeltaEx>();
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    regPtr->put(keyPtr, valPtr);
    valPtr->setDelta(true);
    regPtr->put(keyPtr, valPtr);

    ASSERT(DeltaEx::toDeltaCount == 0, " Delta count should have been 0 ");
    ASSERT(DeltaEx::toDataCount == 2, " Data count should have been 2 ");
    LOG("Step one exited");
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

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1_ForDelta)
  {
    // starting servers
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_with_delta.xml", locatorsG);
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1_DisableDelta)
  {
    // starting servers
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_with_delta.xml", locatorsG,
                              nullptr, false, false);
    }
  }
END_TASK_DEFINITION

void doDeltaPut() {
  CALL_TASK(CreateServer1_ForDelta);

  CALL_TASK(CreateClient1_UsePools);

  CALL_TASK(StepOne);

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseServer1);

  CALL_TASK(CreateServer1_DisableDelta);

  CALL_TASK(CreateClient1_UsePools);

  CALL_TASK(StepOne_DisableDelta);

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseServer1);
}

DUNIT_MAIN
  {
    CALL_TASK(CreateLocator1);

    doDeltaPut();

    CALL_TASK(CloseLocator1);
  }
END_MAIN
