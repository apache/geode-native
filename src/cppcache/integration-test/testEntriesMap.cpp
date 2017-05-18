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

#define ROOT_NAME "testEntriesMap"

#include "fw_helper.hpp"

#ifdef WIN32

BEGIN_TEST(NotOnWindows)
  { LOG("Too many non-external symbols used too fix right now on windows."); }
END_TEST(NotOnWindows)

#else

//#define BUILD_CPPCACHE 1
#include <geode/GeodeCppCache.hpp>
#include <LRUEntriesMap.hpp>
#include <LRUMapEntry.hpp>
#include <VersionTag.hpp>
#include <cstdlib>
#include <LocalRegion.hpp>

using namespace apache::geode::client;
using namespace std;

typedef std::vector<MapEntryImplPtr> VectorOfMapEntry;

CacheableStringPtr createCacheable(const char* value) {
  CacheableStringPtr result = CacheableString::create(value);
  ASSERT(result != nullptr, "expected result non-NULL");
  return result;
}

namespace {
class FakeRegion : public RegionInternal {
 public:
  FakeRegion() : RegionInternal(nullptr) {}
  virtual void registerKeys(const VectorOfCacheableKey& keys,
                            bool isDurable = false,
                            bool getInitialValues = false,
                            bool receiveValues = true) override {}
  virtual void unregisterKeys(const VectorOfCacheableKey& keys) override {}
  virtual void registerAllKeys(bool isDurable = false,
                               VectorOfCacheableKeyPtr resultKeys = nullptr,
                               bool getInitialValues = false,
                               bool receiveValues = true) override {}
  virtual void unregisterAllKeys() override {}

  virtual void registerRegex(const char* regex, bool isDurable = false,
                             VectorOfCacheableKeyPtr resultKeys = nullptr,
                             bool getInitialValues = false,
                             bool receiveValues = true) override {}
  virtual void unregisterRegex(const char* regex) override {}

  virtual SelectResultsPtr query(
      const char* predicate,
      uint32_t timeout = DEFAULT_QUERY_RESPONSE_TIMEOUT) override {
    return nullptr;
  }
  virtual bool existsValue(
      const char* predicate,
      uint32_t timeout = DEFAULT_QUERY_RESPONSE_TIMEOUT) override {
    return false;
  }
  virtual SerializablePtr selectValue(
      const char* predicate,
      uint32_t timeout = DEFAULT_QUERY_RESPONSE_TIMEOUT) override {
    return nullptr;
  }
  virtual PersistenceManagerPtr getPersistenceManager() override {
    return nullptr;
  }
  virtual void setPersistenceManager(PersistenceManagerPtr& pmPtr) override{};

  virtual GfErrType getNoThrow(const CacheableKeyPtr& key, CacheablePtr& value,
                               const UserDataPtr& aCallbackArgument) override {
    return GF_NOERR;
  }
  virtual GfErrType getAllNoThrow(
      const VectorOfCacheableKey& keys, const HashMapOfCacheablePtr& values,
      const HashMapOfExceptionPtr& exceptions, bool addToLocalCache,
      const UserDataPtr& aCallbackArgument) override {
    return GF_NOERR;
  }
  virtual GfErrType putNoThrow(const CacheableKeyPtr& key,
                               const CacheablePtr& value,
                               const UserDataPtr& aCallbackArgument,
                               CacheablePtr& oldValue, int updateCount,
                               const CacheEventFlags eventFlags,
                               VersionTagPtr versionTag,
                               DataInput* delta = NULL,
                               EventIdPtr eventId = nullptr) override {
    return GF_NOERR;
  }
  virtual GfErrType createNoThrow(const CacheableKeyPtr& key,
                                  const CacheablePtr& value,
                                  const UserDataPtr& aCallbackArgument,
                                  int updateCount,
                                  const CacheEventFlags eventFlags,
                                  VersionTagPtr versionTag) override {
    return GF_NOERR;
  }
  virtual GfErrType destroyNoThrow(const CacheableKeyPtr& key,
                                   const UserDataPtr& aCallbackArgument,
                                   int updateCount,
                                   const CacheEventFlags eventFlags,
                                   VersionTagPtr versionTag) override {
    return GF_NOERR;
  }
  virtual GfErrType removeNoThrow(const CacheableKeyPtr& key,
                                  const CacheablePtr& value,
                                  const UserDataPtr& aCallbackArgument,
                                  int updateCount,
                                  const CacheEventFlags eventFlags,
                                  VersionTagPtr versionTag) override {
    return GF_NOERR;
  }
  virtual GfErrType invalidateNoThrow(const CacheableKeyPtr& keyPtr,
                                      const UserDataPtr& aCallbackArgument,
                                      int updateCount,
                                      const CacheEventFlags eventFlags,
                                      VersionTagPtr versionTag) override {
    return GF_NOERR;
  }
  virtual GfErrType invalidateRegionNoThrow(
      const UserDataPtr& aCallbackArgument,
      const CacheEventFlags eventFlags) override {
    return GF_NOERR;
  }
  virtual GfErrType destroyRegionNoThrow(
      const UserDataPtr& aCallbackArgument, bool removeFromParent,
      const CacheEventFlags eventFlags) override {
    return GF_NOERR;
  }

