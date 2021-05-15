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

#include <string>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/process.hpp>

#include <geode/RegionShortcut.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/PersistenceManager.hpp>

#include <CacheableToken.hpp>
#include <CacheRegionHelper.hpp>
#include <MapEntry.hpp>

#include "CacheImpl.hpp"

#include "fw_helper.hpp"

using apache::geode::client::Cache;
using apache::geode::client::Cacheable;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheableToken;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheRegionHelper;
using apache::geode::client::DiskPolicyType;
using apache::geode::client::Exception;
using apache::geode::client::HashMapOfCacheable;
using apache::geode::client::PersistenceManager;
using apache::geode::client::Properties;
using apache::geode::client::Region;
using apache::geode::client::RegionAttributes;
using apache::geode::client::RegionAttributesFactory;
using apache::geode::client::RegionShortcut;

uint32_t numOfEnt;
std::string sqlite_dir = "SqLiteRegionData";

static constexpr char const *MAX_PAGE_COUNT = "MaxPageCount";
static constexpr char const *PAGE_SIZE = "PageSize";
static constexpr char const *PERSISTENCE_DIR = "PersistenceDirectory";

// Return the number of keys and values in entries map.
void getNumOfEntries(std::shared_ptr<Region> &regionPtr, uint32_t num) {
  auto v = regionPtr->keys();
  auto vecValues = regionPtr->values();
  printf("Values vector size is %zd\n", vecValues.size());
  printf("Num is %d\n", num);
  ASSERT(vecValues.size() == num, "size of value vec and num not equal");
}

void setAttributes(RegionAttributes regionAttributes,
                   std::string pDir = sqlite_dir) {
  RegionAttributesFactory regionAttributesFactory;
  regionAttributesFactory.setCachingEnabled(true);
  regionAttributesFactory.setLruEntriesLimit(10);
  regionAttributesFactory.setInitialCapacity(1000);
  regionAttributesFactory.setDiskPolicy(DiskPolicyType::OVERFLOWS);
  auto sqliteProperties = Properties::create();
  sqliteProperties->insert("MaxPageCount", "1073741823");
  sqliteProperties->insert("PageSize", "65536");
  sqliteProperties->insert("PersistenceDirectory", pDir.c_str());
  regionAttributesFactory.setPersistenceManager(
      "SqLiteImpl", "createSqLiteInstance", sqliteProperties);

  regionAttributes = regionAttributesFactory.create();
}
void setAttributesWithMirror(RegionAttributes regionAttributes) {
  RegionAttributesFactory regionAttributesFactory;
  regionAttributesFactory.setCachingEnabled(true);
  regionAttributesFactory.setLruEntriesLimit(20);
  regionAttributesFactory.setInitialCapacity(1000);
  regionAttributesFactory.setDiskPolicy(DiskPolicyType::OVERFLOWS);
  auto sqliteProperties = Properties::create();
  sqliteProperties->insert("MaxPageCount", "1073741823");
  sqliteProperties->insert("PageSize", "65536");
  sqliteProperties->insert("PersistenceDirectory", sqlite_dir.c_str());
  regionAttributesFactory.setPersistenceManager(
      "SqLiteImpl", "createSqLiteInstance", sqliteProperties);
  regionAttributes = regionAttributesFactory.create();
}

// Testing for attibute validation.
void validateAttribute(std::shared_ptr<Region> &regionPtr) {
  RegionAttributes regAttr = regionPtr->getAttributes();
  int initialCapacity = regAttr.getInitialCapacity();
  ASSERT(initialCapacity == 1000, "Expected initial capacity to be 1000");
  const DiskPolicyType type = regAttr.getDiskPolicy();
  ASSERT(type == DiskPolicyType::OVERFLOWS,
         "Expected Action should overflow to disk");
}

void checkOverflowTokenValues(std::shared_ptr<Region> &regionPtr,
                              uint32_t num) {
  std::vector<std::shared_ptr<CacheableKey>> v = regionPtr->keys();
  std::shared_ptr<CacheableKey> keyPtr;
  std::shared_ptr<Cacheable> valuePtr;
  int count = 0;
  size_t nonoverflowCount = 0;
  for (uint32_t i = 0; i < num; i++) {
    keyPtr = v.at(i);
    auto rPtr = regionPtr->getEntry(keyPtr);
    valuePtr = rPtr->getValue();
    if (CacheableToken::isOverflowed(valuePtr) == true) {
      count++;
    } else {
      nonoverflowCount++;
    }

    valuePtr = nullptr;
  }
  ASSERT(count == 0, "No of overflowed entries should be zero");
  ASSERT(nonoverflowCount == v.size(),
         "Non overflowed entries should match key size");
}

