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
#include <geode/FunctionService.hpp>
#include <geode/Execution.hpp>
#include <geode/ResultCollector.hpp>

#define CLIENT1 s1p1
#define LOCATOR1 s2p1
#define SERVER s2p2

bool isLocalServer = false;
bool isLocator = false;
bool isPoolWithEndpoint = false;

const char* endPoints1 = CacheHelper::getTcrEndpoints(isLocalServer, 3);
const char* locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
const char* poolRegNames[] = {"partition_region", "PoolRegion2"};
const char* poolName = "__TEST_POOL1__";

const char* serverGroup = "ServerGroup1";

char* OnServerHAExceptionFunction = (char*)"OnServerHAExceptionFunction";
char* OnServerHAShutdownFunction = (char*)"OnServerHAShutdownFunction";

char* RegionOperationsHAFunction = (char*)"RegionOperationsHAFunction";
#define verifyGetResults()                                      \
  bool found = false;                                           \
  for (int j = 0; j < 34; j++) {                                \
    if (j % 2 == 0) continue;                                   \
    sprintf(buf, "VALUE--%d", j);                               \
    if (strcmp(buf, std::dynamic_pointer_cast<CacheableString>( \
                        resultList->operator[](i))              \
                        ->asChar()) == 0) {                     \
      found = true;                                             \
      break;                                                    \
    }                                                           \
  }                                                             \
  ASSERT(found, "this returned value is invalid");

#define verifyPutResults()                   \
  bool found = false;                        \
  for (int j = 0; j < 34; j++) {             \
    if (j % 2 == 0) continue;                \
    sprintf(buf, "KEY--%d", j);              \
    if (strcmp(buf, value->asChar()) == 0) { \
      found = true;                          \
      break;                                 \
    }                                        \
  }                                          \
  ASSERT(found, "this returned value is invalid");
class MyResultCollector : public ResultCollector {
 public:
  MyResultCollector()
      : m_resultList(CacheableVector::create()),
        m_isResultReady(false),
        m_endResultCount(0),
        m_addResultCount(0),
        m_getResultCount(0) {}
  ~MyResultCollector() {}
  CacheableVectorPtr getResult(uint32_t timeout) {
    m_getResultCount++;
    if (m_isResultReady == true) {
      return m_resultList;
    } else {
      for (uint32_t i = 0; i < timeout; i++) {
        SLEEP(1);
        if (m_isResultReady == true) return m_resultList;
      }
      throw FunctionExecutionException(
          "Result is not ready, endResults callback is called before invoking "
          "getResult() method");
    }
  }

  void addResult(const CacheablePtr& resultItem) {
    m_addResultCount++;
    if (resultItem == nullptr) return;
    auto result = std::dynamic_pointer_cast<CacheableArrayList>(resultItem);
    for (int32_t i = 0; i < result->size(); i++) {
      m_resultList->push_back(result->operator[](i));
    }
  }
  void endResults() {
    m_isResultReady = true;
    m_endResultCount++;
  }
  uint32_t getEndResultCount() { return m_endResultCount; }
  uint32_t getAddResultCount() { return m_addResultCount; }
  uint32_t getGetResultCount() { return m_getResultCount; }

 private:
  CacheableVectorPtr m_resultList;
  volatile bool m_isResultReady;
  uint32_t m_endResultCount;
  uint32_t m_addResultCount;
  uint32_t m_getResultCount;
};
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