  virtual void setRegionExpiryTask() override {}
  virtual void acquireReadLock() override {}
  virtual void releaseReadLock() override {}
  // behaviors for attributes mutator
  virtual uint32_t adjustLruEntriesLimit(uint32_t limit) override { return 0; }
  virtual ExpirationAction::Action adjustRegionExpiryAction(
      ExpirationAction::Action action) override {
    return action;
  }
  virtual ExpirationAction::Action adjustEntryExpiryAction(
      ExpirationAction::Action action) override {
    return action;
  }
  virtual int32_t adjustRegionExpiryDuration(int32_t duration) override {
    return 0;
  }
  virtual int32_t adjustEntryExpiryDuration(int32_t duration) override {
    return 0;
  }
  virtual void adjustCacheListener(const CacheListenerPtr& aListener) override {
  }
  virtual void adjustCacheListener(const char* libpath,
                                   const char* factoryFuncName) override {}
  virtual void adjustCacheLoader(const CacheLoaderPtr& aLoader) override {}
  virtual void adjustCacheLoader(const char* libpath,
                                 const char* factoryFuncName) override {}
  virtual void adjustCacheWriter(const CacheWriterPtr& aWriter) override {}
  virtual void adjustCacheWriter(const char* libpath,
                                 const char* factoryFuncName) override {}
  virtual RegionStats* getRegionStats() override { return nullptr; }
  virtual bool cacheEnabled() override { return 0; }
  virtual bool isDestroyed() const override { return 0; }
  virtual void evict(int32_t percentage) override {}
  virtual CacheImpl* getCacheImpl() const override { return nullptr; }
  virtual TombstoneListPtr getTombstoneList() override { return nullptr; }

  virtual void updateAccessAndModifiedTime(bool modified) override {}
  virtual void updateAccessAndModifiedTimeForEntry(MapEntryImplPtr& ptr,
                                                   bool modified) override {}
  virtual void addDisMessToQueue() override {}

  virtual void txDestroy(const CacheableKeyPtr& key,
                         const UserDataPtr& callBack,
                         VersionTagPtr versionTag) override {}
  virtual void txInvalidate(const CacheableKeyPtr& key,
                            const UserDataPtr& callBack,
                            VersionTagPtr versionTag) override {}
  virtual void txPut(const CacheableKeyPtr& key, const CacheablePtr& value,
                     const UserDataPtr& callBack,
                     VersionTagPtr versionTag) override {}
  virtual const PoolPtr& getPool() override { throw "not implemented"; }

  virtual void destroyRegion(
      const UserDataPtr& aCallbackArgument = nullptr) override {}
  virtual void clear(const UserDataPtr& aCallbackArgument = nullptr) override {}
  virtual void localClear(
      const UserDataPtr& aCallbackArgument = nullptr) override {}
  virtual void localDestroyRegion(
      const UserDataPtr& aCallbackArgument = nullptr) override {}
  virtual RegionPtr getSubregion(const char* path) override { return nullptr; }
  virtual RegionPtr createSubregion(
      const char* subregionName,
      const RegionAttributesPtr& aRegionAttributes) override {
    return nullptr;
  }
  virtual void subregions(const bool recursive, VectorOfRegion& sr) override {}
  virtual RegionEntryPtr getEntry(const CacheableKeyPtr& key) override {
    return nullptr;
  }

