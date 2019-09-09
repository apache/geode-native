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

#include "ClientProxyMembershipIDFactory.hpp"

#include <algorithm>
#include <iterator>
#include <random>

#include <boost/process/environment.hpp>

#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

ClientProxyMembershipIDFactory::ClientProxyMembershipIDFactory(
    std::string dsName)
    : dsName(dsName) {
  static const auto alphabet =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
  static const auto numChars = (sizeof(alphabet) / sizeof(char)) - 2;

  std::random_device rd;
  std::default_random_engine rng(rd());
  std::uniform_int_distribution<> dist(0, numChars);

  randString.reserve(7 + 10 + 15);
  randString.append("Native_");
  std::generate_n(std::back_inserter(randString), 10,
                  [&]() { return alphabet[dist(rng)]; });

  auto pid = boost::this_process::get_id();
  randString.append(std::to_string(pid));

  LOGINFO("Using %s as random data for ClientProxyMembershipID",
          randString.c_str());
}

std::unique_ptr<ClientProxyMembershipID> ClientProxyMembershipIDFactory::create(
    const char* hostname, const ACE_INET_Addr& address, uint32_t hostPort,
    const char* durableClientId,
    const std::chrono::seconds durableClntTimeOut) {
  return std::unique_ptr<ClientProxyMembershipID>(new ClientProxyMembershipID(
      dsName, randString, hostname, address, hostPort, durableClientId,
      durableClntTimeOut));
}

}  // namespace client
}  // namespace geode
}  // namespace apache
