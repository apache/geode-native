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

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTINTEREST3_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTINTEREST3_H_

#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"
#include "TallyListener.hpp"
#include "TallyWriter.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1

namespace { // NOLINT(google-build-namespaces)

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
  LOG_INFO("ValidateEvents called from line (%d).", line);
  int num = reg1Listener1->getCreates();
  char buf[1024];
  sprintf(buf, "Got wrong number of creation events. expected[%d], real[%d]",
          numCreates, num);
  ASSERT(num == numCreates, buf);
  num = reg1Listener1->getUpdates();
  sprintf(buf, "Got wrong number of update events. expected[%d], real[%d]",
          numUpdates, num);
  ASSERT(num == numUpdates, buf);
  num = reg1Writer1->getCreates();
  sprintf(buf, "Got wrong number of writer events. expected[%d], real[%d]",
          numCreates, num);
  ASSERT(num == numCreates, buf);
  num = reg1Listener1->getInvalidates();
  sprintf(buf, "Got wrong number of invalidate events. expected[%d], real[%d]",
          numInvalidates, num);
  ASSERT(num == numInvalidates, buf);
  num = reg1Listener1->getDestroys();
  sprintf(buf, "Got wrong number of destroys events. expected[%d], real[%d]",
          numDestroys, num);
  ASSERT(num == numDestroys, buf);
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
                       "__TEST_POOL1__", true /*client notification*/);
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

    // Do a create followed by a create on the same key
    /*NIL: Changed the asserion due to the change in invalidate.
      Now we create new entery for every invalidate event received or
      localInvalidate call
      so expect  containsKey to returns true insted of false earlier.
      and used put call to update value for that key, instead of create call
      earlier or will get
      Region::create: Entry already exists in the region  */
    ASSERT(regPtr->containsKey(keyPtr1), "Key should be found in region.");
    regPtr->put(keyPtr1, vals[1]);

    numCreates++;
    validateEventCount(__LINE__);

    // Trigger an update event from a put
    regPtr->put(keyPtr1, nvals[1]);
    numUpdates++;
    validateEventCount(__LINE__);

    // This put creates a new value, verify update event is not received
    regPtr->put(keyPtr2, vals[2]);
    numCreates++;
    validateEventCount(__LINE__);

    // Do a get on a preexisting value to confirm no events are triggered
    regPtr->get(keyPtr2);
    validateEventCount(__LINE__);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, testInvalidateAndDestroy)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    auto keyPtr1 = CacheableKey::create(keys[1]);
    auto keyPtr2 = CacheableKey::create(keys[2]);
    regPtr->invalidate(keyPtr1);
    numInvalidates++;
    validateEventCount(__LINE__);

    regPtr->destroy(keyPtr2);
    numDestroys++;
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

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTINTEREST3_H_
