#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTPUTALL_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTPUTALL_H_

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
#include <sstream>
#include <iomanip>

#include "fw_dunit.hpp"
#include "testobject/PdxType.hpp"
#include "testobject/VariousPdxTypes.hpp"

#include "SerializationRegistry.hpp"
#include "CacheRegionHelper.hpp"
#include "CacheImpl.hpp"
#define ROOT_NAME "ThinClientPutAll"
#define ROOT_SCOPE DISTRIBUTED_ACK
#include "CacheHelper.hpp"

namespace {  // NOLINT(google-build-namespaces)

using apache::geode::client::Cache;
using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableInt64;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheHelper;
using apache::geode::client::CacheRegionHelper;
using apache::geode::client::Exception;
using apache::geode::client::HashMapOfCacheable;
using apache::geode::client::IllegalArgumentException;

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2

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

void _verifyEntry(const char* name, const char* key, const char* val,
                  bool noKey, bool isCreated = false) {
  // Verify key and value exist in this region, in this process.
  const char* value = val ? val : "";
  std::string msg;
  if (noKey) {
    msg =
        std::string("Verify key ") + key + " does not exist in region " + name;
  } else if (!val) {
    msg = std::string("Verify value for key ") + key +
          " does not exist in region " + name;
  } else {
    msg = std::string("Verify value for key ") + key + " is: " + value +
          " in region " + name;
  }
  LOG(msg);

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
        LOG("In verify loop, get returned " + checkPtr->value() + " for key " +
            key);
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
  LOG(std::string("verifyEntry() called from ") + std::to_string(line) + "\n");
  _verifyEntry(name, key, val, false);
  LOG("Entry verified.");
}

#define verifyCreated(x, y) _verifyCreated(x, y, __LINE__)

void _verifyCreated(const char* name, const char* key, int line) {
  LOG(std::string("verifyCreated() called from ") + std::to_string(line) +
      "\n");
  _verifyEntry(name, key, nullptr, false, true);
  LOG("Entry created.");
}
void createRegion(const char* name, bool ackMode, bool isCacheEnabled,
                  bool clientNotificationEnabled = false) {
  LOG("createRegion() entered.");
  std::cout << "Creating region --  " << name << " ackMode is " << ackMode
            << "\n"
            << std::flush;
  auto regPtr = getHelper()->createRegion(name, ackMode, isCacheEnabled,
                                          nullptr, clientNotificationEnabled);
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
    const std::string& name, bool ackMode, const std::string& locators,
    const std::string& poolname, bool clientNotificationEnabled = false,
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

void createEntry(const char* name, const char* key,
                 const char* value = nullptr) {
  LOG("createEntry() entered.");
  std::cout << "Creating entry -- key: " << key << " value: " << value
            << " in region " << name << "\n"
            << std::flush;
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
  std::cout << "Updating entry -- key: " << key << " value: " << value
            << " in region " << name << "\n"
            << std::flush;
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
  std::cout << "Netsearching for entry -- key: " << key
            << " expecting value: " << value << " in region " << name << "\n"
            << std::flush;
  // Get entry created in Process A, verify entry is correct
  auto keyPtr = CacheableKey::create(key);

  auto regPtr = getHelper()->getRegion(name);
  std::cout << "netsearch region " << regPtr->getName() << "\n" << std::flush;
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
    LOG(std::string("In net search, get returned ") + checkPtr->value() +
        " for key " + key);
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

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pooled_Locator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, poolName, true,
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, poolName, true, true);
    LOG("StepOne complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pooled_Locator_NoCaching)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, poolName, false,
                       false);
    LOG("StepOne_Pooled_Locator_NoCaching complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pooled_Locator_ConcurrencyCheckDisabled)
  {
    initClient(true);
    createPooledRegionConcurrencyCheckDisabled(
        regionNames[0], USE_ACK, locatorsG, poolName, true, true, false);
    LOG("StepOne_Pooled_Locator_ConcurrencyCheckDisabled complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo_Pooled_Locator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, poolName, true,
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, poolName, true, true);
    LOG("StepOne complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
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

    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepFour)
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
    regPtr0->putAll(map0);
    regPtr1->putAll(map1);
    LOG("StepFour complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, PutAllOps)
  {
    HashMapOfCacheable entryMap;
    entryMap.clear();
    for (int32_t item = 0; item < 500; item++) {
      entryMap.emplace(
          CacheableKey::create(std::string("key-") + std::to_string(item)),
          CacheableString::create(std::to_string(item)));
    }

    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    regPtr0->putAll(entryMap);
    LOG("putAll1 complete");

    std::vector<std::shared_ptr<CacheableKey>> getAllkeys;
    for (int32_t item = 0; item < 500; item++) {
      getAllkeys.push_back(
          CacheableKey::create(std::string("key-") + std::to_string(item)));
    }

    const auto values = regPtr0->getAll(getAllkeys);
    ASSERT(values.size() == 500, "GetAll should return 500 entries.");

    LOG("PutAllOps complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFive)
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

    LOG("StepFive complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepSix)
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
    regPtr0->putAll(map0);
    regPtr1->putAll(map1);

    // register all keys for the large putall test case
    regPtr0->registerAllKeys();

    LOG("StepSix complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepSeven)
  {
    verifyEntry(regionNames[0], keys[0], nvals[0]);
    verifyEntry(regionNames[0], keys[1], nvals[1]);
    // region1 is not changed at client
    verifyEntry(regionNames[1], keys[2], nvals[2]);
    verifyEntry(regionNames[1], keys[3], nvals[3]);
    LOG("StepSeven complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepEight)
  {
    LOG("Do large PutAll");
    HashMapOfCacheable map0;
    map0.clear();
    for (int i = 0; i < 100000; i++) {
      auto key0 = std::string("key-") + std::to_string(i);
      std::ostringstream val0;
      val0 << std::left << std::setfill('0') << std::setw(1000) << i;
      map0.emplace(CacheableKey::create(key0),
                   CacheableString::create(val0.str()));
    }

    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    regPtr0->putAll(map0, std::chrono::seconds(40000));

    LOG("StepEight complete.");
    dunit::sleep(10000);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepNine)
  {
    LOG("Verify large PutAll");
    for (int i = 0; i < 100000; i++) {
      auto key0 = std::string("key-") + std::to_string(i);
      verifyCreated(regionNames[0], key0.c_str());
    }
    LOG("StepNine complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepTen)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    auto& rsp = regPtr0->getRegionService();
    auto regPtr = rsp.getRegion(regionNames[0]);
    ASSERT(regPtr != nullptr, "Failed to get region.");

    auto& rsp1 = regPtr0->getRegionService();
    auto regPtr1 = rsp1.getRegion("NOT_CREATED_REGION");
    ASSERT(regPtr1 == nullptr, "Unknown Region Returned");

    LOG("StepTen complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepEleven)
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

    LOG("StepEleven complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThirteen)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    HashMapOfCacheable map0;
    for (int i = 0; i < 2; i++) {
      map0.emplace(CacheableInt64::create(i), CacheableInt64::create(i));
    }
    regPtr0->putAll(map0);
    for (int i = 0; i < 2; i++) {
      auto checkPtr = std::dynamic_pointer_cast<CacheableInt32>(
          regPtr0->get(CacheableInt64::create(i)));
      ASSERT(checkPtr->value() == i,
             "putAll entry with long key and long value Mismatch.");
    }
    map0.clear();
    const char* testValues[] = {"Value-100", "Value-200"};

    for (int i = 80; i < 82; i++) {
      map0.emplace(CacheableInt64::create(i),
                   CacheableString::create(testValues[i - 80]));
    }
    regPtr0->putAll(map0);
    for (int i = 80; i < 82; i++) {
      auto checkPtr = std::dynamic_pointer_cast<CacheableString>(
          regPtr0->get(CacheableInt64::create(i)));
      ASSERT(strcmp(checkPtr->value().c_str(), testValues[i - 80]) == 0,
             "putAll entry with long key and string value  Mismatch");
    }
    map0.clear();

    auto val111 = std::make_shared<PdxTests::PdxTypes1>();
    map0.emplace(CacheableInt32::create(1211), val111);
    regPtr0->putAll(map0);
    auto retObj = std::dynamic_pointer_cast<PdxTests::PdxTypes1>(
        regPtr0->get(CacheableInt32::create(1211)));
    ASSERT(val111->equals(retObj) == true, "val111 and retObj should match.");
    map0.clear();

    auto keyObject = std::make_shared<PdxTests::PdxType>();
    map0.emplace(keyObject, CacheableInt32::create(111));
    regPtr0->putAll(map0);
    auto checkPtr =
        std::dynamic_pointer_cast<CacheableInt32>(regPtr0->get(keyObject));
    ASSERT(checkPtr->value() == 111,
           "putAll with entry as object key and value as int  Mismatch");
    map0.clear();
    auto keyObject6 = std::make_shared<PdxTests::PdxTypes3>();
    map0.emplace(keyObject6, CacheableString::create("testString"));
    regPtr0->putAll(map0);
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
    regPtr0->putAll(map0);
    auto objVal = std::dynamic_pointer_cast<PdxTests::PdxTypes1>(
        regPtr0->get(keyObject7));
    ASSERT(valObject == objVal, "valObject and objVal should match.");
    map0.clear();

    try {
      map0.emplace(CacheableInt64::create(345),
                   CacheableInt64::create(3465987));
      regPtr0->putAll(map0, std::chrono::seconds(-1));
      auto check = std::dynamic_pointer_cast<CacheableInt64>(
          regPtr0->get(CacheableInt64::create(345)));
      ASSERT(check->value() == 3465987,
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
      regPtr0->putAll(map0, std::chrono::seconds(2147500));
      auto check = std::dynamic_pointer_cast<CacheableInt64>(
          regPtr0->get(CacheableInt64::create(3451)));
      ASSERT(check->value() == 3465987,
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

    regPtr0->localInvalidateRegion();

    std::vector<std::shared_ptr<CacheableKey>> keys1;
    keys1.push_back(keyObject7);
    keys1.push_back(keyObject8);
    const auto values = regPtr0->getAll(keys1);
    if (values.size() == keys1.size()) {
      char buf[2048];
      for (const auto& iter : values) {
        const auto key = std::dynamic_pointer_cast<CacheableKey>(iter.first);
        const auto& mVal = iter.second;
        if (mVal != nullptr) {
          auto val1 = std::dynamic_pointer_cast<PdxTests::PdxTypes1>(mVal);
          LOG(std::string("value from map ") + std::to_string(val1->getm_i1()) +
              ", expected value 34324");
          ASSERT(val1->getm_i1() == 34324, "value not matched");
        }
      }
    }
    LOG("StepThirteen complete.");
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

void runPutAll() {
  CALL_TASK(CreateLocator1);
  CALL_TASK(CreateServer1_With_Locator_XML);
  CALL_TASK(CreateServer2_With_Locator_XML);

  CALL_TASK(StepOne_Pooled_Locator);
  CALL_TASK(StepTwo_Pooled_Locator);

  CALL_TASK(StepThree);
  CALL_TASK(StepFour);
  CALL_TASK(StepFive);
  CALL_TASK(StepSix);
  CALL_TASK(StepSeven);
  CALL_TASK(StepEight);
  CALL_TASK(StepNine);
  CALL_TASK(StepTen);
  CALL_TASK(StepEleven);
  CALL_TASK(StepThirteen);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer1);
  CALL_TASK(CloseServer2);

  CALL_TASK(CloseLocator1);
}

void runPutAll1(bool concurrencyCheckEnabled = true) {
  CALL_TASK(CreateLocator1);
  CALL_TASK(CreateServer1_With_Locator_XML);
  CALL_TASK(CreateServer2_With_Locator_XML);

  if (!concurrencyCheckEnabled) {
    CALL_TASK(StepOne_Pooled_Locator_NoCaching);
  } else {
    CALL_TASK(StepOne_Pooled_Locator_ConcurrencyCheckDisabled);
  }

  CALL_TASK(PutAllOps);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseServer1);
  CALL_TASK(CloseServer2);
  CALL_TASK(CloseLocator1);
}

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTPUTALL_H_
