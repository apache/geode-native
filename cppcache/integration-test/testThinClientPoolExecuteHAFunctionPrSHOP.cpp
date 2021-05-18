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
#include <geode/DefaultResultCollector.hpp>

#define CLIENT1 s1p1
#define LOCATOR1 s2p1
#define SERVER s2p2

using apache::geode::client::Cacheable;
using apache::geode::client::CacheableArrayList;
using apache::geode::client::CacheableVector;
using apache::geode::client::DefaultResultCollector;
using apache::geode::client::Exception;
using apache::geode::client::FunctionService;

bool isLocalServer = false;
bool isLocator = false;

const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
const char *poolRegNames[] = {"partition_region", "PoolRegion2"};
const char *poolName = "__TEST_POOL1__";

const char *serverGroup = "ServerGroup1";

const char *OnServerHAExceptionFunction = "OnServerHAExceptionFunction";
const char *OnServerHAShutdownFunction = "OnServerHAShutdownFunction";

const char *RegionOperationsHAFunction = "RegionOperationsHAFunction";
const char *RegionOperationsHAFunctionPrSHOP =
    "RegionOperationsHAFunctionPrSHOP";
#define verifyGetResults()                                      \
  bool found = false;                                           \
  for (int j = 0; j < 34; j++) {                                \
    if (j % 2 == 0) continue;                                   \
    sprintf(buf, "VALUE--%d", j);                               \
    if (strcmp(buf, std::dynamic_pointer_cast<CacheableString>( \
                        resultList->operator[](i))              \
                        ->value()                               \
                        .c_str()) == 0) {                       \
      found = true;                                             \
      break;                                                    \
    }                                                           \
  }                                                             \
  ASSERT(found, "this returned value is invalid");

class MyResultCollector : public DefaultResultCollector {
 public:
  MyResultCollector()
      : m_endResultCount(0), m_addResultCount(0), m_getResultCount(0) {}
  ~MyResultCollector() noexcept override {}

  std::shared_ptr<CacheableVector> getResult(
      std::chrono::milliseconds timeout) override {
    m_getResultCount++;
    return DefaultResultCollector::getResult(timeout);
  }

  void addResult(const std::shared_ptr<Cacheable> &resultItem) override {
    m_addResultCount++;
    if (resultItem == nullptr) {
      return;
    }
    if (auto results =
            std::dynamic_pointer_cast<CacheableArrayList>(resultItem)) {
      for (auto &result : *results) {
        DefaultResultCollector::addResult(result);
      }
    } else {
      DefaultResultCollector::addResult(resultItem);
    }
  }

  void endResults() override {
    m_endResultCount++;
    DefaultResultCollector::endResults();
  }

  uint32_t getEndResultCount() { return m_endResultCount; }
  uint32_t getAddResultCount() { return m_addResultCount; }
  uint32_t getGetResultCount() { return m_getResultCount; }

