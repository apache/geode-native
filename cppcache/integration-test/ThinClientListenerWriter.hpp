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

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTLISTENERWRITER_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTLISTENERWRITER_H_

#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"
#include "TallyListener.hpp"
#include "TallyWriter.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define CLIENT3 s2p2

namespace { // NOLINT(google-build-namespaces)
using apache::geode::client::EntryEvent;
using apache::geode::client::RegionEvent;

using apache::geode::client::testing::TallyListener;
using apache::geode::client::testing::TallyWriter;

class SimpleCacheListener;

// The SimpleCacheListener class.
class SimpleCacheListener : public CacheListener {
 private:
  int m_creates;
  int m_clears;

 public:
  // The Cache Listener callbacks.
  void afterCreate(const EntryEvent& event) override;
  void afterUpdate(const EntryEvent& event) override;
  void afterInvalidate(const EntryEvent& event) override;
  void afterDestroy(const EntryEvent& event) override;
  void afterRegionInvalidate(const RegionEvent& event) override;
  void afterRegionDestroy(const RegionEvent& event) override;
  void close(Region& region) override;
  void afterRegionClear(const RegionEvent& event) override;

  SimpleCacheListener() : CacheListener(), m_creates(0), m_clears(0) {
    LOG_INFO("SimpleCacheListener contructor called");
  }

  virtual ~SimpleCacheListener() override {}
  int getCreates() { return m_creates; }

  int getClears() { return m_clears; }
};

void SimpleCacheListener::afterCreate(const EntryEvent& event) {
  LOG_INFO("SimpleCacheListener: Got an afterCreate event for %s region .",
          event.getRegion()->getName().c_str());
  m_creates++;
}

void SimpleCacheListener::afterUpdate(const EntryEvent& event) {
  LOG_INFO("SimpleCacheListener: Got an afterUpdate event for %s region .",
          event.getRegion()->getName().c_str());
}

void SimpleCacheListener::afterInvalidate(const EntryEvent& event) {
  LOG_INFO("SimpleCacheListener: Got an afterInvalidate event for %s region .",
          event.getRegion()->getName().c_str());
}

void SimpleCacheListener::afterDestroy(const EntryEvent& event) {
  LOG_INFO("SimpleCacheListener: Got an afterDestroy event for %s region .",
          event.getRegion()->getName().c_str());
}

void SimpleCacheListener::afterRegionInvalidate(const RegionEvent& event) {
  LOG_INFO(
      "SimpleCacheListener: Got an afterRegionInvalidate event for %s region .",
      event.getRegion()->getName().c_str());
}

void SimpleCacheListener::afterRegionDestroy(const RegionEvent& event) {
  LOG_INFO(
      "SimpleCacheListener: Got an afterRegionDestroy event for %s region .",
      event.getRegion()->getName().c_str());
}

void SimpleCacheListener::close(Region& region) {
  LOG_INFO("SimpleCacheListener: Got an close event for %s region .",
          region.getName().c_str());
}

void SimpleCacheListener::afterRegionClear(const RegionEvent& event) {
  LOG_INFO("SimpleCacheListener: Got an afterRegionClear event for %s region .",
          event.getRegion()->getName().c_str());
  m_clears++;
}

/*
 * start server with NBS true/false
 * start one cache less client which will entry operations (
 * put/invalidate/destroy ).
 * start 2nd cacheless client with cache listener,  writer and client
 * notification true.
 * verify that listener is invoked and writer is not being invoked in 2nd client
 */

static bool isLocalServer = false;
static bool isLocator = false;
static int numberOfLocators = 0;
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);
const char* poolName = "__TESTPOOL1_";
std::shared_ptr<TallyListener> regListener;
std::shared_ptr<SimpleCacheListener> parentRegCacheListener;
std::shared_ptr<SimpleCacheListener> subRegCacheListener;
std::shared_ptr<SimpleCacheListener> distRegCacheListener;
std::shared_ptr<TallyWriter> regWriter;

#include "LocatorHelper.hpp"
const char* myRegNames[] = {"DistRegionAck", "DistRegionNoAck", "ExampleRegion",
                            "SubRegion1", "SubRegion2"};
