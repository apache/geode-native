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

#define ROOT_NAME "TestThinClientHeapLRU"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "fw_dunit.hpp"
#include "BuiltinCacheableWrappers.hpp"

#include <string>
#include "ThinClientHelper.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1

#include "locator_globals.hpp"
#include "LocatorHelper.hpp"

using apache::geode::client::internal::DSCode;

using apache::geode::client::testing::CacheableWrapper;
using apache::geode::client::testing::CacheableWrapperFactory;

const char *_regionNames[] = {"DistRegionAck"};

void createOnekEntries() {
  CacheableHelper::registerBuiltins();
  auto dataReg = getHelper()->getRegion(_regionNames[0]);
  for (int i = 0; i < 2048; i++) {
    CacheableWrapper *tmpkey =
        CacheableWrapperFactory::createInstance(DSCode::CacheableInt32);
    CacheableWrapper *tmpval =
        CacheableWrapperFactory::createInstance(DSCode::CacheableBytes);
    tmpkey->initKey(i, 32);
    tmpval->initRandomValue(1024);
    ASSERT(tmpkey->getCacheable() != nullptr,
           "tmpkey->getCacheable() is nullptr");
    ASSERT(tmpval->getCacheable() != nullptr,
           "tmpval->getCacheable() is nullptr");
    dataReg->put(
        std::dynamic_pointer_cast<CacheableKey>(tmpkey->getCacheable()),
        tmpval->getCacheable());
    // delete tmpkey;
    //  delete tmpval;
  }
  dunit::sleep(10000);
  auto me = dataReg->entries(false);
  LOG("Verifying size outside loop");
  LOG(std::string("region size is d") + std::to_string(me.size()));

  ASSERT(me.size() <= 1024, "Should have evicted anything over 1024 entries");
}

DUNIT_TASK_DEFINITION(CLIENT1, StepOne)
  {
    auto pp = Properties::create();
    pp->insert("heap-lru-limit", 1);
    pp->insert("heap-lru-delta", 10);
    initClientWithPool(true, "__TEST_POOL1__", locatorsG, nullptr, pp, 0, true);
    getHelper()->createPooledRegion(_regionNames[0], USE_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);
    LOG("StepOne complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo)
  {
    auto pp = Properties::create();
    pp->insert("heap-lru-limit", 1);
    pp->insert("heap-lru-delta", 10);
    initClientWithPool(true, "__TEST_POOL1__", locatorsG, nullptr, pp, 0, true);
    getHelper()->createPooledRegion(_regionNames[0], USE_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);
    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    // verfy that eviction works
    createOnekEntries();
    LOG("StepThree complete.");
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

DUNIT_MAIN
  {
    CALL_TASK(CreateLocator1);
    CALL_TASK(CreateServer1_With_Locator);
    CALL_TASK(StepOne);
    CALL_TASK(StepTwo);
    CALL_TASK(StepThree);
    CALL_TASK(CloseCache1);
    CALL_TASK(CloseCache2);
    CALL_TASK(CloseServer1);
    CALL_TASK(CloseLocator1);
  }
END_MAIN
