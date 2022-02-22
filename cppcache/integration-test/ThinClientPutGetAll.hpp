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

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTPUTGETALL_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTPUTGETALL_H_

#include "fw_dunit.hpp"
#include <string>
#include <unordered_map>

#define ROOT_NAME "ThinClientPutGetAll"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "ThinClientHelper.hpp"
#include "testobject/VariousPdxTypes.hpp"
#include "testobject/PdxClassV1.hpp"
#include "testobject/PdxClassV2.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1

namespace {  // NOLINT(google-build-namespaces)

using PdxTests::PdxTypes1;
using PdxTests::PdxTypes10;
using PdxTests::PdxTypes2;
using PdxTests::PdxTypes3;
using PdxTests::PdxTypes4;
using PdxTests::PdxTypes5;
using PdxTests::PdxTypes6;
using PdxTests::PdxTypes7;
using PdxTests::PdxTypes8;
using PdxTests::PdxTypes9;

using apache::geode::client::Cacheable;
using apache::geode::client::HashMapOfCacheable;
using apache::geode::client::IllegalStateException;

static bool isLocalServer = false;
static bool isLocator = false;
static int numberOfLocators = 0;

const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);
const char* poolName = "__TEST_POOL1__";

const char* _keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char* _vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char* _nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                        "New Value-4"};
const char* _regionNames[] = {"DistRegionAck"};

#include "LocatorHelper.hpp"

void verifyGetAll(std::shared_ptr<Region> region, const char** _values,
                  int startIndex,
                  std::shared_ptr<Cacheable> callBack = nullptr) {
  auto keyPtr0 = CacheableKey::create(_keys[0]);
  auto keyPtr1 = CacheableKey::create(_keys[1]);
  auto keyPtr2 = CacheableKey::create("keyNotThere");

  std::vector<std::shared_ptr<CacheableKey>> keys1;
  keys1.push_back(keyPtr0);
  keys1.push_back(keyPtr1);
  keys1.push_back(keyPtr2);

  std::unordered_map<std::string, std::string> expected;
  expected[_keys[0]] = _values[startIndex + 0];
  expected[_keys[1]] = _values[startIndex + 1];

  auto valuesMap = region->getAll(keys1, callBack);
  if (valuesMap.size() == keys1.size()) {
    for (const auto& iter : valuesMap) {
      const auto key = std::dynamic_pointer_cast<CacheableKey>(iter.first);
      const auto& mVal = iter.second;
      if (mVal != nullptr) {
        LOG(std::string("Value from map ") + mVal->toString() +
            ", expected value " + expected[key->toString()]);
        ASSERT(mVal->toString() == expected[key->toString()],
               "value not matched");
      } else {
        ASSERT(key->toString() == "keyNotThere",
               "keyNotThere value is not null");
      }
    }
  }
}

void verifyGetAllWithCallBackArg(std::shared_ptr<Region> region,
                                 const char** values, int startIndex,
                                 std::shared_ptr<Cacheable> callBack) {
  verifyGetAll(region, values, startIndex, callBack);
}

