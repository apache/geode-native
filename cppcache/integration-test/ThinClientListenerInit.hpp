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

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTLISTENERINIT_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTLISTENERINIT_H_

#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"
#include "TallyListener.hpp"
#include "TallyWriter.hpp"
#include "TallyLoader.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1

namespace {  // NOLINT(google-build-namespaces)

using apache::geode::client::Cacheable;

using apache::geode::client::testing::TallyListener;
using apache::geode::client::testing::TallyLoader;
using apache::geode::client::testing::TallyWriter;

static bool isLocator = false;
static bool isLocalServer = true;
static int numberOfLocators = 1;
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);
const char* poolName = "__TESTPOOL1_";
std::shared_ptr<TallyListener> reg1Listener1, reg1Listener2;
std::shared_ptr<TallyWriter> reg1Writer1, reg1Writer2;
std::shared_ptr<TallyLoader> reg1Loader1, reg1Loader2;
int numCreates = 0;
int numUpdates = 0;
int numLoads = 0;

#include "LocatorHelper.hpp"

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
    if (key && (!rp.getAttributes().getEndpoints().empty() ||
                !rp.getAttributes().getPoolName().empty())) {
      LOG_DEBUG("Putting the value (%s) for local region clients only ",
               lstrvalue);
      rp.put(key, lreturnValue);
    }
    return std::move(lreturnValue);
  }
};

void setCacheListener(const char* regName,
                      std::shared_ptr<TallyListener> regListener) {
  auto reg = getHelper()->getRegion(regName);
  auto attrMutator = reg->getAttributesMutator();
  attrMutator->setCacheListener(regListener);
}

void setCacheLoader(const char* regName,
                    std::shared_ptr<TallyLoader> regLoader) {
  auto reg = getHelper()->getRegion(regName);
  auto attrMutator = reg->getAttributesMutator();
  attrMutator->setCacheLoader(regLoader);
}

void setCacheWriter(const char* regName,
                    std::shared_ptr<TallyWriter> regWriter) {
  auto reg = getHelper()->getRegion(regName);
  auto attrMutator = reg->getAttributesMutator();
  attrMutator->setCacheWriter(regWriter);
}

void validateEventCount(int line) {
  LOG_INFO("ValidateEvents called from line (%d).", line);
  ASSERT(reg1Listener1->getCreates() == numCreates,
         "Got wrong number of creation events.");
  ASSERT(reg1Listener1->getUpdates() == numUpdates,
         "Got wrong number of update events.");
  ASSERT(reg1Loader1->getLoads() == numLoads,
         "Got wrong number of loader events.");
  ASSERT(reg1Writer1->getCreates() == numCreates,
         "Got wrong number of writer events.");
}

DUNIT_TASK_DEFINITION(SERVER1, StartServer)
  {
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_notify_subscription.xml");
    }
    LOG("SERVER started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, InitClientEvents)
  {
    numCreates = 0;
    numUpdates = 0;
    numLoads = 0;
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, SetupClient_Pooled_Locator)
  {
    initClient(true);
    reg1Listener1 = std::make_shared<TallyListener>();
    createPooledRegion(regionNames[0], false, locatorsG, poolName, true,
                       reg1Listener1);
    reg1Loader1 = std::make_shared<ThinClientTallyLoader>();
    reg1Writer1 = std::make_shared<TallyWriter>();
    setCacheLoader(regionNames[0], reg1Loader1);
    setCacheWriter(regionNames[0], reg1Writer1);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, testLoaderAndWriter)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    auto keyPtr = CacheableKey::create(keys[0]);
    std::vector<std::shared_ptr<CacheableKey>> keysVector;
    keysVector.push_back(keyPtr);
    regPtr->registerKeys(keysVector);

    /*NIL: Changed the asserion due to the change in invalidate.
      Now we create new entery for every invalidate event received or
      localInvalidate call
      so expect  containsKey to returns true insted of false earlier. */
    ASSERT(regPtr->containsKey(keyPtr), "Key should found in region.");
    // now having all the Callbacks set, lets call the loader and writer
    ASSERT(regPtr->get(keyPtr) != nullptr, "Expected non null value");

    auto regEntryPtr = regPtr->getEntry(keyPtr);
    auto valuePtr = regEntryPtr->getValue();
    int val = atoi(valuePtr->toString().c_str());
    LOG_FINE("val for keyPtr is %d", val);
    ASSERT(val == 0, "Expected value CacheLoad value should be 0");
    numLoads++;
    numCreates++;

    validateEventCount(__LINE__);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, testCreatesAndUpdates)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    auto keyPtr1 = CacheableKey::create(keys[1]);
    auto keyPtr2 = CacheableKey::create(keys[2]);
    std::vector<std::shared_ptr<CacheableKey>> keysVector;
    keysVector.push_back(keyPtr1);
    keysVector.push_back(keyPtr2);
    regPtr->registerKeys(keysVector);

    // Do a create followed by a create on the same key
    /*NIL: Changed the asserion due to the change in invalidate.
      Now we create new entery for every invalidate event received or
      localInvalidate call
      so expect  containsKey to returns true insted of false earlier.
      and used put call to update value for that key, instead of create call
      earlier or will get
      Region::create: Entry already exists in the region  */
    ASSERT(regPtr->containsKey(keyPtr1), "Key should found in region.");
    regPtr->put(keyPtr1, vals[1]);
    numCreates++;
    validateEventCount(__LINE__);

    /**
    see bug #252
    try {
      regPtr->create(keyPtr1, nvals[1]);
      ASSERT(nullptr, "Expected an EntryExistsException");
    } catch(EntryExistsException &) {
      //Expected Behavior
    }
    validateEventCount(__LINE__);
    **/

    // Trigger an update event from a put
    regPtr->put(keyPtr1, nvals[1]);
    numUpdates++;
    validateEventCount(__LINE__);

    // This put creates a new value, verify update event is not received
    regPtr->put(keyPtr2, vals[2]);
    numCreates++;
    validateEventCount(__LINE__);

    // Do a get on a preexisting value to confirm no events are triggered
    regPtr->get(keyPtr2);
    validateEventCount(__LINE__);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, testDestroy)
  {
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    auto keyPtr1 = CacheableKey::create(keys[1]);
    auto keyPtr2 = CacheableKey::create(keys[2]);
    regPtr->localInvalidate(keyPtr1);
    // Verify no listener activity after the invalidate
    validateEventCount(__LINE__);

    // Verify after update listener activity after a get on an invalidated value
    regPtr->get(keyPtr1);
    auto regEntryPtr = regPtr->getEntry(keyPtr1);
    auto valuePtr = regEntryPtr->getValue();
    int val = atoi(valuePtr->toString().c_str());
    LOG_FINE("val for keyPtr1 is %d", val);
    ASSERT(val == 0, "Expected value CacheLoad value should be 0");
    numUpdates++;
    validateEventCount(__LINE__);

    regPtr->destroy(keyPtr2);
    regPtr->get(keyPtr2);
    auto regEntryPtr1 = regPtr->getEntry(keyPtr2);
    auto valuePtr1 = regEntryPtr1->getValue();
    int val1 = atoi(valuePtr1->toString().c_str());
    LOG_FINE("val1 for keyPtr2 is %d", val1);
    ASSERT(val1 == 1, "Expected value CacheLoad value should be 1");
    numLoads++;
    numCreates++;
    validateEventCount(__LINE__);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, StopServer)
  {
    if (isLocalServer) CacheHelper::closeServer(1);
    LOG("SERVER stopped");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK_DEFINITION

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTLISTENERINIT_H_
