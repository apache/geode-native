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

#include <thread>
#include <chrono>

/* Testing Parameters              Param's Value
Server Conflation:                   on / off
Client side conflation setting   on/ off / server / not set

Descripton:  This is to test queue conflation property set by client. Client
setting should overwrite
server's bahaviour and accordingly events in server queue should be conflated.
Server side two
 regions have different conflation settings.

*/

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define FEEDER s2p2

using apache::geode::client::EntryEvent;

class OperMonitor : public CacheListener {
  int m_events;
  int m_value;

  void check(const EntryEvent &event) {
    char buf[256] = {'\0'};
    m_events++;
    auto keyPtr = std::dynamic_pointer_cast<CacheableString>(event.getKey());
    auto valuePtr =
        std::dynamic_pointer_cast<CacheableInt32>(event.getNewValue());

    if (valuePtr != nullptr) {
      m_value = valuePtr->value();
    }
    auto msg = std::string("Key = ") + keyPtr->toString() +
               ", Value = " + std::to_string(valuePtr->value());
    LOG(msg);
  }

 public:
  OperMonitor() : m_events(0), m_value(0) {}
  ~OperMonitor() noexcept override = default;

  void afterCreate(const EntryEvent &event) override { check(event); }

  void afterUpdate(const EntryEvent &event) override { check(event); }

  void validate(bool conflation) {
    LOG("validate called");

    if (conflation) {
      auto msg = std::string("Conflation On: Expected events = 2, Actual = ") +
                 std::to_string(m_events);
      ASSERT(m_events == 2, msg);
    } else {
      auto msg = std::string("Conflation Off: Expected events = 5, Actual = ") +
                 std::to_string(m_events);
      ASSERT(m_events == 5, msg);
    }
    auto msg =
        std::string("Expected Value = 5, Actual = ") + std::to_string(m_value);
    ASSERT(m_value == 5, msg);
  }
};

void setCacheListener(const char *regName,
                      std::shared_ptr<OperMonitor> monitor) {
  auto reg = getHelper()->getRegion(regName);
  auto attrMutator = reg->getAttributesMutator();
  attrMutator->setCacheListener(monitor);
}
std::shared_ptr<OperMonitor> mon1C1 = nullptr;
std::shared_ptr<OperMonitor> mon2C1 = nullptr;
std::shared_ptr<OperMonitor> mon1C2 = nullptr;
std::shared_ptr<OperMonitor> mon2C2 = nullptr;

const char *regions[] = {"ConflatedRegion", "NonConflatedRegion"};

#include "ThinClientDurableInit.hpp"
#include "ThinClientTasks_C2S2.hpp"
#include "LocatorHelper.hpp"

void initClientCache(std::shared_ptr<OperMonitor> &mon1,
                     std::shared_ptr<OperMonitor> &mon2, int durableIdx,
                     const char *conflation) {
  initClientAndTwoRegions(durableIdx, 0, std::chrono::seconds(300), conflation,
                          regions);

  // Recreate listener
  mon1 = std::make_shared<OperMonitor>();
  mon2 = std::make_shared<OperMonitor>();

  setCacheListener(regions[0], mon1);
  setCacheListener(regions[1], mon2);
  auto regPtr0 = getHelper()->getRegion(regions[0]);
  auto regPtr1 = getHelper()->getRegion(regions[1]);

  regPtr0->registerAllKeys(true);
  regPtr1->registerAllKeys(true);

  LOG("ClntInit complete.");
}

void feederUpdate(int keyIdx) {
  createIntEntry(regions[0], keys[keyIdx], 1);
  createIntEntry(regions[0], keys[keyIdx], 2);
  createIntEntry(regions[0], keys[keyIdx], 3);
  createIntEntry(regions[0], keys[keyIdx], 4);
  createIntEntry(regions[0], keys[keyIdx], 5);

  createIntEntry(regions[1], keys[keyIdx], 1);
  createIntEntry(regions[1], keys[keyIdx], 2);
  createIntEntry(regions[1], keys[keyIdx], 3);
  createIntEntry(regions[1], keys[keyIdx], 4);
  createIntEntry(regions[1], keys[keyIdx], 5);
}

void closeClient() {
  getHelper()->disconnect(false);
  cleanProc();
  LOG("Client Closed: Keepalive = False");
}