void createPooledRegion(const std::string& name, bool ackMode,
                        const std::string& locators,
                        const std::string& poolname,
                        bool clientNotificationEnabled = false,
                        bool cachingEnable = true) {
  LOG("createRegion_Pool() entered.");
  std::cout << "Creating region --  " << name << " ackMode is " << ackMode
            << "\n"
            << std::flush;
  auto regPtr =
      getHelper()->createPooledRegion(name, ackMode, locators, poolname,
                                      cachingEnable, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Pooled Region created.");
}

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pooled_Locator)
  {
    // waitForDebugger();
    // start 1st client with caching enable true and client notification true
    initClientWithPool(true, "__TEST_POOL1__", locatorsG, nullptr, nullptr, 0,
                       true);
    createPooledRegion(_regionNames[0], USE_ACK, locatorsG, poolName, true,
                       true);
    LOG("StepOne_Pooled_Locator complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo_Pooled_Locator)
  {
    // start 1st client with caching enable true and client notification true
    initClientWithPool(true, "__TEST_POOL1__", locatorsG, nullptr, nullptr, 0,
                       true);
    createPooledRegion(_regionNames[0], USE_ACK, locatorsG, poolName, true,
                       true);
    LOG("StepTwo_Pooled_Locator complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, PutAllInitialValuesFromClientOne)
  {
    // putAll from client 1
    HashMapOfCacheable map0;
    map0.clear();
    for (int i = 0; i < 2; i++) {
      map0.emplace(CacheableKey::create(_keys[i]),
                   CacheableString::create(_vals[i]));
    }
    auto regPtr0 = getHelper()->getRegion(_regionNames[0]);
    regPtr0->putAll(map0);
    LOG("PutAllInitialValuesFromClientOne complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, GetAllInitialValuesFromClientTwo)
  {
    // getAll and validate key and value.
    auto region = getHelper()->getRegion(_regionNames[0]);
    verifyGetAll(region, _vals, 0);
    verifyGetAllWithCallBackArg(region, _vals, 0, CacheableInt32::create(1000));
    LOG("GetAllInitialValuesFromClientTwo complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT1, PutAllUpdatedValuesFromClientOne)
  {
    // update keys,values by putAll
    HashMapOfCacheable map0;
    map0.clear();
    for (int i = 0; i < 2; i++) {
      map0.emplace(CacheableKey::create(_keys[i]),
                   CacheableString::create(_nvals[i]));
    }
    auto regPtr0 = getHelper()->getRegion(_regionNames[0]);
    regPtr0->putAll(map0);
    LOG("PutAllUpdatedValuesFromClientOne complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT2, GetAllUpdatedValuesFromClientTwo)
  {
    // verify getAll get the data from local cache.
    auto region = getHelper()->getRegion(_regionNames[0]);
    verifyGetAll(region, _vals, 0);
    verifyGetAllWithCallBackArg(region, _vals, 0, CacheableInt32::create(1000));
    LOG("GetAllUpdatedValuesFromClientTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, GetAllAfterLocalDestroyRegionOnClientTwo)
  {
    // getAll and validate key and value after localDestroyRegion and recreation
    // of region.
    auto reg0 = getHelper()->getRegion(_regionNames[0]);
    reg0->localDestroyRegion();
    reg0 = nullptr;
    getHelper()->createPooledRegion(regionNames[0], USE_ACK, nullptr,
                                    "__TEST_POOL1__", true, true);
    reg0 = getHelper()->getRegion(_regionNames[0]);
    verifyGetAll(reg0, _nvals, 0);
    verifyGetAllWithCallBackArg(reg0, _nvals, 0, CacheableInt32::create(1000));
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, GetAllAfterLocalDestroyRegionOnClientTwo_Pool)
  {
    // getAll and validate key and value after localDestroyRegion and recreation
    // of region.
    auto reg0 = getHelper()->getRegion(_regionNames[0]);
    reg0->localDestroyRegion();
    reg0 = nullptr;
    createPooledRegion(_regionNames[0], USE_ACK, locatorsG, poolName, true,
                       true);
    reg0 = getHelper()->getRegion(_regionNames[0]);
    verifyGetAll(reg0, _nvals, 0);
    verifyGetAllWithCallBackArg(reg0, _nvals, 0, CacheableInt32::create(1000));
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, putallAndGetallPdxWithCallBackArg)
  {
    LOG("putallAndGetallPdxWithCallBackArg started.");
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes1::createDeserializable);
    } catch (const IllegalStateException&) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes2::createDeserializable);
    } catch (const IllegalStateException&) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes3::createDeserializable);
    } catch (const IllegalStateException&) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes4::createDeserializable);
    } catch (const IllegalStateException&) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes5::createDeserializable);
    } catch (const IllegalStateException&) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes6::createDeserializable);
    } catch (const IllegalStateException&) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes7::createDeserializable);
    } catch (const IllegalStateException&) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes8::createDeserializable);
    } catch (const IllegalStateException&) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes9::createDeserializable);
    } catch (const IllegalStateException&) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes10::createDeserializable);
    } catch (const IllegalStateException&) {
      // ignore exception
    }

    auto p1 = std::make_shared<PdxTypes1>();
    auto p2 = std::make_shared<PdxTypes2>();
    auto p3 = std::make_shared<PdxTypes3>();
    auto p4 = std::make_shared<PdxTypes4>();
    auto p5 = std::make_shared<PdxTypes5>();
    auto p6 = std::make_shared<PdxTypes6>();
    auto p7 = std::make_shared<PdxTypes7>();
    auto p8 = std::make_shared<PdxTypes8>();
    auto p9 = std::make_shared<PdxTypes9>();
    auto p10 = std::make_shared<PdxTypes10>();

    // putAll from client 1
    HashMapOfCacheable map0;
    map0.clear();

    map0.emplace(CacheableInt32::create(21), p1);
    map0.emplace(CacheableInt32::create(22), p2);
    map0.emplace(CacheableInt32::create(23), p3);
    map0.emplace(CacheableInt32::create(24), p4);
    map0.emplace(CacheableInt32::create(25), p5);
    map0.emplace(CacheableInt32::create(26), p6);
    map0.emplace(CacheableInt32::create(27), p7);
    map0.emplace(CacheableInt32::create(28), p8);
    map0.emplace(CacheableInt32::create(29), p9);
    map0.emplace(CacheableInt32::create(30), p10);

    auto regPtr0 = getHelper()->getRegion(_regionNames[0]);
    // TODO: Investigate whether callback is used
    regPtr0->putAll(map0, std::chrono::seconds(15),
                    CacheableInt32::create(1001));
    LOG("putallPdxWithCallBackArg on Pdx objects completed.");

    regPtr0->localDestroy(CacheableInt32::create(21));
    regPtr0->localDestroy(CacheableInt32::create(22));
    regPtr0->localDestroy(CacheableInt32::create(23));
    regPtr0->localDestroy(CacheableInt32::create(24));
    regPtr0->localDestroy(CacheableInt32::create(25));
    regPtr0->localDestroy(CacheableInt32::create(26));
    regPtr0->localDestroy(CacheableInt32::create(27));
    regPtr0->localDestroy(CacheableInt32::create(28));
    regPtr0->localDestroy(CacheableInt32::create(29));
    regPtr0->localDestroy(CacheableInt32::create(30));
    LOG("localDestroy on all Pdx objects completed.");

    std::vector<std::shared_ptr<CacheableKey>> keys1;
    keys1.push_back(CacheableInt32::create(21));
    keys1.push_back(CacheableInt32::create(22));
    keys1.push_back(CacheableInt32::create(23));
    keys1.push_back(CacheableInt32::create(24));
    keys1.push_back(CacheableInt32::create(25));
    keys1.push_back(CacheableInt32::create(26));
    keys1.push_back(CacheableInt32::create(27));
    keys1.push_back(CacheableInt32::create(28));
    keys1.push_back(CacheableInt32::create(29));
    keys1.push_back(CacheableInt32::create(30));
    auto valuesMap = regPtr0->getAll(keys1, CacheableInt32::create(1000));
    LOG("GetallPdxWithCallBackArg on Pdx objects completed.");

    ASSERT(valuesMap.size() == keys1.size(), "getAll size did not match");

    auto pRet10 = std::dynamic_pointer_cast<PdxTypes10>(
        valuesMap[CacheableInt32::create(30)]);
    ASSERT(p10->equals(pRet10) == true,
           "Objects of type PdxTypes10 should be equal");

    auto pRet9 = std::dynamic_pointer_cast<PdxTypes9>(
        valuesMap[CacheableInt32::create(29)]);
    ASSERT(p9->equals(pRet9) == true,
           "Objects of type PdxTypes9 should be equal");

    auto pRet8 = std::dynamic_pointer_cast<PdxTypes8>(
        valuesMap[CacheableInt32::create(28)]);
    ASSERT(p8->equals(pRet8) == true,
           "Objects of type PdxTypes8 should be equal");

    auto pRet7 = std::dynamic_pointer_cast<PdxTypes7>(
        valuesMap[CacheableInt32::create(27)]);
    ASSERT(p7->equals(pRet7) == true,
           "Objects of type PdxTypes7 should be equal");

    auto pRet6 = std::dynamic_pointer_cast<PdxTypes6>(
        valuesMap[CacheableInt32::create(26)]);
    ASSERT(p6->equals(pRet6) == true,
           "Objects of type PdxTypes6 should be equal");

    auto pRet5 = std::dynamic_pointer_cast<PdxTypes5>(
        valuesMap[CacheableInt32::create(25)]);
    ASSERT(p5->equals(pRet5) == true,
           "Objects of type PdxTypes5 should be equal");

    auto pRet4 = std::dynamic_pointer_cast<PdxTypes4>(
        valuesMap[CacheableInt32::create(24)]);
    ASSERT(p4->equals(pRet4) == true,
           "Objects of type PdxTypes4 should be equal");

    auto pRet3 = std::dynamic_pointer_cast<PdxTypes3>(
        valuesMap[CacheableInt32::create(23)]);
    ASSERT(p3->equals(pRet3) == true,
           "Objects of type PdxTypes3 should be equal");

    auto pRet2 = std::dynamic_pointer_cast<PdxTypes2>(
        valuesMap[CacheableInt32::create(22)]);
    ASSERT(p2->equals(pRet2) == true,
           "Objects of type PdxTypes2 should be equal");

    auto pRet1 = std::dynamic_pointer_cast<PdxTypes1>(
        valuesMap[CacheableInt32::create(21)]);
    ASSERT(p1->equals(pRet1) == true,
           "Objects of type PdxTypes1 should be equal");

    LOG("putallAndGetallPdxWithCallBackArg complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, putallAndGetallPdx)
  {
    LOG("putallAndGetallPdx started.");
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes1::createDeserializable);
    } catch (const IllegalStateException&) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes2::createDeserializable);
    } catch (const IllegalStateException&) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes3::createDeserializable);
    } catch (const IllegalStateException&) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes4::createDeserializable);
    } catch (const IllegalStateException&) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes5::createDeserializable);
    } catch (const IllegalStateException&) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes6::createDeserializable);
    } catch (const IllegalStateException&) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes7::createDeserializable);
    } catch (const IllegalStateException&) {
      // ignore exception
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes8::createDeserializable);
    } catch (const IllegalStateException&) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes9::createDeserializable);
    } catch (const IllegalStateException&) {
      // ignore exception
    }
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTypes10::createDeserializable);
    } catch (const IllegalStateException&) {
      // ignore exception
    }

    auto p1 = std::make_shared<PdxTypes1>();
    auto p2 = std::make_shared<PdxTypes2>();
    auto p3 = std::make_shared<PdxTypes3>();
    auto p4 = std::make_shared<PdxTypes4>();
    auto p5 = std::make_shared<PdxTypes5>();
    auto p6 = std::make_shared<PdxTypes6>();
    auto p7 = std::make_shared<PdxTypes7>();
    auto p8 = std::make_shared<PdxTypes8>();
    auto p9 = std::make_shared<PdxTypes9>();
    auto p10 = std::make_shared<PdxTypes10>();

    // putAll from client 1
    HashMapOfCacheable map0;
    map0.clear();

    map0.emplace(CacheableInt32::create(21), p1);
    map0.emplace(CacheableInt32::create(22), p2);
    map0.emplace(CacheableInt32::create(23), p3);
    map0.emplace(CacheableInt32::create(24), p4);
    map0.emplace(CacheableInt32::create(25), p5);
    map0.emplace(CacheableInt32::create(26), p6);
    map0.emplace(CacheableInt32::create(27), p7);
    map0.emplace(CacheableInt32::create(28), p8);
    map0.emplace(CacheableInt32::create(29), p9);
    map0.emplace(CacheableInt32::create(30), p10);
    auto regPtr0 = getHelper()->getRegion(_regionNames[0]);
    regPtr0->put(CacheableInt32::create(30), p10);
    regPtr0->putAll(map0);
    LOG("putAll on Pdx objects completed.");

    regPtr0->localDestroy(CacheableInt32::create(21));
    regPtr0->localDestroy(CacheableInt32::create(22));
    regPtr0->localDestroy(CacheableInt32::create(23));
    regPtr0->localDestroy(CacheableInt32::create(24));
    regPtr0->localDestroy(CacheableInt32::create(25));
    regPtr0->localDestroy(CacheableInt32::create(26));
    regPtr0->localDestroy(CacheableInt32::create(27));
    regPtr0->localDestroy(CacheableInt32::create(28));
    regPtr0->localDestroy(CacheableInt32::create(29));
    regPtr0->localDestroy(CacheableInt32::create(30));
    LOG("localDestroy on all Pdx objects completed.");

    std::vector<std::shared_ptr<CacheableKey>> keys1;
    keys1.push_back(CacheableInt32::create(21));
    keys1.push_back(CacheableInt32::create(22));
    keys1.push_back(CacheableInt32::create(23));
    keys1.push_back(CacheableInt32::create(24));
    keys1.push_back(CacheableInt32::create(25));
    keys1.push_back(CacheableInt32::create(26));
    keys1.push_back(CacheableInt32::create(27));
    keys1.push_back(CacheableInt32::create(28));
    keys1.push_back(CacheableInt32::create(29));
    keys1.push_back(CacheableInt32::create(30));
    auto valuesMap = regPtr0->getAll(keys1);
    LOG("getAll on Pdx objects completed.");

    ASSERT(valuesMap.size() == keys1.size(), "getAll size did not match");

    auto pRet10 = std::dynamic_pointer_cast<PdxTypes10>(
        valuesMap[CacheableInt32::create(30)]);
    ASSERT(p10->equals(pRet10) == true,
           "Objects of type PdxTypes10 should be equal");

    auto pRet9 = std::dynamic_pointer_cast<PdxTypes9>(
        valuesMap[CacheableInt32::create(29)]);
    ASSERT(p9->equals(pRet9) == true,
           "Objects of type PdxTypes9 should be equal");

    auto pRet8 = std::dynamic_pointer_cast<PdxTypes8>(
        valuesMap[CacheableInt32::create(28)]);
    ASSERT(p8->equals(pRet8) == true,
           "Objects of type PdxTypes8 should be equal");

    auto pRet7 = std::dynamic_pointer_cast<PdxTypes7>(
        valuesMap[CacheableInt32::create(27)]);
    ASSERT(p7->equals(pRet7) == true,
           "Objects of type PdxTypes7 should be equal");

    auto pRet6 = std::dynamic_pointer_cast<PdxTypes6>(
        valuesMap[CacheableInt32::create(26)]);
    ASSERT(p6->equals(pRet6) == true,
           "Objects of type PdxTypes6 should be equal");

    auto pRet5 = std::dynamic_pointer_cast<PdxTypes5>(
        valuesMap[CacheableInt32::create(25)]);
    ASSERT(p5->equals(pRet5) == true,
           "Objects of type PdxTypes5 should be equal");

    auto pRet4 = std::dynamic_pointer_cast<PdxTypes4>(
        valuesMap[CacheableInt32::create(24)]);
    ASSERT(p4->equals(pRet4) == true,
           "Objects of type PdxTypes4 should be equal");

    auto pRet3 = std::dynamic_pointer_cast<PdxTypes3>(
        valuesMap[CacheableInt32::create(23)]);
    ASSERT(p3->equals(pRet3) == true,
           "Objects of type PdxTypes3 should be equal");

    auto pRet2 = std::dynamic_pointer_cast<PdxTypes2>(
        valuesMap[CacheableInt32::create(22)]);
    ASSERT(p2->equals(pRet2) == true,
           "Objects of type PdxTypes2 should be equal");

    auto pRet1 = std::dynamic_pointer_cast<PdxTypes1>(
        valuesMap[CacheableInt32::create(21)]);
    ASSERT(p1->equals(pRet1) == true,
           "Objects of type PdxTypes1 should be equal");

    LOG("putallAndGetallPdx complete.");
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

void runPutGetAll() {
  CALL_TASK(CreateLocator1);
  CALL_TASK(CreateServer1_With_Locator_XML);

  CALL_TASK(StepOne_Pooled_Locator);
  CALL_TASK(StepTwo_Pooled_Locator);

  // CALL_TASK(PutAllInitialValuesFromClientOne);
  // CALL_TASK(GetAllInitialValuesFromClientTwo);
  // CALL_TASK(PutAllUpdatedValuesFromClientOne);
  // CALL_TASK(GetAllUpdatedValuesFromClientTwo);

  // CALL_TASK(GetAllAfterLocalDestroyRegionOnClientTwo_Pool);
  CALL_TASK(putallAndGetallPdx);

  // TODO: Does this task add value? Is it same code path as
  // non-WtihCallBackArg?
  //       This task has been found to intermittently error because types are
  //       still
  //       registered from  previous non-WithCallBackArg task. If this task has
  //       value
  //       then we should probably separate it into its own test.
  // CALL_TASK(putallAndGetallPdxWithCallBackArg);

  CALL_TASK(CloseCache1);

  CALL_TASK(CloseServer1);

  CALL_TASK(CloseLocator1);
}

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTPUTGETALL_H_
