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

#ifndef GEODE_GETALLSERVERSRESPONSE_H_
#define GEODE_GETALLSERVERSRESPONSE_H_

#include <vector>

#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>
#include <geode/Serializable.hpp>
#include <geode/internal/DataSerializableFixedId.hpp>

#include "ServerLocation.hpp"

namespace apache {
namespace geode {
namespace client {

class GetAllServersResponse : public internal::DataSerializableFixedId_t<
                                  internal::DSFid::GetAllServersResponse> {
  std::vector<std::shared_ptr<ServerLocation> > m_servers;

 public:
  static std::shared_ptr<Serializable> create() {
    return std::make_shared<GetAllServersResponse>();
  }
  GetAllServersResponse() : Serializable() {}
  GetAllServersResponse(std::vector<std::shared_ptr<ServerLocation> > servers)
      : Serializable() {
    m_servers = servers;
  }
  void toData(DataOutput& output) const override;
  void fromData(DataInput& input) override;

  size_t objectSize() const override {
    return sizeof(GetAllServersResponse) + m_servers.capacity();
  }
  std::vector<std::shared_ptr<ServerLocation> > getServers() {
    return m_servers;
  }
  ~GetAllServersResponse() override = default;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_GETALLSERVERSRESPONSE_H_
