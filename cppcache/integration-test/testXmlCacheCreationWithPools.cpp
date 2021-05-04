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

#include <cinttypes>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include <geode/internal/chrono/duration.hpp>

#include "fw_dunit.hpp"

#define CLIENT1 s1p1

#include "CacheHelper.hpp"

using apache::geode::client::Cache;
using apache::geode::client::CacheableStringArray;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheHelper;
using apache::geode::client::Exception;
using apache::geode::client::Pool;

static bool isLocalServer = false;
static bool isLocator = false;
static int numberOfLocators = 1;
const char *endPoints = nullptr;
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);

using std::string;
using std::vector;

using SLIST = vector<string>;

bool findString(string &item, std::shared_ptr<CacheableStringArray> array) {
  for (int size = 0; size < array->length(); size++) {
    if (strcmp(item.c_str(), array->operator[](size)->value().c_str()) == 0) {
      return true;
    }
  }

  return false;
}

bool checkStringArray(SLIST &first,
                      std::shared_ptr<CacheableStringArray> second) {
  if (second == nullptr && first.size() > 0) return false;

  if (second == nullptr && first.size() == 0) return true;

  if (first.size() != static_cast<size_t>(second->length())) return false;

  for (size_t size = 0; size < first.size(); size++) {
    if (!findString(first[size], second)) {
      return false;
    }
  }

  return true;
}

