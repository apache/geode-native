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

#include "RegionXmlCreation.hpp"

#include <geode/Cache.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"

namespace apache {
namespace geode {
namespace client {

void RegionXmlCreation::addSubregion(
    std::shared_ptr<RegionXmlCreation> regionPtr) {
  subRegions.push_back(regionPtr);
}

void RegionXmlCreation::setAttributes(const RegionAttributes attributes) {
  regionAttributes = attributes;
}

RegionAttributes RegionXmlCreation::getAttributes() { return regionAttributes; }

void RegionXmlCreation::fillIn(std::shared_ptr<Region> regionPtr) {
  for (const auto& regXmlCreation : subRegions) {
    regXmlCreation->create(regionPtr);
  }
}

void RegionXmlCreation::createRoot(CacheImpl& cacheImpl) {
  std::shared_ptr<Region> rootRegPtr;

  cacheImpl.createRegion(regionName, regionAttributes, rootRegPtr);

  fillIn(rootRegPtr);
}

void RegionXmlCreation::create(std::shared_ptr<Region> parent) {
  std::shared_ptr<Region> subRegPtr;

  subRegPtr = parent->createSubregion(regionName, regionAttributes);
  fillIn(subRegPtr);
}

RegionXmlCreation::RegionXmlCreation(std::string name, bool isRootRegion) {
  regionName = std::move(name);
  isRoot = isRootRegion;
  attrId = "";
}

std::string RegionXmlCreation::getAttrId() const { return attrId; }

void RegionXmlCreation::setAttrId(const std::string& pattrId) {
  this->attrId = pattrId;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
