#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTPUTALLWITHCALLBACK_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTPUTALLWITHCALLBACK_H_

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
#include "testobject/PdxType.hpp"
#include "testobject/VariousPdxTypes.hpp"
#include <string>

#define ROOT_NAME "ThinClientPutAllWithCallBack"
#define ROOT_SCOPE DISTRIBUTED_ACK
#include "CacheHelper.hpp"
#include "CacheRegionHelper.hpp"
#include "SerializationRegistry.hpp"
#include "CacheImpl.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2

namespace { // NOLINT(google-build-namespaces)

using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableInt64;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;
using apache::geode::client::CacheRegionHelper;
using apache::geode::client::Exception;
using apache::geode::client::HashMapOfCacheable;
using apache::geode::client::IllegalArgumentException;

CacheHelper* cacheHelper = nullptr;
static bool isLocalServer = false;
static bool isLocator = false;
static int numberOfLocators = 0;

const char* locatorsG =
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

void _verifyEntry(const char* name, const char* key, const char* val,
                  bool noKey, bool isCreated = false) {
  // Verify key and value exist in this region, in this process.
  const char* value = val ? val : "";
  char* buf =
      reinterpret_cast<char*>(malloc(1024 + strlen(key) + strlen(value)));
  ASSERT(buf, "Unable to malloc buffer for logging.");
  if (!isCreated) {
    if (noKey) {
      sprintf(buf, "Verify key %s does not exist in region %s", key, name);
    } else if (!val) {
      sprintf(buf, "Verify value for key %s does not exist in region %s", key,
              name);
    } else {
      sprintf(buf, "Verify value for key %s is: %s in region %s", key, value,
              name);
    }
    LOG(buf);
  }

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  auto keyPtr = CacheableKey::create(key);

  // if the region is no ack, then we may need to wait...
  if (!isCreated) {
    if (noKey == false) {  // need to find the key!
      ASSERT(regPtr->containsKey(keyPtr), "Key not found in region.");
    }
    if (val != nullptr) {  // need to have a value!
      ASSERT(regPtr->containsValueForKey(keyPtr), "Value not found in region.");
    }
  }

  // loop up to MAX times, testing condition
  uint32_t MAX = 100;
  //  changed sleep from 10 ms
  uint32_t SLEEP = 1000;  // milliseconds
  uint32_t containsKeyCnt = 0;
  uint32_t containsValueCnt = 0;
  uint32_t testValueCnt = 0;

  for (int i = MAX; i >= 0; i--) {
    if (isCreated) {
      if (!regPtr->containsKey(keyPtr)) {
        containsKeyCnt++;
      } else {
        break;
      }
      ASSERT(containsKeyCnt < MAX, "Key has not been created in region.");
    } else {
      if (noKey) {
        if (regPtr->containsKey(keyPtr)) {
          containsKeyCnt++;
        } else {
          break;
        }
        ASSERT(containsKeyCnt < MAX, "Key found in region.");
      }
      if (val == nullptr) {
        if (regPtr->containsValueForKey(keyPtr)) {
          containsValueCnt++;
        } else {
          break;
        }
        ASSERT(containsValueCnt < MAX, "Value found in region.");
      }

      if (val != nullptr) {
        auto checkPtr =
            std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr));

        ASSERT(checkPtr != nullptr, "Value Ptr should not be null.");
        char buf[1024];
        sprintf(buf, "In verify loop, get returned %s for key %s",
                checkPtr->value().c_str(), key);
        LOG(buf);
        if (strcmp(checkPtr->value().c_str(), value) != 0) {
          testValueCnt++;
        } else {
          break;
        }
        ASSERT(testValueCnt < MAX, "Incorrect value found.");
      }
    }
    dunit::sleep(SLEEP);
  }
}

#define verifyEntry(x, y, z) _verifyEntry(x, y, z, __LINE__)

