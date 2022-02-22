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
#include "testobject/PdxType.hpp"
#include "testobject/VariousPdxTypes.hpp"

#include <string>

#define ROOT_NAME "testThinClientRemoveOps"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2

using apache::geode::client::Cacheable;
using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableInt64;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;
using apache::geode::client::EntryExistsException;
using apache::geode::client::EntryNotFoundException;
using apache::geode::client::Exception;
using apache::geode::client::IllegalArgumentException;
using apache::geode::client::Properties;
using apache::geode::client::RegionDestroyedException;

CacheHelper *cacheHelper = nullptr;
static bool isLocalServer = false;
static bool isLocator = false;
static int numberOfLocators = 0;

const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);

void initClient(const bool isthinClient) {
  if (cacheHelper == nullptr) {
    auto config = Properties::create();
    cacheHelper = new CacheHelper(isthinClient, config);
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
  if (noKey) {
    LOG(std::string("Verify key ") + key + " does not exist in region " + name);
  } else if (!val) {
    LOG(std::string("Verify value for key ") + key +
        " does not exist in region " + name);
  } else {
    LOG(std::string("Verify value for key ") + key + "is:" + value +
        " in region " + name);
  }

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
  LOG(std::string("verifyInvalid() called from ") + std::to_string(line));
  _verifyEntry(name, key, nullptr, false);
  LOG("Entry invalidated.");
}

void _verifyDestroyed(const char *name, const char *key, int line) {
  LOG(std::string("verifyDestroyed() called from ") + std::to_string(line));
  _verifyEntry(name, key, nullptr, true);
  LOG("Entry destroyed.");
}

#define verifyEntry(x, y, z) _verifyEntry(x, y, z, __LINE__)

void _verifyEntry(const char *name, const char *key, const char *val,
                  int line) {
  LOG(std::string("verifyEntry() called from ") + std::to_string(line));
  _verifyEntry(name, key, val, false);
  LOG("Entry verified.");
}

void createRegion(const char *name, bool ackMode,
                  bool clientNotificationEnabled = false,
                  bool cachingEnable = true) {
  LOG("createRegion() entered.");
  std::cout << "Creating region --  " << name << " ackMode is " << ackMode
            << "\n"
            << std::flush;
  auto regPtr = getHelper()->createRegion(name, ackMode, cachingEnable, nullptr,
                                          clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}

void createPooledRegion(const std::string &name, bool ackMode,
                        const std::string &locators,
                        const std::string &poolname,
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

void putEntry(const char *name, const char *key, const char *value) {
  LOG("putEntry() entered.");
  std::cout << "Creating entry -- key: " << key << " value: " << value
            << " in region " << name << "\n"
            << std::flush;
  // Put entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableString::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(!regPtr->containsKey(keyPtr),
         "Key should not have been found in region.");
  ASSERT(!regPtr->containsValueForKey(keyPtr),
         "Value should not have been found in region.");

  regPtr->put(keyPtr, valPtr);
  LOG("Created entry.");

  verifyEntry(name, key, value);
  LOG("Entry created.");
}

void localPutEntry(const char *name, const char *key, const char *value) {
  LOG("putEntry() entered.");
  std::cout << "Creating entry -- key: " << key << " value: " << value
            << " in region " << name << "\n"
            << std::flush;
  // Put entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableString::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  regPtr->localPut(keyPtr, valPtr);
  LOG("LocalPut entry done.");

  verifyEntry(name, key, value);
  LOG("Local Entry created.");
}

void createEntryTwice(const char *name, const char *key, const char *value) {
  LOG("createEntryTwice() entered.");
  std::cout << "Creating entry -- key: " << key << " value: " << value
            << " in region " << name << "\n";
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableString::create(value);
  auto regPtr = getHelper()->getRegion(name);
  regPtr->create(keyPtr, valPtr);
  try {
    regPtr->create(keyPtr, valPtr);
  } catch (const EntryExistsException &geodeExcp) {
    LOG(geodeExcp.what());
    LOG("createEntryTwice() Clean Exit.");
    return;
  }
  ASSERT(false,
         "Creating key twice is not allowed and while doing that exception was "
         "not thrown");
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
    LOG(std::string("In doGetAgain, get returned ") +
        checkPtr->value().c_str() + " for key " + key);
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
    LOG(std::string("In net search, get returned ") +
        checkPtr->value().c_str() + " for key " + key);
  } else {
    LOG("checkPtr is nullptr");
  }
  verifyEntry(name, key, value);
  LOG("Netsearch complete.");
}

const char *keys[] = {"Key-1", "Key-2", "Key-3", "Key-4", "Non-Existent-Key"};
const char *vals[] = {"Value-1", "Value-2", "Value-3", "Value-4",
                      "Non-Existent-Value"};
const char *nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const char *regionNames[] = {"DistRegionAck", "exampleRegion"};

const bool USE_ACK = true;
const bool NO_ACK = false;

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    if (isLocalServer) CacheHelper::initServer(1);
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2)
  {
    if (isLocalServer) CacheHelper::initServer(2, "cacheserver1_expiry.xml");
    LOG("SERVER2 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2_locator)
  {
    if (isLocalServer) {
      CacheHelper::initServer(2, "cacheserver1_expiry.xml", locatorsG);
    }
    LOG("SERVER2 with locator started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1_With_Locator)
  {
    if (isLocalServer) CacheHelper::initServer(1, {}, locatorsG);
    LOG("SERVER1 with locator started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pooled_Locator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TESTPOOL1_");
    LOG("StepOne_Pooled complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepSeven_Pooled_Locator)
  {
    initClient(true);
    createPooledRegion(regionNames[1], USE_ACK, locatorsG, "__TESTPOOL1_");
    LOG("StepSeven_Pooled_Locator complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo_Pooled_Locator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TESTPOOL1_");
    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    putEntry(regionNames[0], keys[0], vals[0]);
    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFive)
  {
    auto reg0 = getHelper()->getRegion(regionNames[0]);

    auto keyPtr = CacheableString::create(keys[0]);
    auto keyPtr1 = CacheableString::create(keys[2]);

    // Try removing non-existent entry from regions, result should be false.
    ASSERT(reg0->remove(keys[4], vals[4]) == false,
           "Result of remove should be false, as this entry is not present in "
           "first region.");

    // Try removing non-existent key, but existing value from regions, result
    // should be false.
    ASSERT(reg0->remove(keys[4], vals[0]) == false,
           "Result of remove should be false, as this key is not present in "
           "first region.");

    // Try removing existent key, but non-existing value from regions, result
    // should be false.
    ASSERT(reg0->remove(keys[0], vals[4]) == false,
           "Result of remove should be false, as this value is not present in "
           "first region.");

    // Try removing existent key, and existing value from regions, result should
    // be true.
    ASSERT(reg0->remove(keys[0], vals[0]) == true,
           "Result of remove should be true, as this entry is present in first "
           "region.");

    ASSERT(reg0->containsKey(keys[0]) == false, "containsKey should be false");
    ASSERT(reg0->containsKeyOnServer(keyPtr) == false,
           "containsKeyOnServer should be false");

    // Try removing already deleted entry from regions, result should be false,
    // but no exception.
    ASSERT(reg0->remove(keys[0], vals[0]) == false,
           "Result of remove should be false, as this entry is not present in "
           "first region.");

    // Try locally destroying already deleted entry from regions, It should
    // result
    // into exception.
    try {
      reg0->localDestroy(keys[0]);
      FAIL(
          "local destroy on already removed key should have thrown "
          "EntryNotFoundException");
    } catch (EntryNotFoundException &) {
      LOG("Got expected EntryNotFoundException for "
          "localDestroy operation on already removed entry.");
    }

    LOG("StepFive complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepSix)
  {
    auto reg0 = getHelper()->getRegion(regionNames[0]);

    auto keyPtr = CacheableString::create(keys[1]);
    auto keyPtr1 = CacheableString::create(keys[3]);

    putEntry(regionNames[0], keys[1], nvals[1]);

    // Try removing value that is present on client as well as server, result
    // should be true.
    ASSERT(
        reg0->remove(keys[1], nvals[1]) == true,
        "Result of remove should be true, as this value is present locally, & "
        "also present on server.");
    ASSERT(reg0->containsKey(keys[1]) == false, "containsKey should be false");
    ASSERT(reg0->containsKeyOnServer(keyPtr) == false,
           "containsKeyOnServer should be false");
    LOGINFO("Step 6.1 complete.");

    // Try removing value that is present on client but not on server, result
    // should be false.
    putEntry(regionNames[0], keys[1], vals[1]);
    localPutEntry(regionNames[0], keys[1], nvals[1]);
    ASSERT(
        reg0->remove(keys[1], nvals[1]) == false,
        "Result of remove should be false, as this value is present locally, "
        "but not present on server.");
    ASSERT(reg0->containsKey(keys[1]) == true, "containsKey should be true");
    ASSERT(reg0->containsKeyOnServer(keyPtr) == true,
           "containsKeyOnServer should be true");

    LOGINFO("Step 6.2 complete.");

    // Try removing value that is not present on client but present on server,
    // result should be false.
    reg0->destroy(keys[1]);
    putEntry(regionNames[0], keys[1], vals[1]);
    localPutEntry(regionNames[0], keys[1], nvals[1]);
    ASSERT(reg0->remove(keys[1], vals[1]) == false,
           "Result of remove should be false, as this value is not present "
           "locally, but present only on server.");
    ASSERT(reg0->containsKey(keys[1]) == true, "containsKey should be true");
    ASSERT(reg0->containsKeyOnServer(keyPtr) == true,
           "containsKeyOnServer should be true");

    LOGINFO("Step 6.3 complete.");

    // Try removing value that is invalidated on client but exists on server,
    // result should be false.
    reg0->destroy(keys[1]);
    putEntry(regionNames[0], keys[1], nvals[1]);
    reg0->invalidate(keys[1]);
    ASSERT(reg0->remove(keys[1], nvals[1]) == false,
           "Result of remove should be false, as this value is not present "
           "locally, but present only on server.");
    ASSERT(reg0->containsKey(keys[1]) == true, "containsKey should be true");
    ASSERT(reg0->containsKeyOnServer(keyPtr) == true,
           "containsKeyOnServer should be true");

    LOGINFO("Step 6.4 complete.");

    // Try removing null value, that is invalidated on client but exists on the
    // server, result should be false.
    reg0->destroy(keys[1]);
    putEntry(regionNames[0], keys[1], vals[1]);
    reg0->localInvalidate(
        keys[1]);  // Invalidating a key sets its value to null
    ASSERT(reg0->remove(keys[1], static_cast<std::shared_ptr<Cacheable>>(
                                     nullptr)) == false,
           "Result of remove should be false, as this value is not present "
           "locally, but present only on server.");
    ASSERT(reg0->containsKey(keys[1]) == true, "containsKey should be true");
    ASSERT(reg0->containsKeyOnServer(keyPtr) == true,
           "containsKeyOnServer should be true");

    LOGINFO("Step 6.5 complete.");

    // Try removing a entry (value) which is not present on client as well as
    // server, result should be false.
    ASSERT(reg0->remove("NewKey1", "NewValue1") == false,
           "Result of remove should be false, as this value is not present "
           "locally, and not present on server.");
    auto keyPtr2 = CacheableString::create("NewKey1");
    auto keyPtr3 = CacheableString::create("NewKey3");
    ASSERT(reg0->containsKey("NewKey1") == false,
           "containsKey should be false");
    ASSERT(reg0->containsKeyOnServer(keyPtr2) == false,
           "containsKeyOnServer should be false");

    LOGINFO("Step 6.6 complete.");

    // Try removing a entry with a null value, which is not present on client as
    // well as server, result should be false.
    ASSERT(reg0->remove("NewKey1", static_cast<std::shared_ptr<Cacheable>>(
                                       nullptr)) == false,
           "Result of remove should be false, as this value is not present "
           "locally, and not present on server.");

    ASSERT(reg0->containsKey("NewKey1") == false,
           "containsKey should be false");
    ASSERT(reg0->containsKeyOnServer(keyPtr2) == false,
           "containsKeyOnServer should be false");

    LOGINFO("Step 6.7 complete.");

    // Try removing a entry (value) which is not present on client but exists on
    // the server, result should be true.
    reg0->destroy(keys[1]);
    putEntry(regionNames[0], keys[1], nvals[1]);
    reg0->localDestroy(keys[1]);
    ASSERT(reg0->remove(keys[1], nvals[1]) == true,
           "Result of remove should be true, as this value does not exist "
           "locally, but exists on server.");
    ASSERT(reg0->containsKey(keys[1]) == false, "containsKey should be false");
    ASSERT(reg0->containsKeyOnServer(keyPtr) == false,
           "containsKeyOnServer should be false");

    LOG("Step6.8 complete.");

    putEntry(regionNames[0], keys[1], nvals[1]);
    reg0->destroy(keys[1]);
    ASSERT(reg0->remove(keys[1], static_cast<std::shared_ptr<Cacheable>>(
                                     nullptr)) == false,
           "Result of remove should be false, as this value does not exist "
           "locally, but exists on server.");
    ASSERT(reg0->containsKey(keys[1]) == false, "containsKey should be false");
    ASSERT(reg0->containsKeyOnServer(keyPtr) == false,
           "containsKeyOnServer should be false");

    LOG("Step6.8.1 complete.");

    // Try locally removing an entry which is locally destroyed with a nullptr.
    reg0->put(keys[1], vals[1]);
    ASSERT(reg0->remove(keys[1], vals[1]) == true,
           "Result of remove should be true, as this value does not exists "
           "locally.");
    ASSERT(reg0->remove(keys[1], vals[1]) == false,
           "Result of remove should be false, as this value does not exists "
           "locally.");
    ASSERT(reg0->containsKey(keys[1]) == false, "containsKey should be false");

    LOG("Step6.8.2 complete.");

    //-------------------------------------localRemove
    // Testcases------------------------------------------------
    // Try locally removing an entry (value) which is present on the client.
    // reg0->destroy(keys[1]);
    try {
      reg0->destroy(keys[1]);
      FAIL(
          "destroy on already removed key should have thrown "
          "EntryNotFoundException");
    } catch (EntryNotFoundException &) {
      LOG("Got expected EntryNotFoundException for "
          "destroy operation on already removed entry.");
    }

    reg0->localPut(keys[1], vals[1]);
    ASSERT(reg0->localRemove(keys[1], vals[1]) == true,
           "Result of remove should be true, as this value exists locally.");
    ASSERT(reg0->containsKey(keys[1]) == false, "containsKey should be false");

    LOG("Step6.9 complete.");

    // Try local destroy on entry that is already removed, should get an
    // exception.
    try {
      reg0->localDestroy(keys[1]);
      FAIL(
          "local destroy on already removed key should have thrown "
          "EntryNotFoundException");
    } catch (EntryNotFoundException &) {
      LOG("Got expected EntryNotFoundException for "
          "localDestroy operation on already removed entry.");
    }

    LOG("Step6.10 complete.");

    // Try locally removing an entry (value) which is not present on the client
    // (value mismatch).
    reg0->localPut(keys[1], vals[1]);
    ASSERT(reg0->localRemove(keys[1], nvals[1]) == false,
           "Result of remove should be false, as this value does not exists "
           "locally.");

    ASSERT(reg0->containsKey(keys[1]) == true, "containsKey should be true");

    LOG("Step6.11 complete.");

    // Try locally removing an entry (value) which is invalidated with a value.
    reg0->localDestroy(keys[1]);
    reg0->localPut(keys[1], vals[1]);
    reg0->invalidate(keys[1]);
    ASSERT(reg0->localRemove(keys[1], vals[1]) == false,
           "Result of remove should be false, as this value does not exists "
           "locally.");
    ASSERT(reg0->containsKey(keys[1]) == true, "containsKey should be true");

    LOG("Step6.12 complete.");

    // Try locally removing an entry (value) which is invalidated with a
    // nullptr.
    reg0->localDestroy(keys[1]);
    reg0->localPut(keys[1], vals[1]);
    reg0->invalidate(keys[1]);
    ASSERT(reg0->localRemove(keys[1], static_cast<std::shared_ptr<Cacheable>>(
                                          nullptr)) == true,
           "Result of remove should be true, as this value does not exists "
           "locally.");
    ASSERT(reg0->containsKey(keys[1]) == false, "containsKey should be false");

    LOG("Step6.13 complete.");

    // Try locally removing an entry (value) with a nullptr.
    reg0->localPut(keys[1], vals[1]);
    ASSERT(reg0->localRemove(keys[1], static_cast<std::shared_ptr<Cacheable>>(
                                          nullptr)) == false,
           "Result of remove should be false, as this value does not exists "
           "locally.");
    ASSERT(reg0->containsKey(keys[1]) == true, "containsKey should be true");

    LOG("Step6.14 complete.");

    // Try locally removing an entry which is locally destroyed with a value.
    reg0->localPut(keys[1], vals[1]);
    reg0->localDestroy(keys[1]);
    ASSERT(reg0->localRemove(keys[1], vals[1]) == false,
           "Result of remove should be false, as this value does not exists "
           "locally.");
    ASSERT(reg0->containsKey(keys[1]) == false, "containsKey should be true");

    LOG("Step6.15 complete.");

    // Try locally removing an entry which is locally destroyed with a nullptr.
    reg0->localPut(keys[1], vals[1]);
    reg0->localDestroy(keys[1]);
    ASSERT(reg0->localRemove(keys[1], static_cast<std::shared_ptr<Cacheable>>(
                                          nullptr)) == false,
           "Result of remove should be false, as this value does not exists "
           "locally.");
    ASSERT(reg0->containsKey(keys[1]) == false, "containsKey should be false");

    LOG("Step6.16 complete.");

    // Try locally removing an entry which is already removed.
    reg0->localPut(keys[1], vals[1]);
    ASSERT(reg0->localRemove(keys[1], vals[1]) == true,
           "Result of remove should be true, as this value does not exists "
           "locally.");
    ASSERT(reg0->localRemove(keys[1], vals[1]) == false,
           "Result of remove should be false, as this value does not exists "
           "locally.");
    ASSERT(reg0->containsKey(keys[1]) == false, "containsKey should be false");

    LOG("Step6.17 complete.");
    LOG("StepSix complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepEight)
  {
    auto reg = getHelper()->getRegion(regionNames[1]);

    auto keyPtr = CacheableString::create(keys[0]);
    auto keyPtr1 = CacheableString::create(keys[1]);

    // Try removing a entry which is present on client (value) but invalidated
    // on
    // the server, result should be false.
    try {
      reg->destroy(keys[0]);
      FAIL(
          "destroy on already removed key should have thrown "
          "EntryNotFoundException");
    } catch (EntryNotFoundException &) {
      LOG("Got expected EntryNotFoundException for "
          "destroy operation on already removed entry.");
    }
    try {
      reg->destroy(keys[1]);
      FAIL(
          "destroy on already removed key should have thrown "
          "EntryNotFoundException");
    } catch (EntryNotFoundException &) {
      LOG("Got expected EntryNotFoundException for "
          "destroy operation on already removed entry.");
    }
    putEntry(regionNames[1], keys[0], vals[0]);
    putEntry(regionNames[1], keys[1], vals[1]);
    SLEEP(10000);  // This is for expiration on server to execute.
    ASSERT(
        reg->remove(keys[0], vals[0]) == false,
        "Result of remove should be false, as this value is present locally, "
        "but not present on server.");
    ASSERT(
        reg->remove(keys[1], vals[1]) == false,
        "Result of remove should be false, as this value is present locally, "
        "but not present on server.");
    ASSERT(reg->containsKey(keys[0]) == true, "containsKey should be true");
    ASSERT(reg->containsKeyOnServer(keyPtr) == true,
           "containsKeyOnServer should be true");
    ASSERT(reg->containsKey(keys[1]) == true, "containsKey should be true");
    ASSERT(reg->containsKeyOnServer(keyPtr1) == true,
           "containsKeyOnServer should be true");
    LOG("Step 8.1 complete.");

    // Try removing a entry that is not present on client, but invalidated on
    // server with null value, result should be true.
    reg->destroy(keys[0]);
    reg->destroy(keys[1]);
    putEntry(regionNames[1], keys[0], nvals[0]);
    putEntry(regionNames[1], keys[1], nvals[1]);
    reg->localDestroy(keys[0]);
    reg->localDestroy(keys[1]);
    SLEEP(10000);  // This is for expiration on server to execute.
    ASSERT(reg->remove(keys[0], static_cast<std::shared_ptr<Cacheable>>(
                                    nullptr)) == true,
           "Result of remove should be true, as this value is not present "
           "locally, & not present on server.");
    ASSERT(reg->remove(keys[1], static_cast<std::shared_ptr<Cacheable>>(
                                    nullptr)) == true,
           "Result of remove should be true, as this value is not present "
           "locally, & not present on server.");
    ASSERT(reg->containsKeyOnServer(keyPtr) == false,
           "containsKeyOnServer should be false");
    ASSERT(reg->containsKey(keys[0]) == false, "containsKey should be false");
    ASSERT(reg->containsKey(keys[1]) == false, "containsKey should be false");
    ASSERT(reg->containsKeyOnServer(keyPtr1) == false,
           "containsKeyOnServer should be false");
    LOG("Step 8.2 complete.");

    // Try removing a entry with a (value) on client that is invalidated on
    // server
    // with null , result should be false.
    try {
      reg->destroy(keys[0]);
      FAIL(
          "destroy on already removed key should have thrown "
          "EntryNotFoundException");
    } catch (EntryNotFoundException &) {
      LOG("Got expected EntryNotFoundException for "
          "destroy operation on already removed entry.");
    }
    try {
      reg->destroy(keys[1]);
      FAIL(
          "destroy on already removed key should have thrown "
          "EntryNotFoundException");
    } catch (EntryNotFoundException &) {
      LOG("Got expected EntryNotFoundException for "
          "destroy operation on already removed entry.");
    }
    putEntry(regionNames[1], keys[0], nvals[0]);
    putEntry(regionNames[1], keys[1], nvals[1]);
    SLEEP(10000);  // This is for expiration on server to execute.
    ASSERT(
        reg->remove(keys[0],
                    static_cast<std::shared_ptr<Cacheable>>(nullptr)) == false,
        "Result of remove should be false, as this value is present locally, "
        "& not present on server.");
    ASSERT(
        reg->remove(keys[1],
                    static_cast<std::shared_ptr<Cacheable>>(nullptr)) == false,
        "Result of remove should be false, as this value is present locally, "
        "& not present on server.");
    ASSERT(reg->containsKey(keys[0]) == true, "containsKey should be true");
    ASSERT(reg->containsKeyOnServer(keyPtr) == true,
           "containsKeyOnServer should be true");
    ASSERT(reg->containsKey(keys[1]) == true, "containsKey should be true");
    ASSERT(reg->containsKeyOnServer(keyPtr1) == true,
           "containsKeyOnServer should be true");
    LOG("Step 8.3 complete.");

    // Try removing a entry with a entry that is invalidated on the client as
    // well
    // as on server with a null value, result should be true.
    reg->destroy(keys[0]);
    reg->destroy(keys[1]);
    putEntry(regionNames[1], keys[0], nvals[0]);
    putEntry(regionNames[1], keys[1], nvals[1]);
    reg->invalidate(keys[0]);
    reg->invalidate(keys[1]);
    SLEEP(10000);  // This is for expiration on server to execute.
    ASSERT(reg->remove(keys[0], static_cast<std::shared_ptr<Cacheable>>(
                                    nullptr)) == true,
           "Result of remove should be true, as this value is not present "
           "locally, & not present on server.");
    ASSERT(reg->remove(keys[1], static_cast<std::shared_ptr<Cacheable>>(
                                    nullptr)) == true,
           "Result of remove should be true, as this value is not present "
           "locally, & not present on server.");
    ASSERT(reg->containsKey(keys[0]) == false, "containsKey should be false");
    ASSERT(reg->containsKeyOnServer(keyPtr) == false,
           "containsKeyOnServer should be false");
    ASSERT(reg->containsKey(keys[1]) == false, "containsKey should be false");
    ASSERT(reg->containsKeyOnServer(keyPtr1) == false,
           "containsKeyOnServer should be false");

    // Test case for Bug #639, destroy operation on key that is not present in
    // the
    // region reduces the region's size by 1.
    // Steps to reproduce: Put 2 entries in to region. Destroy an entry that is
    // not present in the region. Check for the sizes of keys, values and
    // region.
    // It is observed that regions size is less by 1 from that of keys and
    // values
    // sizes.
    reg->put("Key100", "Value100");
    reg->put("Key200", "Value200");
    LOGINFO("Region 2 puts complete ");
    LOGINFO("Regions size = %d ", reg->size());

    try {
      reg->destroy("key300");
      FAIL(
          "destroy on non=existent key should have thrown "
          "EntryNotFoundException");
    } catch (EntryNotFoundException &) {
      LOG("Got expected EntryNotFoundException for key300");
    }

    ASSERT(reg->size() == 2, "region size should be equal to 2");

    std::vector<std::shared_ptr<CacheableKey>> keysVector = reg->keys();
    LOGINFO("Region keys = %d ", keysVector.size());
    ASSERT(keysVector.size() == reg->size(),
           "region size should be equal to keys size");

    auto values = reg->values();
    LOGINFO("Region values = %d ", values.size());
    ASSERT(values.size() == reg->size(),
           "region size should be equal to values size");

    reg->destroy("Key100");
    keysVector = reg->keys();
    LOGINFO("Region keys = %d ", keysVector.size());
    ASSERT(keysVector.size() == reg->size(),
           "region size should be equal to keys size");

    values = reg->values();
    LOGINFO("Region values = %d ", values.size());
    ASSERT(values.size() == reg->size(),
           "region size should be equal to values size");

    LOG("StepEight complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFiveA)
  {
    putEntry(regionNames[0], keys[0], vals[0]);
    auto reg0 = getHelper()->getRegion(regionNames[0]);

    auto keyPtr = CacheableString::create(keys[0]);
    auto keyPtr1 = CacheableString::create(keys[2]);

    // Try removing non-existent entry from regions, result should be false.
    ASSERT(reg0->removeEx(keys[4]) == false,
           "Result of remove should be false, as this entry is not present in "
           "first region.");

    // Try removing existent key, and existing value from regions, result should
    // be true.

    ASSERT(reg0->removeEx(keys[0]) == true,
           "Result of remove should be true, as this entry is present in first "
           "region.");

    ASSERT(reg0->containsKey(keys[0]) == false, "containsKey should be false");
    ASSERT(reg0->containsKeyOnServer(keyPtr) == false,
           "containsKeyOnServer should be false");

    // Try removing already deleted entry from regions, result should be false,
    // but no exception.
    ASSERT(reg0->removeEx(keys[0]) == false,
           "Result of remove should be false, as this entry is not present in "
           "first region.");

    // Try locally destroying already deleted entry from regions, It should
    // result
    // into exception.
    try {
      reg0->localDestroy(keys[0]);
      FAIL(
          "local destroy on already removed key should have thrown "
          "EntryNotFoundException");
    } catch (EntryNotFoundException &) {
      LOG("Got expected EntryNotFoundException for "
          "localDestroy operation on already removed entry.");
    }

    LOG("StepFiveA complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT1, StepTwelve)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);

    int intKey = 1;
    int intVal = 1;
    regPtr0->localCreate(intKey, CacheableInt32::create(intVal));
    auto vInt = std::dynamic_pointer_cast<CacheableInt32>(regPtr0->get(intKey));
    ASSERT(vInt->value() == 1, "Int Key Int Val Mismatch.");
    regPtr0->localInvalidate(intKey);
    vInt = std::dynamic_pointer_cast<CacheableInt32>(regPtr0->get(intKey));
    ASSERT(vInt == nullptr, "valueshould not be found from sever");

    intKey = 2;
    const char *strVal = "IntKeyStringValue";
    regPtr0->localCreate(intKey, strVal);
    auto vString =
        std::dynamic_pointer_cast<CacheableString>(regPtr0->get(intKey));
    ASSERT(strcmp(vString->value().c_str(), strVal) == 0,
           "Int Key and String Value Mismatch");

    int key1 = 1;
    int val1 = 20;
    try {
      regPtr0->localCreate(CacheableInt32::create(key1),
                           CacheableInt32::create(val1));
      auto vall = std::dynamic_pointer_cast<CacheableInt32>(regPtr0->get(key1));
      FAIL("Expected EntryExistException here");
    } catch (EntryExistsException &) {
      LOG(" Expected EntryExistsException exception thrown by localCreate");
    }

    int64_t longKey = 1000;
    int64_t longVal = 200;
    regPtr0->localCreate(CacheableInt64::create(longKey),
                         CacheableInt64::create(longVal));
    auto vLong = std::dynamic_pointer_cast<CacheableInt64>(
        regPtr0->get(CacheableInt64::create(longKey)));
    ASSERT(vLong->value() == 200, "Long Key Long Val Mismatch.");

    const char *strKey = "StrKeyIntValueKey";
    intVal = 1234;
    regPtr0->localCreate(strKey, CacheableInt32::create(intVal));
    auto vIntPtr =
        std::dynamic_pointer_cast<CacheableInt32>(regPtr0->get(strKey));
    ASSERT(vIntPtr->value() == 1234, "String Key Int Val Mismatch.");

    longKey = 1111;
    strVal = "LongKeyStringValue";
    regPtr0->localCreate(CacheableInt64::create(longKey), strVal);
    auto vStr = std::dynamic_pointer_cast<CacheableString>(
        regPtr0->get(CacheableInt64::create(longKey)));
    ASSERT(strcmp(vStr->value().c_str(), strVal) == 0,
           "Long Key and String Value Mismatch");

    strKey = "StringKey1";
    strVal = "StringKeyStringValue";
    regPtr0->localCreate(strKey, strVal);
    auto v = std::dynamic_pointer_cast<CacheableString>(regPtr0->get(strKey));
    ASSERT(strcmp(v->value().c_str(), strVal) == 0,
           "String Key and String Value Mismatch");

    int i = 1234;
    regPtr0->localCreate(CacheableInt32::create(i), CacheableInt32::create(i));
    auto result = std::dynamic_pointer_cast<CacheableInt32>(
        regPtr0->get(CacheableInt32::create(i)));
    ASSERT(result->value() == i,
           "localcreate new entry with cacheableInt key and cacheableInt value "
           "Fail");

    regPtr0->localCreate(CacheableInt32::create(12345),
                         CacheableString::create("abced"));
    auto resultString = std::dynamic_pointer_cast<CacheableString>(
        regPtr0->get(CacheableInt32::create(12345)));
    ASSERT(strcmp(resultString->value().c_str(), "abced") == 0,
           "localcreate new with entry cacheableInt key and cacheablestring "
           "value Fail");

    regPtr0->localCreate(CacheableString::create("X"),
                         CacheableString::create("Y"));
    auto resultStringY = std::dynamic_pointer_cast<CacheableString>(
        regPtr0->get(CacheableString::create("X")));
    ASSERT(strcmp(resultStringY->value().c_str(), "Y") == 0,
           "localcreate new with entry cacheablestring key and cacheablestring "
           "value");

    try {
      regPtr0->localCreate(CacheableString::create("X"),
                           CacheableString::create("Y"));
      FAIL(
          "Should have thrown EntryExistsException if entry already exist in "
          "region.");
    } catch (EntryExistsException &) {
      LOG("Expected : check EntryExistsException if entry already exist in "
          "region.");
    }

    regPtr0->localDestroy(CacheableString::create("X"));
    if (std::dynamic_pointer_cast<CacheableString>(
            regPtr0->get(CacheableString::create("X"))) != nullptr) {
      LOG("Expected : localDestroy should have failed.");
    }

    std::shared_ptr<CacheableInt32> x;
    try {
      regPtr0->localCreate(x, 1);
      LOG("Entry with null key and value locally created successfully");
      FAIL("Expected IllegalArgumentException here");
    } catch (IllegalArgumentException &ex) {
      LOGINFO("Expected IllegalArgumentException : %s", ex.what());
    }

    try {
      regPtr0->localPut(x, 1);
      LOG("Entry with null key and value locally put successfully");
      FAIL("Expected IllegalArgumentException here");
    } catch (IllegalArgumentException &ex) {
      LOGINFO("Expected IllegalArgumentException : %s", ex.what());
    }

    try {
      regPtr0->localDestroy(x);
      LOG("Entry with null key locally deleted successfully");
      FAIL("Expected IllegalArgumentException here");
    } catch (IllegalArgumentException &ex) {
      LOGINFO("Expected IllegalArgumentException : %s", ex.what());
    }
    try {
      regPtr0->localInvalidate(x);
      LOG("Entry with null key locally invalidated successfully");
      FAIL("Expected IllegalArgumentException here");
    } catch (IllegalArgumentException &ex) {
      LOGINFO("Expected IllegalArgumentException : %s", ex.what());
    }
    try {
      regPtr0->localRemove(x, 1);
      LOG("Entry with null key and value locally removed successfully");
      FAIL("Expected IllegalArgumentException here");
    } catch (IllegalArgumentException &ex) {
      LOGINFO("Expected IllegalArgumentException : %s", ex.what());
    }
    try {
      regPtr0->localRemoveEx(x);
      LOG("Entry with null key locally removed if value exist successfully");
      FAIL("Expected IllegalArgumentException here");
    } catch (IllegalArgumentException &ex) {
      LOGINFO("Expected IllegalArgumentException : %s", ex.what());
    }

    auto keyObject1 = std::make_shared<PdxTests::PdxType>();
    regPtr0->localCreate(keyObject1, x);
    auto retVal = regPtr0->get(keyObject1);
    ASSERT(retVal == x, "retVal and x should match.");
    regPtr0->localInvalidate(keyObject1);
    retVal = regPtr0->get(keyObject1);
    ASSERT(retVal == nullptr, "value should not be found");
    ASSERT(regPtr0->localRemove(keyObject1, x) == true,
           "Result of remove should be true, as this value nullptr exists "
           "locally.");
    ASSERT(regPtr0->containsKey(keyObject1) == false,
           "containsKey should be false");
    regPtr0->localCreate(keyObject1, x);
    ASSERT(regPtr0->localRemoveEx(keyObject1) == true,
           "Result of remove should be true, as this value does not exists "
           "locally.");
    ASSERT(regPtr0->containsKey(keyObject1) == false,
           "containsKey should be false");
    regPtr0->localCreate(keyObject1, x);
    regPtr0->localDestroy(keyObject1);
    ASSERT(regPtr0->containsKey(keyObject1) == false,
           "containsKey should be false");
    try {
      retVal = regPtr0->get(x);
      ASSERT(retVal == nullptr, "value should not be found");
      FAIL("Expected IllegalArgumentException here for get");
    } catch (Exception &) {
      LOG(" Expected exception thrown by get");
    }

    try {
      regPtr0->localPut(keyObject1, x);
    } catch (IllegalArgumentException &ex) {
      LOGINFO("Expected IllegalArgumentException : %s", ex.what());
    }
    retVal = regPtr0->get(keyObject1);
    ASSERT(retVal == x, "retVal and x should match.");
    regPtr0->localPut(keyObject1, 1);
    regPtr0->localInvalidate(keyObject1);
    retVal = regPtr0->get(keyObject1);
    ASSERT(retVal == nullptr, "value should not be found");
    ASSERT(regPtr0->localRemove(keyObject1, 1) == false,
           "Result of remove should be false, as this value does not exists "
           "locally.");
    ASSERT(regPtr0->containsKey(keyObject1) == true,
           "containsKey should be true");
    regPtr0->localPut(keyObject1, 1);
    ASSERT(regPtr0->localRemoveEx(keyObject1) == true,
           "Result of remove should be true, as this value exists locally.");
    ASSERT(regPtr0->containsKey(keyObject1) == false,
           "containsKey should be false");
    auto keyObject2 = std::make_shared<PdxTests::PdxType>();
    regPtr0->localCreate(keyObject2, 1);
    auto intVal1 =
        std::dynamic_pointer_cast<CacheableInt32>(regPtr0->get(keyObject2));
    ASSERT(intVal1->value() == 1, "intVal should be 1.");
    regPtr0->invalidate(keyObject2);
    intVal1 =
        std::dynamic_pointer_cast<CacheableInt32>(regPtr0->get(keyObject2));
    ASSERT(intVal1 == nullptr, "intVal should be null.");

    auto keyObject3 = std::make_shared<PdxTests::PdxType>();
    regPtr0->localCreate(keyObject3, "testString");
    if (regPtr0->containsKey(keyObject3)) {
      auto strVal1 = regPtr0->get(keyObject3);
      ASSERT(strcmp(strVal1->toString().c_str(), "testString") == 0,
             "strVal should be testString.");
    }
    try {
      if (regPtr0->containsKey(keyObject3)) {
        regPtr0->localCreate(keyObject3, 1);
        FAIL("Expected EntryExistException here");
      }
    } catch (EntryExistsException &) {
      LOG(" Expected EntryExistsException exception thrown by localCreate");
    }

    auto keyObject4 = std::make_shared<PdxTests::PdxType>();
    auto valObject1 = std::make_shared<PdxTests::PdxType>();
    regPtr0->localCreate(keyObject4, valObject1);
    auto objVal1 =
        std::dynamic_pointer_cast<PdxTests::PdxType>(regPtr0->get(keyObject4));
    ASSERT(valObject1 == objVal1, "valObject and objVal should match.");
    regPtr0->localInvalidate(keyObject4);
    objVal1 =
        std::dynamic_pointer_cast<PdxTests::PdxType>(regPtr0->get(keyObject4));
    ASSERT(objVal1 == nullptr, "valueshould not be found");
    ASSERT(regPtr0->localRemove(keyObject4, valObject1) == false,
           "Result of remove should be false, as this value does not exists "
           "locally.");
    ASSERT(regPtr0->containsKey(keyObject4) == true,
           "containsKey should be true");
    regPtr0->localPut(keyObject4, valObject1);
    ASSERT(regPtr0->localRemove(keyObject4, valObject1) == true,
           "Result of remove should be true, as this value exists locally.");
    ASSERT(regPtr0->containsKey(keyObject4) == false,
           "containsKey should be false");
    regPtr0->localPut(keyObject4, valObject1);
    ASSERT(regPtr0->localRemoveEx(keyObject4) == true,
           "Result of remove should be true, as this value exists locally.");
    ASSERT(regPtr0->containsKey(keyObject4) == false,
           "containsKey should be false");
    regPtr0->localPut(keyObject4, valObject1);
    regPtr0->localDestroy(keyObject4);

    createRegion("ABC", USE_ACK, true, true);
    auto regPtr2 = getHelper()->getRegion("ABC");
    regPtr2->localDestroyRegion();
    try {
      regPtr2->localCreate(CacheableString::create("XY"),
                           CacheableString::create("Y"));
      FAIL("localDestroyRegion should have thrown exception");
    } catch (RegionDestroyedException &) {
      LOG(" Expected exception thrown by localCreate, since Region does not "
          "exist");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepSixA)
  {
    auto reg0 = getHelper()->getRegion(regionNames[0]);

    auto keyPtr = CacheableString::create(keys[1]);
    auto keyPtr1 = CacheableString::create(keys[3]);

    putEntry(regionNames[0], keys[1], nvals[1]);

    // Try removing value that is present on client as well as server, result
    // should be true.
    ASSERT(
        reg0->removeEx(keys[1]) == true,
        "Result of remove should be true, as this value is present locally, & "
        "also present on server.");
    ASSERT(reg0->containsKey(keys[1]) == false, "containsKey should be false");
    ASSERT(reg0->containsKeyOnServer(keyPtr) == false,
           "containsKeyOnServer should be false");
    LOGINFO("Step 6a.1 complete.");

    // Try removing value that is invalidated on client but exists on server,
    // result should be false.
    try {
      reg0->destroy(keys[0]);
      FAIL(
          "destroy on already removed key should have thrown "
          "EntryNotFoundException");
    } catch (EntryNotFoundException &) {
      LOG("Got expected EntryNotFoundException for "
          "destroy operation on already removed entry.");
    }

    putEntry(regionNames[0], keys[1], nvals[1]);
    reg0->invalidate(keys[1]);
    ASSERT(
        reg0->removeEx(keys[1]) == true,
        "Result of remove should be true, as invalidated key can be deleted.");
    LOGINFO("Step 6a.2 complete.");

    // Try removing a entry (value) which is not present on client as well as
    // server, result should be false.
    ASSERT(reg0->removeEx("NewKey1") == false,
           "Result of remove should be false, as this value is not present.");
    auto keyPtr2 = CacheableString::create("NewKey1");
    auto keyPtr3 = CacheableString::create("NewKey3");
    ASSERT(reg0->containsKey("NewKey1") == false,
           "containsKey should be false");
    ASSERT(reg0->containsKeyOnServer(keyPtr2) == false,
           "containsKeyOnServer should be false");
    LOGINFO("Step 6a.3 complete.");

    // Try removing a entry (value) which is not present on client but exists on
    // the server, result should be true.
    putEntry(regionNames[0], keys[1], nvals[1]);
    reg0->localDestroy(keys[1]);
    ASSERT(reg0->removeEx(keys[1]) == true,
           "Result of remove should be true, as this value does not exist "
           "locally, but exists on server.");
    ASSERT(reg0->containsKey(keys[1]) == false, "containsKey should be false");
    ASSERT(reg0->containsKeyOnServer(keyPtr) == false,
           "containsKeyOnServer should be false");
    LOG("Step 6a.4 complete.");

    //-------------------------------------localRemove
    // Testcases------------------------------------------------
    // Try locally removing an entry (value) which is present on the client.
    reg0->localPut(keys[1], vals[1]);
    ASSERT(reg0->localRemoveEx(keys[1]) == true,
           "Result of remove should be true, as this value exists locally.");
    ASSERT(reg0->containsKey(keys[1]) == false, "containsKey should be false");
    LOG("Step 6a.5 complete.");

    // Try local destroy on entry that is already removed, should get an
    // exception.
    try {
      reg0->localDestroy(keys[1]);
      FAIL(
          "local destroy on already removed key should have thrown "
          "EntryNotFoundException");
    } catch (EntryNotFoundException &) {
      LOG("Got expected EntryNotFoundException for "
          "localDestroy operation on already removed entry.");
    }

    LOG("Step 6a.6 complete.");

    // Try locally removing an entry (value) which is invalidated with a value.
    reg0->localPut(keys[1], vals[1]);
    reg0->invalidate(keys[1]);
    ASSERT(reg0->localRemoveEx(keys[1]) == true,
           "Result of remove should be true.");
    ASSERT(reg0->containsKey(keys[1]) == false, "containsKey should be true");
    LOG("Step 6a.7 complete.");

    // Try locally removing an entry which is locally destroyed with a value.
    reg0->localPut(keys[1], vals[1]);
    reg0->localDestroy(keys[1]);
    ASSERT(reg0->localRemoveEx(keys[1]) == false,
           "Result of remove should be false, as this value does not exists "
           "locally.");
    ASSERT(reg0->containsKey(keys[1]) == false, "containsKey should be true");
    LOG("Step 6a.8 complete.");

    // Try locally removing an entry which is already removed.
    reg0->localPut(keys[1], vals[1]);
    ASSERT(reg0->localRemoveEx(keys[1]) == true,
           "Result of remove should be true, as this value does not exists "
           "locally.");
    ASSERT(reg0->localRemoveEx(keys[1]) == false,
           "Result of remove should be false, as this value does not exists "
           "locally.");
    ASSERT(reg0->containsKey(keys[1]) == false, "containsKey should be false");
    LOG("Step 6a.9 complete.");
    // Try locally removing an entry when region scope is not null.

    LOG("StepSixA complete.");
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

DUNIT_TASK_DEFINITION(SERVER1, CloseLocator1)
  {
    // stop locator
    if (isLocator) {
      CacheHelper::closeLocator(1);
      LOG("Locator1 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CloseLocator2)
  {
    // stop locator
    if (isLocator) {
      CacheHelper::closeLocator(2);
      LOG("Locator2 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateLocator1)
  {
    // starting locator
    if (isLocator) CacheHelper::initLocator(1);
    LOG("Locator1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateLocator2)
  {
    // starting locator
    if (isLocator) CacheHelper::initLocator(2);
    LOG("Locator2 started");
  }
END_TASK_DEFINITION

void runRemoveOps() {
  CALL_TASK(CreateLocator1);
  CALL_TASK(CreateServer1_With_Locator);

  CALL_TASK(StepOne_Pooled_Locator);
  CALL_TASK(StepTwo_Pooled_Locator);

  CALL_TASK(StepThree);
  CALL_TASK(StepFive);
  CALL_TASK(StepFiveA);
  CALL_TASK(StepSix);
  CALL_TASK(StepSixA);
  CALL_TASK(StepTwelve);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer1);

  CALL_TASK(CloseLocator1);
}

void runRemoveOps1() {
  CALL_TASK(CreateLocator2);
  CALL_TASK(CreateServer2_locator);

  CALL_TASK(StepSeven_Pooled_Locator);

  CALL_TASK(StepEight);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseServer2);
  CALL_TASK(CloseLocator2);
}

DUNIT_MAIN
  {
    runRemoveOps();
    runRemoveOps1();
  }
END_MAIN
