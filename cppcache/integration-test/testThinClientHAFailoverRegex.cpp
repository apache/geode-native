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

#define ROOT_NAME "testThinClientHAFailoverRegex"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"

bool isLocalServer = false;

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;
using apache::geode::client::Properties;

CacheHelper *cacheHelper = nullptr;
volatile int g_redundancyLevel = 0;

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2

static bool isLocator = false;
static int numberOfLocators = 1;
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);

void initClient() {
  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(true);
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
      // ASSERT( regPtr->containsValueForKey( keyPtr ), "Value not found in
      // region." );
    }
  }

  // loop up to MAX times, testing condition
  uint32_t MAX = 100;
  //  changed sleep from 10 ms
  uint32_t SLEEP = 10;  // milliseconds
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

void _verifyEntry(const char *name, const char *key, const char *val,
                  int line) {
  LOG(std::string("verifyEntry() called from ") + std::to_string(line) + "\n");
  _verifyEntry(name, key, val, false);
  LOG("Entry verified.");
}

#define verifyCreated(x, y) _verifyCreated(x, y, __LINE__)

void _verifyCreated(const char *name, const char *key, int line) {
  LOG(std::string("verifyCreated() called from ") + std::to_string(line) +
      "\n");
  _verifyEntry(name, key, nullptr, false, true);
  LOG("Entry created.");
}

