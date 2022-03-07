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

#include "CacheXmlCreation.hpp"

#include <geode/Cache.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "PoolAttributes.hpp"

namespace apache {
namespace geode {
namespace client {

void CacheXmlCreation::addRootRegion(std::shared_ptr<RegionXmlCreation> root) {
  rootRegions.push_back(root);
}

void CacheXmlCreation::addPool(std::shared_ptr<PoolXmlCreation> pool) {
  pools.push_back(pool);
}

void CacheXmlCreation::create(Cache& cache) {
  // Create any pools before creating any regions.
  std::for_each(std::begin(pools), std::end(pools),
                std::bind(&PoolXmlCreation::create,
                          std::bind(&std::shared_ptr<PoolXmlCreation>::get,
                                    std::placeholders::_1)));

  std::for_each(std::begin(rootRegions), std::end(rootRegions),
                std::bind(&RegionXmlCreation::createRoot,
                          std::bind(&std::shared_ptr<RegionXmlCreation>::get,
                                    std::placeholders::_1),
                          std::ref(*CacheRegionHelper::getCacheImpl(&cache))));
}

CacheXmlCreation::CacheXmlCreation() {}
}  // namespace client
}  // namespace geode
}  // namespace apache
