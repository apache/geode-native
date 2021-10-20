#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTREMOVEALL_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTREMOVEALL_H_

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

#define ROOT_NAME "ThinClientRemoveAll"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "fw_dunit.hpp"
#include <ace/OS.h>
#include <ace/High_Res_Timer.h>
#include "testobject/PdxType.hpp"
#include "testobject/VariousPdxTypes.hpp"
#include <string>

#include "BuiltinCacheableWrappers.hpp"
#include "Utils.hpp"
#include "statistics/StatisticsFactory.hpp"

#include "CacheHelper.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2

namespace {  // NOLINT(google-build-namespaces)

using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;
using apache::geode::client::EntryNotFoundException;
using apache::geode::client::HashMapOfCacheable;
using apache::geode::client::IllegalArgumentException;

CacheHelper* cacheHelper = nullptr;
static bool isLocalServer = false;
static bool isLocator = false;
static int numberOfLocators = 0;

const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);
const char* poolName = "__TESTPOOL1_";

#include "LocatorHelper.hpp"

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

const char* keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char* vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char* nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const char* regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

const bool USE_ACK = true;
const bool NO_ACK = false;

void createRegion(const char* name, bool ackMode, const char*,
                  bool isCacheEnabled, bool clientNotificationEnabled = false) {
  LOG("createRegion() entered.");
  std::cout << "Creating region --  " << name << " ackMode is " << ackMode
            << "\n"
            << std::flush;
  auto regPtr = getHelper()->createRegion(name, ackMode, isCacheEnabled,
                                          nullptr, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}

void createRegionLocal(const char* name, bool ackMode, const char*,
                       bool isCacheEnabled,
                       bool clientNotificationEnabled = false) {
  LOG("createRegion() entered.");
  std::cout << "Creating region --  " << name << " ackMode is " << ackMode
            << "\n"
            << std::flush;
  auto regPtr = getHelper()->createRegion(
      name, ackMode, isCacheEnabled, nullptr, clientNotificationEnabled, true);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}

void createPooledRegion(const std::string& name, bool ackMode,
                        const std::string& locators,
                        const std::string& poolname,
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

void createPooledRegionConcurrencyCheckDisabled(
    const char* name, bool ackMode, const char*, const char* locators,
    const char* poolname, bool clientNotificationEnabled = false,
    bool cachingEnable = true, bool concurrencyCheckEnabled = true) {
  LOG("createRegion_Pool() entered.");
  std::cout << "Creating region --  " << name << " ackMode is " << ackMode
            << "\n"
            << std::flush;
  auto regPtr = getHelper()->createPooledRegionConcurrencyCheckDisabled(
      name, ackMode, locators, poolname, cachingEnable,
      clientNotificationEnabled, concurrencyCheckEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Pooled Region created.");
}

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_notify_subscription.xml");
      LOG("SERVER1 started");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2)
  {
    if (isLocalServer) {
      CacheHelper::initServer(2, "cacheserver_notify_subscription2.xml");
      LOG("SERVER2 started");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CreateClient1RegionsLocal)
  {
    initClient(true);
    createRegionLocal(regionNames[0], USE_ACK, nullptr, true, false);
    LOG("CreateRegions complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CreateClient1Regions_Pooled_Locator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, poolName, true,
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, poolName, true, true);
    LOG("CreateRegions complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CreateClient1Regions_Pooled_Locator_NoCaching)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, poolName, false,
                       false);
    LOG("CreateRegions_Pooled_Locator_NoCaching complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CreateClient2Regions_Pooled_Locator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, poolName, true,
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, poolName, true, true);
    LOG("CreateRegions complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, removeAllValidation)
  {
    char key[2048];
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    std::vector<std::shared_ptr<CacheableKey>> removeallkeys;
    try {
      regPtr0->removeAll(removeallkeys);
      FAIL("Did not get expected IllegalArgumentException exception");
    } catch (IllegalArgumentException&) {
      LOG("Got expected IllegalArgumentException found exception");
    }

    try {
      regPtr0->removeAll(removeallkeys, CacheableInt32::create(1));
      FAIL("Did not get expected IllegalArgumentException exception");
    } catch (IllegalArgumentException&) {
      LOG("Got expected IllegalArgumentException found exception");
    }

    try {
      regPtr0->removeAll(removeallkeys, nullptr);
      FAIL("Did not get expected IllegalArgumentException exception");
    } catch (IllegalArgumentException&) {
      LOG("Got expected IllegalArgumentException found exception");
    }

    for (int32_t item = 0; item < 1; item++) {
      removeallkeys.push_back(
          CacheableKey::create(std::string("key-") + std::to_string(item)));
    }

    try {
      regPtr0->removeAll(removeallkeys);
    } catch (EntryNotFoundException&) {
      FAIL("Got un expected entry not found exception");
    }

    try {
      regPtr0->removeAll(removeallkeys, CacheableInt32::create(1));
    } catch (EntryNotFoundException&) {
      FAIL("Got un expected entry not found exception");
    }

    try {
      regPtr0->removeAll(removeallkeys, nullptr);
    } catch (EntryNotFoundException&) {
      FAIL("Got un expected entry not found exception");
    }
    LOG("removeAllValidation complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, removeAllValidationLocal)
  {
    char key[2048];
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    std::vector<std::shared_ptr<CacheableKey>> removeallkeys;
    try {
      regPtr0->removeAll(removeallkeys);
      FAIL("Did not get expected IllegalArgumentException exception");
    } catch (IllegalArgumentException&) {
      LOG("Got expected IllegalArgumentException found exception");
    }

    try {
      regPtr0->removeAll(removeallkeys, CacheableInt32::create(1));
      FAIL("Did not get expected IllegalArgumentException exception");
    } catch (IllegalArgumentException&) {
      LOG("Got expected IllegalArgumentException found exception");
    }

    try {
      regPtr0->removeAll(removeallkeys, nullptr);
      FAIL("Did not get expected IllegalArgumentException exception");
    } catch (IllegalArgumentException&) {
      LOG("Got expected IllegalArgumentException found exception");
    }

    for (int32_t item = 0; item < 1; item++) {
      removeallkeys.push_back(
          CacheableKey::create(std::string("key-") + std::to_string(item)));
      removeallkeys.push_back(CacheableKey::create(key));
    }

    try {
      regPtr0->removeAll(removeallkeys);
    } catch (EntryNotFoundException&) {
      FAIL("Got un expected entry not found exception");
    }

    try {
      regPtr0->removeAll(removeallkeys, CacheableInt32::create(1));
    } catch (EntryNotFoundException&) {
      FAIL("Got un expected entry not found exception");
    }

    try {
      regPtr0->removeAll(removeallkeys, nullptr);
    } catch (EntryNotFoundException&) {
      FAIL("Got un expected entry not found exception");
    }

    LOG("removeAllValidation complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, removeAllOps)
  {
    HashMapOfCacheable entryMap;
    entryMap.clear();
    for (int32_t item = 0; item < 1; item++) {
      auto key = std::string("key-") + std::to_string(item);
      auto value = std::to_string(item);
      entryMap.emplace(CacheableKey::create(key),
                       CacheableString::create(value));
    }

    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    regPtr0->putAll(entryMap);
    LOG("putAll1 complete");

    std::vector<std::shared_ptr<CacheableKey>> removeallkeys;
    for (int32_t item = 0; item < 1; item++) {
      removeallkeys.push_back(
          CacheableKey::create(std::string("key-") + std::to_string(item)));
    }

    regPtr0->removeAll(removeallkeys);
    ASSERT(regPtr0->size() == 0,
           "remove all should remove the entries specified.");
    LOG("remove all complete.");
  }
END_TASK_DEFINITION

// Testing that sequence number is correctly incemented after removeAll by the
// number
// of elements being removed.
DUNIT_TASK_DEFINITION(CLIENT1, removeAllSequence)
  {
    HashMapOfCacheable entryMap;
    entryMap.clear();
    entryMap.emplace(CacheableKey::create(1), CacheableInt32::create(1));
    entryMap.emplace(CacheableKey::create(2), CacheableInt32::create(2));
    entryMap.emplace(CacheableKey::create(3), CacheableInt32::create(3));
    entryMap.emplace(CacheableKey::create(4), CacheableInt32::create(4));

    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    regPtr0->putAll(entryMap);
    LOG("putAll1 complete");

    std::vector<std::shared_ptr<CacheableKey>> removeallkeys;
    removeallkeys.push_back(CacheableKey::create(1));
    removeallkeys.push_back(CacheableKey::create(2));
    removeallkeys.push_back(CacheableKey::create(3));
    removeallkeys.push_back(CacheableKey::create(4));

    regPtr0->removeAll(removeallkeys);
    LOG("removeAll complete");

    ASSERT(regPtr0->get(CacheableKey::create(1)) == nullptr, "Key 1 exists");
    ASSERT(regPtr0->get(CacheableKey::create(2)) == nullptr, "Key 2 exists");
    ASSERT(regPtr0->get(CacheableKey::create(3)) == nullptr, "Key 3 exists");
    ASSERT(regPtr0->get(CacheableKey::create(4)) == nullptr, "Key 4 exists");

    entryMap.clear();
    entryMap.emplace(CacheableKey::create(5), CacheableInt32::create(5));
    entryMap.emplace(CacheableKey::create(6), CacheableInt32::create(6));

    regPtr0->putAll(entryMap);
    LOG("putAll2 complete");

    ASSERT(regPtr0->get(CacheableKey::create(5)) != nullptr, "Key 5 missing");
    ASSERT(regPtr0->get(CacheableKey::create(6)) != nullptr, "Key 6 missing");

    LOG("remove all complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, removeAllOpsLocal)
  {
    HashMapOfCacheable entryMap;
    entryMap.clear();
    for (int32_t item = 0; item < 1; item++) {
      auto key = std::string("key-") + std::to_string(item);
      auto value = std::to_string(item);
      entryMap.emplace(CacheableKey::create(key),
                       CacheableString::create(value));
    }

    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    regPtr0->putAll(entryMap);
    LOG("putAll1 complete");

    std::vector<std::shared_ptr<CacheableKey>> removeallkeys;
    for (int32_t item = 0; item < 1; item++) {
      removeallkeys.push_back(
          CacheableKey::create(std::string("key-") + std::to_string(item)));
    }

    regPtr0->removeAll(removeallkeys);
    ASSERT(regPtr0->size() == 0,
           "remove all should remove the entries specified.");
    LOG("remove all complete.");
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

DUNIT_TASK_DEFINITION(SERVER2, CloseServer2)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
    }
  }
END_TASK_DEFINITION

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTREMOVEALL_H_
