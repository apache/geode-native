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

#define ROOT_NAME "testThinClientPRSingleHop"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include <string>
#include <random>
#include <limits>

#include <ace/OS.h>
#include <ace/High_Res_Timer.h>
#include <ace/ACE.h>

#include <statistics/StatisticsFactory.hpp>

#include "fw_dunit.hpp"
#include "BuiltinCacheableWrappers.hpp"
#include "Utils.hpp"

#include "CacheHelper.hpp"

// Include these 2 headers for access to CacheImpl for test hooks.
#include "CacheImplHelper.hpp"
#include "testUtils.hpp"

#include "ThinClientHelper.hpp"

#define CLIENT1 s1p1
#define SERVER1 s2p1
#define SERVER2 s1p2
#define SERVER3 s2p2

using apache::geode::client::CacheServerException;
using apache::geode::client::CacheWriterException;
using apache::geode::client::Exception;
using apache::geode::client::internal::DSCode;

using apache::geode::client::testing::CacheableWrapper;
using apache::geode::client::testing::CacheableWrapperFactory;

bool isLocalServer = false;
const std::string endPoints = CacheHelper::getTcrEndpoints(isLocalServer, 3);

static bool isLocator = false;
const char *locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);

std::string convertHostToCanonicalForm(const char *endpoints) {
  if (endpoints == nullptr) return nullptr;
  std::string hostString("");
  uint16_t port = 0;
  std::string endpointsStr(endpoints);
  std::string endpointsStr1(endpoints);
  // Parse this string to get all hostnames and port numbers.
  std::string endpoint;
  std::string::size_type length = endpointsStr.size();
  std::string::size_type pos = 0;
  ACE_TCHAR hostName[256], fullName[256];
  pos = endpointsStr.find(':', 0);
  if (pos != std::string::npos) {
    endpoint = endpointsStr.substr(0, pos);
    pos += 1;  // skip ':'
    length -= (pos);
    endpointsStr = endpointsStr.substr(pos, length);
  } else {
    hostString = "";
    return "";
  }
  hostString = endpoint;
  port = atoi(endpointsStr.c_str());
  if (strcmp(hostString.c_str(), "localhost") == 0) {
    ACE_OS::hostname(hostName, sizeof(hostName) - 1);
    struct hostent *host;
    host = ACE_OS::gethostbyname(hostName);
    ACE_OS::snprintf(fullName, 256, "%s:%d", host->h_name, port);
    return fullName;
  }
  pos = endpointsStr1.find('.', 0);
  if (pos != std::string::npos) {
    ACE_INET_Addr addr(endpoints);
    addr.get_host_name(hostName, 256);
    ACE_OS::snprintf(fullName, 256, "%s:%d", hostName, port);
    return fullName;
  }
  return endpoints;
}

template <typename T>
T randomValue(T maxValue) {
  static thread_local std::default_random_engine generator(
      std::random_device{}());
  return std::uniform_int_distribution<T>{0, maxValue}(generator);
}

class putThread : public ACE_Task_Base {
 private:
  std::shared_ptr<Region> regPtr;
  int m_min;
  int m_max;
  bool m_isWarmUpTask;
  int m_failureCount;

 public:
  putThread(const char *name, int min, int max, bool isWarmUpTask)
      : regPtr(getHelper()->getRegion(name)),
        m_min(min),
        m_max(max),
        m_isWarmUpTask(isWarmUpTask),
        m_failureCount(0) {}

  int getFailureCount() { return m_failureCount; }

  int svc(void) override {
    std::shared_ptr<CacheableKey> keyPtr;
    for (int i = m_min; i < m_max; i++) {
      if (!m_isWarmUpTask) {
        auto rand = randomValue(std::numeric_limits<int32_t>::max());
        keyPtr = std::dynamic_pointer_cast<CacheableKey>(
            CacheableInt32::create(rand));
        LOGDEBUG("svc: putting key %d  ", rand);
      } else {
        keyPtr =
            std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));
        LOGDEBUG("svc: putting key %d  ", i);
      }
      try {
        regPtr->put(keyPtr, static_cast<int>(keyPtr->hashcode()));
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        if (networkhop) {
          m_failureCount++;
        }
      } catch (const Exception &excp) {
        LOGINFO("Exception occured in put %s: %s ", excp.getName().c_str(),
                excp.what());
      } catch (...) {
        LOG("Random Exception occured");
      }