void _verifyEntry(const char* name, const char* key, const char* val,
                  int line) {
  char logmsg[1024];
  sprintf(logmsg, "verifyEntry() called from %d.\n", line);
  LOG(logmsg);
  _verifyEntry(name, key, val, false);
  LOG("Entry verified.");
}

#define verifyCreated(x, y) _verifyCreated(x, y, __LINE__)

void _verifyCreated(const char* name, const char* key, int line) {
  char logmsg[1024];
  sprintf(logmsg, "verifyCreated() called from %d.\n", line);
  LOG(logmsg);
  _verifyEntry(name, key, nullptr, false, true);
  LOG("Entry created.");
}
void createRegion(const char* name, bool ackMode, bool isCacheEnabled,
                  bool clientNotificationEnabled = false) {
  LOG("createRegion() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  auto regPtr = getHelper()->createRegion(name, ackMode, isCacheEnabled,
                                          nullptr, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}
void createPooledRegion(const char* name, bool ackMode, const char* locators,
                        const char* poolname,
                        bool clientNotificationEnabled = false,
                        bool cachingEnable = true) {
  LOG("createRegion_Pool() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  auto regPtr =
      getHelper()->createPooledRegion(name, ackMode, locators, poolname,
                                      cachingEnable, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Pooled Region created.");
}

void createPooledRegionConcurrencyCheckDisabled(
    const char* name, bool ackMode, const char* locators, const char* poolname,
    bool clientNotificationEnabled = false, bool cachingEnable = true,
    bool concurrencyCheckEnabled = true) {
  LOG("createRegion_Pool() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  auto regPtr = getHelper()->createPooledRegionConcurrencyCheckDisabled(
      name, ackMode, locators, poolname, cachingEnable,
      clientNotificationEnabled, concurrencyCheckEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Pooled Region created.");
}

void createEntry(const char* name, const char* key,
                 const char* value = nullptr) {
  LOG("createEntry() entered.");
  fprintf(stdout, "Creating entry -- key: %s  value: %s in region %s\n", key,
          value, name);
  fflush(stdout);
  // Create entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  if (value == nullptr) {
    value = "";
  }
  auto valPtr = CacheableString::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(!regPtr->containsKey(keyPtr),
         "Key should not have been found in region.");
  ASSERT(!regPtr->containsValueForKey(keyPtr),
         "Value should not have been found in region.");

  regPtr->create(keyPtr, valPtr);
  // regPtr->put( keyPtr, valPtr );
  LOG("Created entry.");

  verifyEntry(name, key, value);
  LOG("Entry created.");
}

void updateEntry(const char* name, const char* key, const char* value) {
  LOG("updateEntry() entered.");
  fprintf(stdout, "Updating entry -- key: %s  value: %s in region %s\n", key,
          value, name);
  fflush(stdout);
  // Update entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableString::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(regPtr->containsKey(keyPtr), "Key should have been found in region.");
  ASSERT(regPtr->containsValueForKey(keyPtr),
         "Value should have been found in region.");

  regPtr->put(keyPtr, valPtr);
  LOG("Put entry.");

  verifyEntry(name, key, value);
  LOG("Entry updated.");
}

void doNetsearch(const char* name, const char* key, const char* value) {
  LOG("doNetsearch() entered.");
  fprintf(
      stdout,
      "Netsearching for entry -- key: %s  expecting value: %s in region %s\n",
      key, value, name);
  fflush(stdout);
  // Get entry created in Process A, verify entry is correct
  auto keyPtr = CacheableKey::create(key);

  auto regPtr = getHelper()->getRegion(name);
  fprintf(stdout, "netsearch  region %s\n", regPtr->getName().c_str());
  fflush(stdout);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(!regPtr->containsKey(keyPtr),
         "Key should not have been found in region.");
  ASSERT(!regPtr->containsValueForKey(keyPtr),
         "Value should not have been found in region.");

  auto checkPtr = std::dynamic_pointer_cast<CacheableString>(
      regPtr->get(keyPtr));  // force a netsearch

  if (checkPtr != nullptr) {
    LOG("checkPtr is not null");
    char buf[1024];
    sprintf(buf, "In net search, get returned %s for key %s",
            checkPtr->value().c_str(), key);
    LOG(buf);
  } else {
    LOG("checkPtr is nullptr");
  }
  verifyEntry(name, key, value);
  LOG("Netsearch complete.");
}

const char* keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char* vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char* nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const char* regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

const bool USE_ACK = true;
const bool NO_ACK = false;

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

DUNIT_TASK_DEFINITION(CLIENT1,
                      CreateClient1RegionsWithCachingWithConcurrencyCheck)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, poolName, true,
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, poolName, true, true);
    LOG("CreateClient1RegionsWithCachingWithConcurrencyCheck complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1,
                      CreateClient1RegionsWithoutCachingWithConcurrencyCheck)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, poolName, false,
                       false);
    LOG("CreateClient1RegionsWithoutCachingWithConcurrencyCheck complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1,
                      CreateClient1RegionsWithCachingWithoutConcurrencyCheck)
  {
    initClient(true);
    createPooledRegionConcurrencyCheckDisabled(
        regionNames[0], USE_ACK, locatorsG, poolName, true, true, false);
    LOG("CreateClient1RegionsWithCachingWithoutConcurrencyCheck complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CreateClient2Regions)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, poolName, true,
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, poolName, true, true);
    LOG("CreateClient2Regions complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, RegisterClient1Keys)
  {
    try {
      auto serializationRegistry =
          CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
              ->getSerializationRegistry();

      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxType::createDeserializable);
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes1::createDeserializable);
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes2::createDeserializable);
      serializationRegistry->addPdxSerializableType(
          PdxTests::Address::createDeserializable);
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes3::createDeserializable);
    } catch (Exception&) {
      LOG("Got expected Exception for Serialization, already registered");
    }
    // the client1 will register k0 and k1
    // createEntry( regionNames[0], keys[0], vals[0] );
    // createEntry( regionNames[0], keys[1], vals[1] );
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);

    auto keyPtr0 = CacheableKey::create(keys[0]);
    auto keyPtr1 = CacheableKey::create(keys[1]);

    std::vector<std::shared_ptr<CacheableKey>> keys1;
    keys1.push_back(keyPtr0);
    keys1.push_back(keyPtr1);
    regPtr0->registerKeys(keys1);

    auto keyPtr2 = CacheableKey::create(keys[2]);
    auto keyPtr3 = CacheableKey::create(keys[3]);

    std::vector<std::shared_ptr<CacheableKey>> keys2;
    keys2.push_back(keyPtr2);
    keys2.push_back(keyPtr3);
    regPtr1->registerKeys(keys2);

    LOG("RegisterClient1Keys complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, RegisterClient2Keys)
  {
    try {
      auto serializationRegistry =
          CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
              ->getSerializationRegistry();
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxType::createDeserializable);
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes1::createDeserializable);
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes2::createDeserializable);
      serializationRegistry->addPdxSerializableType(
          PdxTests::Address::createDeserializable);
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxTypes3::createDeserializable);
    } catch (Exception&) {
      LOG("Got expected Exception for Serialization, already registered");
    }
    HashMapOfCacheable map0;
    HashMapOfCacheable map1;
    map0.clear();
    map1.clear();
    for (int i = 0; i < 2; i++) {
      map0.emplace(CacheableKey::create(keys[i]),
                   CacheableString::create(vals[i]));
    }
    for (int i = 2; i < 4; i++) {
      map1.emplace(CacheableKey::create(keys[i]),
                   CacheableString::create(vals[i]));
    }
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);
    regPtr0->putAll(map0, std::chrono::seconds(15),
                    CacheableInt32::create(1000));
    regPtr1->putAll(map1, std::chrono::seconds(15),
                    CacheableInt32::create(1000));
    LOG("RegisterClient2Keys complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, PutAllOps)
  {
    HashMapOfCacheable entryMap;
    entryMap.clear();
    char key[2048];
    char value[2048];
    for (int32_t item = 0; item < 500; item++) {
      sprintf(key, "key-%d", item);
      sprintf(value, "%d", item);
      entryMap.emplace(CacheableKey::create(key),
                       CacheableString::create(value));
    }

    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    regPtr0->putAll(entryMap, std::chrono::seconds(15),
                    CacheableInt32::create(1000));

    LOG("putAll1 complete");

    std::vector<std::shared_ptr<CacheableKey>> getAllkeys;
    for (int32_t item = 0; item < 500; item++) {
      sprintf(key, "key-%d", item);
      getAllkeys.push_back(CacheableKey::create(key));
    }

    const auto values = regPtr0->getAll(getAllkeys);
    ASSERT(values.size() == 500, "GetAll should return 500 entries.");

    LOG("PutAllOps complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, VerifyInitialEntries)
  {
    verifyCreated(regionNames[0], keys[0]);
    verifyCreated(regionNames[0], keys[1]);

    verifyEntry(regionNames[0], keys[0], vals[0]);
    verifyEntry(regionNames[0], keys[1], vals[1]);

    // doNetsearch( regionNames[1], keys[2], vals[2] );
    // doNetsearch( regionNames[1], keys[3], vals[3] );

    verifyCreated(regionNames[1], keys[2]);
    verifyCreated(regionNames[1], keys[3]);

    verifyEntry(regionNames[1], keys[2], vals[2]);
    verifyEntry(regionNames[1], keys[3], vals[3]);

    LOG("VerifyInitialEntries complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, TriggerAfterUpdateEvents)
  {
    LOG("Trigger afterUpdate events.");
    HashMapOfCacheable map0;
    HashMapOfCacheable map1;
    map0.clear();
    map1.clear();
    for (int i = 0; i < 2; i++) {
      map0.emplace(CacheableKey::create(keys[i]),
                   CacheableString::create(nvals[i]));
    }
    for (int i = 2; i < 4; i++) {
      map1.emplace(CacheableKey::create(keys[i]),
                   CacheableString::create(nvals[i]));
    }
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);
    regPtr0->putAll(map0, std::chrono::seconds(15),
                    CacheableInt32::create(1000));
    regPtr1->putAll(map1, std::chrono::seconds(15),
                    CacheableInt32::create(1000));

    // register all keys for the large putall test case
    regPtr0->registerAllKeys();

    LOG("TriggerAfterUpdateEvents complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, VerifyUpdatedEntries)
  {
    verifyEntry(regionNames[0], keys[0], nvals[0]);
    verifyEntry(regionNames[0], keys[1], nvals[1]);
    // region1 is not changed at client
    verifyEntry(regionNames[1], keys[2], nvals[2]);
    verifyEntry(regionNames[1], keys[3], nvals[3]);
    LOG("VerifyUpdatedEntries complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, ExecuteLargePutAll)
  {
    LOG("Do large PutAll");
    HashMapOfCacheable map0;
    map0.clear();
    for (int i = 0; i < 100000; i++) {
      char key0[50] = {0};
      char val0[2500] = {0};
      sprintf(key0, "key-%d", i);
      sprintf(val0, "%1000d", i);
      map0.emplace(CacheableKey::create(key0), CacheableString::create(val0));
    }
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);

    regPtr0->putAll(map0, std::chrono::seconds(40000),
                    CacheableInt32::create(1000));

    LOG("ExecuteLargePutAll complete.");
    dunit::sleep(10000);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyLargePutAll)
  {
    LOG("Verify large PutAll");
    for (int i = 0; i < 100000; i++) {
      char key0[50] = {0};
      sprintf(key0, "key-%d", i);
      verifyCreated(regionNames[0], key0);
    }
    LOG("VerifyLargePutAll complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, VerifyRegionService)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    auto& rsp = regPtr0->getRegionService();
    auto regPtr = rsp.getRegion(regionNames[0]);
    ASSERT(regPtr != nullptr, "Failed to get region.");

    auto& rsp1 = regPtr0->getRegionService();
    auto regPtr1 = rsp1.getRegion("NOT_CREATED_REGION");
    ASSERT(regPtr1 == nullptr, "Unknown Region Returned");

    LOG("VerifyRegionService complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, InvalidateKeys)
  {
    auto keyPtr0 = CacheableInt64::create(100L);
    auto valPtr0 = CacheableInt64::create(200L);

    auto regPtr0 = getHelper()->getRegion(regionNames[0]);

    regPtr0->put(keyPtr0, valPtr0);
    auto checkPtr =
        std::dynamic_pointer_cast<CacheableInt64>(regPtr0->get(keyPtr0));
    ASSERT(checkPtr != nullptr, "checkPtr should not be null.");

    regPtr0->invalidate(keyPtr0);
    checkPtr = std::dynamic_pointer_cast<CacheableInt64>(regPtr0->get(keyPtr0));
    ASSERT(checkPtr == nullptr, "checkPtr should be null.");

    try {
      std::shared_ptr<CacheableKey> key;
      regPtr0->invalidate(key);
      FAIL("Invalidate on nullptr should throw exception");
    } catch (IllegalArgumentException&) {
      LOG(" Got an expected exception invalidate on nullptr should be throwing "
          "exception ");
    }

    LOG("InvalidateKeys complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, VerifyPutAllWithLongKeyAndStringValue)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    HashMapOfCacheable map0;
    for (int i = 0; i < 2; i++) {
      map0.emplace(CacheableInt64::create(i), CacheableInt64::create(i));
    }
    regPtr0->putAll(map0, std::chrono::seconds(15),
                    CacheableInt32::create(1000));
    for (int i = 0; i < 2; i++) {
      auto checkPtr = std::dynamic_pointer_cast<CacheableInt64>(
          regPtr0->get(CacheableInt64::create(i)));
      ASSERT(checkPtr->value() == i,
             "putAll entry with long key and long value Mismatch.");
    }
    map0.clear();
    const char* vals[] = {"Value-100", "Value-200"};

    for (int i = 80; i < 82; i++) {
      map0.emplace(CacheableInt64::create(i),
                   CacheableString::create(vals[i - 80]));
    }
    regPtr0->putAll(map0, std::chrono::seconds(15),
                    CacheableInt32::create(1000));
    for (int i = 80; i < 82; i++) {
      auto checkPtr = std::dynamic_pointer_cast<CacheableString>(
          regPtr0->get(CacheableInt64::create(i)));
      ASSERT(strcmp(checkPtr->value().c_str(), vals[i - 80]) == 0,
             "putAll entry with long key and string value  Mismatch");
    }

    LOG("InvalidateKeys complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, VerifyPutAllWithLongKeyAndLongValue)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    HashMapOfCacheable map0;
    try {
      map0.emplace(CacheableInt64::create(345),
                   CacheableInt64::create(3465987));
      regPtr0->putAll(map0, std::chrono::seconds(-1),
                      CacheableInt32::create(1000));
      auto checkPtr = std::dynamic_pointer_cast<CacheableInt64>(
          regPtr0->get(CacheableInt64::create(345)));
      ASSERT(checkPtr->value() == 3465987,
             "putAll entry with long key and long value Mismatch.");
    } catch (Exception& excp) {
      std::string logmsg = "";
      logmsg += "expected exception ";
      logmsg += excp.getName();
      logmsg += ": ";
      logmsg += excp.what();
      LOG(logmsg.c_str());
    }
    map0.clear();

    try {
      map0.emplace(CacheableInt64::create(3451),
                   CacheableInt64::create(3465987));
      regPtr0->putAll(map0, std::chrono::seconds(2147500),
                      CacheableInt32::create(1000));
      auto checkPtr = std::dynamic_pointer_cast<CacheableInt64>(
          regPtr0->get(CacheableInt64::create(3451)));
      ASSERT(checkPtr->value() == 3465987,
             "putAll entry with long key and long value Mismatch.");
    } catch (Exception& excp) {
      std::string logmsg = "";
      logmsg += "expected exception ";
      logmsg += excp.getName();
      logmsg += ": ";
      logmsg += excp.what();
      LOG(logmsg.c_str());
    }

    LOG("VerifyPutAllWithLongKeyAndLongValue complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, VerifyPutAllWithObjectKey)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    HashMapOfCacheable map0;
    auto val111 = std::make_shared<PdxTests::PdxTypes1>();
    map0.emplace(CacheableInt32::create(1211), val111);
    regPtr0->putAll(map0, std::chrono::seconds(15),
                    CacheableInt32::create(1000));
    auto retObj = std::dynamic_pointer_cast<PdxTests::PdxTypes1>(
        regPtr0->get(CacheableInt32::create(1211)));
    ASSERT(val111->equals(retObj) == true, "val111 and retObj should match.");
    map0.clear();

    auto keyObject = std::make_shared<PdxTests::PdxType>();
    map0.emplace(keyObject, CacheableInt32::create(111));
    regPtr0->putAll(map0, std::chrono::seconds(15),
                    CacheableInt32::create(1000));
    auto checkPtr =
        std::dynamic_pointer_cast<CacheableInt32>(regPtr0->get(keyObject));
    ASSERT(checkPtr->value() == 111,
           "putAll with entry as object key and value as int  Mismatch");
    map0.clear();
    auto keyObject6 = std::make_shared<PdxTests::PdxTypes3>();
    map0.emplace(keyObject6, CacheableString::create("testString"));
    regPtr0->putAll(map0, std::chrono::seconds(15),
                    CacheableInt32::create(1000));
    auto checkPtr1 = regPtr0->get(keyObject6);
    ASSERT(strcmp(checkPtr1->toString().c_str(), "testString") == 0,
           "strVal should be testString.");
    map0.clear();

    auto keyObject7 = std::make_shared<PdxTests::PdxTypes2>();
    auto valObject = std::make_shared<PdxTests::PdxTypes1>();
    auto keyObject8 = std::make_shared<PdxTests::PdxTypes2>();
    auto valObject2 = std::make_shared<PdxTests::PdxTypes1>();
    map0.emplace(keyObject7, valObject);
    map0.emplace(keyObject8, valObject2);
    regPtr0->putAll(map0, std::chrono::seconds(15),
                    CacheableInt32::create(1000));
    auto objVal = std::dynamic_pointer_cast<PdxTests::PdxTypes1>(
        regPtr0->get(keyObject7));
    ASSERT(valObject == objVal, "valObject and objVal should match.");

    regPtr0->localInvalidateRegion();

    std::vector<std::shared_ptr<CacheableKey>> keys1;
    keys1.push_back(keyObject7);
    keys1.push_back(keyObject8);
    const auto values = regPtr0->getAll(keys1);
    if (values.size() == keys1.size()) {
      char buf[2048];
      for (const auto& iter : values) {
        auto key = std::dynamic_pointer_cast<CacheableKey>(iter.first);
        auto mVal = iter.second;
        if (mVal != nullptr) {
          auto val1 = std::dynamic_pointer_cast<PdxTests::PdxTypes1>(mVal);
          sprintf(buf, "value from map %d , expected value %d ",
                  val1->getm_i1(), 34324);
          LOG(buf);
          ASSERT(val1->getm_i1() == 34324, "value not matched");
        }
      }
    }

    LOG("VerifyPutAllWithObjectKey complete.");
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

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTPUTALLWITHCALLBACK_H_
