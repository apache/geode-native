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

#define ROOT_NAME "testThinClientExecuteFunctionPrSHOP"

#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"
#include "testobject/VariousPdxTypes.hpp"
#include <geode/FunctionService.hpp>
#include <geode/DefaultResultCollector.hpp>

using namespace PdxTests;
/* This is to test
1- funtion execution on pool
*/

#define CLIENT1 s1p1
#define LOCATOR1 s2p1
#define SERVER s2p2

bool isLocalServer = false;
bool isLocator = false;

const char* locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
const char* poolRegNames[] = {"partition_region", "PoolRegion2"};

const char* serverGroup = "ServerGroup1";

char* FEOnRegionPrSHOP = (char*)"FEOnRegionPrSHOP";
char* FEOnRegionPrSHOP_OptimizeForWrite =
    (char*)"FEOnRegionPrSHOP_OptimizeForWrite";
char* getFuncName = (char*)"MultiGetFunction";
char* putFuncName = (char*)"MultiPutFunction";
char* putFuncIName = (char*)"MultiPutFunctionI";
char* FETimeOut = (char*)"FunctionExecutionTimeOut";

class MyResultCollector : public DefaultResultCollector {
 public:
  MyResultCollector()
      : m_endResultCount(0), m_addResultCount(0), m_getResultCount(0) {}
  ~MyResultCollector() noexcept {}

  std::shared_ptr<CacheableVector> getResult(
      std::chrono::milliseconds timeout) override {
    m_getResultCount++;
    return DefaultResultCollector::getResult(timeout);
  }

  void addResult(const std::shared_ptr<Cacheable>& resultItem) override {
    m_addResultCount++;
    if (resultItem == nullptr) {
      return;
    }
    if (auto results =
            std::dynamic_pointer_cast<CacheableArrayList>(resultItem)) {
      for (auto& result : *results) {
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

class MyResultCollector2 : public DefaultResultCollector {
 public:
  MyResultCollector2()
      : m_endResultCount(0), m_addResultCount(0), m_getResultCount(0) {}
  ~MyResultCollector2() noexcept {}

  std::shared_ptr<CacheableVector> getResult(std::chrono::milliseconds timeout) override {
    m_getResultCount++;
    return DefaultResultCollector::getResult(timeout);
  }

  void addResult(const std::shared_ptr<Cacheable>& resultItem) override {
    m_addResultCount++;
    DefaultResultCollector::addResult(resultItem);
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
    initClientWithPool(true, poolRegNames[0], locHostPort, serverGroup, nullptr,
                       0, true, -1, -1, 60000, /*singlehop*/ true,
                       /*threadLocal*/ true);

    auto regPtr0 = createRegionAndAttachPool(poolRegNames[0], USE_ACK, nullptr);
    ;
    regPtr0->registerAllKeys();

    LOG("Clnt1Init complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, Client1OpTest2)
  {
    auto regPtr0 = getHelper()->getRegion(poolRegNames[0]);
    char buf[128];

    for (int i = 0; i < 230; i++) {
      sprintf(buf, "VALUE--%d", i);
      auto value = CacheableString::create(buf);
      regPtr0->put(i, value);
    }
    LOG("Put done.");
    try {
      for (int k = 0; k < 210; k++) {
        auto routingObj = CacheableVector::create();
        for (int i = k; i < k + 20; i++) {
          routingObj->push_back(CacheableInt32::create(i));
        }
        LOGINFO("routingObj size = %d ", routingObj->size());
        auto exe = FunctionService::onRegion(regPtr0);
        ASSERT(exe != nullptr, "onRegion Returned nullptr");

        auto resultList = CacheableVector::create();
        LOG("Executing getFuncName function");
        auto executeFunctionResult =
            exe->withFilter(routingObj)
                ->execute(getFuncName, std::chrono::seconds(15))
                ->getResult();
        if (executeFunctionResult == nullptr) {
          ASSERT(false, "executeFunctionResult is nullptr");
        } else {
          sprintf(buf, "result count = %zd", executeFunctionResult->size());
          LOGINFO(buf);
          resultList->clear();
          for (unsigned item = 0;
               item < static_cast<uint32_t>(executeFunctionResult->size());
               item++) {
            auto arrayList = std::dynamic_pointer_cast<CacheableArrayList>(
                executeFunctionResult->operator[](item));
            for (unsigned pos = 0;
                 pos < static_cast<uint32_t>(arrayList->size()); pos++) {
              resultList->push_back(arrayList->operator[](pos));
            }
          }
          sprintf(buf, "get result count = %zd", resultList->size());
          LOGINFO(buf);
          ASSERT(resultList->size() == 40,
                 "get executeFunctionResult count is not 40");
          for (int32_t i = 0; i < resultList->size(); i++) {
            sprintf(buf, "result[%d] is null\n", i);
            ASSERT(resultList->operator[](i) != nullptr, buf);
            // sprintf(buf, "get result[%d]=%s", i,
            //        std::dynamic_pointer_cast<CacheableString>(resultList->operator[](i))
            //            ->value().c_str());
            // LOGINFO(buf);
          }
        }
        LOGINFO("getFuncName done");
        auto myRC = std::make_shared<MyResultCollector>();
        auto executeFunctionResult1 = exe->withFilter(routingObj)
                                          ->withCollector(myRC)
                                          ->execute(getFuncName)
                                          ->getResult();
        LOGINFO("add result count = %d", myRC->getAddResultCount());
        LOGINFO("end result count = %d", myRC->getEndResultCount());
        LOGINFO("get result count = %d", myRC->getGetResultCount());
        ASSERT(4 == myRC->getAddResultCount(), "add result count is not 4");
        ASSERT(1 == myRC->getEndResultCount(), "end result count is not 1");
        ASSERT(1 == myRC->getGetResultCount(), "get result count is not 1");
        if (executeFunctionResult == nullptr) {
          ASSERT(false, "region get new collector: result is nullptr");
        } else {
          sprintf(buf, "result count = %zd", executeFunctionResult->size());
          LOGINFO(buf);
          resultList->clear();
          for (unsigned item = 0;
               item < static_cast<uint32_t>(executeFunctionResult->size());
               item++) {
            auto arrayList = std::dynamic_pointer_cast<CacheableArrayList>(
                executeFunctionResult->operator[](item));
            for (unsigned pos = 0;
                 pos < static_cast<uint32_t>(arrayList->size()); pos++) {
              resultList->push_back(arrayList->operator[](pos));
            }
          }
          sprintf(buf, "get result count = %zd", resultList->size());
          LOGINFO(buf);
          ASSERT(resultList->size() == 40,
                 "get executeFunctionResult count is not 40");
          for (int32_t i = 0; i < resultList->size(); i++) {
            sprintf(buf, "result[%d] is null\n", i);
            ASSERT(resultList->operator[](i) != nullptr, buf);
            // sprintf(buf, "get result[%d]=%s", i,
            //        std::dynamic_pointer_cast<CacheableString>(
            //            resultList->operator[](i))
            //            ->value().c_str());
            // LOGINFO(buf);
          }
        }
        LOGINFO("getFuncName MyResultCollector done");

        auto executeFunctionResult2 =
            exe->withFilter(routingObj)->execute(FEOnRegionPrSHOP)->getResult();
        if (executeFunctionResult2 == nullptr) {
          ASSERT(false, "executeFunctionResult2 is nullptr");
        } else {
          sprintf(buf, "result count = %zd", executeFunctionResult2->size());
          LOG(buf);
          ASSERT(2 == executeFunctionResult2->size(),
                 "executeFunctionResult2 size is not 2");
          for (int i = 0; i < executeFunctionResult2->size(); i++) {
            bool b = std::dynamic_pointer_cast<CacheableBoolean>(
                         executeFunctionResult2->operator[](i))
                         ->value();
            LOG(b == true ? "true" : "false");
            ASSERT(b == true, "true is not eched back");
          }
        }
        LOGINFO("FEOnRegionPrSHOP withFilter done");

        ///////////////////////// Now same with ResultCollector
        ////////////////////////////

        std::shared_ptr<MyResultCollector2> myRC2(new MyResultCollector2());
        auto executeFunctionResult21 = exe->withFilter(routingObj)
                                           ->withCollector(myRC2)
                                           ->execute(FEOnRegionPrSHOP)
                                           ->getResult();
        LOGINFO("add result count = %d", myRC2->getAddResultCount());
        LOGINFO("end result count = %d", myRC2->getEndResultCount());
        LOGINFO("get result count = %d", myRC2->getGetResultCount());
        ASSERT(2 == myRC2->getAddResultCount(), "add result count is not 2");
        ASSERT(1 == myRC2->getEndResultCount(), "end result count is not 1");
        ASSERT(1 == myRC2->getGetResultCount(), "get result count is not 1");
        if (executeFunctionResult21 == nullptr) {
          ASSERT(false, "executeFunctionResult21 is nullptr");
        } else {
          sprintf(buf, "result count = %zd", executeFunctionResult21->size());
          LOG(buf);
          ASSERT(2 == executeFunctionResult21->size(),
                 "executeFunctionResult21 size is not 2");
          for (int i = 0; i < executeFunctionResult21->size(); i++) {
            bool b = std::dynamic_pointer_cast<CacheableBoolean>(
                         executeFunctionResult21->operator[](i))
                         ->value();
            LOG(b == true ? "true" : "false");
            ASSERT(b == true, "true is not eched back");
          }
        }
        LOGINFO("FEOnRegionPrSHOP done with ResultCollector withFilter");

        /////////////////////// Done with ResultCollector
        ////////////////////////////////

        auto executeFunctionResult3 =
            exe->withFilter(routingObj)
                ->execute(FEOnRegionPrSHOP_OptimizeForWrite)
                ->getResult();
        if (executeFunctionResult3 == nullptr) {
          ASSERT(false, "executeFunctionResult3 is nullptr");
        } else {
          sprintf(buf, "result count = %zd", executeFunctionResult3->size());
          LOG(buf);
          ASSERT(3 == executeFunctionResult3->size(),
                 "executeFunctionResult3->size() is not 3");
          for (int i = 0; i < executeFunctionResult3->size(); i++) {
            bool b = std::dynamic_pointer_cast<CacheableBoolean>(
                         executeFunctionResult3->operator[](i))
                         ->value();
            LOG(b == true ? "true" : "false");
            ASSERT(b == true, "true is not eched back");
          }
        }
        LOGINFO("FEOnRegionPrSHOP_OptimizeForWrite withFilter done");

        ///////////////////////// Now same with ResultCollector
        ////////////////////////////
        std::shared_ptr<MyResultCollector2> myRC3(new MyResultCollector2());
        auto executeFunctionResult31 =
            exe->withFilter(routingObj)
                ->withCollector(myRC3)
                ->execute(FEOnRegionPrSHOP_OptimizeForWrite)
                ->getResult();
        LOGINFO("add result count = %d", myRC3->getAddResultCount());
        LOGINFO("end result count = %d", myRC3->getEndResultCount());
        LOGINFO("get result count = %d", myRC3->getGetResultCount());
        ASSERT(3 == myRC3->getAddResultCount(), "add result count is not 3");
        ASSERT(1 == myRC3->getEndResultCount(), "end result count is not 1");
        ASSERT(1 == myRC3->getGetResultCount(), "get result count is not 1");
        if (executeFunctionResult31 == nullptr) {
          ASSERT(false, "executeFunctionResult31 is nullptr");
        } else {
          sprintf(buf, "result count = %zd", executeFunctionResult31->size());
          LOG(buf);
          ASSERT(3 == executeFunctionResult31->size(),
                 "executeFunctionResult31->size() is not 3");
          for (int i = 0; i < executeFunctionResult31->size(); i++) {
            bool b = std::dynamic_pointer_cast<CacheableBoolean>(
                         executeFunctionResult31->operator[](i))
                         ->value();
            LOG(b == true ? "true" : "false");
            ASSERT(b == true, "true is not eched back");
          }
        }
        LOGINFO(
            "FEOnRegionPrSHOP_OptimizeForWrite done with ResultCollector "
            "withFilter");
      }

     auto exc = FunctionService::onRegion(regPtr0);
     ASSERT(exc != nullptr, "onRegion Returned nullptr");
     // Now w/o filter, chk for singlehop
     auto executeFunctionResult2 = exc->execute(FEOnRegionPrSHOP)->getResult();
     if (executeFunctionResult2 == nullptr) {
       ASSERT(false, "executeFunctionResult2 is nullptr");
      } else {
        sprintf(buf, "result count = %zd", executeFunctionResult2->size());
        LOG(buf);
        ASSERT(2 == executeFunctionResult2->size(),
               "executeFunctionResult2 size is not 2");
        for (int i = 0; i < executeFunctionResult2->size(); i++) {
          bool b = std::dynamic_pointer_cast<CacheableBoolean>(
                       executeFunctionResult2->operator[](i))
                       ->value();
          LOG(b == true ? "true" : "false");
          ASSERT(b == true, "true is not eched back");
        }
      }
      executeFunctionResult2->clear();
      LOGINFO("FEOnRegionPrSHOP without Filter done");

      // Now w/o filter chk single hop
      std::shared_ptr<MyResultCollector2> resultCollector(new MyResultCollector2());
      auto executeFunctionResult21 = exc->withCollector(resultCollector)
                                         ->execute(FEOnRegionPrSHOP)
                                         ->getResult();
      LOGINFO("add result count = %d", resultCollector->getAddResultCount());
      LOGINFO("end result count = %d", resultCollector->getEndResultCount());
      LOGINFO("get result count = %d", resultCollector->getGetResultCount());
      ASSERT(2 == resultCollector->getAddResultCount(),
             "add result count is not 2");
      ASSERT(1 == resultCollector->getEndResultCount(),
             "end result count is not 1");
      ASSERT(1 == resultCollector->getGetResultCount(),
             "get result count is not 1");
      if (executeFunctionResult21 == nullptr) {
        ASSERT(false, "executeFunctionResult21 is nullptr");
      } else {
        sprintf(buf, "result count = %zd", executeFunctionResult21->size());
        LOG(buf);
        ASSERT(2 == executeFunctionResult21->size(),
               "executeFunctionResult21 size is not 2");
        for (int i = 0; i < executeFunctionResult21->size(); i++) {
          bool b = std::dynamic_pointer_cast<CacheableBoolean>(
                       executeFunctionResult21->operator[](i))
                       ->value();
          LOG(b == true ? "true" : "false");
          ASSERT(b == true, "true is not eched back");
        }
      }
      LOGINFO("FEOnRegionPrSHOP done with ResultCollector without filter");

      // Now w/o filter chk for singleHop
      std::shared_ptr<MyResultCollector2> rC(new MyResultCollector2());
      auto executeFunctionResult31 =
          exc->withCollector(rC)
              ->execute(FEOnRegionPrSHOP_OptimizeForWrite)
              ->getResult();
      LOGINFO("add result count = %d", rC->getAddResultCount());
      LOGINFO("end result count = %d", rC->getEndResultCount());
      LOGINFO("get result count = %d", rC->getGetResultCount());
      ASSERT(3 == rC->getAddResultCount(), "add result count is not 3");
      ASSERT(1 == rC->getEndResultCount(), "end result count is not 1");
      ASSERT(1 == rC->getGetResultCount(), "get result count is not 1");
      if (executeFunctionResult31 == nullptr) {
        ASSERT(false, "executeFunctionResult31 is nullptr");
      } else {
        sprintf(buf, "result count = %zd", executeFunctionResult31->size());
        LOG(buf);
        ASSERT(3 == executeFunctionResult31->size(),
               "executeFunctionResult31->size() is not 3");
        for (int i = 0; i < executeFunctionResult31->size(); i++) {
          bool b = std::dynamic_pointer_cast<CacheableBoolean>(
                       executeFunctionResult31->operator[](i))
                       ->value();
          LOG(b == true ? "true" : "false");
          ASSERT(b == true, "true is not eched back");
        }
      }
      LOGINFO(
          "FEOnRegionPrSHOP_OptimizeForWrite done with ResultCollector without "
          "Filter.");

      // Now w/o filter chk for singleHop
     auto functionResult =
          exc->execute(FEOnRegionPrSHOP_OptimizeForWrite)->getResult();
     if (functionResult == nullptr) {
       ASSERT(false, "functionResult is nullptr");
      } else {
        sprintf(buf, "result count = %zd", functionResult->size());
        LOG(buf);
        ASSERT(3 == functionResult->size(), "FunctionResult->size() is not 3");
        for (int i = 0; i < functionResult->size(); i++) {
          bool b = std::dynamic_pointer_cast<CacheableBoolean>(
                       functionResult->operator[](i))
                       ->value();
          LOG(b == true ? "true" : "false");
          ASSERT(b == true, "true is not eched back");
        }
      }
      LOGINFO("FEOnRegionPrSHOP_OptimizeForWrite without Filter done");
      /////////////////////// Done with ResultCollector
      ////////////////////////////////

      char KeyStr[256] = {0};
      char valStr[256] = {0};
     auto fil = CacheableVector::create();
      for (int i = 0; i < 500; i++) {
        ACE_OS::snprintf(KeyStr, 256, "KEY--%d ", i);
        ACE_OS::snprintf(valStr, 256, "VALUE--%d ", i);
        auto keyport = CacheableString::create(KeyStr);
        auto valport = CacheableString::create(valStr);
        regPtr0->put(keyport, valport);
        fil->push_back(CacheableString::create(KeyStr));
      }
      LOGINFO("Put on region complete ");
      LOGINFO("filter count= {0}.", fil->size());

      // Fire N Forget with filter keys
      exc->withFilter(fil)->execute(putFuncName);
      SLEEP(4000);
      LOGINFO(
          "Executing ExecuteFunctionOnRegion on region for execKeys for "
          "arrList "
          "arguement done.");
      for (int i = 0; i < fil->size(); i++) {
        auto str = std::dynamic_pointer_cast<CacheableString>(fil->at(i));
        auto val =
            std::dynamic_pointer_cast<CacheableString>(regPtr0->get(str));
        LOGINFO("Filter Key = %s , get Value = %s ", str->value().c_str(),
                val->value().c_str());
        if (strcmp(str->value().c_str(), val->value().c_str()) != 0) {
          ASSERT(false, "Value after function execution is incorrect");
        }
      }

      // Fire N Forget without filter keys
     auto arrList = CacheableArrayList::create();
      for (int i = 10; i < 200; i++) {
        ACE_OS::snprintf(KeyStr, 256, "KEY--%d ", i);
        arrList->push_back(CacheableString::create(KeyStr));
      }
     auto ex = FunctionService::onRegion(regPtr0);
      ex->withArgs(arrList)->execute(putFuncIName);
      LOGINFO(
          "Executing ExecuteFunctionOnRegion on region for execKeys for "
          "arrList "
          "arguement done.");
      SLEEP(4000);
      for (int i = 0; i < arrList->size(); i++) {
        auto str = std::dynamic_pointer_cast<CacheableString>(arrList->at(i));
        auto val =
            std::dynamic_pointer_cast<CacheableString>(regPtr0->get(str));
        LOGINFO("Filter Key = %s ", str->value().c_str());
        LOGINFO("get Value = %s ", val->value().c_str());
        if (strcmp(str->value().c_str(), val->value().c_str()) != 0) {
          ASSERT(false, "Value after function execution is incorrect");
        }
      }

      ///////////////////TimeOut test //////////////////////
      LOGINFO("FETimeOut begin onRegion");
      auto timeout = std::chrono::milliseconds(15000);
      auto RexecutionPtr = FunctionService::onRegion(regPtr0);
      auto fe = RexecutionPtr->withArgs(CacheableInt32::create(timeout.count()))
                    ->execute(FETimeOut, timeout)
                    ->getResult();
      if (fe == nullptr) {
        ASSERT(false, "functionResult is nullptr");
      } else {
        sprintf(buf, "result count = %zd", fe->size());
        LOG(buf);
        ASSERT(2 == fe->size(), "FunctionResult->size() is not 2");
        for (int i = 0; i < fe->size(); i++) {
          bool b =
              std::dynamic_pointer_cast<CacheableBoolean>(fe->operator[](i))
                  ->value();
          LOG(b == true ? "true" : "false");
          ASSERT(b == true, "true is not eched back");
        }
      }
      LOGINFO("FETimeOut done onRegion");
    } catch (const Exception& excp) {
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
    LOG("Clnt1Down complete: ");
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
  CALL_TASK(StartS13);
  CALL_TASK(StartC1);
  CALL_TASK(Client1OpTest2);
  CALL_TASK(StopC1);
  CALL_TASK(CloseServers);
  CALL_TASK(CloseLocator1);
}

DUNIT_MAIN
  { runFunctionExecution(); }
END_MAIN
