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

#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"
#include "testobject/VariousPdxTypes.hpp"

#include <thread>
#include <chrono>

using namespace PdxTests;
/* This is to test
1- funtion execution on pool
 */

#define CLIENT1 s1p1
#define LOCATOR1 s2p1
#define SERVER s2p2

bool isLocalServer = false;
bool isLocator = false;
bool isPoolWithEndpoint = false;

const char* locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
const char* poolRegNames[] = {"partition_region", "PoolRegion2"};

const char* serverGroup = "ServerGroup1";

char* getFuncIName = (char*)"MultiGetFunctionI";
char* putFuncIName = (char*)"MultiPutFunctionI";
char* getFuncName = (char*)"MultiGetFunction";
char* putFuncName = (char*)"MultiPutFunction";
char* rjFuncName = (char*)"RegionOperationsFunction";
char* exFuncName = (char*)"ExceptionHandlingFunction";
char* exFuncNameSendException = (char*)"executeFunction_SendException";
char* exFuncNamePdxType = (char*)"PdxFunctionTest";
char* FEOnRegionPrSHOP = (char*)"FEOnRegionPrSHOP";
char* FEOnRegionPrSHOP_OptimizeForWrite =
    (char*)"FEOnRegionPrSHOP_OptimizeForWrite";
char* FETimeOut = (char*)"FunctionExecutionTimeOut";

#define verifyGetResults()                                                    \
  bool found = false;                                                         \
  for (int j = 0; j < 34; j++) {                                              \
    if (j % 2 == 0) continue;                                                 \
    sprintf(buf, "VALUE--%d", j);                                             \
    if (strcmp(buf, dynCast<CacheableStringPtr>(resultList->operator[](i))    \
                        ->asChar()) == 0) {                                   \
      LOGINFO(                                                                \
          "buf = %s "                                                         \
          "dynCast<CacheableStringPtr>(resultList->operator[](i))->asChar() " \
          "= %s ",                                                            \
          buf,                                                                \
          dynCast<CacheableStringPtr>(resultList->operator[](i))->asChar());  \
      found = true;                                                           \
      break;                                                                  \
    }                                                                         \
  }                                                                           \
  ASSERT(found, "this returned value is invalid");

