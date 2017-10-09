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

#define ROOT_NAME "testCache"

#include <string>
#include <iostream>

#include <geode/GeodeCppCache.hpp>
#include "CacheRegionHelper.hpp"
#include "CacheImpl.hpp"
#include "fw_helper.hpp"

using namespace apache::geode::client;

BEGIN_TEST(CacheFunction)
  char* host_name = (char*)"TESTCACHE";
  uint16_t port ATTR_UNUSED = 0;
  const uint32_t totalSubRegions = 3;
  char* regionName = (char*)"TESTCACHE_ROOT_REGION";
  char* subRegionName1 = (char*)"TESTCACHE_SUB_REGION1";
  char* subRegionName2 = (char*)"TESTCACHE_SUB_REGION2";
  char* subRegionName21 = (char*)"TESTCACHE_SUB_REGION21";
  CachePtr cptr;
  if (cptr != nullptr) {
    std::cout << "cptr is not null" << std::endl;
  }
  std::cout << "create Cache with name=" << host_name
            << " and unitialized system" << std::endl;
  CacheFactoryPtr cacheFactoryPtr = CacheFactory::createCacheFactory();
  cptr = cacheFactoryPtr->create();
  AttributesFactory attrFac;
  RegionAttributesPtr rAttr;
  std::cout << "create RegionAttributes" << std::endl;
  try {
    rAttr = attrFac.createRegionAttributes();
  } catch (Exception& ex) {
    std::cout << ex.getMessage() << std::endl;
    ASSERT(false, "attribute create failed");
  }
  if (rAttr == nullptr) {
    std::cout << "Warnning! : AttributesFactory returned nullptr" << std::endl;
  }
  RegionPtr rptr;
  if (rptr != nullptr) {
    std::cout << "rptr is not null" << std::endl;
  }
  std::cout << "create Region with name=" << regionName << std::endl;
  try {
    CacheImpl* cacheImpl = CacheRegionHelper::getCacheImpl(cptr.get());
    cacheImpl->createRegion(regionName, rAttr, rptr);
  } catch (Exception& ex) {
    std::cout << ex.getMessage() << std::endl;
    ASSERT(false, (char*)"attribute create failed");
  }
  std::cout << "create Sub Region with name=" << subRegionName1 << std::endl;
  RegionPtr subRptr1;
  try {
    subRptr1 = rptr->createSubregion(subRegionName1, rAttr);
  } catch (Exception& ex) {
    std::cout << ex.getMessage() << std::endl;
    ASSERT(false, (char*)"subregion create failed");
  }
  std::cout << "create Sub Region with name=" << subRegionName2 << std::endl;
  RegionPtr subRptr2;
  try {
    subRptr2 = rptr->createSubregion(subRegionName2, rAttr);
  } catch (Exception& ex) {
    std::cout << ex.getMessage() << std::endl;
    ASSERT(false, (char*)"subregion create failed");
  }
  std::cout << "create Sub Region with name=" << subRegionName21
            << "inside region=" << subRegionName2 << std::endl;
  RegionPtr subRptr21;
  try {
    subRptr21 = subRptr2->createSubregion(subRegionName21, rAttr);
  } catch (Exception& ex) {
    std::cout << ex.getMessage() << std::endl;
    ASSERT(false, (char*)"subregion create failed");
  }
  VectorOfRegion vr = rptr->subregions(true);
  std::cout << "  vr.size=" << vr.size() << std::endl;
  ASSERT(vr.size() == totalSubRegions, "Number of Subregions does not match");
  std::cout << "sub regions:" << std::endl;
  uint32_t i = 0;
  for (i = 0; i < static_cast<uint32_t>(vr.size()); i++) {
    std::cout << "vc[" << i << "]=" << vr.at(i)->getName() << std::endl;
  }
  vr.clear();
  std::cout << "get cache root regions" << std::endl;
  auto vrp = cptr->rootRegions();
  std::cout << "  vrp.size=" << vrp.size() << std::endl;
  std::cout << "root regions in Cache:" << std::endl;
  for (i = 0; i < static_cast<uint32_t>(vrp.size()); i++) {
    std::cout << "vc[" << i << "]=" << vrp.at(i)->getName() << std::endl;
  }
  vr.clear();
  std::string root(regionName);
  std::string subRegion2(subRegionName2);
  std::string subRegion1(subRegionName1);
  std::string subRegion21(subRegionName21);
  std::string sptor("/");
  subRegion2 = root + sptor + subRegion2;
  std::cout << "subRegion2=" << subRegion2.c_str() << std::endl;
  subRegion1 = root + sptor + subRegion1;
  std::cout << "subRegion1=" << subRegion1.c_str() << std::endl;
  subRegion21 = subRegion2 + sptor + subRegion21;
  std::cout << "subRegion21=" << subRegion21.c_str() << std::endl;
  RegionPtr region;
  std::cout << "find region:" << regionName << std::endl;
  try {
    region = cptr->getRegion(root.c_str());
  } catch (Exception& ex) {
    std::cout << ex.getMessage() << std::endl;
    ASSERT(false, (char*)"getRegion");
  }
  if (region == nullptr) {
    ASSERT(false, (char*)"did not find it");
  } else {
    std::cout << "found :" << region->getName() << std::endl;
  }
  std::cout << "find region:" << subRegionName1 << std::endl;
  try {
    region = cptr->getRegion(subRegion1.c_str());
  } catch (Exception& ex) {
    std::cout << ex.getMessage() << std::endl;
    ASSERT(false, (char*)"getRegion");
  }
  if (region == nullptr) {
    ASSERT(false, (char*)"did not find it");
  } else {
    std::cout << "found :" << region->getName() << std::endl;
  }
  std::cout << "find region:" << subRegionName21 << std::endl;
  try {
    region = cptr->getRegion(subRegion21.c_str());
  } catch (Exception& ex) {
    std::cout << ex.getMessage() << std::endl;
    ASSERT(false, (char*)"getRegion");
  }
  if (region == nullptr) {
    ASSERT(false, (char*)"did not find it");
  } else {
    std::cout << "found :" << region->getName() << std::endl;
  }
  subRegion21 = sptor + subRegion21;
  std::cout << "find region:" << subRegionName21 << std::endl;
  try {
    region = cptr->getRegion(subRegion21.c_str());
  } catch (Exception& ex) {
    std::cout << ex.getMessage() << std::endl;
    ASSERT(false, (char*)"getRegion");
  }
  if (region == nullptr) {
    ASSERT(false, (char*)"did not find it");
  } else {
    std::cout << "found :" << region->getName() << std::endl;
  }
  const char notExist[] = "/NotExistentRegion";
  std::cout << "find region:" << notExist << std::endl;
  try {
    region = cptr->getRegion(notExist);
  } catch (Exception& ex) {
    std::cout << ex.getMessage() << std::endl;
    ASSERT(false, (char*)"getRegion");
  }
  if (region == nullptr) {
    std::cout << "not found !" << std::endl;
  } else {
    ASSERT(false, (char*)"found it");
  }
END_TEST(CacheFunction)
