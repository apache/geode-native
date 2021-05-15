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

#define ROOT_NAME "testThinClientListenerCallbackArgTest"

#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"
#include "TallyListener.hpp"
#include "TallyWriter.hpp"
#include "testobject/VariousPdxTypes.hpp"

#include "CacheRegionHelper.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1

#include "testobject/Portfolio.hpp"

using apache::geode::client::Cacheable;
using apache::geode::client::ClassCastException;
using apache::geode::client::EntryEvent;
using apache::geode::client::RegionEvent;

using apache::geode::client::testing::TallyListener;
using apache::geode::client::testing::TallyWriter;

bool isLocalServer = true;
static bool isLocator = false;
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
#include "LocatorHelper.hpp"

using testobject::Portfolio;
using testobject::Position;

class CallbackListener : public CacheListener {
 private:
  int m_creates;
  int m_updates;
  int m_invalidates;
  int m_destroys;
  int m_regionInvalidate;
  int m_regionDestroy;
  int m_regionClear;
  // std::shared_ptr<CacheableString> m_callbackArg;
  std::shared_ptr<Cacheable> m_callbackArg;

 public:
  CallbackListener()
      : CacheListener(),
        m_creates(0),
        m_updates(0),
        m_invalidates(0),
        m_destroys(0),
        m_regionInvalidate(0),
        m_regionDestroy(0),
        m_regionClear(0),
        m_callbackArg(nullptr) {
    LOG("CallbackListener contructor called");
  }

  ~CallbackListener() noexcept override = default;

  int getCreates() { return m_creates; }

  int getUpdates() { return m_updates; }
  int getInvalidates() { return m_invalidates; }
  int getDestroys() { return m_destroys; }
  int getRegionInvalidates() { return m_regionInvalidate; }
  int getRegionDestroys() { return m_regionDestroy; }
  int getRegionClear() { return m_regionClear; }
  void setCallBackArg(const std::shared_ptr<Cacheable> &callbackArg) {
    m_callbackArg = callbackArg;
  }

  void check(std::shared_ptr<Cacheable> eventCallback, int &updateEvent) {
    if (m_callbackArg != nullptr) {
      try {
        auto mCallbkArg = std::dynamic_pointer_cast<Portfolio>(m_callbackArg);

        auto callbkArg = std::dynamic_pointer_cast<Portfolio>(eventCallback);

        auto fromCallback = callbkArg->getPkid();
        auto mCallback = mCallbkArg->getPkid();

        LOG_FINE(" values are %s === %s ", fromCallback->value().c_str(),
                 mCallback->value().c_str());

        if (*(fromCallback.get()) == *(mCallback.get())) {
          LOG_FINE("values are same");
          updateEvent++;
        } else {
          LOG_FINE("values are NOT same");
        }
      } catch (const ClassCastException &ex) {
        LOG_FINE(" in class cast exception %s ", ex.what());
        try {
          auto fromCallback =
              std::dynamic_pointer_cast<CacheableString>(eventCallback);
          auto mCallback =
              std::dynamic_pointer_cast<CacheableString>(m_callbackArg);

          LOG_FINE(" values are %s === %s ", fromCallback->value().c_str(),
                   mCallback->value().c_str());

          if (*(fromCallback.get()) == *(mCallback.get())) {
            LOG_FINE("values are same");
            updateEvent++;
          } else {
            LOG_FINE("values are NOT same");
          }
        } catch (const ClassCastException &ex2) {
          LOG_FINE(" in class cast second exception %s ", ex2.what());
        }
      }
    }
  }

  void checkcallbackArg(const EntryEvent &event, int &updateEvent) {
    check(event.getCallbackArgument(), updateEvent);
  }

  void checkcallbackArg(const RegionEvent &event, int &updateEvent) {
    check(event.getCallbackArgument(), updateEvent);
  }

  void afterCreate(const EntryEvent &event) override {
    checkcallbackArg(event, m_creates);
  }

  void afterUpdate(const EntryEvent &event) override {
    checkcallbackArg(event, m_updates);
  }

  void afterInvalidate(const EntryEvent &event) override {
    checkcallbackArg(event, m_invalidates);
  }

  void afterDestroy(const EntryEvent &event) override {
    checkcallbackArg(event, m_destroys);
  }

  void afterRegionInvalidate(const RegionEvent &event) override {
    checkcallbackArg(event, m_regionInvalidate);
  }

  void afterRegionDestroy(const RegionEvent &event) override {
    checkcallbackArg(event, m_regionDestroy);
  }

  void afterRegionClear(const RegionEvent &event) override {
    checkcallbackArg(event, m_regionClear);
  }
};
//---------------------------------------------------------------------------------
std::shared_ptr<CallbackListener> reg1Listener1 = nullptr;
std::shared_ptr<CacheableString> callBackStrPtr;
std::shared_ptr<Cacheable> callBackPortFolioPtr;

void setCacheListener(const char *regName,
                      std::shared_ptr<CallbackListener> regListener) {
  auto reg = getHelper()->getRegion(regName);
  auto attrMutator = reg->getAttributesMutator();
  attrMutator->setCacheListener(regListener);
}

