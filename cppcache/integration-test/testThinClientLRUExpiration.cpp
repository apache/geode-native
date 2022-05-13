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

#include <string>

#include <geode/internal/chrono/duration.hpp>

#define ROOT_NAME "testThinClientLRUExpiration"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"
#include "TallyListener.hpp"
#include "TallyWriter.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;
using apache::geode::client::EntryNotFoundException;
using apache::geode::client::Exception;
using apache::geode::client::ExpirationAction;
using apache::geode::client::RegionAttributes;
using apache::geode::client::RegionDestroyedException;

using apache::geode::client::testing::TallyListener;
using apache::geode::client::testing::TallyWriter;

CacheHelper *cacheHelper = nullptr;
bool isLocalServer = false;

static bool isLocator = false;
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);

const char *regionNames[] = {"DistRegionAck1", "DistRegionAck2",
                             "DistRegionAck3", "DistRegionAck4",
                             "DistRegionAck5", "DistRegionAck"};
const bool USE_ACK = true;
const bool NO_ACK = false;
std::shared_ptr<TallyListener> regListener;
std::shared_ptr<TallyWriter> regWriter;
bool registerKey = true;
void initClient(const bool isthinClient) {
  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(isthinClient);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}
void cleanProc() {
  if (cacheHelper != nullptr) {
    delete cacheHelper;
    cacheHelper = nullptr;
  }
}

CacheHelper *getHelper() {
  ASSERT(cacheHelper != nullptr, "No cacheHelper initialized.");
  return cacheHelper;
}
void printAttribute(RegionAttributes attr) {
  using apache::geode::internal::chrono::duration::to_string;

  std::cout << "CachingEnable: "
            << (attr.getCachingEnabled() ? "enabled" : "disabled") << "\n";
  std::cout << "InitialCapacity: " << attr.getInitialCapacity() << "\n";
  std::cout << "LoadFactor: " << attr.getLoadFactor() << "\n";
  std::cout << "ConcurencyLevel: " << attr.getConcurrencyLevel() << "\n";
  std::cout << "RegionTimeToLive: " << to_string(attr.getRegionTimeToLive())
            << "\n";
  std::cout << "RegionIdleTimeout: " << to_string(attr.getRegionIdleTimeout())
            << "\n";
  std::cout << "EntryTimeToLive: " << to_string(attr.getEntryTimeToLive())
            << "\n";
  std::cout << "EntryIdleTimeout: " << to_string(attr.getEntryIdleTimeout())
            << "\n";
  std::cout << "getLruEntriesLimit: " << attr.getLruEntriesLimit() << "\n";
  std::cout << "RegionTimeToLiveAction: "
            << static_cast<int>(attr.getRegionTimeToLiveAction()) << "\n";
  std::cout << "RegionIdleTimeoutAction: "
            << static_cast<int>(attr.getRegionIdleTimeoutAction()) << "\n";
  std::cout << "EntryTimeToLiveAction: "
            << static_cast<int>(attr.getEntryTimeToLiveAction()) << "\n";
  std::cout << "EntryIdleTimeoutAction: "
            << static_cast<int>(attr.getEntryIdleTimeoutAction()) << "\n";
  std::cout << "LruEvictionAction: "
            << static_cast<int>(attr.getLruEvictionAction()) << "\n";
  std::cout << "ClientNotification: "
            << (attr.getClientNotificationEnabled() ? "true" : "false") << "\n";
}

void setCacheListener(const char *regName,
                      std::shared_ptr<TallyListener> regionTallyListener) {
  auto reg = getHelper()->getRegion(regName);
  auto attrMutator = reg->getAttributesMutator();
  attrMutator->setCacheListener(regionTallyListener);
}

void setCacheWriter(const char *regName,
                    std::shared_ptr<TallyWriter> regionTallyWriter) {
  auto reg = getHelper()->getRegion(regName);
  auto attrMutator = reg->getAttributesMutator();
  attrMutator->setCacheWriter(regionTallyWriter);
}

void getRegionAttr(const char *name) {
  auto rptr = getHelper()->getRegion(name);
  auto m_currRegionAttributesPtr = rptr->getAttributes();
  printAttribute(m_currRegionAttributesPtr);
}

void ValidateDestroyRegion(const char *name) {
  auto rptr = getHelper()->getRegion(name);
  if (rptr == nullptr) {
    return;
  }
  try {
    rptr->put(1, 2);
    FAIL("Put should not be happened");
  } catch (RegionDestroyedException &ex) {
    LOG(std::string("Got expected exception ") + ex.getName() +
        ": msg = " + ex.what());
  } catch (Exception &ex) {
    FAIL(std::string("Got expected exception ") + ex.getName() +
         ": msg = " + ex.what());
  }
}

void createRegion(const char *name, bool ackMode,
                  const std::chrono::seconds &ettl,
                  const std::chrono::seconds &eit,
                  const std::chrono::seconds &rttl,
                  const std::chrono::seconds &rit, int lel,
                  ExpirationAction action = ExpirationAction::DESTROY) {
  std::cout << "Creating region --  " << name << " ackMode is " << ackMode
            << "\n"
            << std::flush;
  auto
      regPtr =  // getHelper()->createRegion( name, ackMode, true,
                // ettl,eit,rttl,rit,lel,action,endpoints,clientNotificationEnabled
                // );
      getHelper()->createRegionAndAttachPool(name, ackMode, "LRUPool", true,
                                             ettl, eit, rttl, rit, lel, action);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  if (registerKey) regPtr->registerAllKeys(false, false, false);
  LOG("Region created.");
}

void doRgnOperations(const char *name, int n, int rgnOpt = 0) {
  std::shared_ptr<CacheableString> value;
  if (rgnOpt == 0) {
    value = CacheableString::create("Value - AAAAAAA");
    ASSERT(value != nullptr, "Failed to create value.");
  }
  auto rptr = getHelper()->getRegion(name);
  ASSERT(rptr != nullptr, "Region not found.");
  for (int i = 0; i < n; i++) {
    auto keyStr = std::string("KeyA - ") + std::to_string(i + 1);
    auto key = CacheableKey::create(keyStr);
    switch (rgnOpt) {
      case 0:
        rptr->put(key, value);
        break;
      case 1:
        rptr->invalidate(key);
        break;
      case 2:
        rptr->localInvalidate(key);
        break;
      case 3:
        rptr->destroy(key);
        break;
      case 4:
        rptr->localDestroy(key);
        break;
      case 5:
        rptr->get(key);
        break;
    }
  }
}

