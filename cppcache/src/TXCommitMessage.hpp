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

#ifndef GEODE_TXCOMMITMESSAGE_H_
#define GEODE_TXCOMMITMESSAGE_H_

#include <geode/DataInput.hpp>
#include <geode/internal/geode_globals.hpp>

#include "RegionCommit.hpp"

namespace apache {
namespace geode {
namespace client {

class TXCommitMessage
    : public internal::DataSerializableFixedId_t<DSFid::TXCommitMessage> {
 public:
  explicit TXCommitMessage(
      MemberListForVersionStamp& memberListForVersionStamp);
  ~TXCommitMessage() override = default;

  void fromData(DataInput& input) override;
  void toData(DataOutput& output) const override;

  static std::shared_ptr<Serializable> create(
      MemberListForVersionStamp& memberListForVersionStamp);
  //	VectorOfEntryEvent getEvents(Cache* cache);

  void apply(Cache* cache);

 private:
  // UNUSED int32_t m_processorId;
  bool isAckRequired();
  MemberListForVersionStamp& m_memberListForVersionStamp;
  std::vector<std::shared_ptr<RegionCommit>> m_regions;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TXCOMMITMESSAGE_H_
