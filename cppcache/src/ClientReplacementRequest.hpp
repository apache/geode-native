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

#ifndef GEODE_CLIENTREPLACEMENTREQUEST_H_
#define GEODE_CLIENTREPLACEMENTREQUEST_H_

#include <set>
#include <string>

#include <geode/internal/DataSerializableFixedId.hpp>

#include "ClientConnectionRequest.hpp"
#include "ServerLocation.hpp"
#include "ServerLocationRequest.hpp"

namespace apache {
namespace geode {
namespace client {

class ClientReplacementRequest : public ClientConnectionRequest {
 public:
  ClientReplacementRequest(const std::string& serverName,
                           const std::set<ServerLocation>& excludeServergroup,
                           std::string servergroup = "")
      : ClientConnectionRequest(excludeServergroup, servergroup),
        m_serverLocation(ServerLocation(serverName)) {}

  ~ClientReplacementRequest() override = default;

  void toData(DataOutput& output) const override;
  internal::DSFid getDSFID() const override;

 private:
  const ServerLocation m_serverLocation;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CLIENTREPLACEMENTREQUEST_H_