void dumpCounters(const char *regName) {
  auto rptr = getHelper()->getRegion(regName);
  std::cout << "Region size: " << rptr->size() << "\n";
  if (regListener != nullptr) {
    std::cout << "counts:: creates: " << regListener->getCreates()
              << ", updates: " << regListener->getUpdates()
              << ", invalidates: " << regListener->getInvalidates()
              << ", destroys: " << regListener->getDestroys() << "\n";
  }
}

size_t getNumOfEntries(const char *regName, bool isValue = false) {
  static bool useRegionSize = false;

  useRegionSize = !useRegionSize;
  dumpCounters(regName);
  auto rptr = getHelper()->getRegion(regName);
  if (isValue) {
    auto v = rptr->values();
    std::cout << "Region value size: " << v.size() << "\n";
    return v.size();
  } else if (!useRegionSize) {
    auto v = rptr->keys();
    std::cout << "Region key size: " << v.size() << "\n";
    return v.size();
  } else {
    return rptr->size();
  }
}

void localDestroyRegion(const char *name) {
  LOG("localDestroyRegion() entered.");
  auto regPtr = getHelper()->getRegion(name);
  regPtr->localDestroyRegion();
  ASSERT(regPtr->isDestroyed() == true, "Expected Region to be destroyed");
  LOG("Locally Region destroyed.");
}

void createThinClientRegion(
    const char *regionName, const std::chrono::seconds &ettl,
    const std::chrono::seconds &eit, const std::chrono::seconds &rttl,
    const std::chrono::seconds &rit, int lel, int noOfEntry = 0, int rgnOpt = 0,
    bool destroyRgn = true,
    ExpirationAction action = ExpirationAction::DESTROY) {
  if (destroyRgn) {
    try {
      doRgnOperations(regionName, noOfEntry, rgnOpt);
    } catch (EntryNotFoundException &ex) {
      LOG(std::string("Got expected exception ") + ex.getName() +
          ": msg = " + ex.what());
    }
    localDestroyRegion(regionName);
  }
  createRegion(regionName, USE_ACK, ettl, eit, rttl, rit, lel, action);
  getRegionAttr(regionName);
}

DUNIT_TASK(SERVER1, CreateServer1)
  {
    if (isLocalServer) CacheHelper::initServer(1, "cacheserver4.xml");
    LOG("SERVER1 started");
  }
END_TASK(CreateServer1)

DUNIT_TASK(CLIENT1, StepOneCase1)
  {
    initClient(true);
    // regionName, ettl, eit , rttl, rit,lel,endpoints,noOfEntry,rgnOpetation
    // - [put-0/get-5/destroy-3] ,destroyRgn - [true/false]
    // ,clientNotificationEnabled - [true/false] ,ExpirationAction
    getHelper()->createPoolWithLocators("LRUPool", locatorsG, true);
    createThinClientRegion(regionNames[0], std::chrono::seconds(4),
                           std::chrono::seconds(2), std::chrono::seconds(0),
                           std::chrono::seconds(0), 0, 0, 6, false);
  }
END_TASK(StepOneCase1)

DUNIT_TASK(CLIENT2, StepTwoCase1)
  {
    initClient(true);
    // regionName, ettl, eit , rttl,
    // rit,lel,endpoints,noOfEntry,rgnOpetation - [put-0/get-5/destroy-3]
    // ,destroyRgn - [true/false] ,clientNotificationEnabled - [true/false]
    // ,ExpirationAction
    getHelper()->createPoolWithLocators("LRUPool", locatorsG, true);
    createThinClientRegion(regionNames[0], std::chrono::seconds(0),
                           std::chrono::seconds(0), std::chrono::seconds(0),
                           std::chrono::seconds(0), 0, 0, 6, false);
  }
END_TASK(StepTwoCase1)

DUNIT_TASK(CLIENT1, StepThreeCase1)
  {
    doRgnOperations(regionNames[0], 100);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 100, "Expected 100 entries");
    LOG("StepThree complete.");
  }
END_TASK(StepThreeCase1)
DUNIT_TASK(CLIENT2, StepFourCase1)
  {
    doRgnOperations(regionNames[0], 100, 5);
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 100, "Expected 100 entries");
    LOG("StepFour complete.");
  }
END_TASK(StepFourCase1)
DUNIT_TASK(CLIENT1, StepFiveCase1)
  {
    // wair 5 sec so all enteries gone
    std::this_thread::sleep_for(std::chrono::seconds(5));
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 0, "Expected 0 entries");
    LOG("StepFive complete.");
  }
END_TASK(StepFiveCase1)
DUNIT_TASK(CLIENT2, StepSixCase1)
  {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    // all enteris has been deleted
    // int n = getNumOfEntries(regionNames[0],true);
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 0, "Expected 0 entries");
    LOG("StepSix complete.");
  }
END_TASK(StepSixCase1)
DUNIT_TASK(CLIENT1, StepOneCase2)
  {
    // regionName, ettl, eit , rttl,
    // rit,lel,endpoints,noOfEntry,rgnOpetation -
    // [put-0/get-5/destroy-3] ,destroyRgn - [true/false]
    // ,clientNotificationEnabled - [true/false]
    // ,ExpirationAction
    createThinClientRegion(regionNames[0], std::chrono::seconds(4),
                           std::chrono::seconds(2), std::chrono::seconds(0),
                           std::chrono::seconds(0), 0, 100, 3, true,
                           ExpirationAction::LOCAL_INVALIDATE);
    LOG("StepOneCase2 complete.");
  }
END_TASK(StepOneCase2)

DUNIT_TASK(CLIENT2, StepTwoCase2)
  {
    // regionName, ettl, eit , rttl,
    // rit,lel,endpoints,noOfEntry,rgnOpetation -
    // [put-0/get-5/destroy-3] ,destroyRgn - [true/false]
    // ,clientNotificationEnabled - [true/false]
    // ,ExpirationAction
    createThinClientRegion(regionNames[0], std::chrono::seconds(0),
                           std::chrono::seconds(0), std::chrono::seconds(0),
                           std::chrono::seconds(0), 0, 100, 3);
    LOG("StepTwoCase2 complete.");
  }