void checkOverflowToken(std::shared_ptr<Region> &regionPtr, uint32_t lruLimit) {
  std::vector<std::shared_ptr<CacheableKey>> v = regionPtr->keys();
  std::shared_ptr<CacheableKey> keyPtr;
  std::shared_ptr<Cacheable> valuePtr;
  int normalCount = 0;
  int overflowCount = 0;
  int invalidCount = 0;
  int destoyedCount = 0;
  int tombstoneCount = 0;
  for (uint32_t i = 0; i < static_cast<uint32_t>(v.size()); i++) {
    keyPtr = v.at(i);
    auto rPtr = regionPtr->getEntry(keyPtr);
    valuePtr = rPtr->getValue();
    if (CacheableToken::isOverflowed(valuePtr) == true) {
      overflowCount++;
    } else if (CacheableToken::isTombstone(valuePtr)) {
      tombstoneCount++;
    } else if (CacheableToken::isInvalid(valuePtr)) {
      invalidCount++;
    } else if (CacheableToken::isDestroyed(valuePtr)) {
      destoyedCount++;
    } else if (valuePtr != nullptr) {
      normalCount++;
    }
    valuePtr = nullptr;
  }
  printf("Keys vector size is %zd\n", v.size());
  printf("Normal entries count is %d\n", normalCount);
  printf("Overflow entries count is %d\n", overflowCount);
  printf("Invalid entries count is %d\n", invalidCount);
  printf("Destoyed entries count is %d\n", destoyedCount);
  printf("Tombstone entries count is %d\n", tombstoneCount);
  printf("LRU entries limit is %d\n", lruLimit);
  ASSERT(normalCount <= static_cast<int>(lruLimit),
         "Normal entries count should not exceed LRU entries limit.");
}

// Testing for put operation
void doNput(std::shared_ptr<Region> &regionPtr, uint32_t num,
            uint32_t start = 0) {
  char keybuf[100];
  // Put 1 KB of data locally for each entry
  char *text = new char[1024];
  memset(text, 'A', 1023);
  text[1023] = '\0';
  auto valuePtr = CacheableString::create(text);

  for (uint32_t i = start; i < num; i++) {
    sprintf(keybuf, "key-%d", i);
    auto key = CacheableKey::create(keybuf);
    printf("Putting key = %s\n", keybuf);
    regionPtr->put(key, valuePtr);
  }
}

void doNputLargeData(std::shared_ptr<Region> &regionPtr, int num) {
  // Put 1 MB of data locally for each entry
  char *text = new char[1024 * 1024 /* 1 MB */];
  memset(text, 'A', 1024 * 1024 - 1);
  text[1024 * 1024 - 1] = '\0';
  auto valuePtr = CacheableString::create(text);
  for (int item = 0; item < num; item++) {
    regionPtr->put(CacheableKey::create(item), valuePtr);
  }

  LOG_INFO("Put data locally");
}

// Testing for get operation
uint32_t doNgetLargeData(std::shared_ptr<Region> &regionPtr, int num) {
  uint32_t countFound = 0;
  uint32_t countNotFound = 0;

  for (int i = 0; i < num; i++) {
    printf("Getting key = %d\n", i);
    auto valuePtr =
        std::dynamic_pointer_cast<CacheableString>(regionPtr->get(i));
    if (valuePtr == nullptr) {
      countNotFound++;
    } else {
      countFound++;
    }
  }
  LOG_INFO("found:%d and Not found: %d", countFound, countNotFound);
  return countFound;
}

// Testing for get operation
uint32_t doNget(std::shared_ptr<Region> &regionPtr, uint32_t num,
                uint32_t start = 0) {
  uint32_t countFound = 0;
  uint32_t countNotFound = 0;

  for (uint32_t i = start; i < num; i++) {
    char keybuf[100];
    sprintf(keybuf, "key-%d", i);
    auto valuePtr =
        std::dynamic_pointer_cast<CacheableString>(regionPtr->get(keybuf));
    printf("Getting key = %s\n", keybuf);
    if (valuePtr == nullptr) {
      countNotFound++;
    } else {
      countFound++;
    }
  }
  printf("completed doNget");
  printf("count found %d", countFound);
  printf("num found %d", num);
  ASSERT(countFound == (num - start),
         "Number of entries found and put should match");
  LOG_INFO("found:%d and Not found: %d", countFound, countNotFound);
  return countFound;
}
/**
 *  Test the entry operation ( local invalidate, localDestroy ).
 */
