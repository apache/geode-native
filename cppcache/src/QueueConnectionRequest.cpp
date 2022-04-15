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

#include "QueueConnectionRequest.hpp"

#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>

namespace apache {
namespace geode {
namespace client {

void QueueConnectionRequest::toData(DataOutput& output) const {
  output.writeString(m_serverGp);
  output.write(static_cast<int8_t>(DSCode::FixedIDByte));
  output.write(static_cast<int8_t>(DSFid::ClientProxyMembershipId));
  const auto& dsMemberId = m_membershipID.getDSMemberId();
  output.writeBytes(reinterpret_cast<const uint8_t*>(dsMemberId.c_str()),
                    static_cast<int32_t>(dsMemberId.size()));
  output.writeInt(static_cast<int32_t>(1));
  output.writeInt(static_cast<int32_t>(m_redundantCopies));
  writeSetOfServerLocation(output);
  output.writeBoolean(m_findDurable);
}

DSFid QueueConnectionRequest::getDSFID() const {
  return DSFid::QueueConnectionRequest;
}

std::set<ServerLocation> QueueConnectionRequest::getExcludedServer() const {
  return m_excludedServers;
}

const ClientProxyMembershipID& QueueConnectionRequest::getProxyMemberShipId()
    const {
  return m_membershipID;
}

int QueueConnectionRequest::getRedundentCopies() const {
  return m_redundantCopies;
}

bool QueueConnectionRequest::isFindDurable() const { return m_findDurable; }

void QueueConnectionRequest::writeSetOfServerLocation(
    DataOutput& output) const {
  output.writeInt(static_cast<int32_t>(m_excludedServers.size()));
  for (const auto& server : m_excludedServers) {
    server.toData(output);
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
