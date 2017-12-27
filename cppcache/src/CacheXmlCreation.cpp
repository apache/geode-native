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
#include "CacheXmlCreation.hpp"
#include "CacheImpl.hpp"
#include "PoolAttributes.hpp"

using namespace apache::geode::client;

void CacheXmlCreation::addRootRegion(std::shared_ptr<RegionXmlCreation> root) {
  rootRegions.push_back(root);
}

void CacheXmlCreation::addPool(std::shared_ptr<PoolXmlCreation> pool) {
  pools.push_back(pool);
}

void CacheXmlCreation::create(Cache* cache) {
  m_cache = cache;
  m_cache->m_cacheImpl->setPdxIgnoreUnreadFields(m_pdxIgnoreUnreadFields);
  m_cache->m_cacheImpl->setPdxReadSerialized(m_readPdxSerialized);
  // Create any pools before creating any regions.

  for (const auto& pool : pools) {
    pool->create(*m_cache);
  }

  for (const auto& rootRegion : rootRegions) {
    rootRegion->createRoot(cache);
  }
}

void CacheXmlCreation::setPdxIgnoreUnreadField(bool ignore) {
  // m_cache->m_cacheImpl->setPdxIgnoreUnreadFields(ignore);
  m_pdxIgnoreUnreadFields = ignore;
}

void CacheXmlCreation::setPdxReadSerialized(bool val) {
  // m_cache->m_cacheImpl->setPdxIgnoreUnreadFields(ignore);
  m_readPdxSerialized = val;
}

CacheXmlCreation::CacheXmlCreation()

    : m_cache((Cache*)0) {
  m_pdxIgnoreUnreadFields = false;
  m_readPdxSerialized = false;
}
