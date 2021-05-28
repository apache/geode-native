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

// Standard headers
#include <string>
#include <utility>

// C++ client public headers
#include "geode/RegionFactory.hpp"

// C client public headers
#include "geode/region/factory.h"
#include "geode/region.h"

// C client private headers
#include "region.hpp"
#include "region/factory.hpp"
#include "cache.hpp"

RegionFactoryWrapper::RegionFactoryWrapper(CacheWrapper *cache,
    apache::geode::client::RegionFactory regionFactory)
    : regionFactory_(std::move(regionFactory)) {
      AddRecord(this, "RegionFactoryWrapper");
    }

    RegionFactoryWrapper::~RegionFactoryWrapper() {
      RemoveRecord(this);
    }

void RegionFactoryWrapper::setPoolName(const std::string& poolName) {
  regionFactory_.setPoolName(poolName);
}

RegionWrapper* RegionFactoryWrapper::createRegion(
    const std::string& regionName) {
  return new RegionWrapper(regionFactory_.create(regionName));
}

void apache_geode_DestroyRegionFactory(
    apache_geode_region_factory_t* regionFactory) {
  delete reinterpret_cast<RegionFactoryWrapper*>(regionFactory);
}

void apache_geode_RegionFactory_SetPoolName(
    apache_geode_region_factory_t* regionFactory, const char* poolName) {
  RegionFactoryWrapper* regionFactoryWrapper =
      reinterpret_cast<RegionFactoryWrapper*>(regionFactory);
  regionFactoryWrapper->setPoolName(poolName);
}
apache_geode_region_t* apache_geode_RegionFactory_CreateRegion(
    apache_geode_region_factory_t* regionFactory, const char* regionName) {
  RegionFactoryWrapper* regionFactoryWrapper =
      reinterpret_cast<RegionFactoryWrapper*>(regionFactory);
  return reinterpret_cast<apache_geode_region_t*>(
      regionFactoryWrapper->createRegion(regionName));
}
