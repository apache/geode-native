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
 * TXCommitMessage.cpp
 *
 *  Created on: 21-Feb-2011
 *      Author: ankurs
 */

#include "TXCommitMessage.hpp"
#include <geode/DataOutput.hpp>
#include <algorithm>

#include "ClientProxyMembershipID.hpp"
#include "FarSideEntryOp.hpp"
#include <vector>

namespace apache {
namespace geode {
namespace client {

TXCommitMessage::TXCommitMessage(
    MemberListForVersionStamp& memberListForVersionStamp)
    : m_memberListForVersionStamp(memberListForVersionStamp)
// UNUSED : m_processorId(0)
{}

TXCommitMessage::~TXCommitMessage() {}

bool TXCommitMessage::isAckRequired() { return false; }

void TXCommitMessage::fromData(DataInput& input) {
  int32_t pId = input.readInt32();
  /*
if(isAckRequired()) {
m_processorId = pId;
// ReplyProcessor21.setMessageRPId(m_processorId);
} else {
m_processorId = -1;
}
   */

  int32_t m_txIdent = input.readInt32();
  ClientProxyMembershipID memId;
  memId.fromData(input);

  if (input.readBoolean()) {
    memId.fromData(input);
    int32_t m_lockId = input.readInt32();
  }
  int32_t totalMaxSize = input.readInt32();

  int8_t* m_farsideBaseMembershipId;
  int32_t m_farsideBaseMembershipIdLen;
  input.readBytes(&m_farsideBaseMembershipId, &m_farsideBaseMembershipIdLen);

  if (m_farsideBaseMembershipId != nullptr) {
    GF_SAFE_DELETE_ARRAY(m_farsideBaseMembershipId);
    m_farsideBaseMembershipId = nullptr;
  }

  input.readInt64(); // ignore tid
  input.readInt64(); // ignore seqId

  input.readBoolean(); // ignore needsLargeModCount

  int32_t regionSize = input.readInt32();
  for (int32_t i = 0; i < regionSize; i++) {
    auto rc = std::make_shared<RegionCommit>(m_memberListForVersionStamp);
    rc->fromData(input);
    m_regions.push_back(rc);
  }

  const auto fixedId = input.read();
  if (fixedId == GeodeTypeIdsImpl::FixedIDByte) {
    const auto dfsid = input.read();
    if (dfsid == GeodeTypeIdsImpl::ClientProxyMembershipId) {
      ClientProxyMembershipID memId1;

      input.advanceCursor(input.readArrayLen());

      input.readInt32();
    } else {
      LOGERROR(
          "TXCommitMessage::fromData Unexpected type id: %d while "
          "desirializing commit response",
          dfsid);
      GfErrTypeThrowException(
          "TXCommitMessage::fromData Unable to handle commit response",
          GF_CACHE_ILLEGAL_STATE_EXCEPTION);
    }
  } else if (fixedId != GeodeTypeIds::NullObj) {
    LOGERROR(
        "TXCommitMessage::fromData Unexpected type id: %d while desirializing "
        "commit response",
        fixedId);
    GfErrTypeThrowException(
        "TXCommitMessage::fromData Unable to handle commit response",
        GF_CACHE_ILLEGAL_STATE_EXCEPTION);
  }

  int32_t len = input.readArrayLen();
  for (int j = 0; j < len; j++) {
    CacheablePtr tmp;
    input.readObject(tmp);
  }
}

void TXCommitMessage::toData(DataOutput& output) const {}

int32_t TXCommitMessage::classId() const { return 0; }

int8_t TXCommitMessage::typeId() const {
  return static_cast<int8_t>(GeodeTypeIdsImpl::TXCommitMessage);
}

Serializable* TXCommitMessage::create(
    MemberListForVersionStamp& memberListForVersionStamp) {
  return new TXCommitMessage(memberListForVersionStamp);
}

void TXCommitMessage::apply(Cache* cache) {
  for (std::vector<RegionCommitPtr>::iterator iter = m_regions.begin();
       m_regions.end() != iter; iter++) {
    RegionCommitPtr regionCommit =
        std::static_pointer_cast<RegionCommit>(*iter);
    regionCommit->apply(cache);
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
