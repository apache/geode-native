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

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTDURABLEFAILOVER_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTDURABLEFAILOVER_H_

#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"

#include <thread>
#include <chrono>

/* Testing Parameters              Param's Value
Termination :                   Keepalive = true/ false, Client crash
Restart Time:                   Before Timeout / After Timeout
Register Interest               Durable/ Non Durable

Descripton:  There is One server , one feeder and two clients. Both clients
comes up ->
feeder feed -> both clients go down in same way ( keepalive = true/ false ,
crash )->
feeder feed -> Client1 comes up -> Client2 comes up after timeout -> verify ->
Shutdown

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

  void check(const EntryEvent& event) {
    m_ops++;

    auto key = event.getKey();
    std::shared_ptr<CacheableInt32> value = nullptr;
    try {
      value = std::dynamic_pointer_cast<CacheableInt32>(event.getNewValue());
    } catch (Exception&) {
      //  do nothing.
    }

    auto keyPtr = std::dynamic_pointer_cast<CacheableString>(key);
    if (keyPtr != nullptr && value != nullptr) {
      LOG(std::string("Got Key: ") + keyPtr->toString() +
          ", Value: " + std::to_string(value->value()));
    }

    if (value) {
      m_map[key] = value;
    }
  }

 public:
  OperMonitor() : m_ops(0) {}

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
std::shared_ptr<OperMonitor> mon1 = nullptr;
std::shared_ptr<OperMonitor> mon2 = nullptr;

#include "ThinClientDurableInit.hpp"
#include "ThinClientTasks_C2S2.hpp"

/* Total 10 Keys , alternate durable and non-durable */
const char* mixKeys[] = {"Key-1", "D-Key-1", "L-Key", "LD-Key"};
const char* testRegex[] = {"D-Key-.*", "Key-.*"};

void initClientCache(int redundancy, int durableTimeout,
                     std::shared_ptr<OperMonitor>& mon, int sleepDuration = 0,
                     int durableIdx = 0) {
  if (sleepDuration) SLEEP(sleepDuration);

  if (mon == nullptr) {
    mon = std::make_shared<OperMonitor>();
  }

  //  35 sec ack interval to ensure primary clears its Q only
  // after the secondary comes up and is able to receive the QRM
  // otherwise it will get the unacked events from GII causing the
  // client to get 2 extra / replayed events.
  initClientAndRegion(redundancy, durableIdx, std::chrono::seconds(1),
                      std::chrono::seconds(1),
                      std::chrono::seconds(durableTimeout));

  setCacheListener(regionNames[0], mon);

  getHelper()->cachePtr->readyForEvents();

  auto regPtr0 = getHelper()->getRegion(regionNames[0]);

  // for R =1 it will get a redundancy error
  try {
    regPtr0->registerRegex(testRegex[0], true);
  } catch (Exception&) {
    //  do nothing.
  }
  try {
    regPtr0->registerRegex(testRegex[1], false);
  } catch (Exception&) {
    //  do nothing.
  }
}

