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

#define ROOT_NAME "testThinClientPRPutAllFailover"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "fw_dunit.hpp"
#include "BuiltinCacheableWrappers.hpp"
#include "Utils.hpp"

#include <ace/OS.h>
#include <ace/High_Res_Timer.h>
#include "TallyListener.hpp"
#include <string>
#include "CacheHelper.hpp"

// Include these 2 headers for access to CacheImpl for test hooks.
#include "CacheImplHelper.hpp"
#include "testUtils.hpp"

#include "ThinClientHelper.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s2p1
#define SERVER1 s1p2
#define SERVER2 s2p2

using apache::geode::client::Exception;
using apache::geode::client::HashMapOfCacheable;

using apache::geode::client::testing::TallyListener;

std::shared_ptr<TallyListener> reg1Listener1;
bool isLocalServer = false;
static bool isLocator = false;
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);

void setCacheListener(const char *regName,
                      std::shared_ptr<TallyListener> regListener) {
  auto reg = getHelper()->getRegion(regName);
  auto attrMutator = reg->getAttributesMutator();
  attrMutator->setCacheListener(regListener);
}

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    if (isLocalServer) CacheHelper::initServer(1, "cacheserver1_pr_putall.xml");
    LOG("SERVER1 started");
    if (isLocalServer) CacheHelper::initServer(2, "cacheserver2_pr_putall.xml");
    LOG("SERVER2 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2)
  {
    if (isLocalServer) CacheHelper::initServer(3, "cacheserver3_pr_putall.xml");
    LOG("SERVER3 started");
    if (isLocalServer) CacheHelper::initServer(4, "cacheserver4_pr_putall.xml");
    LOG("SERVER4 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pooled_Locator_Client1)
  {
    initClient(true);

    getHelper()->createPoolWithLocators("__TEST_POOL1__", locatorsG);
    getHelper()->createRegionAndAttachPool(regionNames[0], USE_ACK,
                                           "__TEST_POOL1__");

    reg1Listener1 = std::make_shared<TallyListener>();
    setCacheListener(regionNames[0], reg1Listener1);
    LOG("StepOne_Pooled_Locator_Client1 complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepOne_Pooled_Locator_Client2)
  {
    initClient(true);

    getHelper()->createPoolWithLocators("__TEST_POOL1__", locatorsG, true, 2);
    getHelper()->createRegionAndAttachPool(regionNames[0], USE_ACK,
                                           "__TEST_POOL1__");

    reg1Listener1 = std::make_shared<TallyListener>();
    setCacheListener(regionNames[0], reg1Listener1);
    LOG("StepOne_Pooled_Locator_Client2 complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo_RegisterAllKeys)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    regPtr0->registerAllKeys();
    LOG("StepTwo_Pooled_EndPoint_RegisterAllKeys complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, PutAllOneTask)
  {
    LOG("PutAllOneTask started.");
    auto dataReg = getHelper()->getRegion(regionNames[0]);
    HashMapOfCacheable entryMap;
    entryMap.clear();
    char key[256];
    char value[256];
    for (int32_t item = 0; item < 1000; item++) {
      sprintf(key, "key-%d", item);
      sprintf(value, "%d", item);
      entryMap.emplace(CacheableKey::create(key),
                       CacheableString::create(value));
      LOG_DEBUG(
          "CPPTEST:PutAllOneTask Doing PutAll on key using key: = %s: & value: "
          "= "
          "%s",
          key, value);
    }
    try {
      dataReg->putAll(entryMap);
    } catch (Exception &ex) {
      LOG_ERROR("CPPTEST: Unexpected %s: %s", ex.getName().c_str(), ex.what());
      FAIL(ex.what());
    } catch (...) {
      LOG_ERROR("CPPTEST: PutAll caused random exception in PutAllOneTask");
      cleanProc();
      FAIL("PutAll caused unexpected exception");
    }
    LOG("PutAllOneTask completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, PutAllTwoTask)
  {
    LOG("PutAllTwoTask started.");
    auto dataReg = getHelper()->getRegion(regionNames[0]);
    HashMapOfCacheable entryMap;
    entryMap.clear();
    char key[256];
    char value[256];
    for (int32_t item = 1000; item < 2000; item++) {
      sprintf(key, "key-%d", item);
      sprintf(value, "%d", item);
      LOG_DEBUG(
          "CPPTEST:PutAllTwoTask Doing PutAll on key using key: = %s: & value: "
          "= "
          "%s",
          key, value);
      entryMap.emplace(CacheableKey::create(key),
                       CacheableString::create(value));
    }
    try {
      dataReg->putAll(entryMap);
    } catch (Exception &ex) {
      LOG_ERROR("CPPTEST: Unexpected %s: %s", ex.getName().c_str(), ex.what());
      FAIL(ex.what());
    } catch (...) {
      LOG_ERROR("CPPTEST: PutAll caused random exception in PutAllTwoTask");
      cleanProc();
      FAIL("PutAll caused unexpected exception");
    }
    LOG("PutAllTwoTask completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, PutAllThreeTask)
  {
    LOG("PutAllThreeTask started.");
    auto dataReg = getHelper()->getRegion(regionNames[0]);
    HashMapOfCacheable entryMap;
    entryMap.clear();
    char key[256];
    char value[256];
    for (int32_t item = 2000; item < 3000; item++) {
      sprintf(key, "key-%d", item);
      sprintf(value, "%d", item);
      LOG_DEBUG(
          "CPPTEST:PutAllThreeTask Doing PutAll on key using key: = %s: & "
          "value: "
          "= %s",
          key, value);
      entryMap.emplace(CacheableKey::create(key),
                       CacheableString::create(value));
    }
    try {
      dataReg->putAll(entryMap);
    } catch (Exception &ex) {
      LOG_ERROR("CPPTEST: Unexpected %s: %s", ex.getName().c_str(), ex.what());
      FAIL(ex.what());
    } catch (...) {
      LOG_ERROR("CPPTEST: PutAll caused random exception in PutAllThreeTask");
      cleanProc();
      FAIL("PutAll caused unexpected exception");
    }
    LOG("PutAllThreeTask completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, PutAllFourTask)
  {
    LOG("PutAllFourTask started.");
    auto dataReg = getHelper()->getRegion(regionNames[0]);
    HashMapOfCacheable entryMap;
    entryMap.clear();
    char key[256];
    char value[256];
    for (int32_t item = 3000; item < 4000; item++) {
      sprintf(key, "key-%d", item);
      sprintf(value, "%d", item);
      LOG_DEBUG(
          "CPPTEST:PutAllFourTask Doing PutAll on key using key: = %s: & "
          "value: "
          "= %s",
          key, value);
      entryMap.emplace(CacheableKey::create(key),
                       CacheableString::create(value));
    }
    try {
      dataReg->putAll(entryMap);
    } catch (Exception &ex) {
      LOG_ERROR("CPPTEST: Unexpected %s: %s", ex.getName().c_str(), ex.what());
      FAIL(ex.what());
    } catch (...) {
      LOG_ERROR("CPPTEST: PutAll caused random exception in PutAllFourTask");
      cleanProc();
      FAIL("PutAll caused unexpected exception");
    }
    LOG("PutAllFourTask completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyAllPutAllTask)
  {
    LOG("VerifyAllPutAllTask started.");

    auto dataReg = getHelper()->getRegion(regionNames[0]);
    ASSERT(dataReg != nullptr, "Region not found.");
    LOG_INFO("dataregion size is %d: ", dataReg->size());
    LOG_INFO("dataregion getCreates is %d: ", reg1Listener1->getCreates());
    ASSERT(reg1Listener1->getCreates() == 4000,
           "Got wrong number of creation events.");

    ASSERT(dataReg->size() == 4000, "Expected 4000 entries in region");
    char key[256];
    char value[256];
    for (int32_t item = 0; item < 4000; item++) {
      sprintf(key, "key-%d", item);
      sprintf(value, "%d", item);
      LOG_DEBUG("CPPTEST:VerifyAllPutAllTask Doing get on key using: = %s: ",
                key);
      auto checkPtr = std::dynamic_pointer_cast<CacheableString>(
          dataReg->get(CacheableKey::create(key)));
      ASSERT(checkPtr != nullptr, "Value Ptr should not be null.");
      LOG_DEBUG("CPPTEST:VerifyAllPutAllTask value is: = %s: ",
                checkPtr->value().c_str());
      ASSERT(atoi(checkPtr->value().c_str()) == item, "Value did not match.");
    }
    LOG("VerifyAllPutAllTask completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, RemoveAllOneTask)
  {
    LOG("RemoveAllOneTask started.");
    auto dataReg = getHelper()->getRegion(regionNames[0]);
    HashMapOfCacheable entryMap;
    std::vector<std::shared_ptr<CacheableKey>> keysVector;
    entryMap.clear();
    char key[256];
    char value[256];
    for (int32_t item = 0; item < 1000; item++) {
      sprintf(key, "key-%d", item);
      sprintf(value, "%d", item);
      entryMap.emplace(CacheableKey::create(key),
                       CacheableString::create(value));
      keysVector.push_back(CacheableKey::create(key));
      LOG_DEBUG(
          "CPPTEST:RemoveAllOneTask Doing PutAll on key using key: = %s: & "
          "value: = %s",
          key, value);
    }
    try {
      dataReg->putAll(entryMap);
      dataReg->removeAll(keysVector);
    } catch (Exception &ex) {
      LOG_ERROR("CPPTEST: Unexpected %s: %s", ex.getName().c_str(), ex.what());
      FAIL(ex.what());
    } catch (...) {
      LOG_ERROR("CPPTEST: RemoveAll caused random exception in PutAllOneTask");
      cleanProc();
      FAIL("RemoveAll caused unexpected exception");
    }
    LOG("RemoveAllOneTask completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, RemoveAllTwoTask)
  {
    LOG("RemoveAllTwoTask started.");
    auto dataReg = getHelper()->getRegion(regionNames[0]);
    HashMapOfCacheable entryMap;
    std::vector<std::shared_ptr<CacheableKey>> keysVector;
    entryMap.clear();
    char key[256];
    char value[256];
    for (int32_t item = 1000; item < 2000; item++) {
      sprintf(key, "key-%d", item);
      sprintf(value, "%d", item);
      entryMap.emplace(CacheableKey::create(key),
                       CacheableString::create(value));
      keysVector.push_back(CacheableKey::create(key));
      LOG_DEBUG(
          "CPPTEST:RemoveAllTwoTask Doing RemoveAll on key using key: = %s: & "
          "value: = %s",
          key, value);
    }
    try {
      dataReg->putAll(entryMap);
      dataReg->removeAll(keysVector);
    } catch (Exception &ex) {
      LOG_ERROR("CPPTEST: Unexpected %s: %s", ex.getName().c_str(), ex.what());
      FAIL(ex.what());
    } catch (...) {
      LOG_ERROR("CPPTEST: RemoveAll caused random exception in PutAllTwoTask");
      cleanProc();
      FAIL("RemoveAll caused unexpected exception");
    }
    LOG("RemoveAllTwoTask completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, RemoveAllThreeTask)
  {
    LOG("RemoveAllThreeTask started.");
    auto dataReg = getHelper()->getRegion(regionNames[0]);
    HashMapOfCacheable entryMap;
    std::vector<std::shared_ptr<CacheableKey>> keysVector;
    entryMap.clear();
    char key[256];
    char value[256];
    for (int32_t item = 2000; item < 3000; item++) {
      sprintf(key, "key-%d", item);
      sprintf(value, "%d", item);
      entryMap.emplace(CacheableKey::create(key),
                       CacheableString::create(value));
      keysVector.push_back(CacheableKey::create(key));
      LOG_DEBUG(
          "CPPTEST:RemoveAllThreeTask Doing RemoveAll on key using key: = %s: "
          "& "
          "value: = %s",
          key, value);
    }
    try {
      dataReg->putAll(entryMap);
      dataReg->removeAll(keysVector);
    } catch (Exception &ex) {
      LOG_ERROR("CPPTEST: Unexpected %s: %s", ex.getName().c_str(), ex.what());
      FAIL(ex.what());
    } catch (...) {
      LOG_ERROR(
          "CPPTEST: RemoveAll caused random exception in RemoveAllThreeTask");
      cleanProc();
      FAIL("RemoveAll caused unexpected exception");
    }
    LOG("RemoveAllThreeTask completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, RemoveAllFourTask)
  {
    LOG("RemoveAllFourTask started.");
    auto dataReg = getHelper()->getRegion(regionNames[0]);
    HashMapOfCacheable entryMap;
    std::vector<std::shared_ptr<CacheableKey>> keysVector;
    entryMap.clear();
    char key[256];
    char value[256];
    for (int32_t item = 3000; item < 4000; item++) {
      sprintf(key, "key-%d", item);
      sprintf(value, "%d", item);
      entryMap.emplace(CacheableKey::create(key),
                       CacheableString::create(value));
      keysVector.push_back(CacheableKey::create(key));
      LOG_DEBUG(
          "CPPTEST:RemoveAllFourTask Doing RemoveAll on key using key: = %s: & "
          "value: = %s",
          key, value);
    }
    try {
      dataReg->putAll(entryMap);
      dataReg->removeAll(keysVector);
    } catch (Exception &ex) {
      LOG_ERROR("CPPTEST: Unexpected %s: %s", ex.getName().c_str(), ex.what());
      FAIL(ex.what());
    } catch (...) {
      LOG_ERROR(
          "CPPTEST: RemoveAll caused random exception in RemoveAllFourTask");
      cleanProc();
      FAIL("RemoveAll caused unexpected exception");
    }
    LOG("RemoveAllFourTask completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyAllRemoveAllTask)
  {
    LOG("VerifyAllRemoveAllTask started.");

    auto dataReg = getHelper()->getRegion(regionNames[0]);
    ASSERT(dataReg != nullptr, "Region not found.");
    LOG_INFO("dataregion size is %d: ", dataReg->size());
    LOG_INFO("dataregion getDestroys is %d: ", reg1Listener1->getDestroys());
    ASSERT(reg1Listener1->getDestroys() == 4000,
           "Got wrong number of destroy events.");
    ASSERT(dataReg->size() == 0, "Expected 0 entries in region");
    LOG("VerifyAllRemoveAllTask completed.");
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

DUNIT_TASK_DEFINITION(SERVER1, CloseServer2)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CloseServer3)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(3);
      LOG("SERVER3 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CloseServer4)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(4);
      LOG("SERVER4 stopped");
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

DUNIT_TASK_DEFINITION(SERVER1, CloseLocator1)
  {
    // stop locator
    if (isLocator) {
      CacheHelper::closeLocator(1);
      LOG("Locator1 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1_With_Locator_PR)
  {
    // starting servers
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver1_pr_putall.xml", locatorsG);
    }
    if (isLocalServer) {
      CacheHelper::initServer(2, "cacheserver2_pr_putall.xml", locatorsG);
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2_With_Locator_PR)
  {
    // starting servers
    if (isLocalServer) {
      CacheHelper::initServer(3, "cacheserver3_pr_putall.xml", locatorsG);
    }
    if (isLocalServer) {
      CacheHelper::initServer(4, "cacheserver4_pr_putall.xml", locatorsG);
    }
  }
END_TASK_DEFINITION

DUNIT_MAIN
  {
    CALL_TASK(CreateLocator1);

    CALL_TASK(CreateServer1_With_Locator_PR);

    CALL_TASK(StepOne_Pooled_Locator_Client1);
    CALL_TASK(StepOne_Pooled_Locator_Client2);

    CALL_TASK(StepTwo_RegisterAllKeys);

    CALL_TASK(PutAllOneTask);
    CALL_TASK(CreateServer2_With_Locator_PR);

    CALL_TASK(CloseServer1);
    CALL_TASK(CloseServer2);

    CALL_TASK(PutAllTwoTask);
    CALL_TASK(CreateServer1_With_Locator_PR);

    CALL_TASK(CloseServer3);
    CALL_TASK(CloseServer4);

    CALL_TASK(PutAllThreeTask);

    CALL_TASK(CreateServer2_With_Locator_PR);
    CALL_TASK(PutAllFourTask);

    CALL_TASK(VerifyAllPutAllTask);

    CALL_TASK(CloseCache1);
    CALL_TASK(CloseCache2);

    CALL_TASK(CloseServer1);
    CALL_TASK(CloseServer2);
    CALL_TASK(CloseServer3);
    CALL_TASK(CloseServer4);

    CALL_TASK(CloseLocator1);
  }
END_MAIN

DUNIT_MAIN
  {
    CALL_TASK(CreateLocator1);

    CALL_TASK(CreateServer1_With_Locator_PR);

    CALL_TASK(StepOne_Pooled_Locator_Client1);
    CALL_TASK(StepOne_Pooled_Locator_Client2);

    CALL_TASK(StepTwo_RegisterAllKeys);

    CALL_TASK(RemoveAllOneTask);
    CALL_TASK(CreateServer2_With_Locator_PR);

    CALL_TASK(CloseServer1);
    CALL_TASK(CloseServer2);

    CALL_TASK(RemoveAllTwoTask);
    CALL_TASK(CreateServer1_With_Locator_PR);

    CALL_TASK(CloseServer3);
    CALL_TASK(CloseServer4);

    CALL_TASK(RemoveAllThreeTask);

    CALL_TASK(CreateServer2_With_Locator_PR);
    CALL_TASK(RemoveAllFourTask);

    CALL_TASK(VerifyAllRemoveAllTask);

    CALL_TASK(CloseCache1);
    CALL_TASK(CloseCache2);

    CALL_TASK(CloseServer1);
    CALL_TASK(CloseServer2);
    CALL_TASK(CloseServer3);
    CALL_TASK(CloseServer4);

    CALL_TASK(CloseLocator1);
  }

END_MAIN
