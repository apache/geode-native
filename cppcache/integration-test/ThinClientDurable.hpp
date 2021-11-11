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

#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTDURABLE_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTDURABLE_H_

#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"

#include <thread>
#include <chrono>
#include <sstream>

/* Testing Parameters              Param's Value
Termination :                   Keepalive = true/ false, Client crash / Netdown
Restart Time:                   Before Timeout / After Timeout
Register Interest               Durable/ Non Durable ,  Regex / List
Intermediate Feeding       true (Region 1 ) / false ( Region 2)

Descripton:  There is One server , one feeder and two clients. Both clients
comes up -> feeder feed in both regions
both clients go down in same way ( keepalive = true/ false etc.  ) -> feeder
feed  ( only in region 1 )
-> Both comes up ->  verify -> Shutdown

Client 1 is with R =0 and Client 2 with R = 1
*/

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define FEEDER s2p2

namespace {  // NOLINT(google-build-namespaces)

using apache::geode::client::EntryEvent;
using apache::geode::client::Exception;
using apache::geode::client::HashMapOfCacheable;
using apache::geode::client::RegionEvent;

class OperMonitor : public CacheListener {
  int m_ops;
  HashMapOfCacheable m_map;
  std::string m_clientName, m_regionName;

  void check(const EntryEvent& event) {
    m_ops++;

    auto key = event.getKey();
    std::shared_ptr<CacheableInt32> value = nullptr;
    try {
      value = std::dynamic_pointer_cast<CacheableInt32>(event.getNewValue());
    } catch (Exception&) {
      //  do nothing.
    }

    char buff[128] = {'\0'};
    std::stringstream strm;
    auto keyPtr = std::dynamic_pointer_cast<CacheableString>(key);
    if (value != nullptr) {
      strm << "Event[" << keyPtr->toString() << ", " << value->value()
           << "] called for " << m_clientName << ":" << m_regionName;
      m_map[key] = value;
    } else {
      strm << "Event Key=" << keyPtr->toString() << "called for "
           << m_clientName << ":" << m_regionName;
    }
    LOG(strm.str());
  }

 public:
  OperMonitor(const char* clientName, const char* regionName)
      : m_ops(0), m_clientName(clientName), m_regionName(regionName) {}

  ~OperMonitor() override { m_map.clear(); }

  void validate(size_t keyCount, int eventcount, int durableValue,
                int nonDurableValue) {
    LOG("validate called");
    std::stringstream strm;

    strm << "Expected " << keyCount
         << " keys for the region, Actual = " << m_map.size();
    ASSERT(m_map.size() == keyCount, strm.str());

    strm.str("");
    strm << "Expected " << eventcount
         << " events for the region, Actual = " << m_ops;
    ASSERT(m_ops == eventcount, strm.str());

    for (const auto& item : m_map) {
      const auto keyPtr =
          std::dynamic_pointer_cast<CacheableString>(item.first);
      const auto valuePtr =
          std::dynamic_pointer_cast<CacheableInt32>(item.second);

      if (keyPtr->toString().find('D') ==
          std::string::npos) { /*Non Durable Key */
        strm.str("");
        strm << "Expected final value for nonDurable Keys = " << nonDurableValue
             << ", Actual = " << valuePtr->value();
        ASSERT(valuePtr->value() == nonDurableValue, strm.str());
      } else { /*Durable Key */
        strm.str("");
        strm << "Expected final value for Durable Keys = " << durableValue
             << ", Actual = " << valuePtr->value();
        ASSERT(valuePtr->value() == durableValue, strm.str());
      }
    }
  }

  void afterCreate(const EntryEvent& event) override {
    LOG("afterCreate called");
    check(event);
  }

  void afterUpdate(const EntryEvent& event) override {
    LOG("afterUpdate called");
    check(event);
  }

  void afterDestroy(const EntryEvent& event) override {
    LOG("afterDestroy called");
    check(event);
  }

  void afterRegionInvalidate(const RegionEvent&) override {}

