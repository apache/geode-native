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

#include <geode/CacheFactory.hpp>
#include <geode/Region.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include <CacheRegionHelper.hpp>

#include "fw_helper.hpp"

using apache::geode::client::Cache;
using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheFactory;
using apache::geode::client::IllegalArgumentException;
using apache::geode::client::RegionShortcut;

/* testing attributes with invalid value */
/* testing with negative values */          /*see bug no #865 */
/* testing with exceed boundry condition */ /*see bug no #865 */
BEGIN_TEST(REGION_FACTORY)
  {
    auto cf = CacheFactory();
    auto cache = std::make_shared<Cache>(cf.create());

    auto rf = cache->createRegionFactory(RegionShortcut::LOCAL);
    /*see bug no #865 */
    try {
      rf.setInitialCapacity(-1);
      FAIL("Should have got expected IllegalArgumentException");
    } catch (IllegalArgumentException &) {
      LOG("Got expected IllegalArgumentException");
    }

    auto region = rf.create("Local_ETTL_LI");
    LOG_INFO("region->getAttributes().getInitialCapacity() = %d ",
             region->getAttributes().getInitialCapacity());
    ASSERT(region->getAttributes().getInitialCapacity() == 10000,
           "Incorrect InitialCapacity");

    region->put(1, 1);
    auto res = std::dynamic_pointer_cast<CacheableInt32>(region->get(1));
    ASSERT(res->value() == 1, "Expected to find value 1.");

    region->destroyRegion();
    cache->close();
    cache = nullptr;
    region = nullptr;

    auto cf1 = CacheFactory();
    auto cache1 = std::make_shared<Cache>(cf1.create());

    auto rf1 = cache1->createRegionFactory(RegionShortcut::LOCAL);
    /*see bug no #865 */
    try {
      rf1.setInitialCapacity(2147483648U);
      FAIL("Should have got expected IllegalArgumentException");
    } catch (IllegalArgumentException &) {
      LOG("Got expected IllegalArgumentException");
    }
    auto region1 = rf1.create("Local_ETTL_LI");
    LOG_INFO("region1->getAttributes().getInitialCapacity() = %d ",
             region1->getAttributes().getInitialCapacity());
    ASSERT(region1->getAttributes().getInitialCapacity() == 10000,
           "Incorrect InitialCapacity");

    region1->put(1, 1);
    auto res1 = std::dynamic_pointer_cast<CacheableInt32>(region1->get(1));
    ASSERT(res1->value() == 1, "Expected to find value 1.");

    region1->destroyRegion();
    cache1->close();
    cache1 = nullptr;
    region1 = nullptr;
  }
END_TEST(REGION_FACTORY)
