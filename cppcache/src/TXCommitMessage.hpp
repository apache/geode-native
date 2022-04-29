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

#include <memory>
#include <vector>

#include <geode/internal/DataSerializableFixedId.hpp>

#include "RegionCommit.hpp"

namespace apache {
namespace geode {
namespace client {

class MemberListForVersionStamp;
class DataInput;
class DataOutput;
class Cache;

class TXCommitMessage : public internal::DataSerializableFixedId_t<
                            internal::DSFid::TXCommitMessage> {
 public:
  explicit TXCommitMessage(
      MemberListForVersionStamp& memberListForVersionStamp);
  ~TXCommitMessage() noexcept override = default;

  void fromData(DataInput& input) override;
  void toData(DataOutput& output) const override;

  static std::shared_ptr<Serializable> create(
      MemberListForVersionStamp& memberListForVersionStamp);

  void apply(Cache* cache);

 private:
  MemberListForVersionStamp& memberListForVersionStamp_;
  std::vector<std::shared_ptr<RegionCommit>> regions_;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TXCOMMITMESSAGE_H_