void testEntryDestroy(std::shared_ptr<Region> &regionPtr, uint32_t num) {
  std::vector<std::shared_ptr<CacheableKey>> v = regionPtr->keys();
  std::vector<std::shared_ptr<Cacheable>> vecValues;
  std::shared_ptr<Cacheable> valuePtr;
  for (uint32_t i = 45; i < 50; i++) {
    try {
      std::cout << "try to destroy key" << i << std::endl;
      regionPtr->destroy(v.at(i));
    } catch (Exception &ex) {
      std::cout << ex.what() << std::endl;
      ASSERT(false, "entry missing");
    }
  }
  v = regionPtr->keys();
  ASSERT(v.size() == num - 5, "size of key vec not equal");
}

void testEntryInvalidate(std::shared_ptr<Region> &regionPtr, uint32_t num) {
  std::vector<std::shared_ptr<CacheableKey>> v = regionPtr->keys();
  std::vector<std::shared_ptr<Cacheable>> vecValues;
  std::shared_ptr<Cacheable> valuePtr;
  for (uint32_t i = 40; i < 45; i++) {
    try {
      std::cout << "try to invalidate key" << i << std::endl;
      regionPtr->invalidate(v.at(i));
    } catch (Exception &ex) {
      std::cout << ex.what() << std::endl;
      ASSERT(false, "entry missing");
    }
  }
  v = regionPtr->keys();
  ASSERT(v.size() == num, "size of key vec not equal");
}

class PutThread : public ACE_Task_Base {
 private:
  std::shared_ptr<Region> m_regPtr;
  int m_min;
  int m_max;

 public:
  PutThread(std::shared_ptr<Region> &regPtr, int min, int max)
      : m_regPtr(regPtr), m_min(min), m_max(max) {}

  int svc(void) override {
    /** put some values into the cache. */
    doNput(m_regPtr, m_max, m_min);
    /** do some gets... printing what we find in the cache. */
    doNget(m_regPtr, m_max, m_min);
    LOG("Completed doNget");
    return 0;
  }

  void start() { activate(); }

  void stop() { wait(); }
};

void verifyGetAll(std::shared_ptr<Region> region, int startIndex) {
  std::vector<std::shared_ptr<CacheableKey>> keysVector;
  for (int i = 0; i <= 100; i++) keysVector.push_back(CacheableKey::create(i));

  // keysVector.push_back(CacheableKey::create(101)); //key not there
  const auto valuesMap = region->getAll(keysVector);
  if (valuesMap.size() == keysVector.size()) {
    int i = startIndex;
    for (const auto &iter : valuesMap) {
      auto key = std::dynamic_pointer_cast<CacheableKey>(iter.first);
      auto mVal = iter.second;
      if (mVal != nullptr) {
        int val = atoi(mVal->toString().c_str());
        ASSERT(val == i, "value not matched");
      }
    }
  }
}

void createRegion(std::shared_ptr<Region> &regionPtr, const char *regionName,
                  std::shared_ptr<Properties> &cacheProps,
                  std::shared_ptr<Properties> &sqLiteProps) {
  auto cacheFactoryPtr = CacheFactory(cacheProps);
  auto cachePtr = std::make_shared<Cache>(CacheFactory().create());
  ASSERT(cachePtr != nullptr, "Expected cache to be NON-nullptr");
  auto regionFactory = cachePtr->createRegionFactory(RegionShortcut::LOCAL);
  regionFactory.setCachingEnabled(true);
  regionFactory.setLruEntriesLimit(10);
  regionFactory.setInitialCapacity(1000);
  regionFactory.setDiskPolicy(DiskPolicyType::OVERFLOWS);
  regionFactory.setPersistenceManager("SqLiteImpl", "createSqLiteInstance",
                                      sqLiteProps);
  regionPtr = regionFactory.create(regionName);
  ASSERT(regionPtr != nullptr, "Expected regionPtr to be NON-nullptr");
}

