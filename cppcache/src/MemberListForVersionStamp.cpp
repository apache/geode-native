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
#include "MemberListForVersionStamp.hpp"

#include <boost/thread/lock_types.hpp>

#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

MemberListForVersionStamp::MemberListForVersionStamp() { m_memberCounter = 0; }

MemberListForVersionStamp::~MemberListForVersionStamp() {}

// Two hash maps are needed in this class as we have two primary keys on which
// we want a search - integer counter and the hash key of the member.
// Add function searches whether the member is already added to the hash maps.
// If yes, return the integer counter. If not, add it to both the hash maps.
// This function is protected  using readers/writer lock
uint16_t MemberListForVersionStamp::add(
    std::shared_ptr<DSMemberForVersionStamp> member) {
  boost::unique_lock<decltype(mutex_)> guard(mutex_);
  std::unordered_map<std::string, DistributedMemberWithIntIdentifier>::iterator
      it = m_members2.find(member->getHashKey());
  if (it != m_members2.end()) return (*it).second.m_identifier;
  DistributedMemberWithIntIdentifier dmwithIntId(member, ++m_memberCounter);
  m_members1[m_memberCounter] = member;
  m_members2[member->getHashKey()] = dmwithIntId;
  LOGDEBUG(
      "Adding a new member to the member list maintained for version stamps "
      "member Ids. HashKey: %s MemberCounter: %d",
      member->getHashKey().c_str(), m_memberCounter);
  return m_memberCounter;
}

// This function is protected  using readers/writer lock
std::shared_ptr<DSMemberForVersionStamp> MemberListForVersionStamp::getDSMember(
    uint16_t memberId) {
  boost::shared_lock<decltype(mutex_)> guard{mutex_};
  std::unordered_map<uint32_t,
                     std::shared_ptr<DSMemberForVersionStamp>>::iterator it =
      m_members1.find(memberId);
  if (it != m_members1.end()) return (*it).second;
  return nullptr;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
