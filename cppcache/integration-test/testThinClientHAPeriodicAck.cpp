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
#include <thread>
#include <chrono>

#include <ace/High_Res_Timer.h>
#include <ace/OS.h>

#include <geode/EntryEvent.hpp>

#define ROOT_NAME "testThinClientHAPeriodicAck"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"

using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;
using apache::geode::client::CacheListener;
using apache::geode::client::EntryEvent;
using apache::geode::client::HashMapOfCacheable;
using apache::geode::client::Properties;
using apache::geode::client::RegionEvent;

class DupChecker : public CacheListener {
  int m_ops;
  HashMapOfCacheable m_map;

  void check(const EntryEvent &event) {
    m_ops++;

    auto key = event.getKey();
    auto value = std::dynamic_pointer_cast<CacheableInt32>(event.getNewValue());

    const auto &item = m_map.find(key);

    if (item != m_map.end()) {
      auto check = std::dynamic_pointer_cast<CacheableInt32>(item->second);
      ASSERT(check->value() + 1 == value->value(),
             "Duplicate or older value received");
      m_map[key] = value;
    } else {
      m_map.emplace(key, value);
    }
  }

 public:
  DupChecker() : m_ops(0) {}

  ~DupChecker() override { m_map.clear(); }

  void validate() {
    ASSERT(m_map.size() == 4, "Expected 4 keys for the region");
    ASSERT(m_ops == 400, "Expected 400 events (100 per key) for the region");

    for (const auto &item : m_map) {
      auto check = std::dynamic_pointer_cast<CacheableInt32>(item.second);
      ASSERT(check->value() == 100, "Expected final value to be 100");
    }
  }

  void afterCreate(const EntryEvent &event) override { check(event); }

  void afterUpdate(const EntryEvent &event) override { check(event); }

  void afterRegionInvalidate(const RegionEvent &) override {}

  void afterRegionDestroy(const RegionEvent &) override {}
};

///////////////////////////////////////////////////////

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2

CacheHelper *cacheHelper = nullptr;
static bool isLocator = false;
static bool isLocalServer = false;
static int numberOfLocators = 1;
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);
int g_redundancyLevel = 0;
bool g_poolConfig = false;
bool g_poolLocators = false;

void initClient(int redundancyLevel) {
  auto props = Properties::create();
  props->insert("notify-ack-interval", std::chrono::seconds(1));
  props->insert("notify-dupcheck-life", std::chrono::seconds(30));

  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(redundancyLevel, props);
  }
  g_redundancyLevel = redundancyLevel;
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