void setSqLiteProperties(std::shared_ptr<Properties> &sqliteProperties,
                         int maxPageCount = 1073741823, int pageSize = 65536,
                         std::string pDir = sqlite_dir) {
  sqliteProperties = Properties::create();
  sqliteProperties->insert(MAX_PAGE_COUNT, maxPageCount);
  sqliteProperties->insert(PAGE_SIZE, pageSize);
  sqliteProperties->insert(PERSISTENCE_DIR, pDir.c_str());
  ASSERT(sqliteProperties != nullptr,
         "Expected sqlite properties to be NON-nullptr");
}
// creation of subregion.

void createSubRegion(std::shared_ptr<Region> &regionPtr,
                     std::shared_ptr<Region> &subRegion,
                     const std::string &regionName,
                     std::string pDir = sqlite_dir) {
  RegionAttributes regionAttributesPtr;
  setAttributes(regionAttributesPtr, pDir);
  subRegion = regionPtr->createSubregion(regionName, regionAttributesPtr);
  ASSERT(subRegion != nullptr, "Expected region to be NON-nullptr");

  std::string fileName = pDir + '/' + regionName + '/' + regionName + ".db";
  ASSERT(boost::filesystem::exists(fileName), "persistence file not present");
  doNput(subRegion, 50);
  doNget(subRegion, 50);
}

BEGIN_TEST(OverFlowTest)
  {
    /** Creating a cache to manage regions. */
    std::shared_ptr<Properties> sqliteProperties;
    auto cacheProperties = Properties::create();
    setSqLiteProperties(sqliteProperties);
    std::shared_ptr<Region> regionPtr;
    createRegion(regionPtr, "OverFlowRegion", cacheProperties,
                 sqliteProperties);
    ASSERT(regionPtr != nullptr, "Expected regionPtr to be NON-nullptr");
    validateAttribute(regionPtr);
    /** put some values into the cache. */
    doNput(regionPtr, 50);
    checkOverflowToken(regionPtr, 10);

    /** do some gets... printing what we find in the cache. */
    doNget(regionPtr, 50);
    checkOverflowToken(regionPtr, 10);
    LOG("Completed doNget");

    testEntryDestroy(regionPtr, 50);
    checkOverflowToken(regionPtr, 10);
    LOG("Completed testEntryDestroy");

    testEntryInvalidate(regionPtr, 45);
    checkOverflowToken(regionPtr, 10);
    LOG("Completed testEntryInvalidate");

    getNumOfEntries(regionPtr, 40);
    /** check whether value get evicted and token gets set as overflow */
    checkOverflowTokenValues(regionPtr, 45);
    /** test to verify same region repeatedly to ensure that the persistece
    files are created and destroyed correctly */

    std::shared_ptr<Region> subRegion;
    for (int i = 0; i < 10; i++) {
      createSubRegion(regionPtr, subRegion, "SubRegion");
      ASSERT(subRegion != nullptr, "Expected region to be NON-nullptr");
      checkOverflowToken(subRegion,
                         10);  // check the overflow count for each reion
      subRegion->destroyRegion();
      ASSERT(subRegion->isDestroyed(), "Expected region is not destroyed ");
      subRegion = nullptr;

      std::string sqliteDirSubRgn =
          sqlite_dir + "/" + boost::asio::ip::host_name() + '_' +
          std::to_string(boost::this_process::get_id()) + "/_" +
          regionPtr->getName() + "_SubRegion/file_0.db";
      ASSERT(!boost::filesystem::exists(sqliteDirSubRgn),
             "persistence file still present");
    }
    // cache close
    regionPtr->getRegionService().close();
  }
END_TEST(OverFlowTest)