  virtual CacheablePtr get(
      const CacheableKeyPtr& key,
      const UserDataPtr& aCallbackArgument = nullptr) override {
    return nullptr;
  }

  virtual void put(const CacheableKeyPtr& key, const CacheablePtr& value,
                   const UserDataPtr& aCallbackArgument = nullptr) override {}

  virtual void putAll(const HashMapOfCacheable& map,
                      uint32_t timeout = DEFAULT_RESPONSE_TIMEOUT,
                      const UserDataPtr& aCallbackArgument = nullptr) override {
  }
  virtual void localPut(
      const CacheableKeyPtr& key, const CacheablePtr& value,
      const UserDataPtr& aCallbackArgument = nullptr) override {}

  virtual void localCreate(
      const CacheableKeyPtr& key, const CacheablePtr& value,
      const UserDataPtr& aCallbackArgument = nullptr) override {}
  virtual void invalidate(
      const CacheableKeyPtr& key,
      const UserDataPtr& aCallbackArgument = nullptr) override {}

  virtual void localInvalidate(
      const CacheableKeyPtr& key,
      const UserDataPtr& aCallbackArgument = nullptr) override {}
  virtual void destroy(
      const CacheableKeyPtr& key,
      const UserDataPtr& aCallbackArgument = nullptr) override {}

  virtual void localDestroy(
      const CacheableKeyPtr& key,
      const UserDataPtr& aCallbackArgument = nullptr) override {}

  virtual bool remove(const CacheableKeyPtr& key, const CacheablePtr& value,
                      const UserDataPtr& aCallbackArgument = nullptr) override {
    return false;
  }

  virtual bool removeEx(
      const CacheableKeyPtr& key,
      const UserDataPtr& aCallbackArgument = nullptr) override {
    return false;
  }

  virtual bool localRemove(
      const CacheableKeyPtr& key, const CacheablePtr& value,
      const UserDataPtr& aCallbackArgument = nullptr) override {
    return false;
  }

  virtual bool localRemoveEx(
      const CacheableKeyPtr& key,
      const UserDataPtr& aCallbackArgument = nullptr) override {
    return false;
  }
  virtual void keys(VectorOfCacheableKey& v) override {}
  virtual void serverKeys(VectorOfCacheableKey& v) override {}
  virtual void values(VectorOfCacheable& vc) override {}
  virtual void entries(VectorOfRegionEntry& me, bool recursive) override {}
  virtual void getAll(const VectorOfCacheableKey& keys,
                      HashMapOfCacheablePtr values,
                      HashMapOfExceptionPtr exceptions,
                      bool addToLocalCache = false,
                      const UserDataPtr& aCallbackArgument = nullptr) override {
  }
  virtual void removeAll(
      const VectorOfCacheableKey& keys,
      const UserDataPtr& aCallbackArgument = nullptr) override {}
  virtual uint32_t size() override { return 0; }
  virtual const char* getName() const override { return nullptr; }
  virtual const char* getFullPath() const override { return nullptr; }
  virtual RegionPtr getParentRegion() const override { return nullptr; }
  virtual RegionAttributesPtr getAttributes() const override { return nullptr; }
  virtual AttributesMutatorPtr getAttributesMutator() const override {
    return nullptr;
  }
  virtual CacheStatisticsPtr getStatistics() const override { return nullptr; }
  virtual void invalidateRegion(
      const UserDataPtr& aCallbackArgument = nullptr) override {}
  virtual void localInvalidateRegion(
      const UserDataPtr& aCallbackArgument = nullptr) override {}
  virtual void create(const CacheableKeyPtr& key, const CacheablePtr& value,
                      const UserDataPtr& aCallbackArgument = nullptr) override {
  }
  virtual RegionServicePtr getRegionService() const override { return nullptr; }
  virtual bool containsValueForKey(
                                   const CacheableKeyPtr& keyPtr) const override {return false;}
  virtual bool containsKey(const CacheableKeyPtr& keyPtr) const override {
    return false;
  }
  virtual bool containsKeyOnServer(
                                   const CacheableKeyPtr& keyPtr) const override {return false;}
  virtual void getInterestList(VectorOfCacheableKey& vlist) const override {}
  virtual void getInterestListRegex(
      VectorOfCacheableString& vregex) const override {}
};
}