DUNIT_TASK_DEFINITION(SERVER, StartS13)
  {
    const char* lhp = nullptr;
    if (!isPoolWithEndpoint) lhp = locHostPort;
    if (isLocalServer) {
      CacheHelper::initServer(1, "func_cacheserver1_pool.xml", lhp);
    }
    if (isLocalServer) {
      CacheHelper::initServer(2, "func_cacheserver2_pool.xml", lhp);
    }
    if (isLocalServer) {
      CacheHelper::initServer(3, "func_cacheserver3_pool.xml", lhp);
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StartC1)
  {
    initClientWithPool(true, poolRegNames[0], locHostPort, serverGroup, nullptr,
                       0, true, -1, 5, 60000);
    // createPool(poolName, locHostPort,serverGroup, nullptr, 0, true );
    // createRegionAndAttachPool(poolRegNames[0],USE_ACK, poolName);

    RegionPtr regPtr0 =
        createRegionAndAttachPool(poolRegNames[0], USE_ACK, nullptr);
    ;  // getHelper()->createRegion( poolRegNames[0], USE_ACK);
    regPtr0->registerAllKeys();

    LOG("Clnt1Init complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StartC11)
  {
    initClient(true);
    createPool(poolName, locHostPort, serverGroup, 0, true);
    createRegionAndAttachPool(poolRegNames[0], USE_ACK, poolName);

    RegionPtr regPtr0 = getHelper()->getRegion(poolRegNames[0]);
    regPtr0->registerAllKeys();

    LOG("Clnt1Init complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, Client1OpTest)
  {
    auto regPtr0 = getHelper()->getRegion(poolRegNames[0]);
    char buf[128];

    for (int i = 0; i < 34; i++) {
      sprintf(buf, "VALUE--%d", i);
      auto value = CacheableString::create(buf);

      sprintf(buf, "KEY--%d", i);
      auto key = CacheableKey::create(buf);
      regPtr0->put(key, value);
    }
    SLEEP(10000);  // let the put finish
    try {
      auto routingObj = CacheableVector::create();
      for (int i = 0; i < 34; i++) {
        if (i % 2 == 0) continue;
        sprintf(buf, "KEY--%d", i);
        CacheableKeyPtr key = CacheableKey::create(buf);
        routingObj->push_back(key);
      }
      // UNUSED bool getResult = true;
      auto exc = FunctionService::onRegion(regPtr0);
      ASSERT(exc != nullptr, "onRegion Returned nullptr");
      auto resultList = CacheableVector::create();

      auto executeFunctionResult = exc->withArgs(routingObj)
                                       ->execute(RegionOperationsHAFunction, 15)
                                       ->getResult();

      if (executeFunctionResult == nullptr) {
        ASSERT(false, "get executeFunctionResult is nullptr");
      } else {
        sprintf(buf, "echo String : result count = %zd",
                executeFunctionResult->size());
        LOG(buf);
        resultList->clear();

        for (unsigned item = 0;
             item < static_cast<uint32_t>(executeFunctionResult->size());
             item++) {
          auto arrayList = std::dynamic_pointer_cast<CacheableArrayList>(
              executeFunctionResult->operator[](item));
          for (unsigned pos = 0; pos < static_cast<uint32_t>(arrayList->size());
               pos++) {
            resultList->push_back(arrayList->operator[](pos));
          }
        }
        sprintf(buf, "get result count = %zd", resultList->size());
        LOG(buf);
        ASSERT(resultList->size() == 17,
               "get executeFunctionResult count is not 17");
        for (int32_t i = 0; i < resultList->size(); i++) {
          sprintf(buf, "result[%d] is null\n", i);
          ASSERT(resultList->operator[](i) != nullptr, buf);
          sprintf(buf, "get result[%d]=%s", i,
                  std::dynamic_pointer_cast<CacheableString>(
                      resultList->operator[](i))
                      ->asChar());
          LOG(buf);
          verifyGetResults()
        }
      }

      /*-------------------------------onRegion with single filter
       * key---------------------------------------*/
      auto filter = CacheableVector::create();
      const char* key = "KEY--10";
      filter->push_back(CacheableString::create(key));
      executeFunctionResult = exc->withArgs(routingObj)
                                  ->withFilter(filter)
                                  ->execute(RegionOperationsHAFunction, 15)
                                  ->getResult();

      if (executeFunctionResult == nullptr) {
        ASSERT(false, "get executeFunctionResult is nullptr");
      } else {
        sprintf(buf, "echo String : result count = %zd",
                executeFunctionResult->size());
        LOG(buf);
        resultList->clear();

        for (unsigned item = 0;
             item < static_cast<uint32_t>(executeFunctionResult->size());
             item++) {
          auto arrayList = std::dynamic_pointer_cast<CacheableArrayList>(
              executeFunctionResult->operator[](item));
          for (unsigned pos = 0; pos < static_cast<uint32_t>(arrayList->size());
               pos++) {
            resultList->push_back(arrayList->operator[](pos));
          }
        }
        sprintf(buf, "get result count = %zd", resultList->size());
        LOG(buf);
        ASSERT(resultList->size() == 17,
               "get executeFunctionResult count is not 17");
        for (int32_t i = 0; i < resultList->size(); i++) {
          sprintf(buf, "result[%d] is null\n", i);
          ASSERT(resultList->operator[](i) != nullptr, buf);
          sprintf(buf, "get result[%d]=%s", i,
                  std::dynamic_pointer_cast<CacheableString>(
                      resultList->operator[](i))
                      ->asChar());
          LOG(buf);
          verifyGetResults()
        }
      }
      /*-------------------------------onRegion with single filter key
       * done---------------------------------------*/
    } catch (const Exception& excp) {
      std::string logmsg = "";
      logmsg += excp.getName();
      logmsg += ": ";
      logmsg += excp.getMessage();
      LOG(logmsg.c_str());
      excp.printStackTrace();
      FAIL("Function Execution Failed!");
    }
    isPoolWithEndpoint = true;
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, Client1OnServerHATest)
  {
    auto regPtr0 = getHelper()->getRegion(poolRegNames[0]);
    char buf[128];

    for (int i = 0; i < 34; i++) {
      sprintf(buf, "VALUE--%d", i);
      auto value = CacheableString::create(buf);

      sprintf(buf, "KEY--%d", i);
      auto key = CacheableKey::create(buf);
      regPtr0->put(key, value);
    }
    SLEEP(10000);  // let the put finish
    try {
      auto routingObj = CacheableVector::create();
      for (int i = 0; i < 34; i++) {
        if (i % 2 == 0) continue;
        sprintf(buf, "KEY--%d", i);
        auto key = CacheableKey::create(buf);
        routingObj->push_back(key);
      }

      // UNUSED bool getResult = true;
      auto pool =
          getHelper()->getCache()->getPoolManager().find("__TEST_POOL1__");
      auto exc = FunctionService::onServer(pool);
      ASSERT(exc != nullptr, "onServer Returned nullptr");

      auto resultList = CacheableVector::create();

      // Test with HA exception
      auto executeFunctionResult =
          exc->withArgs(routingObj)
              ->execute(OnServerHAExceptionFunction, 15)
              ->getResult();

      if (executeFunctionResult == nullptr) {
        ASSERT(false, "get executeFunctionResult is nullptr");
      } else {
        sprintf(buf, "echo String : result count = %zd",
                executeFunctionResult->size());
        LOG(buf);
        resultList->clear();
        for (unsigned item = 0;
             item < static_cast<uint32_t>(executeFunctionResult->size());
             item++) {
          auto arrayList = std::dynamic_pointer_cast<CacheableArrayList>(
              executeFunctionResult->operator[](item));
          for (unsigned pos = 0; pos < static_cast<uint32_t>(arrayList->size());
               pos++) {
            resultList->push_back(arrayList->operator[](pos));
          }
        }
        sprintf(buf, "get result count = %zd", resultList->size());
        LOG(buf);
        ASSERT(resultList->size() == 17,
               "get executeFunctionResult count is not 17");
        for (int32_t i = 0; i < resultList->size(); i++) {
          sprintf(buf, "result[%d] is null\n", i);
          ASSERT(resultList->operator[](i) != nullptr, buf);
          sprintf(buf, "get result[%d]=%s", i,
                  std::dynamic_pointer_cast<CacheableString>(
                      resultList->operator[](i))
                      ->asChar());
          LOG(buf);
          verifyGetResults()
        }
      }

      // Test with HA server shutdown
      auto executeFunctionResult1 =
          exc->withArgs(routingObj)
              ->execute(OnServerHAShutdownFunction, 15)
              ->getResult();

      if (executeFunctionResult1 == nullptr) {
        ASSERT(false, "get executeFunctionResult1 is nullptr");
      } else {
        sprintf(buf, "echo String : result count = %zd",
                executeFunctionResult1->size());
        LOG(buf);
        resultList->clear();
        for (unsigned item = 0;
             item < static_cast<uint32_t>(executeFunctionResult1->size());
             item++) {
          auto arrayList = std::dynamic_pointer_cast<CacheableArrayList>(
              executeFunctionResult1->operator[](item));
          for (unsigned pos = 0; pos < static_cast<uint32_t>(arrayList->size());
               pos++) {
            resultList->push_back(arrayList->operator[](pos));
          }
        }
        sprintf(buf, "get result count = %zd", resultList->size());
        LOG(buf);
        ASSERT(resultList->size() == 17,
               "get executeFunctionResult1 count is not 17");
        for (int32_t i = 0; i < resultList->size(); i++) {
          sprintf(buf, "result[%d] is null\n", i);
          ASSERT(resultList->operator[](i) != nullptr, buf);
          sprintf(buf, "get result[%d]=%s", i,
                  std::dynamic_pointer_cast<CacheableString>(
                      resultList->operator[](i))
                      ->asChar());
          LOG(buf);
          verifyGetResults()
        }
      }
    } catch (const Exception& excp) {
      std::string logmsg = "";
      logmsg += excp.getName();
      logmsg += ": ";
      logmsg += excp.getMessage();
      LOG(logmsg.c_str());
      excp.printStackTrace();
      FAIL("Function Execution Failed!");
    }
    isPoolWithEndpoint = true;
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StopC1)
  {
    cleanProc();
    LOG("Clnt1Down complete: Keepalive = True");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER, CloseServers12)
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
    isPoolWithEndpoint = true;
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER, CloseServers13)
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

void runFunctionExecution() {
  CALL_TASK(StartLocator1);
  CALL_TASK(StartS12);
  CALL_TASK(StartC1);
  CALL_TASK(Client1OpTest);  // This tests isHA with onRegion
  CALL_TASK(StopC1);
  CALL_TASK(CloseServers12);
  CALL_TASK(CloseLocator1);
}

DUNIT_MAIN
  { runFunctionExecution(); }
END_MAIN