BEGIN_TEST(OverFlowTest_absPath)
  {
    std::shared_ptr<RegionAttributes> attrsPtr;
    auto wdPath = boost::filesystem::current_path().string();
    ASSERT(!wdPath.empty(),
           "Expected current Working Directory to be not empty");
    std::string absPersistenceDir = wdPath + "/absSqLite";

    /** Creating a cache to manage regions. */
    std::shared_ptr<Properties> sqliteProperties;
    auto cacheProperties = Properties::create();
    setSqLiteProperties(sqliteProperties, 1073741823, 65536, absPersistenceDir);
    std::shared_ptr<Region> regionPtr;
    createRegion(regionPtr, "OverFlowRegion", cacheProperties,
                 sqliteProperties);
    ASSERT(regionPtr != nullptr, "Expected regionPtr to be NON-nullptr");

    validateAttribute(regionPtr);
    /** put some values into the cache. */
    doNput(regionPtr, 50);
    checkOverflowToken(regionPtr, 10);
    LOG("Completed doNput");

    /** do some gets... printing what we find in the cache. */
    doNget(regionPtr, 50);
    checkOverflowToken(regionPtr, 10);
    LOG("Completed doNget");

    testEntryDestroy(regionPtr, 50);
    checkOverflowToken(regionPtr, 10);
    LOG("Completed doNput");

    testEntryInvalidate(regionPtr, 45);
    checkOverflowToken(regionPtr, 10);
    LOG("Completed testEntryInvalidate");

    getNumOfEntries(regionPtr, 40);
    /** check whether value get evicted and token gets set as overflow */
    checkOverflowTokenValues(regionPtr, 45);

    std::shared_ptr<Region> subRegion;
    for (int i = 0; i < 10; i++) {
      createSubRegion(regionPtr, subRegion, "SubRegion", absPersistenceDir);
      ASSERT(subRegion != nullptr, "Expected region to be NON-nullptr");
      subRegion->destroyRegion();
      ASSERT(subRegion->isDestroyed(), "Expected region is not destroyed ");
      subRegion = nullptr;

      std::string fileName = absPersistenceDir + "/SubRegion/SubRegion.db";
      ASSERT(!boost::filesystem::exists(fileName),
             "persistence file still present");
    }
    // cache close
    regionPtr->getRegionService().close();
  }
END_TEST(OverFlowTest_absPath)

BEGIN_TEST(OverFlowTest_SqLiteFull)
  {
    auto cacheFactoryPtr = CacheFactory();
    auto cachePtr = std::make_shared<Cache>(CacheFactory().create());
    ASSERT(cachePtr != nullptr, "Expected cache to be NON-nullptr");
    auto regionFactory = cachePtr->createRegionFactory(RegionShortcut::LOCAL);
    regionFactory.setCachingEnabled(true);
    regionFactory.setLruEntriesLimit(1);
    regionFactory.setInitialCapacity(1000);
    regionFactory.setDiskPolicy(DiskPolicyType::OVERFLOWS);
    auto sqliteProperties = Properties::create();
    sqliteProperties->insert(
        "MaxPageCount", "10");  // 10 * 1024 is arround 10kB is the db file size
    sqliteProperties->insert("PageSize", "1024");
    sqliteProperties->insert("PersistenceDirectory", sqlite_dir.c_str());
    regionFactory.setPersistenceManager("SqLiteImpl", "createSqLiteInstance",
                                        sqliteProperties);
    auto regionPtr = regionFactory.create("OverFlowRegion");
    ASSERT(regionPtr != nullptr, "Expected regionPtr to be NON-nullptr");

    try {
      doNput(regionPtr, 100);
      FAIL("Didn't get the expected exception");
    } catch (apache::geode::client::Exception
                 &ex) {  // expected sqlite full exception
                         // catching generic message as we dont
                         // have any specific sqlitefull exception
      char buffer[1024];
      sprintf(buffer, "Got expected exception %s: msg = %s",
              ex.getName().c_str(), ex.what());
      LOG(buffer);
    }

    // cache close
    cachePtr->close();
  }
END_TEST(OverFlowTest_SqLiteFull)

