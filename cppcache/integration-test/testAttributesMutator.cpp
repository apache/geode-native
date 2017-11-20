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

#define ROOT_NAME "testAttributesMutator"

#include "fw_dunit.hpp"
#include "CacheRegionHelper.hpp"
#include "CacheImpl.hpp"

// this is a test.

using namespace apache::geode::client;

class TestData {
 public:
  std::shared_ptr<Region> m_region;
  std::shared_ptr<Cache> m_cache;

} Test;

#define A s1p1

/* setup recipient */
DUNIT_TASK(A, Init)
  {
    auto cacheFactoryPtr = CacheFactory::createCacheFactory();
    Test.m_cache.reset(new Cache(cacheFactoryPtr->create()));

    AttributesFactory af;
    af.setEntryTimeToLive(ExpirationAction::LOCAL_INVALIDATE,
                          std::chrono::seconds(5));
    std::shared_ptr<RegionAttributes> attrs = af.createRegionAttributes();

    CacheImpl* cacheImpl = CacheRegionHelper::getCacheImpl(Test.m_cache.get());
    cacheImpl->createRegion("Local_ETTL_LI", attrs, Test.m_region);
  }
ENDTASK

DUNIT_TASK(A, CreateAndVerifyExpiry)
  {
    auto value = CacheableInt32::create(1);
    LOGDEBUG("### About to put of :one:1: ###");
    Test.m_region->put("one", value);
    LOGDEBUG("### Finished put of :one:1: ###");

    // countdown begins... it is ttl so access should not play into it..
    SLEEP(3000);  // sleep for a second, expect value to still be there.
    auto res =
        std::dynamic_pointer_cast<CacheableInt32>(Test.m_region->get("one"));
    ASSERT(res->value() == 1, "Expected to find value 1.");
    fflush(stdout);
    SLEEP(5000);  // sleep for 5 more seconds, expect value to be invalid.
    fflush(stdout);
    res = nullptr;
    ASSERT(Test.m_region->containsValueForKey("one") == false,
           "should not contain value.");
  }
ENDTASK

/* Test value sizes up to 1meg */
DUNIT_TASK(A, Close)
  {
    Test.m_region->destroyRegion();
    Test.m_cache->close();
    Test.m_cache = nullptr;
    Test.m_region = nullptr;
  }
ENDTASK
