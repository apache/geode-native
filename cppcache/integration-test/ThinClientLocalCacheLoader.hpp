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

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTLOCALCACHELOADER_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTLOCALCACHELOADER_H_

#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"
#include "TallyLoader.hpp"
#include "ThinClientHelper.hpp"

#define CLIENT1 s1p1
#define SERVER1 s2p1

namespace {  // NOLINT(google-build-namespaces)

using apache::geode::client::Cacheable;
using apache::geode::client::CacheFactory;

using apache::geode::client::testing::TallyLoader;

std::shared_ptr<TallyLoader> reg1Loader1;
int numLoads = 0;
std::shared_ptr<Cache> cachePtr;
std::shared_ptr<Region> regionPtr;

class ThinClientTallyLoader : public TallyLoader {
 public:
  ThinClientTallyLoader() : TallyLoader() {}

  ~ThinClientTallyLoader() noexcept override = default;

  std::shared_ptr<Cacheable> load(
      Region& rp, const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument) override {
    int32_t loadValue = std::dynamic_pointer_cast<CacheableInt32>(
                            TallyLoader::load(rp, key, aCallbackArgument))
                            ->value();
    char lstrvalue[32];
    sprintf(lstrvalue, "%i", loadValue);
    auto lreturnValue = CacheableString::create(lstrvalue);
    if (key != nullptr && (!rp.getAttributes().getEndpoints().empty() ||
                           !rp.getAttributes().getPoolName().empty())) {
      LOG_DEBUG("Putting the value (%s) for local region clients only ",
               lstrvalue);
      rp.put(key, lreturnValue);
    }
    return std::move(lreturnValue);
  }

