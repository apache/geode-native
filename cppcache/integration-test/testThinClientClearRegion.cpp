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
#define CLIENT2 s1p2
#define SERVER1 s2p1

using apache::geode::client::CacheableBytes;
using apache::geode::client::CacheWriter;
using apache::geode::client::RegionEvent;

class MyCacheWriter : public CacheWriter {
  uint32_t m_clear;

 public:
  MyCacheWriter() : m_clear(0) {}
  bool beforeRegionClear(const RegionEvent &) override {
    LOG("beforeRegionClear called");
    m_clear++;
    return true;
  }
  uint32_t getClearCnt() { return m_clear; }
};
class MyCacheListener : public CacheListener {
  uint32_t m_clear;

 public:
  MyCacheListener() : m_clear(0) {}
  void afterRegionClear(const RegionEvent &) override {
    LOG("afterRegionClear called");
    m_clear++;
  }
  uint32_t getClearCnt() { return m_clear; }
};

static int numberOfLocators = 1;
bool isLocalServer = true;
bool isLocator = true;
const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);

DUNIT_TASK(SERVER1, StartServer)
  {
    if (isLocalServer) {
      CacheHelper::initLocator(1);
      CacheHelper::initServer(1, "cacheserver_notify_subscription.xml",
                              locHostPort);
    }
    LOG("SERVER started");
  }
END_TASK(StartServer)

DUNIT_TASK(CLIENT1, SetupClient1)
  {
    initClientWithPool(true, "__TEST_POOL1__", locHostPort, nullptr, nullptr, 0,
                       true);
    getHelper()->createPooledRegion(regionNames[0], false, locHostPort,
                                    "__TEST_POOL1__", true, true);
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    auto mtor = regPtr->getAttributesMutator();
    auto lster = std::make_shared<MyCacheListener>();
    auto wter = std::make_shared<MyCacheWriter>();
    mtor->setCacheListener(lster);
    mtor->setCacheWriter(wter);
    regPtr->registerAllKeys();
  }
END_TASK(SetupClient1)

DUNIT_TASK(CLIENT2, SetupClient2)
  {
    initClientWithPool(true, "__TEST_POOL1__", locHostPort, nullptr, nullptr, 0,
                       true);
    getHelper()->createPooledRegion(regionNames[0], false, locHostPort,
                                    "__TEST_POOL1__", true, true);
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    regPtr->registerAllKeys();
    auto keyPtr = CacheableKey::create("key01");
    auto valPtr = CacheableBytes::create(
        std::vector<int8_t>{'v', 'a', 'l', 'u', 'e', '0', '1'});
    regPtr->put(keyPtr, valPtr);
    ASSERT(regPtr->size() == 1, "size incorrect");
  }
END_TASK(SetupClient2)

DUNIT_TASK(CLIENT1, clear)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    ASSERT(regPtr->size() == 1, "size incorrect");
    regPtr->clear();
    ASSERT(regPtr->size() == 0, "size incorrect");
  }
END_TASK(clear)

DUNIT_TASK(CLIENT2, VerifyClear)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    ASSERT(regPtr->size() == 0, "size incorrect");
    auto keyPtr = CacheableKey::create("key02");
    auto valPtr = CacheableBytes::create(
        std::vector<int8_t>{'v', 'a', 'l', 'u', 'e', '0', '2'});
    regPtr->put(keyPtr, valPtr);
    ASSERT(regPtr->size() == 1, "size incorrect");
    regPtr->localClear();
    ASSERT(regPtr->size() == 0, "size incorrect");
    ASSERT(regPtr->containsKeyOnServer(keyPtr), "key should be there");
  }
END_TASK(VerifyClear)

DUNIT_TASK(CLIENT1, VerifyClear1)
  {
    auto region = getHelper()->getRegion(regionNames[0]);
    ASSERT(region->size() == 1, "size incorrect");

    region->localClear();
    ASSERT(region->size() == 0, "size incorrect");

    auto key = CacheableKey::create("key02");
    ASSERT(region->containsKeyOnServer(key), "key should be there");

    auto cacheListener = region->getAttributes().getCacheListener();
    auto myCacheListener =
        std::dynamic_pointer_cast<MyCacheListener>(cacheListener);
    auto listenerClearCount =
        "listener clear count" + std::to_string(myCacheListener->getClearCnt());
    LOG(listenerClearCount);
    ASSERT(myCacheListener->getClearCnt() == 2, listenerClearCount.c_str());

    auto cacheWriter = region->getAttributes().getCacheWriter();
    auto myCacheWriter = std::dynamic_pointer_cast<MyCacheWriter>(cacheWriter);
    auto writerClearCount =
        "writer clear count" + std::to_string(myCacheWriter->getClearCnt());
    LOG(writerClearCount);
    ASSERT(myCacheWriter->getClearCnt() == 2, writerClearCount.c_str());
  }
END_TASK(VerifyClear1)

DUNIT_TASK(SERVER1, StopServer)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      CacheHelper::closeLocator(1);
    }

    LOG("SERVER stopped");
  }
END_TASK(StopServer)
DUNIT_TASK(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK(CloseCache1)
DUNIT_TASK(CLIENT2, CloseCache2)
  { cleanProc(); }
END_TASK(CloseCache2)
