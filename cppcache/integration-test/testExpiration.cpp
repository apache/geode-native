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

#include <geode/ExpirationAction.hpp>
#include <geode/Region.hpp>

#include "fw_helper.hpp"
#include "CacheRegionHelper.hpp"
#include "CacheImpl.hpp"

using apache::geode::client::Cache;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheRegionHelper;
using apache::geode::client::ExpirationAction;
using apache::geode::client::Properties;
using apache::geode::client::Region;
using apache::geode::client::RegionAttributes;
using apache::geode::client::RegionAttributesFactory;

ExpirationAction action = ExpirationAction::DESTROY;

// This test is for serially running the tests.

size_t getNumOfEntries(std::shared_ptr<Region> &R1) {
  std::vector<std::shared_ptr<CacheableKey>> v = R1->keys();
  LOG_FINE("Number of keys in region %s is %d", R1->getFullPath().c_str(),
           v.size());
  return v.size();
}

void startDSandCreateCache(std::shared_ptr<Cache> &cache) {
  auto pp = Properties::create();
  auto cacheFactory = CacheFactory(pp);
  cacheFactory.set("statistic-sampling-enabled", "true");
  cache = std::make_shared<Cache>(cacheFactory.create());
  ASSERT(cache != nullptr, "cache not equal to null expected");
}

void doNPuts(std::shared_ptr<Region> &rptr, int n) {
  std::shared_ptr<CacheableString> value;
  char buf[16];
  memset(buf, 'A', 15);
  buf[15] = '\0';
  memcpy(buf, "Value - ", 8);
  value = CacheableString::create(buf);
  ASSERT(value != nullptr, "Failed to create value.");

  for (int i = 0; i < n; i++) {
    sprintf(buf, "KeyA - %d", i + 1);
    auto key = CacheableKey::create(buf);
    LOG_INFO("Putting key %s value %s in region %s", buf,
             value->toString().c_str(), rptr->getFullPath().c_str());
    rptr->put(key, value);
  }
}
std::shared_ptr<CacheableKey> do1Put(std::shared_ptr<Region> &rptr) {
  std::shared_ptr<CacheableString> value;
  char buf[16];
  memset(buf, 'A', 15);
  buf[15] = '\0';
  memcpy(buf, "Value - ", 8);
  value = CacheableString::create(buf);
  ASSERT(value != nullptr, "Failed to create value.");

  sprintf(buf, "KeyA - %d", 0 + 1);
  auto key = CacheableKey::create(buf);
  LOG_INFO("Putting key %s value %s in region %s", buf,
           value->toString().c_str(), rptr->getFullPath().c_str());
  rptr->put(key, value);
  return key;
}

RegionAttributes setRegionAttributesTimeouts(
    const std::chrono::seconds &entryTimeToLive = std::chrono::seconds::zero(),
    const std::chrono::seconds &entryIdleTimeout = std::chrono::seconds::zero(),
    const std::chrono::seconds &regionTimeToLive = std::chrono::seconds::zero(),
    const std::chrono::seconds &regionIdleTimeout =
        std::chrono::seconds::zero()) {
  RegionAttributesFactory regionAttributesFactory;

  regionAttributesFactory.setEntryTimeToLive(action, entryTimeToLive);
  regionAttributesFactory.setEntryIdleTimeout(action, entryIdleTimeout);
  regionAttributesFactory.setRegionTimeToLive(action, regionTimeToLive);
  regionAttributesFactory.setRegionIdleTimeout(action, regionIdleTimeout);

  return regionAttributesFactory.create();
}

