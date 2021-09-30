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

#include <cinttypes>
#include <sstream>

#include <geode/internal/geode_base.hpp>

#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"
#include "testUtils.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1

#define MEGABYTE (1024 * 1024)
#define GROWTH 5 * MEGABYTE
#define MAX_PAYLOAD 20 * MEGABYTE
#define MAX_PUTS 100000

using apache::geode::client::CacheableBytes;
using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableInt64;

using unitTests::TestUtils;

void grow(int *iptr) { *iptr = *iptr + GROWTH; }

void putSize(std::shared_ptr<Region> &rptr, const char *buf, int size) {
  auto keyPtr = CacheableKey::create(buf);
  uint8_t base = 0;

  uint8_t *valbuf = new uint8_t[size + 1];
  for (int i = 0; i <= size; i++) {
    valbuf[i] = base;
    if (base == 255) {
      base = 0;
    } else {
      base++;
    }
  }
  auto valPtr =
      CacheableBytes::create(std::vector<int8_t>(valbuf, valbuf + size));
  // auto valPtr = CacheableString::create( valbuf);
  auto msg = std::string("put key: ") + buf +
             ", with value size : " + std::to_string(size);
  LOG(std::string("about to ") + msg);
  rptr->put(keyPtr, valPtr);
  delete[] valbuf;
  LOG(msg);
}

void verify(std::shared_ptr<CacheableBytes> &valuePtr, int size) {
  ASSERT(size == 0 || valuePtr != nullptr,
         std::string("verifying value of size ") + std::to_string(size));
  int tryCnt = 0;
  bool notIt = true;
  int valSize = (valuePtr != nullptr ? valuePtr->length() : 0);
  while ((notIt == true) && (tryCnt++ < 10)) {
    notIt = (valSize != size);
    SLEEP(100);
  }

  ASSERT(valSize == size,
         std::string("value size is not %d") + std::to_string(size));

  auto &&bytes = reinterpret_cast<const uint8_t *>(valuePtr->value().data());
  uint8_t base = 0;
  for (int i = 0; i < size; i++) {
    if (bytes[i] != base) {
      std::stringstream strm;
      strm << "verifying buf[" << i << "] == " << base << " for size " << size
           << ", found " << valuePtr->value()[i] << " instead";
      ASSERT(false, strm.str());
    }
    if (base == 255) {
      base = 0;
    } else {
      base++;
    }
  }
}

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
  }
END_TASK(SetupClient1)

DUNIT_TASK(CLIENT2, SetupClient2)
  {
    initClientWithPool(true, "__TEST_POOL1__", locHostPort, nullptr, nullptr, 0,
                       true);
    getHelper()->createPooledRegion(regionNames[0], false, locHostPort,
                                    "__TEST_POOL1__", true, true);
  }
END_TASK(SetupClient2)

DUNIT_TASK(CLIENT1, puts)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    char buf[1024];
    LOG("Beginning puts.");
    int expectEntries = 0;
    for (int i = 0; i <= MAX_PAYLOAD; grow(&i)) {
      LOG(std::string("put size=") + std::to_string(i));
      auto keyName =
          std::string("key") + TestUtils::zeroPaddedStringFromInt(i, 10);
      putSize(regPtr, keyName.c_str(), i);
      expectEntries++;
    }

    dunit::globals().find_or_construct<int>("entriesToExpect")(expectEntries);
    LOG("Finished putting entries.");
  }
END_TASK(puts)

DUNIT_TASK(CLIENT2, VerifyPuts)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    // region should already have n entries...

    for (int i = 0; i <= MAX_PAYLOAD; grow(&i)) {
      auto keyName =
          std::string("key") + TestUtils::zeroPaddedStringFromInt(i, 10);
      auto keyPtr = CacheableKey::create(keyName);
      auto valPtr =
          std::dynamic_pointer_cast<CacheableBytes>(regPtr->get(keyPtr));
      int ntry = 20;
      while ((ntry-- > 0) && (valPtr == nullptr)) {
        SLEEP(200);
        valPtr = std::dynamic_pointer_cast<CacheableBytes>(regPtr->get(keyPtr));
      }
      LOG("from VerifyPuts");
      verify(valPtr, i);
    }
    LOG("On client Found all entries with correct size via get.");
  }
END_TASK(VerifyPuts)