void setCacheListener(const char* regName,
                      std::shared_ptr<TallyListener> regionTallyListener) {
  auto reg = getHelper()->getRegion(regName);
  auto attrMutator = reg->getAttributesMutator();
  attrMutator->setCacheListener(regionTallyListener);
}

void setCacheWriter(const char* regName,
                    std::shared_ptr<TallyWriter> regionTallyWriter) {
  auto reg = getHelper()->getRegion(regName);
  auto attrMutator = reg->getAttributesMutator();
  attrMutator->setCacheWriter(regionTallyWriter);
}

void validateEvents() {
  SLEEP(5000);
  regListener->showTallies();
  ASSERT(regListener->getCreates() == 0, "Should be 0 creates");
  ASSERT(regListener->getUpdates() == 0, "Should be 0 updates");
  // invalidate message is not implemented so expecting 0 events..
  ASSERT(regListener->getInvalidates() == 10, "Should be 10 Invalidate");
  ASSERT(regListener->getDestroys() == 5, "Should be 5 destroy");
  ASSERT(regWriter->isWriterInvoked() == false, "Writer Should not be invoked");
}
DUNIT_TASK_DEFINITION(SERVER1, CreateServerWithNBSTrue)
  {
    // starting server with notify_subscription true
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_notify_subscription.xml");
    }
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    LOG("Starting SERVER1...");
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_notify_subscriptionBug849.xml");
    }
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServerWithNBSFalse)
  {
    // starting server with notify_subscription false
    if (isLocalServer) CacheHelper::initServer(1, "cacheserver.xml");
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1_With_Locator_NBSFalse)
  {
    // starting server with notify_subscription false
    if (isLocalServer) CacheHelper::initServer(1, "cacheserver.xml", locatorsG);
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, SetupClient1_Pooled_Locator)
  {
    initClient(true);
    LOG("Creating region in CLIENT1, no-ack, no-cache, no-listener");
    createPooledRegion(regionNames[0], false, locatorsG, poolName, true,
                       nullptr, false);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, SetupClient1withCachingEnabled_Pooled_Locator)
  {
    initClient(true);
    LOG("Creating region in CLIENT1, no-ack, no-cache, no-listener");
    createPooledRegion(myRegNames[0], false, locatorsG, poolName, true, nullptr,
                       true);
    createPooledRegion(myRegNames[1], false, locatorsG, poolName, true, nullptr,
                       true);
    createPooledRegion(myRegNames[2], false, locatorsG, poolName, true, nullptr,
                       true);

    // create subregion
    auto exmpRegptr = getHelper()->getRegion(myRegNames[2]);
    auto lattribPtr = exmpRegptr->getAttributes();
    auto subregPtr1 = exmpRegptr->createSubregion(myRegNames[3], lattribPtr);
    auto subregPtr2 = exmpRegptr->createSubregion(myRegNames[4], lattribPtr);

    LOG_INFO(
        " CLIENT1 SetupClient1withCachingEnabled_Pooled_Locator subRegions "
        "created successfully");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, Register2WithTrue)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    regPtr0->registerAllKeys();
  }
END_TASK_DEFINITION

// RegisterKeys
DUNIT_TASK_DEFINITION(CLIENT2, RegisterKeys)
  {
    auto regPtr0 = getHelper()->getRegion(myRegNames[0]);

    auto exmpRegPtr = getHelper()->getRegion(myRegNames[2]);
    auto subregPtr0 = exmpRegPtr->getSubregion(myRegNames[3]);
    auto subregPtr1 = exmpRegPtr->getSubregion(myRegNames[4]);

    // 1. registerAllKeys on parent and both subregions
    regPtr0->registerAllKeys();
    exmpRegPtr->registerAllKeys();
    subregPtr0->registerAllKeys();
    subregPtr1->registerAllKeys();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, Register2WithFalse)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    regPtr0->registerAllKeys(false, false, false);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT3, Register3WithFalse)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    regPtr0->registerAllKeys(false, false, false);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, SetupClient2_Pooled_Locator)
  {
    initClient(true);
    LOG("Creating region in CLIENT2 , no-ack, no-cache, with-listener and "
        "writer");
    regListener = std::make_shared<TallyListener>();
    createPooledRegion(regionNames[0], false, locatorsG, poolName, true,
                       regListener, false);
    regWriter = std::make_shared<TallyWriter>();
    setCacheWriter(regionNames[0], regWriter);
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    // regPtr0->registerAllKeys();
  }
