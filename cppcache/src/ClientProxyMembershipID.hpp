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

#include <string>

#include <ace/OS.h>

#include <geode/internal/geode_globals.hpp>
#include <geode/DataOutput.hpp>
#include <geode/internal/functional.hpp>

#include "GeodeTypeIdsImpl.hpp"
#include "DSMemberForVersionStamp.hpp"

namespace apache {
namespace geode {
namespace client {

class ClientProxyMembershipID;

class ClientProxyMembershipID : public DSMemberForVersionStamp {
 public:
  const char* getDSMemberId(uint32_t& mesgLength) const;
  const char* getDSMemberIdForCS43(uint32_t& mesgLength) const;

  ClientProxyMembershipID(std::string dsName, std::string randString,
                          const char* hostname, uint32_t hostAddr,
                          uint32_t hostPort,
                          const char* durableClientId = nullptr,
                          const std::chrono::seconds durableClntTimeOut =
                              std::chrono::seconds::zero());

  // This constructor is only for testing and should not be used for any
  // other purpose. See testEntriesMapForVersioning.cpp for more details
  ClientProxyMembershipID(uint8_t* hostAddr, uint32_t hostAddrLen,
                          uint32_t hostPort, const char* dsname,
                          const char* uniqueTag, uint32_t vmViewId);
  // ClientProxyMembershipID(const char *durableClientId = nullptr, const
  // uint32_t durableClntTimeOut = 0);
  ClientProxyMembershipID();
  ~ClientProxyMembershipID() noexcept override;
  static void increaseSynchCounter();
  static std::shared_ptr<Serializable> createDeserializable() {
    return std::make_shared<ClientProxyMembershipID>();
  }
  // Do an empty check on the returned value. Only use after handshake is done.
  const std::string& getDSMemberIdForThinClientUse();

  // Serializable interface:
  void toData(DataOutput& output) const override;
  void fromData(DataInput& input) override;
  int32_t getDSFID() const override {
    return GeodeTypeIdsImpl::InternalDistributedMember;
  }
  size_t objectSize() const override { return 0; }

  void initObjectVars(const char* hostname, uint8_t* hostAddr,
                      uint32_t hostAddrLen, bool hostAddrLocalMem,
                      uint32_t hostPort, const char* durableClientId,
                      const std::chrono::seconds durableClntTimeOut,
                      int32_t dcPort, int32_t vPID, int8_t vmkind,
                      int8_t splitBrainFlag, const char* dsname,
                      const char* uniqueTag, uint32_t vmViewId);

  std::string getDSName() const { return m_dsname; }
  std::string getUniqueTag() const { return m_uniqueTag; }
  uint8_t* getHostAddr() const { return m_hostAddr; }
  uint32_t getHostAddrLen() const { return m_hostAddrLen; }
  uint32_t getHostPort() const { return m_hostPort; }
  std::string getHashKey() override;
  int16_t compareTo(const DSMemberForVersionStamp&) const override;
  int32_t hashcode() const override {
    uint32_t result = 0;
    char hostInfo[255] = {0};
    uint32_t offset = 0;
    for (uint32_t i = 0; i < getHostAddrLen(); i++) {
      offset += ACE_OS::snprintf(hostInfo + offset, 255 - offset, ":%x",
                                 m_hostAddr[i]);
    }
    result += geode_hash<std::string>{}(std::string(hostInfo, offset));
    result += m_hostPort;
    return result;
  }

  bool operator==(const CacheableKey& other) const override {
    return (this->compareTo(
                dynamic_cast<const DSMemberForVersionStamp&>(other)) == 0);
  }

  Serializable* readEssentialData(DataInput& input);

 private:
  std::string m_memIDStr;
  std::string m_dsmemIDStr;
  std::string clientID;

  std::string m_dsname;
  uint32_t m_hostPort;
  uint8_t* m_hostAddr;
  uint32_t m_hostAddrLen;
  uint32_t m_hostAddrAsUInt32;
  std::string m_uniqueTag;
  std::string m_hashKey;
  bool m_hostAddrLocalMem;
  uint32_t m_vmViewId;
  static const uint8_t LONER_DM_TYPE = 13;
  static const int VERSION_MASK;
  static const int8_t TOKEN_ORDINAL;

  void readVersion(int flags, DataInput& input);
  void writeVersion(int16_t ordinal, DataOutput& output);

  void readAdditionalData(DataInput& input);
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CLIENTPROXYMEMBERSHIPID_H_
