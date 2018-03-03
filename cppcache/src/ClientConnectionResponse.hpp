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

#ifndef GEODE_CLIENTCONNECTIONRESPONSE_H_
#define GEODE_CLIENTCONNECTIONRESPONSE_H_

#include "ServerLocationResponse.hpp"
#include "ServerLocation.hpp"
#include <memory>

namespace apache {
namespace geode {
namespace client {

class ClientConnectionResponse : public ServerLocationResponse {
 public:
  ClientConnectionResponse() : ServerLocationResponse(), m_serverFound(false) {}
  void fromData(DataInput& input) override;
  int32_t getDSFID() const override;
  size_t objectSize() const override;
  virtual ServerLocation getServerLocation() const;
  void printInfo() { m_server.printInfo(); }
  static std::shared_ptr<Serializable> create() {
    return std::make_shared<ClientConnectionResponse>();
  }
  ~ClientConnectionResponse() override = default;
  bool serverFound() { return m_serverFound; }

 private:
  bool m_serverFound;
  ServerLocation m_server;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CLIENTCONNECTIONRESPONSE_H_
