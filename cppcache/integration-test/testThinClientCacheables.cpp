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

#define ROOT_NAME "testThinClientCacheables"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "fw_dunit.hpp"
#include "BuiltinCacheableWrappers.hpp"
#include "Utils.hpp"

#include <iostream>
#include <string>

#include "CacheHelper.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1

using apache::geode::client::Cacheable;
using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheHelper;
using apache::geode::client::EntryNotFoundException;
using apache::geode::client::Region;
using apache::geode::client::Utils;
using apache::geode::client::internal::DSCode;

using apache::geode::client::testing::CacheableWrapper;
using apache::geode::client::testing::CacheableWrapperFactory;

CacheHelper *cacheHelper = nullptr;
bool isLocalServer = false;

#if defined(WIN32)
// because we run out of memory on our pune windows desktops
#define DEFAULTNUMKEYS 5
#else
#define DEFAULTNUMKEYS 15
#endif
#define KEYSIZE 256
#define VALUESIZE 1024

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

CacheHelper *getHelper() {
  ASSERT(cacheHelper != nullptr, "No cacheHelper initialized.");
  return cacheHelper;
}

void createRegion(const char *name, bool ackMode,
                  bool clientNotificationEnabled = false) {
  LOG("createRegion() entered.");
  std::cout << "Creating region --  " << name << "  ackMode is " << ackMode
            << "\n"
            << std::flush;
  auto regPtr = getHelper()->createRegion(name, ackMode, true, nullptr,
                                          clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}

void checkGets(int maxKeys, DSCode keyTypeId, DSCode valTypeId,
               const std::shared_ptr<Region> &dataReg,
               const std::shared_ptr<Region> &verifyReg) {
  for (int i = 0; i < maxKeys; i++) {
    CacheableWrapper *tmpkey =
        CacheableWrapperFactory::createInstance(keyTypeId);
    ASSERT(tmpkey != nullptr, "tmpkey is nullptr");
    CacheableWrapper *tmpval =
        CacheableWrapperFactory::createInstance(valTypeId);
    ASSERT(tmpval != nullptr, "tmpval is nullptr");
    tmpkey->initKey(i, KEYSIZE);
    auto key = std::dynamic_pointer_cast<CacheableKey>(tmpkey->getCacheable());
    auto val = dataReg->get(key);
    // also check that value is in local cache
    auto entry = dataReg->getEntry(key);
    ASSERT(entry != nullptr, "entry is nullptr");
    auto localVal = entry->getValue();
    uint32_t keychksum = tmpkey->getCheckSum();
    auto int32val = std::dynamic_pointer_cast<CacheableInt32>(
        verifyReg->get(static_cast<int32_t>(keychksum)));
    if (int32val == nullptr) {
      std::cout << "GetsTask::keychksum: " << keychksum
                << ", key: " << Utils::nullSafeToString(key) << "\n";
      FAIL("Could not find the checksum for the given key.");
    }
    uint32_t valchksum = static_cast<uint32_t>(int32val->value());
    uint32_t gotValChkSum = tmpval->getCheckSum(val);
    uint32_t gotLocalValChkSum = tmpval->getCheckSum(localVal);
    ASSERT(valchksum == gotValChkSum, "Expected valchksum == gotValChkSum");
    ASSERT(valchksum == gotLocalValChkSum,
           "Expected valchksum == gotLocalValChkSum");
    delete tmpkey;
    delete tmpval;
  }
}

const char *regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

const bool USE_ACK = true;
const bool NO_ACK = false;

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    if (isLocalServer) CacheHelper::initServer(1);
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne)
  {
    initClient(true);
    createRegion(regionNames[0], USE_ACK);
    createRegion(regionNames[1], NO_ACK);
    CacheableHelper::registerBuiltins();
    LOG("StepOne complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo)
  {
    initClient(true);
    createRegion(regionNames[0], USE_ACK);
    createRegion(regionNames[1], NO_ACK);
    CacheableHelper::registerBuiltins();
    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, PutsTask)
  {
    LOG("PutsTask started.");
    static int taskIndexPut = 0;

    auto keyTypes = CacheableWrapperFactory::getRegisteredKeyTypes();
    auto valueTypes = CacheableWrapperFactory::getRegisteredValueTypes();

    size_t keyTypeIndex = taskIndexPut / valueTypes.size();
    size_t valueTypeIndex = taskIndexPut % valueTypes.size();

    DSCode keyTypeId = keyTypes[keyTypeIndex];
    DSCode valTypeId = valueTypes[valueTypeIndex];

    std::cout << "PutsTask::keyType = "
              << CacheableWrapperFactory::getTypeForId(keyTypeId)
              << " and valType = "
              << CacheableWrapperFactory::getTypeForId(valTypeId)
              << " and taskIndexPut = " << taskIndexPut << "\n";

    CacheableWrapper *key = CacheableWrapperFactory::createInstance(keyTypeId);
    int maxKeys =
        (key->maxKeys() < DEFAULTNUMKEYS ? key->maxKeys() : DEFAULTNUMKEYS);
    delete key;

    auto dataReg = getHelper()->getRegion(regionNames[0]);
    auto verifyReg = getHelper()->getRegion(regionNames[1]);
    for (int i = 0; i < maxKeys; i++) {
      CacheableWrapper *tmpkey =
          CacheableWrapperFactory::createInstance(keyTypeId);
      CacheableWrapper *tmpval =
          CacheableWrapperFactory::createInstance(valTypeId);
      tmpkey->initKey(i, KEYSIZE);
      tmpval->initRandomValue(CacheableHelper::random(VALUESIZE) + 1);
      ASSERT(tmpkey->getCacheable() != nullptr,
             "tmpkey->getCacheable() is nullptr");
      // we can have nullptr values now after fix for bug #294
      if (tmpval->getCacheable() != nullptr) {
        dataReg->put(
            std::dynamic_pointer_cast<CacheableKey>(tmpkey->getCacheable()),
            tmpval->getCacheable());
      } else {
        try {
          dataReg->destroy(
              std::dynamic_pointer_cast<CacheableKey>(tmpkey->getCacheable()));
        } catch (const EntryNotFoundException &) {
          // expected
        }
        dataReg->create(
            std::dynamic_pointer_cast<CacheableKey>(tmpkey->getCacheable()),
            tmpval->getCacheable());
      }
      uint32_t keychksum = tmpkey->getCheckSum();
      uint32_t valchksum = tmpval->getCheckSum();
      verifyReg->put(static_cast<int32_t>(keychksum),
                     static_cast<int32_t>(valchksum));
      // also check that value is in local cache
      auto entry = dataReg->getEntry(
          std::dynamic_pointer_cast<CacheableKey>(tmpkey->getCacheable()));
      std::shared_ptr<Cacheable> localVal;
      if (entry != nullptr) {
        localVal = entry->getValue();
      }
      uint32_t localValChkSum = tmpval->getCheckSum(localVal);
      ASSERT(valchksum == localValChkSum,
             "Expected valchksum == localValChkSum");
      delete tmpkey;
      delete tmpval;
    }
    taskIndexPut++;
    LOG("PutsTask completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, GetsTask)
  {
    LOG("GetsTask started.");
    static int taskIndexGet = 0;

    auto keyTypes = CacheableWrapperFactory::getRegisteredKeyTypes();
    auto valueTypes = CacheableWrapperFactory::getRegisteredValueTypes();

    size_t keyTypeIndex = taskIndexGet / valueTypes.size();
    size_t valueTypeIndex = taskIndexGet % valueTypes.size();

    DSCode keyTypeId = keyTypes[keyTypeIndex];
    DSCode valTypeId = valueTypes[valueTypeIndex];

    std::cout << "GetsTask::keyType = "
              << CacheableWrapperFactory::getTypeForId(keyTypeId)
              << " and valType = "
              << CacheableWrapperFactory::getTypeForId(valTypeId)
              << " and taskIndexGet = " << taskIndexGet << "\n";

    CacheableWrapper *key = CacheableWrapperFactory::createInstance(keyTypeId);
    int maxKeys =
        (key->maxKeys() < DEFAULTNUMKEYS ? key->maxKeys() : DEFAULTNUMKEYS);
    delete key;

    auto dataReg = getHelper()->getRegion(regionNames[0]);
    auto verifyReg = getHelper()->getRegion(regionNames[1]);
    dataReg->localInvalidateRegion();
    verifyReg->localInvalidateRegion();
    checkGets(maxKeys, keyTypeId, valTypeId, dataReg, verifyReg);

    // Also check after running a region query. This ensures that the values
    // have deserialized on server so checks serialization/deserialization
    // compatibility with java server.
    std::string queryStr = "SELECT DISTINCT iter.key, iter.value FROM /";
    queryStr += (std::string(regionNames[0]) + ".entrySet AS iter");
    dataReg->query(queryStr.c_str());
    dataReg->localInvalidateRegion();
    verifyReg->localInvalidateRegion();
    checkGets(maxKeys, keyTypeId, valTypeId, dataReg, verifyReg);

    taskIndexGet++;
    LOG("GetsTask completed.");
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

DUNIT_MAIN
  {
    CacheableHelper::registerBuiltins();
    CALL_TASK(CreateServer1);
    CALL_TASK(StepOne);
    CALL_TASK(StepTwo);
    size_t totKeyTypes =
        CacheableWrapperFactory::getRegisteredKeyTypes().size();
    size_t totValTypes =
        CacheableWrapperFactory::getRegisteredValueTypes().size();
    for (size_t i = 0; i < totKeyTypes; i++) {
      for (size_t j = 0; j < totValTypes; j++) {
        CALL_TASK(PutsTask);
        CALL_TASK(GetsTask);
      }
    }
    CALL_TASK(CloseCache1);
    CALL_TASK(CloseCache2);
    CALL_TASK(CloseServer1);
  }
END_MAIN