void initClient() {
  auto props = Properties::create();
  props->insert("notify-ack-interval", std::chrono::seconds(1));
  props->insert("notify-dupcheck-life", std::chrono::seconds(30));

  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(true, props);
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
      msg = std::string("Verify key ") + key + " does not exist in region " + name;
    }
    else if (!val) {
      msg = std::string("Verify value for key ") + key +
        " does not exist in region " + name;
    }
    else {
      msg = std::string("Verify value for key ") + key + " is: " + value + " in region " + name;
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

void _verifyIntEntry(const char *name, const char *key, const int val,
                     bool noKey, bool isCreated = false) {
  // Verify key and value exist in this region, in this process.
  int value = val;
  std::string msg;
  if (!isCreated) {
    if (noKey) {
      msg = std::string("Verify key ") + key + " does not exist in region " + name;
    }
    else if (!val) {
      msg = std::string("Verify value for key ") + key +
        " does not exist in region " + name;
    }
    else {
      msg = std::string("Verify value for key ") + key + " is: " + std::to_string(value) + " in region " + name;
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
    if (val != 0) {  // need to have a value!
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
      if (val == 0) {
        if (regPtr->containsValueForKey(keyPtr)) {
          containsValueCnt++;
        } else {
          break;
        }
        ASSERT(containsValueCnt < MAX, "Value found in region.");
      }

      if (val != 0) {
        auto checkPtr =
            std::dynamic_pointer_cast<CacheableInt32>(regPtr->get(keyPtr));

        ASSERT(checkPtr != nullptr, "Value Ptr should not be null.");
        LOG("In verify loop, get returned " +
            std::to_string(checkPtr->value()) + " for key " + key);
        // if ( strcmp( checkPtr->value().c_str(), value ) != 0 ){
        if (checkPtr->value() != value) {
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

#define verifyIntEntry(x, y, z) _verifyIntEntry(x, y, z, __LINE__)

void _verifyIntEntry(const char *name, const char *key, const int val,
                     int line) {
  LOG(std::string("verifyIntEntry() called from ") + std::to_string(line) + "\n");
  _verifyIntEntry(name, key, val, false);
  LOG("Entry verified.");
}

void _verifyCreated(const char *name, const char *key, int line) {
  LOG(std::string("verifyCreated() called from ") + std::to_string(line) + "\n");
  _verifyEntry(name, key, nullptr, false, true);
  LOG("Entry created.");
}

void createRegion(const char *name, bool ackMode,
                  bool clientNotificationEnabled = true) {
  LOG("createRegion() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  char *endpoints = nullptr;
  // ack, caching
  auto regPtr = getHelper()->createRegion(name, ackMode, true, nullptr,
                                          endpoints, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}

void createEntry(const char *name, const char *key, const char *value) {
  LOG("createEntry() entered.");
  fprintf(stdout, "Creating entry -- key: %s  value: %s in region %s\n", key,
          value, name);
  fflush(stdout);
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

void createIntEntry(const char *name, const char *key, const int value) {
  LOG("createEntry() entered.");
  fprintf(stdout, "Creating entry -- key: %s  value: %d in region %s\n", key,
          value, name);
  fflush(stdout);
  // Create entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableInt32::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  // ASSERT( !regPtr->containsKey( keyPtr ), "Key should not have been found in
  // region." );
  // ASSERT( !regPtr->containsValueForKey( keyPtr ), "Value should not have been
  // found in region." );

  // regPtr->create( keyPtr, valPtr );
  regPtr->put(keyPtr, valPtr);
  LOG("Created entry.");

  verifyIntEntry(name, key, value);
  LOG("Entry created.");
}

void setCacheListener(const char *regName,
                      std::shared_ptr<DupChecker> checker) {
  auto reg = getHelper()->getRegion(regName);
  auto attrMutator = reg->getAttributesMutator();
  attrMutator->setCacheListener(checker);
}

const char *keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char *vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char *nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const char *regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

const bool USE_ACK = true;
const bool NO_ACK = false;
std::shared_ptr<DupChecker> checker1;
std::shared_ptr<DupChecker> checker2;

void initClientAndRegion(int redundancy) {
  auto pp = Properties::create();
  getHelper()->createPoolWithLocators("__TESTPOOL1_", locatorsG, true,
                                      redundancy);
  getHelper()->createRegionAndAttachPool(regionNames[0], USE_ACK,
                                         "__TESTPOOL1_", true);
  getHelper()->createRegionAndAttachPool(regionNames[1], NO_ACK, "__TESTPOOL1_",
                                         true);
}

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

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2)
  {
    if (isLocalServer) {
      CacheHelper::initServer(2, "cacheserver_notify_subscription2.xml");
    }
    LOG("SERVER2 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, InitClient1_R1)
  {
    initClient();
    initClientAndRegion(1);
    LOG("Initialized client with redundancy level 1.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, InitClient2_R1)
  {
    initClient();
    initClientAndRegion(1);
    LOG("Initialized client with redundancy level 1.");

    checker1 = std::make_shared<DupChecker>();
    checker2 = std::make_shared<DupChecker>();

    setCacheListener(regionNames[0], checker1);
    setCacheListener(regionNames[1], checker2);

    getHelper()->getRegion(regionNames[0])->registerAllKeys();
    getHelper()->getRegion(regionNames[1])->registerAllKeys();

    LOG("Region creation complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CreateEntries)
  {
    for (int value = 1; value <= 100; value++) {
      createIntEntry(regionNames[0], keys[0], value);
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      createIntEntry(regionNames[0], keys[1], value);
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      createIntEntry(regionNames[0], keys[2], value);
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      createIntEntry(regionNames[0], keys[3], value);
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      createIntEntry(regionNames[1], keys[0], value);
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      createIntEntry(regionNames[1], keys[1], value);
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      createIntEntry(regionNames[1], keys[2], value);
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      createIntEntry(regionNames[1], keys[3], value);
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CheckClient2)
  {
    verifyIntEntry(regionNames[0], keys[0], 100);
    verifyIntEntry(regionNames[0], keys[1], 100);
    verifyIntEntry(regionNames[0], keys[2], 100);
    verifyIntEntry(regionNames[0], keys[3], 100);
    verifyIntEntry(regionNames[1], keys[0], 100);
    verifyIntEntry(regionNames[1], keys[1], 100);
    verifyIntEntry(regionNames[1], keys[2], 100);
    verifyIntEntry(regionNames[1], keys[3], 100);

    LOG("Validating checker1 cachelistener");
    checker1->validate();
    LOG("Validating checker2 cachelistener");
    checker2->validate();

    LOG("CheckClient2 complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseClient1)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CloseClient2)
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
    CALL_TASK(CreateLocator1);
    CALL_TASK(CreateServer1_With_Locator_XML);
    CALL_TASK(CreateServer2_With_Locator_XML);

    CALL_TASK(InitClient1_R1);
    CALL_TASK(InitClient2_R1);

    CALL_TASK(CreateEntries);
    CALL_TASK(CheckClient2);

    CALL_TASK(CloseClient1);
    CALL_TASK(CloseClient2);
    CALL_TASK(CloseServer1);
    CALL_TASK(CloseServer2);

    CALL_TASK(CloseLocator1);
  }
END_MAIN
