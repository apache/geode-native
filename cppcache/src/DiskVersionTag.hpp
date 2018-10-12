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

#pragma once

#ifndef GEODE_DISKVERSIONTAG_H_
#define GEODE_DISKVERSIONTAG_H_

#include "DiskStoreId.hpp"
#include "VersionTag.hpp"

namespace apache {
namespace geode {
namespace client {

class RegionInternal;
class CacheImpl;

class DiskVersionTag : public VersionTag {
 protected:
  void readMembers(uint16_t flags, DataInput& input) override {
    if ((flags & HAS_MEMBER_ID) != 0) {
      auto internalMemId = std::make_shared<DiskStoreId>();
      internalMemId->fromData(input);
      m_internalMemId = m_memberListForVersionStamp.add(internalMemId);
    }

    if ((flags & HAS_PREVIOUS_MEMBER_ID) != 0) {
      if ((flags & DUPLICATE_MEMBER_IDS) != 0) {
        m_previousMemId = m_internalMemId;
      } else {
        auto previousMemId = std::make_shared<DiskStoreId>();
        previousMemId->fromData(input);
        m_previousMemId = m_memberListForVersionStamp.add(previousMemId);
      }
    }
  }

 public:
  explicit DiskVersionTag(MemberListForVersionStamp& memberListForVersionStamp)
      : VersionTag(memberListForVersionStamp) {}

  DSFid getDSFID() const override { return DSFid::DiskVersionTag; }

  static std::shared_ptr<Serializable> createDeserializable(
      MemberListForVersionStamp& memberListForVersionStamp) {
    return std::make_shared<DiskVersionTag>(memberListForVersionStamp);
  }

  /**
   * for internal testing
   */
  DiskVersionTag(int32_t entryVersion, int16_t regionVersionHighBytes,
                 int32_t regionVersionLowBytes, uint16_t internalMemId,
                 uint16_t previousMemId,
                 MemberListForVersionStamp& memberListForVersionStamp)
      : VersionTag(entryVersion, regionVersionHighBytes, regionVersionLowBytes,
                   internalMemId, previousMemId, memberListForVersionStamp) {}
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_DISKVERSIONTAG_H_
