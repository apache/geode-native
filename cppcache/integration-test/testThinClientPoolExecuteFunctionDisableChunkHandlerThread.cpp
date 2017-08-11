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
#include "ThinClientHelper.hpp"
#include "testobject/VariousPdxTypes.hpp"
#include <ace/OS.h>
#include <ace/High_Res_Timer.h>

#include <ace/ACE.h>

using namespace PdxTests;
/* This is to test
1- funtion execution on pool
*/

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define LOCATOR1 s2p1
#define SERVER s2p2

bool isLocalServer = false;
bool isLocator = false;
bool isPoolWithEndpoint = false;

const char* locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
const char* poolRegNames[] = {"partition_region", "PoolRegion2"};

const char* serverGroup = "ServerGroup1";

char* getFuncName2 = (char*)"MultiGetFunction2";

DUNIT_TASK_DEFINITION(LOCATOR1, StartLocator1)
  {
    // starting locator
    if (isLocator) {
      CacheHelper::initLocator(1);
      LOG("Locator1 started");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER, StartS12)
  {
    const char* lhp = nullptr;
    if (!isPoolWithEndpoint) lhp = locHostPort;
    if (isLocalServer) {
      CacheHelper::initServer(1, "func_cacheserver1_pool.xml", lhp);
    }
    if (isLocalServer) {
      CacheHelper::initServer(2, "func_cacheserver2_pool.xml", lhp);
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER, startServer2)
  {
    const char* lhp = nullptr;
    if (!isPoolWithEndpoint) lhp = locHostPort;
    if (isLocalServer) {
      CacheHelper::initServer(2, "func_cacheserver2_pool.xml", lhp);
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StartC1)
  {
    // initClient(true);
    initClientWithPool(true, nullptr, locHostPort, serverGroup, nullptr, 0,
                       true);
    // createPool(poolName, locHostPort,serverGroup, nullptr, 0, true );
    // createRegionAndAttachPool(poolRegNames[0],USE_ACK, poolName);

    RegionPtr regPtr0 =
        createRegionAndAttachPool(poolRegNames[0], USE_ACK, nullptr);
    ;  // getHelper()->createRegion( poolRegNames[0], USE_ACK);
    regPtr0->registerAllKeys();

    LOG("Clnt1Init complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StopC1)
  {
    cleanProc();
    LOG("Clnt1Down complete: Keepalive = True");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER, CloseServers)
  {
    // stop servers
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
    isPoolWithEndpoint = true;
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(LOCATOR1, CloseLocator1)
  {
    // stop locator
    if (isLocator) {
      CacheHelper::closeLocator(1);
      LOG("Locator1 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StartTestClient)
  {
    LOG("in before starting StartTestClient");
    PropertiesPtr config = Properties::create();
    config->insert("disable-chunk-handler-thread", "true");
    config->insert("read-timeout-unit-in-millis", "true");
    config->insert("ping-interval", "-1");
    config->insert("bucket-wait-timeout", "2000");
    config->insert("connect-wait-timeout", "10");

    initClientWithPool(true, nullptr, locHostPort, serverGroup, config, 0, true,
                       -1, -1, -1, true, false);
    // createPool(poolName, locHostPort,serverGroup, nullptr, 0, true );

    RegionPtr regPtr0 =
        createRegionAndAttachPool(poolRegNames[0], USE_ACK, nullptr);
    ;  // getHelper()->createRegion( poolRegNames[0], USE_ACK);

    LOG("StartTestClient complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StartTestClient2)
  {
    LOG("in before starting StartTestClient");
    PropertiesPtr config = Properties::create();
    config->insert("disable-chunk-handler-thread", "true");
    config->insert("read-timeout-unit-in-millis", "true");
    config->insert("ping-interval", "-1");
    config->insert("bucket-wait-timeout", "2000");
    config->insert("connect-wait-timeout", "10");

    initClientWithPool(true, nullptr, locHostPort, serverGroup, config, 0, true,
                       -1, -1, -1, true, false);
    // createPool(poolName, locHostPort,serverGroup, nullptr, 0, true );

    RegionPtr regPtr0 =
        createRegionAndAttachPool(poolRegNames[0], USE_ACK, nullptr);
    ;  // getHelper()->createRegion( poolRegNames[0], USE_ACK);

    LOG("StartTestClient complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, clientPuts)
  {
    RegionPtr regPtr0 = getHelper()->getRegion(poolRegNames[0]);
    char buf[128];
    for (int i = 1; i <= 500; i++) {
      CacheablePtr value(CacheableInt32::create(i));

      sprintf(buf, "am-%d", i);
      CacheableKeyPtr key = CacheableKey::create(buf);
      regPtr0->put(key, value);
    }
    LOG("clientPuts complete.");
  }
END_TASK_DEFINITION

class putThread : public ACE_Task_Base {
 private:
  RegionPtr regPtr;
  int m_min;
  int m_max;
  int m_failureCount;
  int m_timeoutCount;
  volatile bool m_stop;

 public:
  putThread(RegionPtr rp, int min, int max, bool isWarmUpTask)
      : regPtr(rp),
        m_min(min),
        m_max(max),
        m_failureCount(0),
        m_timeoutCount(0),
        m_stop(false) {}

  int getFailureCount() { return m_failureCount; }

  int getTimeoutCount() { return m_timeoutCount; }

  int svc(void) {
    bool networkhop ATTR_UNUSED = false;
    CacheableKeyPtr keyPtr;
    CacheablePtr args = nullptr;
    ResultCollectorPtr rPtr = nullptr;
    RegionPtr regPtr0 = getHelper()->getRegion(poolRegNames[0]);
    while (!m_stop) {
      for (int i = m_min; i < m_max; i++) {
        try {
          char buf[128];
          sprintf(buf, "am-%d", i);
          CacheableKeyPtr key = CacheableKey::create(buf);
          CacheableVectorPtr routingObj = CacheableVector::create();
          routingObj->push_back(key);
          ExecutionPtr exc = FunctionService::onRegion(regPtr0);
          exc->execute(routingObj, args, rPtr, getFuncName2, 300 /*in millis*/)
              ->getResult();
        } catch (const TimeoutException& te) {
          LOGINFO("Timeout exception occurred %s", te.getMessage());
          m_timeoutCount++;
        } catch (const Exception&) {
          LOG("Exception occurred");
        } catch (...) {
          LOG("Random Exception occurred");
        }
      }
    }
    LOG("PutThread done");
    return 0;
  }
  void start() { activate(); }
  void stop() {
    m_stop = true;
    wait();
  }
};

void executeFunction() {
  RegionPtr regPtr0 = getHelper()->getRegion(poolRegNames[0]);
  TestUtils::getCacheImpl(getHelper()->cachePtr)->getAndResetNetworkHopFlag();
  CacheablePtr args = nullptr;
  ResultCollectorPtr rPtr = nullptr;
  int failureCount = 0;
  LOGINFO("executeFunction started");
  for (int i = 0; i < 300; i++) {
    LOGINFO("executeFunction %d ", i);
    bool networkhop = TestUtils::getCacheImpl(getHelper()->cachePtr)
                          ->getAndResetNetworkHopFlag();
    if (networkhop) {
      failureCount++;
    }
    char buf[128];
    sprintf(buf, "am-%d", i);
    CacheableKeyPtr key = CacheableKey::create(buf);
    CacheableVectorPtr routingObj = CacheableVector::create();
    routingObj->push_back(key);
    ExecutionPtr exc = FunctionService::onRegion(regPtr0);
    exc->execute(routingObj, args, rPtr, getFuncName2, 300 /*in millis*/)
        ->getResult();
  }
  LOGINFO("executeFunction failureCount %d", failureCount);
  ASSERT(failureCount <= 10 && failureCount > 0, "failureCount should be zero");
}

const int nThreads = 10;
putThread* threads[nThreads];

DUNIT_TASK_DEFINITION(CLIENT1, dofuncOps)
  {
    RegionPtr regPtr0 = getHelper()->getRegion(poolRegNames[0]);
    // check nextwork hop for single key
    executeFunction();

#ifdef __linux

    for (int thdIdx = 0; thdIdx < nThreads; thdIdx++) {
      threads[thdIdx] = new putThread(regPtr0, 0, 500, false);
      threads[thdIdx]->start();
    }
#endif
    LOG("dofuncOps complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, closeServer2)
  {
    // stop servers
    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
    }
  }
END_TASK_DEFINITION

void waitForNoTimeout() {
  LOGINFO("entering into waitForNoTimeout");
  SLEEP(10);
  int maxTry = 1000;  // 10 seconds
  int thdIdx = 0;
  while (maxTry-- > 0) {
    for (thdIdx = 0; thdIdx < nThreads; thdIdx++) {
      int currentTimeout = threads[thdIdx]->getTimeoutCount();
      SLEEP(10);
      if (currentTimeout != threads[thdIdx]->getTimeoutCount()) break;
    }
    if (thdIdx == nThreads) break;
  }

  LOGINFO("waitForNoTimeout nThreads: %d,  thdIdx: %d", nThreads, thdIdx);
  if (thdIdx < nThreads) {
    LOGINFO(
        "waitForNoTimeout failed still getting timeouts nThreads: %d,  thdIdx: "
        "%d",
        nThreads, thdIdx);
    ASSERT(thdIdx < nThreads, "waitForNoTimeout failed still getting timeouts");
  }
  SLEEP(20000);
}

void verifyTimeoutFirst() {
  int totalTimeoutCount = 0;
  for (int thdIdx = 0; thdIdx < nThreads; thdIdx++) {
    totalTimeoutCount += threads[thdIdx]->getTimeoutCount();
  }

  LOGINFO("Total timeout %d", totalTimeoutCount);

  int blackListBucketTimeouts = TestUtils::getCacheImpl(getHelper()->cachePtr)
                                    ->getBlackListBucketTimeouts();

  LOGINFO("blackListBucketTimeouts %d", blackListBucketTimeouts);

  ASSERT(totalTimeoutCount > 0,
         "totalTimeoutCount should be greater than zero");

  ASSERT(blackListBucketTimeouts > 0,
         "blackListBucketTimeouts should be greater than zero");
}

DUNIT_TASK_DEFINITION(CLIENT1, stopClientThreads)
  {
#ifdef __linux
    for (int thdIdx = 0; thdIdx < nThreads; thdIdx++) {
      threads[thdIdx]->stop();
    }

    LOG("Linux is defined");
#endif
    LOG("completed stopClientThreads");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, verifyClientResults)
  {
#ifdef __linux
    /*for(int thdIdx = 0; thdIdx < nThreads; thdIdx++)
    {
      threads[thdIdx]->stop();
    }*/

    verifyTimeoutFirst();

    waitForNoTimeout();

    LOG("Linux is defined");
#endif
    LOG("completed verifyClientResults");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, closeServer1)
  {
    // stop servers
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
  }
END_TASK_DEFINITION

void runFunctionExecutionDisableChunkHandlerThread() {
  // with locator
  CALL_TASK(StartLocator1);
  // start two servers
  CALL_TASK(StartS12);
  CALL_TASK(StartTestClient);
  CALL_TASK(StartTestClient2);
  // to create pr meta data
  CALL_TASK(clientPuts);
  // need to spawn thread which will do continuous FE
  CALL_TASK(dofuncOps);
  CALL_TASK(closeServer2);
  // check whether you get timeouts
  CALL_TASK(verifyClientResults);

  // starting server2
  CALL_TASK(startServer2);         // starting server again
  CALL_TASK(verifyClientResults);  // verifying timeouts again

  // stopping server1
  CALL_TASK(closeServer1);
  CALL_TASK(verifyClientResults);

  CALL_TASK(stopClientThreads);  // verifying timeouts again
  CALL_TASK(StopC1);
  CALL_TASK(closeServer2);
  CALL_TASK(CloseLocator1);
}

DUNIT_MAIN
  { runFunctionExecutionDisableChunkHandlerThread(); }
END_MAIN
