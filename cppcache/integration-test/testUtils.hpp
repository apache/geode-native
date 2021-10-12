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

#ifndef GEODE_INTEGRATION_TEST_TESTUTILS_H_
#define GEODE_INTEGRATION_TEST_TESTUTILS_H_

#include <sstream>
#include <iomanip>

/* use CacheHelper to gain the impl pointer from cache or region object
 */

#include <CacheRegionHelper.hpp>
#include <geode/CacheableBuiltins.hpp>

#ifdef _WIN32
// ???
#pragma warning(disable : 4290)
// truncated debugging symbol to 255 characters
#pragma warning(disable : 4786)
// template instantiation must have dllinterface
#pragma warning(disable : 4251)
#endif

#include <RegionInternal.hpp>
#include <LocalRegion.hpp>
// #include <DistributedRegion.hpp>
#include <DistributedSystemImpl.hpp>
#include <CacheImpl.hpp>

namespace {  // NOLINT(google-build-namespaces)

using apache::geode::client::Cache;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheableVector;
using apache::geode::client::CacheImpl;
using apache::geode::client::CacheRegionHelper;
using apache::geode::client::Region;
using apache::geode::client::RegionInternal;

namespace unitTests {

class TestUtils {
 public:
  static RegionInternal* getRegionInternal(std::shared_ptr<Region>& rptr) {
    return dynamic_cast<RegionInternal*>(rptr.get());
  }

  static CacheImpl* getCacheImpl(const std::shared_ptr<Cache>& cptr) {
    return CacheRegionHelper::getCacheImpl(cptr.get());
  }

  static size_t testNumberOfPreservedData(const CacheImpl& cacheImpl) {
    return cacheImpl.getPdxTypeRegistry()->testNumberOfPreservedData();
  }

  static bool waitForKey(std::shared_ptr<CacheableKey>& keyPtr,
                         std::shared_ptr<Region>& rptr, int maxTry,
                         uint32_t msleepTime) {
    int tries = 0;
    bool found = false;
    while ((tries < maxTry) && (!(found = rptr->containsKey(keyPtr)))) {
      SLEEP(msleepTime);
      tries++;
    }
    return found;
  }
  static bool waitForValueForKey(std::shared_ptr<CacheableKey>& keyPtr,
                                 std::shared_ptr<Region>& rptr, int maxTry,
                                 uint32_t msleepTime) {
    int tries = 0;
    bool found = false;
    while ((tries < maxTry) && (!(found = rptr->containsValueForKey(keyPtr)))) {
      SLEEP(msleepTime);
      tries++;
    }
    return found;
  }
  static bool waitForValueForKeyGoAway(std::shared_ptr<CacheableKey>& keyPtr,
                                       std::shared_ptr<Region>& rptr,
                                       int maxTry, uint32_t msleepTime) {
    int tries = 0;
    bool found = true;
    while ((tries < maxTry) && (found = rptr->containsValueForKey(keyPtr))) {
      SLEEP(msleepTime);
      tries++;
    }
    return found;
  }
  static bool waitForValueNotNULL(std::shared_ptr<CacheableString>& valPtr,
                                  int maxTry, uint32_t msleepTime) {
    int tries = 0;
    bool found = false;
    // @TODO: ? How will valPtr every point to something else in this loop?
    while ((found = (valPtr == nullptr)) && (tries < maxTry)) {
      SLEEP(msleepTime);
      tries++;
    }
    return !found;
  }

  static int waitForValue(std::shared_ptr<CacheableKey>& keyPtr, int expected,
                          std::shared_ptr<CacheableString>& valPtr,
                          std::shared_ptr<Region>& rptr, int maxTry,
                          uint32_t msleepTime) {
    int tries = 0;
    int val = 0;
    do {
      valPtr = std::dynamic_pointer_cast<CacheableString>(rptr->get(keyPtr));
      ASSERT(valPtr != nullptr, "value should not be null.");
      val = atoi(valPtr->value().c_str());
      SLEEP(msleepTime);
      tries++;
    } while ((val != expected) && (tries < maxTry));
    return val;
  }

  static void showKeys(std::shared_ptr<Region>& rptr) {
    if (!rptr) {
      LOG("this region does not exist!\n");
    }
    std::vector<std::shared_ptr<CacheableKey>> v = rptr->keys();
    auto len = v.size();
    LOG(std::string("Total keys in region ") + rptr->getName() + " : " +
        std::to_string(len));
    for (uint32_t i = 0; i < len; i++) {
      LOG(std::string("key[") + std::to_string(i) + "] = '" +
          (v[i] == nullptr ? "nullptr KEY" : v[i]->toString()) + "'");
    }
  }

  static void showKeyValues(std::shared_ptr<Region>& rptr) {
    if (!rptr) {
      LOG("this region does not exist!\n");
    }
    std::vector<std::shared_ptr<CacheableKey>> v = rptr->keys();
    auto len = v.size();
    LOG(std::string("Total keys in region ") + rptr->getName() + " : " +
        std::to_string(len));
    for (uint32_t i = 0; i < len; i++) {
      std::stringstream strm;
      auto keyPtr = v[i];
      auto valPtr =
          std::dynamic_pointer_cast<CacheableString>(rptr->get(keyPtr));
      strm << "key[" << i << "]='"
           << (keyPtr == nullptr ? "nullptr KEY" : keyPtr->toString())
           << "', value[" << i << "]='"
           << (valPtr == nullptr ? "NULL_VALUE" : valPtr->value()) << "'";
      LOG(strm.str());
    }
  }
  static void showValues(std::shared_ptr<Region>& rptr) {
    if (!rptr) {
      LOG("this region does not exist!\n");
    }
    auto v = rptr->values();
    auto len = v.size();
    LOG(std::string("Total values in region ") + rptr->getName() + " : " +
        std::to_string(len));
    for (size_t i = 0; i < len; i++) {
      auto value = std::dynamic_pointer_cast<CacheableString>(v[i]);
      LOG(std::string("value[") + std::to_string(i) + "] = '" +
          (v[i] == nullptr ? "nullptr VALUE" : value->value()) + "'");
    }
  }

  static std::string zeroPaddedStringFromInt(int32_t number, uint16_t width) {
    std::ostringstream strm;
    strm << std::setw(width) << std::setfill('0') << number;
    return strm.str();
  }

  static void verifyGetResults(const CacheableVector* resultList, int index) {
    auto expected = std::string("VALUE--") + std::to_string((index * 2) + 1);
    auto actual = std::dynamic_pointer_cast<CacheableString>(
                      resultList->operator[](index))
                      ->value();
    auto msg = std::string("Failed to find the value 'VALUE--") +
               +"' in the result list.";
    ASSERT(expected == actual, msg);
  }
};
}  // namespace unitTests

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_TESTUTILS_H_