void validateEventCount(int line) {
  LOG_INFO("ValidateEvents called from line (%d).", line);
  int num = reg1Listener1->getCreates();
  char buf[1024];
  sprintf(buf, "Didn't get expected callback arg in aftercreate event");
  ASSERT(7 == num, buf);
  num = reg1Listener1->getUpdates();
  sprintf(buf, "Didn't get expected callback arg in afterupdate events");
  ASSERT(3 == num, buf);
  num = reg1Listener1->getInvalidates();
  sprintf(buf, "Didn't get expected callback arg in afterInvalidates events");
  ASSERT(2 == num, buf);
  num = reg1Listener1->getDestroys();
  sprintf(buf, "Didn't get expected callback arg in afterdestroy events");
  ASSERT(5 == num, buf);
  num = reg1Listener1->getRegionInvalidates();
  sprintf(buf,
          "Didn't get expected callback arg in afterRegionInvalidates events");
  ASSERT(1 == num, buf);
  num = reg1Listener1->getRegionDestroys();
  sprintf(buf, "Didn't get expected callback arg in afterRegiondestroy events");
  ASSERT(1 == num, buf);
  num = reg1Listener1->getRegionClear();
  sprintf(buf, "Didn't get expected callback arg in afterRegionClear events");
  ASSERT(1 == num, buf);
}

DUNIT_TASK_DEFINITION(SERVER1, StartServer)
  {
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_notify_subscription5.xml");
    }
    LOG("SERVER started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, SetupClient1_Pool_Locator)
  {
    initClient(true);

    callBackStrPtr = CacheableString::create("Gemstone's Callback");

    createPooledRegion(regionNames[0], false /*ack mode*/, locatorsG,
                       "__TEST_POOL1__", true /*client notification*/);
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    serializationRegistry->addDataSerializableType(
        Portfolio::createDeserializable, 3);
    serializationRegistry->addDataSerializableType(
        Position::createDeserializable, 2);
    reg1Listener1 = std::make_shared<CallbackListener>();
    callBackPortFolioPtr = std::make_shared<Portfolio>(1, 0, nullptr);

    reg1Listener1->setCallBackArg(callBackPortFolioPtr);
    setCacheListener(regionNames[0], reg1Listener1);
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    regPtr->registerAllKeys();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, SetupClient2_Pool_Locator)
  {
    initClient(true);

    callBackStrPtr = CacheableString::create("Gemstone's Callback");

    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    serializationRegistry->addDataSerializableType(
        Portfolio::createDeserializable, 3);
    serializationRegistry->addDataSerializableType(
        Position::createDeserializable, 2);

    createPooledRegion(regionNames[0], false /*ack mode*/, locatorsG,
                       "__TEST_POOL1__", true /*client notification*/);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, testCreatesAndUpdates)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);

    callBackPortFolioPtr = std::make_shared<Portfolio>(1, 0, nullptr);
    regPtr->create("aaa", "bbb", callBackPortFolioPtr);
    regPtr->create(keys[1], vals[1], callBackPortFolioPtr);
    regPtr->put(keys[1], nvals[1], callBackPortFolioPtr);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, testInvalidates)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);

    callBackPortFolioPtr = std::make_shared<Portfolio>(1, 0, nullptr);
    regPtr->localCreate(1234, 1234, callBackPortFolioPtr);
    regPtr->localCreate(12345, 12345, callBackPortFolioPtr);
    regPtr->localCreate(12346, 12346, callBackPortFolioPtr);
    regPtr->localPut(1234, vals[1], callBackPortFolioPtr);
    regPtr->localInvalidate(1234, callBackPortFolioPtr);
    ASSERT(regPtr->localRemove(12345, 12345, callBackPortFolioPtr) == true,
           "Result of remove should be true, as this value exists locally.");
    ASSERT(regPtr->containsKey(12345) == false, "containsKey should be false");
    ASSERT(regPtr->localRemoveEx(12346, callBackPortFolioPtr) == true,
           "Result of remove should be true, as this value exists locally.");
    ASSERT(regPtr->containsKey(12346) == false, "containsKey should be false");
    regPtr->invalidate(keys[1], callBackPortFolioPtr);
    regPtr->invalidateRegion(callBackPortFolioPtr);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, testDestroy)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);

    callBackPortFolioPtr = std::make_shared<Portfolio>(1, 0, nullptr);
    regPtr->destroy(keys[1], callBackPortFolioPtr);
    ASSERT(regPtr->removeEx("aaa", callBackPortFolioPtr) == true,
           "Result of remove should be true, as this value exists.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, testRemove)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    regPtr->remove(keys[1], nvals[1], callBackPortFolioPtr);
    regPtr->destroyRegion(callBackPortFolioPtr);
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT1, testlocalClear)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    regPtr->localClear(callBackPortFolioPtr);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, testValidate)
  {
    dunit::sleep(10000);
    validateEventCount(__LINE__);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StopClient1)
  {
    cleanProc();
    LOG("CLIENT1 stopped");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StopClient2)
  {
    cleanProc();
    LOG("CLIENT2 stopped");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, StopServer)
  {
    if (isLocalServer) CacheHelper::closeServer(1);
    LOG("SERVER stopped");
  }
END_TASK_DEFINITION

DUNIT_MAIN
  {
    CALL_TASK(CreateLocator1);
    CALL_TASK(CreateServer1_With_Locator_XML5);

    CALL_TASK(SetupClient1_Pool_Locator);
    CALL_TASK(SetupClient2_Pool_Locator);
    CALL_TASK(testCreatesAndUpdates);
    CALL_TASK(testInvalidates);
    CALL_TASK(testDestroy);
    CALL_TASK(testCreatesAndUpdates);
    CALL_TASK(testlocalClear);
    CALL_TASK(testRemove);
    CALL_TASK(testValidate);
    CALL_TASK(StopClient1);
    CALL_TASK(StopClient2);
    CALL_TASK(StopServer);
    CALL_TASK(CloseLocator1);
  }
END_MAIN
