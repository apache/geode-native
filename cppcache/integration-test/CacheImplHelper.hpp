#pragma once

#ifndef GEODE_INTEGRATION_TEST_CACHEIMPLHELPER_H_
#define GEODE_INTEGRATION_TEST_CACHEIMPLHELPER_H_

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

#include <cstdlib>
#include <geode/SystemProperties.hpp>
#include "testUtils.hpp"

#ifndef ROOT_NAME
#define ROOT_NAME "DEFINE ROOT_NAME before including CacheHelper.hpp"
#endif

#ifndef ROOT_SCOPE
#define ROOT_SCOPE LOCAL
#endif

namespace {  // NOLINT(google-build-namespaces)

using apache::geode::client::CacheHelper;
using apache::geode::client::Properties;
using apache::geode::client::RegionAttributesFactory;

using unitTests::TestUtils;

class CacheImplHelper : public CacheHelper {
 public:
  explicit CacheImplHelper(
      const char* member_id,
      const std::shared_ptr<Properties>& configPtr = nullptr)
      : CacheHelper(member_id, configPtr) {}

  explicit CacheImplHelper(
      const std::shared_ptr<Properties>& configPtr = nullptr)
      : CacheHelper(configPtr) {}

  virtual void createRegion(const char* regionName,
                            std::shared_ptr<Region>& regionPtr, uint32_t size,
                            bool cacheEnabled = true) {
    RegionAttributesFactory regionAttributesFactory;
    // set lru attributes...
    regionAttributesFactory.setLruEntriesLimit(0);     // no limit.
    regionAttributesFactory.setInitialCapacity(size);  // no limit.
    // then...
    regionAttributesFactory.setCachingEnabled(cacheEnabled);
    auto regionAttributes = regionAttributesFactory.create();
    showRegionAttributes(regionAttributes);
    CacheImpl* cimpl = TestUtils::getCacheImpl(cachePtr);
    ASSERT(cimpl != nullptr, "failed to get cacheImpl *.");
    cimpl->createRegion(regionName, regionAttributes, regionPtr);
    ASSERT(regionPtr != nullptr, "failed to create region.");
  }
};
}  // namespace

#endif  // GEODE_INTEGRATION_TEST_CACHEIMPLHELPER_H_
