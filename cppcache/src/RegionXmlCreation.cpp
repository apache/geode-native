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

#include <geode/Cache.hpp>
#include <CacheRegionHelper.hpp>
#include "RegionXmlCreation.hpp"
#include "CacheImpl.hpp"
using namespace apache::geode::client;

void RegionXmlCreation::addSubregion(
    std::shared_ptr<RegionXmlCreation> regionPtr) {
  subRegions.push_back(regionPtr);
}

void RegionXmlCreation::setAttributes(
    std::shared_ptr<RegionAttributes> attrsPtr) {
  regAttrs = attrsPtr;
}
std::shared_ptr<RegionAttributes> RegionXmlCreation::getAttributes() {
  return regAttrs;
}

void RegionXmlCreation::fillIn(std::shared_ptr<Region> regionPtr) {
  for (const auto& regXmlCreation : subRegions) {
    regXmlCreation->create(regionPtr);
  }
}

void RegionXmlCreation::createRoot(Cache* cache) {
  GF_D_ASSERT(this->isRoot);
  std::shared_ptr<Region> rootRegPtr = nullptr;

  CacheImpl* cacheImpl = CacheRegionHelper::getCacheImpl(cache);
  cacheImpl->createRegion(regionName.c_str(), regAttrs, rootRegPtr);
  fillIn(rootRegPtr);
}

void RegionXmlCreation::create(std::shared_ptr<Region> parent) {
  GF_D_ASSERT(!(this->isRoot));
  std::shared_ptr<Region> subRegPtr = nullptr;

  subRegPtr = parent->createSubregion(regionName.c_str(), regAttrs);
  fillIn(subRegPtr);
}

RegionXmlCreation::RegionXmlCreation(char* name, bool isRootRegion)
    : regAttrs(nullptr) {
  std::string tempName(name);
  regionName = tempName;
  isRoot = isRootRegion;
  attrId = "";
}

std::string RegionXmlCreation::getAttrId() const { return attrId; }

void RegionXmlCreation::setAttrId(const std::string& pattrId) {
  this->attrId = pattrId;
}