DUNIT_TASK_DEFINITION(SERVER1, StartServer)
  {
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_conflation.xml", locatorsG);
    }
    LOG("SERVER started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, InitializeClient1WithConflation)
  {
    LOG("ClntUp_start");
    initClientCache(mon1C1, mon2C1, 0, "true");
    LOG("InitializeClient1WithConflation complete");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, InitializeClient2WithoutConflation)
  {
    initClientCache(mon1C2, mon2C2, 1, "false");
    LOG("InitializeClient2WithoutConflation complete");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(FEEDER, CreateRegionsAndFirstFeederUpdate)
  {
    initClientWithPool(true, "__TEST_POOL1__", locatorsG, nullptr, nullptr, 0,
                       true);
    getHelper()->createPooledRegion(regions[0], USE_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);
    getHelper()->createPooledRegion(regions[1], USE_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);
    feederUpdate(0);
    LOG("CreateRegionsAndFirstFeederUpdate complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, ValidateClient1Conflation)
  {
    // Client Already Initiated , Send Client Ready and wait
    getHelper()->cachePtr->readyForEvents();
    std::this_thread::sleep_for(std::chrono::seconds(5));

    mon1C1->validate(true);
    LOG("Client 1 region 1 verified for conflation = true");
    mon2C1->validate(true);
    LOG("Client 1 region 2 verified for conflation = true");
    closeClient();
    LOG("ValidateClient1Conflation complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, ValidateClient2Conflation)
  {
    // Client Already Initiated , Send Client Ready and wait
    getHelper()->cachePtr->readyForEvents();
    std::this_thread::sleep_for(std::chrono::seconds(5));

    mon1C2->validate(false);
    LOG("Client 2 region 1 verified for conflation = false");
    mon2C2->validate(false);
    LOG("Client 2 region 2 verified for conflation = false");
    closeClient();
    LOG("ValidateClient2Conflation complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, InitializeClient1WithServer)
  {
    initClientCache(mon1C1, mon2C1, 0, "server");
    LOG("InitializeClient1WithServer complete");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, InitializeClient2WithNone)
  {
    initClientCache(mon1C2, mon2C2, 1, nullptr);
    LOG("InitializeClient2WithNone complete");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(FEEDER, SecondFeederUpdate)
  {
    feederUpdate(1);
    LOG("SecondFeederUpdate complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, ValidateClient1Server)
  {
    // Client Already Initiated , Send Client Ready and wait
    getHelper()->cachePtr->readyForEvents();
    std::this_thread::sleep_for(std::chrono::seconds(5));

    mon1C1->validate(true);
    LOG("Client 1 region 1 verified for conflation = server");
    mon2C1->validate(false);
    LOG("Client 1 region 2 verified for conflation = server");
    closeClient();
    LOG("ValidateClient1Server complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, ValidateClient2None)
  {
    // Client Already Initiated , Send Client Ready and wait
    getHelper()->cachePtr->readyForEvents();
    std::this_thread::sleep_for(std::chrono::seconds(5));

    mon1C2->validate(true);
    LOG("Client 2 region 1 verified for no conflation setting");
    mon2C2->validate(false);
    LOG("Client 2 region 2 verified for no conflation setting");
    closeClient();
    LOG("Client 2 ValidateClient2None.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(FEEDER, CloseFeeder)
  {
    cleanProc();
    LOG("FEEDER closed");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer)
  {
    CacheHelper::closeServer(1);
    LOG("SERVER closed");
  }
END_TASK_DEFINITION

DUNIT_MAIN
  {
    CALL_TASK(CreateLocator1);
    CALL_TASK(StartServer);

    CALL_TASK(InitializeClient1WithConflation);
    CALL_TASK(InitializeClient2WithoutConflation);

    CALL_TASK(CreateRegionsAndFirstFeederUpdate);

    CALL_TASK(ValidateClient1Conflation);
    CALL_TASK(ValidateClient2Conflation);

    CALL_TASK(InitializeClient1WithServer);
    CALL_TASK(InitializeClient2WithNone);

    CALL_TASK(SecondFeederUpdate);

    CALL_TASK(ValidateClient1Server);
    CALL_TASK(ValidateClient2None);

    CALL_TASK(CloseFeeder);
    CALL_TASK(CloseServer);
    CALL_TASK(CloseLocator1);

    closeLocator();
  }
END_MAIN
