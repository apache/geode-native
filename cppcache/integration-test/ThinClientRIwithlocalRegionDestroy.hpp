#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTRIWITHLOCALREGIONDESTROY_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTRIWITHLOCALREGIONDESTROY_H_

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
#include <geode/RegionEvent.hpp>
#include <ace/OS.h>
#include <ace/High_Res_Timer.h>
#include <string>

#define ROOT_NAME "ThinClientRIwithlocalRegionDestroy"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"

namespace { // NOLINT(google-build-namespaces)

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;
using apache::geode::client::CacheListener;
using apache::geode::client::EntryEvent;
using apache::geode::client::Exception;
using apache::geode::client::Region;
using apache::geode::client::RegionEvent;

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1

CacheHelper* cacheHelper = nullptr;
bool isLocalServer = false;

static bool isLocator = false;
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
#include "LocatorHelper.hpp"

class SimpleCacheListener : public CacheListener {
 public:
  int m_totalEvents;

  SimpleCacheListener() : m_totalEvents(0) {}

  ~SimpleCacheListener() override {}

 public:
  // The Cache Listener callbacks.
  virtual void afterCreate(const EntryEvent&) override {
    LOG_INFO("SimpleCacheListener: Got an afterCreate event.");
    m_totalEvents++;
  }

  virtual void afterUpdate(const EntryEvent&) override {
    LOG_INFO("SimpleCacheListener: Got an afterUpdate event.");
    m_totalEvents++;
  }

  virtual void afterInvalidate(const EntryEvent&) override {
    LOG_INFO("SimpleCacheListener: Got an afterInvalidate event.");
    m_totalEvents++;
  }

  virtual void afterDestroy(const EntryEvent&) override {
    LOG_INFO("SimpleCacheListener: Got an afterDestroy event.");
    m_totalEvents++;
  }

  virtual void afterRegionInvalidate(const RegionEvent&) override {
    LOG_INFO("SimpleCacheListener: Got an afterRegionInvalidate event.");
    m_totalEvents++;
  }

  virtual void afterRegionDestroy(const RegionEvent& event) override {
    LOG_INFO("SimpleCacheListener: Got an afterRegionDestroy event.");
    if (event.remoteOrigin()) {
      m_totalEvents++;
    }
  }

