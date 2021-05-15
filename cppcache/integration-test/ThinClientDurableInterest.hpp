#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTDURABLEINTEREST_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTDURABLEINTEREST_H_

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
/*
 * ThinClientDurableInterest.hpp
 *
 *  Created on: Nov 3, 2008
 *      Author: abhaware
 */

#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"

#include <thread>
#include <chrono>

/* This is to test
1- If client doesn't do explicit registration on reconnect, durable events shud
be recieved.
2- If client do explicit Unregistration on reconnect, no events should be
recieved.
*/

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define FEEDER s2p2

namespace { // NOLINT(google-build-namespaces)

using apache::geode::client::EntryEvent;
using apache::geode::client::HashMapOfCacheable;
using apache::geode::client::RegionEvent;

class OperMonitor : public CacheListener {
  int m_ops;
  HashMapOfCacheable m_map;
  int m_id;

  void check(const EntryEvent& event) {
    m_ops++;

    auto key = event.getKey();
    auto value = std::dynamic_pointer_cast<CacheableInt32>(event.getNewValue());

    char buf[256];
    sprintf(buf,
            "Received event for Cachelistener id =%d with key %s and value %d.",
            m_id, key->toString().c_str(), value->value());
    LOG(buf);

    m_map[key] = value;
  }

 public:
  OperMonitor() : m_ops(0), m_id(-1) {}

  explicit OperMonitor(int id) : m_ops(0), m_id(id) {
    LOG_INFO("Inside OperMonitor %d ", m_id);
  }

  ~OperMonitor() override { m_map.clear(); }

  void validate(size_t keyCount, int eventcount, int durableValue,
                int nonDurableValue) {
    LOG("validate called");
    char buf[256] = {'\0'};

    sprintf(buf, "Expected %zd keys for the region, Actual = %zd", keyCount,
            m_map.size());
    ASSERT(m_map.size() == keyCount, buf);

    sprintf(buf, "Expected %d events for the region, Actual = %d", eventcount,
            m_ops);
    ASSERT(m_ops == eventcount, buf);

    for (const auto& item : m_map) {
      const auto keyPtr =
          std::dynamic_pointer_cast<CacheableString>(item.first);
      const auto valuePtr =
          std::dynamic_pointer_cast<CacheableInt32>(item.second);

      if (keyPtr->toString().find('D') ==
          std::string::npos) { /*Non Durable Key */
        sprintf(buf,
                "Expected final value for nonDurable Keys = %d, Actual = %d",
                nonDurableValue, valuePtr->value());
        ASSERT(valuePtr->value() == nonDurableValue, buf);
      } else { /*Durable Key */
        sprintf(buf, "Expected final value for Durable Keys = %d, Actual = %d",
                durableValue, valuePtr->value());
        ASSERT(valuePtr->value() == durableValue, buf);
      }
    }
  }

  void afterCreate(const EntryEvent& event) override { check(event); }

  void afterUpdate(const EntryEvent& event) override { check(event); }

  void afterRegionInvalidate(const RegionEvent&) override{}

  void afterRegionDestroy(const RegionEvent&) override{}