BEGIN_TEST(PutAndGet)
  {
    CacheableStringPtr ccstr = createCacheable("100");
    CacheablePtr ct = ccstr;
    EntryFactory* entryFactory = EntryFactory::singleton;
    AttributesFactory af;
    auto region = std::make_shared<FakeRegion>();
    EntriesMap* entries =
        new ConcurrentEntriesMap(entryFactory, false, region.get());
    entries->open();
    CacheableKeyPtr keyPtr = CacheableKey::create((char*)"foobar");
    ASSERT(keyPtr != nullptr, "expected keyPtr non-NULL");
    MapEntryImplPtr me;
    VersionTagPtr versionTag;
    CacheablePtr oldValue;
    entries->put(keyPtr, ct, me, oldValue, -1, 0, versionTag);
    CacheablePtr myValuePtr;
    entries->get(keyPtr, myValuePtr, me);
    ASSERT(myValuePtr != nullptr, "expected non-NULL");
    auto strValue = std::dynamic_pointer_cast<CacheableString>(myValuePtr);
    ASSERT(ccstr->operator==(*strValue), "expected 100");
    delete entries;
  }
END_TEST(PutAndGet)

BEGIN_TEST(CheckMapEntryImplPtr)
  {
    char error[1000] ATTR_UNUSED;
    MapEntryImplPtr mePtr;
    ASSERT(mePtr == nullptr, "expected mePtr to be NULL");
    CacheableKeyPtr keyPtr = CacheableKey::create(fwtest_Name);
    ASSERT(keyPtr != nullptr, "expected keyPtr non-NULL");
    EntryFactory::singleton->newMapEntry(keyPtr, mePtr);
    ASSERT(mePtr != nullptr, "expected to not be null.");
  }
END_TEST(CheckMapEntryImplPtr)

BEGIN_TEST(RemoveTest)
  {
    CacheableStringPtr cst = createCacheable("200");
    CacheablePtr ct = cst;
    EntryFactory* entryFactory = EntryFactory::singleton;
    auto region = std::make_shared<FakeRegion>();
    EntriesMap* entries =
        new ConcurrentEntriesMap(entryFactory, false, region.get());
    entries->open();
    CacheableKeyPtr keyPtr = CacheableKey::create(fwtest_Name);
    MapEntryImplPtr me;
    ASSERT(keyPtr != nullptr, "expected keyPtr non-NULL");
    CacheablePtr oldValue;
    VersionTagPtr versionTag;
    entries->put(keyPtr, ct, me, oldValue, -1, 0, versionTag);
    CacheablePtr myValuePtr;
    (void)entries->remove(keyPtr, myValuePtr, me, -1, versionTag, false);
    auto resPtr = std::dynamic_pointer_cast<CacheableString>(myValuePtr);
    ASSERT(myValuePtr != nullptr, "expected to not be null.");
    ASSERT(resPtr->operator==(*createCacheable("200")),
           "CustomerType with m_foobar 200.");
    (void)entries->remove(keyPtr, myValuePtr, me, -1, versionTag, false);
    ASSERT(myValuePtr == nullptr,
           "expected already removed, and null result should clear ptr.");
  }
END_TEST(RemoveTest)

BEGIN_TEST(GetEntryTest)
  {
    CacheableStringPtr cst = createCacheable("200");
    CacheablePtr ct = cst;
    EntryFactory* entryFactory = EntryFactory::singleton;
    auto region = std::make_shared<FakeRegion>();
    EntriesMap* entries =
        new ConcurrentEntriesMap(entryFactory, false, region.get());
    entries->open();
    CacheableKeyPtr keyPtr;
    MapEntryImplPtr me;
    keyPtr = CacheableKey::create(fwtest_Name);
    ASSERT(keyPtr != nullptr, "expected keyPtr non-NULL");
    CacheablePtr oldValue;
    VersionTagPtr versionTag;
    entries->put(keyPtr, ct, me, oldValue, -1, 0, versionTag);
    MapEntryImplPtr mePtr;
    CacheablePtr ctPtr;
    entries->getEntry(keyPtr, mePtr, ctPtr);
    ASSERT(mePtr != nullptr, "should not be null.");
    auto valPtr = std::dynamic_pointer_cast<CacheableString>(ctPtr);
    ASSERT(valPtr->operator==(*cst),
           "Entry should have a CustomerType Value of 200");
    CacheableKeyPtr keyPtr1;
    mePtr->getKey(keyPtr1);
    ASSERT(keyPtr1->operator==(*keyPtr), "should have same key.");
  }
