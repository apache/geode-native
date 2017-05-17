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
extern bool Cache_CreatedFromCacheFactory;

void RegionXmlCreation::addSubregion(RegionXmlCreation* regionPtr) {
  subRegions.push_back(regionPtr);
}

void RegionXmlCreation::setAttributes(RegionAttributesPtr attrsPtr) {
  regAttrs = attrsPtr;
}

RegionAttributesPtr RegionXmlCreation::getAttributes() { return regAttrs; }

void RegionXmlCreation::fillIn(RegionPtr regionPtr) {
  std::vector<RegionXmlCreation*>::iterator start = subRegions.begin();
  while (start != subRegions.end()) {
    RegionXmlCreation* regXmlCreation = *start;
    regXmlCreation->create(regionPtr);
    start++;
  }
}

void RegionXmlCreation::createRoot(Cache* cache) {
  GF_D_ASSERT(this->isRoot);
  RegionPtr rootRegPtr = nullptr;

  if (Cache_CreatedFromCacheFactory) {
    //  if(cache->m_cacheImpl->getDefaultPool() == nullptr)
    {
      // we may need to initialize default pool
      if (regAttrs->getEndpoints() == nullptr) {
        if (regAttrs->getPoolName() == nullptr) {
          PoolPtr pool = CacheFactory::createOrGetDefaultPool();

          if (pool == nullptr) {
            throw IllegalStateException("Pool is not defined create region.");
          }
          regAttrs->setPoolName(pool->getName());
        }
      }
    }
  }

  CacheImpl* cacheImpl = CacheRegionHelper::getCacheImpl(cache);
  cacheImpl->createRegion(regionName.c_str(), regAttrs, rootRegPtr);
  fillIn(rootRegPtr);
}

void RegionXmlCreation::create(RegionPtr parent) {
  GF_D_ASSERT(!(this->isRoot));
  RegionPtr subRegPtr = nullptr;

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

RegionXmlCreation::~RegionXmlCreation() {
  std::vector<RegionXmlCreation*>::iterator start = subRegions.begin();
  while (start != subRegions.end()) {
    delete *start;
    *start = nullptr;
    ++start;
  }
}

std::string RegionXmlCreation::getAttrId() const { return attrId; }

void RegionXmlCreation::setAttrId(const std::string& pattrId) {
  this->attrId = pattrId;
}