DUNIT_TASK(CLIENT1, ManyPuts)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    LOG("Beginning many puts.");
    int expectEntries = 0;
    char keybuf[100];
    char valbuf[200];
    for (int index = 0; index < MAX_PUTS; ++index) {
      auto key =
          std::string("keys1") + TestUtils::zeroPaddedStringFromInt(index, 10);
      auto value = std::string("values1") +
                   TestUtils::zeroPaddedStringFromInt(index, 100);
      regPtr->put(key, value);
      expectEntries++;
    }

    dunit::globals().find_or_construct<int>("entriesToExpect")(expectEntries);
    LOG("Finished putting entries.");
  }
END_TASK(ManyPuts)

DUNIT_TASK(CLIENT2, VerifyManyPuts)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    // region should already have n entries...
    auto entriesExpected = dunit::globals().find<int>("entriesToExpect").first;
    ASSERT(entriesExpected != nullptr, "entriesExpected is null");

    for (int index = 0; index < *entriesExpected; ++index) {
      auto key =
          std::string("keys1") + TestUtils::zeroPaddedStringFromInt(index, 10);
      auto valPtr =
          std::dynamic_pointer_cast<CacheableString>(regPtr->get(key));
      ASSERT(valPtr != nullptr, "expected non-null value");
      ASSERT(valPtr->length() == 107, "unexpected size of value in verify");
    }
    LOG("On client Found all entries with correct size via get.");
  }
END_TASK(VerifyManyPuts)

DUNIT_TASK(CLIENT1, UpdateManyPuts)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    LOG("Beginning updated many puts.");
    int expectEntries = 0;
    for (int index = 0; index < MAX_PUTS; ++index) {
      auto key =
          std::string("keys1") + TestUtils::zeroPaddedStringFromInt(index, 10);
      auto value = std::string("values2") +
                   TestUtils::zeroPaddedStringFromInt(index, 1000);
      regPtr->put(key, value);
      expectEntries++;
    }

    dunit::globals().find_or_construct<int>("entriesToExpect")(expectEntries);
    LOG("Finished putting entries.");
  }
END_TASK(UpdateManyPuts)

DUNIT_TASK(CLIENT2, VerifyOldManyPuts)
  {
    // region should given old entries from cache
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    auto entriesExpected = dunit::globals().find<int>("entriesToExpect").first;
    ASSERT(entriesExpected != nullptr, "entriesExpected is null");

    for (int index = 0; index < *entriesExpected; ++index) {
      auto key =
          std::string("keys1") + TestUtils::zeroPaddedStringFromInt(index, 10);
      auto valPtr =
          std::dynamic_pointer_cast<CacheableString>(regPtr->get(key));
      ASSERT(valPtr != nullptr, "expected non-null value");
      ASSERT(valPtr->length() == 107, "unexpected size of value in verify");
    }
    LOG("On client Found all entries with correct size via "
        "get.");
  }
END_TASK(VerifyOldManyPuts)

DUNIT_TASK(CLIENT2, VerifyUpdatedManyPuts)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    // region should already have n entries...
    auto entriesExpected = dunit::globals().find<int>("entriesToExpect").first;
    ASSERT(entriesExpected != nullptr, "entriesExpected is null");

    // invalidate the region to force getting new values
    regPtr->localInvalidateRegion();

    for (int index = 0; index < *entriesExpected; ++index) {
      auto key =
          std::string("keys1") + TestUtils::zeroPaddedStringFromInt(index, 10);
      auto valPtr =
          std::dynamic_pointer_cast<CacheableString>(regPtr->get(key));
      ASSERT(valPtr != nullptr, "expected non-null value");
      ASSERT(valPtr->length() == 1007, "unexpected size of value in verify");
    }
    LOG("On client Found all entries with correct size via "
        "get.");
  }
END_TASK(VerifyUpdatedManyPuts)

DUNIT_TASK(CLIENT2, VerifyUpdatedManyPutsGetAll)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    // region should already have n entries...
    auto entriesExpected = dunit::globals().find<int>("entriesToExpect").first;
    ASSERT(entriesExpected != nullptr, "entriesExpected is null");

    // invalidate the region to force getting new values
    regPtr->localInvalidateRegion();

    std::vector<std::shared_ptr<CacheableKey>> vec;
    for (int index = 0; index < *entriesExpected; ++index) {
      auto key =
          std::string("keys1") + TestUtils::zeroPaddedStringFromInt(index, 10);
      vec.push_back(CacheableKey::create(key));
    }
    regPtr->getAll(vec);
    LOG("On client getAll for entries completed.");
    for (int index = 0; index < *entriesExpected; ++index) {
      auto key =
          std::string("keys1") + TestUtils::zeroPaddedStringFromInt(index, 10);
      auto valPtr =
          std::dynamic_pointer_cast<CacheableString>(regPtr->get(key));
      ASSERT(valPtr != nullptr, "expected non-null value");
      ASSERT(valPtr->length() == 1007, "unexpected size of value in verify");
    }
    LOG("On client Found all entries with correct size via "
        "getAll.");
  }