END_TASK(StepTwoCase2)
DUNIT_TASK(CLIENT1, StepThreeCase2)
  {
    doRgnOperations(regionNames[0], 100);
    // 100 entry with invalidate
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 100, "Expected 100 entries");
    LOG("StepThreeCase2 complete.");
  }
END_TASK(StepThreeCase2)
DUNIT_TASK(CLIENT2, StepFourCase2)
  {
    doRgnOperations(regionNames[0], 100, 5);
    // should have 100
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 100, "Expected 100 entries");
    LOG("StepFourCase2 complete.");
  }
END_TASK(StepFourCase2)
DUNIT_TASK(CLIENT1, StepFiveCase2)
  {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 100, "Expected 100 entries");
    std::this_thread::sleep_for(std::chrono::seconds(5));
    // value should be invalidate as passing true
    n = getNumOfEntries(regionNames[0], true);
    ASSERT(n == 0, "Expected 0 entries");
    LOG("StepFiveCase2 complete.");
  }
END_TASK(StepFiveCase2)
DUNIT_TASK(CLIENT2, StepSixCase2)
  {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    auto n = getNumOfEntries(regionNames[0], true);
    ASSERT(n == 100, "Expected 100 entries");
    LOG("StepSixCase2 complete.");
  }
END_TASK(StepSixCase2)

DUNIT_TASK(CLIENT1, StepOneCase3)
  {
    // regionName, ettl, eit , rttl,
    // rit,lel,endpoints,noOfEntry,rgnOpetation -
    // [put-0/get-5/destroy-3] ,destroyRgn -
    // [true/false] ,clientNotificationEnabled -
    // [true/false] ,ExpirationAction
    createThinClientRegion(regionNames[0], std::chrono::seconds(0),
                           std::chrono::seconds(0), std::chrono::seconds(0),
                           std::chrono::seconds(0), 5, 10, 3);
  }
END_TASK(StepOneCase3)

DUNIT_TASK(CLIENT2, StepTwoCase3)
  {
    // regionName, ettl, eit , rttl,
    // rit,lel,endpoints,noOfEntry,rgnOpetation -
    // [put-0/get-5/destroy-3] ,destroyRgn -
    // [true/false] ,clientNotificationEnabled -
    // [true/false] ,ExpirationAction
    createThinClientRegion(regionNames[0], std::chrono::seconds(0),
                           std::chrono::seconds(0), std::chrono::seconds(0),
                           std::chrono::seconds(0), 0, 10, 3);
  }
END_TASK(StepTwoCase3)
DUNIT_TASK(CLIENT1, StepThreeCase3)
  {
    doRgnOperations(regionNames[0], 10);
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 5, "Expected 5 entries");
    LOG("StepThreeCase3 complete.");
  }
END_TASK(StepThreeCase3)
DUNIT_TASK(CLIENT2, StepFourCase3)
  {
    doRgnOperations(regionNames[0], 10, 5);
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 10, "Expected 10 entries");
    LOG("StepFourCase3 complete.");
  }
END_TASK(StepFourCase3)
DUNIT_TASK(CLIENT1, StepFiveCase3)
  {
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 5, "Expected 5 entries");
    n = getNumOfEntries(regionNames[0], true);
    ASSERT(n == 5, "Expected 5 entries");
    LOG("StepFiveCase3 complete.");
  }
END_TASK(StepFiveCase3)
DUNIT_TASK(CLIENT2, StepSixCase3)
  {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 10, "Expected 10 entries");
    n = getNumOfEntries(regionNames[0], true);
    ASSERT(n == 10, "Expected 10 entries");
    LOG("StepSixCase3 complete.");
  }
END_TASK(StepSixCase3)

DUNIT_TASK(CLIENT1, StepOneCase4)
  {
    // regionName, ettl, eit , rttl,
    // rit,lel,endpoints,noOfEntry,rgnOpetation
    // - [put-0/get-5/destroy-3]
    // ,destroyRgn - [true/false]
    // ,clientNotificationEnabled -
    // [true/false]
    // ,ExpirationAction
    createThinClientRegion(regionNames[0], std::chrono::seconds(5),
                           std::chrono::seconds(0), std::chrono::seconds(0),
                           std::chrono::seconds(0), 5, 10, 3, true,
                           ExpirationAction::LOCAL_INVALIDATE);
  }
END_TASK(StepOneCase4)

DUNIT_TASK(CLIENT2, StepTwoCase4)
  {
    // regionName, ettl, eit , rttl,
    // rit,lel,endpoints,noOfEntry,rgnOpetation
    // - [put-0/get-5/destroy-3]
    // ,destroyRgn - [true/false]
    // ,clientNotificationEnabled -
    // [true/false]
    // ,ExpirationAction
    createThinClientRegion(regionNames[0], std::chrono::seconds(0),
                           std::chrono::seconds(0), std::chrono::seconds(0),
                           std::chrono::seconds(0), 0, 10, 3);
  }
END_TASK(StepTwoCase4)
DUNIT_TASK(CLIENT1, StepThreeCase4)
  {
    doRgnOperations(regionNames[0], 10);
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 5, "Expected 5 entries");
    LOG("StepThreeCase4 complete.");
  }
END_TASK(StepThreeCase4)
DUNIT_TASK(CLIENT2, StepFourCase4)
  {
    doRgnOperations(regionNames[0], 10, 5);
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 10, "Expected 10 entries");
    LOG("StepFourCase4 complete.");
  }
END_TASK(StepFourCase4)
DUNIT_TASK(CLIENT1, StepFiveCase4)
  {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 5, "Expected 5 entries");
    n = getNumOfEntries(regionNames[0], true);
    ASSERT(n == 5, "Expected 5 entries");
    std::this_thread::sleep_for(std::chrono::seconds(4));
    n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 5, "Expected 5 entries");
    LOG("StepFiveCase4 "
        "complete.");
  }
END_TASK(StepFiveCase4)
DUNIT_TASK(CLIENT2, StepSixCase4)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 10, "Expected 10 entries");
    n = getNumOfEntries(regionNames[0], true);
    ASSERT(n == 10, "Expected 10 entries");
    LOG("StepSixCase4 "
        "complete.");
  }
END_TASK(StepSixCase4)

