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

#include "TXCommitMessage.hpp"

#include <algorithm>

#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>

#include "ClientProxyMembershipID.hpp"
#include "FarSideEntryOp.hpp"
#include "util/exception.hpp"

namespace apache {
namespace geode {
namespace client {

TXCommitMessage::TXCommitMessage(
    MemberListForVersionStamp& memberListForVersionStamp)
    : memberListForVersionStamp_(memberListForVersionStamp) {}

void TXCommitMessage::fromData(DataInput& input) {
  // read and ignore pId
  input.readInt32();

  // read and ignore txIdent
  input.readInt32();
  ClientProxyMembershipID memId;
  memId.fromData(input);

  if (input.readBoolean()) {
    // read and ignore lockId
    memId.fromData(input);
    input.readInt32();
  }
  // read and ignore totalMaxSize
  input.readInt32();

  // ignore farsideBaseMembershipId
  const auto ignoreLength = input.readArrayLength();
  if (ignoreLength > 0) {
    input.advanceCursor(ignoreLength);
  }

  input.readInt64();  // ignore tid
  input.readInt64();  // ignore seqId

  input.readBoolean();  // ignore needsLargeModCount
  input.readBoolean();  // ignore shadow keys flag

  const auto regionSize = input.readInt32();
  for (int32_t i = 0; i < regionSize; i++) {
    auto rc = std::make_shared<RegionCommit>(memberListForVersionStamp_);
    rc->fromData(input);
    regions_.push_back(rc);
  }

  const auto dsCode = static_cast<const DSCode>(input.read());
  if (dsCode == DSCode::FixedIDByte) {
    const auto fixedId = static_cast<const DSFid>(input.read());
    if (fixedId == DSFid::ClientProxyMembershipId) {
      ClientProxyMembershipID memId1;

      input.advanceCursor(input.readArrayLength());

      input.readInt32();
    } else {
      LOGERROR("TXCommitMessage::fromData Unexpected type id: %" PRId8
               "while deserializing commit response",
               fixedId);
      GfErrTypeThrowException(
          "TXCommitMessage::fromData Unable to handle commit response",
          GF_CACHE_ILLEGAL_STATE_EXCEPTION);
    }
  } else if (dsCode != DSCode::NullObj) {
    LOGERROR("TXCommitMessage::fromData Unexpected type id: %" PRId8
             "while deserializing commit response",
             dsCode);
    GfErrTypeThrowException(
        "TXCommitMessage::fromData Unable to handle commit response",
        GF_CACHE_ILLEGAL_STATE_EXCEPTION);
  }

  const auto len = input.readArrayLength();
  for (int32_t j = 0; j < len; j++) {
    std::shared_ptr<Cacheable> tmp;
    input.readObject(tmp);
  }
}

void TXCommitMessage::toData(DataOutput&) const {}

std::shared_ptr<Serializable> TXCommitMessage::create(
    MemberListForVersionStamp& memberListForVersionStamp) {
  return std::make_shared<TXCommitMessage>(memberListForVersionStamp);
}

void TXCommitMessage::apply(Cache* cache) {
  for (const auto& region : regions_) {
    region->apply(cache);
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
