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

#include <geode/CacheFactory.hpp>
#include <geode/RegionAttributesFactory.hpp>
#include <geode/Region.hpp>

#include "CacheRegionHelper.hpp"
#include "CacheImpl.hpp"
#include "fw_helper.hpp"

using apache::geode::client::Cache;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheRegionHelper;
using apache::geode::client::Exception;
using apache::geode::client::Region;
using apache::geode::client::RegionAttributesFactory;

BEGIN_TEST(CacheFunction)
  const uint32_t totalSubRegions = 3;
  const char *regionName = "TESTCACHE_ROOT_REGION";
  const char *subRegionName1 = "TESTCACHE_SUB_REGION1";
  const char *subRegionName2 = "TESTCACHE_SUB_REGION2";
  const char *subRegionName21 = "TESTCACHE_SUB_REGION21";
  std::shared_ptr<Cache> cptr;
  if (cptr != nullptr) {
    std::cout << "cptr is not null" << std::endl;
  }
  std::cout << "create Cache with name=TESTCACHE"
            << " and unitialized system" << std::endl;
  auto cacheFactory = CacheFactory();
  cptr = std::make_shared<Cache>(cacheFactory.create());
  RegionAttributesFactory regionAttributesFactory;

  std::cout << "create RegionAttributes" << std::endl;
  auto regionAttributes = regionAttributesFactory.create();

  std::shared_ptr<Region> rptr;
  if (rptr != nullptr) {
    std::cout << "rptr is not null" << std::endl;
  }
  std::cout << "create Region with name=" << regionName << std::endl;
  try {
    auto cacheImpl = CacheRegionHelper::getCacheImpl(cptr.get());
    cacheImpl->createRegion(regionName, regionAttributes, rptr);
  } catch (Exception &ex) {
    std::cout << ex.what() << std::endl;
    ASSERT(false, "attribute create failed");
  }
  std::cout << "create Sub Region with name=" << subRegionName1 << std::endl;
  std::shared_ptr<Region> subRptr1;
  try {
    subRptr1 = rptr->createSubregion(subRegionName1, regionAttributes);
  } catch (Exception &ex) {
    std::cout << ex.what() << std::endl;
    ASSERT(false, "subregion create failed");
  }
  std::cout << "create Sub Region with name=" << subRegionName2 << std::endl;
  std::shared_ptr<Region> subRptr2;
  try {
    subRptr2 = rptr->createSubregion(subRegionName2, regionAttributes);
  } catch (Exception &ex) {
    std::cout << ex.what() << std::endl;
    ASSERT(false, "subregion create failed");
  }
  std::cout << "create Sub Region with name=" << subRegionName21
            << "inside region=" << subRegionName2 << std::endl;
  std::shared_ptr<Region> subRptr21;
  try {
    subRptr21 = subRptr2->createSubregion(subRegionName21, regionAttributes);
  } catch (Exception &ex) {
    std::cout << ex.what() << std::endl;
    ASSERT(false, "subregion create failed");
  }
  std::vector<std::shared_ptr<Region>> vr = rptr->subregions(true);
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
  std::shared_ptr<Region> region;
  std::cout << "find region:" << regionName << std::endl;
  try {
    region = cptr->getRegion(root.c_str());
  } catch (Exception &ex) {
    std::cout << ex.what() << std::endl;
    ASSERT(false, "getRegion");
  }
  if (region == nullptr) {
    ASSERT(false, "did not find it");
  } else {
    std::cout << "found :" << region->getName() << std::endl;
  }
  std::cout << "find region:" << subRegionName1 << std::endl;
  try {
    region = cptr->getRegion(subRegion1.c_str());
  } catch (Exception &ex) {
    std::cout << ex.what() << std::endl;
    ASSERT(false, "getRegion");
  }
  if (region == nullptr) {
    ASSERT(false, "did not find it");
  } else {
    std::cout << "found :" << region->getName() << std::endl;
  }
  std::cout << "find region:" << subRegionName21 << std::endl;
  try {
    region = cptr->getRegion(subRegion21.c_str());
  } catch (Exception &ex) {
    std::cout << ex.what() << std::endl;
    ASSERT(false, "getRegion");
  }
  if (region == nullptr) {
    ASSERT(false, "did not find it");
  } else {
    std::cout << "found :" << region->getName() << std::endl;
  }
  subRegion21 = sptor + subRegion21;
  std::cout << "find region:" << subRegionName21 << std::endl;
  try {
    region = cptr->getRegion(subRegion21.c_str());
  } catch (Exception &ex) {
    std::cout << ex.what() << std::endl;
    ASSERT(false, "getRegion");
  }
  if (region == nullptr) {
    ASSERT(false, "did not find it");
  } else {
    std::cout << "found :" << region->getName() << std::endl;
  }
  const char notExist[] = "/NotExistentRegion";
  std::cout << "find region:" << notExist << std::endl;
  try {
    region = cptr->getRegion(notExist);
  } catch (Exception &ex) {
    std::cout << ex.what() << std::endl;
    ASSERT(false, "getRegion");
  }
  if (region == nullptr) {
    std::cout << "not found !" << std::endl;
  } else {
    ASSERT(false, "found it");
  }
END_TEST(CacheFunction)