DUNIT_TASK(CLIENT1, StepOneCase5)
  {
    // regionName, ettl, eit ,
    // rttl,
    // rit,lel,endpoints,noOfEntry,rgnOpetation
    // -
    // [put-0/get-5/destroy-3]
    // ,destroyRgn -
    // [true/false]
    // ,clientNotificationEnabled
    // - [true/false]
    // ,ExpirationAction
    createThinClientRegion(regionNames[0], std::chrono::seconds(5),
                           std::chrono::seconds(0), std::chrono::seconds(0),
                           std::chrono::seconds(0), 5, 10, 3);
  }
END_TASK(StepOneCase5)

DUNIT_TASK(CLIENT2, StepTwoCase5)
  {
    // regionName, ettl, eit
    // , rttl,
    // rit,lel,endpoints,noOfEntry,rgnOpetation
    // -
    // [put-0/get-5/destroy-3]
    // ,destroyRgn -
    // [true/false]
    // ,clientNotificationEnabled
    // - [true/false]
    // ,ExpirationAction
    createThinClientRegion(regionNames[0], std::chrono::seconds(0),
                           std::chrono::seconds(0), std::chrono::seconds(0),
                           std::chrono::seconds(0), 0, 10, 3);
  }
END_TASK(StepTwoCase5)
DUNIT_TASK(CLIENT1, StepThreeCase5)
  {
    doRgnOperations(regionNames[0], 10);
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 5,
           "Expected 5 "
           "entries");
    LOG("StepTwoCase5 "
        "complete.");
  }
END_TASK(StepThreeCase5)
DUNIT_TASK(CLIENT2, StepFourCase5)
  {
    doRgnOperations(regionNames[0], 10, 5);
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 10,
           "Expected "
           "10 "
           "entries");
    LOG("StepFourCase5 "
        "complete.");
  }
END_TASK(StepFourCase5)
DUNIT_TASK(CLIENT1, StepFiveCase5)
  {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 5,
           "Expected "
           "5 "
           "entries");
    n = getNumOfEntries(regionNames[0], true);
    ASSERT(n == 5,
           "Expected "
           "5 "
           "entries");
    std::this_thread::sleep_for(std::chrono::seconds(4));
    n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 0,
           "Expected "
           "0 "
           "entries");
    LOG("StepFiveCase"
        "5 "
        "complete.");
  }
END_TASK(StepFiveCase5)
DUNIT_TASK(CLIENT2, StepSixCase5)
  {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 5,
           "Expecte"
           "d 5 "
           "entrie"
           "s");
    n = getNumOfEntries(regionNames[0], true);
    ASSERT(n == 5,
           "Expecte"
           "d 5 "
           "entrie"
           "s");
    LOG("StepSixCas"
        "e5 "
        "complete"
        ".");
  }
END_TASK(StepSixCase5)
DUNIT_TASK(CLIENT1, StepOneCase6)
  {
    // regionName,
    // ettl, eit ,
    // rttl,
    // rit,lel,endpoints,noOfEntry,rgnOpetation
    // -
    // [put-0/get-5/destroy-3]
    // ,destroyRgn
    // -
    // [true/false]
    // ,clientNotificationEnabled
    // -
    // [true/false]
    // ,ExpirationAction
    createThinClientRegion(regionNames[0], std::chrono::seconds(0),
                           std::chrono::seconds(5), std::chrono::seconds(0),
                           std::chrono::seconds(0), 5, 10, 3, true,
                           ExpirationAction::LOCAL_INVALIDATE);
  }
END_TASK(StepOneCase6)

DUNIT_TASK(CLIENT2, StepTwoCase6)
  {
    // regionName,
    // ettl, eit
    // , rttl,
    // rit,lel,endpoints,noOfEntry,rgnOpetation
    // -
    // [put-0/get-5/destroy-3]
    // ,destroyRgn
    // -
    // [true/false]
    // ,clientNotificationEnabled
    // -
    // [true/false]
    // ,ExpirationAction
    createThinClientRegion(regionNames[0], std::chrono::seconds(0),
                           std::chrono::seconds(0), std::chrono::seconds(0),
                           std::chrono::seconds(0), 0, 10, 3);
  }
END_TASK(StepTwoCase6)
DUNIT_TASK(CLIENT1, StepThreeCase6)
  {
    doRgnOperations(regionNames[0], 10);
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 5,
           "Expe"
           "cted"
           " 5 "
           "entr"
           "ie"
           "s");
    LOG("Step"
        "Thre"
        "eCas"
        "e6 "
        "comp"
        "lete"
        ".");
  }
END_TASK(StepThreeCase6)
DUNIT_TASK(CLIENT2, StepFourCase6)
  {
    doRgnOperations(regionNames[0], 10, 5);
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 10, "Expected 10 entries");
    LOG("StepFourCase6 complete.");
  }
END_TASK(StepFourCase6)
DUNIT_TASK(CLIENT1, StepFiveCase6)
  {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 5, "Expected 5 entries");
    n = getNumOfEntries(regionNames[0], true);
    ASSERT(n == 5, "Expected 5 entries");
    std::this_thread::sleep_for(std::chrono::seconds(4));
    n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 5, "Expected 5 entries");
    LOG("StepFiveCase6 complete.");
  }
END_TASK(StepFiveCase6)
DUNIT_TASK(CLIENT2, StepSixCase6)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 10, "Expected 10 entries");
    n = getNumOfEntries(regionNames[0], true);
    ASSERT(n == 10, "Expected 10 entries");
    LOG("StepSixCase6 complete.");
  }
END_TASK(StepSixCase6)
DUNIT_TASK(CLIENT1, StepOneCase7)
  {
    // regionName, ettl, eit , rttl, rit,lel,endpoints,noOfEntry,rgnOpetation -
    // [put-0/get-5/destroy-3] ,destroyRgn - [true/false]
    // ,clientNotificationEnabled - [true/false] ,ExpirationAction
    createThinClientRegion(regionNames[0], std::chrono::seconds(0),
                           std::chrono::seconds(0), std::chrono::seconds(10),
                           std::chrono::seconds(0), 0, 10, 3);
  }
END_TASK(StepOneCase7)

DUNIT_TASK(CLIENT2, StepTwoCase7)
  {
    // regionName, ettl, eit , rttl, rit,lel,endpoints,noOfEntry,rgnOpetation -
    // [put-0/get-5/destroy-3] ,destroyRgn - [true/false]
    // ,clientNotificationEnabled - [true/false] ,ExpirationAction
    createThinClientRegion(regionNames[0], std::chrono::seconds(0),
                           std::chrono::seconds(0), std::chrono::seconds(0),
                           std::chrono::seconds(0), 0, 10, 3);
  }