  void afterRegionLive(const RegionEvent&) override {
    LOG("afterRegionLive called.");
  }
};

const char* mixKeys[] = {"Key-1", "D-Key-1", "Key-2", "D-Key-2"};
const char* testRegex[] = {"D-Key-.*", "Key-.*"};
std::shared_ptr<OperMonitor> mon1, mon2;

#include "ThinClientDurableInit.hpp"
#include "ThinClientTasks_C2S2.hpp"

void setCacheListener(const char* regName,
                      std::shared_ptr<OperMonitor> monitor) {
  LOG_INFO("setCacheListener to %s ", regName);
  auto reg = getHelper()->getRegion(regName);
  auto attrMutator = reg->getAttributesMutator();
  attrMutator->setCacheListener(monitor);
}

void initClientWithIntrest(int ClientIdx, std::shared_ptr<OperMonitor>& mon) {
  if (mon == nullptr) {
    mon = std::make_shared<OperMonitor>();
  }

  initClientAndRegion(0, ClientIdx);

  setCacheListener(regionNames[0], mon);

  try {
    getHelper()->cachePtr->readyForEvents();
  } catch (...) {
    LOG("Exception occured while sending readyForEvents");
  }

  auto regPtr0 = getHelper()->getRegion(regionNames[0]);
  regPtr0->registerRegex(testRegex[0], true);
  regPtr0->registerRegex(testRegex[1], false);
}

void initClientWithIntrest2(int ClientIdx,
                            std::shared_ptr<OperMonitor>& monitor1,
                            std::shared_ptr<OperMonitor>& monitor2) {
  initClientAndTwoRegionsAndTwoPools(0, ClientIdx, std::chrono::seconds(60));
  if (monitor1 == nullptr) {
    monitor1 = std::make_shared<OperMonitor>(1);
  }
  if (monitor2 == nullptr) {
    monitor2 = std::make_shared<OperMonitor>(2);
  }
  setCacheListener(regionNames[0], monitor1);
  setCacheListener(regionNames[1], monitor2);
}

void initClientNoIntrest(int ClientIdx, std::shared_ptr<OperMonitor> mon) {
  initClientAndRegion(0, ClientIdx);

  setCacheListener(regionNames[0], mon);

  getHelper()->cachePtr->readyForEvents();
}

void initClientRemoveIntrest(int ClientIdx, std::shared_ptr<OperMonitor> mon) {
  initClientAndRegion(0, ClientIdx);
  setCacheListener(regionNames[0], mon);
  getHelper()->cachePtr->readyForEvents();

  // Only unregister durable Intrest.
  auto regPtr0 = getHelper()->getRegion(regionNames[0]);
  regPtr0->registerRegex(testRegex[0], true);
  regPtr0->unregisterRegex(testRegex[0]);
}

void feederUpdate(int value) {
  createIntEntry(regionNames[0], mixKeys[0], value);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  createIntEntry(regionNames[0], mixKeys[1], value);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void feederUpdate1(int value) {
  createIntEntry(regionNames[0], mixKeys[0], value);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  createIntEntry(regionNames[0], mixKeys[1], value);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  createIntEntry(regionNames[1], mixKeys[2], value);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  createIntEntry(regionNames[1], mixKeys[3], value);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

DUNIT_TASK_DEFINITION(FEEDER, FeederInit)
  {
    initClientWithPool(true, "__TEST_POOL1__", locatorsG, {}, nullptr, 0,
                       true);
    getHelper()->createPooledRegion(regionNames[0], USE_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);
    getHelper()->createPooledRegion(regionNames[1], NO_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);
    LOG("FeederInit complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, Clnt1Init)
  {
    initClientWithIntrest(0, mon1);
    LOG("Clnt1Init complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, Clnt12Init)
  {
    initClientWithIntrest2(0, mon1, mon2);
    LOG("Clnt12Init complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, Clnt2Init)
  {
    initClientWithIntrest(1, mon2);
    LOG("Clnt2Init complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(FEEDER, FeederUpdate1)
  {
    feederUpdate(1);
    // Time to confirm that events has been dispatched.
    SLEEP(100);
    LOG("FeederUpdate1 complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(FEEDER, FeederUpdate12)
  {
    feederUpdate1(1);
    // Time to confirm that events has been dispatched.
    SLEEP(100);
    LOG("FeederUpdate1 complete.");
  }
END_TASK_DEFINITION

/* Close Client 1 with option keep alive = true*/
DUNIT_TASK_DEFINITION(CLIENT1, Clnt1Down)
  {
    // sleep 10 sec to allow periodic ack (1 sec) to go out
    SLEEP(10000);
    getHelper()->disconnect(true);
    cleanProc();
    LOG("Clnt1Down complete: Keepalive = True");
  }
END_TASK_DEFINITION

/* Close Client 2 with option keep alive = true*/
DUNIT_TASK_DEFINITION(CLIENT2, Clnt2Down)
  {
    getHelper()->disconnect(true);
    cleanProc();
    LOG("Clnt2Down complete: Keepalive = True");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, Clnt1Up)
  {
    // No RegisterIntrest again
    initClientNoIntrest(0, mon1);
    LOG("Clnt1Up complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, Clnt2Up)
  {
    initClientRemoveIntrest(1, mon2);
    LOG("Clnt2Up complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(FEEDER, FeederUpdate2)
  {
    feederUpdate(2);
    // Time to confirm that events has been dispatched.
    SLEEP(100);
    LOG("FeederUpdate complete.");
  }
END_TASK_DEFINITION

/* For No register again */
DUNIT_TASK_DEFINITION(CLIENT1, ValidateClient1ListenerEventPayloads)
  {
    // Only durable should be get
    mon1->validate(2, 3, 2, 1);
    LOG("Client 1 Verify.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, ValidateClient2ListenerEventPayloads)
  {
    // no 2nd feed
    mon2->validate(2, 2, 1, 1);
    LOG("Client 2 Verify.");
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

DUNIT_TASK_DEFINITION(CLIENT1, CloseClient12)
  {
    mon1 = nullptr;
    mon2 = nullptr;
    cleanProc();
    LOG("CLIENT12 closed");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CloseClient2)
  {
    mon2 = nullptr;
    cleanProc();
    LOG("CLIENT2 closed");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer)
  {
    CacheHelper::closeServer(1);
    CacheHelper::closeServer(2);
    LOG("SERVER closed");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, closeServer)
  {
    CacheHelper::closeServer(1);
    LOG("SERVER closed");
  }
END_TASK_DEFINITION

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTDURABLEINTEREST_H_