bool checkPoolAttribs(std::shared_ptr<Pool> pool, SLIST &locators,
                      SLIST &servers, int freeConnectionTimeout,
                      int loadConditioningInterval, int minConnections,
                      int maxConnections, int retryAttempts,
                      std::chrono::milliseconds idleTimeout, int pingInterval,
                      const std::string &name, int readTimeout,
                      const std::string &serverGroup, int socketBufferSize,
                      bool subscriptionEnabled,
                      int subscriptionMessageTrackingTimeout,
                      int subscriptionAckInterval, int subscriptionRedundancy,
                      int statisticInterval, bool prSingleHopEnabled,
                      int updateLocatorListInterval) {
  using apache::geode::internal::chrono::duration::to_string;

  char logmsg[500] = {0};

  if (pool == nullptr) {
    LOG("checkPoolAttribs: std::shared_ptr<Pool> is nullptr");
    return false;
  }

  std::cout << "Checking pool " << pool->getName() << std::endl;

  if (pool->getName() != name) {
    sprintf(logmsg, "checkPoolAttribs: Pool name expected [%s], actual [%s]",
            name.c_str(), pool->getName().c_str());
    LOG(logmsg);
    return false;
  }
  if (!checkStringArray(locators, pool->getLocators())) {
    LOG("checkPoolAttribs: locators mismatch");
    return false;
  }
  if (servers.size() > 0 && !checkStringArray(servers, pool->getServers())) {
    LOG("checkPoolAttribs: servers mismatch");
    return false;
  }
  if (std::chrono::milliseconds(freeConnectionTimeout) !=
      pool->getFreeConnectionTimeout()) {
    sprintf(logmsg,
            "checkPoolAttribs: Pool freeConnectionTimeout expected [%d], "
            "actual [%" PRId64 "]",
            freeConnectionTimeout,
            static_cast<int64_t>(pool->getFreeConnectionTimeout().count()));
    LOG(logmsg);
    return false;
  }
  if (std::chrono::milliseconds(loadConditioningInterval) !=
      pool->getLoadConditioningInterval()) {
    sprintf(logmsg,
            "checkPoolAttribs: Pool loadConditioningInterval expected [%d], "
            "actual [%" PRId64 "]",
            loadConditioningInterval,
            static_cast<int64_t>(pool->getLoadConditioningInterval().count()));
    LOG(logmsg);
    return false;
  }
  if (minConnections != pool->getMinConnections()) {
    sprintf(logmsg,
            "checkPoolAttribs: Pool minConnections expected [%d], actual [%d]",
            minConnections, pool->getMinConnections());
    LOG(logmsg);
    return false;
  }
  if (maxConnections != pool->getMaxConnections()) {
    sprintf(logmsg,
            "checkPoolAttribs: Pool maxConnections expected [%d], actual [%d]",
            maxConnections, pool->getMaxConnections());
    LOG(logmsg);
    return false;
  }
  if (retryAttempts != pool->getRetryAttempts()) {
    sprintf(logmsg,
            "checkPoolAttribs: Pool retryAttempts expected [%d], actual [%d]",
            retryAttempts, pool->getRetryAttempts());
    LOG(logmsg);
    return false;
  }
  if (idleTimeout != pool->getIdleTimeout()) {
    sprintf(logmsg,
            "checkPoolAttribs: Pool idleTimeout expected [%s], actual [%s]",
            to_string(idleTimeout).c_str(),
            to_string(pool->getIdleTimeout()).c_str());
    LOG(logmsg);
    return false;
  }
  if (std::chrono::milliseconds(pingInterval) != pool->getPingInterval()) {
    sprintf(
        logmsg,
        "checkPoolAttribs: Pool pingInterval expected [%d], actual [%" PRId64
        "]",
        pingInterval, static_cast<int64_t>(pool->getPingInterval().count()));
    LOG(logmsg);
    return false;
  }
  if (std::chrono::milliseconds(readTimeout) != pool->getReadTimeout()) {
    sprintf(logmsg,
            "checkPoolAttribs: Pool readTimeout expected [%d], actual [%" PRId64
            "]",
            readTimeout, static_cast<int64_t>(pool->getReadTimeout().count()));
    LOG(logmsg);
    return false;
  }
  if (serverGroup != pool->getServerGroup()) {
    sprintf(logmsg,
            "checkPoolAttribs: Pool serverGroup expected [%s], actual [%s]",
            serverGroup.c_str(), pool->getServerGroup().c_str());
    LOG(logmsg);
    return false;
  }
  if (socketBufferSize != pool->getSocketBufferSize()) {
    sprintf(
        logmsg,
        "checkPoolAttribs: Pool socketBufferSize expected [%d], actual [%d]",
        socketBufferSize, pool->getSocketBufferSize());
    LOG(logmsg);
    return false;
  }
  if (subscriptionEnabled != pool->getSubscriptionEnabled()) {
    sprintf(
        logmsg,
        "checkPoolAttribs: Pool subscriptionEnabled expected [%s], actual [%s]",
        subscriptionEnabled ? "true" : "false",
        pool->getSubscriptionEnabled() ? "true" : "false");
    LOG(logmsg);
    return false;
  }
  if (std::chrono::milliseconds(subscriptionMessageTrackingTimeout) !=
      pool->getSubscriptionMessageTrackingTimeout()) {
    sprintf(logmsg,
            "checkPoolAttribs: Pool subscriptionMessageTrackingTimeout "
            "expected [%d], actual [%" PRId64 "]",
            subscriptionMessageTrackingTimeout,
            static_cast<int64_t>(
                pool->getSubscriptionMessageTrackingTimeout().count()));
    LOG(logmsg);
    return false;
  }
  if (std::chrono::milliseconds(subscriptionAckInterval) !=
      pool->getSubscriptionAckInterval()) {
    sprintf(logmsg,
            "checkPoolAttribs: Pool subscriptionAckInterval expected [%d], "
            "actual [%" PRId64 "]",
            subscriptionAckInterval,
            static_cast<int64_t>(pool->getSubscriptionAckInterval().count()));
    LOG(logmsg);
    return false;
  }
  if (subscriptionRedundancy != pool->getSubscriptionRedundancy()) {
    sprintf(logmsg,
            "checkPoolAttribs: Pool subscriptionRedundancy expected [%d], "
            "actual [%d]",
            subscriptionRedundancy, pool->getSubscriptionRedundancy());
    LOG(logmsg);
    return false;
  }
  if (std::chrono::milliseconds(statisticInterval) !=
      pool->getStatisticInterval()) {
    sprintf(logmsg,
            "checkPoolAttribs: Pool statisticInterval expected [%d], actual "
            "[%" PRId64 "]",
            statisticInterval,
            static_cast<int64_t>(pool->getStatisticInterval().count()));
    LOG(logmsg);
    return false;
  }
  if (prSingleHopEnabled != pool->getPRSingleHopEnabled()) {
    sprintf(
        logmsg,
        "checkPoolAttribs: Pool prSingleHopEnabled expected [%d], actual [%d]",
        prSingleHopEnabled, pool->getPRSingleHopEnabled());
    LOG(logmsg);
    return false;
  }
  if (std::chrono::milliseconds(updateLocatorListInterval) !=
      pool->getUpdateLocatorListInterval()) {
    sprintf(logmsg,
            "checkPoolAttribs: Pool updateLocatorListInterval expected [%d], "
            "actual [%" PRId64 "]",
            updateLocatorListInterval,
            static_cast<int64_t>(pool->getUpdateLocatorListInterval().count()));
    LOG(logmsg);
    return false;
  }
  return true;
}