END_TASK(StepTwoCase7)
DUNIT_TASK(CLIENT1, StepThreeCase7)
  {
    doRgnOperations(regionNames[0], 10);
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 10, "Expected 10 entries");
    LOG("StepThreeCase7 complete.");
  }
END_TASK(StepThreeCase7)
DUNIT_TASK(CLIENT2, StepFourCase7)
  {
    doRgnOperations(regionNames[0], 10, 5);
    auto n = getNumOfEntries(regionNames[0]);
    ASSERT(n == 10, "Expected 10 entries");
    LOG("StepFourCase7 complete.");
  }
END_TASK(StepFourCase7)
DUNIT_TASK(CLIENT1, StepFiveCase7)
  {
    std::this_thread::sleep_for(std::chrono::seconds(15));
    ValidateDestroyRegion(regionNames[0]);
    LOG("StepFiveCase7 complete.");
  }
END_TASK(StepFiveCase7)
DUNIT_TASK(CLIENT2, StepSixCase7)
  {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    ValidateDestroyRegion(regionNames[0]);
    LOG("StepSixCase7 complete.");
  }
END_TASK(StepSixCase7)
DUNIT_TASK(CLIENT1, StepOneCase8)
  {
    std::this_thread::sleep_for(std::chrono::seconds(10));
    // regionName, ettl, eit , rttl, rit,lel,endpoints,noOfEntry,rgnOpetation -
    // [put-0/get-5/destroy-3] ,destroyRgn - [true/false]
    // ,clientNotificationEnabled - [true/false] ,ExpirationAction
    createThinClientRegion(regionNames[1], std::chrono::seconds(0),
                           std::chrono::seconds(0), std::chrono::seconds(8),
                           std::chrono::seconds(0), 0, 0, 6, false);
  }
END_TASK(StepOneCase8)

DUNIT_TASK(CLIENT2, StepTwoCase8)
  {
    // regionName, ettl, eit , rttl, rit,lel,endpoints,noOfEntry,rgnOpetation -
    // [put-0/get-5/destroy-3] ,destroyRgn - [true/false]
    // ,clientNotificationEnabled - [true/false] ,ExpirationAction
    createThinClientRegion(regionNames[1], std::chrono::seconds(0),
                           std::chrono::seconds(0), std::chrono::seconds(0),
                           std::chrono::seconds(0), 0, 0, 6, false);
  }
END_TASK(StepTwoCase8)
DUNIT_TASK(CLIENT1, StepThreeCase8)
  {
    doRgnOperations(regionNames[1], 10);
    auto n = getNumOfEntries(regionNames[1]);
    ASSERT(n == 10, "Expected 10 entries");
    LOG("StepThreeCase8 complete.");
  }
END_TASK(StepThreeCase8)
DUNIT_TASK(CLIENT2, StepFourCase8)
  {
    doRgnOperations(regionNames[1], 10, 5);
    auto n = getNumOfEntries(regionNames[1]);
    ASSERT(n == 10, "Expected 10 entries");
    LOG("StepFourCase8 complete.");
  }
END_TASK(StepFourCase8)
DUNIT_TASK(CLIENT1, StepFiveCase8)
  {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    auto n = getNumOfEntries(regionNames[1]);
    ASSERT(n == 10, "Expected 0 entries");
    std::this_thread::sleep_for(std::chrono::seconds(10));
    ValidateDestroyRegion(regionNames[1]);
    LOG("StepFiveCase8 complete.");
  }
END_TASK(StepFiveCase8)
DUNIT_TASK(CLIENT2, StepSixCase8)
  {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ValidateDestroyRegion(regionNames[1]);
    LOG("StepSixCase8 complete.");
  }
END_TASK(StepSixCase8)
DUNIT_TASK(CLIENT1, StepOneCase9)
  {
    // regionName, ettl, eit , rttl, rit,lel,endpoints,noOfEntry,rgnOpetation -
    // [put-0/get-5/destroy-3] ,destroyRgn - [true/false]
    // ,clientNotificationEnabled - [true/false] ,ExpirationAction
    createThinClientRegion(regionNames[2], std::chrono::seconds(4),
                           std::chrono::seconds(0), std::chrono::seconds(8),
                           std::chrono::seconds(0), 5, 0, 6, false);
  }
END_TASK(StepOneCase9)

DUNIT_TASK(CLIENT2, StepTwoCase9)
  {
    // regionName, ettl, eit , rttl, rit,lel,endpoints,noOfEntry,rgnOpetation -
    // [put-0/get-5/destroy-3] ,destroyRgn - [true/false]
    // ,clientNotificationEnabled - [true/false] ,ExpirationAction
    createThinClientRegion(regionNames[2], std::chrono::seconds(0),
                           std::chrono::seconds(0), std::chrono::seconds(0),
                           std::chrono::seconds(0), 0, 0, 6, false);
  }
END_TASK(StepTwoCase9)
DUNIT_TASK(CLIENT1, StepThreeCase9)
  {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    doRgnOperations(regionNames[2], 10);
    auto n = getNumOfEntries(regionNames[2]);
    ASSERT(n == 5, "Expected 5 entries");
    LOG("StepThreeCase9 complete.");
  }
END_TASK(StepThreeCase9)
DUNIT_TASK(CLIENT2, StepFourCase9)
  {
    doRgnOperations(regionNames[2], 10, 5);
    auto n = getNumOfEntries(regionNames[2]);
    ASSERT(n == 10, "Expected 10 entries");
    LOG("StepFourCase9 complete.");
  }
END_TASK(StepFourCase9)
DUNIT_TASK(CLIENT1, StepFiveCase9)
  {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    auto n = getNumOfEntries(regionNames[2]);
    ASSERT(n == 0, "Expected 0 entries");
    std::this_thread::sleep_for(std::chrono::seconds(8));
    ValidateDestroyRegion(regionNames[2]);
    LOG("StepFiveCase9 complete.");
  }
END_TASK(StepFiveCase9)
DUNIT_TASK(CLIENT2, StepSixCase9)
  {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    ValidateDestroyRegion(regionNames[2]);
    LOG("StepSixCase9 complete.");
  }
