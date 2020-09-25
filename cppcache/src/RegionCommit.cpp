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
 * RegionCommit.cpp
 *
 *  Created on: 23-Feb-2011
 *      Author: ankurs
 */

#include "RegionCommit.hpp"

namespace apache {
namespace geode {
namespace client {

void RegionCommit::fromData(DataInput& input) {
  m_regionPath = std::dynamic_pointer_cast<CacheableString>(input.readObject());
  m_parentRegionPath =
      std::dynamic_pointer_cast<CacheableString>(input.readObject());
  auto size = input.readInt32();
  if (size > 0) {
    const auto largeModCount = input.readBoolean();
    auto dsMember =
        std::dynamic_pointer_cast<DSMemberForVersionStamp>(input.readObject());

    auto memId = m_memberListForVersionStamp.add(dsMember);
    for (int32_t i = 0; i < size; i++) {
      auto entryOp =
          std::make_shared<FarSideEntryOp>(m_memberListForVersionStamp);
      entryOp->fromData(input, largeModCount, memId);
      m_farSideEntryOps.push_back(entryOp);
    }
  }
}

void RegionCommit::apply(Cache* cache) {
  for (auto& entryOp : m_farSideEntryOps) {
    auto region = cache->getRegion(m_regionPath->value().c_str());
    if (region == nullptr && m_parentRegionPath != nullptr) {
      const auto parentRegionPath = m_parentRegionPath->value();
      if (!parentRegionPath.empty()) {
        region = cache->getRegion(parentRegionPath);
      }
    }
    if (region != nullptr) {
      std::static_pointer_cast<FarSideEntryOp>(entryOp)->apply(region);
    }
  }
}

void RegionCommit::fillEvents(
    std::vector<std::shared_ptr<FarSideEntryOp>>& ops) {
  for (auto& entryOp : m_farSideEntryOps) {
    ops.push_back(std::static_pointer_cast<FarSideEntryOp>(entryOp));
  }
}
}  // namespace client
}  // namespace geode
}  // namespace apache
