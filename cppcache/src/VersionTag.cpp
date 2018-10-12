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
#include "VersionTag.hpp"

#include "CacheImpl.hpp"
#include "ClientProxyMembershipID.hpp"
#include "MemberListForVersionStamp.hpp"
#include "RegionInternal.hpp"

namespace apache {
namespace geode {
namespace client {

VersionTag::VersionTag(MemberListForVersionStamp& memberListForVersionStamp)
    : VersionTag(0, 0, 0, 0, 0, memberListForVersionStamp) {}

VersionTag::VersionTag(int32_t entryVersion, int16_t regionVersionHighBytes,
                       int32_t regionVersionLowBytes, uint16_t internalMemId,
                       uint16_t previousMemId,
                       MemberListForVersionStamp& memberListForVersionStamp)
    : m_bits(0),
      m_entryVersion(entryVersion),
      m_regionVersionHighBytes(regionVersionHighBytes),
      m_regionVersionLowBytes(regionVersionLowBytes),
      m_internalMemId(internalMemId),
      m_previousMemId(previousMemId),
      m_timeStamp(0),
      m_memberListForVersionStamp(memberListForVersionStamp) {}

void VersionTag::toData(DataOutput&) const {
  throw IllegalStateException("VersionTag::toData not implemented");
}

void VersionTag::fromData(DataInput& input) {
  uint16_t flags = input.readInt16();
  m_bits = input.readInt16();
  input.read();  // skip distributedSystemId
  if ((flags & VERSION_TWO_BYTES) != 0) {
    m_entryVersion = input.readInt16();
    m_entryVersion &= 0xffff;
  } else {
    m_entryVersion = input.readInt32();
    m_entryVersion &= 0xffffffff;
  }
  if ((flags & HAS_RVV_HIGH_BYTE) != 0) {
    m_regionVersionHighBytes = input.readInt16();
  }
  m_regionVersionLowBytes = input.readInt32();
  m_timeStamp = input.readUnsignedVL();
  readMembers(flags, input);
}

std::shared_ptr<Serializable> VersionTag::createDeserializable(
    MemberListForVersionStamp& memberListForVersionStamp) {
  return std::make_shared<VersionTag>(memberListForVersionStamp);
}

void VersionTag::replaceNullMemberId(uint16_t memId) {
  if (m_previousMemId == 0) {
    m_previousMemId = memId;
  }
  if (m_internalMemId == 0) {
    m_internalMemId = memId;
  }
}
void VersionTag::readMembers(uint16_t flags, DataInput& input) {
  if ((flags & HAS_MEMBER_ID) != 0) {
    auto internalMemId = std::make_shared<ClientProxyMembershipID>();
    internalMemId->readEssentialData(input);
    m_internalMemId = m_memberListForVersionStamp.add(
        std::dynamic_pointer_cast<DSMemberForVersionStamp>(internalMemId));
  }
  if ((flags & HAS_PREVIOUS_MEMBER_ID) != 0) {
    if ((flags & DUPLICATE_MEMBER_IDS) != 0) {
      m_previousMemId = m_internalMemId;
    } else {
      auto previousMemId = std::make_shared<ClientProxyMembershipID>();
      previousMemId->readEssentialData(input);
      m_previousMemId = m_memberListForVersionStamp.add(previousMemId);
    }
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