END_TEST(GetEntryTest)

BEGIN_TEST(MapEntryImplPtrRCTest)
  {
    // Test Reference Counting and destruction for MapEntry.
    CacheableKeyPtr keyPtr = CacheableKey::create("foobar");
    ASSERT(keyPtr != nullptr, "expected keyPtr non-NULL");
    MapEntryImplPtr mePtr;
    EntryFactory ef;
    ef.newMapEntry(keyPtr, mePtr);
    CacheablePtr ct = createCacheable("someval");
    mePtr->setValue(ct);
  }
END_TEST(MapEntryImplPtrRCTest)

BEGIN_TEST(VectorOfMapEntryTestA)
  {
    VectorOfMapEntry* meVec = new VectorOfMapEntry();
    delete meVec;
  }
END_TEST(VectorOfMapEntryTestA)

BEGIN_TEST(VectorOfMapEntryTestB)
  {
    VectorOfMapEntry* meVec = new VectorOfMapEntry();
    meVec->resize(100);
    meVec->clear();
    meVec->resize(10);
    MapEntryImplPtr mePtr;
    for (int i = 0; i < 10; i++) {
      meVec->push_back(mePtr);
    }
    for (int j = 0; j < 10; j++) {
      meVec->pop_back();
    }
    delete meVec;
  }
END_TEST(VectorOfMapEntryTestB)

BEGIN_TEST(EntriesTest)
  {
    EntryFactory* entryFactory = EntryFactory::singleton;
    auto region = std::make_shared<FakeRegion>();
    EntriesMap* entries =
        new ConcurrentEntriesMap(entryFactory, false, region.get());
    entries->open();
    char keyBuf[100];
    char valBuf[100];
    VersionTagPtr versionTag;
    MapEntryImplPtr me;
    for (int i = 0; i < 10; i++) {
      sprintf(keyBuf, "key_%d", i);
      sprintf(valBuf, "%d", i);
      CacheableKeyPtr keyPtr = CacheableKey::create(keyBuf);
      ASSERT(keyPtr != nullptr, "expected keyPtr non-NULL");
      CacheablePtr v = createCacheable(valBuf);
      CacheablePtr oldValue;
      entries->put(keyPtr, v, me, oldValue, -1, 0, versionTag);
    }
    VectorOfRegionEntry* entriesVec = new VectorOfRegionEntry();
    entriesVec->resize(1);
    entries->entries(*entriesVec);
    // should be 10, but they are hashed so, we don't know what order they will
    // come in...
    int total = 0;
    int expectedTotal = 0;
    for (int k = 0; k < 10; k++) {
      expectedTotal += k;
      RegionEntryPtr rePtr = entriesVec->back();
      CacheableStringPtr ctPtr;
      CacheablePtr ccPtr;
      ccPtr = rePtr->getValue();
      ctPtr = std::dynamic_pointer_cast<CacheableString>(ccPtr);
      test::cout << "value is " << ctPtr->asChar() << test::endl;
      int val = atoi(ctPtr->asChar());
      test::cout << "atoi returned " << val << test::endl;
      total += val;
      entriesVec->pop_back();
    }
    entriesVec->clear();
    entriesVec->resize(0);
    delete entriesVec;
    sprintf(keyBuf, "total = %d, expected = %d", total, expectedTotal);
    ASSERT(total == expectedTotal, keyBuf);
    delete entries;
  }
END_TEST(EntriesTest)

