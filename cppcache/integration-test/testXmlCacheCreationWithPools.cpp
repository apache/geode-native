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

#include <geode/GeodeCppCache.hpp>

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2

#include "CacheHelper.hpp"

static bool isLocalServer = false;
static bool isLocator = false;
static int numberOfLocators = 1;
const char* endPoints = (const char*)nullptr;
const char* locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);

#include "LocatorHelper.hpp"

using namespace apache::geode::client;
using namespace test;
using namespace std;

#define SLIST vector<string>

bool findString(string& item, CacheableStringArrayPtr array) {
  for (int size = 0; size < array->length(); size++) {
    if (strcmp(item.c_str(), array->operator[](size)->asChar()) == 0) {
      return true;
    }
  }

  return false;
}

bool checkStringArray(SLIST& first, CacheableStringArrayPtr second) {
  if (second == nullptr && first.size() > 0) return false;

  if (second == nullptr && first.size() == 0) return true;

  if (first.size() != second->length()) return false;

  for (size_t size = 0; size < first.size(); size++) {
    if (!findString(first[size], second)) {
      return false;
    }
  }

  return true;
}

bool checkPoolAttribs(PoolPtr pool, SLIST& locators, SLIST& servers,
                      int freeConnectionTimeout, int loadConditioningInterval,
                      int minConnections, int maxConnections, int retryAttempts,
                      int idleTimeout, int pingInterval, const char* name,
                      int readTimeout, const char* serverGroup,
                      int socketBufferSize, bool subscriptionEnabled,
                      int subscriptionMessageTrackingTimeout,
                      int subscriptionAckInterval, int subscriptionRedundancy,
                      int statisticInterval, int threadLocalConnections,
                      bool prSingleHopEnabled, int updateLocatorListInterval) {
  char logmsg[500] = {0};

  if (pool == nullptr) {
    LOG("checkPoolAttribs: PoolPtr is nullptr");
    return false;
  }

  std::cout << "Checking pool " << pool->getName() << std::endl;

  if (strcmp(pool->getName(), name)) {
    sprintf(logmsg, "checkPoolAttribs: Pool name expected [%s], actual [%s]",
            name, pool->getName() == nullptr ? "null" : pool->getName());
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
  if (freeConnectionTimeout != pool->getFreeConnectionTimeout()) {
    sprintf(logmsg,
            "checkPoolAttribs: Pool freeConnectionTimeout expected [%d], "
            "actual [%d]",
            freeConnectionTimeout, pool->getFreeConnectionTimeout());
    LOG(logmsg);
    return false;
  }
  if (loadConditioningInterval != pool->getLoadConditioningInterval()) {
    sprintf(logmsg,
            "checkPoolAttribs: Pool loadConditioningInterval expected [%d], "
            "actual [%d]",
            loadConditioningInterval, pool->getLoadConditioningInterval());
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
            "checkPoolAttribs: Pool idleTimeout expected [%d], actual [%ld]",
            idleTimeout, pool->getIdleTimeout());
    LOG(logmsg);
    return false;
  }
  if (pingInterval != pool->getPingInterval()) {
    sprintf(logmsg,
            "checkPoolAttribs: Pool pingInterval expected [%d], actual [%ld]",
            pingInterval, pool->getPingInterval());
    LOG(logmsg);
    return false;
  }
  if (readTimeout != pool->getReadTimeout()) {
    sprintf(logmsg,
            "checkPoolAttribs: Pool readTimeout expected [%d], actual [%d]",
            readTimeout, pool->getReadTimeout());
    LOG(logmsg);
    return false;
  }
  if (strcmp(serverGroup, pool->getServerGroup())) {
    sprintf(
        logmsg, "checkPoolAttribs: Pool serverGroup expected [%s], actual [%s]",
        serverGroup,
        pool->getServerGroup() == nullptr ? "null" : pool->getServerGroup());
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
  if (subscriptionMessageTrackingTimeout !=
      pool->getSubscriptionMessageTrackingTimeout()) {
    sprintf(logmsg,
            "checkPoolAttribs: Pool subscriptionMessageTrackingTimeout "
            "expected [%d], actual [%d]",
            subscriptionMessageTrackingTimeout,
            pool->getSubscriptionMessageTrackingTimeout());
    LOG(logmsg);
    return false;
  }
  if (subscriptionAckInterval != pool->getSubscriptionAckInterval()) {
    sprintf(logmsg,
            "checkPoolAttribs: Pool subscriptionAckInterval expected [%d], "
            "actual [%d]",
            subscriptionAckInterval, pool->getSubscriptionAckInterval());
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
  if (statisticInterval != pool->getStatisticInterval()) {
    sprintf(
        logmsg,
        "checkPoolAttribs: Pool statisticInterval expected [%d], actual [%d]",
        statisticInterval, pool->getStatisticInterval());
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
  if (updateLocatorListInterval != pool->getUpdateLocatorListInterval()) {
    sprintf(logmsg,
            "checkPoolAttribs: Pool updateLocatorListInterval expected [%d], "
            "actual [%ld]",
            updateLocatorListInterval, pool->getUpdateLocatorListInterval());
    LOG(logmsg);
    return false;
  }
  return true;
}

int testXmlCacheCreationWithPools() {
  char* host_name = (char*)"XML_CACHE_CREATION_TEST";
  CacheFactoryPtr cacheFactory;
  CachePtr cptr;

  std::cout << "create DistributedSytem with name=" << host_name << std::endl;
  try {
    cacheFactory = CacheFactory::createCacheFactory();
  } catch (Exception& ex) {
    ex.showMessage();
    ex.printStackTrace();
    return -1;
  }

  std::cout
      << "Create cache with the configurations provided in valid_cache_pool.xml"
      << std::endl;

  try {
    std::string filePath = "valid_cache_pool.xml";
    std::string duplicateFile;
    CacheHelper::createDuplicateXMLFile(duplicateFile, filePath);
    cptr = cacheFactory->set("cache-xml-file", duplicateFile.c_str())->create();
    if (cptr->getPdxIgnoreUnreadFields() != true) {
      std::cout << "getPdxIgnoreUnreadFields should return true." << std::endl;
      return -1;
    } else {
      std::cout << "getPdxIgnoreUnreadFields returned true." << std::endl;
    }
  } catch (Exception& ex) {
    ex.showMessage();
    ex.printStackTrace();
    return -1;
  } catch (...) {
    LOGINFO("unknown exception");
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
  for (int32_t i = 0; i < vrp.size(); i++) {
    std::cout << "vc[" << i << "].m_regionPtr=" << vrp.at(i).get() << std::endl;
    std::cout << "vc[" << i << "]=" << vrp.at(i)->getName() << std::endl;
  }
  RegionPtr regPtr1 = vrp.at(0);

  VectorOfRegion vr = regPtr1->subregions(true);
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
  for (int32_t i = 0; i < vr.size(); i++) {
    std::cout << "vc[" << i << "].m_regionPtr=" << vr.at(i).get() << std::endl;
    std::cout << "vc[" << i << "]=" << vr.at(i)->getName() << std::endl;
  }

  // TODO - global Issue is that we cannot have config with server and locator
  // pools. Check if this assumption is valid and if so then break up this test.
  RegionPtr subRegPtr = vr.at(0);

  RegionPtr regPtr2 = vrp.at(1);

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

  const char* poolNameReg1 = regPtr1->getAttributes()->getPoolName();
  const char* poolNameSubReg = subRegPtr->getAttributes()->getPoolName();
  const char* poolNameReg2 = regPtr2->getAttributes()->getPoolName();

  if (strcmp(poolNameReg1, "test_pool_1")) {
    std::cout << "Wrong pool name for region 1" << std::endl;
    return -1;
  }
  if (strcmp(poolNameReg2, "test_pool_2")) {
    std::cout << "Wrong pool name for region 2" << std::endl;
    return -1;
  }
  if (strcmp(poolNameSubReg, "test_pool_2")) {
    std::cout << "Wrong pool name for sub region" << std::endl;
    return -1;
  }

  PoolPtr poolOfReg1 = cptr->getPoolManager().find(poolNameReg1);
  PoolPtr poolOfSubReg = cptr->getPoolManager().find(poolNameSubReg);
  PoolPtr poolOfReg2 = cptr->getPoolManager().find(poolNameReg2);
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

  bool check1 =
      checkPoolAttribs(poolOfReg1, locators, emptylist, 12345, 23456, 3, 7, 3,
                       5555, 12345, "test_pool_1", 23456, "ServerGroup1", 32768,
                       true, 900123, 567, 0, 10123, 5, true, 250001);

  bool check2 =
      checkPoolAttribs(poolOfReg2, emptylist, servers, 23456, 34567, 2, 8, 5,
                       6666, 23456, "test_pool_2", 34567, "ServerGroup2", 65536,
                       false, 800222, 678, 1, 20345, 3, false, 5000);
  bool check3 =
      checkPoolAttribs(poolOfSubReg, emptylist, servers, 23456, 34567, 2, 8, 5,
                       6666, 23456, "test_pool_2", 34567, "ServerGroup2", 65536,
                       false, 800222, 678, 1, 20345, 3, false, 5000);

  if (!cptr->isClosed()) {
    cptr->close();
    cptr = nullptr;
  }

  if (!check1 || !check2 || !check3) {
    std::cout << "Property check failed" << std::endl;
    return -1;
  }
  ////////////////////////////testing of cache.xml completed///////////////////

  try {
    std::cout << "Testing invalid pool xml 1" << std::endl;
    std::string filePath = "invalid_cache_pool.xml";
    std::string duplicateFile;
    CacheHelper::createDuplicateXMLFile(duplicateFile, filePath);
    cptr = cacheFactory->set("cache-xml-file", duplicateFile.c_str())->create();
    return -1;
  } catch (Exception& ex) {
    std::cout << "EXPECTED EXCEPTION" << std::endl;
    ex.showMessage();
    ex.printStackTrace();
  }

  try {
    std::cout << "Testing invalid pool xml 2" << std::endl;
    std::string filePath = "invalid_cache_pool2.xml";
    std::string duplicateFile;
    CacheHelper::createDuplicateXMLFile(duplicateFile, filePath);
    cptr = cacheFactory->set("cache-xml-file", duplicateFile.c_str())->create();
    return -1;
  } catch (Exception& ex) {
    std::cout << "EXPECTED EXCEPTION" << std::endl;
    ex.showMessage();
    ex.printStackTrace();
  }

  try {
    std::cout << "Testing invalid pool xml 3" << std::endl;
    std::string filePath = "invalid_cache_pool3.xml";
    std::string duplicateFile;
    CacheHelper::createDuplicateXMLFile(duplicateFile, filePath);
    cptr = cacheFactory->set("cache-xml-file", duplicateFile.c_str())->create();
    return -1;
  } catch (Exception& ex) {
    std::cout << "EXPECTED EXCEPTION" << std::endl;
    ex.showMessage();
    ex.printStackTrace();
  }

  try {
    std::cout << "Testing invalid pool xml 4" << std::endl;
    std::string filePath = "invalid_cache_pool4.xml";
    std::string duplicateFile;
    CacheHelper::createDuplicateXMLFile(duplicateFile, filePath);
    cptr = cacheFactory->set("cache-xml-file", duplicateFile.c_str())->create();
    return -1;
  } catch (Exception& ex) {
    std::cout << "EXPECTED EXCEPTION" << std::endl;
    ex.showMessage();
    ex.printStackTrace();
  }

  std::cout << "disconnecting..." << std::endl;
  try {
    std::cout << "just before disconnecting..." << std::endl;
    if (cptr != nullptr) cptr->close();
  } catch (Exception& ex) {
    ex.showMessage();
    ex.printStackTrace();
    return -1;
  }
  std::cout << "done with test" << std::endl;
  std::cout << "Test successful!" << std::endl;
  return 0;
}

int testXmlDeclarativeCacheCreation() {
  char* host_name = (char*)"XML_DECLARATIVE_CACHE_CREATION_TEST";
  CacheFactoryPtr cacheFactory;
  CachePtr cptr;

  char* path = ACE_OS::getenv("TESTSRC");
  std::string directory(path);

  std::cout << "create DistributedSytem with name=" << host_name << std::endl;
  try {
    cacheFactory = CacheFactory::createCacheFactory();
  } catch (Exception& ex) {
    ex.showMessage();
    ex.printStackTrace();
    return -1;
  }

  try {
    std::string filePath = directory + "/valid_declarative_cache_creation.xml";
    cptr = cacheFactory->set("cache-xml-file", filePath.c_str())->create();

  } catch (Exception& ex) {
    ex.showMessage();
    ex.printStackTrace();
    return -1;
  } catch (...) {
    LOGINFO("unknown exception");
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
  for (int32_t i = 0; i < vrp.size(); i++) {
    std::cout << "vc[" << i << "].m_reaPtr=" << vrp.at(i).get() << std::endl;
    std::cout << "vc[" << i << "]=" << vrp.at(i)->getName() << std::endl;
  }
  RegionPtr regPtr1 = vrp.at(0);

  RegionAttributesPtr raPtr = regPtr1->getAttributes();
  RegionAttributes* regAttr = raPtr.get();
  std::cout << "Test Attributes of root region Root1 " << std::endl;
  std::cout << "Region name " << regPtr1->getName() << std::endl;

  if (regAttr->getCacheLoader() == nullptr) {
    std::cout << "Cache Loader not initialized." << std::endl;
    return -1;
  }

  if (regAttr->getCacheListener() == nullptr) {
    std::cout << "Cache Listener not initialized." << std::endl;
    return -1;
  }

  if (regAttr->getCacheWriter() == nullptr) {
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

DUNIT_TASK_DEFINITION(CLIENT1, ValidXmlTestDeclarativeCacheCreation)
  {
    int res = testXmlDeclarativeCacheCreation();
    if (res != 0) {
      FAIL("DeclarativeCacheCreation Test Failed.");
    }
  }
END_TASK_DEFINITION

DUNIT_MAIN
  {
    CALL_TASK(ValidXmlTestPools);
    CALL_TASK(ValidXmlTestDeclarativeCacheCreation);
  }
END_MAIN
