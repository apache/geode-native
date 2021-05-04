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
#include <iostream>
#include <vector>

#include "fw_dunit.hpp"

#include <geode/internal/chrono/duration.hpp>

#define CLIENT1 s1p1
#define SERVER1 s2p1
#define SERVER2 s2p2

#include "CacheHelper.hpp"

using apache::geode::client::Cache;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheHelper;
using apache::geode::client::Exception;

static bool isLocalServer = false;
static bool isLocator = false;
static int numberOfLocators = 1;
const char *endPoints = nullptr;
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);

#include "LocatorHelper.hpp"

int testXmlDeclarativeCacheCreation() {
  auto cacheFactory = CacheFactory();
  std::shared_ptr<Cache> cptr;

  std::string directory(std::getenv("TESTSRC"));

  std::cout
      << "create DistributedSytem with name=XML_DECLARATIVE_CACHE_CREATION_TEST"
      << std::endl;

  try {
    const auto filePath =
        directory + "/resources/valid_declarative_cache_creation.xml";
    cptr = std::make_shared<Cache>(
        cacheFactory.set("cache-xml-file", filePath).create());

  } catch (Exception &ex) {
    std::cout << "Exception: msg = " << ex.what() << std::endl;
    LOG(ex.getStackTrace());
    return -1;
  } catch (...) {
    LOG_INFO("unknown exception");
    return -1;
  }

  std::cout << "Test if number of root regions are correct" << std::endl;
  auto vrp = cptr->rootRegions();
  std::cout << "  vrp.size=" << vrp.size() << std::endl;

  if (vrp.size() != 1) {
    std::cout << "Number of root regions does not match" << std::endl;
    return -1;
  }

  std::cout << "Root regions in Cache :" << std::endl;
  for (size_t i = 0; i < vrp.size(); i++) {
    std::cout << "vc[" << i << "].m_reaPtr=" << vrp.at(i).get() << std::endl;
    std::cout << "vc[" << i << "]=" << vrp.at(i)->getName() << std::endl;
  }
  auto regPtr1 = vrp.at(0);

  auto regionAttributes = regPtr1->getAttributes();
  std::cout << "Test Attributes of root region Root1 " << std::endl;
  std::cout << "Region name " << regPtr1->getName() << std::endl;

  if (regionAttributes.getCacheLoader() == nullptr) {
    std::cout << "Cache Loader not initialized." << std::endl;
    return -1;
  }

  if (regionAttributes.getCacheListener() == nullptr) {
    std::cout << "Cache Listener not initialized." << std::endl;
    return -1;
  }

  if (regionAttributes.getCacheWriter() == nullptr) {
    std::cout << "Cache Writer not initialized." << std::endl;
    return -1;
  }

  std::cout << "Attributes of Root1 are correctly set" << std::endl;

  if (!cptr->isClosed()) {
    cptr->close();
    cptr = nullptr;
  }

  return 0;
}

int testSetCacheXmlThenGetRegion() {
  auto cacheFactory = CacheFactory();
  std::shared_ptr<Cache> cptr;

  std::cout
      << "Create cache with the configurations provided in valid_cache_pool.xml"
      << std::endl;

  try {
    auto duplicateFile =
        CacheHelper::createDuplicateXMLFile("valid_cache_pool.xml");

    cptr = std::make_shared<Cache>(
        cacheFactory.set("enable-time-statistics", "false")
            .set("statistic-sampling-enabled", "false")
            .set("cache-xml-file", duplicateFile)
            .create());

  } catch (Exception &ex) {
    std::cout << "Exception: msg = " << ex.what() << std::endl;
    LOG(ex.getStackTrace());
    return -1;
  } catch (...) {
    LOG_INFO("unknown exception");
    return -1;
  }

  auto region = cptr->getRegion("Root1");

  if (region == nullptr || region->getName() != "Root1") {
    return -1;
  }

  return 0;
}

DUNIT_TASK_DEFINITION(CLIENT1, ValidXmlTestDeclarativeCacheCreation)
  {
    int res = testXmlDeclarativeCacheCreation();

    if (res != 0) {
      FAIL("DeclarativeCacheCreation Test Failed.");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, SetCacheXmlThenGetRegion)
  {
    // Reusing server setup from ValidXmlTestPools for simplicity
    CacheHelper::initLocator(1);
    char tmp[128];
    sprintf(tmp, "localhost:%d", CacheHelper::staticLocatorHostPort1);
    CacheHelper::initServer(1, "cacheserver1_pool.xml", tmp);
    CacheHelper::initServer(2, "cacheserver2_pool.xml", tmp);

    int res = testSetCacheXmlThenGetRegion();

    CacheHelper::closeServer(1);
    CacheHelper::closeServer(2);

    CacheHelper::closeLocator(1);

    if (res != 0) {
      FAIL("SetCacheXmlThenGetRegion Test Failed.");
    }
  }
END_TASK_DEFINITION

DUNIT_MAIN
  {
    CALL_TASK(ValidXmlTestDeclarativeCacheCreation);
    CALL_TASK(SetCacheXmlThenGetRegion);
  }
END_MAIN
