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

#include "ClientProxyMembershipID.hpp"

#include <ctime>
#include <iostream>
#include <memory>
#include <string>

#include <boost/process/environment.hpp>

#include <geode/CacheableBuiltins.hpp>

#include "DataOutputInternal.hpp"
#include "DistributedSystem.hpp"
#include "Version.hpp"

#define ADDRSIZE 4
#define DCPORT 12334
#define VMKIND 13
#define ROLEARRLENGTH 0

namespace apache {
namespace geode {
namespace client {

static int synch_counter = 2;

const int ClientProxyMembershipID::VERSION_MASK = 0x8;
const int8_t ClientProxyMembershipID::TOKEN_ORDINAL = -1;

ClientProxyMembershipID::ClientProxyMembershipID()
    : m_hostPort(0),
      m_hostAddr(nullptr),
      m_hostAddrLen(0),
      m_hostAddrLocalMem(false),
      m_vmViewId(0) {}

ClientProxyMembershipID::~ClientProxyMembershipID() noexcept {
  if (m_hostAddrLocalMem) delete[] m_hostAddr;
}

ClientProxyMembershipID::ClientProxyMembershipID(
    std::string dsName, std::string randString, const char* hostname,
    uint32_t hostAddr, uint32_t hostPort, const char* durableClientId,
    const std::chrono::seconds durableClntTimeOut)
    : m_hostAddrAsUInt32(hostAddr) {
  auto vmPID = boost::this_process::get_id();
  initObjectVars(hostname, reinterpret_cast<uint8_t*>(&m_hostAddrAsUInt32), 4,
                 false, hostPort, durableClientId, durableClntTimeOut, DCPORT,
                 vmPID, VMKIND, 0, dsName.c_str(), randString.c_str(), 0);
}

// This is only for unit tests and should not be used for any other purpose. See
// testEntriesMapForVersioning.cpp for more details
ClientProxyMembershipID::ClientProxyMembershipID(
    uint8_t* hostAddr, uint32_t hostAddrLen, uint32_t hostPort,
    const char* dsname, const char* uniqueTag, uint32_t vmViewId) {
  auto vmPID = boost::this_process::get_id();
  initObjectVars("localhost", hostAddr, hostAddrLen, false, hostPort, "",
                 std::chrono::seconds::zero(), DCPORT, vmPID, VMKIND, 0, dsname,
                 uniqueTag, vmViewId);
}
void ClientProxyMembershipID::initObjectVars(
    const char* hostname, uint8_t* hostAddr, uint32_t hostAddrLen,
    bool hostAddrLocalMem, uint32_t hostPort, const char* durableClientId,
    const std::chrono::seconds durableClntTimeOut, int32_t dcPort, int32_t vPID,
    int8_t vmkind, int8_t splitBrainFlag, const char* dsname,
    const char* uniqueTag, uint32_t vmViewId) {
  DataOutputInternal m_memID;
  if (dsname == nullptr) {
    m_dsname = std::string("");
  } else {
    m_dsname = std::string(dsname);
  }
  m_hostPort = hostPort;
  m_hostAddr = hostAddr;
  m_hostAddrLen = hostAddrLen;
  m_hostAddrLocalMem = hostAddrLocalMem;
  if (uniqueTag == nullptr) {
    m_uniqueTag = std::string("");
  } else {
    m_uniqueTag = std::string(uniqueTag);
  }

  m_vmViewId = vmViewId;
  m_memID.write(static_cast<int8_t>(DSCode::FixedIDByte));
  m_memID.write(static_cast<int8_t>(DSCode::InternalDistributedMember));
  m_memID.writeArrayLen(ADDRSIZE);
  // writing first 4 bytes of the address. This will be same until
  // IPV6 support is added in the client
  uint32_t temp;
  memcpy(&temp, hostAddr, 4);
  m_memID.writeInt(static_cast<int32_t>(temp));
  // m_memID.writeInt((int32_t)hostPort);
  m_memID.writeInt(static_cast<int32_t>(synch_counter));
  m_memID.writeString(hostname);
  m_memID.write(splitBrainFlag);  // splitbrain flags

  m_memID.writeInt(dcPort);

  m_memID.writeInt(vPID);
  m_memID.write(vmkind);
  m_memID.writeArrayLen(ROLEARRLENGTH);
  m_memID.writeString(dsname);
  m_memID.writeString(uniqueTag);

  if (durableClientId != nullptr &&
      durableClntTimeOut != std::chrono::seconds::zero()) {
    m_memID.writeString(durableClientId);
    const auto int32ptr = CacheableInt32::create(
        static_cast<int32_t>(durableClntTimeOut.count()));
    int32ptr->toData(m_memID);
  }
  writeVersion(Version::getOrdinal(), m_memID);
  size_t len;
  char* buf =
      reinterpret_cast<char*>(const_cast<uint8_t*>(m_memID.getBuffer(&len)));
  m_memIDStr.append(buf, len);

  clientID.append(hostname);
  clientID.append("(");
  clientID.append(std::to_string(vPID));
  clientID.append(":loner):");
  clientID.append(std::to_string(synch_counter));
  clientID.append(":");
  clientID.append(getUniqueTag());
  clientID.append(":");
  clientID.append(getDSName());
  // Hash key.

  // int offset = 0;
  for (uint32_t i = 0; i < getHostAddrLen(); i++) {
    m_hashKey.append(":");
    m_hashKey.append(std::to_string(m_hostAddr[i]));
  }
  m_hashKey.append(":");
  m_hashKey.append(std::to_string(getHostPort()));
  m_hashKey.append(":");
  m_hashKey.append(getDSName());
  m_hashKey.append(":");
  if (m_uniqueTag.size() != 0) {
    m_hashKey.append(getUniqueTag());
  } else {
    m_hashKey.append(":");
    m_hashKey.append(std::to_string(m_vmViewId));
  }
  LOGDEBUG("GethashKey %s client id: %s ", m_hashKey.c_str(), clientID.c_str());
}
const char* ClientProxyMembershipID::getDSMemberId(uint32_t& mesgLength) const {
  mesgLength = static_cast<int32_t>(m_memIDStr.size());
  return m_memIDStr.c_str();
}
const char* ClientProxyMembershipID::getDSMemberIdForCS43(
    uint32_t& mesgLength) const {
  mesgLength = static_cast<int32_t>(m_dsmemIDStr.size());
  return m_dsmemIDStr.c_str();
}

const std::string& ClientProxyMembershipID::getDSMemberIdForThinClientUse() {
  return clientID;
}

std::string ClientProxyMembershipID::getHashKey() { return m_hashKey; }

void ClientProxyMembershipID::toData(DataOutput&) const {
  throw IllegalStateException("Member ID toData() not implemented.");
}

void ClientProxyMembershipID::fromData(DataInput& input) {
  // deserialization for PR FX HA

  auto len = input.readArrayLength();  // inetaddress len
  m_hostAddrLocalMem = true;
  auto hostAddr = new uint8_t[len];
  input.readBytesOnly(hostAddr, len);  // inetaddress
  auto hostPort = input.readInt32();   // port
  auto hostname =
      std::dynamic_pointer_cast<CacheableString>(input.readObject());
  auto splitbrain = input.read();   // splitbrain
  auto dcport = input.readInt32();  // port
  auto vPID = input.readInt32();    // pid
  auto vmKind = input.read();       // vmkind
  auto aStringArray = CacheableStringArray::create();
  aStringArray->fromData(input);
  auto dsName = std::dynamic_pointer_cast<CacheableString>(input.readObject());
  auto uniqueTag =
      std::dynamic_pointer_cast<CacheableString>(input.readObject());
  auto durableClientId =
      std::dynamic_pointer_cast<CacheableString>(input.readObject());
  auto durableClntTimeOut =
      std::chrono::seconds(input.readInt32());  // durable client timeout
  int32_t vmViewId = 0;
  readVersion(splitbrain, input);

  if (vmKind != ClientProxyMembershipID::LONER_DM_TYPE) {
    vmViewId = std::stoi(uniqueTag->value());
    initObjectVars(hostname->value().c_str(), hostAddr, len, true, hostPort,
                   durableClientId->value().c_str(), durableClntTimeOut, dcport,
                   vPID, vmKind, splitbrain, dsName->value().c_str(), nullptr,
                   vmViewId);
  } else {
    // initialize the object
    initObjectVars(hostname->value().c_str(), hostAddr, len, true, hostPort,
                   durableClientId->value().c_str(), durableClntTimeOut, dcport,
                   vPID, vmKind, splitbrain, dsName->value().c_str(),
                   uniqueTag->value().c_str(), 0);
  }

  readAdditionalData(input);
}

Serializable* ClientProxyMembershipID::readEssentialData(DataInput& input) {
  uint8_t* hostAddr;
  int32_t len, hostPort, vmViewId = 0;
  std::shared_ptr<CacheableString> hostname, dsName, uniqueTag, vmViewIdstr;

  len = input.readArrayLength();  // inetaddress len
  m_hostAddrLocalMem = true;
  /* adongre - Coverity II
   * CID 29183: Out-of-bounds access (OVERRUN_DYNAMIC)
   */
  // hostAddr = new uint8_t(len);
  hostAddr = new uint8_t[len];

  input.readBytesOnly(hostAddr, len);  // inetaddress

  hostPort = input.readInt32();  // port
  // TODO: RVV get the host name from

  // read and ignore flag
  input.read();

  const auto vmKind = input.read();  // vmkind

  if (vmKind == ClientProxyMembershipID::LONER_DM_TYPE) {
    uniqueTag = std::dynamic_pointer_cast<CacheableString>(input.readObject());
  } else {
    vmViewIdstr =
        std::dynamic_pointer_cast<CacheableString>(input.readObject());
    vmViewId = std::stoi(vmViewIdstr->value());
  }

  dsName = std::dynamic_pointer_cast<CacheableString>(input.readObject());

  if (vmKind != ClientProxyMembershipID::LONER_DM_TYPE) {
    // initialize the object with the values read and some dummy values
    initObjectVars("", hostAddr, len, true, hostPort, "",
                   std::chrono::seconds::zero(), DCPORT, 0, vmKind, 0,
                   dsName->value().c_str(), nullptr, vmViewId);
  } else {
    // initialize the object with the values read and some dummy values
    initObjectVars("", hostAddr, len, true, hostPort, "",
                   std::chrono::seconds::zero(), DCPORT, 0, vmKind, 0,
                   dsName->value().c_str(), uniqueTag->value().c_str(),
                   vmViewId);
  }

  readAdditionalData(input);

  return this;
}

void ClientProxyMembershipID::readAdditionalData(DataInput& input) {
  // Skip unused UUID (16) and weight (0);
  input.advanceCursor(17);
}

void ClientProxyMembershipID::increaseSynchCounter() { ++synch_counter; }

// Compares two membershipIds. This is based on the compareTo function
// of InternalDistributedMember class of Java.
// Any change to the java function should be reflected here as well.
int16_t ClientProxyMembershipID::compareTo(
    const DSMemberForVersionStamp& other) const {
  if (this == &other) {
    return 0;
  }

  const ClientProxyMembershipID& otherMember =
      static_cast<const ClientProxyMembershipID&>(other);
  uint32_t myPort = getHostPort();
  uint32_t otherPort = otherMember.getHostPort();

  if (myPort < otherPort) return -1;
  if (myPort > otherPort) return 1;

  uint8_t* myAddr = getHostAddr();
  uint8_t* otherAddr = otherMember.getHostAddr();
  // Discard null cases
  if (myAddr == nullptr && otherAddr == nullptr) {
    if (myPort < otherPort) {
      return -1;
    } else if (myPort > otherPort) {
      return 1;
    } else {
      return 0;
    }
  } else if (myAddr == nullptr) {
    return -1;
  } else if (otherAddr == nullptr) {
    return 1;
  }
  for (uint32_t i = 0; i < getHostAddrLen(); i++) {
    if (myAddr[i] < otherAddr[i]) return -1;
    if (myAddr[i] > otherAddr[i]) return 1;
  }

  std::string myUniqueTag = getUniqueTag();
  std::string otherUniqueTag = otherMember.getUniqueTag();
  if (myUniqueTag.empty() && otherUniqueTag.empty()) {
    if (m_vmViewId < otherMember.m_vmViewId) {
      return -1;
    } else if (m_vmViewId > otherMember.m_vmViewId) {
      return 1;
    }  // else they're the same, so continue
  } else if (myUniqueTag.empty()) {
    return -1;
  } else if (otherUniqueTag.empty()) {
    return 1;
  } else {
    int i = myUniqueTag.compare(otherUniqueTag);
    if (i != 0) {
      return i;
    }
  }
  return 0;
}

void ClientProxyMembershipID::readVersion(int flags, DataInput& input) {
  if ((flags & ClientProxyMembershipID::VERSION_MASK) != 0) {
    int8_t ordinal = input.read();
    LOGDEBUG("ClientProxyMembershipID::readVersion ordinal = %d ", ordinal);
    if (ordinal != ClientProxyMembershipID::TOKEN_ORDINAL) {
    } else {
      LOGDEBUG("ClientProxyMembershipID::readVersion ordinal = %d ",
               input.readInt16());
    }
  }
}

void ClientProxyMembershipID::writeVersion(int16_t ordinal,
                                           DataOutput& output) {
  if (ordinal <= SCHAR_MAX) {
    output.write(static_cast<int8_t>(ordinal));
    LOGDEBUG("ClientProxyMembershipID::writeVersion ordinal = %d ", ordinal);
  } else {
    output.write(ClientProxyMembershipID::TOKEN_ORDINAL);
    output.writeInt(ordinal);
    LOGDEBUG("ClientProxyMembershipID::writeVersion ordinal = %d ", ordinal);
  }
}

std::shared_ptr<Serializable> ClientProxyMembershipID::createDeserializable() {
  return std::make_shared<ClientProxyMembershipID>();
}

DSFid ClientProxyMembershipID::getDSFID() const {
  return DSFid::InternalDistributedMember;
}

size_t ClientProxyMembershipID::objectSize() const { return 0; }

std::string ClientProxyMembershipID::getDSName() const { return m_dsname; }

std::string ClientProxyMembershipID::getUniqueTag() const {
  return m_uniqueTag;
}

uint8_t* ClientProxyMembershipID::getHostAddr() const { return m_hostAddr; }

uint32_t ClientProxyMembershipID::getHostAddrLen() const {
  return m_hostAddrLen;
}

uint32_t ClientProxyMembershipID::getHostPort() const { return m_hostPort; }

int32_t ClientProxyMembershipID::hashcode() const {
  uint32_t result = 0;
  char hostInfo[255] = {0};
  uint32_t offset = 0;
  for (uint32_t i = 0; i < getHostAddrLen(); i++) {
    offset +=
        std::snprintf(hostInfo + offset, 255 - offset, ":%x", m_hostAddr[i]);
  }
  result += internal::geode_hash<std::string>{}(std::string(hostInfo, offset));
  result += m_hostPort;
  return result;
}

bool ClientProxyMembershipID::operator==(const CacheableKey& other) const {
  return (this->compareTo(
              dynamic_cast<const DSMemberForVersionStamp&>(other)) == 0);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