END_TASK(VerifyUpdatedManyPutsGetAll)

DUNIT_TASK(CLIENT1, UpdateManyPutsInt64)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    LOG("Beginning updated many puts for int64.");
    int expectEntries = 0;
    char valbuf[1100];
    for (int64_t index = 0; index < MAX_PUTS; ++index) {
      int64_t key = index * index * index;
      auto value =
          std::string("values3") +
          TestUtils::zeroPaddedStringFromInt(static_cast<int32_t>(index), 200);
      regPtr->put(CacheableInt64::create(key), value);
      expectEntries++;
    }

    dunit::globals().find_or_construct<int>("entriesToExpect")(expectEntries);
    LOG("Finished putting int64 entries.");
  }
END_TASK(UpdateManyPutsInt64)

DUNIT_TASK(CLIENT2, VerifyManyPutsInt64)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    auto entriesExpected = dunit::globals().find<int>("entriesToExpect").first;
    ASSERT(entriesExpected != nullptr, "entriesExpected is null");

    for (int64_t index = 0; index < *entriesExpected; ++index) {
      int64_t key = index * index * index;
      // auto valPtr =
      // std::dynamic_pointer_cast<CacheableString>(regPtr->get(key));
      auto valPtr = std::dynamic_pointer_cast<CacheableString>(
          regPtr->get(CacheableInt64::create(key)));
      ASSERT(valPtr != nullptr, "expected non-null value");
      ASSERT(valPtr->length() == 207, "unexpected size of value in verify");
    }
    LOG("On client Found all int64 entries with "
        "correct size via get.");
    for (int64_t index = 0; index < *entriesExpected; ++index) {
      // auto key =
      // CacheableKey::create(CacheableInt64::create(index
      // * index * index));
      auto key = CacheableInt64::create(index * index * index);
      auto entry = regPtr->getEntry(key);
      ASSERT(entry != nullptr, "expected non-null entry");
      auto valPtr =
          std::dynamic_pointer_cast<CacheableString>(entry->getValue());
      ASSERT(valPtr != nullptr, "expected non-null value");
      ASSERT(valPtr->length() == 207, "unexpected size of value in verify");
    }
    LOG("On client Found all int64 entries with "
        "correct size via getEntry.");
  }
END_TASK(VerifyManyPutsInt64)

DUNIT_TASK(CLIENT2, VerifyUpdatedManyPutsInt64GetAll)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    // region should already have n entries...
    auto entriesExpected = dunit::globals().find<int>("entriesToExpect").first;
    ASSERT(entriesExpected != nullptr, "entriesExpected is null");

    // invalidate the region to force getting new
    // values
    regPtr->localInvalidateRegion();

    std::vector<std::shared_ptr<CacheableKey>> vec;
    for (int64_t index = 0; index < *entriesExpected; ++index) {
      int64_t key = index * index * index;
      vec.push_back(CacheableInt64::create(key));
    }
    const auto valuesMap = regPtr->getAll(vec);
    LOG("On client getAll for int64 entries completed.");
    for (int32_t index = 0; index < *entriesExpected; ++index) {
      auto key = vec[index];
      ASSERT(regPtr->containsKey(key), "must contain key");
      auto entry = regPtr->getEntry(key);
      ASSERT(entry != nullptr, "expected non-null entry");
      auto valPtr = entry->getValue();
      // std::dynamic_pointer_cast<CacheableString>(entry->getValue());
      ASSERT(valPtr != nullptr, "expected non-null value");
      // ASSERT(valPtr->length() == 207, "unexpected size of value in verify");
      const auto &iter = valuesMap.find(key);
      ASSERT(iter != valuesMap.end(), "expected to find key in map");
      valPtr = std::dynamic_pointer_cast<CacheableString>(iter->second);
      ASSERT(valPtr != nullptr, "expected non-null value");
      // ASSERT(valPtr->length() == 207, "unexpected size of value in verify");
    }
    LOG("On client Found all int64 entries with "
        "correct size via getAll.");
  }
END_TASK(VerifyUpdatedManyPutsInt64GetAll)

DUNIT_TASK(SERVER1, StopServer)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      CacheHelper::closeLocator(1);
    }
    LOG("SERVER stopped");
  }
END_TASK(StopServer)
