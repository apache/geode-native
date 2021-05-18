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

bool isLocalServer = false;
bool isLocator = false;

const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define LOCATOR1 s2p1
#define SERVER12 s2p2
DUNIT_TASK(LOCATOR1, CreateLocator1)
  {
    // starting locator
    if (isLocator) CacheHelper::initLocator(1);
    LOG("Locator1 started");
  }
END_TASK(CreateLocator1)
DUNIT_TASK(SERVER12, CreateServer12)
  {
    // starting servers
    if (isLocalServer) {
      CacheHelper::initServer(1, {}, locHostPort);
    }
    if (isLocalServer) {
      CacheHelper::initServer(2, {}, locHostPort);
    }
    LOG("Server12 started");
  }
END_TASK(CreateServer12)
DUNIT_TASK(CLIENT1, StepOne)
  {
    // starting client 1
    initClientWithPool(true, "__TEST_POOL1__", locHostPort, nullptr, nullptr, 0,
                       true);
    getHelper()->createPooledRegion(regionNames[0], USE_ACK, locHostPort,
                                    "__TEST_POOL1__", true, true);
    LOG("StepOne complete.");
  }
END_TASK(StepOne)
DUNIT_TASK(CLIENT2, StepTwo)
  {
    initClientWithPool(true, "__TEST_POOL1__", locHostPort, nullptr, nullptr, 0,
                       true);
    getHelper()->createPooledRegion(regionNames[0], USE_ACK, locHostPort,
                                    "__TEST_POOL1__", true, true);
    LOG("StepTwo complete.");
  }
END_TASK(StepTwo)

DUNIT_TASK(CLIENT1, StepThree)
  {
    createEntry(regionNames[0], keys[0], vals[0]);
    LOG("StepThree complete.");
  }
END_TASK(StepThree)
DUNIT_TASK(CLIENT2, StepFour)
  {
    doNetsearch(regionNames[0], keys[0], vals[0]);
    createEntry(regionNames[0], keys[1], vals[1]);
    LOG("StepFour complete.");
  }
END_TASK(StepFour)
DUNIT_TASK(CLIENT1, StepFive)
  {
    doNetsearch(regionNames[0], keys[1], vals[1]);
    updateEntry(regionNames[0], keys[0], nvals[0]);
    LOG("StepFive complete.");
  }
END_TASK(StepFive)
DUNIT_TASK(CLIENT2, StepSix)
  {
    doNetsearch(regionNames[0], keys[0], vals[0], false);
    updateEntry(regionNames[0], keys[1], nvals[1]);
    LOG("StepSix complete.");
  }
END_TASK(StepSix)
DUNIT_TASK(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK(CloseCache1)

DUNIT_TASK(CLIENT2, CloseCache2)
  { cleanProc(); }
END_TASK(CloseCache2)
DUNIT_TASK(SERVER12, CloseServers)
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
  }
END_TASK(CloseServers)

DUNIT_TASK(LOCATOR1, CloseLocator1)
  {
    // stop locator
    if (isLocator) {
      CacheHelper::closeLocator(1);
      LOG("Locator1 stopped");
    }
  }
END_TASK(CloseLocator1)