BEGIN_TEST(TEST_EXPIRATION)
  {
    std::shared_ptr<Cache> cache;

    startDSandCreateCache(cache);

    ASSERT(cache != nullptr, "Expected cache to be NON-nullptr");

    ASSERT(cache->getSystemProperties().statisticsEnabled(),
           "Statistics must be enabled for expiration.");

    auto cacheImpl = CacheRegionHelper::getCacheImpl(cache.get());

    size_t n;

    RegionAttributes attrs_1;
    // ettl = 0, eit = 0, rttl = 0, reit = 0
    attrs_1 = setRegionAttributesTimeouts();
    std::shared_ptr<Region> R1;
    cacheImpl->createRegion("R1", attrs_1, R1);
    ASSERT(R1 != nullptr, "Expected R1 to be NON-nullptr");

    doNPuts(R1, 100);

    std::this_thread::sleep_for(std::chrono::seconds(10));

    n = getNumOfEntries(R1);
    ASSERT(n == 100, "Expected 100 entries");

    ASSERT(R1->isDestroyed() == false, "Expected R1 to be alive");

    RegionAttributes attrs_2;

    attrs_2 = setRegionAttributesTimeouts(
        std::chrono::seconds(20), std::chrono::seconds(2),
        std::chrono::seconds(0), std::chrono::seconds(0));

    std::shared_ptr<Region> R2;
    cacheImpl->createRegion("R2", attrs_2, R2);
    ASSERT(R2 != nullptr, "Expected R2 to be NON-nullptr");
    LOG("Region R2 created");
    doNPuts(R2, 1);

    std::this_thread::sleep_for(std::chrono::seconds(5));

    n = getNumOfEntries(R2);
    ASSERT(n == 1, "Expected 1 entry");

    ASSERT(R2->isDestroyed() == false, "Expected R2 to be alive");

    RegionAttributes attrs_3;
    // rttl = 20, reit = 2
    attrs_3 = setRegionAttributesTimeouts(
        std::chrono::seconds(0), std::chrono::seconds(0),
        std::chrono::seconds(20), std::chrono::seconds(2));

    std::shared_ptr<Region> R3;
    cacheImpl->createRegion("R3", attrs_3, R3);
    ASSERT(R3 != nullptr, "Expected R3 to be NON-nullptr");

    std::this_thread::sleep_for(std::chrono::seconds(5));

    ASSERT(R3->isDestroyed() == false, "Expected R3 to be alive");

    RegionAttributes attrs_4;

    attrs_4 = setRegionAttributesTimeouts(
        std::chrono::seconds(5), std::chrono::seconds(0),
        std::chrono::seconds(0), std::chrono::seconds(0));

    std::shared_ptr<Region> R4;
    cacheImpl->createRegion("R4", attrs_4, R4);
    ASSERT(R4 != nullptr, "Expected R4 to be NON-nullptr");

    doNPuts(R4, 1);
    // This will be same as updating the object

    std::this_thread::sleep_for(std::chrono::seconds(10));

    n = getNumOfEntries(R4);
    ASSERT(n == 0, "Expected 0 entry");

    ASSERT(R4->isDestroyed() == false, "Expected R4 to be alive");

    RegionAttributes attrs_5;
    // ettl = 0, eit = 0, rttl = 0, reit = 0
    attrs_5 = setRegionAttributesTimeouts(
        std::chrono::seconds(0), std::chrono::seconds(5),
        std::chrono::seconds(0), std::chrono::seconds(0));

    std::shared_ptr<Region> R5;
    cacheImpl->createRegion("R5", attrs_5, R5);
    ASSERT(R5 != nullptr, "Expected R5 to be NON-nullptr");

    auto key_0 = do1Put(R5);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    R5->get(key_0);
    std::this_thread::sleep_for(std::chrono::seconds(3));

    n = getNumOfEntries(R5);

    printf("n ==  %zd\n", n);
    ASSERT(n == 1, "Expected 1 entry");

    // std::this_thread::sleep_for(std::chrono::seconds(3));
    std::this_thread::sleep_for(std::chrono::seconds(6));
    n = getNumOfEntries(R5);

    ASSERT(n == 0, "Expected 0 entry");
    ASSERT(R5->isDestroyed() == false, "Expected R5 to be alive");

    RegionAttributes attrs_6;
    // ettl = 0, eit = 0, rttl = 0, reit = 0
    attrs_6 = setRegionAttributesTimeouts(
        std::chrono::seconds(0), std::chrono::seconds(0),
        std::chrono::seconds(5), std::chrono::seconds(0));

    std::shared_ptr<Region> R6;
    cacheImpl->createRegion("R6", attrs_6, R6);
    ASSERT(R6 != nullptr, "Expected R6 to be NON-nullptr");

    doNPuts(R6, 1);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    doNPuts(R6, 1);

    std::this_thread::sleep_for(std::chrono::seconds(7));

    ASSERT(R6->isDestroyed() == true, "Expected R6 to be dead");

    RegionAttributes attrs_7;
    // ettl = 0, eit = 0, rttl = 0, reit = 0
    attrs_7 = setRegionAttributesTimeouts(
        std::chrono::seconds(0), std::chrono::seconds(0),
        std::chrono::seconds(0), std::chrono::seconds(5));

    std::shared_ptr<Region> R7;
    cacheImpl->createRegion("R7", attrs_7, R7);
    ASSERT(R7 != nullptr, "Expected R7 to be NON-nullptr");

    doNPuts(R7, 1);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    doNPuts(R7, 1);

    std::this_thread::sleep_for(std::chrono::seconds(10));

    ASSERT(R7->isDestroyed() == true, "Expected R7 to be dead");

    RegionAttributes attrs_8;
    // ettl = 0, eit = 0, rttl = 0, reit = 0
    attrs_8 = setRegionAttributesTimeouts(
        std::chrono::seconds(10), std::chrono::seconds(0),
        std::chrono::seconds(0), std::chrono::seconds(0));

    std::shared_ptr<Region> R8;
    cacheImpl->createRegion("R8", attrs_8, R8);
    ASSERT(R8 != nullptr, "Expected R8 to be NON-nullptr");

    auto key = do1Put(R8);

    std::this_thread::sleep_for(std::chrono::seconds(5));

    R8->get(key);

    std::this_thread::sleep_for(std::chrono::seconds(6));

    n = getNumOfEntries(R8);
    ASSERT(n == 0, "Expected 1 entries");

    RegionAttributes attrs_9;
    // ettl = 0, eit = 0, rttl = 0, reit = 0
    attrs_9 = setRegionAttributesTimeouts(
        std::chrono::seconds(0), std::chrono::seconds(0),
        std::chrono::seconds(0), std::chrono::seconds(8));

    std::shared_ptr<Region> R9;
    cacheImpl->createRegion("R9", attrs_9, R9);
    ASSERT(R9 != nullptr, "Expected R9 to be NON-nullptr");

    auto key_1 = do1Put(R9);

    std::this_thread::sleep_for(std::chrono::seconds(5));

    R9->get(key_1);

    std::this_thread::sleep_for(std::chrono::seconds(5));

    n = getNumOfEntries(R9);
    ASSERT(n == 1, "Expected 1 entries");

    ASSERT(R9->isDestroyed() == false, "Expected R9 to be alive");

    RegionAttributes attrs_10;
    // ettl = 0, eit = 0, rttl = 0, reit = 0
    attrs_10 = setRegionAttributesTimeouts(
        std::chrono::seconds(6), std::chrono::seconds(0),
        std::chrono::seconds(0), std::chrono::seconds(12));

    std::shared_ptr<Region> R10;
    cacheImpl->createRegion("R10", attrs_10, R10);
    ASSERT(R10 != nullptr, "Expected R10 to be NON-nullptr");

    doNPuts(R10, 1);

    std::this_thread::sleep_for(std::chrono::seconds(10));

    n = getNumOfEntries(R10);
    ASSERT(n == 0, "Expected 0 entries");

    std::this_thread::sleep_for(std::chrono::seconds(11));

    ASSERT(R10->isDestroyed() == true, "Expected R10 to be dead");

    RegionAttributes attrs_11;

    // ettl = 0, eit = 0, rttl = 0, reit = 0
    attrs_11 = setRegionAttributesTimeouts(
        std::chrono::seconds(0), std::chrono::seconds(4),
        std::chrono::seconds(0), std::chrono::seconds(7));

    std::shared_ptr<Region> R11;
    cacheImpl->createRegion("R11", attrs_11, R11);
    ASSERT(R11 != nullptr, "Expected R11 to be NON-nullptr");

    auto k11 = do1Put(R11);

    std::this_thread::sleep_for(std::chrono::seconds(3));

    n = getNumOfEntries(R11);
    ASSERT(n == 1, "Expected 1 entries");

    R11->get(k11);

    std::this_thread::sleep_for(std::chrono::seconds(5));

    ASSERT(R11->isDestroyed() == false,
           "Expected R11 to be alive as the get has changed the access time");

    std::this_thread::sleep_for(std::chrono::seconds(5));

    ASSERT(R11->isDestroyed() == true, "Expected R11 to be dead");

    RegionAttributes attrs_12;
    // ettl = 0, eit = 0, rttl = 0, reit = 0
    attrs_12 = setRegionAttributesTimeouts(
        std::chrono::seconds(5), std::chrono::seconds(0),
        std::chrono::seconds(0), std::chrono::seconds(0));

    std::shared_ptr<Region> R12;
    cacheImpl->createRegion("R12", attrs_12, R12);
    ASSERT(R12 != nullptr, "Expected R12 to be NON-nullptr");

    auto key_3 = do1Put(R12);

    std::this_thread::sleep_for(std::chrono::seconds(6));

    n = getNumOfEntries(R12);
    ASSERT(n == 0, "Expected 0 entries");

    ASSERT(R12->isDestroyed() == false, "Expected R12 to be alive");
    /////////

    RegionAttributes attrs_14;
    // ettl = 0, eit = 0, rttl = 0, reit = 0
    attrs_14 = setRegionAttributesTimeouts(
        std::chrono::seconds(0), std::chrono::seconds(0),
        std::chrono::seconds(10), std::chrono::seconds(0));

    std::shared_ptr<Region> R14;
    cacheImpl->createRegion("R14", attrs_14, R14);
    ASSERT(R14 != nullptr, "Expected R14 to be NON-nullptr");

    doNPuts(R14, 1);

    std::this_thread::sleep_for(std::chrono::seconds(12));

    ASSERT(R14->isDestroyed() == true, "Expected R14 to be dead");

    RegionAttributes attrs_15;
    // ettl = 0, eit = 0, rttl = 0, reit = 0
    attrs_15 = setRegionAttributesTimeouts(
        std::chrono::seconds(0), std::chrono::seconds(5),
        std::chrono::seconds(0), std::chrono::seconds(0));

    std::shared_ptr<Region> R15;
    cacheImpl->createRegion("R15", attrs_15, R15);
    ASSERT(R15 != nullptr, "Expected R15 to be NON-nullptr");

    auto key_4 = do1Put(R15);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    R15->destroy(key_4);

    std::this_thread::sleep_for(std::chrono::seconds(5));

    ASSERT(R15->isDestroyed() == false, "Expected R15 to be alive");

    //////////////
    RegionAttributes attrs_18;
    // ettl = 0, eit = 0, rttl = 0, reit = 0
    attrs_18 = setRegionAttributesTimeouts(
        std::chrono::seconds(6), std::chrono::seconds(3),
        std::chrono::seconds(0), std::chrono::seconds(0));

    std::shared_ptr<Region> R18;
    cacheImpl->createRegion("R18", attrs_18, R18);
    ASSERT(R18 != nullptr, "Expected R18 to be NON-nullptr");

    doNPuts(R18, 1);

    std::this_thread::sleep_for(std::chrono::seconds(4));

    n = getNumOfEntries(R18);
    ASSERT(n == 1, "entry idle should be useless as ttl is > 0");

    std::this_thread::sleep_for(std::chrono::seconds(4));
    n = getNumOfEntries(R18);
    ASSERT(n == 0, "ttl is over so it should be 0");

    RegionAttributes attrs_19;
    // ettl = 0, eit = 0, rttl = 0, reit = 0
    attrs_19 = setRegionAttributesTimeouts(
        std::chrono::seconds(0), std::chrono::seconds(0),
        std::chrono::seconds(6), std::chrono::seconds(3));

    std::shared_ptr<Region> R19;
    cacheImpl->createRegion("R19x", attrs_19, R19);
    ASSERT(R19 != nullptr, "Expected R19 to be NON-nullptr");

    std::this_thread::sleep_for(std::chrono::seconds(4));

    doNPuts(R19, 1);

    std::this_thread::sleep_for(std::chrono::seconds(4));

    ASSERT(R19->isDestroyed() == false,
           "Expected R19 to be alive as an entry was put");

    std::this_thread::sleep_for(std::chrono::seconds(4));

    ASSERT(R19->isDestroyed() == true, "Expected R19 to be dead");
    cache->close();
  }
END_TEST(TEST_EXPIRATION)