END_TASK_DEFINITION

//
DUNIT_TASK_DEFINITION(CLIENT2, SetupClient2withCachingEnabled_Pooled_Locator)
  {
    initClient(true);
    LOG("Creating region in CLIENT2 , no-ack, no-cache, with-listener and "
        "writer");
    parentRegCacheListener = std::make_shared<SimpleCacheListener>();
    distRegCacheListener = std::make_shared<SimpleCacheListener>();

    createPooledRegion(myRegNames[0], false, locatorsG, poolName, true,
                       distRegCacheListener, true);
    createPooledRegion(myRegNames[1], false, locatorsG, poolName, true, nullptr,
                       true);
    createPooledRegion(myRegNames[2], false, locatorsG, poolName, true,
                       parentRegCacheListener, true);

    regWriter = std::make_shared<TallyWriter>();
    setCacheWriter(myRegNames[2], regWriter);

    // create subregion
    auto exmpRegptr = getHelper()->getRegion(myRegNames[2]);
    auto lattribPtr = exmpRegptr->getAttributes();
    auto subregPtr1 = exmpRegptr->createSubregion(myRegNames[3], lattribPtr);
    auto subregPtr2 = exmpRegptr->createSubregion(myRegNames[4], lattribPtr);

    LOG_INFO(
        "CLIENT2 SetupClient2withCachingEnabled_Pooled_Locator:: subRegions "
        "created successfully");

    // Attach Listener to subRegion
    // Attache Listener

    auto subregAttrMutatorPtr = subregPtr1->getAttributesMutator();
    subRegCacheListener = std::make_shared<SimpleCacheListener>();
    subregAttrMutatorPtr->setCacheListener(subRegCacheListener);

    LOG("StepTwo_Pool complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT3, SetupClient3_Pooled_Locator)
  {
    // client with no registerAllKeys.....
    initClient(true);
    LOG("Creating region in CLIENT2 , no-ack, no-cache, with-listener and "
        "writer");
    regListener = std::make_shared<TallyListener>();
    createPooledRegion(regionNames[0], false, locatorsG, poolName, true,
                       regListener, false);
    regWriter = std::make_shared<TallyWriter>();
    setCacheWriter(regionNames[0], regWriter);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, doOperations)
  {
    LOG("do entry operation from client 1");
    RegionOperations region(regionNames[0]);
    region.putOp(5);
    SLEEP(1000);  // let the events reach at other end.
    region.putOp(5);
    SLEEP(1000);
    region.invalidateOp(5);
    SLEEP(1000);
    region.destroyOp(5);
    SLEEP(1000);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, validateListenerWriterWithNBSTrue)
  {
    SLEEP(5000);
    LOG("Verifying TallyListener has received verious events.");
    regListener->showTallies();
    ASSERT(regListener->getCreates() == 5, "Should be 5 creates");
    ASSERT(regListener->getUpdates() == 5, "Should be 5 updates");
    // invalidate message is not implemented so expecting 0 events..
    ASSERT(regListener->getInvalidates() == 0, "Should be 0 Invalidate");
    ASSERT(regListener->getDestroys() == 5, "Should be 5 destroy");
    ASSERT(regWriter->isWriterInvoked() == false,
           "Writer Should not be invoked");

    LOG_INFO("Total cleared Entries = %d ", regListener->getClears());
  }
END_TASK_DEFINITION

//
DUNIT_TASK_DEFINITION(CLIENT1, doEventOperations)
  {
    LOG("do entry operation from client 1");

    auto regPtr0 = getHelper()->getRegion(myRegNames[0]);
    auto exmpRegPtr = getHelper()->getRegion(myRegNames[2]);

    auto subregPtr1 = exmpRegPtr->getSubregion(myRegNames[3]);
    auto subregPtr2 = exmpRegPtr->getSubregion(myRegNames[4]);

    for (int index = 0; index < 5; index++) {
      std::string key = "Key-" + std::to_string(index);
      std::string value = "Value-" + std::to_string(index);

      auto keyptr = CacheableKey::create(key);
      auto valuePtr = CacheableString::create(value);
      regPtr0->put(keyptr, valuePtr);
      exmpRegPtr->put(keyptr, valuePtr);
      subregPtr1->put(keyptr, valuePtr);
      subregPtr2->put(keyptr, valuePtr);
    }

    LOG_INFO(
        "CLIENT-1 localCaching Enabled After Put ....ExampleRegion.size() = %d",
        exmpRegPtr->size());
    ASSERT(exmpRegPtr->size() == 5,
           "Total number of entries in the region should be 5");

    LOG_INFO(
        "CLIENT-1 localCaching Enabled After Put ....DistRegionAck.size() = %d",
        regPtr0->size());

    // TEST COVERAGE FOR cacheListener.afterRegionClear() API
    exmpRegPtr->clear();
    LOG_INFO("CLIENT-1 AFTER Clear() call ....reg.size() = %d",
            exmpRegPtr->size());
    ASSERT(exmpRegPtr->size() == 0,
           "Total number of entries in the region should be 0");

    LOG_INFO("CLIENT-1 AFTER Clear() call ....SubRegion-1.size() = %d",
            subregPtr1->size());
    ASSERT(subregPtr1->size() == 5,
           "Total number of entries in the region should be 0");

    LOG_INFO("CLIENT-1 AFTER Clear() call ....SubRegion-2.size() = %d",
            subregPtr2->size());
    ASSERT(subregPtr2->size() == 5,
           "Total number of entries in the region should be 0");

    SLEEP(1000);
  }
END_TASK_DEFINITION

//
DUNIT_TASK_DEFINITION(CLIENT2, validateListenerWriterEventsWithNBSTrue)
  {
    SLEEP(5000);
    LOG("Verifying SimpleListerner has received verious events.");
    // regListener->showTallies();
    LOG_INFO(" distRegCacheListener->getCreates() = %d",
            distRegCacheListener->getCreates());

    // LOG_INFO(" parentRegCacheListener->getCreates() = %d",
    // parentRegCacheListener->getCreates());
    ASSERT(parentRegCacheListener->getCreates() == 10, "Should be 10 creates");
    ASSERT(regWriter->isWriterInvoked() == false,
           "Writer Should not be invoked");

    // Verify that the region.clear event is received and it has cleared all
    // entries in region
    // LOG_INFO("parentRegCacheListener::m_clears = %d ",
    // parentRegCacheListener->getClears());
    ASSERT(parentRegCacheListener->getClears() == 1,
           "region.clear() should be called once");

    auto exmpRegPtr = getHelper()->getRegion(myRegNames[2]);
    // LOG_INFO(" Total Entries in ExampleRegion = %d ", exmpRegPtr->size());
    ASSERT(exmpRegPtr->size() == 0,
           "Client-2 ExampleRegion.clear() should have called and so "
           "Exampleregion size is expected to 0 ");

    // Verify entries in Sub-Region.
    auto subregPtr1 = exmpRegPtr->getSubregion(myRegNames[3]);
    auto subregPtr2 = exmpRegPtr->getSubregion(myRegNames[4]);

    // LOG_INFO(" Total Entries in SubRegion-1 = %d ", subregPtr1->size());
    // LOG_INFO(" Total Entries in SubRegion-2 = %d ", subregPtr2->size());
    ASSERT(subRegCacheListener->getCreates() == 5,
           "should be 5 creates for SubRegion-1 ");
    ASSERT(subRegCacheListener->getClears() == 0,
           "should be 0 clears for SubRegion-1 ");
    ASSERT(subregPtr1->size() == 5,
           "Client-2 SubRegion-1 should contains 5 entries ");
    ASSERT(subregPtr2->size() == 5,
           "Client-2 SubRegion-2 should contains 5 entries ");

    // LOG_INFO(" SubRegion-1 CREATES:: subRegCacheListener::m_creates = %d ",
    // subRegCacheListener->getCreates());
    // LOG_INFO(" SubRegion-1 CLEARS:: subRegCacheListener::m_clears = %d ",
    // subRegCacheListener->getClears());

    LOG_INFO(
        "validateListenerWriterEventsWithNBSTrue :: Event Validation "
        "Passed....!!");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, validateListenerWriterWithNBSFalse)
  { validateEvents(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT3, validateListenerWriterWithNBSFalseForClient3)
  { validateEvents(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CloseCache2)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT3, CloseCache3)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer1)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
  }
END_TASK_DEFINITION

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTLISTENERWRITER_H_