END_TASK(StepSixCase9)
DUNIT_TASK(CLIENT1, StepOneCase10)
  {
    // regionName, ettl, eit , rttl, rit,lel,endpoints,noOfEntry,rgnOpetation -
    // [put-0/get-5/destroy-3] ,destroyRgn - [true/false]
    // ,clientNotificationEnabled - [true/false] ,ExpirationAction
    createThinClientRegion(regionNames[3], std::chrono::seconds(4),
                           std::chrono::seconds(0), std::chrono::seconds(8),
                           std::chrono::seconds(0), 5, 0, 6, false);
  }
END_TASK(StepOneCase10)

DUNIT_TASK(CLIENT2, StepTwoCase10)
  {
    // regionName, ettl, eit , rttl, rit,lel,endpoints,noOfEntry,rgnOpetation -
    // [put-0/get-5/destroy-3] ,destroyRgn - [true/false]
    // ,clientNotificationEnabled - [true/false] ,ExpirationAction
    createThinClientRegion(regionNames[3], std::chrono::seconds(0),
                           std::chrono::seconds(0), std::chrono::seconds(0),
                           std::chrono::seconds(0), 0, 0, 6, false);
  }
END_TASK(StepTwoCase10)
DUNIT_TASK(CLIENT1, StepThreeCase10)
  {
    doRgnOperations(regionNames[3], 10);
    auto n = getNumOfEntries(regionNames[3]);
    ASSERT(n == 5, "Expected 5 entries");
    LOG("StepThreeCase10 complete.");
  }
END_TASK(StepThreeCase10)
DUNIT_TASK(CLIENT2, StepFourCase10)
  {
    doRgnOperations(regionNames[3], 10, 5);
    auto n = getNumOfEntries(regionNames[3]);
    ASSERT(n == 10, "Expected 10 entries");
    LOG("StepFourCase10 complete.");
  }
END_TASK(StepFourCase10)
DUNIT_TASK(CLIENT1, StepFiveCase10)
  {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    auto n = getNumOfEntries(regionNames[3]);
    ASSERT(n == 0, "Expected 0 entries");
    std::this_thread::sleep_for(std::chrono::seconds(10));
    ValidateDestroyRegion(regionNames[3]);
    LOG("StepFiveCase10 complete.");
  }
END_TASK(StepFiveCase10)
DUNIT_TASK(CLIENT2, StepSixCase10)
  {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    ValidateDestroyRegion(regionNames[3]);
    LOG("StepSixCase10 complete.");
  }
END_TASK(StepSixCase10)

// tests for local and distributed listener/writer invocation with expiration
DUNIT_TASK(CLIENT1, StepOneCase11)
  {
    // regionName, ettl, eit , rttl, rit,lel,endpoints,noOfEntry,rgnOpetation -
    // [put-0/get-5/destroy-3] ,destroyRgn - [true/false]
    // ,clientNotificationEnabled - [true/false] ,ExpirationAction
    createThinClientRegion(regionNames[4], std::chrono::seconds(4),
                           std::chrono::seconds(0), std::chrono::seconds(0),
                           std::chrono::seconds(0), 5, 0, 6, false);
    regListener = std::make_shared<TallyListener>();
    regWriter = std::make_shared<TallyWriter>();
    setCacheListener(regionNames[4], regListener);
    setCacheWriter(regionNames[4], regWriter);
  }
END_TASK(StepOneCase11)

DUNIT_TASK(CLIENT2, StepTwoCase11)
  {
    // regionName, ettl, eit , rttl, rit,lel,endpoints,noOfEntry,rgnOpetation -
    // [put-0/get-5/destroy-3] ,destroyRgn - [true/false]
    // ,clientNotificationEnabled - [true/false] ,ExpirationAction
    createThinClientRegion(regionNames[4], std::chrono::seconds(0),
                           std::chrono::seconds(0), std::chrono::seconds(0),
                           std::chrono::seconds(0), 0, 0, 6, false);
    regListener = std::make_shared<TallyListener>();
    regWriter = std::make_shared<TallyWriter>();
    setCacheListener(regionNames[4], regListener);
    setCacheWriter(regionNames[4], regWriter);
  }
END_TASK(StepTwoCase11)
DUNIT_TASK(CLIENT1, StepThreeCase11)
  {
    doRgnOperations(regionNames[4], 10);
    auto n = getNumOfEntries(regionNames[4]);
    regListener->showTallies();
    ASSERT(regWriter->isWriterInvoked() == true, "Writer Should be invoked");
    ASSERT(regListener->isListenerInvoked() == true,
           "Listener Should be invoked");

    ASSERT(regListener->getCreates() == 10, "Should be 10 creates");
    ASSERT(regListener->getUpdates() == 0, "Should be 0 updates");
    ASSERT(regListener->getInvalidates() == 0, "Should be 0 invalidate");
    ASSERT(regListener->getDestroys() == 5, "Should be 5 destroy");
    ASSERT(n == 5, "Expected 5 entries");
    LOG("StepThreeCase11 complete.");
  }
END_TASK(StepThreeCase11)
DUNIT_TASK(CLIENT2, StepFourCase11)
  {
    auto n = getNumOfEntries(regionNames[4]);
    regListener->showTallies();
    ASSERT(regWriter->isWriterInvoked() == false,
           "Writer Should not be invoked");
    ASSERT(regListener->isListenerInvoked() == true,
           "Listener Should be invoked");
    ASSERT(regListener->getCreates() == 0, "Should be 0 creates");
    ASSERT(regListener->getUpdates() == 0, "Should be 0 updates");
    // NBS=false will lead to only invalidate messages
    ASSERT(regListener->getInvalidates() == 10, "Should be 10 invalidate");
    // LRU action is LOCAL_DESTROY so no destroys should be received here
    ASSERT(regListener->getDestroys() == 0, "Should be 0 destroy");

    // expect zero entries in NBS=false case
    /*NIL: Changed the asserion to the change in invalidate.
      Now we create new entery for every invalidate event received or
      localInvalidate call
      so expect 10 entries instead of 0 earlier. */
    ASSERT(n == 10, "Expected 10 entries");
    LOG("StepFourCase11 complete.");
  }
