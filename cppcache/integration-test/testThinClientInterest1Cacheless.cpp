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

using apache::geode::client::EntryEvent;

class MyListener;

class MyListener : public CacheListener {
  uint8_t m_gotit[5];

 public:
  MyListener() : CacheListener() {
    for (int i = 0; i < 5; i++) m_gotit[i] = 0;
  }
  inline void checkEntry(const EntryEvent &event) {
    auto keyPtr = std::dynamic_pointer_cast<CacheableString>(event.getKey());
    for (int i = 0; i < 5; i++) {
      if (keyPtr->value() == keys[i]) {
        auto valPtr =
            std::dynamic_pointer_cast<CacheableString>(event.getNewValue());
        if (valPtr->value() == vals[i]) {
          m_gotit[i] = 1;
        }
        break;
      }
    }
  }
  void afterCreate(const EntryEvent &event) override { checkEntry(event); }
  void afterUpdate(const EntryEvent &event) override { checkEntry(event); }
  inline bool gotAll() {
    for (int i = 0; i < 5; i++) {
      if (m_gotit[i] == 0) return false;
    }
    return true;
  }
};
std::shared_ptr<MyListener> mylistner = nullptr;

void setCacheListener(const char *regName,
                      std::shared_ptr<MyListener> regListener) {
  auto reg = getHelper()->getRegion(regName);
  auto attrMutator = reg->getAttributesMutator();
  attrMutator->setCacheListener(regListener);
}

DUNIT_TASK(SERVER1, StartServer)
  {
    if (isLocalServer) {
      CacheHelper::initLocator(1);
      CacheHelper::initServer(1, "cacheserver_notify_subscription.xml",
                              locatorsG);
    }
    LOG("SERVER started");
  }
END_TASK(StartServer)

DUNIT_TASK(CLIENT1, SetupClient1)
  {
    initClientWithPool(true, "__TEST_POOL1__", locatorsG, nullptr, nullptr, 0,
                       true);
    getHelper()->createPooledRegion(regionNames[0], false, locatorsG,
                                    "__TEST_POOL1__", true, true);
  }
END_TASK(SetupClient1)

DUNIT_TASK(CLIENT2, setupClient2)
  {
    initClientWithPool(true, "__TEST_POOL1__", locatorsG, nullptr, nullptr, 0,
                       true);
    getHelper()->createPooledRegion(regionNames[0], false, locatorsG,
                                    "__TEST_POOL1__", true, true);
    mylistner = std::make_shared<MyListener>();
    setCacheListener(regionNames[0], mylistner);
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    regPtr->registerAllKeys(false, true);
  }
END_TASK(setupClient2)

DUNIT_TASK(CLIENT1, populateServer)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    for (int i = 0; i < 5; i++) {
      auto keyPtr = CacheableKey::create(keys[i]);
      regPtr->create(keyPtr, vals[i]);
    }
    SLEEP(1000);
  }
END_TASK(populateServer)

DUNIT_TASK(CLIENT2, verify)
  { ASSERT(mylistner->gotAll(), "Did not get all"); }
END_TASK(verify)

DUNIT_TASK(CLIENT1, StopClient1)
  {
    cleanProc();
    LOG("CLIENT1 stopped");
  }
END_TASK(StopClient1)

DUNIT_TASK(CLIENT2, StopClient2)
  {
    cleanProc();
    LOG("CLIENT2 stopped");
  }
END_TASK(StopClient2)

DUNIT_TASK(SERVER1, StopServer)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      CacheHelper::closeLocator(1);
    }
    LOG("SERVER stopped");
  }
END_TASK(StopServer)
