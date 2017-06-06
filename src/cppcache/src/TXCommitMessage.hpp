#pragma once

#ifndef GEODE_TXCOMMITMESSAGE_H_
#define GEODE_TXCOMMITMESSAGE_H_

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

#include <geode/geode_globals.hpp>
#include <geode/geode_types.hpp>
#include <geode/DataInput.hpp>
#include "RegionCommit.hpp"

namespace apache {
namespace geode {
namespace client {
_GF_PTR_DEF_(TXCommitMessage, TXCommitMessagePtr);

class TXCommitMessage : public apache::geode::client::Cacheable {
 public:
  TXCommitMessage(MemberListForVersionStamp & memberListForVersionStamp);
  virtual ~TXCommitMessage();

  virtual Serializable* fromData(DataInput& input);
  virtual void toData(DataOutput& output) const;
  virtual int32_t classId() const;
  int8_t typeId() const;
  static Serializable* create(MemberListForVersionStamp & memberListForVersionStamp);
  //	VectorOfEntryEvent getEvents(Cache* cache);

  void apply(Cache* cache);

 private:
  // UNUSED int32_t m_processorId;
  bool isAckRequired();
  MemberListForVersionStamp & m_memberListForVersionStamp;
  std::vector<RegionCommitPtr> m_regions;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TXCOMMITMESSAGE_H_