      if (!m_isWarmUpTask) {
        try {
          regPtr->get(keyPtr);
          bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                                ->getAndResetNetworkHopFlag();
          if (networkhop) {
            m_failureCount++;
          }
        } catch (const Exception &excp) {
          LOGINFO("Exception occured in get %s: %s ", excp.getName().c_str(),
                  excp.what());
        } catch (...) {
          LOG("Random Exception occured");
        }

        try {
          regPtr->destroy(keyPtr);
          bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                                ->getAndResetNetworkHopFlag();
          if (networkhop) {
            m_failureCount++;
          }
        } catch (const Exception &excp) {
          LOGINFO("Exception occured in destroy %s: %s ",
                  excp.getName().c_str(), excp.what());
        } catch (...) {
          LOG("Random Exception occured");
        }
      }
    }
    LOG("releaseThreadLocalConnection PutThread");
    auto pool =
        getHelper()->getCache()->getPoolManager().find("__TEST_POOL1__");
    pool->releaseThreadLocalConnection();
    LOG("releaseThreadLocalConnection PutThread done");
    return 0;
  }
  void start() { activate(); }
  void stop() { wait(); }
};

#if defined(WIN32)
// because we run out of memory on our pune windows desktops
#define DEFAULTNUMKEYS 5
#else
#define DEFAULTNUMKEYS 15
#endif
#define KEYSIZE 256

std::vector<std::string> storeEndPoints(const std::string points) {
  std::vector<std::string> endpointNames;
  size_t end = 0;
  size_t start;
  std::string delim = ",";
  while ((start = points.find_first_not_of(delim, end)) != std::string::npos) {
    end = points.find(delim, start);
    if (end == std::string::npos) {
      end = points.length();
    }
    endpointNames.push_back(points.substr(start, end - start));
  }
  ASSERT(endpointNames.size() == 3, "There should be 3 end points");
  return endpointNames;
}