  virtual void close(Region&) override {
    LOG_INFO("SimpleCacheListener: Got a close event.");
  }
};
std::shared_ptr<SimpleCacheListener> eventListener1 = nullptr;
std::shared_ptr<SimpleCacheListener> eventListener2 = nullptr;

void initClient(const bool isthinClient) {
  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(isthinClient);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}
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

void createPooledRegion(const std::string& name, bool ackMode, const std::string& locators,
                        const std::string& poolname,
                        bool clientNotificationEnabled = false,
                        bool cachingEnable = true) {
  LOG("createRegion_Pool() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name.c_str(), ackMode);
  fflush(stdout);
  auto regPtr =
      getHelper()->createPooledRegion(name, ackMode, locators, poolname,
                                      cachingEnable, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Pooled Region created.");
}

const char* testregex[] = {"Key-*1", "Key-*2", "Key-*3", "Key-*4"};
const char* keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char* regionNames[] = {"DistRegionAck", "DistRegionNoAck",
                             "ExampleRegion", "SubRegion1", "SubRegion2"};
const char* vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char* nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const bool USE_ACK = true;
const bool NO_ACK = false;

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pool_Locator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TESTPOOL1_",
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TESTPOOL1_", true);
    createPooledRegion(regionNames[2], NO_ACK, locatorsG, "__TESTPOOL1_", true);

    // create subregion
    auto regptr = getHelper()->getRegion(regionNames[2]);
    auto subregPtr1 =
        regptr->createSubregion(regionNames[3], regptr->getAttributes());
    auto subregPtr2 =
        regptr->createSubregion(regionNames[4], regptr->getAttributes());

    LOG_INFO(
        "NIL: CLIENT1 StepOne_Pool_Locator subregions created successfully");

    // Attache Listener
    auto regionPtr0 = getHelper()->getRegion(regionNames[0]);
    auto attrMutatorPtr = regionPtr0->getAttributesMutator();
    eventListener1 = std::make_shared<SimpleCacheListener>();
    attrMutatorPtr->setCacheListener(eventListener1);

    auto subregAttrMutatorPtr = subregPtr1->getAttributesMutator();
    eventListener2 = std::make_shared<SimpleCacheListener>();
    subregAttrMutatorPtr->setCacheListener(eventListener2);

    LOG("StepOne_Pool complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo_Pool_Locator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TESTPOOL1_",
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TESTPOOL1_", true);
    createPooledRegion(regionNames[2], NO_ACK, locatorsG, "__TESTPOOL1_", true);

    // create subregion
    auto regptr = getHelper()->getRegion(regionNames[2]);
    auto subregPtr1 =
        regptr->createSubregion(regionNames[3], regptr->getAttributes());
    auto subregPtr2 =
        regptr->createSubregion(regionNames[4], regptr->getAttributes());

    LOG_INFO(
        "NIL: CLIENT2 StepTwo_Pool_Locator:: subregions created successfully");
    LOG("StepTwo_Pool complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, registerKeysOnRegion)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    // auto regPtr1 = getHelper()->getRegion(regionNames[1]);
    /*regPtr0->registerRegex(testregex[0]);
    regPtr1->registerRegex(testregex[1]);*/

    // NIL
    std::vector<std::shared_ptr<CacheableKey>> keysVector;
    keysVector.push_back(CacheableString::create("Key-1"));
    keysVector.push_back(CacheableString::create("Key-2"));
    regPtr0->registerKeys(keysVector, false);
    LOG_INFO("NIL CLIENT-1 registerAllKeys() done ");

    regPtr0->localDestroyRegion();
    LOG_INFO("NIL CLIENT-1 localDestroyRegion() done");
    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, putOps)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);

    for (int index = 0; index < 5; index++) {
      auto key = "Key-" + std::to_string(index);
      auto value = "Value-" + std::to_string(index);

      auto keyptr = CacheableKey::create(key);
      auto valuePtr = CacheableString::create(value);

      regPtr0->put(keyptr, valuePtr);
    }
    LOG("StepFour complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);

    regPtr0->registerAllKeys();
    LOG_INFO("NIL CLIENT-1 registerAllKeys() done ");

    regPtr0->localDestroyRegion();
    LOG_INFO("NIL CLIENT-1 localDestroyRegion() done");
    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepFour)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);

    for (int index = 0; index < 5; index++) {
      auto key = "Key-" + std::to_string(index);
      auto value = "Value-" + std::to_string(index);

      auto keyptr = CacheableKey::create(key);
      auto valuePtr = CacheableString::create(value);

      regPtr0->put(keyptr, valuePtr);
    }
    LOG("StepFour complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, verifyEventsForDestroyedregion)
  {
    LOG_INFO("NIL:LINE_537 eventListener1->m_totalEvents = %d ",
            eventListener1->m_totalEvents);
    ASSERT(eventListener1->m_totalEvents == 0,
           "Region Event count must be zero");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, verifyEventsForDestroyedSubregion)
  {
    LOG_INFO("NIL:LINE_543 eventListener2->m_totalEvents = %d ",
            eventListener2->m_totalEvents);
    ASSERT(eventListener2->m_totalEvents == 0,
           "Subregion Event count must be zero");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFive)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);
    regPtr0->registerRegex(testregex[0]);
    regPtr1->registerRegex(testregex[1]);
    LOG_INFO("NIL CLIENT-1 registerRegex() done ");

    regPtr0->localDestroyRegion();
    LOG_INFO("NIL CLIENT-1 localDestroyRegion() done");
    LOG("NIL: Client-1 StepFive complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepSix)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);

    for (int index = 0; index < 5; index++) {
      auto key = "Key-" + std::to_string(index);
      auto value = "Value-" + std::to_string(index);

      auto keyptr = CacheableKey::create(key);
      auto valuePtr = CacheableString::create(value);

      regPtr0->put(keyptr, valuePtr);
      regPtr1->put(keyptr, valuePtr);
    }
    LOG("NIL : Client-2 StepSix complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, VerifyOps)
  {
    // regPtr0 is destroyed
    // auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);

    auto keyptr = CacheableKey::create("Key-2");

    // regPtr0 is destroyed
    // ASSERT( !regPtr0->containsKey( keyptr ), "Key must not found in region0."
    // );

    ASSERT(regPtr1->containsKey(keyptr), "Key must found in region1.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepSeven)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);
    auto regPtr2 = getHelper()->getRegion(regionNames[2]);

    auto subregPtr0 = regPtr2->getSubregion(regionNames[3]);
    auto subregPtr1 = regPtr2->getSubregion(regionNames[4]);

    // 1. registerAllKeys on parent and both subregions
    regPtr2->registerAllKeys();
    subregPtr0->registerAllKeys();
    subregPtr1->registerAllKeys();

    LOG_INFO("NIL CLIENT-1 StepSeven ::  registerAllKeys() done ");

    // 2. Now locally destroy SubRegion1
    subregPtr0->localDestroyRegion();
    LOG_INFO("NIL CLIENT-1 SubRegion1 locally destroyed successfully");

    LOG("NIL: Client-1 StepSeven complete.");

    /*
    regPtr0->registerRegex(testregex[0]);
    regPtr1->registerRegex(testregex[1]);
    LOG_INFO("NIL CLIENT-1 registerRegex() done ");
    */
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepEight)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);
    auto regPtr2 = getHelper()->getRegion(regionNames[2]);

    auto subregPtr0 = regPtr2->getSubregion(regionNames[3]);
    auto subregPtr1 = regPtr2->getSubregion(regionNames[4]);

    for (int index = 0; index < 5; index++) {
      auto key = "Key-" + std::to_string(index);
      auto value = "Value-" + std::to_string(index);

      auto keyptr = CacheableKey::create(key);
      auto valuePtr = CacheableString::create(value);

      regPtr2->put(keyptr, valuePtr);
      subregPtr0->put(keyptr, valuePtr);
      subregPtr1->put(keyptr, valuePtr);
    }

    LOG("NIL : Client-2 StepSix complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, VerifySubRegionOps)
  {
    auto regPtr2 = getHelper()->getRegion(regionNames[2]);
    // locally destroyed
    // auto subregPtr0 = regPtr2->getSubregion( regionNames[3] );
    auto subregPtr1 = regPtr2->getSubregion(regionNames[4]);

    for (int index = 0; index < 5; index++) {
      auto key = "Key-" + std::to_string(index);
      auto value = "Value-" + std::to_string(index);

      auto keyptr = CacheableKey::create(key);
      auto valuePtr = CacheableString::create(value);

      ASSERT(regPtr2->containsKey(keyptr), "Key must found in region1.");
      ASSERT(subregPtr1->containsKey(keyptr), "Key must found in region1.");

      // Need to check in cliet/server logs that client-1 do not receive any
      // notification for subregPtr0
    }
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

// testRegisterKeyForLocalRegionDestroy
void testRegisterKeyForLocalRegionDestroy() {
  CALL_TASK(CreateLocator1);
  CALL_TASK(CreateServer1_With_Locator_XML_Bug849);

  CALL_TASK(StepOne_Pool_Locator);
  CALL_TASK(StepTwo_Pool_Locator);

  CALL_TASK(registerKeysOnRegion);
  CALL_TASK(putOps);
  CALL_TASK(verifyEventsForDestroyedregion);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer1);
  CALL_TASK(CloseLocator1);
}

void testRegisterAllKeysForLocalRegionDestroy() {
  CALL_TASK(CreateLocator1);
  CALL_TASK(CreateServer1_With_Locator_XML_Bug849);

  CALL_TASK(StepOne_Pool_Locator);
  CALL_TASK(StepTwo_Pool_Locator);

  CALL_TASK(StepThree);
  CALL_TASK(StepFour);
  CALL_TASK(verifyEventsForDestroyedregion);

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer1);

  CALL_TASK(CloseLocator1);
}

void testRegisterRegexForLocalRegionDestroy() {
  CALL_TASK(CreateLocator1);
  CALL_TASK(CreateServer1_With_Locator_XML_Bug849);

  CALL_TASK(StepOne_Pool_Locator);
  CALL_TASK(StepTwo_Pool_Locator);

  CALL_TASK(StepFive);
  CALL_TASK(StepSix);
  CALL_TASK(verifyEventsForDestroyedregion);
  CALL_TASK(VerifyOps);

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer1);

  CALL_TASK(CloseLocator1);
}

void testSubregionForLocalRegionDestroy() {
  CALL_TASK(CreateLocator1);
  CALL_TASK(CreateServer1_With_Locator_XML_Bug849);

  CALL_TASK(StepOne_Pool_Locator);
  CALL_TASK(StepTwo_Pool_Locator);

  CALL_TASK(StepSeven);
  CALL_TASK(StepEight);
  CALL_TASK(verifyEventsForDestroyedSubregion);
  CALL_TASK(VerifySubRegionOps);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer1);

  CALL_TASK(CloseLocator1);
}

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTRIWITHLOCALREGIONDESTROY_H_
