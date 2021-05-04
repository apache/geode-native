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
#include "BuiltinCacheableWrappers.hpp"
#include "Utils.hpp"
#include <geode/FixedPartitionResolver.hpp>
#include <ace/OS.h>
#include <ace/High_Res_Timer.h>

#include <string>

#include "CacheHelper.hpp"

// Include these 2 headers for access to CacheImpl for test hooks.
#include "CacheImplHelper.hpp"
#include "testUtils.hpp"

#include "ThinClientHelper.hpp"

using apache::geode::client::CacheServerException;
using apache::geode::client::CacheWriterException;
using apache::geode::client::EntryEvent;
using apache::geode::client::Exception;
using apache::geode::client::FixedPartitionResolver;

const char *partitionRegionNames[] = {"R1", "R2", "R3"};
const char *partitionRegionName;

class CustomFixedPartitionResolver1 : public FixedPartitionResolver {
 public:
  CustomFixedPartitionResolver1() {}
  ~CustomFixedPartitionResolver1() override {}
  const std::string &getName() override {
    static std::string name = "CustomFixedPartitionResolver1";
    LOG("CustomFixedPartitionResolver1::getName()");
    return name;
  }

  std::shared_ptr<CacheableKey> getRoutingObject(
      const EntryEvent &opDetails) override {
    LOG("CustomFixedPartitionResolver1::getRoutingObject()");
    int32_t key = atoi(opDetails.getKey()->toString().c_str());
    int32_t newKey = key + 5;
    return CacheableKey::create(newKey);
  }

  const std::string &getPartitionName(const EntryEvent &opDetails) override {
    LOG("CustomFixedPartitionResolver1::getPartitionName()");
    int32_t key = atoi(opDetails.getKey()->toString().c_str());
    int32_t newkey = key % 6;
    if (newkey == 0) {
      static std::string P1 = "P1";
      return P1;
    } else if (newkey == 1) {
      static std::string P2 = "P2";
      return P2;
    } else if (newkey == 2) {
      static std::string P3 = "P3";
      return P3;
    } else if (newkey == 3) {
      static std::string P4 = "P4";
      return P4;
    } else if (newkey == 4) {
      static std::string P5 = "P5";
      return P5;
    } else if (newkey == 5) {
      static std::string P6 = "P6";
      return P6;
    } else {
      static std::string Invalid = "Invalid";
      return Invalid;
    }
  }
};
auto cptr1 = std::make_shared<CustomFixedPartitionResolver1>();

class CustomFixedPartitionResolver2 : public FixedPartitionResolver {
 public:
  CustomFixedPartitionResolver2() {}
  ~CustomFixedPartitionResolver2() override {}
  const std::string &getName() override {
    static std::string name = "CustomFixedPartitionResolver2";
    LOG("CustomFixedPartitionResolver2::getName()");
    return name;
  }

  std::shared_ptr<CacheableKey> getRoutingObject(
      const EntryEvent &opDetails) override {
    LOG("CustomFixedPartitionResolver2::getRoutingObject()");
    int32_t key = atoi(opDetails.getKey()->toString().c_str());
    int32_t newKey = key + 4;
    return CacheableKey::create(newKey /*key*/);
  }

  const std::string &getPartitionName(const EntryEvent &opDetails) override {
    LOG("CustomFixedPartitionResolver2::getPartitionName()");
    int32_t key = atoi(opDetails.getKey()->toString().c_str());
    int32_t newkey = key % 6;
    if (newkey == 0) {
      static std::string P1 = "P1";
      return P1;
    } else if (newkey == 1) {
      static std::string P2 = "P2";
      return P2;
    } else if (newkey == 2) {
      static std::string P3 = "P3";
      return P3;
    } else if (newkey == 3) {
      static std::string P4 = "P4";
      return P4;
    } else if (newkey == 4) {
      static std::string P5 = "P5";
      return P5;
    } else if (newkey == 5) {
      static std::string P6 = "P6";
      return P6;
    } else {
      static std::string Invalid = "Invalid";
      return Invalid;
    }
  }
};
auto cptr2 = std::make_shared<CustomFixedPartitionResolver2>();

class CustomFixedPartitionResolver3 : public FixedPartitionResolver {
 public:
  CustomFixedPartitionResolver3() {}
  ~CustomFixedPartitionResolver3() override {}
  const std::string &getName() override {
    static std::string name = "CustomFixedPartitionResolver3";
    LOG("CustomFixedPartitionResolver3::getName()");
    return name;
  }

