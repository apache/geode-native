#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTNOTIFICATION_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTNOTIFICATION_H_

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
#include <string>

#define ROOT_NAME "ThinClientNotification"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"

namespace {  // NOLINT(google-build-namespaces)

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;

CacheHelper* cacheHelper = nullptr;

#include "locator_globals.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#include "LocatorHelper.hpp"
void initClient(const bool isthinClient) {
  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(isthinClient, "__TEST_POOL1__", locatorsG,
                                  nullptr, nullptr, 0, true);
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
                  bool noKey) {
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

  LOG("Before contains key");
  // if the region is no ack, then we may need to wait...
  if (noKey == false) {  // need to find the key!
    ASSERT(regPtr->containsKey(keyPtr), "Key not found in region.");
  }
  LOG("after contains key");
  // loop up to MAX times, testing condition
  uint32_t MAX = 100;
  uint32_t SLEEP = 10;  // milliseconds
  uint32_t containsKeyCnt = 0;
  uint32_t containsValueCnt = 0;
  uint32_t testValueCnt = 0;

  for (int i = MAX; i >= 0; i--) {
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
      LOG(" checkin val");
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
    dunit::sleep(SLEEP);
  }
  LOG(" verify done");
}

#define verifyInvalid(x, y) _verifyInvalid(x, y, __LINE__)

void _verifyInvalid(const char* name, const char* key, int line) {
  LOG(std::string("verifyInvalid() called from ") + std::to_string(line) +
      "\n");
  _verifyEntry(name, key, nullptr, false);
  LOG("Entry invalidated.");
}

#define verifyDestroyed(x, y) _verifyDestroyed(x, y, __LINE__)

void _verifyDestroyed(const char* name, const char* key, int line) {
  LOG(std::string("verifyDestroyed() called from ") + std::to_string(line) +
      "\n");
  _verifyEntry(name, key, nullptr, true);
  LOG("Entry destroyed.");
}

#define verifyEntry(x, y, z) _verifyEntry(x, y, z, __LINE__)

void _verifyEntry(const char* name, const char* key, const char* val,
                  int line) {
  LOG(std::string("verifyEntry() called from ") + std::to_string(line) + "\n");
  _verifyEntry(name, key, val, false);
  LOG("Entry verified.");
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

void createEntry(const char* name, const char* key, const char* value) {
  LOG("createEntry() entered.");
  std::cout << "Creating entry -- key: " << key << " value: " << value
            << " in region " << name << "\n"
            << std::flush;
  // Create entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableString::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(!regPtr->containsKey(keyPtr),
         "Key should not have been found in region.");
  ASSERT(!regPtr->containsValueForKey(keyPtr),
         "Value should not have been found in region.");

  // regPtr->create( keyPtr, valPtr );
  regPtr->put(keyPtr, valPtr);
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
  /*NIL: Changed the asserion due to the change in invalidate.
    Now we create new entery for every invalidate event received or
    localInvalidate call
    so expect  containsKey to returns true insted of false earlier. */
  ASSERT(regPtr->containsKey(keyPtr), "Key should  have been found in region.");
  ASSERT(!regPtr->containsValueForKey(keyPtr),
         "Value should not have been found in region.");

  auto theValue = regPtr->get(keyPtr);
  auto checkPtr = std::dynamic_pointer_cast<CacheableString>(
      theValue);  // force a netsearch

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

void invalidateEntry(const char* name, const char* key) {
  LOG("invalidateEntry() entered.");
  std::cout << "Invalidating entry -- key: " << key << " in region " << name << "\n" << std::flush;
  // Invalidate entry, verify entry is invalidated
  auto keyPtr = CacheableKey::create(key);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(regPtr->containsKey(keyPtr), "Key should have been found in region.");
  ASSERT(regPtr->containsValueForKey(keyPtr),
         "Value should have been found in region.");

  regPtr->localInvalidate(keyPtr);
  LOG("Invalidate entry.");

  verifyInvalid(name, key);
  LOG("Entry invalidated.");
}

void destroyEntry(const char* name, const char* key) {
  LOG("destroyEntry() entered.");
  std::cout << "Destroying entry -- key: " << key << " in region " << name << "\n" << std::flush;
  // Destroy entry, verify entry is destroyed
  auto keyPtr = CacheableKey::create(key);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(regPtr->containsKey(keyPtr), "Key should have been found in region.");

  regPtr->destroy(keyPtr);
  LOG("Destroy entry.");

  verifyDestroyed(name, key);
  LOG("Entry destroyed.");
}

void destroyRegion(const char* name) {
  LOG("destroyRegion() entered.");
  auto regPtr = getHelper()->getRegion(name);
  regPtr->localDestroyRegion();
  LOG("Region destroyed.");
}

const char* keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char* vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char* nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const char* regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

const bool USE_ACK = true;
const bool NO_ACK = false;

DUNIT_TASK_DEFINITION(CLIENT1, CreateClient1)
  { initClient(true); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CreateClient2)
  { initClient(true); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CreateRegions1_PoolLocators)
  {
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TESTPOOL1_",
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TESTPOOL1_", true);
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    regPtr->registerAllKeys(false, false, false);
    regPtr = getHelper()->getRegion(regionNames[1]);
    regPtr->registerAllKeys(false, false, false);
    LOG("CreateRegions1_PoolLocators complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CreateRegions2_PoolLocators)
  {
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TESTPOOL1_",
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TESTPOOL1_", true);
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    regPtr->registerAllKeys(false, false, false);
    regPtr = getHelper()->getRegion(regionNames[1]);
    regPtr->registerAllKeys(false, false, false);
    LOG("CreateRegions2_PoolLocators complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CreateEvenValues)
  {
    createEntry(regionNames[0], keys[0], vals[0]);
    createEntry(regionNames[1], keys[2], vals[2]);
    LOG("CreateEvenValues complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, FindEvenAndCreateOddValues)
  {
    doNetsearch(regionNames[0], keys[0], vals[0]);
    doNetsearch(regionNames[1], keys[2], vals[2]);
    createEntry(regionNames[0], keys[1], vals[1]);
    createEntry(regionNames[1], keys[3], vals[3]);
    LOG("FindEvenAndCreateOddValues complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, FindOddAndUpdateEvenValues)
  {
    doNetsearch(regionNames[0], keys[1], vals[1]);
    doNetsearch(regionNames[1], keys[3], vals[3]);
    updateEntry(regionNames[0], keys[0], nvals[0]);
    updateEntry(regionNames[1], keys[2], nvals[2]);
    LOG("FindOddAndUpdateEvenValues complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyEvenAndUpdateOddValues)
  {
    LOG("VerifyEvenAndUpdateOddValues started");
    verifyEntry(regionNames[0], keys[0], nvals[0]);
    verifyEntry(regionNames[1], keys[2], nvals[2]);
    updateEntry(regionNames[0], keys[1], nvals[1]);
    updateEntry(regionNames[1], keys[3], nvals[3]);
    LOG("VerifyEvenAndUpdateOddValues complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, VerifyOddExistenceAndValuesAndInvalidateEven)
  {
    verifyInvalid(regionNames[0], keys[1]);
    verifyInvalid(regionNames[1], keys[3]);
    verifyEntry(regionNames[0], keys[1], nvals[1]);
    verifyEntry(regionNames[1], keys[3], nvals[3]);
    invalidateEntry(regionNames[0], keys[0]);
    invalidateEntry(regionNames[1], keys[2]);
    LOG("VerifyFirstTwoExistenceAndValuesAndInvalidate complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyEvenValuesAndInvalidateOdd)
  {
    verifyEntry(regionNames[0], keys[0], nvals[0]);
    verifyEntry(regionNames[1], keys[2], nvals[2]);
    invalidateEntry(regionNames[0], keys[1]);
    invalidateEntry(regionNames[1], keys[3]);
    LOG("VerifyEvenValuesAndInvalidateOdd complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, VerifyOddAndDestroyEven)
  {
    verifyEntry(regionNames[0], keys[1], nvals[1]);
    verifyEntry(regionNames[1], keys[3], nvals[3]);
    destroyEntry(regionNames[0], keys[0]);
    destroyEntry(regionNames[1], keys[2]);
    LOG("VerifyOddAndDestroyEven complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyEvenDestructionAndDestroyOdd)
  {
    verifyDestroyed(regionNames[0], keys[0]);
    verifyDestroyed(regionNames[1], keys[2]);
    destroyEntry(regionNames[0], keys[1]);
    destroyEntry(regionNames[1], keys[3]);
    LOG("VerifyEvenDestructionAndDestroyOdd complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, VerifyOddDestructionAndDestroyRegions)
  {
    verifyDestroyed(regionNames[0], keys[1]);
    verifyDestroyed(regionNames[1], keys[3]);
    destroyRegion(regionNames[0]);
    destroyRegion(regionNames[1]);
    LOG("VerifyOddDestructionAndDestroyRegions complete.");
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

void doThinClientNotification() {
  CALL_TASK(CreateLocator1);
  CALL_TASK(CreateServer1_With_Locator);

  CALL_TASK(CreateClient1);
  CALL_TASK(CreateClient2);

  CALL_TASK(CreateRegions1_PoolLocators);
  CALL_TASK(CreateRegions2_PoolLocators);

  CALL_TASK(CreateEvenValues);
  CALL_TASK(FindEvenAndCreateOddValues);
  CALL_TASK(FindOddAndUpdateEvenValues);
  CALL_TASK(VerifyEvenAndUpdateOddValues);
  CALL_TASK(VerifyOddExistenceAndValuesAndInvalidateEven);
  CALL_TASK(VerifyEvenValuesAndInvalidateOdd);
  CALL_TASK(VerifyOddAndDestroyEven);
  CALL_TASK(VerifyEvenDestructionAndDestroyOdd);
  CALL_TASK(VerifyOddDestructionAndDestroyRegions);

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer1);
  CALL_TASK(CloseLocator1);
}

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTNOTIFICATION_H_
