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

#define ROOT_NAME "testThinClientHADistOps"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;
using apache::geode::client::Properties;

CacheHelper *cacheHelper = nullptr;
bool isLocalServer = false;

static bool isLocator = false;
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
#include "LocatorHelper.hpp"
static int clientWithRedundancy = 0;
void initClient(int redundancyLevel) {
  if (cacheHelper == nullptr) {
    auto config = Properties::create();
    if (clientWithRedundancy > 0) config->insert("grid-client", "true");
    clientWithRedundancy += 1;
    cacheHelper = new CacheHelper(redundancyLevel, config);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

static int clientWithNothing = 0;
void initClient() {
  if (cacheHelper == nullptr) {
    auto config = Properties::create();
    if (clientWithNothing > 1) config->insert("grid-client", "true");
    clientWithNothing += 1;
    cacheHelper = new CacheHelper(true, config);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

static int clientWithXml = 0;
void initClient(const char *clientXmlFile) {
  if (cacheHelper == nullptr) {
    auto config = Properties::create();
    if (clientWithXml > 2) config->insert("grid-client", "true");
    clientWithXml += 1;
    cacheHelper = new CacheHelper(nullptr, clientXmlFile, config);
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
                  bool noKey) {
  // Verify key and value exist in this region, in this process.
  const char *value = val ? val : "";
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
  if (noKey == false) {  // need to find the key!
    ASSERT(regPtr->containsKey(keyPtr), "Key not found in region.");
  }
  if (val != nullptr) {  // need to have a value!
    ASSERT(regPtr->containsValueForKey(keyPtr), "Value not found in region.");
  }

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
}

void _verifyInvalid(const char *name, const char *key, int line) {
  LOG(std::string("verifyInvalid() called from ") + std::to_string(line) +
      "\n");
  _verifyEntry(name, key, nullptr, false);
  LOG("Entry invalidated.");
}

#define verifyDestroyed(x, y) _verifyDestroyed(x, y, __LINE__)

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

void destroyEntry(const char *name, const char *key) {
  LOG("destroyEntry() entered.");
  std::cout << "Destroying entry -- key: " << key << " in region " << name
            << "\n"
            << std::flush;
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

void createRegion(const char *name, bool ackMode,
                  bool clientNotificationEnabled = false) {
  LOG("createRegion() entered.");
  std::cout << "Creating region --  " << name << " ackMode is " << ackMode
            << "\n"
            << std::flush;
  char *endpoints = nullptr;
  auto regPtr = getHelper()->createRegion(name, ackMode, true, nullptr,
                                          endpoints, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}

void createPooledRegion(const char *name, bool ackMode, const char *locators,
                        const char *poolname, int reduendency = 1,
                        bool clientNotificationEnabled = false,
                        bool cachingEnable = true) {
  LOG("createRegion_Pool() entered.");
  std::cout << "Creating region --  " << name << " ackMode is " << ackMode
            << "\n"
            << std::flush;
  auto poolPtr = getHelper()->createPool(
      poolname, locators, nullptr, reduendency, clientNotificationEnabled);
  auto regPtr = getHelper()->createRegionAndAttachPool(name, ackMode, poolname,
                                                       cachingEnable);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Pooled Region created.");
}
void createEntry(const char *name, const char *key, const char *value) {
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

void updateEntry(const char *name, const char *key, const char *value) {
  LOG("updateEntry() entered.");
  std::cout << "Creating entry -- key: " << key << " value: " << value
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

void doGetAgain(const char *name, const char *key, const char *value) {
  LOG("doGetAgain() entered.");
  std::cout << "get for entry -- key: " << key << " expecting value: " << value
            << " in region " << name << "\n"
            << std::flush;
  // Get entry created in Process A, verify entry is correct
  auto keyPtr = CacheableKey::create(key);

  auto regPtr = getHelper()->getRegion(name);
  std::cout << "get  region name " << regPtr->getName() << "\n" << std::flush;
  ASSERT(regPtr != nullptr, "Region not found.");

  auto checkPtr = std::dynamic_pointer_cast<CacheableString>(
      regPtr->get(keyPtr));  // force a netsearch

  if (checkPtr != nullptr) {
    LOG("checkPtr is not null");
    LOG(std::string("In doGetAgain, get returned ") + checkPtr->value() +
        " for key  " + key);
  } else {
    LOG("checkPtr is nullptr");
  }
  verifyEntry(name, key, value);
  LOG("GetAgain complete.");
}

void doNetsearch(const char *name, const char *key, const char *value) {
  LOG("doNetsearch() entered.");
  std::cout << "Netsearching for entry -- key: " << key
            << " expecting value: " << value << " in region " << name << "\n"
            << std::flush;
  static int count = 0;
  // Get entry created in Process A, verify entry is correct
  auto keyPtr = CacheableKey::create(key);

  auto regPtr = getHelper()->getRegion(name);
  std::cout << "netsearch region " << regPtr->getName() << "\n" << std::flush;
  ASSERT(regPtr != nullptr, "Region not found.");

  if (count == 0) {
    ASSERT(!regPtr->containsKey(keyPtr),
           "Key should not have been found in region.");
    ASSERT(!regPtr->containsValueForKey(keyPtr),
           "Value should not have been found in region.");
    count++;
  }
  auto checkPtr = std::dynamic_pointer_cast<CacheableString>(
      regPtr->get(keyPtr));  // force a netsearch

  if (checkPtr != nullptr) {
    LOG("checkPtr is not null");
    LOG(std::string("In net search, get returned ") + checkPtr->value() +
        " for key  " + key);
  } else {
    LOG("checkPtr is nullptr");
  }
  verifyEntry(name, key, value);
  LOG("Netsearch complete.");
}

const char *keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char *vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char *nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const char *regionName = "DistRegionAck";

const bool USE_ACK = true;
const bool NO_ACK = false;

DUNIT_TASK_DEFINITION(CLIENT1, InitClient1)
  { initClient(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, InitClient2)
  { initClient(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CreateClient1PoolAndRegions)
  {
    getHelper()->createPoolWithLocators("__TESTPOOL1_", locatorsG, true, 1);

    getHelper()->createRegionAndAttachPool(regionName, USE_ACK, "__TESTPOOL1_",
                                           true);

    createEntry(regionName, keys[0], vals[0]);

    LOG("CreateClient1PoolAndRegions complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CreateClient2PoolAndRegions)
  {
    getHelper()->createPoolWithLocators("__TESTPOOL1_", locatorsG, true, 1);

    getHelper()->createRegionAndAttachPool(regionName, USE_ACK, "__TESTPOOL1_",
                                           true);

    createEntry(regionName, keys[1], vals[1]);

    LOG("CreateClient2PoolAndRegions complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, RegisterClient1Keys)
  {
    auto reg0 = getHelper()->getRegion(regionName);
    auto vec0 = reg0->serverKeys();
    ASSERT(vec0.size() == 2, "Should have 2 keys in first region.");
    std::string key0, key1;
    key0 = vec0[0]->toString().c_str();
    key1 = vec0[1]->toString().c_str();
    ASSERT(key0 != key1, "The two keys should be different in first region.");
    ASSERT(key0 == keys[0] || key0 == keys[1],
           "Unexpected key in first region.");
    ASSERT(key1 == keys[0] || key1 == keys[1],
           "Unexpected key in first region.");

    doNetsearch(regionName, keys[1], vals[1]);

    auto keyPtr1 = CacheableKey::create(keys[1]);

    auto regPtr0 = getHelper()->getRegion(regionName);

    std::vector<std::shared_ptr<CacheableKey>> keys0, keys1;
    keys0.push_back(keyPtr1);
    regPtr0->registerKeys(keys0);
    LOG("RegisterClient1Keys complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, RegisterClient2Keys)
  {
    doNetsearch(regionName, keys[0], vals[0]);

    auto keyPtr0 = CacheableKey::create(keys[0]);
    auto keyPtr2 = CacheableKey::create(keys[2]);

    auto regPtr0 = getHelper()->getRegion(regionName);

    std::vector<std::shared_ptr<CacheableKey>> keys0, keys1;
    keys0.push_back(keyPtr0);
    keys1.push_back(keyPtr2);
    regPtr0->registerKeys(keys0);
    LOG("RegisterClient2Keys complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, UpdateClient1Entries)
  {
    updateEntry(regionName, keys[0], nvals[0]);
    LOG("UpdateClient1Entries complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyAndUpdateClient2Entries)
  {
    verifyEntry(regionName, keys[0], nvals[0]);

    updateEntry(regionName, keys[1], nvals[1]);
    LOG("VerifyAndUpdateClient2Entries complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, VerifyClient1Entries)
  { verifyEntry(regionName, keys[1], nvals[1]); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, DoAbsolutelyNothing)
  {}
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, DestroyAllKeys)
  {
    destroyEntry(regionName, keys[0]);
    destroyEntry(regionName, keys[1]);
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

DUNIT_MAIN
  {
    /* Starting Server*/
    CALL_TASK(CreateLocator1);
    CALL_TASK(CreateServer1_With_Locator_XML);
    CALL_TASK(CreateServer2_With_Locator_XML);

    /* Client init*/
    CALL_TASK(InitClient1);
    CALL_TASK(InitClient2);

    CALL_TASK(CreateClient1PoolAndRegions);
    CALL_TASK(CreateClient2PoolAndRegions);

    CALL_TASK(RegisterClient1Keys);
    CALL_TASK(RegisterClient2Keys);
    CALL_TASK(UpdateClient1Entries);
    CALL_TASK(VerifyAndUpdateClient2Entries);
    CALL_TASK(VerifyClient1Entries);
    CALL_TASK(DoAbsolutelyNothing);
    CALL_TASK(DestroyAllKeys);

    CALL_TASK(CloseServer1);
    CALL_TASK(CloseServer2);

    CALL_TASK(CloseCache1);
    CALL_TASK(CloseCache2);

    CALL_TASK(CloseLocator1);
  }
END_MAIN