  std::shared_ptr<CacheableKey> getRoutingObject(
      const EntryEvent &opDetails) override {
    LOG("CustomFixedPartitionResolver3::getRoutingObject()");
    int32_t key = atoi(opDetails.getKey()->toString().c_str());
    int32_t newKey = key % 5;
    return CacheableKey::create(newKey /*key*/);
  }

  const std::string &getPartitionName(const EntryEvent &opDetails) override {
    LOG("CustomFixedPartitionResolver3::getPartitionName()");
    int32_t key = atoi(opDetails.getKey()->toString().c_str());
    int32_t newkey = key % 3;
    if (newkey == 0) {
      static std::string P1 = "P1";
      return P1;
    } else if (newkey == 1) {
      static std::string P2 = "P2";
      return P2;
    } else if (newkey == 2) {
      static std::string P3 = "P3";
      return P3;
    } else {
      static std::string Invalid = "Invalid";
      return Invalid;
    }
  }
};
auto cptr3 = std::make_shared<CustomFixedPartitionResolver3>();

#define CLIENT1 s1p1
#define SERVER1 s2p1
#define SERVER2 s1p2
#define SERVER3 s2p2

bool isLocalServer = false;

static bool isLocator = false;
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);

std::vector<char *> storeEndPoints(const char *points) {
  std::vector<char *> endpointNames;
  if (points != nullptr) {
    char *ep = strdup(points);
    char *token = strtok(ep, ",");
    while (token) {
      endpointNames.push_back(token);
      token = strtok(nullptr, ",");
    }
    free(ep);
  }
  ASSERT(endpointNames.size() == 3, "There should be 3 end points");
  return endpointNames;
}

