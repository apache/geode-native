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

#ifndef GEODE_CLIENTCONNECTIONREQUEST_H_
#define GEODE_CLIENTCONNECTIONREQUEST_H_

#include "ServerLocationRequest.hpp"
#include "TcrEndpoint.hpp"
#include <string>
#include <set>
#include "ServerLocation.hpp"

namespace apache {
namespace geode {
namespace client {

class ClientConnectionRequest : public ServerLocationRequest {
 public:
  ClientConnectionRequest(const std::set<ServerLocation>& excludeServergroup,
                          std::string servergroup = "")
      : ServerLocationRequest(),
        m_servergroup(servergroup),
        m_excludeServergroup_serverLocation(excludeServergroup) {}
  void toData(DataOutput& output) const override;
  int32_t getDSFID() const override;
  std::string getServerGroup() const { return m_servergroup; }
  const std::set<ServerLocation>& getExcludedServerGroup() const {
    return m_excludeServergroup_serverLocation;
  }
  ~ClientConnectionRequest() override = default;

 private:
  void writeSetOfServerLocation(DataOutput& output) const;
  std::string m_servergroup;
  const std::set<ServerLocation>& m_excludeServergroup_serverLocation;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CLIENTCONNECTIONREQUEST_H_
