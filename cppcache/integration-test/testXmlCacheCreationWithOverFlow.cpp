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

#define ROOT_NAME "testXmlCacheCreationWithOverFlow"

#include <geode/GeodeCppCache.hpp>

#include <string>
#include <iostream>

#include "fw_helper.hpp"

using namespace apache::geode::client;

int testXmlCacheCreationWithOverflow() {
  char* host_name = (char*)"XML_CACHE_CREATION_TEST";
  CacheFactoryPtr cacheFactory;
  CachePtr cptr;
  const uint32_t totalSubRegionsRoot1 = 2;
  const uint32_t totalRootRegions = 2;

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

  std::cout << "Create cache with the configurations provided in "
          "valid_overflowAttr.xml"
       << std::endl;

  try {
    std::string filePath = directory + "/non-existent.xml";
    cptr = cacheFactory->set("cache-xml-file", filePath.c_str())->create();
    return -1;
  } catch (CacheXmlException& ex) {
    ex.showMessage();
    ex.printStackTrace();
  } catch (...) {
    LOGINFO("Unknown exception");
    return -1;
  }

  /// return 0;
  try {
    std::string filePath = directory + "/valid_overflowAttr.xml";
    std::cout << "getPdxIgnoreUnreadFields should return true.1" << std::endl;
    cptr = cacheFactory->set("cache-xml-file", filePath.c_str())->create();
    if (cptr->getPdxIgnoreUnreadFields() != false) {
      std::cout << "getPdxIgnoreUnreadFields should return true." << std::endl;
      return -1;
    } else {
      std::cout << "getPdxIgnoreUnreadFields returned true." << std::endl;
    }
    // return 0;
  } catch (Exception& ex) {
    std::cout << "getPdxIgnoreUnreadFields should return true2." << std::endl;
    ex.showMessage();
    ex.printStackTrace();
    return -1;
  } catch (...) {
    LOGINFO(" unknown exception");
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
  for (int32_t i = 0; i < vrp.size(); i++) {
    std::cout << "vc[" << i << "].m_reaPtr=" << vrp.at(i).get() << std::endl;
    std::cout << "vc[" << i << "]=" << vrp.at(i)->getName() << std::endl;
  }
  RegionPtr regPtr1 = vrp.at(0);

  uint32_t i ATTR_UNUSED = 0;
  std::cout << "Test if the number of sub regions with the root region Root1 are "
          "correct"
       << std::endl;
  VectorOfRegion vr = regPtr1->subregions(true);
  std::cout << "  vr.size=" << vr.size() << std::endl;
  if (vr.size() != totalSubRegionsRoot1) {
    std::cout << "Number of Subregions does not match" << std::endl;
    return -1;
  }

  std::cout << "get subregions from the root region :" << vrp.at(0)->getName()
       << std::endl;
  for (int32_t i = 0; i < vr.size(); i++) {
    std::cout << "vc[" << i << "].m_reaPtr=" << vr.at(i).get() << std::endl;
    std::cout << "vc[" << i << "]=" << vr.at(i)->getName() << std::endl;
  }

  std::cout << "Test if the nesting of regions is correct" << std::endl;
  const char* parentName;
  const char* childName;
  RegionPtr regPtr2 = vrp.at(1);
  VectorOfRegion vsr = regPtr2->subregions(true);
  for (uint32_t i = 0; i < static_cast<uint32_t>(vsr.size()); i++) {
    Region* regPtr = vsr.at(i).get();
    childName = regPtr->getName();

    RegionPtr x = regPtr->getParentRegion();
    parentName = (x.get())->getName();
    if (strcmp(childName, "SubSubRegion221") == 0) {
      if (strcmp(parentName, "SubRegion22") != 0) {
        std::cout << "Incorrect parent: tree structure not formed correctly" << std::endl;
        return -1;
      }
    }
  }
  std::cout << "****Correct region tree structure created from valid_cache.xml****"
       << std::endl;

  vr.clear();
  vrp.clear();

  std::cout << "Test the attributes of region" << std::endl;

  RegionAttributesPtr raPtr = regPtr1->getAttributes();
  RegionAttributes* regAttr = raPtr.get();
  std::cout << "Attributes of root region Root1 are : " << std::endl;

  bool cachingEnabled = regAttr->getCachingEnabled();
  std::cout << "Caching-enabled :true" << std::endl;
  if (!cachingEnabled) {
    return -1;
  }
  int lruEL = regAttr->getLruEntriesLimit();
  std::cout << "lru-entries-limit : 35" << std::endl;
  if (lruEL != 35) {
    return -1;
  }
  int concurrency = regAttr->getConcurrencyLevel();
  std::cout << "concurrency-level : 10" << std::endl;
  if (concurrency != 10) {
    return -1;
  }
  int initialCapacity = regAttr->getInitialCapacity();
  std::cout << "initial-capacity : 25" << std::endl;
  if (initialCapacity != 25) {
    return -1;
  }
  int regionIdleTO = regAttr->getRegionIdleTimeout();
  std::cout << "RegionIdleTimeout:20 " << std::endl;
  if (regionIdleTO != 20) {
    return -1;
  }

  ExpirationAction::Action action1 = regAttr->getRegionIdleTimeoutAction();
  std::cout << "RegionIdleTimeoutAction : Destroy" << std::endl;
  if (action1 != ExpirationAction::DESTROY) {
    return -1;
  }
  const DiskPolicyType::PolicyType type = regAttr->getDiskPolicy();
  std::cout << "DiskPolicy : overflows" << std::endl;
  if (type != DiskPolicyType::OVERFLOWS) {
    std::cout << " diskpolicy is not overflows " << std::endl;
    return -1;
  }

  const char* lib = regAttr->getPersistenceLibrary();
  const char* libFun = regAttr->getPersistenceFactory();
  printf(" persistence library1 = %s\n", lib);
  printf(" persistence function1 = %s\n", libFun);
  std::cout << "persistence library = " << regAttr->getPersistenceLibrary() << std::endl;
  std::cout << "persistence function = " << regAttr->getPersistenceFactory() << std::endl;
  PropertiesPtr pconfig = regAttr->getPersistenceProperties();
  if (pconfig != nullptr) {
    std::cout << " persistence property is not null" << std::endl;
    std::cout << " persistencedir = "
         << pconfig->find("PersistenceDirectory")->asChar() << std::endl;
    std::cout << " pagesize = " << pconfig->find("PageSize")->asChar() << std::endl;
    std::cout << " maxpagecount = " << pconfig->find("MaxPageCount")->asChar()
         << std::endl;
  }
  std::cout << "****Attributes of Root1 are correctly set****" << std::endl;

  RegionAttributesPtr raPtr2 = regPtr2->getAttributes();
  const char* lib2 = raPtr2->getPersistenceLibrary();
  const char* libFun2 = raPtr2->getPersistenceFactory();
  printf(" persistence library2 = %s\n", lib2);
  printf(" persistence function2 = %s\n", libFun2);
  std::cout << "persistence library = " << raPtr2->getPersistenceLibrary() << std::endl;
  std::cout << "persistence function = " << raPtr2->getPersistenceFactory() << std::endl;
  PropertiesPtr pconfig2 = raPtr2->getPersistenceProperties();
  if (pconfig2 != nullptr) {
    std::cout << " persistence property is not null for Root2" << std::endl;
    std::cout << " persistencedir2 = "
         << pconfig2->find("PersistenceDirectory")->asChar() << std::endl;
    std::cout << " pagesize2 = " << pconfig->find("PageSize")->asChar() << std::endl;
    std::cout << " maxpagecount2 = " << pconfig->find("MaxPageCount")->asChar()
         << std::endl;
  }
  std::cout << "Destroy region" << std::endl;
  try {
    regPtr1->localDestroyRegion();
    regPtr2->localDestroyRegion();
  } catch (Exception& ex) {
    ex.showMessage();
    ex.printStackTrace();
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
    std::string filePath = directory + "/invalid_overflowAttr1.xml";
    cptr = cacheFactory->set("cache-xml-file", filePath.c_str())->create();
    return -1;
  } catch (Exception& ex) {
    std::cout << std::endl;
    ex.showMessage();
    ex.printStackTrace();
  }

  ///////////////testing of invalid_cache1.xml completed///////////////////

  std::cout << "Create cache with the configurations provided in the "
          "invalid_overflowAttr2.xml."
       << std::endl;
  std::cout << " This is a well-formed xml....attribute values is not provided for "
          "persistence library name......should throw an exception"
       << std::endl;

  try {
    std::string filePath = directory + "/invalid_overflowAttr2.xml";
    cptr = cacheFactory->set("cache-xml-file", filePath.c_str())->create();
    return -1;
  } catch (CacheXmlException& ex) {
    std::cout << std::endl;
    ex.showMessage();
    ex.printStackTrace();
  }

  ///////////////testing of invalid_cache2.xml completed///////////////////

  std::cout << "Create cache with the configurations provided in the "
          "invalid_overflowAttr3.xml."
       << std::endl;

  std::cout << "This is a well-formed xml....but region-attributes for persistence "
          "invalid......should throw an exception"
       << std::endl;

  try {
    std::string filePath = directory + "/invalid_overflowAttr3.xml";
    cptr = cacheFactory->set("cache-xml-file", filePath.c_str())->create();
    return -1;
  } catch (Exception& ex) {
    std::cout << std::endl;
    ex.showMessage();
    ex.printStackTrace();
  }

  ///////////////testing of invalid_cache3.xml completed///////////////////
  std::cout << "disconnecting..." << std::endl;
  try {
    std::cout << "just before disconnecting..." << std::endl;
    if (cptr != nullptr && !cptr->isClosed()) {
      cptr->close();
      cptr = nullptr;
    }
  } catch (Exception& ex) {
    ex.showMessage();
    ex.printStackTrace();
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
