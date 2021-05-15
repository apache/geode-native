#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTFAILOVERINTERESTALLWITHCACHE_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTFAILOVERINTERESTALLWITHCACHE_H_

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
#include <string>

#define ROOT_NAME "ThinClientFailoverInterestAllWithCache"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"

namespace { // NOLINT(google-build-namespaces)

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;

CacheHelper* cacheHelper = nullptr;
bool isLocalServer = false;

volatile bool g_poolConfig = false;
volatile bool g_poolLocators = false;

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2
static bool isLocator = false;
// static int numberOfLocators = 0;
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
#include "LocatorHelper.hpp"
#include "ThinClientTasks_C2S2.hpp"
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
  free(buf);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  auto keyPtr = CacheableKey::create(key);

  // if the region is no ack, then we may need to wait...
  if (!isCreated) {
    if (!noKey) {  // need to find the key!
      ASSERT(regPtr->containsKey(keyPtr), "Key not found in region.");
    }
    if (val != nullptr) {  // need to have a value!
      // ASSERT(regPtr->containsValueForKey(keyPtr),
      //    "Value not found in region.");
    }
  }

  // loop up to MAX times, testing condition
  uint32_t MAX = 100;
  //  changed sleep from 10 ms
  uint32_t SLEEP = 10;  // milliseconds
  uint32_t containsKeyCnt = 0;
  uint32_t containsValueCnt = 0;
  uint32_t testValueCnt = 0;

