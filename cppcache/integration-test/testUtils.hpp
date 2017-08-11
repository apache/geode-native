#pragma once

#ifndef GEODE_INTEGRATION_TEST_TESTUTILS_H_
#define GEODE_INTEGRATION_TEST_TESTUTILS_H_

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
#include <geode/GeodeCppCache.hpp>

/* use CacheHelper to gain the impl pointer from cache or region object
 */

#include <CacheRegionHelper.hpp>

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

using namespace apache::geode::client;

namespace unitTests {

class TestUtils {
 public:
  static RegionInternal* getRegionInternal(RegionPtr& rptr) {
    return dynamic_cast<RegionInternal*>(rptr.get());
  }

  static CacheImpl* getCacheImpl(const CachePtr& cptr) {
    return CacheRegionHelper::getCacheImpl(cptr.get());
  }

  static size_t testNumberOfPreservedData(const CacheImpl& cacheImpl) {
    return cacheImpl.getPdxTypeRegistry()->testNumberOfPreservedData();
  }

  static bool waitForKey(CacheableKeyPtr& keyPtr, RegionPtr& rptr, int maxTry,
                         uint32_t msleepTime) {
    int tries = 0;
    bool found = false;
    while ((tries < maxTry) && (!(found = rptr->containsKey(keyPtr)))) {
      SLEEP(msleepTime);
      tries++;
    }
    return found;
  }
  static bool waitForValueForKey(CacheableKeyPtr& keyPtr, RegionPtr& rptr,
                                 int maxTry, uint32_t msleepTime) {
    int tries = 0;
    bool found = false;
    while ((tries < maxTry) && (!(found = rptr->containsValueForKey(keyPtr)))) {
      SLEEP(msleepTime);
      tries++;
    }
    return found;
  }
  static bool waitForValueForKeyGoAway(CacheableKeyPtr& keyPtr, RegionPtr& rptr,
                                       int maxTry, uint32_t msleepTime) {
    int tries = 0;
    bool found = true;
    while ((tries < maxTry) && (found = rptr->containsValueForKey(keyPtr))) {
      SLEEP(msleepTime);
      tries++;
    }
    return found;
  }
  static bool waitForValueNotNULL(CacheableStringPtr& valPtr, int maxTry,
                                  uint32_t msleepTime) {
    int tries = 0;
    bool found = false;
    // @TODO: ? How will valPtr every point to something else in this loop?
    while ((found = (valPtr == nullptr)) && (tries < maxTry)) {
      SLEEP(msleepTime);
      tries++;
    }
    return !found;
  }

  static int waitForValue(CacheableKeyPtr& keyPtr, int expected,
                          CacheableStringPtr& valPtr, RegionPtr& rptr,
                          int maxTry, uint32_t msleepTime) {
    int tries = 0;
    int val = 0;
    do {
      valPtr = std::dynamic_pointer_cast<CacheableString>(rptr->get(keyPtr));
      ASSERT(valPtr != nullptr, "value should not be null.");
      val = atoi(valPtr->asChar());
      SLEEP(msleepTime);
      tries++;
    } while ((val != expected) && (tries < maxTry));
    return val;
  }
  static void showKeys(RegionPtr& rptr) {
    char buf[2048];
    if (rptr == nullptr) {
      sprintf(buf, "this region does not exist!\n");
      LOG(buf);
      return;
    }
    VectorOfCacheableKey v;
    rptr->keys(v);
    auto len = v.size();
    sprintf(buf, "Total keys in region %s : %zu\n", rptr->getName(), len);
    LOG(buf);
    for (uint32_t i = 0; i < len; i++) {
      char keyText[100];
      v[i]->logString(keyText, 100);
      sprintf(buf, "key[%u] = '%s'\n", i,
              (v[i] == nullptr) ? "nullptr KEY" : keyText);
      LOG(buf);
    }
  }
  static void showKeyValues(RegionPtr& rptr) {
    char buf[2048];
    if (rptr == nullptr) {
      sprintf(buf, "this region does not exist!\n");
      LOG(buf);
      return;
    }
    VectorOfCacheableKey v;
    rptr->keys(v);
    auto len = v.size();
    sprintf(buf, "Total keys in region %s : %zu\n", rptr->getName(), len);
    LOG(buf);
    for (uint32_t i = 0; i < len; i++) {
      CacheableKeyPtr keyPtr = v[i];
      char keyText[100];
      keyPtr->logString(keyText, 100);
      auto valPtr =
          std::dynamic_pointer_cast<CacheableString>(rptr->get(keyPtr));
      sprintf(buf, "key[%u] = '%s', value[%u]='%s'\n", i,
              (keyPtr == nullptr) ? "nullptr KEY" : keyText, i,
              (valPtr == nullptr) ? "NULL_VALUE" : valPtr->asChar());
      LOG(buf);
    }
  }
  static void showValues(RegionPtr& rptr) {
    char buf[2048];
    if (rptr == nullptr) {
      sprintf(buf, "this region does not exist!\n");
      LOG(buf);
      return;
    }
    VectorOfCacheable v;
    rptr->values(v);
    auto len = v.size();
    sprintf(buf, "Total values in region %s : %zu\n", rptr->getName(), len);
    LOG(buf);
    for (size_t i = 0; i < len; i++) {
      auto value = std::dynamic_pointer_cast<CacheableString>(v[i]);
      sprintf(buf, "value[%zu] = '%s'\n", i,
              (value == nullptr) ? "nullptr VALUE" : value->asChar());
      LOG(buf);
    }
  }
};
}  // namespace unitTests

#endif  // GEODE_INTEGRATION_TEST_TESTUTILS_H_
