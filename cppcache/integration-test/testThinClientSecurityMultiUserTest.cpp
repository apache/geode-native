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

#include <geode/AuthInitialize.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "fw_dunit.hpp"
#include "ThinClientSecurity.hpp"
#include "ThinClientHelper.hpp"

using apache::geode::client::CacheFactory;
using apache::geode::client::RegionShortcut;

#define CLIENT1 s1p1
#define LOCATORSERVER s2p2

DUNIT_TASK_DEFINITION(LOCATORSERVER, CreateLocator)
  {
    CacheHelper::initLocator(1, false, false, -1, 0, false, true);
    LOG("Locator started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(LOCATORSERVER, CreateServer)
  {
    CacheHelper::initServer(
        1, "cacheserver_notify_subscription2.xml",
        CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1),
        "--J=-Dsecurity-manager=javaobject.SimpleSecurityManager", false, true,
        false, false, false, true);
    LOG("Server started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, PerformSecureOperationsWithUserCredentials)
  {
    auto cache = CacheFactory().create();
    auto poolFactory = cache.getPoolManager().createFactory();
    poolFactory.setMultiuserAuthentication(true);
    poolFactory.addLocator("localhost", CacheHelper::staticLocatorHostPort1);
    poolFactory.create("mypool");

    auto regionFactory = cache.createRegionFactory(RegionShortcut::PROXY);
    regionFactory.setPoolName("mypool");
    regionFactory.create("DistRegionAck");

    auto config1 = Properties::create();
    config1->insert("security-username", "root");
    config1->insert("security-password", "root-password");

    cache.createAuthenticatedView(config1, "mypool")
        .getRegion("DistRegionAck")
        ->put("akey", "avalue");

    auto config2 = Properties::create();
    config2->insert("security-username", "reader");
    config2->insert("security-password", "reader-password");

    try {
      cache.createAuthenticatedView(config2, "mypool")
          .getRegion("DistRegionAck")
          ->put("akey", "avalue");
      FAIL("Didn't throw expected AuthenticationFailedException.");
    } catch (const apache::geode::client::NotAuthorizedException &) {
      LOG("Caught expected AuthenticationFailedException.");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(LOCATORSERVER, CloseServer)
  {
    CacheHelper::closeServer(1);
    LOG("SERVER1 stopped");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(LOCATORSERVER, CloseLocator)
  {
    CacheHelper::closeLocator(1);
    LOG("Locator1 stopped");
  }
END_TASK_DEFINITION

DUNIT_MAIN
  {
    CALL_TASK(CreateLocator);
    CALL_TASK(CreateServer);
    CALL_TASK(PerformSecureOperationsWithUserCredentials);
    CALL_TASK(CloseServer);
    CALL_TASK(CloseLocator);
  }
END_MAIN
