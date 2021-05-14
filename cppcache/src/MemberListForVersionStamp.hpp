#pragma once

#ifndef GEODE_MEMBERLISTFORVERSIONSTAMP_H_
#define GEODE_MEMBERLISTFORVERSIONSTAMP_H_

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

#include <memory>
#include <unordered_map>

#include <boost/thread/shared_mutex.hpp>

#include <geode/internal/geode_globals.hpp>

#include "DSMemberForVersionStamp.hpp"
namespace apache {
namespace geode {
namespace client {
struct DistributedMemberWithIntIdentifier {
 public:
  explicit DistributedMemberWithIntIdentifier(
      std::shared_ptr<DSMemberForVersionStamp> dsmember = nullptr,
      uint16_t id = 0) {
    this->m_member = dsmember;
    this->m_identifier = id;
  }
  std::shared_ptr<DSMemberForVersionStamp> m_member;
  uint16_t m_identifier;
};

class MemberListForVersionStamp {
 public:
  MemberListForVersionStamp();
  virtual ~MemberListForVersionStamp();
  uint16_t add(std::shared_ptr<DSMemberForVersionStamp> member);
  std::shared_ptr<DSMemberForVersionStamp> getDSMember(uint16_t memberId);

 private:
  std::unordered_map<uint32_t, std::shared_ptr<DSMemberForVersionStamp>>
      m_members1;
  std::unordered_map<std::string, DistributedMemberWithIntIdentifier>
      m_members2;

  boost::shared_mutex mutex_;
  uint32_t m_memberCounter;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_MEMBERLISTFORVERSIONSTAMP_H_