BEGIN_TEST(ValuesTest)
  {
    EntryFactory* entryFactory = EntryFactory::singleton;
    auto region = std::make_shared<FakeRegion>();
    EntriesMap* entries =
        new ConcurrentEntriesMap(entryFactory, false, region.get());
    entries->open();
    char keyBuf[100];
    char valBuf[100];
    VersionTagPtr versionTag;
    MapEntryImplPtr me;
    for (int i = 0; i < 10; i++) {
      sprintf(keyBuf, "key_%d", i);
      sprintf(valBuf, "%d", i);
      CacheableKeyPtr keyPtr = CacheableKey::create(keyBuf);
      ASSERT(keyPtr != nullptr, "expected keyPtr non-NULL");
      CacheablePtr v = createCacheable(valBuf);
      CacheablePtr oldValue;
      entries->put(keyPtr, v, me, oldValue, -1, 0, versionTag);
    }
    VectorOfCacheable* valuesVec = new VectorOfCacheable();
    valuesVec->resize(1);
    entries->values(*valuesVec);
    // should be 10, but they are hashed so, we don't know what order they will
    // come in...
    int total = 0;
    int expectedTotal = 0;
    for (int k = 0; k < 10; k++) {
      expectedTotal += k;
      auto valuePtr =
          std::dynamic_pointer_cast<CacheableString>(valuesVec->back());
      total += atoi(valuePtr->asChar());
      valuesVec->pop_back();
    }
    delete valuesVec;
    sprintf(keyBuf, "total = %d, expected = %d", total, expectedTotal);
    ASSERT(total == expectedTotal, keyBuf);
    delete entries;
  }
END_TEST(ValuesTest)

BEGIN_TEST(KeysTest)
  {
    EntryFactory* entryFactory = EntryFactory::singleton;
    auto region = std::make_shared<FakeRegion>();
    EntriesMap* entries =
        new ConcurrentEntriesMap(entryFactory, false, region.get());
    entries->open();
    char keyBuf[100];
    char valBuf[100];
    VersionTagPtr versionTag;
    MapEntryImplPtr me;
    for (int i = 0; i < 10; i++) {
      sprintf(keyBuf, "key_%d", i);
      sprintf(valBuf, "%d", i);
      CacheableKeyPtr keyPtr = CacheableKey::create(keyBuf);
      ASSERT(keyPtr != nullptr, "expected keyPtr non-NULL");
      CacheablePtr v = createCacheable(valBuf);
      CacheablePtr oldValue;
      entries->put(keyPtr, v, me, oldValue, -1, 0, versionTag);
    }
    VectorOfCacheableKey keysVec;
    // keysVec.resize( 1 );
    entries->keys(keysVec);
    // should be 10, but they are hashed so, we don't know what order they will
    // come in...
    int total = 0;
    int expectedTotal = 0;
    for (int k = 0; k < 10; k++) {
      expectedTotal += k;
      CacheableKeyPtr keyPtr = keysVec.back();
      CacheablePtr cvPtr;
      entries->get(keyPtr, cvPtr, me);
      auto valuePtr = std::dynamic_pointer_cast<CacheableString>(cvPtr);
      total += atoi(valuePtr->asChar());
      keysVec.pop_back();
    }
    sprintf(keyBuf, "total = %d, expected = %d", total, expectedTotal);
    ASSERT(total == expectedTotal, keyBuf);
    delete entries;
  }
END_TEST(KeysTest)

BEGIN_TEST(TestRehash)
  {
    EntryFactory* entryFactory = EntryFactory::singleton;
    ConcurrentEntriesMap* entries =
        new ConcurrentEntriesMap(entryFactory, false, NULL, 1);
    entries->open(10);
    ASSERT(entries->totalSegmentRehashes() == 0,
           "should not have rehashed yet.");
    char keyBuf[100];
    char valBuf[100];
    MapEntryImplPtr me;

    for (uint32_t i = 0; i < 10000; i++) {
      sprintf(keyBuf, "key_%d", i);
      sprintf(valBuf, "%d", i);
      CacheableKeyPtr keyPtr = CacheableKey::create(keyBuf);
      ASSERT(keyPtr != nullptr, "expected keyPtr non-NULL");
      CacheablePtr v = createCacheable(valBuf);
      CacheablePtr oldValue;
      VersionTagPtr versionTag;
      entries->put(keyPtr, v, me, oldValue, -1, 0, versionTag);
    }
    // check rehash count...
    ASSERT(entries->totalSegmentRehashes() > 0,
           "should have rehashed several times.");
    // VectorOfMapEntry result ;
    // entries->entries( result );
    //  printf("entries->size()=%d\n", entries->size());
    ASSERT(entries->size() == 10000, "should be 10k items");
    for (uint32_t j = 0; j < 10000; j++) {
      sprintf(keyBuf, "key_%d", j);
      CacheableStringPtr valuePtr;
      CacheableKeyPtr keyPtr = CacheableKey::create(keyBuf);
      ASSERT(keyPtr != nullptr, "expected keyPtr non-NULL");
      CacheablePtr cvPtr;
      entries->get(keyPtr, cvPtr, me);
      valuePtr = std::dynamic_pointer_cast<CacheableString>(cvPtr);
      if (valuePtr == nullptr) {
        test::cout << "error finding key: " << keyBuf << test::endl;
        FAIL("should have found value for all keys after rehash.");
      }
    }
  }
