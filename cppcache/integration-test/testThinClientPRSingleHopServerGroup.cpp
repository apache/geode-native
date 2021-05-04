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

#define ROOT_NAME "testThinClientPRSingleHopServerGroup"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include <string>

#include <statistics/StatisticsFactory.hpp>

#include "fw_dunit.hpp"
#include "Utils.hpp"

#include "CacheHelper.hpp"

// Include these 2 headers for access to CacheImpl for test hooks.
#include "testUtils.hpp"
#include "ThinClientHelper.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s2p1
#define CLIENT3 s1p2
#define SERVER1 s2p2

using apache::geode::client::CacheServerException;
using apache::geode::client::CacheWriterException;
using apache::geode::client::Exception;
using apache::geode::client::HashMapOfException;

bool isLocalServer = false;

static bool isLocator = false;
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);

const char *group1 = "ABC";
const char *group2 = "BC";
const char *group3 = "C";

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pooled_Locator1)
  {
    initClient(true);
    getHelper()->createPoolWithLocators("__TEST_POOL1__", locatorsG, false, -1,
                                        std::chrono::seconds::zero(), -1, false,
                                        group1);
    getHelper()->createRegionAndAttachPool(regionNames[0], USE_ACK,
                                           "__TEST_POOL1__", false);
    LOG("StepOne_Pooled_Locator1 complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepOne_Pooled_Locator2)
  {
    initClient(true);
    getHelper()->createPoolWithLocators("__TEST_POOL2__", locatorsG, false, -1,
                                        std::chrono::seconds::zero(), -1, false,
                                        group2);
    getHelper()->createRegionAndAttachPool(regionNames[0], USE_ACK,
                                           "__TEST_POOL2__", false);
    LOG("StepOne_Pooled_Locator2 complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT3, StepOne_Pooled_Locator3)
  {
    initClient(true);
    getHelper()->createPoolWithLocators("__TEST_POOL3__", locatorsG, false, -1,
                                        std::chrono::seconds::zero(), -1, false,
                                        group3);
    getHelper()->createRegionAndAttachPool(regionNames[0], USE_ACK,
                                           "__TEST_POOL3__", false);
    LOG("StepOne_Pooled_Locator3 complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CheckPrSingleHopForIntKeysTask_CLIENT1)
  {
    LOG("CheckPrSingleHopForIntKeysTask_CLIENT1 started.");
    int failureCount = 0;

    auto dataReg = getHelper()->getRegion(regionNames[0]);

    for (int i = 0; i < 1000; i++) {
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));

      try {
        dataReg->put(keyPtr, keyPtr->hashcode());
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        if (networkhop) {
          failureCount++;
        }
        int8_t serverGroupFlag = TestUtils::getCacheImpl(getHelper()->cachePtr)
                                     ->getAndResetServerGroupFlag();
        ASSERT(serverGroupFlag != 2,
               "serverGroupFlag should not be equal to 2");

      } catch (CacheServerException &) {
        LOG_ERROR("CPPTEST: Put caused extra hop.");
        FAIL("Put caused extra hop.");
      } catch (CacheWriterException &) {
        LOG_ERROR("CPPTEST: Put caused extra hop.");
        FAIL("Put caused extra hop.");
      } catch (Exception &ex) {
        LOG_ERROR("CPPTEST: Put caused unexpected %s: %s", ex.getName().c_str(),
                  ex.what());
        cleanProc();
        FAIL("Put caused unexpected exception");
      } catch (...) {
        LOG_ERROR("CPPTEST: Put caused random exception");
        cleanProc();
        FAIL("Put caused unexpected exception");
      }
    }
    // relaxed this limit as it takes time
    ASSERT(failureCount < 70, "Count should be less then 70");
    LOG("CheckPrSingleHopForIntKeysTask_CLIENT1 put completed.");

    for (int i = 0; i < 1000; i++) {
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));

      try {
        dataReg->get(keyPtr);
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        ASSERT(!networkhop, "It is networkhop operation.");
        int8_t serverGroupFlag = TestUtils::getCacheImpl(getHelper()->cachePtr)
                                     ->getAndResetServerGroupFlag();
        ASSERT(serverGroupFlag != 2,
               "serverGroupFlag should not be equal to 2");
      } catch (CacheServerException &) {
        LOG_ERROR("CPPTEST: get caused extra hop.");
        FAIL("get caused extra hop.");
      } catch (CacheWriterException &) {
        LOG_ERROR("CPPTEST: get caused extra hop.");
        FAIL("get caused extra hop.");
      } catch (Exception &ex) {
        LOG_ERROR("CPPTEST: get caused unexpected %s: %s", ex.getName().c_str(),
                  ex.what());
        cleanProc();
        FAIL("get caused unexpected exception");
      } catch (...) {
        LOG_ERROR("CPPTEST: get caused random exception");
        cleanProc();
        FAIL("get caused unexpected exception");
      }
    }
    LOG("CheckPrSingleHopForIntKeysTask_CLIENT1 get completed.");

    for (int i = 1000; i < 2000; i++) {
      std::vector<std::shared_ptr<CacheableKey>> keysVector;
      for (int j = i; j < i + 5; j++) {
        keysVector.push_back(CacheableInt32::create(j));
      }

      auto exceptions = std::make_shared<HashMapOfException>();

      try {
        auto values = dataReg->getAll(keysVector);
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        ASSERT(!networkhop, "It is networkhop operation.");
        int8_t serverGroupFlag = TestUtils::getCacheImpl(getHelper()->cachePtr)
                                     ->getAndResetServerGroupFlag();
        ASSERT(serverGroupFlag != 2,
               "serverGroupFlag should not be equal to 2");
      } catch (CacheServerException &) {
        LOG_ERROR("CPPTEST: getAll caused extra hop.");
        FAIL("getAll caused extra hop.");
      } catch (CacheWriterException &) {
        LOG_ERROR("CPPTEST: getAll caused extra hop.");
        FAIL("getAll caused extra hop.");
      } catch (Exception &ex) {
        LOG_ERROR("CPPTEST: getALL caused unexpected %s: %s",
                  ex.getName().c_str(), ex.what());
        cleanProc();
        FAIL("getAll caused unexpected exception");
      } catch (...) {
        LOG_ERROR("CPPTEST: getAll caused random exception");
        cleanProc();
        FAIL("getAll caused unexpected exception");
      }
    }
    LOG("CheckPrSingleHopForIntKeysTask_CLIENT1 getAll completed.");

    for (int i = 0; i < 1000; i++) {
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));

      try {
        dataReg->destroy(keyPtr);
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        ASSERT(!networkhop, "It is networkhop operation.");
        int8_t serverGroupFlag = TestUtils::getCacheImpl(getHelper()->cachePtr)
                                     ->getAndResetServerGroupFlag();
        ASSERT(serverGroupFlag != 2,
               "serverGroupFlag should not be equal to 2");
      } catch (CacheServerException &) {
        LOG_ERROR("CPPTEST: destroy caused extra hop.");
        FAIL("destroy caused extra hop.");
      } catch (CacheWriterException &) {
        LOG_ERROR("CPPTEST: destroy caused extra hop.");
        FAIL("destroy caused extra hop.");
      } catch (Exception &ex) {
        LOG_ERROR("CPPTEST: destroy caused unexpected %s: %s",
                  ex.getName().c_str(), ex.what());
        cleanProc();
        FAIL("destroy caused unexpected exception");
      } catch (...) {
        LOG_ERROR("CPPTEST: destroy caused random exception");
        cleanProc();
        FAIL("destroy caused unexpected exception");
      }
    }
    LOG("CheckPrSingleHopForIntKeysTask_CLIENT1 destroy completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CheckPrSingleHopForIntKeysTask_CLIENT2)
  {
    LOG("CheckPrSingleHopForIntKeysTask_CLIENT2 started.");
    int failureCount = 0;

    auto dataReg = getHelper()->getRegion(regionNames[0]);

    for (int i = 0; i < 1000; i++) {
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));

      try {
        dataReg->put(keyPtr, keyPtr->hashcode());
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        if (networkhop) {
          failureCount++;
        }
        int8_t serverGroupFlag = TestUtils::getCacheImpl(getHelper()->cachePtr)
                                     ->getAndResetServerGroupFlag();
        ASSERT(serverGroupFlag != 2,
               "serverGroupFlag should not be equal to 2");

      } catch (CacheServerException &) {
        LOG_ERROR("CPPTEST: Put caused extra hop.");
        FAIL("Put caused extra hop.");
      } catch (CacheWriterException &) {
        LOG_ERROR("CPPTEST: Put caused extra hop.");
        FAIL("Put caused extra hop.");
      } catch (Exception &ex) {
        LOG_ERROR("CPPTEST: Put caused unexpected %s: %s", ex.getName().c_str(),
                  ex.what());
        cleanProc();
        FAIL("Put caused unexpected exception");
      } catch (...) {
        LOG_ERROR("CPPTEST: Put caused random exception");
        cleanProc();
        FAIL("Put caused unexpected exception");
      }
    }
    ASSERT(failureCount > 0, "Count should be greater than 1");
    LOG("CheckPrSingleHopForIntKeysTask_CLIENT2 put completed.");

    for (int i = 0; i < 1000; i++) {
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));

      try {
        dataReg->get(keyPtr);
        int8_t serverGroupFlag = TestUtils::getCacheImpl(getHelper()->cachePtr)
                                     ->getAndResetServerGroupFlag();
        ASSERT(serverGroupFlag != 2,
               "serverGroupFlag should not be equal to 2");
      } catch (CacheServerException &) {
        LOG_ERROR("CPPTEST: get caused extra hop.");
        FAIL("get caused extra hop.");
      } catch (CacheWriterException &) {
        LOG_ERROR("CPPTEST: get caused extra hop.");
        FAIL("get caused extra hop.");
      } catch (Exception &ex) {
        LOG_ERROR("CPPTEST: get caused unexpected %s: %s", ex.getName().c_str(),
                  ex.what());
        cleanProc();
        FAIL("get caused unexpected exception");
      } catch (...) {
        LOG_ERROR("CPPTEST: get caused random exception");
        cleanProc();
        FAIL("get caused unexpected exception");
      }
    }
    LOG("CheckPrSingleHopForIntKeysTask_CLIENT2 get completed.");

    for (int i = 1000; i < 2000; i++) {
      std::vector<std::shared_ptr<CacheableKey>> keysVector;
      for (int j = i; j < i + 5; j++) {
        keysVector.push_back(CacheableInt32::create(j));
      }

      try {
        const auto values = dataReg->getAll(keysVector);
        int8_t serverGroupFlag = TestUtils::getCacheImpl(getHelper()->cachePtr)
                                     ->getAndResetServerGroupFlag();
        ASSERT(values.size() == 5, "getall size should be 5 ");
        ASSERT(serverGroupFlag != 2,
               "serverGroupFlag should not be equal to 2");
      } catch (CacheServerException &) {
        LOG_ERROR("CPPTEST: getAll caused extra hop.");
        FAIL("getAll caused extra hop.");
      } catch (CacheWriterException &) {
        LOG_ERROR("CPPTEST: getAll caused extra hop.");
        FAIL("getAll caused extra hop.");
      } catch (Exception &ex) {
        LOG_ERROR("CPPTEST: getALL caused unexpected %s: %s",
                  ex.getName().c_str(), ex.what());
        cleanProc();
        FAIL("getAll caused unexpected exception");
      } catch (...) {
        LOG_ERROR("CPPTEST: getAll caused random exception");
        cleanProc();
        FAIL("getAll caused unexpected exception");
      }
    }
    LOG("CheckPrSingleHopForIntKeysTask_CLIENT2 getAll completed.");

    for (int i = 0; i < 1000; i++) {
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));

      try {
        dataReg->destroy(keyPtr);
        int8_t serverGroupFlag = TestUtils::getCacheImpl(getHelper()->cachePtr)
                                     ->getAndResetServerGroupFlag();
        ASSERT(serverGroupFlag != 2,
               "serverGroupFlag should not be equal to 2");
      } catch (CacheServerException &) {
        LOG_ERROR("CPPTEST: destroy caused extra hop.");
        FAIL("destroy caused extra hop.");
      } catch (CacheWriterException &) {
        LOG_ERROR("CPPTEST: destroy caused extra hop.");
        FAIL("destroy caused extra hop.");
      } catch (Exception &ex) {
        LOG_ERROR("CPPTEST: destroy caused unexpected %s: %s",
                  ex.getName().c_str(), ex.what());
        cleanProc();
        FAIL("destroy caused unexpected exception");
      } catch (...) {
        LOG_ERROR("CPPTEST: destroy caused random exception");
        cleanProc();
        FAIL("destroy caused unexpected exception");
      }
    }
    LOG("CheckPrSingleHopForIntKeysTask_CLIENT2 destroy completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT3, CheckPrSingleHopForIntKeysTask_CLIENT3)
  {
    LOG("CheckPrSingleHopForIntKeysTask_CLIENT3 started.");
    int failureCount = 0;

    auto dataReg = getHelper()->getRegion(regionNames[0]);

    for (int i = 0; i < 1000; i++) {
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));

      try {
        dataReg->put(keyPtr, keyPtr->hashcode());
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        if (networkhop) {
          failureCount++;
        }
        int8_t serverGroupFlag = TestUtils::getCacheImpl(getHelper()->cachePtr)
                                     ->getAndResetServerGroupFlag();
        ASSERT(serverGroupFlag != 2,
               "serverGroupFlag should not be equal to 2");

      } catch (CacheServerException &) {
        LOG_ERROR("CPPTEST: Put caused extra hop.");
        FAIL("Put caused extra hop.");
      } catch (CacheWriterException &) {
        LOG_ERROR("CPPTEST: Put caused extra hop.");
        FAIL("Put caused extra hop.");
      } catch (Exception &ex) {
        LOG_ERROR("CPPTEST: Put caused unexpected %s: %s", ex.getName().c_str(),
                  ex.what());
        cleanProc();
        FAIL("Put caused unexpected exception");
      } catch (...) {
        LOG_ERROR("CPPTEST: Put caused random exception");
        cleanProc();
        FAIL("Put caused unexpected exception");
      }
    }
    ASSERT(failureCount > 0, "Count should be greater than 1");
    LOG("CheckPrSingleHopForIntKeysTask_CLIENT3 put completed.");

    for (int i = 0; i < 1000; i++) {
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));

      try {
        dataReg->get(keyPtr);
        int8_t serverGroupFlag = TestUtils::getCacheImpl(getHelper()->cachePtr)
                                     ->getAndResetServerGroupFlag();
        ASSERT(serverGroupFlag != 2,
               "serverGroupFlag should not be equal to 2");
      } catch (CacheServerException &) {
        LOG_ERROR("CPPTEST: get caused extra hop.");
        FAIL("get caused extra hop.");
      } catch (CacheWriterException &) {
        LOG_ERROR("CPPTEST: get caused extra hop.");
        FAIL("get caused extra hop.");
      } catch (Exception &ex) {
        LOG_ERROR("CPPTEST: get caused unexpected %s: %s", ex.getName().c_str(),
                  ex.what());
        cleanProc();
        FAIL("get caused unexpected exception");
      } catch (...) {
        LOG_ERROR("CPPTEST: get caused random exception");
        cleanProc();
        FAIL("get caused unexpected exception");
      }
    }
    LOG("CheckPrSingleHopForIntKeysTask_CLIENT3 get completed.");

    for (int i = 1000; i < 2000; i++) {
      std::vector<std::shared_ptr<CacheableKey>> keysVector;
      for (int j = i; j < i + 5; j++) {
        keysVector.push_back(CacheableInt32::create(j));
      }

      try {
        dataReg->getAll(keysVector);
        int8_t serverGroupFlag = TestUtils::getCacheImpl(getHelper()->cachePtr)
                                     ->getAndResetServerGroupFlag();
        ASSERT(serverGroupFlag != 2,
               "serverGroupFlag should not be equal to 2");
      } catch (CacheServerException &) {
        LOG_ERROR("CPPTEST: getAll caused extra hop.");
        FAIL("getAll caused extra hop.");
      } catch (CacheWriterException &) {
        LOG_ERROR("CPPTEST: getAll caused extra hop.");
        FAIL("getAll caused extra hop.");
      } catch (Exception &ex) {
        LOG_ERROR("CPPTEST: getALL caused unexpected %s: %s",
                  ex.getName().c_str(), ex.what());
        cleanProc();
        FAIL("getAll caused unexpected exception");
      } catch (...) {
        LOG_ERROR("CPPTEST: getAll caused random exception");
        cleanProc();
        FAIL("getAll caused unexpected exception");
      }
    }
    LOG("CheckPrSingleHopForIntKeysTask_CLIENT3 getAll completed.");

    for (int i = 0; i < 1000; i++) {
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));

      try {
        dataReg->destroy(keyPtr);
        int8_t serverGroupFlag = TestUtils::getCacheImpl(getHelper()->cachePtr)
                                     ->getAndResetServerGroupFlag();
        ASSERT(serverGroupFlag != 2,
               "serverGroupFlag should not be equal to 2");
      } catch (CacheServerException &) {
        LOG_ERROR("CPPTEST: destroy caused extra hop.");
        FAIL("destroy caused extra hop.");
      } catch (CacheWriterException &) {
        LOG_ERROR("CPPTEST: destroy caused extra hop.");
        FAIL("destroy caused extra hop.");
      } catch (Exception &ex) {
        LOG_ERROR("CPPTEST: destroy caused unexpected %s: %s",
                  ex.getName().c_str(), ex.what());
        cleanProc();
        FAIL("destroy caused unexpected exception");
      } catch (...) {
        LOG_ERROR("CPPTEST: destroy caused random exception");
        cleanProc();
        FAIL("destroy caused unexpected exception");
      }
    }
    LOG("CheckPrSingleHopForIntKeysTask_CLIENT3 destroy completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CloseCache2)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT3, CloseCache3)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServers_With_Locator_PR)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }

    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
    }

    if (isLocalServer) {
      CacheHelper::closeServer(3);
      LOG("SERVER3 stopped");
    }

    if (isLocator) {
      CacheHelper::closeLocator(1);
      LOG("Locator1 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServers_With_Locator_PR)
  {
    // starting locator
    if (isLocator) CacheHelper::initLocator(1);
    LOG("Locator1 started");

    // starting servers
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver1_partitioned_servergroup.xml",
                              locatorsG);
    }

    // starting servers
    if (isLocalServer) {
      CacheHelper::initServer(2, "cacheserver2_partitioned_servergroup.xml",
                              locatorsG);
    }

    // starting servers
    if (isLocalServer) {
      CacheHelper::initServer(3, "cacheserver3_partitioned_servergroup.xml",
                              locatorsG);
    }
  }
END_TASK_DEFINITION

DUNIT_MAIN
  {
    // server-group applicable only with locators, so no test case with pool
    // endpoints.

    CALL_TASK(CreateServers_With_Locator_PR);

    CALL_TASK(StepOne_Pooled_Locator1);
    CALL_TASK(StepOne_Pooled_Locator2);
    CALL_TASK(StepOne_Pooled_Locator3);

    CALL_TASK(CheckPrSingleHopForIntKeysTask_CLIENT1);
    CALL_TASK(CheckPrSingleHopForIntKeysTask_CLIENT2);
    CALL_TASK(CheckPrSingleHopForIntKeysTask_CLIENT3);

    CALL_TASK(CloseCache1);
    CALL_TASK(CloseCache2);
    CALL_TASK(CloseCache3);

    CALL_TASK(CloseServers_With_Locator_PR);
  }
END_MAIN