  void afterRegionDestroy(const RegionEvent&) override {}
};

void setCacheListener(const char* regName,
                      std::shared_ptr<OperMonitor> monitor) {
  auto reg = getHelper()->getRegion(regName);
  auto attrMutator = reg->getAttributesMutator();
  attrMutator->setCacheListener(monitor);
}
std::shared_ptr<OperMonitor> mon1C1 = nullptr;
std::shared_ptr<OperMonitor> mon1C2 = nullptr;

/* Total 10 Keys , alternate durable and non-durable */
const char* mixKeys[] = {"Key-1", "D-Key-1", "L-Key", "LD-Key"};
const char* testRegex[] = {"D-Key-.*", "Key-.*"};

#include "ThinClientDurableInit.hpp"
#include "ThinClientTasks_C2S2.hpp"

void initClientCache(int durableIdx, int redundancy,
                     std::chrono::seconds durableTimeout,
                     std::shared_ptr<OperMonitor>& mon1,
                     int sleepDuration = 0) {
  // Sleep before starting , Used for Timeout testing.
  if (sleepDuration) SLEEP(sleepDuration);

  initClientAndTwoRegions(durableIdx, redundancy, durableTimeout);

  setCacheListener(regionNames[0], mon1);

  getHelper()->cachePtr->readyForEvents();

  auto regPtr0 = getHelper()->getRegion(regionNames[0]);

  // Register Regex in both region.
  regPtr0->registerRegex(testRegex[0], true);
  regPtr0->registerRegex(testRegex[1], false);

  // Register List in both regions
  std::vector<std::shared_ptr<CacheableKey>> v;
  auto ldkey = CacheableKey::create(mixKeys[3]);
  v.push_back(ldkey);
  regPtr0->registerKeys(v, true);
  v.clear();
  auto lkey = CacheableKey::create(mixKeys[2]);
  v.push_back(lkey);
  regPtr0->registerKeys(v);

  LOG("Clnt1Init complete.");
}

void feederUpdate(int value) {
  for (int regIdx = 0; regIdx < 1; regIdx++) {
    createIntEntry(regionNames[regIdx], mixKeys[0], value);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    createIntEntry(regionNames[regIdx], mixKeys[1], value);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    createIntEntry(regionNames[regIdx], mixKeys[2], value);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    createIntEntry(regionNames[regIdx], mixKeys[3], value);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    destroyEntry(regionNames[regIdx], mixKeys[0]);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    destroyEntry(regionNames[regIdx], mixKeys[1]);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    destroyEntry(regionNames[regIdx], mixKeys[2]);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    destroyEntry(regionNames[regIdx], mixKeys[3]);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

DUNIT_TASK_DEFINITION(FEEDER, FeederInit)
  {
    initClientWithPool(true, "__TEST_POOL1__", locatorsG, {}, nullptr, 0, true);
    getHelper()->createPooledRegion(regionNames[0], USE_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);
    LOG("FeederInit complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, InitClient1Timeout300)
  {
    if (mon1C1 == nullptr) {
      mon1C1 = std::make_shared<OperMonitor>(durableIds[0], regionNames[0]);
    }
    initClientCache(0, 0 /* Redundancy */,
                    std::chrono::seconds(300) /* D Timeout */, mon1C1);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, InitClient1Timeout30)
  {
    if (mon1C1 == nullptr) {
      mon1C1 = std::make_shared<OperMonitor>(durableIds[0], regionNames[0]);
    }
    initClientCache(0, 0 /* Redundancy */,
                    std::chrono::seconds(30) /* D Timeout */, mon1C1);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, InitClient1DelayedStart)
  {
    if (mon1C1 == nullptr) {
      mon1C1 = std::make_shared<OperMonitor>(durableIds[0], regionNames[0]);
    }
    initClientCache(0, 0 /* Redundancy */,
                    std::chrono::seconds(30) /* D Timeout */, mon1C1,
                    35000 /* Sleep before starting */);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, InitClient2Timeout300)
  {
    if (mon1C2 == nullptr) {
      mon1C2 = std::make_shared<OperMonitor>(durableIds[1], regionNames[0]);
    }
    initClientCache(1, 1 /* Redundancy */,
                    std::chrono::seconds(300) /* D Timeout */, mon1C2);
  }
END_TASK_DEFINITION

// Client 2 don't need to sleep for timeout as C1 does before it
DUNIT_TASK_DEFINITION(CLIENT2, InitClient2Timeout30)
  {
    if (mon1C2 == nullptr) {
      mon1C2 = std::make_shared<OperMonitor>(durableIds[1], regionNames[0]);
    }
    initClientCache(1, 1 /* Redundancy */,
                    std::chrono::seconds(30) /* D Timeout */, mon1C2);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, ReviveClient1)
  { revive(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, ReviveClient1Delayed)
  {
    SLEEP(35000);
    revive();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, ReviveClient2AndWait)
  {
    revive();
    // Give Time to revive connections.
    SLEEP(15000);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(FEEDER, FeederUpdate1)
  {
    feederUpdate(1);

    //  Wait 5 seconds for events to be removed from ha queues.
    std::this_thread::sleep_for(std::chrono::seconds(5));

    LOG("FeederUpdate1 complete.");
  }
END_TASK_DEFINITION

/* Close Client 1 with option keep alive = true*/
DUNIT_TASK_DEFINITION(CLIENT1, CloseClient1KeepAliveTrue)
  {
    getHelper()->disconnect(true);
    cleanProc();
    LOG("Clnt1Down complete: Keepalive = True");
  }
END_TASK_DEFINITION

/* Close Client 1 with option keep alive = false*/
DUNIT_TASK_DEFINITION(CLIENT1, CloseClient1KeepAliveFalse)
  {
    getHelper()->disconnect();
    cleanProc();
    LOG("Clnt1Down complete: Keepalive = false");
  }
END_TASK_DEFINITION

/* Close Client 1 Abruptly*/
DUNIT_TASK_DEFINITION(CLIENT1, CrashClient1)
  {
    // TODO: fix for pool case
    crashClient();
    getHelper()->disconnect();
    cleanProc();
    LOG("Clnt1Down complete: Crashed");
  }
END_TASK_DEFINITION

/* Disconnect Client 1 (netdown) */
DUNIT_TASK_DEFINITION(CLIENT1, DisconnectClient1)
  {
    // TODO: fix for pool case
    netDown();
    LOG("Clnt1Down complete: Network disconnection has been simulated");
  }
END_TASK_DEFINITION

/* Close Client 2 with option keep alive = true*/
DUNIT_TASK_DEFINITION(CLIENT2, CloseClient2KeepAliveTrue)
  {
    getHelper()->disconnect(true);
    cleanProc();
    LOG("Clnt2Down complete: Keepalive = True");
  }
END_TASK_DEFINITION

/* Close Client 2 with option keep alive = false*/
DUNIT_TASK_DEFINITION(CLIENT2, CloseClient2KeepAliveFalse)
  {
    getHelper()->disconnect();
    cleanProc();
    LOG("Clnt2Down complete: Keepalive = false");
  }
END_TASK_DEFINITION

/* Close Client 2 Abruptly*/
DUNIT_TASK_DEFINITION(CLIENT2, CrashClient2)
  {
    crashClient();
    getHelper()->disconnect();
    cleanProc();
    LOG("Clnt2Down complete: Crashed");
  }
END_TASK_DEFINITION

/* Disconnect Client 2 (netdown) */
DUNIT_TASK_DEFINITION(CLIENT2, DisconnectClient2)
  {
    netDown();
    LOG("Clnt2Down complete: Network disconnection has been simulated");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(FEEDER, FeederUpdate2)
  {
    feederUpdate(2);
    LOG("FeederUpdate2 complete.");
  }
END_TASK_DEFINITION

/* Verify that clients receive feeder update 1  */
DUNIT_TASK_DEFINITION(CLIENT1, VerifyFeederUpdate_1_C1)
  {
    LOG("Client 1 Verify first feeder update.");
    mon1C1->validate(4, 8, 1, 1);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyFeederUpdate_1_C2)
  {
    LOG("Client 2 Verify first feeder udpate.");
    mon1C2->validate(4, 8, 1, 1);
  }
END_TASK_DEFINITION

/* For Keep Alive = True or crash, netdown  */
DUNIT_TASK_DEFINITION(CLIENT1, VerifyClient1)
  {
    LOG("Client 1 Verify.");
    mon1C1->validate(4, 12, 2, 1);
  }
END_TASK_DEFINITION

/* For Keep Alive = false */
DUNIT_TASK_DEFINITION(CLIENT1, VerifyClient1KeepAliveFalse)
  {
    LOG("Client 1 Verify.");
    mon1C1->validate(4, 8, 1, 1);
  }
END_TASK_DEFINITION

/* For Keep Alive = True or crash, netdown  */
DUNIT_TASK_DEFINITION(CLIENT2, VerifyClient2)
  {
    LOG("Client 2 Verify.");
    mon1C2->validate(4, 12, 2, 1);
  }
END_TASK_DEFINITION

/* For Keep Alive = false */
DUNIT_TASK_DEFINITION(CLIENT2, VerifyClient2KeepAliveFalse)
  {
    LOG("Client 2 Verify.");
    mon1C2->validate(4, 8, 1, 1);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(FEEDER, CloseFeeder)
  {
    cleanProc();
    LOG("FEEDER closed");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseClient1)
  {
    mon1C1 = nullptr;
    cleanProc();
    LOG("CLIENT1 closed");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CloseClient2)
  {
    mon1C2 = nullptr;
    cleanProc();
    LOG("CLIENT2 closed");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServers)
  {
    CacheHelper::closeServer(1);
    CacheHelper::closeServer(2);
    LOG("SERVERs closed");
  }
END_TASK_DEFINITION

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTDURABLE_H_
