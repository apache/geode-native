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

#ifndef GEODE_CLIENTPROXYMEMBERSHIPIDFACTORY_H_
#define GEODE_CLIENTPROXYMEMBERSHIPIDFACTORY_H_

#include <string>

#include "ClientProxyMembershipID.hpp"

namespace apache {
namespace geode {
namespace client {

class ClientProxyMembershipIDFactory {
 public:
  ClientProxyMembershipIDFactory(std::string dsName);

  std::unique_ptr<ClientProxyMembershipID> create(
      const char* hostname, uint32_t hostAddr, uint32_t hostPort,
      const char* durableClientId = nullptr,
      const uint32_t durableClntTimeOut = 0);

 private:
  std::string dsName;
  std::string randString;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CLIENTPROXYMEMBERSHIPID_H_