 private:
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
    if (isLocalServer) {
      CacheHelper::initServer(1, "func_cacheserver1_pool.xml", locHostPort);
    }
    if (isLocalServer) {
      CacheHelper::initServer(2, "func_cacheserver2_pool.xml", locHostPort);
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER, StartS13)
  {
    if (isLocalServer) {
      CacheHelper::initServer(1, "func_cacheserver1_pool.xml", locHostPort);
    }
    if (isLocalServer) {
      CacheHelper::initServer(2, "func_cacheserver2_pool.xml", locHostPort);
    }
    if (isLocalServer) {
      CacheHelper::initServer(3, "func_cacheserver3_pool.xml", locHostPort);
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StartC1)
  {
    // initClient(true);
    initClientWithPool(true, poolRegNames[0], locHostPort, serverGroup, nullptr,
                       1, true, -1, 5, 60000, /*singlehop*/ true,
                       /*threadLocal*/ true);
    // createPool(poolName, locHostPort,serverGroup, nullptr, 0, true );
    // createRegionAndAttachPool(poolRegNames[0],USE_ACK, poolName);

    auto regPtr0 = createRegionAndAttachPool(poolRegNames[0], USE_ACK);
    // getHelper()->createRegion( poolRegNames[0], USE_ACK);
    regPtr0->registerAllKeys();

    LOG("Clnt1Init complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StartC11)
  {
    initClient(true);
    createPool(poolName, locHostPort, serverGroup, 0, true);
    createRegionAndAttachPool(poolRegNames[0], USE_ACK, poolName);

    auto regPtr0 = getHelper()->getRegion(poolRegNames[0]);
    regPtr0->registerAllKeys();

    LOG("Clnt1Init complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, Client1OpTest)
  {
    auto regPtr0 = getHelper()->getRegion(poolRegNames[0]);
    char buf[128];

    for (int i = 0; i < 350; i++) {
      sprintf(buf, "VALUE--%d", i);
      auto value = CacheableString::create(buf);

      sprintf(buf, "KEY--%d", i);
      auto key = CacheableString::create(buf);
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
      auto exc = FunctionService::onRegion(regPtr0);
      auto resultList = CacheableVector::create();

      auto executeFunctionResult =
          exc.withArgs(routingObj)
              .execute(RegionOperationsHAFunctionPrSHOP,
                       std::chrono::seconds(15))
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
        for (size_t i = 0; i < resultList->size(); i++) {
          sprintf(buf, "result[%zd] is null\n", i);
          ASSERT(resultList->operator[](i) != nullptr, buf);
          sprintf(buf, "get result[%zd]=%s", i,
                  std::dynamic_pointer_cast<CacheableString>(
                      resultList->operator[](i))
                      ->value()
                      .c_str());
          LOG(buf);
          verifyGetResults()
        }
      }
    } catch (const Exception &excp) {
      std::string logmsg = "";
      logmsg += excp.getName();
      logmsg += ": ";
      logmsg += excp.what();
      LOG(logmsg.c_str());
      LOG(excp.getStackTrace());
      FAIL("Function Execution Failed!");
    }
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
      auto key = CacheableString::create(buf);
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

      auto resultList = CacheableVector::create();

      // Test with HA exception
      auto executeFunctionResult =
          exc.withArgs(routingObj)
              .execute(OnServerHAExceptionFunction, std::chrono::seconds(15))
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
        for (size_t i = 0; i < resultList->size(); i++) {
          sprintf(buf, "result[%zd] is null\n", i);
          ASSERT(resultList->operator[](i) != nullptr, buf);
          sprintf(buf, "get result[%zd]=%s", i,
                  std::dynamic_pointer_cast<CacheableString>(
                      resultList->operator[](i))
                      ->value()
                      .c_str());
          LOG(buf);
          verifyGetResults()
        }
      }

      // Test with HA server shutdown
      auto executeFunctionResult1 =
          exc.withArgs(routingObj)
              .execute(OnServerHAShutdownFunction, std::chrono::seconds(15))
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
        for (size_t i = 0; i < resultList->size(); i++) {
          sprintf(buf, "result[%zd] is null\n", i);
          ASSERT(resultList->operator[](i) != nullptr, buf);
          sprintf(buf, "get result[%zd]=%s", i,
                  std::dynamic_pointer_cast<CacheableString>(
                      resultList->operator[](i))
                      ->value()
                      .c_str());
          LOG(buf);
          verifyGetResults()
        }
      }
    } catch (const Exception &excp) {
      std::string logmsg = "";
      logmsg += excp.getName();
      logmsg += ": ";
      logmsg += excp.what();
      LOG(logmsg.c_str());
      LOG(excp.getStackTrace());
      FAIL("Function Execution Failed!");
    }
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

  CALL_TASK(StartLocator1);
  CALL_TASK(StartS13);
  CALL_TASK(StartC11);
  CALL_TASK(Client1OnServerHATest);  // This tests isHA with onServer
  CALL_TASK(StopC1);
  CALL_TASK(CloseServers13);
  CALL_TASK(CloseLocator1);
}

DUNIT_MAIN
  { runFunctionExecution(); }
END_MAIN
