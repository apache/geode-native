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

#ifndef GEODE_CLIENTPROXYMEMBERSHIPID_H_
#define GEODE_CLIENTPROXYMEMBERSHIPID_H_

#include <memory>
#include <string>
#include <vector>

#include <boost/asio.hpp>

#include <geode/internal/functional.hpp>
#include <geode/internal/geode_globals.hpp>

#include "DSMemberForVersionStamp.hpp"

namespace apache {
namespace geode {
namespace client {

class ClientProxyMembershipID : public DSMemberForVersionStamp {
 public:
  const std::string& getDSMemberId() const;

  ClientProxyMembershipID(std::string dsName, std::string randString,
                          const std::string& hostname,
                          const boost::asio::ip::address& address,
                          uint32_t hostPort, const std::string& durableClientId,
                          const std::chrono::seconds durableClientTimeOut =
                              std::chrono::seconds::zero());

  /**
   * This constructor is only for testing and should not be used for any
   * other purpose. See testEntriesMapForVersioning.cpp for more details
   */
  ClientProxyMembershipID(const uint8_t* hostAddr, uint32_t hostAddrLen,
                          uint32_t hostPort, const char* dsname,
                          const char* uniqueTag, uint32_t vmViewId);

  ClientProxyMembershipID();

  ~ClientProxyMembershipID() noexcept override;

  static void increaseSyncCounter();

  static std::shared_ptr<Serializable> createDeserializable() {
    return std::make_shared<ClientProxyMembershipID>();
  }

  const std::string& getClientId();

  void toData(DataOutput& output) const override;

  void fromData(DataInput& input) override;

  internal::DSFid getDSFID() const override {
    return internal::DSFid::InternalDistributedMember;
  }

  size_t objectSize() const override { return 0; }

  void initHostAddressVector(const boost::asio::ip::address& address);

  void initHostAddressVector(const uint8_t* hostAddr, uint32_t hostAddrLen);

  void initObjectVars(const std::string& hostname, uint32_t hostPort,
                      const std::string& durableClientId,
                      const std::chrono::seconds durableClntTimeOut,
                      int32_t dcPort, int32_t vPID, int8_t vmkind,
                      int8_t splitBrainFlag, const char* dsname,
                      const char* uniqueTag, uint32_t vmViewId);

  std::string getDSName() const { return dsName_; }

  std::string getUniqueTag() const { return uniqueTag_; }

  const std::vector<uint8_t>& getHostAddr() const { return hostAddr_; }

  uint32_t getHostAddrLen() const {
    return static_cast<uint32_t>(hostAddr_.size());
  }

  uint32_t getHostPort() const { return hostPort_; }

  std::string getHashKey() override;

  int16_t compareTo(const DSMemberForVersionStamp&) const override;

  int32_t hashcode() const override;

  bool operator==(const CacheableKey& other) const override {
    return (this->compareTo(
                dynamic_cast<const DSMemberForVersionStamp&>(other)) == 0);
  }

  void readEssentialData(DataInput& input);

 private:
  std::string memIdStr_;
  std::string clientId_;

  std::string dsName_;
  uint32_t hostPort_;
  std::vector<uint8_t> hostAddr_;

  std::string uniqueTag_;
  std::string hashKey_;
  uint32_t vmViewId_;

  void readVersion(int32_t flags, DataInput& input);
  void readAdditionalData(DataInput& input);
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CLIENTPROXYMEMBERSHIPID_H_