END_TEST(TestRehash)

//---- LRU variants

BEGIN_TEST(LRUPutAndGet)
  {
    CacheableStringPtr cst = createCacheable("100");
    CacheablePtr ct = cst;
    MapEntryImplPtr me;
    EntryFactory* entryFactory = LRUEntryFactory::singleton;
    EntriesMap* entries = new LRUEntriesMap(
        entryFactory, nullptr, LRUAction::LOCAL_DESTROY, 20, false);
    entries->open();
    ASSERT(entries->size() == 0, "expected size 0.");
    CacheableKeyPtr keyPtr = CacheableKey::create("foobar");
    CacheablePtr oldValue;
    VersionTagPtr versionTag;
    entries->put(keyPtr, ct, me, oldValue, -1, 0, versionTag);
    ASSERT(entries->size() == 1, "expected size 1.");
    CacheableStringPtr myValuePtr;
    CacheablePtr cvPtr;
    ASSERT(keyPtr != nullptr, "expected keyPtr non-NULL");
    entries->get(keyPtr, cvPtr, me);
    myValuePtr = std::dynamic_pointer_cast<CacheableString>(cvPtr);
    ASSERT(myValuePtr != nullptr, "expected non-NULL");
    ASSERT(cst->operator==(*myValuePtr), "expected 100");
    delete entries;
  }
END_TEST(LRUPutAndGet)

BEGIN_TEST(CheckLRUMapEntryImplPtr)
  {
    char error[1000] ATTR_UNUSED;
    MapEntryImplPtr mePtr;
    ASSERT(mePtr == nullptr, "expected mePtr to be NULL");
    CacheableKeyPtr keyPtr = CacheableKey::create(fwtest_Name);
    ASSERT(keyPtr != nullptr, "expected keyPtr non-NULL");
    LRUEntryFactory::singleton->newMapEntry(keyPtr, mePtr);
    ASSERT(mePtr != nullptr, "expected to not be null.");
    auto lmePtr = std::dynamic_pointer_cast<LRUMapEntry>(mePtr);
    ASSERT(lmePtr != nullptr, "expected to cast successfully to LRUMapEntry.");
  }
END_TEST(LRUCheckMapEntryImplPtr)

BEGIN_TEST(LRURemoveTest)
  {
    CacheableStringPtr cst = createCacheable("200");
    CacheablePtr ct = cst;
    EntryFactory* entryFactory = LRUEntryFactory::singleton;
    EntriesMap* entries = new LRUEntriesMap(
        entryFactory, NULL, LRUAction::LOCAL_DESTROY, 20, false);
    entries->open();
    ASSERT(entries->size() == 0, "expected size 0.");
    CacheableKeyPtr keyPtr;
    MapEntryImplPtr me;
    keyPtr = CacheableKey::create(fwtest_Name);
    CacheablePtr oldValue;
    VersionTagPtr versionTag;
    entries->put(keyPtr, ct, me, oldValue, -1, 0, versionTag);
    ASSERT(entries->size() == 1, "expected size 1.");
    CacheableStringPtr myValuePtr;
    CacheablePtr cvPtr;
    (void)entries->remove(keyPtr, cvPtr, me, -1, versionTag, false);
    myValuePtr = std::dynamic_pointer_cast<CacheableString>(cvPtr);
    ASSERT(entries->size() == 0, "expected size 0.");
    ASSERT(cvPtr != nullptr, "expected to not be null.");
    ASSERT(myValuePtr->operator==(*createCacheable("200")),
           "CustomerType with m_foobar 200.");

    (void)entries->remove(keyPtr, cvPtr, me, -1, versionTag, false);
    ASSERT(cvPtr == nullptr,
           "expected already removed, and null result should clear ptr.");
  }