void createRegion(const char *name, bool ackMode,
                  bool clientNotificationEnabled = false) {
  LOG("createRegion() entered.");
  std::cout << "Creating region --  " << name << " ackMode is " << ackMode
            << "\n"
            << std::flush;
  // ack, caching
  auto regPtr = getHelper()->createRegion(name, ackMode, true, nullptr,
                                          clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
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
  std::cout << "Updating entry -- key: " << key << " value: " << value
            << " in region " << name << "\n"
            << std::flush;
  // Update entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableString::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(regPtr->containsKey(keyPtr), "Key should have been found in region.");
  // ASSERT( regPtr->containsValueForKey( keyPtr ), "Value should have been
  // found in region." );

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

  // ASSERT( !regPtr->containsKey( keyPtr ), "Key should not have been found in
  // region." );
  // ASSERT( !regPtr->containsValueForKey( keyPtr ), "Value should not have been
  // found in region." );

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

const char *testregex[] = {"Key-*1", "Key-*2", "Key-*3", "Key-*4"};
const char *keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char *vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char *nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const char *regionName = "DistRegionAck";

const bool USE_ACK = true;
void initClientAndRegion(int redundancy) {
  g_redundancyLevel = redundancy;
  auto pp = Properties::create();
  getHelper()->createPoolWithLocators("__TESTPOOL1_", locatorsG, true,
                                      redundancy);
  getHelper()->createRegionAndAttachPool(regionName, USE_ACK, "__TESTPOOL1_",
                                         true);
}
//#include "ThinClientDurableInit.hpp"
#include "LocatorHelper.hpp"
#include "ThinClientTasks_C2S2.hpp"
DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_notify_subscription.xml");
    }
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, InitClient1_R1)
  {
    initClient();
    LOG("Initialized client with redundancy level 1.");
    initClientAndRegion(1);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, InitClient2_R1)
  {
    initClient();
    initClientAndRegion(1);
    LOG("Initialized client with redundancy level 1.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, InitClient1_R3)
  {
    initClient();
    initClientAndRegion(3);
    LOG("Initialized client with redundancy level 3.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, InitClient2_R3)
  {
    initClient();
    initClientAndRegion(3);
    LOG("Initialized client with redundancy level 3.");
  }
END_TASK_DEFINITION

/*DUNIT_TASK_DEFINITION( CLIENT1, StepOne )
{
  createRegion( regionName, USE_ACK, false );
  createRegion( regionNames[1], NO_ACK, false );
  LOG( "StepOne complete." );
}
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION( CLIENT2, StepTwo )
{
  createRegion( regionName, USE_ACK, false );
  createRegion( regionNames[1], NO_ACK, false );

  LOG( "StepTwo complete." );
}
END_TASK_DEFINITION
*/
DUNIT_TASK_DEFINITION(SERVER2, CreateServer2)
  {
    if (isLocalServer) {
      CacheHelper::initServer(2, "cacheserver_notify_subscription2.xml");
    }
    LOG("SERVER2 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, RegisterRegexes)
  {
    auto regPtr0 = getHelper()->getRegion(regionName);

    if (g_redundancyLevel > 1) {
      regPtr0->registerRegex(testregex[0]);
    } else {
      regPtr0->registerRegex(testregex[0]);
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    createEntry(regionName, keys[0], vals[0]);
    createEntry(regionName, keys[1], vals[1]);
    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepFour)
  {
    verifyCreated(regionName, keys[0]);
    verifyEntry(regionName, keys[0], vals[0]);
    doNetsearch(regionName, keys[1], vals[1]);
    LOG("StepFour complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer1)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
    SLEEP(5000);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFive)
  {
    updateEntry(regionName, keys[0], nvals[0]);
    updateEntry(regionName, keys[1], nvals[1]);
    SLEEP(1000);
    LOG("StepFive complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepSix)
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

    verifyEntry(regionName, keys[0], nvals[0]);
    verifyEntry(regionName, keys[1], vals[1]);
    LOG("StepSix complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServer11)
  {
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_notify_subscription.xml",
                              locatorsG);
      LOG("SERVER1 started");
    }
    SLEEP(5000);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CloseServer2)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
    }
    SLEEP(5000);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServer3)
  {
    if (isLocalServer) {
      CacheHelper::initServer(3, "cacheserver_notify_subscription3.xml",
                              locatorsG);
    }
    LOG("SERVER3 started");
    SLEEP(15000);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepSeven)
  {
    updateEntry(regionName, keys[0], vals[0]);
    updateEntry(regionName, keys[1], vals[1]);
    SLEEP(1000);
    LOG("StepSeven complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepEight)
  {
    verifyEntry(regionName, keys[0], vals[0]);
    verifyEntry(regionName, keys[1], vals[1]);

    LOG("StepEight complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer21)
  {
    if (isLocalServer) {
      CacheHelper::initServer(2, "cacheserver_notify_subscription2.xml",
                              locatorsG);
    }
    LOG("SERVER2 started");
    SLEEP(15000);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer3)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(3);
      LOG("SERVER3 stopped");
    }
    SLEEP(15000);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepNine)
  {
    updateEntry(regionName, keys[0], nvals[0]);
    updateEntry(regionName, keys[1], nvals[1]);
    SLEEP(1000);
    LOG("StepNine complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTen)
  {
    verifyEntry(regionName, keys[0], nvals[0]);
    verifyEntry(regionName, keys[1], vals[1]);
    LOG("StepTen complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer11)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
    SLEEP(15000);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepEleven)
  {
    updateEntry(regionName, keys[0], vals[0]);
    updateEntry(regionName, keys[1], vals[1]);
    SLEEP(1000);
    LOG("StepEleven complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwelve)
  {
    verifyEntry(regionName, keys[0], vals[0]);
    verifyEntry(regionName, keys[1], vals[1]);
    LOG("StepTwelve complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CloseCache2)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CloseServer21)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
    }
    SLEEP(5000);
  }
END_TASK_DEFINITION

void runRegexFailOver() {
  for (int runNum = 1; runNum <= 2; runNum++) {
    CALL_TASK(CreateLocator1);
    CALL_TASK(CreateServer1_With_Locator_XML);

    if (runNum == 1) {
      CALL_TASK(InitClient1_R1);
      CALL_TASK(InitClient2_R1);
    } else {
      CALL_TASK(InitClient1_R3);
      CALL_TASK(InitClient2_R3);
    }
    // CALL_TASK( StepOne );
    // CALL_TASK( StepTwo );

    CALL_TASK(CreateServer2_With_Locator_XML);

    CALL_TASK(RegisterRegexes);
    CALL_TASK(StepThree);
    CALL_TASK(StepFour);
    CALL_TASK(CloseServer1);
    CALL_TASK(StepFive);
    CALL_TASK(StepSix);
    CALL_TASK(CreateServer11);
    CALL_TASK(CloseServer2);
    CALL_TASK(CreateServer3);
    CALL_TASK(StepSeven);
    CALL_TASK(StepEight);
    CALL_TASK(CreateServer21);
    CALL_TASK(CloseServer3);
    CALL_TASK(StepNine);
    CALL_TASK(StepTen);
    CALL_TASK(CloseServer11);
    CALL_TASK(StepEleven);
    CALL_TASK(StepTwelve);
    CALL_TASK(CloseCache1);
    CALL_TASK(CloseCache2);
    CALL_TASK(CloseServer21);

    CALL_TASK(CloseLocator1);
    SLEEP(1000);
  }
}

DUNIT_MAIN
  { runRegexFailOver(); }
END_MAIN