std::vector<std::string> endpointNames = storeEndPoints(endPoints);

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver1_partitioned.xml");
    }
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2)
  {
    if (isLocalServer) {
      CacheHelper::initServer(2, "cacheserver2_partitioned.xml");
    }
    LOG("SERVER2 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER3, CreateServer3)
  {
    if (isLocalServer) {
      CacheHelper::initServer(3, "cacheserver3_partitioned.xml");
    }
    LOG("SERVER3 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1_R1)
  {
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver1_partitioned_R1.xml");
    }
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2_R1)
  {
    if (isLocalServer) {
      CacheHelper::initServer(2, "cacheserver2_partitioned_R1.xml");
    }
    LOG("SERVER2 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pooled_Locator)
  {
    initClient(true);

    getHelper()->createPoolWithLocators("__TEST_POOL1__", locatorsG);
    getHelper()->createRegionAndAttachPool(regionNames[0], USE_ACK,
                                           "__TEST_POOL1__", false);
    getHelper()->createRegionAndAttachPool(regionNames[1], NO_ACK,
                                           "__TEST_POOL1__", false);

    LOG("StepOne_Pooled_Locator complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pooled_LocatorTL)
  {
    initClient(true);

    auto regPtr = getHelper()->createPooledRegionStickySingleHop(
        regionNames[0], USE_ACK, locatorsG, "__TEST_POOL1__", false, false);
    ASSERT(regPtr != nullptr, "Failed to create region.");
    regPtr = getHelper()->createPooledRegionStickySingleHop(
        regionNames[1], NO_ACK, locatorsG, "__TEST_POOL1__", false, false);
    ASSERT(regPtr != nullptr, "Failed to create region.");

    LOG("StepOne_Pooled_LocatorTL complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, WarmUpTask)
  {
    LOG("WarmUpTask started.");
    int failureCount = 0;
    auto dataReg = getHelper()->getRegion(regionNames[0]);

    // This is to get MetaDataService going.
    for (int i = 0; i < 2000; i++) {
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));
      try {
        LOGDEBUG("CPPTEST: put item %d", i);
        dataReg->put(keyPtr, keyPtr->hashcode());
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        LOGINFO("WarmUpTask: networkhop is %d ", networkhop);
        if (networkhop) {
          failureCount++;
        }
        LOGINFO("CPPTEST: put success ");
      } catch (CacheServerException &) {
        // This is actually a success situation!
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: Put caused extra hop.");
        FAIL("Put caused extra hop.");
        //}
        // LOGINFO("CPPTEST: SINGLEHOP SUCCEEDED while putting key %s with
        // hashcode %d", logmsg, (int32_t)keyPtr->hashcode());
      } catch (CacheWriterException &) {
        // This is actually a success situation! Once bug #521 is fixed.
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: Put caused extra hop.");
        FAIL("Put caused extra hop.");
        //}
        // LOGINFO("CPPTEST: SINGLEHOP SUCCEEDED while putting key %s with
        // hashcode %d", logmsg, (int32_t)keyPtr->hashcode());
      } catch (Exception &ex) {
        LOGERROR("CPPTEST: Unexpected %s: %s", ex.getName().c_str(), ex.what());
        FAIL(ex.what());
      } catch (...) {
        LOGERROR("CPPTEST: Put caused random exception in WarmUpTask");
        cleanProc();
        FAIL("Put caused unexpected exception");
      }
    }
    // it takes time to fetch prmetadata so relaxing this limit
    ASSERT(failureCount < 100, "Count should be less than 100");
    int poolconn = TestUtils::getCacheImpl(getHelper()->cachePtr)
                       ->getPoolSize("__TEST_POOL1__");
    LOGINFO("poolconn = %d and endpoints size = %d ", poolconn,
            endpointNames.size());

    // SLEEP(20000);

    LOG("WarmUpTask completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, WarmUpTask3)
  {
    LOG("WarmUpTask3 started.");
    int failureCount = 0;
    auto dataReg = getHelper()->getRegion(regionNames[0]);

    // This is to get MetaDataService going.
    for (int i = 0; i < 2000; i++) {
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));
      try {
        LOGDEBUG("CPPTEST: put item %d", i);
        dataReg->put(keyPtr, keyPtr->hashcode());
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        LOGINFO("WarmUpTask3: networkhop is %d ", networkhop);
        if (networkhop) {
          failureCount++;
        }
        LOGINFO("CPPTEST: put success ");
      } catch (CacheServerException &) {
        // This is actually a success situation!
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: Put caused extra hop.");
        FAIL("Put caused extra hop.");
        //}
        // LOGINFO("CPPTEST: SINGLEHOP SUCCEEDED while putting key %s with
        // hashcode %d", logmsg, (int32_t)keyPtr->hashcode());
      } catch (CacheWriterException &) {
        // This is actually a success situation! Once bug #521 is fixed.
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: Put caused extra hop.");
        FAIL("Put caused extra hop.");
        //}
        // LOGINFO("CPPTEST: SINGLEHOP SUCCEEDED while putting key %s with
        // hashcode %d", logmsg, (int32_t)keyPtr->hashcode());
      } catch (Exception &ex) {
        LOGERROR("CPPTEST: Unexpected %s: %s", ex.getName().c_str(), ex.what());
        FAIL(ex.what());
      } catch (...) {
        LOGERROR("CPPTEST: Put caused random exception in WarmUpTask");
        cleanProc();
        FAIL("Put caused unexpected exception");
      }
    }
    // it takes time to fetch prmetadata so relaxing this limit
    int expectedFailCount = 2000 / 3;
    ASSERT(failureCount < expectedFailCount,
           "Count should be less than expectedFailCount");
    int poolconn = TestUtils::getCacheImpl(getHelper()->cachePtr)
                       ->getPoolSize("__TEST_POOL1__");
    LOGINFO("poolconn = %d and endpoints size = %d ", poolconn,
            endpointNames.size());

    LOG("WarmUpTask3 completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, ThreadedWarmUpTask)
  {
    LOG("ThreadedWarmUpTask started.");
    bool isWarmUpTask = true;
    putThread *threads[1];

    for (int thdIdx = 0; thdIdx < 1; thdIdx++) {
      threads[thdIdx] = new putThread(regionNames[0], 0, 200, isWarmUpTask);
      threads[thdIdx]->start();
    }
    // SLEEP(10000); // wait for threads to become active

    for (int thdIdx = 0; thdIdx < 1; thdIdx++) {
      threads[thdIdx]->stop();
    }
    // SLEEP(20000);
    int totalFailureCount = 0;
    for (int thdIdx = 0; thdIdx < 1; thdIdx++) {
      totalFailureCount += threads[thdIdx]->getFailureCount();
    }
    LOGINFO("ThreadedWarmUpTask totalFailureCount is %d.", totalFailureCount);
    // relaxing this limit as it takes time
    ASSERT(totalFailureCount < 100,
           "totalFailureCount should be less than 100");

    LOG("ThreadedWarmUpTask completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CheckPrSingleHopForAllKeysTask)
  {
    LOG("CheckPrSingleHopForAllKeysTask started.");
    static size_t taskIndexPut = 0;

    auto keyTypes = CacheableWrapperFactory::getRegisteredKeyTypes();
    auto valueTypes = CacheableWrapperFactory::getRegisteredValueTypes();

    size_t keyTypeIndex = taskIndexPut / valueTypes.size();
    size_t valueTypeIndex = taskIndexPut % valueTypes.size();

    DSCode keyTypeId = keyTypes[keyTypeIndex];
    DSCode valTypeId = valueTypes[valueTypeIndex];

    LOGDEBUG(
        "CheckPrSingleHopForAllKeysTask::keyType = %s and valType = %s and "
        "taskIndexPut = %d\n",
        CacheableWrapperFactory::getTypeForId(keyTypeId).c_str(),
        CacheableWrapperFactory::getTypeForId(valTypeId).c_str(), taskIndexPut);

    CacheableWrapper *key = CacheableWrapperFactory::createInstance(keyTypeId);
    int maxKeys =
        (key->maxKeys() < DEFAULTNUMKEYS ? key->maxKeys() : DEFAULTNUMKEYS);
    delete key;

    auto dataReg = getHelper()->getRegion(regionNames[0]);
    auto verifyReg = getHelper()->getRegion(regionNames[1]);
    for (int i = 0; i < maxKeys; i++) {
      CacheableWrapper *tmpkey =
          CacheableWrapperFactory::createInstance(keyTypeId);
      tmpkey->initKey(i, KEYSIZE);
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(tmpkey->getCacheable());

      ASSERT(tmpkey->getCacheable() != nullptr,
             "tmpkey->getCacheable() is nullptr");

      try {
        LOGDEBUG("CPPTEST: Putting key %s with hashcode %d",
                 keyPtr->toString().c_str(), keyPtr->hashcode());

        dataReg->put(keyPtr, keyPtr->hashcode());
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        ASSERT(!networkhop, "It is networkhop operation");
      } catch (CacheServerException &) {
        LOGERROR("CPPTEST: Put caused extra hop.");
        FAIL("Put caused extra hop.");
      } catch (CacheWriterException &) {
        LOGERROR("CPPTEST: Put caused extra hop.");
        FAIL("Put caused extra hop.");
      } catch (Exception &ex) {
        LOGERROR("CPPTEST: Put caused unexpected %s: %s", ex.getName().c_str(),
                 ex.what());
        cleanProc();
        FAIL("Put caused unexpected exception");
      } catch (...) {
        LOGERROR(
            "CPPTEST: Put caused random exception in "
            "CheckPrSingleHopForAllKeysTask");
        cleanProc();
        FAIL("Put caused unexpected exception");
      }

      try {
        LOGDEBUG("CPPTEST: Destroying key %s with hashcode %d",
                 keyPtr->toString().c_str(), keyPtr->hashcode());

        dataReg->destroy(keyPtr);
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        ASSERT(!networkhop, "It is networkhop operation");
      } catch (CacheServerException &) {
        LOGERROR("CPPTEST: Destroy caused extra hop.");
        FAIL("Destroy caused extra hop.");
      } catch (CacheWriterException &) {
        LOGERROR("CPPTEST: Destroy caused extra hop.");
        FAIL("Destroy caused extra hop.");
      } catch (Exception &ex) {
        LOGERROR("CPPTEST: Destroy caused unexpected %s: %s",
                 ex.getName().c_str(), ex.what());
        cleanProc();
        FAIL("Destroy caused unexpected exception");
      } catch (...) {
        LOGERROR(
            "CPPTEST: Destroy caused random exception in "
            "CheckPrSingleHopForAllKeysTask");
        cleanProc();
        FAIL("Destroy caused unexpected exception");
      }
      delete tmpkey;
    }
    taskIndexPut += static_cast<int>(valueTypes.size());
    if (taskIndexPut == valueTypes.size() * keyTypes.size()) {
      taskIndexPut = 0;
    }

    LOG("CheckPrSingleHopForAllKeysTask completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, ThreadedCheckPrSingleHopForIntKeysTask)
  {
    bool isWarmUpTask = false;
    LOG("ThreadedCheckPrSingleHopForIntKeysTask started.");

    putThread *threads[100];

    for (int thdIdx = 0; thdIdx < 100; thdIdx++) {
      threads[thdIdx] = new putThread(regionNames[0], 200, 1000, isWarmUpTask);
      threads[thdIdx]->start();
    }
    // SLEEP(10000); // wait for threads to become active

    for (int thdIdx = 0; thdIdx < 100; thdIdx++) {
      threads[thdIdx]->stop();
    }

    // SLEEP(20000);
    int totalFailureCount = 0;
    for (int thdIdx = 0; thdIdx < 100; thdIdx++) {
      totalFailureCount += threads[thdIdx]->getFailureCount();
    }
    ASSERT(totalFailureCount == 0,
           "totalFailureCount should be exaclty equal to 0");
    int poolconn = TestUtils::getCacheImpl(getHelper()->cachePtr)
                       ->getPoolSize("__TEST_POOL1__");
    LOGINFO("ThreadedCheckPrSingleHopForIntKeysTask: poolconn is %d ",
            poolconn);

    LOG("ThreadedCheckPrSingleHopForIntKeysTask completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CheckPrSingleHopForIntKeysTask2)
  {
    LOG("CheckPrSingleHopForIntKeysTask2 started.");

    auto dataReg = getHelper()->getRegion(regionNames[0]);

    for (int i = 2000; i < 3000; i++) {
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));

      try {
        LOGDEBUG("CPPTEST: Putting key %d with hashcode %d", i,
                 keyPtr->hashcode());
        dataReg->put(keyPtr, keyPtr->hashcode());
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        int poolconn = TestUtils::getCacheImpl(getHelper()->cachePtr)
                           ->getPoolSize("__TEST_POOL1__");
        LOGINFO(
            "CheckPrSingleHopForIntKeysTask2: networkhop %d poolconn is %d ",
            networkhop, poolconn);
        ASSERT(!networkhop, "It is networkhop operation.");
        ASSERT(poolconn == 2, "pool connections should be equal to 2.");
        // ASSERT( singlehop, "It is not single hop operation" );
      } catch (CacheServerException &) {
        // This is actually a success situation!
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: Put caused extra hop.");
        FAIL("Put caused extra hop.");
        //}
      } catch (CacheWriterException &) {
        // This is actually a success situation! Once bug #521 is fixed.
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: Put caused extra hop.");
        FAIL("Put caused extra hop.");
        //}
      } catch (Exception &ex) {
        LOGERROR("CPPTEST: Put caused unexpected %s: %s", ex.getName().c_str(),
                 ex.what());
        cleanProc();
        FAIL("Put caused unexpected exception");
      } catch (...) {
        LOGERROR("CPPTEST: Put caused random exception");
        cleanProc();
        FAIL("Put caused unexpected exception");
      }

      try {
        LOGINFO("CPPTEST: Destroying key %i with hashcode %d", i,
                keyPtr->hashcode());
        dataReg->destroy(keyPtr);
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        int poolconn = TestUtils::getCacheImpl(getHelper()->cachePtr)
                           ->getPoolSize("__TEST_POOL1__");
        LOGDEBUG(
            "CheckPrSingleHopForIntKeysTask2: networkhop %d poolconn is %d ",
            networkhop, poolconn);
        ASSERT(!networkhop, "It is networkhop operation.");
        ASSERT(poolconn == 2, "pool connections should be equal to 2.");
        // LOGINFO("CheckPrSingleHopForIntKeysTask: networkhop %d ",
        // networkhop);
        // ASSERT(!networkhop , "It is networkhop operation.");
        // ASSERT( singlehop , "It is not single hop operation" );
      } catch (CacheServerException &) {
        // This is actually a success situation!
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: Destroy caused extra hop.");
        FAIL("Destroy caused extra hop.");
        //}
        LOGINFO(
            "CPPTEST: SINGLEHOP SUCCEEDED while destroying key %d with "
            "hashcode "
            "%d",
            i, keyPtr->hashcode());
      } catch (CacheWriterException &) {
        // This is actually a success situation! Once bug #521 is fixed.
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: Destroy caused extra hop.");
        FAIL("Destroy caused extra hop.");
        //}
        LOGINFO(
            "CPPTEST: SINGLEHOP SUCCEEDED while destroying key %d with "
            "hashcode "
            "%d",
            i, keyPtr->hashcode());
      } catch (Exception &ex) {
        LOGERROR("CPPTEST: Destroy caused unexpected %s: %s",
                 ex.getName().c_str(), ex.what());
        cleanProc();
        FAIL("Destroy caused unexpected exception");
      } catch (...) {
        LOGERROR("CPPTEST: Put caused random exception");
        cleanProc();
        FAIL("Put caused unexpected exception");
      }
    }
    LOG("CheckPrSingleHopForIntKeysTask2 put completed.");

    for (int i = 0; i < 1000; i++) {
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));

      try {
        LOGDEBUG("CPPTEST: getting key %d with hashcode %d", i,
                 keyPtr->hashcode());
        dataReg->get(keyPtr /*,(int32_t)keyPtr->hashcode()*/);
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        int poolconn = TestUtils::getCacheImpl(getHelper()->cachePtr)
                           ->getPoolSize("__TEST_POOL1__");
        LOGINFO(
            "CheckPrSingleHopForIntKeysTask2: networkhop %d poolconn is %d ",
            networkhop, poolconn);
        ASSERT(!networkhop, "It is networkhop operation.");
        ASSERT(poolconn == 2, "pool connections should be equal to 2.");
        // LOGINFO("CheckPrSingleHopForIntKeysTask2: networkhop %d ",
        // networkhop);
        // ASSERT(!networkhop , "It is networkhop operation.");
        // ASSERT( singlehop, "It is not single hop operation" );
      } catch (CacheServerException &) {
        // This is actually a success situation!
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: get caused extra hop.");
        FAIL("get caused extra hop.");
        //}
        LOGINFO(
            "CPPTEST: SINGLEHOP SUCCEEDED while getting key %d with hashcode "
            "%d",
            i, keyPtr->hashcode());
      } catch (CacheWriterException &) {
        // This is actually a success situation! Once bug #521 is fixed.
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: get caused extra hop.");
        FAIL("get caused extra hop.");
        //}
        LOGINFO(
            "CPPTEST: SINGLEHOP SUCCEEDED while getting key %d with hashcode "
            "%d",
            i, keyPtr->hashcode());
      } catch (Exception &ex) {
        LOGERROR("CPPTEST: get caused unexpected %s: %s", ex.getName().c_str(),
                 ex.what());
        cleanProc();
        FAIL("get caused unexpected exception");
      } catch (...) {
        LOGERROR("CPPTEST: get caused random exception");
        cleanProc();
        FAIL("get caused unexpected exception");
      }
    }
    LOG("CheckPrSingleHopForIntKeysTask2 get completed.");

    for (int i = 1000; i < 2000; i++) {
      std::vector<std::shared_ptr<CacheableKey>> keysVector;
      for (int j = i; j < i + 5; j++) {
        keysVector.push_back(CacheableInt32::create(j));
      }

      try {
        // LOGINFO("CPPTEST: getting key %d with hashcode %d", i,
        // (int32_t)keyPtr->hashcode());
        dataReg->getAll(keysVector);
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        LOGINFO("CheckPrSingleHopForIntKeysTask2: networkhop %d ", networkhop);
        ASSERT(!networkhop, "It is networkhop operation.");
        // ASSERT( singlehop, "It is not single hop operation" );
      } catch (CacheServerException &) {
        // This is actually a success situation!
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: getAll caused extra hop.");
        FAIL("getAll caused extra hop.");
        //}
        // LOGINFO("CPPTEST: SINGLEHOP SUCCEEDED while getting key %d with
        // hashcode %d", i, (int32_t)keyPtr->hashcode());
      } catch (CacheWriterException &) {
        // This is actually a success situation! Once bug #521 is fixed.
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: getAll caused extra hop.");
        FAIL("getAll caused extra hop.");
        //}
        // LOGINFO("CPPTEST: SINGLEHOP SUCCEEDED while getting key %d with
        // hashcode %d", i, (int32_t)keyPtr->hashcode());
      } catch (Exception &ex) {
        LOGERROR("CPPTEST: getALL caused unexpected %s: %s",
                 ex.getName().c_str(), ex.what());
        cleanProc();
        FAIL("getAll caused unexpected exception");
      } catch (...) {
        LOGERROR("CPPTEST: getAll caused random exception");
        cleanProc();
        FAIL("getAll caused unexpected exception");
      }
    }

    LOG("CheckPrSingleHopForIntKeysTask2 get completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CheckPrSingleHopForGetAllTask)
  {
    auto dataReg = getHelper()->getRegion(regionNames[0]);
    std::vector<std::shared_ptr<CacheableKey>> keysVector;
    for (int i = 0; i < 100; i++) {
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));
      dataReg->put(keyPtr, keyPtr->hashcode());
      keysVector.push_back(keyPtr);
    }

    auto valuesMap = dataReg->getAll(keysVector);
    ASSERT(valuesMap.size() == 100, "GetAll returns wrong number of values");

    valuesMap = dataReg->getAll(keysVector, CacheableInt32::create(1000));
    ASSERT(valuesMap.size() == 100,
           "GetAllWithCallBack returns wrong number of values");

    LOG("CheckPrSingleHopForGetAll get completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CheckPrSingleHopForIntKeysTask)
  {
    LOG("CheckPrSingleHopForIntKeysTask started.");

    auto dataReg = getHelper()->getRegion(regionNames[0]);

    for (int i = 2000; i < 3000; i++) {
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));

      try {
        LOGDEBUG("CPPTEST: Putting key %d with hashcode %d", i,
                 keyPtr->hashcode());
        dataReg->put(keyPtr, keyPtr->hashcode());
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        LOGINFO("CheckPrSingleHopForIntKeysTask: networkhop %d ", networkhop);
        ASSERT(!networkhop, "It is networkhop operation.");
        // ASSERT( singlehop, "It is not single hop operation" );
      } catch (CacheServerException &) {
        // This is actually a success situation!
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: Put caused extra hop.");
        FAIL("Put caused extra hop.");
        //}
      } catch (CacheWriterException &) {
        // This is actually a success situation! Once bug #521 is fixed.
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: Put caused extra hop.");
        FAIL("Put caused extra hop.");
      } catch (Exception &ex) {
        LOGERROR("CPPTEST: Put caused unexpected %s: %s", ex.getName().c_str(),
                 ex.what());
        cleanProc();
        FAIL("Put caused unexpected exception");
      } catch (...) {
        LOGERROR("CPPTEST: Put caused random exception");
        cleanProc();
        FAIL("Put caused unexpected exception");
      }

      try {
        LOGDEBUG("CPPTEST: Destroying key %i with hashcode %d", i,
                 keyPtr->hashcode());
        dataReg->destroy(keyPtr);
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        LOGINFO("CheckPrSingleHopForIntKeysTask: networkhop %d ", networkhop);
        ASSERT(!networkhop, "It is networkhop operation.");
        // ASSERT( singlehop , "It is not single hop operation" );
      } catch (CacheServerException &) {
        // This is actually a success situation!
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: Destroy caused extra hop.");
        FAIL("Destroy caused extra hop.");
        //}
      } catch (CacheWriterException &) {
        // This is actually a success situation! Once bug #521 is fixed.
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: Destroy caused extra hop.");
        FAIL("Destroy caused extra hop.");
      } catch (Exception &ex) {
        LOGERROR("CPPTEST: Destroy caused unexpected %s: %s",
                 ex.getName().c_str(), ex.what());
        cleanProc();
        FAIL("Destroy caused unexpected exception");
      } catch (...) {
        LOGERROR("CPPTEST: Put caused random exception");
        cleanProc();
        FAIL("Put caused unexpected exception");
      }
    }
    LOG("CheckPrSingleHopForIntKeysTask put completed.");

    for (int i = 0; i < 1000; i++) {
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));

      try {
        LOGDEBUG("CPPTEST: getting key %d with hashcode %d", i,
                 keyPtr->hashcode());
        dataReg->get(keyPtr /*,(int32_t)keyPtr->hashcode()*/);
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        LOGINFO("CheckPrSingleHopForIntKeysTask: networkhop %d ", networkhop);
        ASSERT(!networkhop, "It is networkhop operation.");
        // ASSERT( singlehop, "It is not single hop operation" );
      } catch (CacheServerException &) {
        // This is actually a success situation!
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: get caused extra hop.");
        FAIL("get caused extra hop.");
        //}
      } catch (CacheWriterException &) {
        // This is actually a success situation! Once bug #521 is fixed.
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: get caused extra hop.");
        FAIL("get caused extra hop.");
      } catch (Exception &ex) {
        LOGERROR("CPPTEST: get caused unexpected %s: %s", ex.getName().c_str(),
                 ex.what());
        cleanProc();
        FAIL("get caused unexpected exception");
      } catch (...) {
        LOGERROR("CPPTEST: get caused random exception");
        cleanProc();
        FAIL("get caused unexpected exception");
      }
    }
    LOG("CheckPrSingleHopForIntKeysTask get completed.");

    for (int i = 1000; i < 2000; i++) {
      std::vector<std::shared_ptr<CacheableKey>> keysVector;
      for (int j = i; j < i + 5; j++) {
        keysVector.push_back(CacheableInt32::create(j));
      }

      try {
        // LOGINFO("CPPTEST: getting key %d with hashcode %d", i,
        // (int32_t)keyPtr->hashcode());
        dataReg->getAll(keysVector);
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        LOGINFO("CheckPrSingleHopForIntKeysTask: networkhop %d ", networkhop);
        ASSERT(!networkhop, "It is networkhop operation.");
        // ASSERT( singlehop, "It is not single hop operation" );
      } catch (CacheServerException &) {
        // This is actually a success situation!
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: getAll caused extra hop.");
        FAIL("getAll caused extra hop.");
        //}
        // LOGINFO("CPPTEST: SINGLEHOP SUCCEEDED while getting key %d with
        // hashcode %d", i, (int32_t)keyPtr->hashcode());
      } catch (CacheWriterException &) {
        // This is actually a success situation! Once bug #521 is fixed.
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: getAll caused extra hop.");
        FAIL("getAll caused extra hop.");
        //}
        // LOGINFO("CPPTEST: SINGLEHOP SUCCEEDED while getting key %d with
        // hashcode %d", i, (int32_t)keyPtr->hashcode());
      } catch (Exception &ex) {
        LOGERROR("CPPTEST: getALL caused unexpected %s: %s",
                 ex.getName().c_str(), ex.what());
        cleanProc();
        FAIL("getAll caused unexpected exception");
      } catch (...) {
        LOGERROR("CPPTEST: getAll caused random exception");
        cleanProc();
        FAIL("getAll caused unexpected exception");
      }

      try {
        // LOGINFO("CPPTEST: getting key %d with hashcode %d", i,
        // (int32_t)keyPtr->hashcode());
        dataReg->getAll(keysVector, CacheableInt32::create(1000));
        bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                              ->getAndResetNetworkHopFlag();
        LOGINFO("CheckPrSingleHopForIntKeysTask: networkhop %d ", networkhop);
        ASSERT(!networkhop, "It is networkhop operation.");
        // ASSERT( singlehop, "It is not single hop operation" );
      } catch (CacheServerException &) {
        // This is actually a success situation!
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: getAll caused extra hop.");
        FAIL("getAll caused extra hop.");
        //}
        // LOGINFO("CPPTEST: SINGLEHOP SUCCEEDED while getting key %d with
        // hashcode %d", i, (int32_t)keyPtr->hashcode());
      } catch (CacheWriterException &) {
        // This is actually a success situation! Once bug #521 is fixed.
        // bool singlehop = TestUtils::getCacheImpl(getHelper(
        // )->cachePtr)->getAndResetSingleHopFlag();
        // if (!singlehop) {
        LOGERROR("CPPTEST: getAll caused extra hop.");
        FAIL("getAll caused extra hop.");
        //}
        // LOGINFO("CPPTEST: SINGLEHOP SUCCEEDED while getting key %d with
        // hashcode %d", i, (int32_t)keyPtr->hashcode());
      } catch (Exception &ex) {
        LOGERROR("CPPTEST: getALL caused unexpected %s: %s",
                 ex.getName().c_str(), ex.what());
        cleanProc();
        FAIL("getAll caused unexpected exception");
      } catch (...) {
        LOGERROR("CPPTEST: getAll caused random exception");
        cleanProc();
        FAIL("getAll caused unexpected exception");
      }
    }
    int poolconn = TestUtils::getCacheImpl(getHelper()->cachePtr)
                       ->getPoolSize("__TEST_POOL1__");
    LOGINFO("CheckPrSingleHopForIntKeysTask: poolconn is %d ", poolconn);
    LOG("CheckPrSingleHopForIntKeysTask get completed.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  {
    auto pool =
        getHelper()->getCache()->getPoolManager().find("__TEST_POOL1__");
    if (pool->getThreadLocalConnections()) {
      LOG("releaseThreadLocalConnection1 doing...");
      pool->releaseThreadLocalConnection();
      LOG("releaseThreadLocalConnection1 done");
    }
    cleanProc();
  }
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
      CacheHelper::initServer(1, "cacheserver1_partitioned.xml", locatorsG);
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2_With_Locator_PR)
  {
    // starting servers
    if (isLocalServer) {
      CacheHelper::initServer(2, "cacheserver2_partitioned.xml", locatorsG);
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER3, CreateServer3_With_Locator_PR)
  {
    // starting servers
    if (isLocalServer) {
      CacheHelper::initServer(3, "cacheserver3_partitioned.xml", locatorsG);
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CheckGetAllTask)
  {
    auto dataReg = getHelper()->getRegion(regionNames[0]);
    std::vector<std::shared_ptr<CacheableKey>> keysVector;
    for (int i = 0; i < 100000; i++) {
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));
      dataReg->put(keyPtr, keyPtr->hashcode());
      keysVector.push_back(keyPtr);
    }

    ACE_Time_Value startTime = ACE_OS::gettimeofday();
    auto valuesMap = dataReg->getAll(keysVector);
    ACE_Time_Value interval = ACE_OS::gettimeofday() - startTime;
    LOGDEBUG("NILKANTH: Time taken to execute getALL sec = %d and MSec = %d ",
             interval.sec(), interval.usec());
    ASSERT(valuesMap.size() == 100000, "GetAll returns wrong number of values");

    valuesMap = dataReg->getAll(keysVector, CacheableInt32::create(10000));
    ASSERT(valuesMap.size() == 100000,
           "GetAllWithCallBack returns wrong number of values");

    LOGINFO(
        "CheckPrSingleHopForGetAllWithCallBack get completed. "
        "valuesMap->size() "
        "= %d",
        valuesMap.size());
  }
END_TASK_DEFINITION

DUNIT_MAIN
  {
    CacheableHelper::registerBuiltins(true);

    for (int k = 0; k < 2; k++) {
      CALL_TASK(CreateLocator1);

      CALL_TASK(CreateServer1_With_Locator_PR);
      CALL_TASK(CreateServer2_With_Locator_PR);
      CALL_TASK(CreateServer3_With_Locator_PR);

      if (k == 0) {
        CALL_TASK(StepOne_Pooled_Locator);
      } else {
        CALL_TASK(StepOne_Pooled_LocatorTL);
      }

      CALL_TASK(WarmUpTask);
      CALL_TASK(CheckPrSingleHopForIntKeysTask);
      size_t totKeyTypes =
          CacheableWrapperFactory::getRegisteredKeyTypes().size();

      CALL_TASK(CheckPrSingleHopForIntKeysTask);
      for (size_t i = 0; i < totKeyTypes; i++) {
        CALL_TASK(CheckPrSingleHopForAllKeysTask);
      }

      CALL_TASK(CheckPrSingleHopForGetAllTask);

      CALL_TASK(CloseCache1);

      CALL_TASK(CloseServer1);
      CALL_TASK(CloseServer2);
      CALL_TASK(CloseServer3);

      CALL_TASK(CloseLocator1);
    }

    // Check for max-connection value when 3 servers and max-connections set to
    // 2,
    // then chk pools max connection size.
    // TestCase for getAll thread safe. bug #783 and #792
    CALL_TASK(CreateLocator1);  // s1

    CALL_TASK(CreateServer1_With_Locator_PR);  // s1
    CALL_TASK(CreateServer2_With_Locator_PR);  // s2
    CALL_TASK(CreateServer3_With_Locator_PR);  // s3

    CALL_TASK(StepOne_Pooled_Locator);  // c1

    CALL_TASK(CheckGetAllTask);

    CALL_TASK(CloseCache1);

    CALL_TASK(CloseServer1);
    CALL_TASK(CloseServer2);
    CALL_TASK(CloseServer3);

    CALL_TASK(CloseLocator1);
  }
END_MAIN
