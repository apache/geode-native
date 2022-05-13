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

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTDISTOPS2_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTDISTOPS2_H_

#include "fw_dunit.hpp"

#include <string>
#include <vector>

#define ROOT_NAME "ThinClientDistOps2"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "ThinClientHelper.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2

namespace {  // NOLINT(google-build-namespaces)

using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableInt64;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;
using apache::geode::client::CacheServerException;
using apache::geode::client::EntryExistsException;
using apache::geode::client::IllegalArgumentException;
using apache::geode::client::Properties;

static bool isLocalServer = false;
static bool isLocator = false;
static int numberOfLocators = 0;
const char* poolName = "__TEST_POOL1__";

const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);

#include "LocatorHelper.hpp"

#define verifyEntry(a, b, c, d) _verifyEntry(a, b, c, d, __LINE__)

void _verifyEntry(const std::string& name, const char* key, const char* val,
                  bool checkLocal, int line) {
  LOG(std::string("verifyEntry() called from ") + std::to_string(line) + ".");
  _verifyEntry(name, key, val, false, checkLocal);
  LOG("Entry verified.");
}

const char* _keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char* _vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char* _nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                        "New Value-4"};

const char* _regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    if (isLocalServer) CacheHelper::initServer(1);
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2And3)
  {
    if (isLocalServer) CacheHelper::initServer(2);
    if (isLocalServer) CacheHelper::initServer(3);
    LOG("SERVER23 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2And3_Locator)
  {
    if (isLocalServer) {
      CacheHelper::initServer(2, {}, locatorsG);
    }
    if (isLocalServer) {
      CacheHelper::initServer(3, {}, locatorsG);
    }
    LOG("SERVER23 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CreateClient1Regions_Pooled_Locator)
  {
    initClientWithPool(true, "__TEST_POOL1__", locatorsG, {}, nullptr, 0, true);
    createPooledRegion(_regionNames[0], USE_ACK, locatorsG, poolName);
    createPooledRegion(_regionNames[1], NO_ACK, locatorsG, poolName);
    LOG("CreateClient1Regions complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CreateClient2Regions_Pooled_Locator)
  {
    initClientWithPool(true, "__TEST_POOL1__", locatorsG, {}, nullptr, 0, true);
    createPooledRegion(_regionNames[0], USE_ACK, locatorsG, poolName);
    createPooledRegion(_regionNames[1], NO_ACK, locatorsG, poolName);
    LOG("CreateClient1Regions complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CreateClient1Entries)
  {
    createEntry(_regionNames[0], _keys[0], _vals[0]);
    createEntry(_regionNames[1], _keys[2], _vals[2]);
    LOG("CreateClient1Entries complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CreateClient2Entries)
  {
    doNetsearch(_regionNames[0], _keys[0], _vals[0]);
    doNetsearch(_regionNames[1], _keys[2], _vals[2]);
    createEntry(_regionNames[0], _keys[1], _vals[1]);
    createEntry(_regionNames[1], _keys[3], _vals[3]);
    LOG("CreateClient2Entries complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, UpdateClient1Entry)
  {
    doNetsearch(_regionNames[0], _keys[1], _vals[1]);
    doNetsearch(_regionNames[1], _keys[3], _vals[3]);
    updateEntry(_regionNames[0], _keys[0], _nvals[0]);
    updateEntry(_regionNames[1], _keys[2], _nvals[2]);
    LOG("UpdateClient1Entry complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, UpdateClient2Entry)
  {
    doNetsearch(_regionNames[0], _keys[0], _vals[0], false);
    doNetsearch(_regionNames[1], _keys[2], _vals[2], false);
    updateEntry(_regionNames[0], _keys[1], _nvals[1]);
    updateEntry(_regionNames[1], _keys[3], _nvals[3]);
    LOG("UpdateClient2Entry complete.");
  }
END_TASK_DEFINITION

// Test for getAll
DUNIT_TASK_DEFINITION(CLIENT1, Client1GetAll)
  {
    auto reg0 = getHelper()->getRegion(_regionNames[0]);

    std::vector<std::shared_ptr<CacheableKey>> keys0;
    auto key0 = CacheableString::create(_keys[0]);
    auto key1 = CacheableString::create(_keys[1]);

    // re-create region with caching enabled
    reg0->localDestroyRegion();
    reg0 = nullptr;
    getHelper()->createPooledRegion(regionNames[0], USE_ACK, {},
                                    "__TEST_POOL1__", true, true);
    reg0 = getHelper()->getRegion(_regionNames[0]);
    // check for IllegalArgumentException for empty key list
    keys0.clear();
    try {
      reg0->getAll(keys0);
      FAIL("Expected IllegalArgumentException");
    } catch (const IllegalArgumentException&) {
      LOG("Got expected IllegalArgumentException");
    }

    keys0.push_back(key0);
    keys0.push_back(key1);
    {
      auto values = reg0->getAll(keys0);
      ASSERT(values.size() == 2, "Expected 2 values");
      auto val0 = std::dynamic_pointer_cast<CacheableString>(values[key0]);
      auto val1 = std::dynamic_pointer_cast<CacheableString>(values[key1]);
      ASSERT(strcmp(_nvals[0], val0->value().c_str()) == 0,
             "Got unexpected value");
      ASSERT(strcmp(_nvals[1], val1->value().c_str()) == 0,
             "Got unexpected value");
    }

    // for second region invalidate only one key to have a partial get
    // from java server
    auto reg1 = getHelper()->getRegion(_regionNames[1]);
    auto key2 = CacheableString::create(_keys[2]);
    auto key3 = CacheableString::create(_keys[3]);
    reg1->localInvalidate(key2);
    std::vector<std::shared_ptr<CacheableKey>> keys1;
    keys1.push_back(key2);
    keys1.push_back(key3);

    {
      auto values = reg1->getAll(keys1);
      ASSERT(values.size() == 2, "Expected 2 values");
      auto val2 = std::dynamic_pointer_cast<CacheableString>(values[key2]);
      auto val3 = std::dynamic_pointer_cast<CacheableString>(values[key3]);
      ASSERT(strcmp(_nvals[2], val2->value().c_str()) == 0,
             "Got unexpected value");
      ASSERT(strcmp(_vals[3], val3->value().c_str()) == 0,
             "Got unexpected value");
    }

    // also check that the region is properly populated
    ASSERT(reg1->size() == 2, "Expected 2 entries in the region");
    auto regEntries = reg1->entries(false);
    ASSERT(regEntries.size() == 2, "Expected 2 entries in the region.entries");
    verifyEntry(_regionNames[1], _keys[2], _nvals[2], true);
    verifyEntry(_regionNames[1], _keys[3], _vals[3], true);

    // also check with nullptr values that region is properly populated
    {
      reg1->localInvalidate(key3);
      auto values = reg1->getAll(keys1);
      // now check that the region is properly populated
      ASSERT(reg1->size() == 2, "Expected 2 entries in the region");
      regEntries = reg1->entries(false);
      ASSERT(regEntries.size() == 2,
             "Expected 2 entries in the region.entries");
      verifyEntry(_regionNames[1], _keys[2], _nvals[2], true);
      verifyEntry(_regionNames[1], _keys[3], _nvals[3], true);
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, Client1GetAll_Pool)
  {
    {
      auto reg0 = getHelper()->getRegion(_regionNames[0]);

      std::vector<std::shared_ptr<CacheableKey>> keys0;
      auto key0 = CacheableString::create(_keys[0]);
      auto key1 = CacheableString::create(_keys[1]);

      getHelper()->createRegionAndAttachPool(_regionNames[0], USE_ACK,
                                             poolName);
      reg0 = getHelper()->getRegion(_regionNames[0]);
      // check for IllegalArgumentException for empty key list
      keys0.clear();
      try {
        reg0->getAll(keys0);
        FAIL("Expected IllegalArgumentException");
      } catch (const IllegalArgumentException&) {
        LOG("Got expected IllegalArgumentException");
      }

      keys0.push_back(key0);
      keys0.push_back(key1);

      auto values = reg0->getAll(keys0);

      ASSERT(values.size() == 2, "Expected 2 values");
      auto val0 = std::dynamic_pointer_cast<CacheableString>(values[key0]);
      auto val1 = std::dynamic_pointer_cast<CacheableString>(values[key1]);
      ASSERT(strcmp(_nvals[0], val0->value().c_str()) == 0,
             "Got unexpected value");
      ASSERT(strcmp(_nvals[1], val1->value().c_str()) == 0,
             "Got unexpected value");
    }

    {
      // for second region invalidate only one key to have a partial get
      // from java server
      auto reg1 = getHelper()->getRegion(_regionNames[1]);
      auto key2 = CacheableString::create(_keys[2]);
      auto key3 = CacheableString::create(_keys[3]);
      reg1->localInvalidate(key2);
      std::vector<std::shared_ptr<CacheableKey>> keys1;
      keys1.push_back(key2);
      keys1.push_back(key3);

      auto values = reg1->getAll(keys1);

      ASSERT(values.size() == 2, "Expected 2 values");
      auto val2 = std::dynamic_pointer_cast<CacheableString>(values[key2]);
      auto val3 = std::dynamic_pointer_cast<CacheableString>(values[key3]);
      ASSERT(strcmp(_nvals[2], val2->value().c_str()) == 0,
             "Got unexpected value");
      ASSERT(strcmp(_vals[3], val3->value().c_str()) == 0,
             "Got unexpected value");

      // also check that the region is properly populated
      ASSERT(reg1->size() == 2, "Expected 2 entries in the region");
      auto regEntries = reg1->entries(false);
      ASSERT(regEntries.size() == 2,
             "Expected 2 entries in the region.entries");
      verifyEntry(_regionNames[1], _keys[2], _nvals[2], true);
      verifyEntry(_regionNames[1], _keys[3], _vals[3], true);

      // also check with nullptr values that region is properly populated
      reg1->localInvalidate(key3);

      reg1->getAll(keys1);
      // now check that the region is properly populated
      ASSERT(reg1->size() == 2, "Expected 2 entries in the region");
      regEntries = reg1->entries(false);
      ASSERT(regEntries.size() == 2,
             "Expected 2 entries in the region.entries");
      verifyEntry(_regionNames[1], _keys[2], _nvals[2], true);
      verifyEntry(_regionNames[1], _keys[3], _nvals[3], true);
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CloseCache2)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer1)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CloseServer2)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
    }
    if (isLocalServer) {
      CacheHelper::closeServer(3);
      LOG("SERVER3 stopped");
    }
  }
END_TASK_DEFINITION

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTDISTOPS2_H_
