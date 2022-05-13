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

#include <chrono>
#include <memory>
#include <string>

namespace apache {
namespace geode {
namespace client {

class ClientProxyMembershipID;

class ClientProxyMembershipIDFactory {
 public:
  explicit ClientProxyMembershipIDFactory(std::string dsName);

  std::unique_ptr<ClientProxyMembershipID> create(
      const std::string& durableClientId = nullptr,
      const std::chrono::seconds durableClntTimeOut =
          std::chrono::seconds::zero());

 private:
  std::string dsName_;
  std::string randString_;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CLIENTPROXYMEMBERSHIPID_H_