DUNIT_TASK_DEFINITION(CLIENT1, SetRegion1)
  { partitionRegionName = partitionRegionNames[0]; }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, SetRegion2)
  { partitionRegionName = partitionRegionNames[1]; }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, SetRegion3)
  { partitionRegionName = partitionRegionNames[2]; }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    if (isLocalServer) CacheHelper::initServer(1, "cacheserver1_fpr.xml");
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2)
  {
    if (isLocalServer) CacheHelper::initServer(2, "cacheserver2_fpr.xml");
    LOG("SERVER2 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER3, CreateServer3)
  {
    if (isLocalServer) CacheHelper::initServer(3, "cacheserver3_fpr.xml");
    LOG("SERVER3 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pooled_Locator)
  {
    initClient(true);

    getHelper()->createPoolWithLocators("__TEST_POOL1__", locatorsG);
    getHelper()->createRegionAndAttachPool2(partitionRegionNames[0], USE_ACK,
                                            "__TEST_POOL1__", cptr1);
    getHelper()->createRegionAndAttachPool2(partitionRegionNames[1], USE_ACK,
                                            "__TEST_POOL1__", cptr2);
    getHelper()->createRegionAndAttachPool2(partitionRegionNames[2], USE_ACK,
                                            "__TEST_POOL1__", cptr3);

    LOG("StepOne_Pooled_Locator complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CheckPrSingleHopForIntKeysTask_REGION)
  {
    LOG("CheckPrSingleHopForIntKeysTask_REGION started.");
    int failureCount = 0;

    LOG_DEBUG("CheckPrSingleHopForIntKeysTask_REGION create region  = %s ",
              partitionRegionName);
    auto dataReg = getHelper()->getRegion(partitionRegionName);

    for (int i = 0; i < 3000; i++) {
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));

      try {
        LOG_DEBUG("CPPTEST: Putting key %d with hashcode %d", i,
                  keyPtr->hashcode());
        dataReg->put(keyPtr, keyPtr->hashcode());
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        LOG_DEBUG("CheckPrSingleHopForIntKeysTask_REGION: networkhop %d ",
                  networkhop);
        if (networkhop) {
          failureCount++;
        }
        int8_t serverGroupFlag = TestUtils::getCacheImpl(getHelper()->cachePtr)
                                     ->getAndResetServerGroupFlag();
        LOG_DEBUG(
            "CheckPrSingleHopForIntKeysTask_REGION: serverGroupFlag is %d "
            "failureCount = %d",
            serverGroupFlag, failureCount);
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
    ASSERT(failureCount < 70, "Count should be less than 70");
    LOG("CheckPrSingleHopForIntKeysTask_REGION put completed.");

    for (int i = 0; i < 1000; i++) {
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));

      try {
        LOG_DEBUG("CPPTEST: getting key %d with hashcode %d", i,
                  keyPtr->hashcode());
        dataReg->get(keyPtr);
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        LOG_DEBUG("CheckPrSingleHopForIntKeysTask_REGION: networkhop %d ",
                  networkhop);
        ASSERT(!networkhop, "It is networkhop operation.");
        int8_t serverGroupFlag = TestUtils::getCacheImpl(getHelper()->cachePtr)
                                     ->getAndResetServerGroupFlag();
        LOG_DEBUG(
            "CheckPrSingleHopForIntKeysTask_REGION: serverGroupFlag is %d ",
            serverGroupFlag);
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
    LOG("CheckPrSingleHopForIntKeysTask_REGION get completed.");

    for (int i = 1000; i < 2000; i++) {
      std::vector<std::shared_ptr<CacheableKey>> keysVector;
      for (int j = i; j < i + 5; j++) {
        keysVector.push_back(CacheableInt32::create(j));
      }

      try {
        const auto values = dataReg->getAll(keysVector);
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        ASSERT(values.size() == 5, "number of value size should be 5");
        LOG_DEBUG("CheckPrSingleHopForIntKeysTask_REGION: networkhop %d ",
                  networkhop);
        ASSERT(!networkhop, "It is networkhop operation.");
        int8_t serverGroupFlag = TestUtils::getCacheImpl(getHelper()->cachePtr)
                                     ->getAndResetServerGroupFlag();
        LOG_DEBUG(
            "CheckPrSingleHopForIntKeysTask_REGION: serverGroupFlag is %d ",
            serverGroupFlag);
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

      try {
        const auto values =
            dataReg->getAll(keysVector, CacheableInt32::create(1000));
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        ASSERT(values.size() == 5, "number of value size should be 5");
        LOG_DEBUG("CheckPrSingleHopForIntKeysTask_REGION: networkhop %d ",
                  networkhop);
        ASSERT(!networkhop, "It is networkhop operation.");
        int8_t serverGroupFlag = TestUtils::getCacheImpl(getHelper()->cachePtr)
                                     ->getAndResetServerGroupFlag();
        LOG_DEBUG(
            "CheckPrSingleHopForIntKeysTask_REGION: serverGroupFlag is %d ",
            serverGroupFlag);
        ASSERT(serverGroupFlag != 2,
               "serverGroupFlag should not be equal to 2");
      } catch (CacheServerException &) {
        LOG_ERROR("CPPTEST: getAllwithCallBackArg caused extra hop.");
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
    LOG("CheckPrSingleHopForIntKeysTask_REGION getAll completed.");

    for (int i = 0; i < 1000; i++) {
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));

      try {
        LOG_DEBUG("CPPTEST: destroying key %d with hashcode %d", i,
                  keyPtr->hashcode());
        dataReg->destroy(keyPtr);
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        LOG_DEBUG("CheckPrSingleHopForIntKeysTask_REGION: networkhop %d ",
                  networkhop);
        ASSERT(!networkhop, "It is networkhop operation.");
        int8_t serverGroupFlag = TestUtils::getCacheImpl(getHelper()->cachePtr)
                                     ->getAndResetServerGroupFlag();
        LOG_DEBUG(
            "CheckPrSingleHopForIntKeysTask_REGION: serverGroupFlag is %d ",
            serverGroupFlag);
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
    LOG("CheckPrSingleHopForIntKeysTask_REGION destroy completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
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

DUNIT_TASK_DEFINITION(SERVER3, CloseServer3)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(3);
      LOG("SERVER3 stopped");
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
      CacheHelper::initServer(1, "cacheserver1_fpr.xml", locatorsG);
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2_With_Locator_PR)
  {
    // starting servers
    if (isLocalServer) {
      CacheHelper::initServer(2, "cacheserver2_fpr.xml", locatorsG);
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER3, CreateServer3_With_Locator_PR)
  {
    // starting servers
    if (isLocalServer) {
      CacheHelper::initServer(3, "cacheserver3_fpr.xml", locatorsG);
    }
  }
END_TASK_DEFINITION

DUNIT_MAIN
  {
    CALL_TASK(CreateLocator1);

    CALL_TASK(CreateServer1_With_Locator_PR);
    CALL_TASK(CreateServer2_With_Locator_PR);
    CALL_TASK(CreateServer3_With_Locator_PR);

    CALL_TASK(StepOne_Pooled_Locator);

    CALL_TASK(SetRegion1);
    CALL_TASK(CheckPrSingleHopForIntKeysTask_REGION);

    CALL_TASK(SetRegion2);
    CALL_TASK(CheckPrSingleHopForIntKeysTask_REGION);

    CALL_TASK(SetRegion3);
    CALL_TASK(CheckPrSingleHopForIntKeysTask_REGION);

    CALL_TASK(CloseCache1);

    CALL_TASK(CloseServer1);
    CALL_TASK(CloseServer2);
    CALL_TASK(CloseServer3);

    CALL_TASK(CloseLocator1);
  }
END_MAIN
