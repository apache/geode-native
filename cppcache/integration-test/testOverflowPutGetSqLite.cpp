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

#define ROOT_NAME "testOverflowPutGetSqLite"

#include <string>
#include <iostream>

#include <ace/OS.h>

#include <geode/GeodeCppCache.hpp>

#include "fw_helper.hpp"
#include <CacheableToken.hpp>
#include <MapEntry.hpp>
#include <CacheRegionHelper.hpp>

using namespace apache::geode::client;

uint32_t numOfEnt;
std::string sqlite_dir = "SqLiteRegionData";

// Return the number of keys and values in entries map.
void getNumOfEntries(RegionPtr& regionPtr, uint32_t num) {
  auto v = regionPtr->keys();
  auto vecValues = regionPtr->values();
  printf("Values vector size is %zd\n", vecValues.size());
  printf("Num is %d\n", num);
  ASSERT(vecValues.size() == num, (char*)"size of value vec and num not equal");
}

void setAttributes(RegionAttributesPtr& attrsPtr,
                   std::string pDir = sqlite_dir) {
  AttributesFactory attrsFact;
  attrsFact.setCachingEnabled(true);
  attrsFact.setLruEntriesLimit(10);
  attrsFact.setInitialCapacity(1000);
  attrsFact.setDiskPolicy(DiskPolicyType::OVERFLOWS);
  PropertiesPtr sqliteProperties = Properties::create();
  sqliteProperties->insert("MaxPageCount", "1073741823");
  sqliteProperties->insert("PageSize", "65536");
  sqliteProperties->insert("PersistenceDirectory", pDir.c_str());
  attrsFact.setPersistenceManager("SqLiteImpl", "createSqLiteInstance",
                                  sqliteProperties);

  attrsPtr = attrsFact.createRegionAttributes();
}
void setAttributesWithMirror(RegionAttributesPtr& attrsPtr) {
  AttributesFactory attrsFact;
  attrsFact.setCachingEnabled(true);
  attrsFact.setLruEntriesLimit(20);
  attrsFact.setInitialCapacity(1000);
  attrsFact.setDiskPolicy(DiskPolicyType::OVERFLOWS);
  PropertiesPtr sqliteProperties = Properties::create();
  sqliteProperties->insert("MaxPageCount", "1073741823");
  sqliteProperties->insert("PageSize", "65536");
  sqliteProperties->insert("PersistenceDirectory", sqlite_dir.c_str());
  attrsFact.setPersistenceManager("SqLiteImpl", "createSqLiteInstance",
                                  sqliteProperties);
  attrsPtr = attrsFact.createRegionAttributes();
}

// Testing for attibute validation.
void validateAttribute(RegionPtr& regionPtr) {
  RegionAttributesPtr regAttr = regionPtr->getAttributes();
  int initialCapacity = regAttr->getInitialCapacity();
  ASSERT(initialCapacity == 1000, "Expected initial capacity to be 1000");
  const DiskPolicyType::PolicyType type = regAttr->getDiskPolicy();
  ASSERT(type == DiskPolicyType::OVERFLOWS,
         "Expected Action should overflow to disk");
}

