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

#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTHELPER_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTHELPER_H_

#include <chrono>

#include <ace/High_Res_Timer.h>

#include <boost/process.hpp>

#include "testUtils.hpp"
#include "security/typedefs.hpp"
#include "security/CredentialGenerator.hpp"
#include "security/DummyCredentialGenerator.hpp"
#include "security/CredentialGenerator.cpp"

#include <string>

#ifndef ROOT_NAME
#define ROOT_NAME "ThinClientHelper"
#endif

#ifndef ROOT_SCOPE
#define ROOT_SCOPE DISTRIBUTED_ACK
#endif

#include "CacheHelper.hpp"

namespace {  // NOLINT(google-build-namespaces)

using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheHelper;
using apache::geode::client::CacheListener;
using apache::geode::client::DiskPolicyType;
using apache::geode::client::ExpirationAction;
using apache::geode::client::Pool;
using apache::geode::client::Properties;
using apache::geode::client::RegionAttributesFactory;
using apache::geode::client::Serializable;
using unitTests::TestUtils;

CacheHelper* cacheHelper = nullptr;

void initClient(const bool isthinClient,
                const std::shared_ptr<Properties>& configPtr = nullptr) {
  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(isthinClient, configPtr);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

void initClientWithPool(const bool isthinClient, const char* poolName,
                        const std::string& locators, const char* serverGroup,
                        const std::shared_ptr<Properties>& configPtr = nullptr,
                        int redundancy = 0, bool clientNotification = false,
                        int subscriptionAckInterval = -1, int connections = -1,
                        int loadConditioningInterval = -1,
                        bool prSingleHop = false, bool threadLocal = false) {
  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(
        isthinClient, poolName, locators, serverGroup, configPtr, redundancy,
        clientNotification, subscriptionAckInterval, connections,
        loadConditioningInterval, false, prSingleHop, threadLocal);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

/* For HA Clients */
void initClient(int redundancyLevel,
                const std::shared_ptr<Properties>& configPtr = nullptr) {
  if (cacheHelper == nullptr) {
    auto config = configPtr;
    if (config == nullptr) {
      config = Properties::create();
    }
    cacheHelper = new CacheHelper(redundancyLevel, config);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

void cleanProc() {
  if (cacheHelper != nullptr) {
    delete cacheHelper;
    cacheHelper = nullptr;
  }
}

void netDown() {
  if (cacheHelper != nullptr) {
    TestUtils::getCacheImpl(cacheHelper->cachePtr)->netDown();
  }
}

void revive() {
  if (cacheHelper != nullptr) {
    TestUtils::getCacheImpl(cacheHelper->cachePtr)->revive();
  }
}

void crashClient() {
  if (cacheHelper != nullptr) {
    TestUtils::getCacheImpl(cacheHelper->cachePtr)->setClientCrashTEST();
  }
}

CacheHelper* getHelper() {
  ASSERT(cacheHelper != nullptr, "No cacheHelper initialized.");
  return cacheHelper;
}

const char* testregex[] = {"Key-*1", "Key-*2", "Key-*3",
                           "Key-*4", "Key-*5", "Key-*6"};
const char* keys[] = {"Key-1", "Key-2", "Key-3", "Key-4", "Key-5", "Key-6"};
const char* vals[] = {"Value-1", "Value-2", "Value-3",
                      "Value-4", "Value-5", "Value-6"};
const char* nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4", "New Value-5", "New Value-6"};

const char* regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

const bool USE_ACK = true;
const bool NO_ACK = false;

void _verifyEntry(const std::string& name, const char* key, const char* val,
                  bool noKey, bool checkVal = true) {
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

  if (noKey == false) {  // need to find the key!
    ASSERT(regPtr->containsKey(keyPtr), "Key not found in region.");
  }
  if (val != nullptr && checkVal) {  // need to have a value!
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

void verifyEntry(const std::string& name, const char* key, const char* val,
                 bool checkVal = true) {
  LOG(std::string("verifyEntry() called from ") + std::to_string(__LINE__) +
      "\n");
  _verifyEntry(name, key, val, false, checkVal);
  LOG("Entry verified.");
}

void _verifyIntEntry(const char* name, const char* key, const int val,
                     bool noKey, bool isCreated = false) {
  // Verify key and value exist in this region, in this process.
  int value = val;
  std::string msg;
  char* buf = reinterpret_cast<char*>(malloc(1024 + strlen(key) + 20));
  ASSERT(buf, "Unable to malloc buffer for logging.");
  if (!isCreated) {
    if (noKey) {
      msg = std::string("Verify key ") + key + " does not exist in region " +
            name;
    } else if (val == 0) {
      msg = std::string("Verify value for key ") + key +
            " does not exist in region " + name;
    } else {
      msg = std::string("Verify value for key ") + key +
            " is: " + std::to_string(value) + " in region " + name;
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

#define verifyIntEntry(x, y, z) _verifyIntEntry(x, y, z, __LINE__)
void _verifyIntEntry(const char* name, const char* key, const int val,
                     int line) {
  LOG(std::string("verifyIntEntry() called from ") + std::to_string(line) +
      "\n");
  _verifyIntEntry(name, key, val, false);
  LOG("Entry verified.");
}

void createRegion(const char* name, bool ackMode,
                  bool clientNotificationEnabled = false,
                  const std::shared_ptr<CacheListener>& listener = nullptr,
                  bool caching = true) {
  LOG("createRegion() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  // ack, caching
  auto regPtr = getHelper()->createRegion(name, ackMode, caching, listener,
                                          clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}
std::shared_ptr<Region> createOverflowRegion(const char* name, bool,
                                             int lel = 0, bool caching = true) {
  std::string bdb_dir = "BDB";
  std::string bdb_dirEnv = "BDBEnv";
  RegionAttributesFactory regionAttributesFactory;
  regionAttributesFactory.setCachingEnabled(caching);
  regionAttributesFactory.setLruEntriesLimit(lel);
  regionAttributesFactory.setDiskPolicy(DiskPolicyType::OVERFLOWS);

  auto sqLiteProps = Properties::create();
  sqLiteProps->insert("PageSize", "65536");
  sqLiteProps->insert("MaxPageCount", "1073741823");
  std::string sqlite_dir =
      "SqLiteRegionData" + std::to_string(boost::this_process::get_id());
  sqLiteProps->insert("PersistenceDirectory", sqlite_dir.c_str());
  regionAttributesFactory.setPersistenceManager(
      "SqLiteImpl", "createSqLiteInstance", sqLiteProps);

  auto regionAttributes = regionAttributesFactory.create();
  auto cache = getHelper()->cachePtr;
  CacheImpl* cacheImpl = CacheRegionHelper::getCacheImpl(cache.get());
  std::shared_ptr<Region> regionPtr;
  cacheImpl->createRegion(name, regionAttributes, regionPtr);
  return regionPtr;
}
std::shared_ptr<Region> createPooledRegion(
    const char* name, bool ackMode, const std::string& locators,
    const char* poolname, bool clientNotificationEnabled = false,
    const std::shared_ptr<CacheListener>& listener = nullptr,
    bool caching = true) {
  LOG("createPooledRegion() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);

  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(true, poolname, locators, nullptr);
  }

  // ack, caching
  auto regPtr = getHelper()->createPooledRegion(
      name, ackMode, locators, poolname, caching, clientNotificationEnabled,
      std::chrono::seconds(0), std::chrono::seconds(0), std::chrono::seconds(0),
      std::chrono::seconds(0), 0, listener);

  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
  return regPtr;
}
std::shared_ptr<Pool> findPool(const char* poolName) {
  LOG("findPool() entered.");
  auto poolPtr = getHelper()->getCache()->getPoolManager().find(poolName);
  ASSERT(poolPtr != nullptr, "Failed to find pool.");
  return poolPtr;
}
std::shared_ptr<Pool> createPool(
    const std::string& poolName, const std::string& locators,
    const std::string& serverGroup, int redundancy = 0,
    bool clientNotification = false,
    std::chrono::milliseconds subscriptionAckInterval =
        std::chrono::milliseconds::zero(),
    int connections = -1, int loadConditioningInterval = -1) {
  LOG("createPool() entered.");

  auto poolPtr = getHelper()->createPool(
      poolName, locators, serverGroup, redundancy, clientNotification,
      subscriptionAckInterval, connections, loadConditioningInterval);
  ASSERT(poolPtr != nullptr, "Failed to create pool.");
  LOG("Pool created.");
  return poolPtr;
}
std::shared_ptr<Pool> createPoolAndDestroy(
    const std::string& poolName, const std::string& locators,
    const std::string& serverGroup, int redundancy = 0,
    bool clientNotification = false,
    std::chrono::milliseconds subscriptionAckInterval =
        std::chrono::milliseconds::zero(),
    int connections = -1) {
  LOG("createPoolAndDestroy() entered.");

  auto poolPtr = getHelper()->createPool(poolName, locators, serverGroup,
                                         redundancy, clientNotification,
                                         subscriptionAckInterval, connections);
  ASSERT(poolPtr != nullptr, "Failed to create pool.");
  poolPtr->destroy();
  LOG("Pool created and destroyed.");
  return poolPtr;
}
// this will create pool even endpoints and locatorhost has been not defined
std::shared_ptr<Pool> createPool2(const std::string& poolName,
                                  const std::string& locators,
                                  const std::string& serverGroup,
                                  const std::string& servers = nullptr,
                                  int redundancy = 0,
                                  bool clientNotification = false) {
  LOG("createPool2() entered.");

  auto poolPtr = getHelper()->createPool2(
      poolName, locators, serverGroup, servers, redundancy, clientNotification);
  ASSERT(poolPtr != nullptr, "Failed to create pool.");
  LOG("Pool created.");
  return poolPtr;
}
std::shared_ptr<Region> createRegionAndAttachPool(
    const std::string& name, bool ack, const std::string& poolName = "",
    bool caching = true,
    const std::chrono::seconds& ettl = std::chrono::seconds::zero(),
    const std::chrono::seconds& eit = std::chrono::seconds::zero(),
    const std::chrono::seconds& rttl = std::chrono::seconds::zero(),
    const std::chrono::seconds& rit = std::chrono::seconds::zero(), int lel = 0,
    ExpirationAction action = ExpirationAction::DESTROY) {
  LOG("createRegionAndAttachPool() entered.");
  auto regPtr = getHelper()->createRegionAndAttachPool(
      name, ack, poolName, caching, ettl, eit, rttl, rit, lel, action);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
  return regPtr;
}

void createEntry(const std::string& name, const char* key, const char* value) {
  LOG("createEntry() entered.");
  fprintf(stdout, "Creating entry -- key: %s  value: %s in region %s\n", key,
          value, name.c_str());
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

void updateEntry(const char* name, const char* key, const char* value,
                 bool checkVal = true, bool checkKey = true) {
  LOG("updateEntry() entered.");
  fprintf(stdout, "Updating entry -- key: %s  value: %s in region %s\n", key,
          value, name);
  fflush(stdout);
  // Update entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableString::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  if (checkKey) {
    ASSERT(regPtr->containsKey(keyPtr),
           "Key should have been found in region.");
  }
  if (checkVal) {
    ASSERT(regPtr->containsValueForKey(keyPtr),
           "Value should have been found in region.");
  }

  regPtr->put(keyPtr, valPtr);
  LOG("Put entry.");

  verifyEntry(name, key, value, checkVal);
  LOG("Entry updated.");
}

void doNetsearch(const char* name, const char* key, const char* value,
                 bool checkVal = true) {
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

  // ASSERT( !regPtr->containsKey( keyPtr ), "Key should not have been found in
  // region." );
  if (checkVal) {
    ASSERT(!regPtr->containsValueForKey(keyPtr),
           "Value should not have been found in region.");
  }

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

void createIntEntry(const char* name, const char* key, const int value,
                    bool onlyCreate = false) {
  LOG("createEntry() entered.");
  fprintf(stdout, "Creating entry -- key: %s  value: %d in region %s\n", key,
          value, name);
  fflush(stdout);

  // Create entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableInt32::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  if (onlyCreate) {
    ASSERT(!regPtr->containsKey(keyPtr),
           "Key should not have been found in region.");
    ASSERT(!regPtr->containsValueForKey(keyPtr),
           "Value should not have been found in region.");
  }

  regPtr->put(keyPtr, valPtr);
  LOG("Created entry.");

  verifyIntEntry(name, key, value);
  LOG("Entry created.");
}

void invalidateEntry(const char* name, const char* key) {
  LOG("invalidateEntry() entered.");
  fprintf(stdout, "Invalidating entry -- key: %s  in region %s\n", key, name);
  fflush(stdout);
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
  fprintf(stdout, "Destroying entry -- key: %s  in region %s\n", key, name);
  fflush(stdout);
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

void destroyRegion(const std::string& name) {
  LOG("destroyRegion() entered.");
  auto regPtr = getHelper()->getRegion(name);
  regPtr->localDestroyRegion();
  LOG("Region destroyed.");
}

class RegionOperations {
 public:
  explicit RegionOperations(const char* name)
      : m_regionPtr(getHelper()->getRegion(name)) {}

  void putOp(int keysMax = 1,
             const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) {
    for (int i = 1; i <= keysMax; i++) {
      auto key = std::string("key") + std::to_string(i);
      auto value = std::string("value") + std::to_string(i);
      auto valPtr = CacheableString::create(value);
      m_regionPtr->put(key, valPtr, aCallbackArgument);
    }
  }
  void invalidateOp(
      int keysMax = 1,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) {
    for (int i = 1; i <= keysMax; i++) {
      auto key = std::string("key") + std::to_string(i);
      std::string value;
      auto valPtr = CacheableString::create(value);
      m_regionPtr->localInvalidate(key, aCallbackArgument);
    }
  }
  void destroyOp(
      int keysMax = 1,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) {
    for (int i = 1; i <= keysMax; i++) {
      auto key = std::string("key") + std::to_string(i);
      std::string value;
      auto valPtr = CacheableString::create(value);
      m_regionPtr->destroy(key, aCallbackArgument);
    }
  }
  void removeOp(
      int keysMax = 1,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) {
    for (int i = 1; i <= keysMax; i++) {
      auto key = std::string("key") + std::to_string(i);
      auto value = std::string("value") + std::to_string(i);
      auto valPtr = CacheableString::create(value);
      m_regionPtr->remove(key, valPtr, aCallbackArgument);
    }
  }
  std::shared_ptr<Region> m_regionPtr;
};

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTHELPER_H_
