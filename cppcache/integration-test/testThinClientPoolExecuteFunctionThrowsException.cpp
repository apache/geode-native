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

#define ROOT_NAME "testThinClientPoolExecuteFunctionThrowsException"

#include <thread>
#include <chrono>

#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"
#include "testobject/VariousPdxTypes.hpp"
#include <geode/UserFunctionExecutionException.hpp>
#include <geode/FunctionService.hpp>
#include <geode/DefaultResultCollector.hpp>

/* This is to test
1- funtion execution on pool
 */

#define CLIENT1 s1p1
#define LOCATOR1 s2p1
#define SERVER s2p2

using apache::geode::client::Cacheable;
using apache::geode::client::CacheableArrayList;
using apache::geode::client::CacheableBoolean;
using apache::geode::client::CacheableVector;
using apache::geode::client::DefaultResultCollector;
using apache::geode::client::FunctionService;
using apache::geode::client::UserFunctionExecutionException;

bool isLocalServer = false;
bool isLocator = false;
bool isPoolWithEndpoint = false;

const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
const char *poolRegNames[] = {"partition_region", "PoolRegion2"};

const char *serverGroup = "ServerGroup1";

const char *getFuncIName = "MultiGetFunctionI";
const char *putFuncIName = "MultiPutFunctionI";
const char *getFuncName = "MultiGetFunction";
const char *putFuncName = "MultiPutFunction";
const char *rjFuncName = "RegionOperationsFunction";
const char *exFuncName = "ExceptionHandlingFunction";
const char *exFuncNameSendException = "executeFunction_SendException";
const char *exFuncNamePdxType = "PdxFunctionTest";
const char *FEOnRegionPrSHOP = "FEOnRegionPrSHOP";
const char *FEOnRegionPrSHOP_OptimizeForWrite =
    "FEOnRegionPrSHOP_OptimizeForWrite";

class MyResultCollector : public DefaultResultCollector {
 public:
  MyResultCollector()
      : m_endResultCount(0), m_addResultCount(0), m_getResultCount(0) {}
  ~MyResultCollector() override = default;

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
    std::string lhp;
    if (!isPoolWithEndpoint) {
      lhp = locHostPort;
    }

    if (isLocalServer) {
      CacheHelper::initServer(1, "func_cacheserver1_pool.xml", lhp);
    }

    if (isLocalServer) {
      CacheHelper::initServer(2, "func_cacheserver2_pool.xml", lhp);
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StartC1)
  {
    // initClient(true);
    initClientWithPool(true, poolRegNames[0], locHostPort, serverGroup, nullptr,
                       0, true, -1, -1, 60000, /*singlehop*/ true,
                       /*threadLocal*/ true);
    // createPool(poolName, locHostPort,serverGroup, nullptr, 0, true );
    // createRegionAndAttachPool(poolRegNames[0],USE_ACK, poolName);

    auto regPtr0 = createRegionAndAttachPool(poolRegNames[0], USE_ACK);
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
      auto key = CacheableString::create(buf);
      regPtr0->put(key, value);
    }
    std::this_thread::sleep_for(
        std::chrono::seconds(10));  // let the put finish

    //-----------------------Test with sendException
    // onRegion-------------------------------//

    for (int i = 1; i <= 200; i++) {
      std::shared_ptr<Cacheable> value(CacheableInt32::create(i));

      sprintf(buf, "execKey-%d", i);
      auto key = CacheableKey::create(buf);
      regPtr0->put(key, value);
    }
    LOG("Put for execKey's on region complete.");

    LOG("Adding filter");
    auto arrList = CacheableArrayList::create();
    for (int i = 100; i < 120; i++) {
      sprintf(buf, "execKey-%d", i);
      auto key = CacheableKey::create(buf);
      arrList->push_back(key);
    }

    auto filter = CacheableVector::create();
    for (int i = 100; i < 120; i++) {
      sprintf(buf, "execKey-%d", i);
      auto key = CacheableKey::create(buf);
      filter->push_back(key);
    }
    LOG("Adding filter done.");

    auto args = CacheableBoolean::create(1);

    auto funcExec = FunctionService::onRegion(regPtr0);

    auto collector = funcExec.withArgs(args).withFilter(filter).execute(
        exFuncNameSendException, std::chrono::seconds(15));

    auto result = collector->getResult();

    if (result == nullptr) {
      ASSERT(false, "echo String : result is nullptr");
    } else {
      for (size_t i = 0; i < result->size(); i++) {
        if (auto uFEPtr =
                std::dynamic_pointer_cast<UserFunctionExecutionException>(
                    result->operator[](i))) {
          LOG_INFO("Done casting to uFEPtr");
          LOG_INFO("Read expected uFEPtr exception %s ",
                   uFEPtr->getMessage().c_str());
        } else {
          FAIL(
              "exFuncNameSendException casting to string for bool argument "
              "exception.");
        }
      }
    }

    LOG("exFuncNameSendException done for bool argument.");

    collector = funcExec.withArgs(arrList).withFilter(filter).execute(
        exFuncNameSendException, std::chrono::seconds(15));
    ASSERT(collector != nullptr, "onRegion collector for arrList nullptr");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    try {
      auto fil = CacheableVector::create();
      fil->push_back(CacheableInt32::create(1));
      auto exe = FunctionService::onRegion(regPtr0);

      LOG_INFO("Executing the exception test it is expected to throw.");
      auto executeFunctionResult3 =
          funcExec.withArgs(arrList)
              .withFilter(filter)
              .execute("ThinClientRegionExceptionTest",
                       std::chrono::seconds(15))
              ->getResult();
      FAIL("Failed to throw expected exception.");
    } catch (...) {
      LOG_INFO("Finished Executing the exception test Successfully");
    }
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

void runFunctionExecution() {
  CALL_TASK(StartLocator1);
  CALL_TASK(StartS12);
  CALL_TASK(StartC1);
  CALL_TASK(Client1OpTest);
  CALL_TASK(StopC1);
  CALL_TASK(CloseServers);
  CALL_TASK(CloseLocator1);
}

DUNIT_MAIN
  { runFunctionExecution(); }
END_MAIN