void feederUpdate(int value) {
  createIntEntry(regionNames[0], mixKeys[0], value);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  createIntEntry(regionNames[0], mixKeys[1], value);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

/* Close Client 1 with option keep alive = true*/
DUNIT_TASK_DEFINITION(CLIENT1, CloseClient1WithKeepAlive)
  {
    // sleep 30 sec to allow clients' periodic acks (1 sec) to go out
    // this is along with the 5 sec sleep after feeder update and
    // tied to the notify-ack-interval setting of 35 sec.
    SLEEP(30000);
    getHelper()->disconnect(true);
    cleanProc();
    LOG("CloseClient1WithKeepAlive complete: Keepalive = True");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, StartServer1)
  {
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_notify_subscription.xml",
                              locatorsG);
    }

    LOG("SERVER started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, StartServer2)
  {
    if (isLocalServer) {
      CacheHelper::initServer(2, "cacheserver_notify_subscription2.xml",
                              locatorsG);
    }

    //  sleep for 3 seconds to allow redundancy monitor to detect new server.
    std::this_thread::sleep_for(std::chrono::seconds(3));
    LOG("SERVER started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(FEEDER, FeederInit)
  {
    initClientWithPool(true, "__TEST_POOL1__", locatorsG, {}, nullptr, 0, true);
    getHelper()->createPooledRegion(regionNames[0], USE_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);
    LOG("FeederInit complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, InitClient1NoRedundancy)
  { initClientCache(0, 300, mon1); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, InitClient1WithRedundancy)
  { initClientCache(1, 300, mon1); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(FEEDER, FeederUpdate1)
  {
    feederUpdate(1);

    //  Wait 5 seconds for events to be removed from ha queues.
    std::this_thread::sleep_for(std::chrono::seconds(5));

    LOG("FeederUpdate1 complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(FEEDER, FeederUpdate2)
  {
    feederUpdate(2);

    //  Wait 5 seconds for events to be removed from ha queues.
    std::this_thread::sleep_for(std::chrono::seconds(5));

    LOG("FeederUpdate2 complete.");
  }
END_TASK_DEFINITION

// R =0 and clientDown, Intermediate events lost.
DUNIT_TASK_DEFINITION(CLIENT1, VerifyClientDownWithEventsLost)
  {
    LOG("Client Verify 1.");
    mon1->validate(2, 2, 1, 1);
  }
END_TASK_DEFINITION

// R =1 and clientDown, Durable events recieved
DUNIT_TASK_DEFINITION(CLIENT1, VerifyClientDownDurableEventsRecieved)
  {
    LOG("Client Verify 2.");
    mon1->validate(2, 3, 2, 1);
  }
END_TASK_DEFINITION

// No clientDown, All events recieved
DUNIT_TASK_DEFINITION(CLIENT1, VeryifyNoClientDownAllEventsReceived)
  {
    LOG("Client Verify 3.");
    mon1->validate(2, 4, 2, 2);
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
    mon1 = nullptr;
    cleanProc();
    LOG("CLIENT1 closed");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer1)
  {
    CacheHelper::closeServer(1);
    //  Wait 2 seconds to allow client failover.
    std::this_thread::sleep_for(std::chrono::seconds(2));
    LOG("SERVER closed");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer2)
  {
    CacheHelper::closeServer(2);
    LOG("SERVER closed");
  }
END_TASK_DEFINITION

void doThinClientDurableFailoverClientClosedNoRedundancy() {
  CALL_TASK(StartLocator);

  CALL_TASK(StartServer1);

  CALL_TASK(FeederInit);

  CALL_TASK(InitClient1NoRedundancy);

  CALL_TASK(StartServer2);

  CALL_TASK(FeederUpdate1);

  CALL_TASK(CloseClient1WithKeepAlive);

  CALL_TASK(CloseServer1);

  CALL_TASK(FeederUpdate2);

  CALL_TASK(InitClient1NoRedundancy);

  CALL_TASK(VerifyClientDownWithEventsLost);

  CALL_TASK(CloseClient1);
  CALL_TASK(CloseFeeder);
  CALL_TASK(CloseServer2);

  CALL_TASK(CloseLocator);
}

void doThinClientDurableFailoverClientNotClosedRedundancy() {
  CALL_TASK(StartLocator);

  CALL_TASK(StartServer1);

  CALL_TASK(FeederInit);

  CALL_TASK(InitClient1WithRedundancy);

  CALL_TASK(StartServer2);

  CALL_TASK(FeederUpdate1);

  CALL_TASK(CloseServer1);

  CALL_TASK(FeederUpdate2);

  CALL_TASK(VeryifyNoClientDownAllEventsReceived);

  CALL_TASK(CloseClient1);
  CALL_TASK(CloseFeeder);
  CALL_TASK(CloseServer2);

  CALL_TASK(CloseLocator);
}

void doThinClientDurableFailoverClientClosedRedundancy() {
  CALL_TASK(StartLocator);

  CALL_TASK(StartServer1);

  CALL_TASK(FeederInit);

  CALL_TASK(InitClient1WithRedundancy);

  CALL_TASK(StartServer2);

  CALL_TASK(FeederUpdate1);

  CALL_TASK(CloseClient1WithKeepAlive);

  CALL_TASK(CloseServer1);

  CALL_TASK(FeederUpdate2);

  CALL_TASK(InitClient1WithRedundancy);

  CALL_TASK(VerifyClientDownDurableEventsRecieved);

  CALL_TASK(CloseClient1);
  CALL_TASK(CloseFeeder);
  CALL_TASK(CloseServer2);

  CALL_TASK(CloseLocator);
}

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTDURABLEFAILOVER_H_
