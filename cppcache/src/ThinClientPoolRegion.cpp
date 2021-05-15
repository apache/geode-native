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
/*
 * ThinClientPoolRegion.cpp
 *
 *  Created on: Nov 20, 2008
 *      Author: abhaware
 */

#include "ThinClientPoolRegion.hpp"

#include <geode/PoolManager.hpp>
#include <geode/SystemProperties.hpp>

#include "CacheImpl.hpp"
#include "ThinClientPoolDM.hpp"

namespace apache {
namespace geode {
namespace client {

ThinClientPoolRegion::ThinClientPoolRegion(
    const std::string& name, CacheImpl* cache,
    const std::shared_ptr<RegionInternal>& rPtr, RegionAttributes attributes,
    const std::shared_ptr<CacheStatistics>& stats, bool shared)
    : ThinClientRegion(name, cache, rPtr, attributes, stats, shared) {}

void ThinClientPoolRegion::initTCR() {
  try {
    auto poolDM = std::dynamic_pointer_cast<ThinClientPoolDM>(
        getCache().getPoolManager().find(m_regionAttributes.getPoolName()));
    m_tcrdm = poolDM;
    if (!m_tcrdm) {
      throw IllegalStateException("pool not found");
    }
    poolDM->incRegionCount();
  } catch (const Exception& ex) {
    LOG_ERROR("Failed to initialize region due to %s: %s", ex.getName().c_str(),
              ex.what());
    throw;
  }
}

void ThinClientPoolRegion::destroyDM(bool) {
  std::dynamic_pointer_cast<ThinClientPoolDM>(m_tcrdm)->decRegionCount();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