BEGIN_TEST(OverFlowTest_HeapLRU)
  {
    /** Creating a cache to manage regions. */
    auto pp = Properties::create();
    pp->insert("heap-lru-limit", 1);
    pp->insert("heap-lru-delta", 10);
    auto cacheFactoryPtr = CacheFactory(pp);
    auto cachePtr = std::make_shared<Cache>(CacheFactory().create());
    ASSERT(cachePtr != nullptr, "Expected cache to be NON-nullptr");
    auto regionFactory = cachePtr->createRegionFactory(RegionShortcut::LOCAL);
    regionFactory.setCachingEnabled(true);
    regionFactory.setLruEntriesLimit(1024 * 10);
    regionFactory.setInitialCapacity(1000);
    regionFactory.setDiskPolicy(DiskPolicyType::OVERFLOWS);
    auto sqliteProperties = Properties::create();
    sqliteProperties->insert(
        "MaxPageCount",
        "2147483646");  // 10 * 1024 is arround 10kB is the db file size
    sqliteProperties->insert("PageSize", "65536");
    sqliteProperties->insert("PersistenceDirectory", sqlite_dir.c_str());
    regionFactory.setPersistenceManager("SqLiteImpl", "createSqLiteInstance",
                                        sqliteProperties);
    auto regionPtr = regionFactory.create("OverFlowRegion");
    ASSERT(regionPtr != nullptr, "Expected regionPtr to be NON-nullptr");

    validateAttribute(regionPtr);
    /** put some values into the cache. */
    doNput(regionPtr, 1024 * 10);
    LOG("Completed doNput");

    /** do some gets... printing what we find in the cache. */
    doNget(regionPtr, 1024 * 10);
    LOG("Completed doNget");

    testEntryDestroy(regionPtr, 1024 * 10);
    LOG("Completed testEntryDestroy");

    /** test to verify same region repeatedly to ensure that the persistece
    files are created and destroyed correctly */

    std::shared_ptr<Region> subRegion;
    for (int i = 0; i < 10; i++) {
      RegionAttributes regionAttributes;
      setAttributes(regionAttributes);
      subRegion = regionPtr->createSubregion("SubRegion", regionAttributes);
      ASSERT(subRegion != nullptr, "Expected region to be NON-nullptr");

      std::string fileName = sqlite_dir + "/SubRegion/SubRegion.db";
      ASSERT(boost::filesystem::exists(fileName),
             "persistence file not present");
      doNput(subRegion, 50);
      doNget(subRegion, 50);
      subRegion->destroyRegion();
      ASSERT(subRegion->isDestroyed(), "Expected region is not destroyed ");
      subRegion = nullptr;
      ASSERT(!boost::filesystem::exists(fileName),
             "persistence file still present");
    }
    // cache close
    cachePtr->close();
  }
END_TEST(OverFlowTest_HeapLRU)

BEGIN_TEST(OverFlowTest_MultiThreaded)
  {
    /** Creating a cache to manage regions. */
    auto cachePtr = std::make_shared<Cache>(CacheFactory().create());
    ASSERT(cachePtr != nullptr, "Expected cache to be NON-nullptr");

    RegionAttributes regionAttributes;
    setAttributes(regionAttributes);

    /** Create a region with caching and LRU. */
    std::shared_ptr<Region> regionPtr;
    auto cacheImpl = CacheRegionHelper::getCacheImpl(cachePtr.get());
    cacheImpl->createRegion("OverFlowRegion", regionAttributes, regionPtr);
    ASSERT(regionPtr != nullptr, "Expected regionPtr to be NON-nullptr");
    validateAttribute(regionPtr);

    /** test to verify same region repeatedly to ensure that the persistece
    files are created and destroyed correctly */

    /** put some values into the cache. */
    PutThread *threads[4];

    for (int thdIdx = 0; thdIdx < 4; thdIdx++) {
      threads[thdIdx] =
          new PutThread(regionPtr, thdIdx * 100 + 1, 100 * (thdIdx + 1));
      threads[thdIdx]->start();
    }
    SLEEP(2000);  // wait for threads to become active

    for (int thdIdx = 0; thdIdx < 4; thdIdx++) {
      threads[thdIdx]->stop();
    }

    SLEEP(2000);
    checkOverflowToken(regionPtr, 10);
    // cache close
    cachePtr->close();
  }
END_TEST(OverFlowTest_MultiThreaded)

BEGIN_TEST(OverFlowTest_PutGetAll)
  {
    /** Creating a cache to manage regions. */
    auto cachePtr = std::make_shared<Cache>(CacheFactory().create());
    ASSERT(cachePtr != nullptr, "Expected cache to be NON-nullptr");

    RegionAttributes regionAttributes;
    setAttributes(regionAttributes);

    /** Create a region with caching and LRU. */
    std::shared_ptr<Region> regionPtr;
    auto cacheImpl = CacheRegionHelper::getCacheImpl(cachePtr.get());
    cacheImpl->createRegion("OverFlowRegion", regionAttributes, regionPtr);
    ASSERT(regionPtr != nullptr, "Expected regionPtr to be NON-nullptr");
    validateAttribute(regionPtr);

    // putAll some entries
    HashMapOfCacheable map0;
    map0.clear();
    for (int i = 1; i <= 50; i++) {
      map0.emplace(CacheableKey::create(i), Cacheable::create(i));
    }
    regionPtr->putAll(map0);
    checkOverflowToken(regionPtr, 10);
    LOG("Completed putAll");

    verifyGetAll(regionPtr, 1);
    checkOverflowToken(regionPtr, 10);
    LOG("Completed getAll");

    // cache close
    cachePtr->close();
  }
END_TEST(OverFlowTest_PutGetAll)
