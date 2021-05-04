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

#include <boost/asio/ip/host_name.hpp>
#include <boost/process/environment.hpp>

#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

ClientProxyMembershipIDFactory::ClientProxyMembershipIDFactory(
    std::string dsName)
    : dsName_(std::move(dsName)) {
  static const auto alphabet = std::string(
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
  static const auto numChars = static_cast<int>(alphabet.size()) - 2;

  std::random_device rd;
  std::default_random_engine rng(rd());
  std::uniform_int_distribution<> dist(0, numChars);

  randString_.reserve(7 + 10 + 15);
  randString_.append("Native_");
  std::generate_n(std::back_inserter(randString_), 10,
                  [&]() { return alphabet[dist(rng)]; });

  auto pid = boost::this_process::get_id();
  randString_.append(std::to_string(pid));

  LOG_INFO("Using %s as random data for ClientProxyMembershipID",
           randString_.c_str());
}

std::unique_ptr<ClientProxyMembershipID> ClientProxyMembershipIDFactory::create(
    const std::string& durableClientId,
    const std::chrono::seconds durableClntTimeOut) {
  const auto hostname = boost::asio::ip::host_name();
  const ACE_INET_Addr address("", hostname.c_str(), "tcp");
  return std::unique_ptr<ClientProxyMembershipID>(
      new ClientProxyMembershipID(dsName_, randString_, hostname, address, 0,
                                  durableClientId, durableClntTimeOut));
}

}  // namespace client
}  // namespace geode
}  // namespace apache