  for (int i = MAX; i >= 0; --i) {
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
          ++containsKeyCnt;
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
        LOG("In verify loop, get returned " + checkPtr->value() + " for key " + key);
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

void createRegion(const std::string& name, bool ackMode, const std::string&,
                  bool clientNotificationEnabled = false) {
  LOG("createRegion() entered.");
  LOG_INFO("Creating region --  %s  ackMode is %d", name.c_str(), ackMode);
  // ack, caching
  auto regPtr = getHelper()->createRegion(name, ackMode, true, nullptr,
                                          clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}

void createEntry(const char* name, const char* key, const char* value) {
  LOG("createEntry() entered.");
  LOG_INFO("Creating entry -- key: %s  value: %s in region %s", key, value,
          name);
  // Create entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableString::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(!regPtr->containsKey(keyPtr),
         "Key should not have been found in region.");
  ASSERT(!regPtr->containsValueForKey(keyPtr),
         "Value should not have been found in region.");

  // regPtr->create(keyPtr, valPtr);
  regPtr->put(keyPtr, valPtr);
  LOG("Created entry.");

  verifyEntry(name, key, value);
  LOG("Entry created.");
}

void updateEntry(const char* name, const char* key, const char* value) {
  LOG("updateEntry() entered.");
  LOG_INFO("Updating entry -- key: %s  value: %s in region %s", key, value,
          name);
  // Update entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableString::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(regPtr->containsKey(keyPtr), "Key should have been found in region.");
  // ASSERT(regPtr->containsValueForKey(keyPtr),
  //    "Value should have been found in region.");

  regPtr->put(keyPtr, valPtr);
  LOG("Put entry.");

  verifyEntry(name, key, value);
  LOG("Entry updated.");
}

void doNetsearch(const char* name, const char* key, const char* value) {
  LOG("doNetsearch() entered.");
  LOG_INFO("Netsearching for entry -- key: %s  expecting value: %s in region %s",
          key, value, name);
  // Get entry created in Process A, verify entry is correct
  auto keyPtr = CacheableKey::create(key);

  auto regPtr = getHelper()->getRegion(name);
  LOG_INFO("netsearch  region %s", regPtr->getName().c_str());
  ASSERT(regPtr != nullptr, "Region not found.");

  // ASSERT(!regPtr->containsKey(keyPtr),
  //    "Key should not have been found in region.");
  // ASSERT(!regPtr->containsValueForKey(keyPtr),
  //    "Value should not have been found in region.");

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

// End: Utility methods

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
    }
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, InitializeClient1)
  {
    initClient(true);
    getHelper()->createPooledRegion(regionNames[0], USE_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);
    getHelper()->createPooledRegion(regionNames[1], NO_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);
    // create some entries in the cache from client 1
    createEntry(regionNames[0], keys[1], vals[1]);
    createEntry(regionNames[1], keys[3], vals[3]);
    LOG("InitializeClient1 complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, InitializeClient2)
  {
    // Client two register all keys.
    initClient(true);
    getHelper()->createPooledRegion(regionNames[0], USE_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);
    getHelper()->createPooledRegion(regionNames[1], NO_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);

    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);
    // create a local entry to check for no change after register interest
    createEntry(regionNames[0], keys[1], nvals[1]);
    regPtr0->registerAllKeys(false, true);
    regPtr1->registerAllKeys(false, true);

    // check that initial entries are created properly
    ASSERT(regPtr0->size() == 1, "Expected one entry in region");
    ASSERT(regPtr1->size() == 1, "Expected one entry in region");

    LOG("InitializeClient2 complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, InitializeClient2Regex)
  {
    initClient(true);
    // Same tests as in StepTwo with registerRegex(".*")
    createRegion(regionNames[0], USE_ACK, locatorsG, true);
    createRegion(regionNames[1], NO_ACK, locatorsG, true);

    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);
    // create a local entry to check for no change after register interest
    createEntry(regionNames[0], keys[1], nvals[1]);
    regPtr0->registerRegex(".*", false, true);
    regPtr1->registerRegex(".*", false, true);

    // check that initial entries are created properly
    ASSERT(regPtr0->size() == 1, "Expected one entry in region");
    ASSERT(regPtr1->size() == 1, "Expected one entry in region");

    verifyCreated(regionNames[0], keys[1]);
    verifyCreated(regionNames[1], keys[3]);
    verifyEntry(regionNames[0], keys[1], nvals[1]);
    verifyEntry(regionNames[1], keys[3], vals[3]);

    LOG("ReinitializeClient2 complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, VerifyClient1)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    regPtr0->registerAllKeys(false, false);

    ASSERT(regPtr0->size() == 1, "Expected one entry in region");
    ASSERT(regPtr0->containsKey(keys[1]), "Expected region to contain the key");
    ASSERT(!regPtr0->containsValueForKey(keys[1]),
           "Expected region to not contain the value");

    // check the same for registerRegex(".*")
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);
    regPtr1->registerRegex(".*", false, false);

    ASSERT(regPtr1->size() == 1, "Expected one entry in region");
    ASSERT(regPtr1->containsKey(keys[3]), "Expected region to contain the key");
    ASSERT(!regPtr1->containsValueForKey(keys[3]),
           "Expected region to not contain the value");

    createEntry(regionNames[0], keys[0], vals[0]);
    updateEntry(regionNames[0], keys[1], vals[1]);
    createEntry(regionNames[1], keys[2], vals[2]);

    LOG("VerifyClient1 complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyClient2)
  {
    // Client two should recieve all entries
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);

    ASSERT(regPtr0->size() == 2, "Expected two entries in region");
    ASSERT(regPtr1->size() == 2, "Expected two entries in region");

    verifyCreated(regionNames[0], keys[0]);
    verifyCreated(regionNames[1], keys[2]);
    verifyEntry(regionNames[0], keys[0], vals[0]);
    verifyEntry(regionNames[0], keys[1], vals[1]);
    verifyEntry(regionNames[1], keys[2], vals[2]);
    verifyEntry(regionNames[1], keys[3], vals[3]);

    LOG("VerifyClient2 complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2)
  {
    if (isLocalServer) {
      CacheHelper::initServer(2, "cacheserver_notify_subscription2.xml");
    }
    LOG("SERVER2 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer1)
  {
    // failover happens.
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, UpdateClient1)
  {
    // Client one update entries
    updateEntry(regionNames[0], keys[0], nvals[0]);
    updateEntry(regionNames[1], keys[2], nvals[2]);
    LOG("UpdateClient1 complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyUpatesClient2)
  {
    // Client two should see updates after failover.
    verifyEntry(regionNames[0], keys[0], nvals[0]);
    verifyEntry(regionNames[0], keys[1], vals[1]);
    verifyEntry(regionNames[1], keys[2], nvals[2]);
    verifyEntry(regionNames[1], keys[3], vals[3]);
    LOG("VerifyUpatesClient2 complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, Client2UnregisterAllKeys)
  {
    // Client two unregister all keys
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);
    regPtr0->unregisterAllKeys();
    regPtr1->unregisterAllKeys();

    LOG("Client2UnregisterAllKeys complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, UpdateAndVerifyClient1)
  {
    // Client one update entries
    updateEntry(regionNames[0], keys[0], vals[0]);
    verifyEntry(regionNames[0], keys[1], vals[1]);
    updateEntry(regionNames[1], keys[2], vals[2]);
    verifyEntry(regionNames[1], keys[3], vals[3]);
    LOG("UpdateAndVerifyClient1 complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, Client2VerifyOriginalValues)
  {
    // Client two should still have the original values
    verifyEntry(regionNames[0], keys[0], nvals[0]);
    verifyEntry(regionNames[0], keys[1], vals[1]);
    verifyEntry(regionNames[1], keys[2], nvals[2]);
    verifyEntry(regionNames[1], keys[3], vals[3]);
    LOG("Client2VerifyOriginalValues complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CloseCache2)
  { cleanProc(); }
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

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTFAILOVERINTERESTALLWITHCACHE_H_
