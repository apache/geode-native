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

#define ROOT_NAME "testThinClientIntResPolKeysInv"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;

CacheHelper *cacheHelper = nullptr;

#include "locator_globals.hpp"
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

CacheHelper *getHelper() {
  ASSERT(cacheHelper != nullptr, "No cacheHelper initialized.");
  return cacheHelper;
}

void _verifyEntry(const char *name, const char *key, const char *val,
                  bool noKey, bool isCreated = false) {
  // Verify key and value exist in this region, in this process.
  const char *value = val ? val : "";
  std::string msg;
  if (!isCreated) {
    if (noKey) {
      msg = std::string("Verify key ") + key + " does not exist in region " +
            name;
    } else if (!val) {
      msg = std::string("Verify value for key ") + key +
            " does not exist in region " + name;
    } else {
      msg = std::string("Verify value for key ") + key + " is: " + value +
            " in region " + name;
    }
    LOG(msg);
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

#define verifyInvalid(x, y) _verifyInvalid(x, y, __LINE__)

void _verifyInvalid(const char *name, const char *key, int line) {
  LOG(std::string("verifyInvalid() called from ") + std::to_string(line) +
      "\n");
  _verifyEntry(name, key, nullptr, false);
  LOG("Entry invalidated.");
}

void _verifyDestroyed(const char *name, const char *key, int line) {
  LOG(std::string("verifyDestroyed() called from ") + std::to_string(line) +
      "\n");
  _verifyEntry(name, key, nullptr, true);
  LOG("Entry destroyed.");
}

#define verifyEntry(x, y, z) _verifyEntry(x, y, z, __LINE__)

void _verifyEntry(const char *name, const char *key, const char *val,
                  int line) {
  LOG(std::string("verifyEntry() called from ") + std::to_string(line) + "\n");
  _verifyEntry(name, key, val, false);
  LOG("Entry verified.");
}

void _verifyCreated(const char *name, const char *key, int line) {
  LOG(std::string("verifyCreated() called from ") + std::to_string(line) +
      "\n");
  _verifyEntry(name, key, nullptr, false, true);
  LOG("Entry created.");
}

void createRegion(const char *name, bool ackMode, const char *endpoints,
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
void createEntry(const char *name, const char *key,
                 const char *value = nullptr) {
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

void updateEntry(const char *name, const char *key, const char *value) {
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

void doNetsearch(const char *name, const char *key, const char *value) {
  LOG("doNetsearch() entered.");
  std::cout << "Netsearching for entry -- key: " << key << " value: " << value
            << " in region " << name << "\n"
            << std::flush;
  // Get entry created in Process A, verify entry is correct
  auto keyPtr = CacheableKey::create(key);

  auto regPtr = getHelper()->getRegion(name);
  std::cout << "netsearch  region " << regPtr->getName() << "\n" << std::flush;
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

const char *regex23 = "Key-[2-3]";
const char *regexWildcard = "Key-.*";
const char *keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char *regionNames[] = {"DistRegionAck", "DistRegionNoAck"};
const char *vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char *nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const bool USE_ACK = true;
const bool NO_ACK = false;

DUNIT_TASK_DEFINITION(CLIENT1, StepOne)
  {
    initClient(true);
    getHelper()->createPooledRegion(regionNames[0], USE_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);
    getHelper()->createPooledRegion(regionNames[1], NO_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);
    LOG("StepOne complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo)
  {
    initClient(true);
    getHelper()->createPooledRegion(regionNames[0], USE_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);
    getHelper()->createPooledRegion(regionNames[1], NO_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);
    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    createEntry(regionNames[1], keys[0], vals[0]);
    createEntry(regionNames[1], keys[1], vals[1]);
    createEntry(regionNames[1], keys[2], vals[2]);
    createEntry(regionNames[1], keys[3], vals[3]);
    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepFour)
  {
    createEntry(regionNames[0], keys[0], vals[0]);
    createEntry(regionNames[0], keys[1], vals[1]);
    createEntry(regionNames[0], keys[2], vals[2]);
    createEntry(regionNames[0], keys[3], vals[3]);
    LOG("StepFour complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepFive)
  {
    doNetsearch(regionNames[1], keys[0], vals[0]);
    doNetsearch(regionNames[1], keys[1], vals[1]);
    doNetsearch(regionNames[1], keys[2], vals[2]);
    doNetsearch(regionNames[1], keys[3], vals[3]);
    LOG("StepFive complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepSix)
  {
    doNetsearch(regionNames[0], keys[0], vals[0]);
    doNetsearch(regionNames[0], keys[1], vals[1]);
    doNetsearch(regionNames[0], keys[2], vals[2]);
    doNetsearch(regionNames[0], keys[3], vals[3]);
    LOG("StepSix complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepSeven)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    regPtr0->registerRegex(regex23);

    verifyInvalid(regionNames[0], keys[1]);
    verifyInvalid(regionNames[0], keys[2]);

    verifyEntry(regionNames[0], keys[0], vals[0]);
    verifyEntry(regionNames[0], keys[3], vals[3]);

    LOG("StepSeven complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepEight)
  {
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);
    regPtr1->registerAllKeys();

    verifyInvalid(regionNames[1], keys[0]);
    verifyInvalid(regionNames[1], keys[1]);
    verifyInvalid(regionNames[1], keys[2]);
    verifyInvalid(regionNames[1], keys[3]);

    updateEntry(regionNames[0], keys[0], nvals[0]);
    updateEntry(regionNames[0], keys[1], nvals[1]);
    updateEntry(regionNames[0], keys[2], nvals[2]);
    updateEntry(regionNames[0], keys[3], nvals[3]);

    LOG("StepEight complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepNine)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    regPtr0->unregisterRegex(regex23);

    auto keyptr0 = CacheableKey::create(keys[0]);
    auto keyptr1 = CacheableKey::create(keys[1]);
    auto keyptr2 = CacheableKey::create(keys[2]);
    std::vector<std::shared_ptr<CacheableKey>> keylist;
    keylist.push_back(keyptr0);
    keylist.push_back(keyptr1);
    keylist.push_back(keyptr2);

    regPtr0->registerKeys(keylist);

    verifyInvalid(regionNames[0], keys[0]);
    verifyInvalid(regionNames[0], keys[1]);
    verifyInvalid(regionNames[0], keys[2]);

    LOG("StepNine complete.");
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

DUNIT_MAIN
  {
    CALL_TASK(CreateLocator1);
    CALL_TASK(CreateServer1_With_Locator);
    CALL_TASK(StepOne);
    CALL_TASK(StepTwo);
    CALL_TASK(StepThree);
    CALL_TASK(StepFour);
    CALL_TASK(StepFive);
    CALL_TASK(StepSix);
    CALL_TASK(StepSeven);
    CALL_TASK(StepEight);
    CALL_TASK(StepNine);
    CALL_TASK(CloseCache1);
    CALL_TASK(CloseCache2);
    CALL_TASK(CloseServer1);
    CALL_TASK(CloseLocator1);
  }
END_MAIN
