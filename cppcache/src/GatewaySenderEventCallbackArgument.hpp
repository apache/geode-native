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

#ifndef GEODE_GATEWAYSENDEREVENTCALLBACKARGUMENT_H_
#define GEODE_GATEWAYSENDEREVENTCALLBACKARGUMENT_H_

#include <geode/DataInput.hpp>
#include <geode/internal/DSFixedId.hpp>
#include <geode/internal/DataSerializableFixedId.hpp>

namespace apache {
namespace geode {
namespace client {

using internal::DSFid;

class GatewaySenderEventCallbackArgument
    : public internal::DataSerializableFixedId {
 public:
  GatewaySenderEventCallbackArgument()
      : originatingDSId(GwSenderDefaultDistributedSystemId) {}
  void fromData(DataInput& input) override;
  void toData(DataOutput&) const final {}
  DSFid getDSFID() const override;
  int getOriginatingDSId() { return originatingDSId; }
  std::vector<int> getRecipientDSIds() { return recipientDSIds; }
  static std::shared_ptr<Serializable> create() {
    return std::make_shared<GatewaySenderEventCallbackArgument>();
  }
  ~GatewaySenderEventCallbackArgument() override = default;

 private:
  const int GwSenderDefaultDistributedSystemId = -1;
  int originatingDSId;
  std::vector<int> recipientDSIds;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_GATEWAYSENDEREVENTCALLBACKARGUMENT_H_
