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

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTDURABLEINIT_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTDURABLEINIT_H_

bool isLocalServer = false;

const char* durableIds[] = {"DurableId1", "DurableId2"};

static bool isLocator = false;
static int numberOfLocators = 1;
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);

void initClientAndRegion(
    int redundancy, int ClientIdx,
    std::chrono::seconds subscriptionAckInterval = std::chrono::seconds(1),
    std::chrono::seconds redundancyMonitorInterval =
        std::chrono::seconds::zero(),
    std::chrono::seconds durableClientTimeout = std::chrono::seconds(60)) {
  auto pp = Properties::create();
  if (ClientIdx < 2) {
    pp->insert("durable-client-id", durableIds[ClientIdx]);
    pp->insert("durable-timeout", durableClientTimeout);
    if (redundancyMonitorInterval > std::chrono::seconds::zero()) {
      pp->insert("redundancy-monitor-interval", redundancyMonitorInterval);
    }

    initClient(true, pp);
    getHelper()->createPoolWithLocators("__TESTPOOL1_", locatorsG, true,
                                        redundancy, subscriptionAckInterval);
    createRegionAndAttachPool(regionNames[0], USE_ACK, "__TESTPOOL1_", true);
  }
}
void initClientAndTwoRegions(int ClientIdx, int redundancy,
                             std::chrono::seconds durableClientTimeout,
                             const char* conflation = nullptr,
                             const char* rNames[] = regionNames) {
  auto pp = Properties::create();
  pp->insert("durable-client-id", durableIds[ClientIdx]);
  pp->insert("durable-timeout", durableClientTimeout);
  if (conflation) {
    pp->insert("conflate-events", conflation);
  }

  initClient(true, pp);
  getHelper()->createPoolWithLocators("__TESTPOOL1_", locatorsG, true,
                                      redundancy, std::chrono::seconds(1));
  createRegionAndAttachPool(rNames[0], USE_ACK, "__TESTPOOL1_", true);
  createRegionAndAttachPool(rNames[1], USE_ACK, "__TESTPOOL1_", true);
}
void initClientAndTwoRegionsAndTwoPools(
    int ClientIdx, int redundancy, std::chrono::seconds durableClientTimeout,
    const char* conflation = nullptr, const char* rNames[] = regionNames) {
  auto pp = Properties::create();
  pp->insert("durable-client-id", durableIds[ClientIdx]);
  pp->insert("durable-timeout", durableClientTimeout);
  if (conflation) {
    pp->insert("conflate-events", conflation);
  }

  initClient(true, pp);
  getHelper()->createPoolWithLocators("__TESTPOOL2_", locatorsG, true,
                                      redundancy, std::chrono::seconds(1));
  createRegionAndAttachPool(rNames[1], USE_ACK, "__TESTPOOL2_", true);
  // Calling readyForEvents() here instead of below causes duplicate durableId
  // exception reproduced.
  /*LOG( "Calling readyForEvents:");
  try {
    getHelper()->cachePtr->readyForEvents();
  }catch(...) {
    LOG("Exception occured while sending readyForEvents");
  }*/

  auto regPtr1 = getHelper()->getRegion(rNames[1]);
  regPtr1->registerAllKeys(true);
  getHelper()->createPoolWithLocators("__TESTPOOL1_", locatorsG, true,
                                      redundancy, std::chrono::seconds(1));
  createRegionAndAttachPool(rNames[0], USE_ACK, "__TESTPOOL1_", true);
  auto regPtr0 = getHelper()->getRegion(rNames[0]);
  regPtr0->registerAllKeys(true);

  LOG("Calling readyForEvents:");
  try {
    getHelper()->cachePtr->readyForEvents();
  } catch (...) {
    LOG("Exception occured while sending readyForEvents");
  }
}

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTDURABLEINIT_H_