END_TASK(StepFourCase11)
DUNIT_TASK(CLIENT1, StepFiveCase11)
  {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    auto n = getNumOfEntries(regionNames[4]);

    ASSERT(regWriter->isWriterInvoked() == true, "Writer Should be invoked");
    ASSERT(regListener->isListenerInvoked() == true,
           "Listener Should be invoked");
    regListener->showTallies();
    ASSERT(regListener->getCreates() == 10, "Should be 10 creates");
    ASSERT(regListener->getUpdates() == 0, "Should be 0 updates");
    ASSERT(regListener->getInvalidates() == 0, "Should be 0 invalidate");
    ASSERT(regListener->getDestroys() == 10, "Should be 10 destroy");

    ASSERT(n == 0, "Expected 0 entries");
    LOG("StepFiveCase11 complete.");
  }
END_TASK(StepFiveCase11)
DUNIT_TASK(CLIENT2, StepSixCase11)
  {
    auto n = getNumOfEntries(regionNames[4]);
    ASSERT(regWriter->isWriterInvoked() == false,
           "Writer Should not be invoked");
    ASSERT(regListener->isListenerInvoked() == true,
           "Listener Should be invoked");
    regListener->showTallies();
    ASSERT(regListener->getCreates() == 0, "Should be 0 creates");
    ASSERT(regListener->getUpdates() == 0, "Should be 0 updates");
    // NBS=false will lead to only invalidate messages
    ASSERT(regListener->getInvalidates() == 10, "Should be 10 invalidate");
    ASSERT(regListener->getDestroys() == 5, "Should be 5 destroy");

    // expect zero entries in NBS=false case
    /*NIL: Changed the asserion to the change in invalidate.
      Now we create new entery for every invalidate event received or
      localInvalidate call
      so expect 5 entries instead of 0 earlier. */
    ASSERT(n == 5, "Expected 5 entries");

    std::this_thread::sleep_for(std::chrono::seconds(3));
    LOG("StepSixCase11 complete.");
  }
END_TASK(StepSixCase11)

DUNIT_TASK(SERVER1, CloseServer1)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
  }
END_TASK(CloseServer1)
DUNIT_TASK(SERVER1, CreateServer1)
  {
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_notify_subscription.xml");
    }
    LOG("SERVER2 started");
  }
END_TASK(CreateServer1)

DUNIT_TASK(CLIENT1, RegisterKeyC1)
  { registerKey = false; }
END_TASK(RegisterKeyC1)

DUNIT_TASK(CLIENT2, RegisterKeyC2)
  { registerKey = false; }
END_TASK(RegisterKeyC2)

DUNIT_TASK(CLIENT1, StepOneCase12)
  {
    // regionName, ettl, eit , rttl, rit,lel,endpoints,noOfEntry,rgnOpetation -
    // [put-0/get-5/destroy-3] ,destroyRgn - [true/false]
    // ,clientNotificationEnabled - [true/false] ,ExpirationAction
    createThinClientRegion(regionNames[5], std::chrono::seconds(4),
                           std::chrono::seconds(0), std::chrono::seconds(0),
                           std::chrono::seconds(0), 5, 0, 6, false);
    regListener = std::make_shared<TallyListener>();
    regWriter = std::make_shared<TallyWriter>();
    setCacheListener(regionNames[5], regListener);
    setCacheWriter(regionNames[5], regWriter);
  }
END_TASK(StepOneCase12)

DUNIT_TASK(CLIENT2, StepTwoCase12)
  {
    // regionName, ettl, eit , rttl, rit,lel,endpoints,noOfEntry,rgnOpetation -
    // [put-0/get-5/destroy-3] ,destroyRgn - [true/false]
    // ,clientNotificationEnabled - [true/false] ,ExpirationAction
    createThinClientRegion(regionNames[5], std::chrono::seconds(0),
                           std::chrono::seconds(0), std::chrono::seconds(0),
                           std::chrono::seconds(0), 0, 0, 6, false);
    regListener = std::make_shared<TallyListener>();
    regWriter = std::make_shared<TallyWriter>();
    setCacheListener(regionNames[5], regListener);
    setCacheWriter(regionNames[5], regWriter);
  }
END_TASK(StepTwoCase12)
DUNIT_TASK(CLIENT1, StepThreeCase12)
  {
    doRgnOperations(regionNames[5], 10);
    auto n = getNumOfEntries(regionNames[5]);
    regListener->showTallies();
    ASSERT(regWriter->isWriterInvoked() == true, "Writer Should be invoked");
    ASSERT(regListener->isListenerInvoked() == true,
           "Listener Should be invoked");

    ASSERT(regListener->getCreates() == 10, "Should be 10 creates");
    ASSERT(regListener->getUpdates() == 0, "Should be 0 updates");
    // invalidate message is not implemented so expecting 0 events..
    ASSERT(regListener->getInvalidates() == 0, "Should be 0 invalidate");
    ASSERT(regListener->getDestroys() == 5, "Should be 5 destroy");
    ASSERT(n == 5, "Expected 5 entries");
    LOG("StepThreeCase12 complete.");
  }
END_TASK(StepThreeCase12)
DUNIT_TASK(CLIENT2, StepFourCase12)
  {
    auto n = getNumOfEntries(regionNames[5]);
    ASSERT(regWriter->isWriterInvoked() == false,
           "Writer Should not be invoked");
    ASSERT(regListener->isListenerInvoked() == false,
           "Listener Should not be invoked");

    ASSERT(n == 0, "Expected 0 entries");
    LOG("StepFourCase12 complete.");
  }
END_TASK(StepFourCase12)
DUNIT_TASK(CLIENT1, StepFiveCase12)
  {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    auto n = getNumOfEntries(regionNames[5]);

    ASSERT(regWriter->isWriterInvoked() == true, "Writer Should be invoked");
    ASSERT(regListener->isListenerInvoked() == true,
           "Listener Should be invoked");
    regListener->showTallies();
    ASSERT(regListener->getCreates() == 10, "Should be 10 creates");
    ASSERT(regListener->getUpdates() == 0, "Should be 0 updates");
    // invalidate message is not implemented so expecting 0 events..
    ASSERT(regListener->getInvalidates() == 0, "Should be 0 invalidate");
    ASSERT(regListener->getDestroys() == 10, "Should be 10 destroy");

    ASSERT(n == 0, "Expected 0 entries");
    LOG("StepFiveCase12 complete.");
  }