int testXmlCacheCreationWithPools() {
  auto cacheFactory = CacheFactory();
  std::shared_ptr<Cache> cptr;

  std::cout << "create DistributedSytem with name=XML_CACHE_CREATION_TEST"
            << std::endl;
  std::cout
      << "Create cache with the configurations provided in valid_cache_pool.xml"
      << std::endl;

  try {
    auto duplicateFile =
        CacheHelper::createDuplicateXMLFile("valid_cache_pool.xml");
    cptr = std::make_shared<Cache>(
        cacheFactory.set("cache-xml-file", duplicateFile).create());
    if (!cptr->getPdxIgnoreUnreadFields()) {
      std::cout << "getPdxIgnoreUnreadFields should return true." << std::endl;
      return -1;
    } else {
      std::cout << "getPdxIgnoreUnreadFields returned true." << std::endl;
    }
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

  if (vrp.size() != 2) {
    std::cout << "Number of root regions does not match" << std::endl;
    return -1;
  }

  std::cout << "Root regions in Cache :" << std::endl;
  for (size_t i = 0; i < vrp.size(); i++) {
    std::cout << "vc[" << i << "].m_regionPtr=" << vrp.at(i).get() << std::endl;
    std::cout << "vc[" << i << "]=" << vrp.at(i)->getName() << std::endl;
  }
  auto regPtr1 = cptr->getRegion("Root1");

  auto &&vr = regPtr1->subregions(true);
  std::cout << "Test if the number of sub regions with the root region Root1 "
               "are correct"
            << std::endl;

  std::cout << "  vr.size=" << vr.size() << std::endl;
  if (vr.size() != 1) {
    std::cout << "Number of Subregions does not match" << std::endl;
    return -1;
  }

  std::cout << "get subregions from the root region :" << vrp.at(0)->getName()
            << std::endl;
  for (size_t i = 0; i < vr.size(); i++) {
    std::cout << "vc[" << i << "].m_regionPtr=" << vr.at(i).get() << std::endl;
    std::cout << "vc[" << i << "]=" << vr.at(i)->getName() << std::endl;
  }

  // TODO - global Issue is that we cannot have config with server and locator
  // pools. Check if this assumption is valid and if so then break up this test.
  auto subRegPtr = vr.at(0);

  auto regPtr2 = cptr->getRegion("Root2");

  std::cout << "Test if the number of sub regions with the root region Root2 "
               "are correct"
            << std::endl;
  vr = regPtr2->subregions(true);
  std::cout << "  vr.size=" << vr.size() << std::endl;
  if (vr.size() != 0) {
    std::cout << "Number of Subregions does not match" << std::endl;
    return -1;
  }

  vr.clear();
  vrp.clear();

  std::cout << "Test the attributes of region" << std::endl;

  const auto &poolNameReg1 = regPtr1->getAttributes().getPoolName();
  const auto &poolNameSubReg = subRegPtr->getAttributes().getPoolName();
  const auto &poolNameReg2 = regPtr2->getAttributes().getPoolName();

  if (poolNameReg1 != "test_pool_1") {
    std::cout << "Wrong pool name for region 1" << std::endl;
    return -1;
  }
  if (poolNameReg2 != "test_pool_2") {
    std::cout << "Wrong pool name for region 2" << std::endl;
    return -1;
  }
  if (poolNameSubReg != "test_pool_2") {
    std::cout << "Wrong pool name for sub region" << std::endl;
    return -1;
  }

  auto poolOfReg1 = cptr->getPoolManager().find(poolNameReg1);
  auto poolOfSubReg = cptr->getPoolManager().find(poolNameSubReg);
  auto poolOfReg2 = cptr->getPoolManager().find(poolNameReg2);
  SLIST locators;
  SLIST servers;
  SLIST emptylist;

  locators.clear();
  servers.clear();
  emptylist.clear();
  char tmp[128];
  sprintf(tmp, "localhost:%d", CacheHelper::staticLocatorHostPort1);

  locators.push_back(string(tmp));
  sprintf(tmp, "localhost:%d", CacheHelper::staticHostPort1);
  servers.push_back(string(tmp));
  sprintf(tmp, "localhost:%d", CacheHelper::staticHostPort2);
  servers.push_back(string(tmp));

  // THIS MUST MATCH WITH THE CLIENT CACHE XML LOADED

  bool check1 = checkPoolAttribs(
      poolOfReg1, locators, emptylist, 12345, 23456, 3, 7, 3,
      std::chrono::milliseconds(5555), 12345, "test_pool_1", 23456,
      "ServerGroup1", 32768, true, 900123, 567, 0, 10123, true, 250001);

  bool check2 = checkPoolAttribs(
      poolOfReg2, emptylist, servers, 23456, 34567, 2, 8, 5,
      std::chrono::milliseconds(6666), 23456, "test_pool_2", 34567,
      "ServerGroup2", 65536, false, 800222, 678, 1, 20345, false, 5000);
  bool check3 = checkPoolAttribs(
      poolOfSubReg, emptylist, servers, 23456, 34567, 2, 8, 5,
      std::chrono::milliseconds(6666), 23456, "test_pool_2", 34567,
      "ServerGroup2", 65536, false, 800222, 678, 1, 20345, false, 5000);

  if (!cptr->isClosed()) {
    cptr->close();
    // Do not set it to null because the destructor will be invoked here and
    // the regions and pools previously obtained, that will be deleted when the
    // function returns, will make use of the their reference to the deleted
    // cache and thus make the process crash.
    // cptr = nullptr;
  }

  if (!check1 || !check2 || !check3) {
    std::cout << "Property check failed" << std::endl;
    return -1;
  }
  ////////////////////////////testing of cache.xml completed///////////////////

  try {
    std::cout << "Testing invalid pool xml 1" << std::endl;
    auto duplicateFile =
        CacheHelper::createDuplicateXMLFile("invalid_cache_pool.xml");
    Cache cache = cacheFactory.set("cache-xml-file", duplicateFile).create();
    return -1;
  } catch (Exception &ex) {
    std::cout << "EXPECTED EXCEPTION" << std::endl;
    std::cout << "Exception: msg = " << ex.what() << std::endl;
    LOG(ex.getStackTrace());
  }

  try {
    std::cout << "Testing invalid pool xml 2" << std::endl;
    auto duplicateFile =
        CacheHelper::createDuplicateXMLFile("invalid_cache_pool2.xml");
    Cache cache = cacheFactory.set("cache-xml-file", duplicateFile).create();
    return -1;
  } catch (Exception &ex) {
    std::cout << "EXPECTED EXCEPTION" << std::endl;
    std::cout << "Exception: msg = " << ex.what() << std::endl;
    LOG(ex.getStackTrace());
  }

  try {
    std::cout << "Testing invalid pool xml 3" << std::endl;
    auto duplicateFile =
        CacheHelper::createDuplicateXMLFile("invalid_cache_pool3.xml");
    Cache cache = cacheFactory.set("cache-xml-file", duplicateFile).create();
    return -1;
  } catch (Exception &ex) {
    std::cout << "EXPECTED EXCEPTION" << std::endl;
    std::cout << "Exception: msg = " << ex.what() << std::endl;
    LOG(ex.getStackTrace());
  }

  try {
    std::cout << "Testing invalid pool xml 4" << std::endl;
    auto duplicateFile =
        CacheHelper::createDuplicateXMLFile("invalid_cache_pool4.xml");
    Cache cache = cacheFactory.set("cache-xml-file", duplicateFile).create();
    return -1;
  } catch (Exception &ex) {
    std::cout << "EXPECTED EXCEPTION" << std::endl;
    std::cout << "Exception: msg = " << ex.what() << std::endl;
    LOG(ex.getStackTrace());
  }

  std::cout << "done with test" << std::endl;
  std::cout << "Test successful!" << std::endl;
  return 0;
}

DUNIT_TASK_DEFINITION(CLIENT1, ValidXmlTestPools)
  {
    CacheHelper::initLocator(1);
    char tmp[128];
    sprintf(tmp, "localhost:%d", CacheHelper::staticLocatorHostPort1);
    CacheHelper::initServer(1, "cacheserver1_pool.xml", tmp);
    CacheHelper::initServer(2, "cacheserver2_pool.xml", tmp);

    int res = testXmlCacheCreationWithPools();

    CacheHelper::closeServer(1);
    CacheHelper::closeServer(2);

    CacheHelper::closeLocator(1);

    if (res != 0) {
      FAIL("Pool Test Failed.");
    }
  }
END_TASK_DEFINITION

DUNIT_MAIN
  { CALL_TASK(ValidXmlTestPools); }
END_MAIN
