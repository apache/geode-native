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
#include <math.h>

#include <geode/CacheFactory.hpp>
#include <geode/Region.hpp>

#include "fw_helper.hpp"

using apache::geode::client::Cache;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheXmlException;
using apache::geode::client::ExpirationAction;
using apache::geode::client::Region;

int testXmlCacheCreationWithRefid(const char *fileName) {
  auto cacheFactory = CacheFactory();
  std::shared_ptr<Cache> cptr;

  char *path = std::getenv("TESTSRC");
  std::string directory(path);

  std::cout << "create DistributedSytem with name=XML_CACHE_CREATION_TEST"
            << std::endl;

  std::cout << "Create cache with the configurations provided in "
               "valid_cache_refid.xml"
            << std::endl;

  try {
    const auto filePath = directory + fileName;
    cptr = std::make_shared<Cache>(
        cacheFactory.set("cache-xml-file", filePath).create());
    if (cptr->getPdxIgnoreUnreadFields() != false) {
      std::cout << "getPdxIgnoreUnreadFields should return false." << std::endl;
      return -1;
    } else {
      std::cout << "getPdxIgnoreUnreadFields returned false." << std::endl;
    }
  } catch (CacheXmlException &ex) {
    std::cout << "CacheXmlException: msg = " << ex.what() << std::endl;
    LOG(ex.getStackTrace());
    return -1;
  }

  std::shared_ptr<Region> Root1;
  std::shared_ptr<Region> SubRegion1;
  std::shared_ptr<Region> SubRegion11;
  std::shared_ptr<Region> SubRegion2;

  std::shared_ptr<Region> Root2;
  std::shared_ptr<Region> SubRegion21;

  std::cout << "Verify whether all the regions are created" << std::endl;

  try {
    Root1 = cptr->getRegion("Root1");
    SubRegion1 = Root1->getSubregion("SubRegion1");
    SubRegion11 = SubRegion1->getSubregion("SubRegion11");
    SubRegion2 = Root1->getSubregion("SubRegion2");

    Root2 = cptr->getRegion("Root2");
    SubRegion21 = Root2->getSubregion("SubRegion21");
  } catch (...) {
    LOG_INFO("Unknown Exception while getting one of the regions");
    return -1;
  }

  std::cout << "Verify whether region 'SubRegion11' has correct attributes"
            << std::endl;

  auto atts = SubRegion11->getAttributes();

  if (atts.getCachingEnabled() != true) {
    LOG_INFO("Caching is not enabled in SubRegion11");
    return -1;
  }

  if (atts.getInitialCapacity() != 10) {
    LOG_INFO("Initial capacity of SubRegion11 is not 10");
    return -1;
  }

  if (atts.getConcurrencyLevel() != 52) {
    LOG_INFO("Concurrency level of SubRegion11 is not 52");
    return -1;
  }

  if (fabs(atts.getLoadFactor() - 0.89) > 0.001) {
    LOG_INFO("Load factor of SubRegion11 is not 0.89");
    return -1;
  }

  std::cout << "Verify whether region 'SubRegion2' has correct attributes"
            << std::endl;

  atts = SubRegion2->getAttributes();

  if (atts.getCachingEnabled() != true) {
    LOG_INFO("Caching is not enabled in SubRegion2");
    return -1;
  }

  if (atts.getInitialCapacity() != 10) {
    LOG_INFO("Initial capacity of SubRegion2 is not 10");
    return -1;
  }

  if (fabs(atts.getLoadFactor() - 0.89) > 0.001) {
    LOG_INFO("Load factor of SubRegion2 is not 0.89");
    return -1;
  }

  if (atts.getConcurrencyLevel() != 52) {
    LOG_INFO("Concurrency level of SubRegion2 is not 52");
    return -1;
  }

  std::cout << "Verify whether region 'SubRegion21' has correct attributes"
            << std::endl;

  atts = SubRegion21->getAttributes();

  if (atts.getCachingEnabled() != true) {
    LOG_INFO("Caching is not enabled in SubRegion21");
    return -1;
  }

  if (atts.getInitialCapacity() != 10) {
    LOG_INFO("Initial capacity of SubRegion21 is not 10");
    return -1;
  }

  if (fabs(atts.getLoadFactor() - 0.89) > 0.001) {
    LOG_INFO("Load factor of SubRegion21 is not 0.89");
    return -1;
  }

  if (atts.getConcurrencyLevel() != 52) {
    LOG_INFO("Concurrency level of SubRegion21 is not 52");
    return -1;
  }

  if (atts.getEntryIdleTimeout().count() != 10) {
    LOG_INFO("Entryidletimeout of SubRegion21 is not 10");
    return -1;
  }

  if (atts.getEntryIdleTimeoutAction() != ExpirationAction::INVALIDATE) {
    LOG_INFO("Entryidletimeoutaction of SubRegion21 is not invalidate");
    return -1;
  }

  if (atts.getRegionIdleTimeout().count() != 20) {
    LOG_INFO("Regionidletimeout of SubRegion21 is not 20");
    return -1;
  }

  if (atts.getRegionIdleTimeoutAction() != ExpirationAction::DESTROY) {
    LOG_INFO("Regionidletimeoutaction of SubRegion21 is not destroy");
    return -1;
  }

  std::cout << "Verify whether region 'Root2' has correct attributes"
            << std::endl;

  atts = Root2->getAttributes();

  if (atts.getCachingEnabled() != true) {
    LOG_INFO("Caching is not enabled in Root2");
    return -1;
  }

  if (atts.getInitialCapacity() != 25) {
    LOG_INFO("Initial capacity of Root2 is not 10");
    return -1;
  }

  if (fabs(atts.getLoadFactor() - 0.32) > 0.001) {
    LOG_INFO("Load factor of Root2 is not 0.0.32");
    return -1;
  }

  if (atts.getConcurrencyLevel() != 16) {
    LOG_INFO("Concurrency level of Root2 is not 16");
    return -1;
  }

  if (atts.getEntryIdleTimeout().count() != 10) {
    LOG_INFO("Entryidletimeout of Root2 is not 10");
    return -1;
  }

  if (atts.getEntryIdleTimeoutAction() != ExpirationAction::INVALIDATE) {
    LOG_INFO("Entryidletimeoutaction of Root2 is not invalidate");
    return -1;
  }

  if (atts.getEntryTimeToLive().count() != 0) {
    LOG_INFO("Entrytimetolive of Root2 is not 0");
    return -1;
  }

  if (atts.getEntryTimeToLiveAction() != ExpirationAction::LOCAL_INVALIDATE) {
    LOG_INFO("Entrytimetoliveaction of Root2 is not local_invalidate");
    return -1;
  }

  if (atts.getRegionIdleTimeout().count() != 0) {
    LOG_INFO("Regionidletimeout of Root2 is not 0");
    return -1;
  }

  if (atts.getRegionIdleTimeoutAction() != ExpirationAction::INVALIDATE) {
    LOG_INFO("Regionidletimeoutaction of Root2 is not invalidate");
    return -1;
  }

  if (atts.getRegionTimeToLive().count() != 0) {
    LOG_INFO("Regiontimetolive of Root2 is not 0");
    return -1;
  }

  if (atts.getRegionTimeToLiveAction() != ExpirationAction::DESTROY) {
    LOG_INFO("Regiontimetoliveaction of Root2 is not destroy");
    return -1;
  }

  cptr->close();

  return 0;
}

BEGIN_TEST(ValidXmlTestRefid)
  {
    int res = testXmlCacheCreationWithRefid("/resources/valid_cache_refid.xml");
    if (res != 0) {
      FAIL("Test Failed.");
    }

    res = testXmlCacheCreationWithRefid(
        "/resources/valid_cache_region_refid.xml");
    if (res != 0) {
      FAIL("Test Failed.");
    }
  }
END_TEST(ValidXmlTestRefid)
