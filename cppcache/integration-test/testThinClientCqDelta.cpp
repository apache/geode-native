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

#include "testobject/DeltaTestImpl.hpp"
#include "fw_dunit.hpp"
#include <string>
#include "CacheHelper.hpp"
#include <geode/CqAttributesFactory.hpp>
#include <geode/CqAttributes.hpp>
#include <geode/CqListener.hpp>
#include <geode/CqQuery.hpp>
#include <geode/CqServiceStatistics.hpp>
#include <geode/Cache.hpp>

#include "SerializationRegistry.hpp"
#include "CacheRegionHelper.hpp"
#include "CacheImpl.hpp"

using namespace apache::geode::client;
using namespace test;
using namespace testobject;

CacheHelper* cacheHelper = nullptr;

#include "locator_globals.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#include "LocatorHelper.hpp"

CacheHelper* getHelper() {
  ASSERT(cacheHelper != nullptr, "No cacheHelper initialized.");
  return cacheHelper;
}

class CqDeltaListener : public CqListener {
 public:
  CqDeltaListener() : m_deltaCount(0), m_valueCount(0) {}

  virtual void onEvent(const CqEvent& aCqEvent) {
    auto deltaValue = aCqEvent.getDeltaValue();
    DeltaTestImpl newValue;
    auto input = getHelper()->getCache()->createDataInput(
        reinterpret_cast<const uint8_t*>(deltaValue->value().data()),
        deltaValue->length());
    newValue.fromDelta(input);
    if (newValue.getIntVar() == 5) {
      m_deltaCount++;
    }
    auto dptr = std::static_pointer_cast<DeltaTestImpl>(aCqEvent.getNewValue());
    if (dptr->getIntVar() == 5) {
      m_valueCount++;
    }
  }

  int getDeltaCount() { return m_deltaCount; }
  int getValueCount() { return m_valueCount; }

 private:
  int m_deltaCount;
  int m_valueCount;
};

std::shared_ptr<CqDeltaListener> g_CqListener;

void initClient(const bool isthinClient) {
  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(isthinClient);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

void initClientNoPools() {
  cacheHelper = new CacheHelper(0);
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

void cleanProc() {
  if (cacheHelper != nullptr) {
    delete cacheHelper;
    cacheHelper = nullptr;
  }
}

void createPooledRegion(const char* name, bool ackMode, const char* locators,
                        const char* poolname,
                        bool clientNotificationEnabled = false,
                        bool cachingEnable = true) {
  LOG("createRegion_Pool() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  auto regPtr =
      getHelper()->createPooledRegion(name, ackMode, locators, poolname,
                                      cachingEnable, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Pooled Region created.");
}

void createPooledLRURegion(const char* name, bool ackMode, const char* locators,
                           const char* poolname,
                           bool clientNotificationEnabled = false,
                           bool cachingEnable = true) {
  LOG(" createPooledLRURegion entered");
  auto regPtr = getHelper()->createPooledRegionDiscOverFlow(
      name, ackMode, locators, poolname, cachingEnable,
      clientNotificationEnabled, std::chrono::seconds(0),
      std::chrono::seconds(0), std::chrono::seconds(0), std::chrono::seconds(0),
      3 /*LruLimit = 3*/);
  LOG(" createPooledLRURegion exited");
}

void createRegion(const char* name, bool ackMode,
                  bool clientNotificationEnabled = false) {
  LOG("createRegion() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  // ack, caching
  auto regPtr = getHelper()->createRegion(name, ackMode, true, nullptr,
                                          clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}
const char* keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};

const char* regionNames[] = {"DistRegionAck", "DistRegionAck1"};

const bool USE_ACK = true;
const bool NO_ACK = false;

DUNIT_TASK_DEFINITION(CLIENT1, CreateClient1)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TESTPOOL1_",
                       true);
    try {
      auto serializationRegistry =
          CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
              ->getSerializationRegistry();

      serializationRegistry->addType(DeltaTestImpl::create);
    } catch (IllegalStateException&) {
      //  ignore exception caused by type reregistration.
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CreateClient2)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TESTPOOL1_",
                       true);
    try {
      auto serializationRegistry =
          CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
              ->getSerializationRegistry();

      serializationRegistry->addType(DeltaTestImpl::create);
    } catch (IllegalStateException&) {
      //  ignore exception caused by type reregistration.
    }
    auto regPtr = getHelper()->getRegion(regionNames[0]);

    auto pool = getHelper()->getCache()->getPoolManager().find("__TESTPOOL1_");
    std::shared_ptr<QueryService> qs;
    qs = pool->getQueryService();
    CqAttributesFactory cqFac;
    g_CqListener = std::make_shared<CqDeltaListener>();
    auto cqListener = g_CqListener;
    cqFac.addCqListener(cqListener);
    auto cqAttr = cqFac.create();
    auto qry =
        qs->newCq("Cq_with_delta",
                  "select * from /DistRegionAck d where d.intVar > 4", cqAttr);
    qs->executeCqs();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CreateClient1_NoPools)
  {
    initClientNoPools();
    createRegion(regionNames[0], USE_ACK, true);
    try {
      auto serializationRegistry =
          CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
              ->getSerializationRegistry();

      serializationRegistry->addType(DeltaTestImpl::create);
    } catch (IllegalStateException&) {
      //  ignore exception caused by type reregistration.
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CreateClient2_NoPools)
  {
    initClientNoPools();
    createRegion(regionNames[0], USE_ACK, true);
    try {
      auto serializationRegistry =
          CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
              ->getSerializationRegistry();

      serializationRegistry->addType(DeltaTestImpl::create);
    } catch (IllegalStateException&) {
      //  ignore exception caused by type reregistration.
    }
    auto regPtr = getHelper()->getRegion(regionNames[0]);

    std::shared_ptr<QueryService> qs;
    qs = getHelper()->getQueryService();
    CqAttributesFactory cqFac;
    g_CqListener = std::make_shared<CqDeltaListener>();
    auto cqListener = g_CqListener;
    cqFac.addCqListener(cqListener);
    auto cqAttr = cqFac.create();
    auto qry =
        qs->newCq("Cq_with_delta",
                  "select * from /DistRegionAck d where d.intVar > 4", cqAttr);
    qs->executeCqs();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, Client1_Put)
  {
    auto keyPtr = CacheableKey::create(keys[0]);
    auto dptr = std::make_shared<DeltaTestImpl>();
    std::shared_ptr<Cacheable> valPtr(dptr);
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    regPtr->put(keyPtr, valPtr);
    dptr->setIntVar(5);
    dptr->setDelta(true);
    regPtr->put(keyPtr, valPtr);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, Client2_VerifyDelta)
  {
    // Wait for notification
    SLEEP(5000);
    ASSERT(g_CqListener->getDeltaCount() == 1,
           "Delta from CQ event does not have expected value");
    ASSERT(g_CqListener->getValueCount() == 1,
           "Value from CQ event is incorrect");
  }

END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CloseCache2)
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

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1_ForCqDelta)
  {
    // starting servers
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_with_delta_test_impl.xml",
                              locatorsG);
    }
  }
END_TASK_DEFINITION

DUNIT_MAIN
  {
    CALL_TASK(CreateLocator1);

    CALL_TASK(CreateServer1_ForCqDelta)

    CALL_TASK(CreateClient1);
    CALL_TASK(CreateClient2);

    CALL_TASK(Client1_Put);
    CALL_TASK(Client2_VerifyDelta);

    CALL_TASK(CloseCache1);
    CALL_TASK(CloseCache2);

    CALL_TASK(CloseServer1);

    CALL_TASK(CloseLocator1);
  }
END_MAIN