  void close(Region& region) override {
    LOG(" ThinClientTallyLoader::close() called");
    LOG_INFO(" Region %s is Destroyed = %d ", region.getName().c_str(),
            region.isDestroyed());
    ASSERT(region.isDestroyed() == true,
           "region.isDestroyed should return true");
    /*
    if(region.get() != nullptr && region.get()->getCache() != nullptr){
      LOG_INFO(" Cache Name is Closed = %d ",
    region.get()->getCache()->isClosed());
    }else{
      LOG_INFO(" regionPtr or cachePtr is nullptr");
    }
    */
  }
};

void validateEventCount(int line) {
  LOG_INFO("ValidateEvents called from line (%d).", line);
  ASSERT(reg1Loader1->getLoads() == numLoads,
         "Got wrong number of loader events.");
}

DUNIT_TASK_DEFINITION(SERVER1, StartServer)
  {
    CacheHelper::initServer(1, "cacheserver_loader.xml");
    LOG("SERVER started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, SetupClient)
  {
    // Create a Geode Cache with the "client_Loader.xml" Cache XML file.
    const char* clientXmlFile = "client_Loader.xml";
    static char* path = std::getenv("TESTSRC");
    std::string clientXml = path;
    clientXml += "/resources/";
    clientXml += clientXmlFile;
    auto cacheFactory = CacheFactory().set("cache-xml-file", clientXml.c_str());
    cachePtr = std::make_shared<Cache>(cacheFactory.create());
    LOG_INFO("Created the Geode Cache");

    // Get the example Region from the Cache which is declared in the Cache XML
    // file.
    regionPtr = cachePtr->getRegion("/root/exampleRegion");
    LOG_INFO("Obtained the Region from the Cache");

    // Plugin the ThinClientTallyLoader to the Region.
    auto attrMutatorPtr = regionPtr->getAttributesMutator();
    reg1Loader1 = std::make_shared<ThinClientTallyLoader>();
    attrMutatorPtr->setCacheLoader(reg1Loader1);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, InitClientEvents)
  {
    numLoads = 0;
    regionPtr = nullptr;
    cachePtr = nullptr;
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, testLoader)
  {
    auto keyPtr = CacheableKey::create("Key0");

    ASSERT(!regionPtr->containsKey(keyPtr),
           "Key should not have been found in region.");
    // now having the Callbacks set, lets call the loader
    ASSERT(regionPtr->get(keyPtr) != nullptr, "Expected non null value");

    auto regEntryPtr = regionPtr->getEntry(keyPtr);
    auto valuePtr = regEntryPtr->getValue();
    int val = atoi(valuePtr->toString().c_str());
    LOG_FINE("val for keyPtr is %d", val);
    numLoads++;
    validateEventCount(__LINE__);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, testDestroy)
  {
    auto keyPtr2 = CacheableKey::create("Key1");
    regionPtr->destroy(keyPtr2);
    // Verify the sequence destroy()->get() :- CacheLoader to be invoked.
    regionPtr->get(keyPtr2);
    auto regEntryPtr2 = regionPtr->getEntry(keyPtr2);
    auto valuePtr2 = regEntryPtr2->getValue();
    int val2 = atoi(valuePtr2->toString().c_str());
    LOG_FINE("val2 for keyPtr2 is %d", val2);
    numLoads++;
    validateEventCount(__LINE__);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, testInvalidateKey)
  {
    auto keyPtr2 = CacheableKey::create("Key2");
    regionPtr->put(keyPtr2, "Value2");
    regionPtr->invalidate(keyPtr2);
    // Verify the sequence invalidate()->get() :- CacheLoader to be invoked.
    regionPtr->get(keyPtr2);
    auto regEntryPtr = regionPtr->getEntry(keyPtr2);
    auto valuePtr = regEntryPtr->getValue();
    int val = atoi(valuePtr->toString().c_str());
    LOG_FINE("val for keyPtr1 is %d", val);
    numLoads++;
    validateEventCount(__LINE__);

    // Verify the sequence put()->invalidate()->get()->invalidate()->get() :-
    // CacheLoader to be invoked twice
    // once after each get.
    auto keyPtr4 = CacheableKey::create("Key4");
    regionPtr->put(keyPtr4, "Value4");
    regionPtr->invalidate(keyPtr4);
    regionPtr->get(keyPtr4);
    auto regEntryPtr1 = regionPtr->getEntry(keyPtr4);
    auto valuePtr1 = regEntryPtr1->getValue();
    int val1 = atoi(valuePtr1->toString().c_str());
    LOG_FINE("val1 for keyPtr4 is %d", val1);
    numLoads++;
    validateEventCount(__LINE__);

    regionPtr->invalidate(keyPtr4);
    regionPtr->get(keyPtr4);
    auto regEntryPtr2 = regionPtr->getEntry(keyPtr4);
    auto valuePtr2 = regEntryPtr2->getValue();
    int val2 = atoi(valuePtr2->toString().c_str());
    LOG_FINE("val2 for keyPtr4 is %d", val2);
    numLoads++;
    validateEventCount(__LINE__);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, testInvalidateRegion)
  {
    auto keyPtr3 = CacheableKey::create("Key3");
    regionPtr->put(keyPtr3, "Value3");
    // Verify the sequence invalidateRegion()->get() :- CacheLoader to be
    // invoked.
    regionPtr->invalidateRegion();
    regionPtr->get(keyPtr3);
    auto regEntryPtr = regionPtr->getEntry(keyPtr3);
    auto valuePtr = regEntryPtr->getValue();
    int val = atoi(valuePtr->toString().c_str());
    LOG_FINE("val for keyPtr3 is %d", val);
    numLoads++;
    validateEventCount(__LINE__);

    // Verify the sequence
    // put()->invalidateRegion()->get()->invalidateRegion()->get() :-
    // CacheLoader
    // to be invoked twice.
    // once after each get.
    auto keyPtr4 = CacheableKey::create("Key4");
    regionPtr->put(keyPtr4, "Value4");
    regionPtr->invalidateRegion();
    regionPtr->get(keyPtr4);
    auto regEntryPtr1 = regionPtr->getEntry(keyPtr4);
    auto valuePtr1 = regEntryPtr1->getValue();
    int val1 = atoi(valuePtr1->toString().c_str());
    LOG_FINE("val1 for keyPtr4 is %d", val1);
    numLoads++;
    validateEventCount(__LINE__);

    regionPtr->invalidateRegion();
    regionPtr->get(keyPtr4);
    auto regEntryPtr2 = regionPtr->getEntry(keyPtr4);
    auto valuePtr2 = regEntryPtr2->getValue();
    int val2 = atoi(valuePtr2->toString().c_str());
    LOG_FINE("val2 for keyPtr4 is %d", val2);
    numLoads++;
    validateEventCount(__LINE__);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, StopServer)
  {
    CacheHelper::closeServer(1);
    LOG("SERVER stopped");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK_DEFINITION

void runCacheLoaderTest() {
  CALL_TASK(InitClientEvents);
  CALL_TASK(StartServer);
  CALL_TASK(SetupClient);
  CALL_TASK(testLoader);
  CALL_TASK(testDestroy);
  CALL_TASK(testInvalidateKey);
  CALL_TASK(testInvalidateRegion);
  CALL_TASK(CloseCache1);
  CALL_TASK(StopServer);
}

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTLOCALCACHELOADER_H_