END_TEST(LRURemoveTest)

BEGIN_TEST(LRUGetEntryTest)
  {
    CacheableStringPtr cst = createCacheable("200");
    CacheablePtr ct = cst;
    EntryFactory* entryFactory = LRUEntryFactory::singleton;
    EntriesMap* entries = new LRUEntriesMap(
        entryFactory, NULL, LRUAction::LOCAL_DESTROY, 20, false);
    entries->open();
    CacheableKeyPtr keyPtr;
    MapEntryImplPtr me;
    keyPtr = CacheableKey::create(fwtest_Name);
    ASSERT(keyPtr != nullptr, "expected keyPtr non-NULL");
    CacheablePtr oldValue;
    VersionTagPtr versionTag;
    entries->put(keyPtr, ct, me, oldValue, -1, 0, versionTag);
    ASSERT(entries->size() == 1, "expected size 1.");
    MapEntryImplPtr mePtr;
    CacheablePtr cvPtr;
    entries->getEntry(keyPtr, mePtr, cvPtr);
    ASSERT(mePtr != nullptr, "should not be null.");
    CacheableStringPtr ctPtr;
    ctPtr = std::dynamic_pointer_cast<CacheableString>(cvPtr);
    ASSERT(ctPtr->operator==(*cst),
           "Entry should have a CustomerType Value of 200");
    CacheableKeyPtr keyPtr1;
    mePtr->getKey(keyPtr1);
    ASSERT(keyPtr1->operator==(*keyPtr), "should have same key.");
  }
END_TEST(LRUGetEntryTest)

BEGIN_TEST(LRULimitEvictTest)
  {
    EntryFactory* entryFactory = LRUEntryFactory::singleton;
    EntriesMap* entries = new LRUEntriesMap(entryFactory, NULL,
                                            LRUAction::LOCAL_DESTROY, 5, false);
    entries->open();
    MapEntryImplPtr me;
    CacheablePtr ct = createCacheable("somevalue");
    CacheablePtr oldValue;
    CacheableKeyPtr keyPtr = CacheableKey::create("1");
    ASSERT(keyPtr != nullptr, "expected keyPtr non-NULL");
    VersionTagPtr versionTag;
    entries->put(keyPtr, ct, me, oldValue, -1, 0, versionTag);
    ASSERT(entries->size() == 1, "expected size 1.");
    keyPtr = CacheableKey::create("2");
    ASSERT(keyPtr != nullptr, "expected keyPtr non-NULL");
    entries->put(keyPtr, ct, me, oldValue, -1, 0, versionTag);
    ASSERT(entries->size() == 2, "expected size 2.");
    keyPtr = CacheableKey::create("3");
    ASSERT(keyPtr != nullptr, "expected keyPtr non-NULL");
    entries->put(keyPtr, ct, me, oldValue, -1, 0, versionTag);
    ASSERT(entries->size() == 3, "expected size 3.");
    keyPtr = CacheableKey::create("4");
    ASSERT(keyPtr != nullptr, "expected keyPtr non-NULL");
    entries->put(keyPtr, ct, me, oldValue, -1, 0, versionTag);
    ASSERT(entries->size() == 4, "expected size 4.");
    keyPtr = CacheableKey::create("5");
    ASSERT(keyPtr != nullptr, "expected keyPtr non-NULL");
    entries->put(keyPtr, ct, me, oldValue, -1, 0, versionTag);
    ASSERT(entries->size() == 5, "expected size 5.");
    LOG("Map is now at the limit.");
    keyPtr = CacheableKey::create("6");
    ASSERT(keyPtr != nullptr, "expected keyPtr non-NULL");
    LOG("About to spill over.");
    entries->put(keyPtr, ct, me, oldValue, -1, 0, versionTag);
    LOG("Spilled over.");
    ASSERT(entries->size() == 5, "expected size 5.");
    LOG("Limit was preserved.");
  }
END_TEST(LRULimitEvictTest)

#endif
