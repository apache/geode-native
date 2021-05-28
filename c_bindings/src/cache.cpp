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
#include <cstdint>

// C++ client public headers
#include "geode/RegionShortcut.hpp"

// C++ client private headers
#include "util/Log.hpp"

// C client public headers
#include "geode/cache.h"

// C client private headers
#include "cache.hpp"
#include "pool/manager.hpp"
#include "region/factory.hpp"
#include "cache/factory.hpp"

void apache_geode_DestroyCache(apache_geode_cache_t* cache) {
  CacheWrapper* cacheWrapper = reinterpret_cast<CacheWrapper*>(cache);
  LOGDEBUG("%s: destroying cache %p", __FUNCTION__, cache);
  delete cacheWrapper;
}

bool apache_geode_Cache_GetPdxIgnoreUnreadFields(apache_geode_cache_t* cache) {
  LOGDEBUG("%s: cache=%p", __FUNCTION__, cache);
  CacheWrapper* cacheWrapper = reinterpret_cast<CacheWrapper*>(cache);
  return cacheWrapper->getPdxIgnoreUnreadFields();
}

bool apache_geode_Cache_GetPdxReadSerialized(apache_geode_cache_t* cache) {
  LOGDEBUG("%s: cache=%p", __FUNCTION__, cache);
  CacheWrapper* cacheWrapper = reinterpret_cast<CacheWrapper*>(cache);
  return cacheWrapper->getPdxReadSerialized();
}

apache_geode_pool_manager_t* apache_geode_Cache_GetPoolManager(
    apache_geode_cache_t* cache) {
  LOGDEBUG("%s: cache=%p", __FUNCTION__, cache);
  CacheWrapper* cacheWrapper = reinterpret_cast<CacheWrapper*>(cache);
  return reinterpret_cast<apache_geode_pool_manager_t*>(
      cacheWrapper->getPoolManager());
}

apache_geode_region_factory_t* apache_geode_Cache_CreateRegionFactory(
    apache_geode_cache_t* cache, std::int32_t regionType) {
  LOGDEBUG("%s: cache=%p", __FUNCTION__, cache);
  CacheWrapper* cacheWrapper = reinterpret_cast<CacheWrapper*>(cache);
  apache::geode::client::RegionShortcut regionShortcut =
      static_cast<apache::geode::client::RegionShortcut>(regionType);
  return reinterpret_cast<apache_geode_region_factory_t*>(
      cacheWrapper->createRegionFactory(regionShortcut));
}

const char* apache_geode_Cache_GetName(apache_geode_cache_t* cache) {
  LOGDEBUG("%s: cache=%p", __FUNCTION__, cache);
  CacheWrapper* cacheWrapper = reinterpret_cast<CacheWrapper*>(cache);
  return cacheWrapper->getName();
}

void apache_geode_Cache_Close(apache_geode_cache_t* cache, bool keepalive) {
  LOGDEBUG("%s: cache=%p", __FUNCTION__, cache);
  CacheWrapper* cacheWrapper = reinterpret_cast<CacheWrapper*>(cache);
  cacheWrapper->close(keepalive);
}

bool apache_geode_Cache_IsClosed(apache_geode_cache_t* cache) {
  LOGDEBUG("%s: cache=%p", __FUNCTION__, cache);
  CacheWrapper* cacheWrapper = reinterpret_cast<CacheWrapper*>(cache);
  return cacheWrapper->isClosed();
}

CacheWrapper::CacheWrapper(apache::geode::client::Cache cache)
    : cache_(std::move(cache)) {
      AddRecord(this, "CacheWrapper");
    }
    
CacheWrapper::~CacheWrapper() {
  RemoveRecord(this);
}

bool CacheWrapper::getPdxIgnoreUnreadFields() {
  return cache_.getPdxIgnoreUnreadFields();
}

bool CacheWrapper::getPdxReadSerialized() {
  return cache_.getPdxReadSerialized();
}

PoolManagerWrapper* CacheWrapper::getPoolManager() {
  return new PoolManagerWrapper(this, cache_.getPoolManager());
}

RegionFactoryWrapper* CacheWrapper::createRegionFactory(
    apache::geode::client::RegionShortcut regionShortcut) {
  return new RegionFactoryWrapper(this, cache_.createRegionFactory(regionShortcut));
}

const char* CacheWrapper::getName() { return cache_.getName().c_str(); }

void CacheWrapper::close(bool keepalive) { cache_.close(keepalive); }

bool CacheWrapper::isClosed() { return cache_.isClosed(); }
