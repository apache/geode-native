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

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1

#include "locator_globals.hpp"
#include "LocatorHelper.hpp"

using apache::geode::client::Cacheable;
using apache::geode::client::CacheableFloat;
using apache::geode::client::CacheableKey;
using apache::geode::client::Exception;

std::shared_ptr<CacheableString> getUString(int index) {
  std::wostringstream strm;
  std::wstring baseStr(40, L'\x20AC');
  strm << baseStr << std::setw(10) << std::setfill(L'0') << index;
  return CacheableString::create(strm.str());
}
std::shared_ptr<CacheableString> getUAString(int index) {
  std::wostringstream strm;
  std::wstring baseStr(40, L'A');
  strm << baseStr << std::setw(10) << std::setfill(L'0') << index;
  return CacheableString::create(strm.str());
}

DUNIT_TASK_DEFINITION(CLIENT1, SetupClient1)
  {
    initClientWithPool(true, "__TEST_POOL1__", locatorsG, nullptr, nullptr, 0,
                       true);
    getHelper()->createPooledRegion(regionNames[0], false, locatorsG,
                                    "__TEST_POOL1__", true, true);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, populateServer)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    for (int i = 0; i < 5; i++) {
      auto keyPtr = CacheableKey::create(keys[i]);
      regPtr->create(keyPtr, vals[i]);
    }
    SLEEP(200);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, setupClient2)
  {
    initClientWithPool(true, "__TEST_POOL1__", locatorsG, nullptr, nullptr, 0,
                       true);
    getHelper()->createPooledRegion(regionNames[0], false, locatorsG,
                                    "__TEST_POOL1__", true, true);
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    regPtr->registerAllKeys(false, true);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, verify)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    for (int i = 0; i < 5; i++) {
      auto keyPtr1 = CacheableKey::create(keys[i]);
      auto msg = std::string("key[") + keys[i] + "] should have been found";
      ASSERT(regPtr->containsKey(keyPtr1), msg);
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, updateKeys)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    for (int index = 0; index < 5; ++index) {
      auto keyPtr = CacheableKey::create(keys[index]);
      regPtr->put(keyPtr, nvals[index]);
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, verifyUpdates)
  {
    SLEEP(2000);
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    for (int index = 0; index < 5; ++index) {
      auto keyPtr = CacheableKey::create(keys[index]);
      auto msg = std::string("key[") + keys[index] + "] should have been found";
      ASSERT(regPtr->containsKey(keyPtr), msg);
      auto val = std::dynamic_pointer_cast<CacheableString>(
          regPtr->getEntry(keyPtr)->getValue());
      ASSERT(strcmp(val->value().c_str(), nvals[index]) == 0,
             "Incorrect value for key");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, PutUnicodeStrings)
  {
    auto reg0 = getHelper()->getRegion(regionNames[0]);
    for (int index = 0; index < 5; ++index) {
      auto key = getUString(index);
      auto val = Cacheable::create(index + 100);
      reg0->put(key, val);
    }
    LOG("PutUnicodeStrings complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, GetUnicodeStrings)
  {
    auto reg0 = getHelper()->getRegion(regionNames[0]);
    for (int index = 0; index < 5; ++index) {
      auto key = getUString(index);
      auto val = std::dynamic_pointer_cast<CacheableInt32>(reg0->get(key));
      ASSERT(val != nullptr, "expected non-null value in get");
      ASSERT(val->value() == (index + 100), "unexpected value in get");
    }
    reg0->unregisterAllKeys();
    std::vector<std::shared_ptr<CacheableKey>> vec;
    for (int index = 0; index < 5; ++index) {
      auto key = getUString(index);
      vec.push_back(key);
    }
    reg0->registerKeys(vec);
    LOG("GetUnicodeStrings complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, UpdateUnicodeStrings)
  {
    auto reg0 = getHelper()->getRegion(regionNames[0]);
    for (int index = 0; index < 5; ++index) {
      auto key = getUString(index);
      auto val = CacheableFloat::create(index + 20.0F);
      reg0->put(key, val);
    }
    LOG("UpdateUnicodeStrings complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CheckUpdateUnicodeStrings)
  {
    SLEEP(2000);
    auto reg0 = getHelper()->getRegion(regionNames[0]);
    for (int index = 0; index < 5; ++index) {
      auto key = getUString(index);
      auto val = std::dynamic_pointer_cast<CacheableFloat>(
          reg0->getEntry(key)->getValue());
      ASSERT(val != nullptr, "expected non-null value in get");
      ASSERT(val->value() == (index + 20.0F), "unexpected value in get");
    }
    LOG("CheckUpdateUnicodeStrings complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, PutASCIIAsWideStrings)
  {
    auto reg0 = getHelper()->getRegion(regionNames[0]);
    for (int index = 0; index < 5; ++index) {
      auto key = getUAString(index);
      auto val = getUString(index + 20);
      reg0->put(key, val);
    }
    LOG("PutASCIIAsWideStrings complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, GetASCIIAsWideStrings)
  {
    auto reg0 = getHelper()->getRegion(regionNames[0]);
    for (int index = 0; index < 5; ++index) {
      auto key = getUAString(index);
      auto val = std::dynamic_pointer_cast<CacheableString>(reg0->get(key));
      auto expectedVal = getUString(index + 20);
      ASSERT(val != nullptr, "expected non-null value in get");
      ASSERT(val->value() == expectedVal->value(), "values aren't equal");
    }
    std::vector<std::shared_ptr<CacheableKey>> vec;
    for (int index = 0; index < 5; ++index) {
      auto key = getUAString(index);
      vec.push_back(key);
    }
    reg0->registerKeys(vec);
    LOG("GetASCIIAsWideStrings complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, UpdateASCIIAsWideStrings)
  {
    auto reg0 = getHelper()->getRegion(regionNames[0]);
    for (int index = 0; index < 5; ++index) {
      auto key = getUAString(index);
      auto val = getUAString(index + 10);
      reg0->put(key, val);
    }
    LOG("UpdateASCIIAsWideStrings complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CheckUpdateASCIIAsWideStrings)
  {
    SLEEP(2000);
    auto reg0 = getHelper()->getRegion(regionNames[0]);
    for (int index = 0; index < 5; ++index) {
      auto key = getUAString(index);
      auto val = std::dynamic_pointer_cast<CacheableString>(reg0->get(key));
      auto expectedVal = getUAString(index + 10);
      ASSERT(val != nullptr, "expected non-null value in get");
      ASSERT(*val == *expectedVal, "unexpected value in get");
    }
    LOG("CheckUpdateASCIIAsWideStrings complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CheckUpdateBug1001)
  {
    try {
      const wchar_t *str = L"Pivotal";
      auto lCStringP = CacheableString::create(str);
      lCStringP->value();
    } catch (const Exception &geodeExcp) {
      std::cout << geodeExcp.getName() << " : " << geodeExcp.what();
      FAIL("Should not have got exception.");
    }

    try {
      const wchar_t *str =
          L"Pivotal\\u7b2c"
          "17"
          "\\u53f7";
      auto lCStringP = CacheableString::create(str);
      lCStringP->value();
    } catch (const Exception &geodeExcp) {
      std::cout << geodeExcp.getName() << " : " << geodeExcp.what();
      FAIL("Should not have got exception.");
    }

    LOG("CheckUpdateBug1001 complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StopClient1)
  {
    cleanProc();
    LOG("CLIENT1 stopped");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StopClient2)
  {
    cleanProc();
    LOG("CLIENT2 stopped");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, StopServer)
  {
    if (isLocalServer) CacheHelper::closeServer(1);
    LOG("SERVER stopped");
  }
END_TASK_DEFINITION

DUNIT_MAIN
  {
    CALL_TASK(CreateLocator1);
    CALL_TASK(CreateServer1_With_Locator);
    CALL_TASK(SetupClient1);
    CALL_TASK(populateServer);
    CALL_TASK(setupClient2);
    CALL_TASK(verify);
    CALL_TASK(updateKeys);
    CALL_TASK(verifyUpdates);
    CALL_TASK(PutUnicodeStrings);
    CALL_TASK(GetUnicodeStrings);
    CALL_TASK(UpdateUnicodeStrings);
    CALL_TASK(CheckUpdateUnicodeStrings);
    CALL_TASK(PutASCIIAsWideStrings);
    CALL_TASK(GetASCIIAsWideStrings);
    CALL_TASK(UpdateASCIIAsWideStrings);
    CALL_TASK(CheckUpdateASCIIAsWideStrings);
    CALL_TASK(CheckUpdateBug1001);
    CALL_TASK(StopClient1);
    CALL_TASK(StopClient2);
    CALL_TASK(StopServer);
    CALL_TASK(CloseLocator1);
  }
END_MAIN