void checkOverflowTokenValues(RegionPtr& regionPtr, uint32_t num) {
  VectorOfCacheableKey v = regionPtr->keys();
  CacheableKeyPtr keyPtr;
  CacheablePtr valuePtr;
  int count = 0;
  int nonoverflowCount = 0;
  for (uint32_t i = 0; i < num; i++) {
    keyPtr = v.at(i);
    RegionEntryPtr rPtr = regionPtr->getEntry(keyPtr);
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

void checkOverflowToken(RegionPtr& regionPtr, uint32_t lruLimit) {
  VectorOfCacheableKey v = regionPtr->keys();
  CacheableKeyPtr keyPtr;
  CacheablePtr valuePtr;
  int normalCount = 0;
  int overflowCount = 0;
  int invalidCount = 0;
  int destoyedCount = 0;
  int tombstoneCount = 0;
  for (uint32_t i = 0; i < static_cast<uint32_t>(v.size()); i++) {
    keyPtr = v.at(i);
    RegionEntryPtr rPtr = regionPtr->getEntry(keyPtr);
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
  ASSERT(normalCount <= (int)lruLimit,
         "Normal entries count should not exceed LRU entries limit.");
}

// Testing for put operation
void doNput(RegionPtr& regionPtr, uint32_t num, uint32_t start = 0) {
  char keybuf[100];
  // Put 1 KB of data locally for each entry
  char* text = new char[1024];
  memset(text, 'A', 1023);
  text[1023] = '\0';
  CacheableStringPtr valuePtr = CacheableString::create(text);

  for (uint32_t i = start; i < num; i++) {
    sprintf(keybuf, "key-%d", i);
    CacheableKeyPtr key = CacheableKey::create(keybuf);
    printf("Putting key = %s\n", keybuf);
    regionPtr->put(key, valuePtr);
  }
}

void doNputLargeData(RegionPtr& regionPtr, int num) {
  // Put 1 MB of data locally for each entry
  char* text = new char[1024 * 1024 /* 1 MB */];
  memset(text, 'A', 1024 * 1024 - 1);
  text[1024 * 1024 - 1] = '\0';
  CacheableStringPtr valuePtr = CacheableString::create(text);
  for (int item = 0; item < num; item++) {
    regionPtr->put(CacheableKey::create(item), valuePtr);
  }

  LOGINFO("Put data locally");
}

// Testing for get operation
uint32_t doNgetLargeData(RegionPtr& regionPtr, int num) {
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
  LOGINFO("found:%d and Not found: %d", countFound, countNotFound);
  return countFound;
}

// Testing for get operation
uint32_t doNget(RegionPtr& regionPtr, uint32_t num, uint32_t start = 0) {
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
  LOGINFO("found:%d and Not found: %d", countFound, countNotFound);
  return countFound;
}
/**
 *  Test the entry operation ( local invalidate, localDestroy ).
 */
void testEntryDestroy(RegionPtr& regionPtr, uint32_t num) {
  VectorOfCacheableKey v = regionPtr->keys();
  VectorOfCacheable vecValues;
  CacheablePtr valuePtr;
  for (uint32_t i = 45; i < 50; i++) {
    try {
      std::cout << "try to destroy key" << i << std::endl;
      regionPtr->destroy(v.at(i));
    } catch (Exception& ex) {
      std::cout << ex.getMessage() << std::endl;
      ASSERT(false, (char*)"entry missing");
    }
  }
  v = regionPtr->keys();
  ASSERT(v.size() == num - 5, (char*)"size of key vec not equal");
}

void testEntryInvalidate(RegionPtr& regionPtr, uint32_t num) {
  VectorOfCacheableKey v = regionPtr->keys();
  VectorOfCacheable vecValues;
  CacheablePtr valuePtr;
  for (uint32_t i = 40; i < 45; i++) {
    try {
      std::cout << "try to invalidate key" << i << std::endl;
      regionPtr->invalidate(v.at(i));
    } catch (Exception& ex) {
      std::cout << ex.getMessage() << std::endl;
      ASSERT(false, (char*)"entry missing");
    }
  }
  v = regionPtr->keys();
  ASSERT(v.size() == num, (char*)"size of key vec not equal");
}

class PutThread : public ACE_Task_Base {
 private:
  RegionPtr m_regPtr;
  int m_min;
  int m_max;

 public:
  PutThread(RegionPtr& regPtr, int min, int max)
      : m_regPtr(regPtr), m_min(min), m_max(max) {}

  int svc(void) {
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

void verifyGetAll(RegionPtr region, int startIndex) {
  VectorOfCacheableKey keysVector;
  for (int i = 0; i <= 100; i++) keysVector.push_back(CacheableKey::create(i));

  // keysVector.push_back(CacheableKey::create(101)); //key not there
  const auto valuesMap = region->getAll(keysVector);
  if (valuesMap.size() == keysVector.size()) {
    int i = startIndex;
    for (const auto& iter : valuesMap) {
      auto key = std::dynamic_pointer_cast<CacheableKey>(iter.first);
      CacheablePtr mVal = iter.second;
      if (mVal != nullptr) {
        int val = atoi(mVal->toString()->asChar());
        ASSERT(val == i, "value not matched");
      }
    }
  }
}

void createRegion(RegionPtr& regionPtr, const char* regionName,
                  PropertiesPtr& cacheProps, PropertiesPtr& sqLiteProps) {
  auto cacheFactoryPtr = CacheFactory::createCacheFactory(cacheProps);
  auto cachePtr = CacheFactory::createCacheFactory()->create();
  ASSERT(cachePtr != nullptr, "Expected cache to be NON-nullptr");
  auto regionFactory = cachePtr->createRegionFactory(LOCAL);
  regionFactory.setCachingEnabled(true);
  regionFactory.setLruEntriesLimit(10);
  regionFactory.setInitialCapacity(1000);
  regionFactory.setDiskPolicy(DiskPolicyType::OVERFLOWS);
  regionFactory.setPersistenceManager("SqLiteImpl", "createSqLiteInstance",
                                      sqLiteProps);
  regionPtr = regionFactory.create(regionName);
  ASSERT(regionPtr != nullptr, "Expected regionPtr to be NON-nullptr");
}

void setSqLiteProperties(PropertiesPtr& sqliteProperties,
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

void createSubRegion(RegionPtr& regionPtr, RegionPtr& subRegion,
                     const char* regionName, std::string pDir = sqlite_dir) {
  RegionAttributesPtr regionAttributesPtr;
  setAttributes(regionAttributesPtr, pDir);
  subRegion = regionPtr->createSubregion(regionName, regionAttributesPtr);
  ASSERT(subRegion != nullptr, "Expected region to be NON-nullptr");
  char fileName[512];
  sprintf(fileName, "%s/%s/%s.db", pDir.c_str(), regionName, regionName);
  ACE_stat fileStat;
  ASSERT(ACE_OS::stat(fileName, &fileStat) == 0,
         "persistence file not present");
  doNput(subRegion, 50);
  doNget(subRegion, 50);
}

BEGIN_TEST(OverFlowTest)
  {
    /** Creating a cache to manage regions. */
    PropertiesPtr sqliteProperties;
    PropertiesPtr cacheProperties = Properties::create();
    setSqLiteProperties(sqliteProperties);
    RegionPtr regionPtr;
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

    RegionPtr subRegion;
    for (int i = 0; i < 10; i++) {
      createSubRegion(regionPtr, subRegion, "SubRegion");
      ASSERT(subRegion != nullptr, "Expected region to be NON-nullptr");
      checkOverflowToken(subRegion,
                         10);  // check the overflow count for each reion
      subRegion->destroyRegion();
      ASSERT(subRegion->isDestroyed(), "Expected region is not destroyed ");
      subRegion = nullptr;
      ACE_TCHAR hname[MAXHOSTNAMELEN];
      ACE_OS::hostname(hname, sizeof(hname) - 1);
      char sqliteDirSubRgn[512];
      sprintf(sqliteDirSubRgn, "%s/%s_%u/_%s_SubRegion/file_0.db",
              sqlite_dir.c_str(), hname, ACE_OS::getpid(),
              regionPtr->getName());

      ACE_stat fileStat;
      ASSERT(ACE_OS::stat(sqliteDirSubRgn, &fileStat) == -1,
             "persistence file still present");
    }
    // cache close
    regionPtr->getRegionService()->close();
  }
END_TEST(OverFlowTest)

BEGIN_TEST(OverFlowTest_absPath)
  {
    RegionAttributesPtr attrsPtr;
    char currWDPath[512];
    char* wdPath = ACE_OS::getcwd(currWDPath, 512);
    ASSERT(wdPath != nullptr,
           "Expected current Working Directory to be NON-nullptr");
    std::string absPersistenceDir = std::string(wdPath) + "/absSqLite";

    /** Creating a cache to manage regions. */
    PropertiesPtr sqliteProperties;
    PropertiesPtr cacheProperties = Properties::create();
    setSqLiteProperties(sqliteProperties, 1073741823, 65536, absPersistenceDir);
    RegionPtr regionPtr;
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

    RegionPtr subRegion;
    for (int i = 0; i < 10; i++) {
      createSubRegion(regionPtr, subRegion, "SubRegion", absPersistenceDir);
      ASSERT(subRegion != nullptr, "Expected region to be NON-nullptr");
      subRegion->destroyRegion();
      ASSERT(subRegion->isDestroyed(), "Expected region is not destroyed ");
      subRegion = nullptr;
      char fileName[512];
      sprintf(fileName, "%s/%s/%s.db", absPersistenceDir.c_str(), "SubRegion",
              "SubRegion");
      ACE_stat fileStat;
      ASSERT(ACE_OS::stat(fileName, &fileStat) == -1,
             "persistence file still present");
    }
    // cache close
    regionPtr->getRegionService()->close();
  }
END_TEST(OverFlowTest_absPath)

BEGIN_TEST(OverFlowTest_SqLiteFull)
  {
    auto cacheFactoryPtr = CacheFactory::createCacheFactory();
    auto cachePtr = CacheFactory::createCacheFactory()->create();
    ASSERT(cachePtr != nullptr, "Expected cache to be NON-nullptr");
    auto regionFactory = cachePtr->createRegionFactory(LOCAL);
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
                 ex) {  // expected sqlite full exception
                        // catching generic message as we dont
                        // have any specific sqlitefull exception
      char buffer[1024];
      sprintf(buffer, "Got expected exception %s: msg = %s", ex.getName(),
              ex.getMessage());
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
    auto cacheFactoryPtr = CacheFactory::createCacheFactory(pp);
    auto cachePtr = CacheFactory::createCacheFactory()->create();
    ASSERT(cachePtr != nullptr, "Expected cache to be NON-nullptr");
    auto regionFactory = cachePtr->createRegionFactory(LOCAL);
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

    RegionPtr subRegion;
    for (int i = 0; i < 10; i++) {
      RegionAttributesPtr regionAttributesPtr;
      setAttributes(regionAttributesPtr);
      subRegion = regionPtr->createSubregion("SubRegion", regionAttributesPtr);
      ASSERT(subRegion != nullptr, "Expected region to be NON-nullptr");
      char fileName[512];
      sprintf(fileName, "%s/%s/%s.db", sqlite_dir.c_str(), "SubRegion",
              "SubRegion");
      ACE_stat fileStat;
      ASSERT(ACE_OS::stat(fileName, &fileStat) == 0,
             "persistence file not present");
      doNput(subRegion, 50);
      doNget(subRegion, 50);
      subRegion->destroyRegion();
      ASSERT(subRegion->isDestroyed(), "Expected region is not destroyed ");
      subRegion = nullptr;
      ASSERT(ACE_OS::stat(fileName, &fileStat) == -1,
             "persistence file still present");
    }
    // cache close
    cachePtr->close();
  }
END_TEST(OverFlowTest_HeapLRU)

BEGIN_TEST(OverFlowTest_MultiThreaded)
  {
    /** Creating a cache to manage regions. */
    CachePtr cachePtr = CacheFactory::createCacheFactory()->create();
    ASSERT(cachePtr != nullptr, "Expected cache to be NON-nullptr");

    RegionAttributesPtr attrsPtr;
    setAttributes(attrsPtr);
    ASSERT(attrsPtr != nullptr, "Expected region attributes to be NON-nullptr");
    /** Create a region with caching and LRU. */

    RegionPtr regionPtr;
    CacheImpl* cacheImpl = CacheRegionHelper::getCacheImpl(cachePtr.get());
    cacheImpl->createRegion("OverFlowRegion", attrsPtr, regionPtr);
    ASSERT(regionPtr != nullptr, "Expected regionPtr to be NON-nullptr");
    validateAttribute(regionPtr);

    /** test to verify same region repeatedly to ensure that the persistece
    files are created and destroyed correctly */

    /** put some values into the cache. */
    PutThread* threads[4];

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
    CachePtr cachePtr = CacheFactory::createCacheFactory()->create();
    ASSERT(cachePtr != nullptr, "Expected cache to be NON-nullptr");

    RegionAttributesPtr attrsPtr;
    setAttributes(attrsPtr);
    ASSERT(attrsPtr != nullptr, "Expected region attributes to be NON-nullptr");
    /** Create a region with caching and LRU. */

    RegionPtr regionPtr;
    CacheImpl* cacheImpl = CacheRegionHelper::getCacheImpl(cachePtr.get());
    cacheImpl->createRegion("OverFlowRegion", attrsPtr, regionPtr);
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
