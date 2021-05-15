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
#include <string>
#include <iostream>

#include "fw_helper.hpp"

using apache::geode::client::Cache;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheXmlException;
using apache::geode::client::DiskPolicyType;
using apache::geode::client::Exception;
using apache::geode::client::ExpirationAction;
using apache::geode::client::Region;

int testXmlCacheCreationWithOverflow() {
  auto cacheFactory = CacheFactory();
  std::shared_ptr<Cache> cptr;
  const uint32_t totalSubRegionsRoot1 = 2;
  const uint32_t totalRootRegions = 2;

  char *path = std::getenv("TESTSRC");
  std::string directory(path);

  std::cout << "create DistributedSytem with name=XML_CACHE_CREATION_TEST"
            << std::endl;
  std::cout << "Create cache with the configurations provided in "
               "valid_overflowAttr.xml"
            << std::endl;

  try {
    std::string filePath = directory + "/resources/non-existent.xml";
    cptr = std::make_shared<Cache>(
        cacheFactory.set("cache-xml-file", filePath.c_str()).create());
    return -1;
  } catch (CacheXmlException &ex) {
    std::cout << "CacheXmlException: msg = " << ex.what() << std::endl;
    LOG(ex.getStackTrace());
  } catch (...) {
    LOG_INFO("Unknown exception");
    return -1;
  }

  /// return 0;
  try {
    std::string filePath = directory + "/resources/valid_overflowAttr.xml";
    std::cout << "getPdxIgnoreUnreadFields should return true.1" << std::endl;
    cptr = std::make_shared<Cache>(
        cacheFactory.set("cache-xml-file", filePath.c_str()).create());
    if (cptr->getPdxIgnoreUnreadFields() != false) {
      std::cout << "getPdxIgnoreUnreadFields should return true." << std::endl;
      return -1;
    } else {
      std::cout << "getPdxIgnoreUnreadFields returned true." << std::endl;
    }
    // return 0;
  } catch (Exception &ex) {
    std::cout << "getPdxIgnoreUnreadFields should return true2." << std::endl;
    std::cout << "Exception: msg = " << ex.what() << std::endl;
    LOG(ex.getStackTrace());
    return -1;
  } catch (...) {
    LOG_INFO(" unknown exception");
    return -1;
  }

  std::cout << "Test if number of root regions are correct" << std::endl;
  auto vrp = cptr->rootRegions();
  std::cout << "  vrp.size=" << vrp.size() << std::endl;

  if (vrp.size() != totalRootRegions) {
    std::cout << "Number of root regions does not match" << std::endl;
    return -1;
  }

  std::cout << "Root regions in Cache :" << std::endl;
  for (size_t i = 0; i < vrp.size(); i++) {
    std::cout << "vc[" << i << "].m_reaPtr=" << vrp.at(i).get() << std::endl;
    std::cout << "vc[" << i << "]=" << vrp.at(i)->getName() << std::endl;
  }
  auto regPtr1 = cptr->getRegion("Root1");

  std::cout
      << "Test if the number of sub regions with the root region Root1 are "
         "correct"
      << std::endl;
  auto vr = regPtr1->subregions(true);
  std::cout << "  vr.size=" << vr.size() << std::endl;
  if (vr.size() != totalSubRegionsRoot1) {
    std::cout << "Number of Subregions does not match" << std::endl;
    return -1;
  }

  std::cout << "get subregions from the root region :" << vrp.at(0)->getName()
            << std::endl;
  for (size_t i = 0; i < vr.size(); i++) {
    std::cout << "vc[" << i << "].m_reaPtr=" << vr.at(i).get() << std::endl;
    std::cout << "vc[" << i << "]=" << vr.at(i)->getName() << std::endl;
  }

  std::cout << "Test if the nesting of regions is correct" << std::endl;
  auto regPtr2 = cptr->getRegion("Root2");
  auto &&vsr = regPtr2->subregions(true);
  for (auto &&regPtr : vsr) {
    auto &&childName = regPtr->getName();
    auto &&parentName = regPtr->getParentRegion()->getName();
    if (childName == "SubSubRegion221") {
      if (parentName != "SubRegion22") {
        std::cout << "Incorrect parent: tree structure not formed correctly"
                  << std::endl;
        return -1;
      }
    }
  }
  std::cout
      << "****Correct region tree structure created from valid_cache.xml****"
      << std::endl;

  vr.clear();
  vrp.clear();

  std::cout << "Test the attributes of region" << std::endl;

  auto regionAttributes = regPtr1->getAttributes();
  std::cout << "Attributes of root region Root1 are : " << std::endl;

  bool cachingEnabled = regionAttributes.getCachingEnabled();
  std::cout << "Caching-enabled :true" << std::endl;
  if (!cachingEnabled) {
    return -1;
  }
  int lruEL = regionAttributes.getLruEntriesLimit();
  std::cout << "lru-entries-limit : 35" << std::endl;
  if (lruEL != 35) {
    return -1;
  }
  int concurrency = regionAttributes.getConcurrencyLevel();
  std::cout << "concurrency-level : 10" << std::endl;
  if (concurrency != 10) {
    return -1;
  }
  int initialCapacity = regionAttributes.getInitialCapacity();
  std::cout << "initial-capacity : 25" << std::endl;
  if (initialCapacity != 25) {
    return -1;
  }
  auto regionIdleTO = regionAttributes.getRegionIdleTimeout().count();
  std::cout << "RegionIdleTimeout:20 " << std::endl;
  if (regionIdleTO != 20) {
    return -1;
  }

  ExpirationAction action1 = regionAttributes.getRegionIdleTimeoutAction();
  std::cout << "RegionIdleTimeoutAction : Destroy" << std::endl;
  if (action1 != ExpirationAction::DESTROY) {
    return -1;
  }
  const DiskPolicyType type = regionAttributes.getDiskPolicy();
  std::cout << "DiskPolicy : overflows" << std::endl;
  if (type != DiskPolicyType::OVERFLOWS) {
    std::cout << " diskpolicy is not overflows " << std::endl;
    return -1;
  }

  std::cout << "persistence library = "
            << regionAttributes.getPersistenceLibrary() << std::endl;
  std::cout << "persistence function = "
            << regionAttributes.getPersistenceFactory() << std::endl;
  auto pconfig = regionAttributes.getPersistenceProperties();
  if (pconfig != nullptr) {
    std::cout << " persistence property is not null" << std::endl;
    std::cout << " persistencedir = "
              << pconfig->find("PersistenceDirectory")->value().c_str()
              << std::endl;
    std::cout << " pagesize = " << pconfig->find("PageSize")->value().c_str()
              << std::endl;
    std::cout << " maxpagecount = "
              << pconfig->find("MaxPageCount")->value().c_str() << std::endl;
  }
  std::cout << "****Attributes of Root1 are correctly set****" << std::endl;

  auto regionAttributes2 = regPtr2->getAttributes();
  std::cout << "persistence library = "
            << regionAttributes2.getPersistenceLibrary() << std::endl;
  std::cout << "persistence function = "
            << regionAttributes2.getPersistenceFactory() << std::endl;
  auto pconfig2 = regionAttributes2.getPersistenceProperties();
  if (pconfig2 != nullptr) {
    std::cout << " persistence property is not null for Root2" << std::endl;
    std::cout << " persistencedir2 = "
              << pconfig2->find("PersistenceDirectory")->value().c_str()
              << std::endl;
    std::cout << " pagesize2 = " << pconfig->find("PageSize")->value().c_str()
              << std::endl;
    std::cout << " maxpagecount2 = "
              << pconfig->find("MaxPageCount")->value().c_str() << std::endl;
  }
  std::cout << "Destroy region" << std::endl;
  try {
    regPtr1->localDestroyRegion();
    regPtr2->localDestroyRegion();
  } catch (Exception &ex) {
    std::cout << "Exception: msg = " << ex.what() << std::endl;
    LOG(ex.getStackTrace());
    return -1;
  }

  regPtr1 = nullptr;
  regPtr2 = nullptr;

  if (!cptr->isClosed()) {
    cptr->close();
    cptr = nullptr;
  }
  ////////////////////////////testing of cache.xml completed///////////////////

  std::cout << "Create cache with the configurations provided in the "
               "invalid_overflowAttr1.xml."
            << std::endl;
  std::cout << "This is a well-formed xml....attributes not provided for "
               "persistence manager. exception should be thrown"
            << std::endl;

  try {
    const auto filePath = directory + "/invalid_overflowAttr1.xml";
    cptr = std::make_shared<Cache>(
        cacheFactory.set("cache-xml-file", filePath).create());
    return -1;
  } catch (Exception &ex) {
    std::cout << std::endl;
    std::cout << "Exception: msg = " << ex.what() << std::endl;
    LOG(ex.getStackTrace());
  }

  ///////////////testing of invalid_cache1.xml completed///////////////////

  std::cout << "Create cache with the configurations provided in the "
               "invalid_overflowAttr2.xml."
            << std::endl;
  std::cout
      << " This is a well-formed xml....attribute values is not provided for "
         "persistence library name......should throw an exception"
      << std::endl;

  try {
    const auto filePath = directory + "/invalid_overflowAttr2.xml";
    cptr = std::make_shared<Cache>(
        cacheFactory.set("cache-xml-file", filePath).create());
    return -1;
  } catch (CacheXmlException &ex) {
    std::cout << std::endl;
    std::cout << "CacheXmlException: msg = " << ex.what() << std::endl;
    LOG(ex.getStackTrace());
  }

  ///////////////testing of invalid_cache2.xml completed///////////////////

  std::cout << "Create cache with the configurations provided in the "
               "invalid_overflowAttr3.xml."
            << std::endl;

  std::cout
      << "This is a well-formed xml....but region-attributes for persistence "
         "invalid......should throw an exception"
      << std::endl;

  try {
    const auto filePath = directory + "/invalid_overflowAttr3.xml";
    cptr = std::make_shared<Cache>(
        cacheFactory.set("cache-xml-file", filePath).create());
    return -1;
  } catch (Exception &ex) {
    std::cout << std::endl;
    std::cout << "Exception: msg = " << ex.what() << std::endl;
    LOG(ex.getStackTrace());
  }

  ///////////////testing of invalid_cache3.xml completed///////////////////
  std::cout << "disconnecting..." << std::endl;
  try {
    std::cout << "just before disconnecting..." << std::endl;
    if (cptr != nullptr && !cptr->isClosed()) {
      cptr->close();
      cptr = nullptr;
    }
  } catch (Exception &ex) {
    std::cout << "Exception: msg = " << ex.what() << std::endl;
    LOG(ex.getStackTrace());
    return -1;
  }
  std::cout << "done with test" << std::endl;
  std::cout << "Test successful!" << std::endl;
  return 0;
}

BEGIN_TEST(ValidXmlTestOverFlow)
  {
    int res = testXmlCacheCreationWithOverflow();
    if (res != 0) {
      FAIL("Test Failed.");
    }
  }
END_TEST(ValidXmlTestOverFlow)
