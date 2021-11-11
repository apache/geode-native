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

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTINTEREST3CACHELESS_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTINTEREST3CACHELESS_H_

#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"
#include "TallyListener.hpp"
#include "TallyWriter.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1

namespace {  // NOLINT(google-build-namespaces)

using apache::geode::client::testing::TallyListener;
using apache::geode::client::testing::TallyWriter;

bool isLocalServer = true;
static bool isLocator = false;
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
#include "LocatorHelper.hpp"
std::shared_ptr<TallyListener> reg1Listener1;
std::shared_ptr<TallyWriter> reg1Writer1;
int numCreates = 0;
int numUpdates = 0;
int numInvalidates = 0;
int numDestroys = 0;

void setCacheListener(const char* regName,
                      std::shared_ptr<TallyListener> regListener) {
  auto reg = getHelper()->getRegion(regName);
  auto attrMutator = reg->getAttributesMutator();
  attrMutator->setCacheListener(regListener);
}

void setCacheWriter(const char* regName,
                    std::shared_ptr<TallyWriter> regWriter) {
  auto reg = getHelper()->getRegion(regName);
  auto attrMutator = reg->getAttributesMutator();
  attrMutator->setCacheWriter(regWriter);
}

void validateEventCount(int line) {
  LOGINFO("ValidateEvents called from line (%d).", line);
  int num = reg1Listener1->getCreates();
  auto msg = std::string("Got wrong number of creation events. expected[") +
             std::to_string(numCreates) + "], real[" + std::to_string(num) +
             "]";
  ASSERT(num == numCreates, msg);
  num = reg1Listener1->getUpdates();
  msg = std::string("Got wrong number of update events. expected[") +
        std::to_string(numUpdates) + "], real[" + std::to_string(num) + "]";
  ASSERT(num == numUpdates, msg);
  num = reg1Writer1->getCreates();
  msg = std::string("Got wrong number of writer events. expected[") +
        std::to_string(numCreates) + "], real[" + std::to_string(num) + "]";
  ASSERT(num == numCreates, msg);
  num = reg1Listener1->getInvalidates();
  msg = std::string("Got wrong number of invalidate events. expected[") +
        std::to_string(numInvalidates) + "], real[" + std::to_string(num) + "]";
  ASSERT(num == numInvalidates, msg);
  num = reg1Listener1->getDestroys();
  msg = std::string("Got wrong number of destroys events. expected[") +
        std::to_string(numDestroys) + "], real[" + std::to_string(num) + "]";
  ASSERT(num == numDestroys, msg);
}

DUNIT_TASK_DEFINITION(SERVER1, StartServer)
  {
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_notify_subscription.xml");
    }
    LOG("SERVER started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, SetupClient1_Pool_Locator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], false /*ack mode*/, locatorsG,
                       "__TEST_POOL1__", true /*client notification*/,
                       nullptr /*cachelistener*/, false /*caching*/);
    reg1Listener1 = std::make_shared<TallyListener>();
    reg1Writer1 = std::make_shared<TallyWriter>();
    setCacheListener(regionNames[0], reg1Listener1);
    setCacheWriter(regionNames[0], reg1Writer1);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, testCreatesAndUpdates)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    auto keyPtr1 = CacheableKey::create(keys[1]);
    auto keyPtr2 = CacheableKey::create(keys[2]);
    std::vector<std::shared_ptr<CacheableKey>> keysVector;
    keysVector.push_back(keyPtr1);
    keysVector.push_back(keyPtr2);
    regPtr->registerKeys(keysVector);

    regPtr->create(keyPtr1, vals[1]);
    numCreates++;
    validateEventCount(__LINE__);

    // Trigger an update event from a put; for cacheless case this should be
    // received as a create
    regPtr->put(keyPtr1, nvals[1]);
    numCreates++;
    validateEventCount(__LINE__);

    // This put creates a new value, verify update event is not received
    regPtr->put(keyPtr2, vals[2]);
    numCreates++;
    validateEventCount(__LINE__);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StopClient1)
  {
    cleanProc();
    LOG("CLIENT1 stopped");
    // Reset numValues
    numCreates = 0;
    numUpdates = 0;
    numInvalidates = 0;
    numDestroys = 0;
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, StopServer)
  {
    if (isLocalServer) CacheHelper::closeServer(1);
    LOG("SERVER stopped");
  }
END_TASK_DEFINITION

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTINTEREST3CACHELESS_H_