#define verifyGetKeyResults()                                                 \
  bool found = false;                                                         \
  for (int j = 0; j < 34; j++) {                                              \
    if (j % 2 == 0) continue;                                                 \
    sprintf(buf, "KEY--%d", j);                                               \
    if (strcmp(buf, dynCast<CacheableStringPtr>(resultList->operator[](i))    \
                        ->asChar()) == 0) {                                   \
      LOGINFO(                                                                \
          "buf = %s "                                                         \
          "dynCast<CacheableStringPtr>(resultList->operator[](i))->asChar() " \
          "= %s ",                                                            \
          buf,                                                                \
          dynCast<CacheableStringPtr>(resultList->operator[](i))->asChar());  \
      found = true;                                                           \
      break;                                                                  \
    }                                                                         \
  }                                                                           \
  ASSERT(found, "this returned KEY is invalid");

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
    if (resultItem == nullptr) {
      return;
    }
    if (auto result =
            std::dynamic_pointer_cast<CacheableArrayList>(resultItem)) {
      for (int32_t i = 0; i < result->size(); i++) {
        m_resultList->push_back(result->operator[](i));
      }
    } else {
      auto ex =
          std::dynamic_pointer_cast<UserFunctionExecutionException>(resultItem);
      m_resultList->push_back(ex);
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
typedef std::shared_ptr<MyResultCollector> MyResultCollectorPtr;

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

DUNIT_TASK_DEFINITION(CLIENT1, StartC1)
  {
    // initClient(true);
    initClientWithPool(true, poolRegNames[0], locHostPort, serverGroup, nullptr,
                       0, true, -1, -1, 60000, /*singlehop*/ true,
                       /*threadLocal*/ true);
    // createPool(poolName, locHostPort,serverGroup, nullptr, 0, true );
    // createRegionAndAttachPool(poolRegNames[0],USE_ACK, poolName);

    RegionPtr regPtr0 =
        createRegionAndAttachPool(poolRegNames[0], USE_ACK, nullptr);
    ;  // getHelper()->createRegion( poolRegNames[0], USE_ACK);
    regPtr0->registerAllKeys();

    LOG("Clnt1Init complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, Client1OpTest)
  {
    RegionPtr regPtr0 = getHelper()->getRegion(poolRegNames[0]);
    char buf[128];

    for (int i = 0; i < 34; i++) {
      sprintf(buf, "VALUE--%d", i);
      CacheablePtr value(CacheableString::create(buf));

      sprintf(buf, "KEY--%d", i);
      CacheableKeyPtr key = CacheableKey::create(buf);
      regPtr0->put(key, value);
    }
    std::this_thread::sleep_for(
        std::chrono::seconds(10));  // let the put finish

    //-----------------------Test with sendException
    // onRegion-------------------------------//

    for (int i = 1; i <= 200; i++) {
      CacheablePtr value(CacheableInt32::create(i));

      sprintf(buf, "execKey-%d", i);
      CacheableKeyPtr key = CacheableKey::create(buf);
      regPtr0->put(key, value);
    }
    LOG("Put for execKey's on region complete.");

    LOG("Adding filter");
    CacheableArrayListPtr arrList = CacheableArrayList::create();
    for (int i = 100; i < 120; i++) {
      sprintf(buf, "execKey-%d", i);
      CacheableKeyPtr key = CacheableKey::create(buf);
      arrList->push_back(key);
    }

    CacheableVectorPtr filter = CacheableVector::create();
    for (int i = 100; i < 120; i++) {
      sprintf(buf, "execKey-%d", i);
      CacheableKeyPtr key = CacheableKey::create(buf);
      filter->push_back(key);
    }
    LOG("Adding filter done.");

    CacheablePtr args = CacheableBoolean::create(1);

    ExecutionPtr funcExec = FunctionService::onRegion(regPtr0);
    ASSERT(funcExec != nullptr, "onRegion Returned nullptr");

    ResultCollectorPtr collector =
        funcExec->withArgs(args)->withFilter(filter)->execute(
            exFuncNameSendException, 15);
    ASSERT(collector != nullptr, "onRegion collector nullptr");

    CacheableVectorPtr result = collector->getResult();

    if (result == nullptr) {
      ASSERT(false, "echo String : result is nullptr");
    } else {
      for (int i = 0; i < result->size(); i++) {
        if (auto uFEPtr =
                std::dynamic_pointer_cast<UserFunctionExecutionException>(
                    result->operator[](i))) {
          LOGINFO("Done casting to uFEPtr");
          LOGINFO("Read expected uFEPtr exception %s ",
                  uFEPtr->getMessage()->asChar());
        } else {
          FAIL(
              "exFuncNameSendException casting to string for bool argument "
              "exception.");
        }
      }
    }

    LOG("exFuncNameSendException done for bool argument.");

    collector = funcExec->withArgs(arrList)->withFilter(filter)->execute(
        exFuncNameSendException, 15);
    ASSERT(collector != nullptr, "onRegion collector for arrList nullptr");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    try {
      CacheableVectorPtr fil = CacheableVector::create();
      fil->push_back(CacheableInt32::create(1));
      ExecutionPtr exe = FunctionService::onRegion(regPtr0);

      LOGINFO("Executing the exception test it is expected to throw.");
      CacheableVectorPtr executeFunctionResult3 =
          funcExec->withArgs(arrList)
              ->withFilter(filter)
              ->execute("ThinClientRegionExceptionTest", 15)
              ->getResult();
      FAIL("Failed to throw expected exception.");
    } catch (...) {
      LOGINFO("Finished Executing the exception test Successfully");
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