END_TASK(StepFiveCase12)
DUNIT_TASK(CLIENT2, StepSixCase12)
  {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    auto n = getNumOfEntries(regionNames[5]);
    ASSERT(regWriter->isWriterInvoked() == false,
           "Writer Should not be invoked");
    ASSERT(regListener->isListenerInvoked() == false,
           "Listener Should not be invoked");
    ASSERT(n == 0, "Expected 0 entries");
    LOG("StepSixCase12 complete.");
  }
END_TASK(StepSixCase12)
DUNIT_TASK(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK(CloseCache1)

DUNIT_TASK(CLIENT2, CloseCache2)
  { cleanProc(); }
END_TASK(CloseCache2)

DUNIT_TASK(CLIENT1, StepOneCase13)
  {
    // regionName, ettl, eit , rttl, rit,lel,endpoints,noOfEntry,rgnOpetation -
    // [put-0/get-5/destroy-3] ,destroyRgn - [true/false]
    // ,clientNotificationEnabled - [true/false] ,ExpirationAction
    initClient(true);
    getHelper()->createPoolWithLocators("LRUPool", locatorsG, true);
    createThinClientRegion(regionNames[5], std::chrono::seconds(4),
                           std::chrono::seconds(0), std::chrono::seconds(0),
                           std::chrono::seconds(0), 5, 0, 6, false);
    regListener = std::make_shared<TallyListener>();
    regWriter = std::make_shared<TallyWriter>();
    setCacheListener(regionNames[5], regListener);
    setCacheWriter(regionNames[5], regWriter);
  }
END_TASK(StepOneCase13)

DUNIT_TASK(CLIENT2, StepTwoCase13)
  {
    // regionName, ettl, eit , rttl, rit,lel,endpoints,noOfEntry,rgnOpetation -
    // [put-0/get-5/destroy-3] ,destroyRgn - [true/false]
    // ,clientNotificationEnabled - [true/false] ,ExpirationAction
    initClient(true);
    getHelper()->createPoolWithLocators("LRUPool", locatorsG, true);
    createThinClientRegion(regionNames[5], std::chrono::seconds(0),
                           std::chrono::seconds(0), std::chrono::seconds(0),
                           std::chrono::seconds(0), 0, 0, 6, false);
    regListener = std::make_shared<TallyListener>();
    regWriter = std::make_shared<TallyWriter>();
    setCacheListener(regionNames[5], regListener);
    setCacheWriter(regionNames[5], regWriter);
    auto regPtr0 = getHelper()->getRegion(regionNames[5]);
    regPtr0->registerAllKeys();
  }
END_TASK(StepTwoCase13)

DUNIT_TASK(CLIENT1, StepThreeCase13)
  {
    doRgnOperations(regionNames[5], 10);
    auto n = getNumOfEntries(regionNames[5]);
    regListener->showTallies();
    ASSERT(regWriter->isWriterInvoked() == true, "Writer Should be invoked");
    ASSERT(regListener->isListenerInvoked() == true,
           "Listener Should be invoked");

    ASSERT(regListener->getCreates() == 10, "Should be 10 creates");
    ASSERT(regListener->getUpdates() == 0, "Should be 0 updates");
    // invalidate message is not implemented so expecting 0 events..
    ASSERT(regListener->getInvalidates() == 0, "Should be 0 invalidate");
    ASSERT(regListener->getDestroys() == 5, "Should be 5 destroy");
    ASSERT(n == 5, "Expected 5 entries");
    LOG("StepThreeCase13 complete.");
  }
END_TASK(StepThreeCase13)
DUNIT_TASK(CLIENT2, StepFourCase13)
  {
    auto n = getNumOfEntries(regionNames[5]);
    ASSERT(regWriter->isWriterInvoked() == false,
           "Writer Should not be invoked");
    ASSERT(regListener->isListenerInvoked() == true,
           "Listener Should be invoked");
    regListener->showTallies();
    // five keys that have not been destroyed in previous test case are there on
    // the server so we receive creates for 5 and updates for the other 5
    ASSERT(regListener->getCreates() == 5, "Should be 5 creates");
    ASSERT(regListener->getUpdates() == 5, "Should be 5 updates");
    // invalidate message is not implemented so expecting 0 events..
    ASSERT(regListener->getInvalidates() == 0, "Should be 0 invalidate");
    ASSERT(regListener->getDestroys() == 0, "Should be 0 destroy");

    ASSERT(n == 10, "Expected 10 entries");
    LOG("StepFourCase13 complete.");
  }
END_TASK(StepFourCase13)
DUNIT_TASK(CLIENT1, StepFiveCase13)
  {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    auto n = getNumOfEntries(regionNames[5]);

    ASSERT(regWriter->isWriterInvoked() == true, "Writer Should be invoked");
    ASSERT(regListener->isListenerInvoked() == true,
           "Listener Should be invoked");
    regListener->showTallies();
    ASSERT(regListener->getCreates() == 10, "Should be 10 creates");
    ASSERT(regListener->getUpdates() == 0, "Should be 0 updates");
    // invalidate message is not implemented so expecting 0 events..
    ASSERT(regListener->getInvalidates() == 0, "Should be 0 invalidate");
    ASSERT(regListener->getDestroys() == 10, "Should be 10 destroy");

    ASSERT(n == 0, "Expected 0 entries");
    LOG("StepFiveCase13 complete.");
  }
END_TASK(StepFiveCase13)
DUNIT_TASK(CLIENT2, StepSixCase13)
  {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    auto n = getNumOfEntries(regionNames[5]);
    ASSERT(regWriter->isWriterInvoked() == false,
           "Writer Should not be invoked");
    ASSERT(regListener->isListenerInvoked() == true,
           "Listener Should be invoked");
    regListener->showTallies();
    // listener now shows updates for update events received from the server
    ASSERT(regListener->getCreates() == 5, "Should be 5 creates");
    ASSERT(regListener->getUpdates() == 5, "Should be 5 updates");
    // invalidate message is not implemented so expecting 0 events..
    ASSERT(regListener->getInvalidates() == 0, "Should be 0 invalidate");
    ASSERT(regListener->getDestroys() == 5, "Should be 5 destroy");

    ASSERT(n == 5, "Expected 5 entries");

    LOG("StepSixCase13 complete.");
  }
END_TASK(StepSixCase13)
DUNIT_TASK(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK(CloseCache1)

DUNIT_TASK(CLIENT2, CloseCache2)
  { cleanProc(); }
END_TASK(CloseCache2)

DUNIT_TASK(SERVER1, CloseServer1)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
  }
END_TASK(CloseServer1)
