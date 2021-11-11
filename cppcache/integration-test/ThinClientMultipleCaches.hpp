#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTMULTIPLECACHES_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTMULTIPLECACHES_H_

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

#include <string>

#include <geode/CacheFactory.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "CacheHelper.hpp"
#include "fw_dunit.hpp"

#define CLIENT1 s1p1
#define SERVER1 s2p1

namespace {  // NOLINT(google-build-namespaces)

using apache::geode::client::Cache;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheHelper;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

static bool isLocalServer = false;
static bool isLocator = false;

const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer);

#include "LocatorHelper.hpp"

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    CacheHelper::initServer(1, "cacheserver_notify_subscription.xml");
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION
std::shared_ptr<Region> createRegionFromCache(std::shared_ptr<Cache>& cache) {
  auto poolFactory = cache->getPoolManager().createFactory();
  CacheHelper::getHelper().addServerLocatorEPs(locatorsG, poolFactory, true);
  poolFactory.create("DistRegionAck");
  return cache->createRegionFactory(RegionShortcut::PROXY)
      .create("DistRegionAck");
}

DUNIT_TASK_DEFINITION(CLIENT1, SetupAndTestMutlipleCaches)
  {
    auto factory = CacheFactory();
    auto cache1 = std::make_shared<Cache>(factory.create());
    auto region1 = createRegionFromCache(cache1);

    auto cache2 = std::make_shared<Cache>(factory.create());
    auto region2 = createRegionFromCache(cache2);

    region1->put("a", "key");

    LOG("Closing first cache.");
    cache1->close();

    ASSERT(cache1->isClosed(), "Cache 1 is not closed.");
    ASSERT(!cache2->isClosed(), "Cache 2 is closed.");

    LOG("Doing get() on second cache.");
    auto value = region2->get("a")->toString();

    ASSERT(value == "key",
           std::string("Expected value 'key' didn't equal actual value '")
               .append(value)
               .append("'")
               .c_str());
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer)
  {
    CacheHelper::closeServer(1);
    LOG("SERVER1 stopped");
  }
END_TASK_DEFINITION

void run() {
  CALL_TASK(CreateLocator1);
  CALL_TASK(CreateServer1_With_Locator_XML);

  CALL_TASK(SetupAndTestMutlipleCaches);

  CALL_TASK(CloseServer);
  CALL_TASK(CloseLocator1);
}

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTMULTIPLECACHES_H_
