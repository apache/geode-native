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
#define SERVER1 s2p1

#include "locator_globals.hpp"
#include "LocatorHelper.hpp"

using apache::geode::client::EntryEvent;
using apache::geode::client::RegionEvent;

class SimpleCacheListener;

// The SimpleCacheListener class.
class SimpleCacheListener : public CacheListener {
 public:
  // The Cache Listener callbacks.
  SimpleCacheListener() { m_count = 0; }
  void afterCreate(const EntryEvent &) override {
    m_count++;
    LOG_INFO("SimpleCacheListener: Got an afterCreate event.");
  }
  void afterUpdate(const EntryEvent &) override {
    LOG_INFO("SimpleCacheListener: Got an afterUpdate event.");
  }
  void afterInvalidate(const EntryEvent &) override {
    LOG_INFO("SimpleCacheListener: Got an afterInvalidate event.");
  }
  void afterDestroy(const EntryEvent &) override {
    LOG_INFO("SimpleCacheListener: Got an afterDestroy event.");
  }
  void afterRegionInvalidate(const RegionEvent &) override {
    LOG_INFO("SimpleCacheListener: Got an afterRegionInvalidate event.");
  }
  void afterRegionDestroy(const RegionEvent &) override {
    LOG_INFO("SimpleCacheListener: Got an afterRegionDestroy event.");
  }
  void close(Region &) override {
    LOG_INFO("SimpleCacheListener: Got an close event.");
  }

  int getCount() { return m_count; }

 private:
  int m_count;
};
//---------------------------------------------------------------------------------

DUNIT_TASK_DEFINITION(SERVER1, StartServer)
  {
    if (isLocalServer) CacheHelper::initServer(1, "cacheserver.xml");
    LOG("SERVER started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, SetupClient1)
  {
    initClientWithPool(true, "__TEST_POOL1__", locatorsG, nullptr, nullptr, 0,
                       true);
    getHelper()->createPooledRegion(regionNames[0], false, locatorsG,
                                    "__TEST_POOL1__", true, true);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, doRemoteGet)
  {
    auto regionPtr = getHelper()->getRegion(regionNames[0]);

    auto attrMutatorPtr = regionPtr->getAttributesMutator();
    auto regListener1 = std::make_shared<SimpleCacheListener>();
    attrMutatorPtr->setCacheListener(regListener1);

    // Put 3 Entries into the Region.
    regionPtr->put("Key1", "Value1");
    regionPtr->put("Key2", "Value2");
    regionPtr->put("Key3", "Value3");

    // Update Key3.
    regionPtr->put("Key3", "Value3-updated");

    // Destroy Key3.
    regionPtr->localDestroy("Key3");

    // Perform remote get (Locally destroyed).
    regionPtr->get("Key3");
    int toalFunCall = regListener1->getCount();
    ASSERT(4 == toalFunCall,
           "afterCreate() did not call expected number of times");
    // printf("[NIL_DEBUG_DUnitTest:149] Total Function Call =
    // %d.............\n",
    // toalFunCall);
    // printf("\n[NIL_DEBUG_DUnitTest:150:Remote get ended.
    // ..................\n");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StopClient1)
  {
    cleanProc();
    LOG("CLIENT1 stopped");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, StopServer)
  {
    if (isLocalServer) CacheHelper::closeServer(1);
    LOG("SERVER stopped");
  }
END_TASK_DEFINITION

void runThinClientListenerEventsTest() {
  CALL_TASK(CreateLocator1);
  CALL_TASK(CreateServer1_With_Locator);
  CALL_TASK(SetupClient1);

  CALL_TASK(doRemoteGet);

  CALL_TASK(StopClient1);
  CALL_TASK(StopServer);
  CALL_TASK(CloseLocator1);
}

DUNIT_MAIN
  { runThinClientListenerEventsTest(); }
END_MAIN
