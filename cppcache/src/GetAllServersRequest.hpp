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

#ifndef GEODE_GETALLSERVERSREQUEST_H_
#define GEODE_GETALLSERVERSREQUEST_H_

#include <geode/Serializable.hpp>
#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>
#include <geode/CacheableString.hpp>
#include "GeodeTypeIdsImpl.hpp"
#include <string>

namespace apache {
namespace geode {
namespace client {

class GetAllServersRequest
    : public DataSerializableFixedId_t<GeodeTypeIdsImpl::GetAllServersRequest> {
  std::shared_ptr<CacheableString> m_serverGroup;

 public:
  GetAllServersRequest(const std::string& serverGroup) : Serializable() {
    m_serverGroup = CacheableString::create(serverGroup);
  }
  void toData(DataOutput& output) const override;
  void fromData(DataInput& input) override;

  size_t objectSize() const override {
    return sizeof(GetAllServersRequest) + m_serverGroup->objectSize();
  }
  ~GetAllServersRequest() override = default;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_GETALLSERVERSREQUEST_H_
