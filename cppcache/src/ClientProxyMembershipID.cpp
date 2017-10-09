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
#include <ace/OS.h>
#include <geode/DistributedSystem.hpp>
#include <geode/GeodeTypeIds.hpp>
#include "GeodeTypeIdsImpl.hpp"
#include <geode/CacheableBuiltins.hpp>
#include "DataOutputInternal.hpp"
#include "Version.hpp"
#include <string>
#include <memory>

#define ADDRSIZE 4
#define DCPORT 12334
#define VMKIND 13
#define ROLEARRLENGTH 0
static int synch_counter = 2;
using namespace apache::geode::client;

namespace {
static class RandomInitializer {
 public:
  RandomInitializer() {
    // using current time and
    // processor time would be good enough for our purpose
    unsigned long seed =
        ACE_OS::getpid() + ACE_OS::gettimeofday().msec() + clock();
    seed += ACE_OS::gettimeofday().usec();
    // LOGINFO("PID %ld seed %ld ACE_OS::gettimeofday().usec() = %ld clock =
    // %ld ACE_OS::gettimeofday().msec() = %ld", pid, seed ,
    // ACE_OS::gettimeofday().usec() , clock(),
    // ACE_OS::gettimeofday().msec());
    ACE_OS::srand(seed);
  }
} oneTimeRandomInitializer;
}  // namespace

const int ClientProxyMembershipID::VERSION_MASK = 0x8;
const int8_t ClientProxyMembershipID::TOKEN_ORDINAL = -1;

ClientProxyMembershipID::ClientProxyMembershipID()
    : m_hostPort(0),
      m_hostAddr(nullptr)
      /* adongre  - Coverity II
       * CID 29278: Uninitialized scalar field (UNINIT_CTOR)
       */
      ,
      m_hostAddrLen(0),
      m_hostAddrLocalMem(false),
      m_vmViewId(0) {}

ClientProxyMembershipID::~ClientProxyMembershipID() {
  if (m_hostAddrLocalMem) delete[] m_hostAddr;
}

ClientProxyMembershipID::ClientProxyMembershipID(
    std::string dsName, std::string randString, const char* hostname,
    uint32_t hostAddr, uint32_t hostPort, const char* durableClientId,
    const uint32_t durableClntTimeOut)
    : m_hostAddrAsUInt32(hostAddr) {
  int32_t vmPID = ACE_OS::getpid();
  initObjectVars(hostname, reinterpret_cast<uint8_t*>(&m_hostAddrAsUInt32), 4,
                 false, hostPort, durableClientId, durableClntTimeOut, DCPORT,
                 vmPID, VMKIND, 0, dsName.c_str(), randString.c_str(), 0);
}

// This is only for unit tests and should not be used for any other purpose. See
// testEntriesMapForVersioning.cpp for more details
ClientProxyMembershipID::ClientProxyMembershipID(
    uint8_t* hostAddr, uint32_t hostAddrLen, uint32_t hostPort,
    const char* dsname, const char* uniqueTag, uint32_t vmViewId) {
  int32_t vmPID = ACE_OS::getpid();
  initObjectVars("localhost", hostAddr, hostAddrLen, false, hostPort, "", 0,
                 DCPORT, vmPID, VMKIND, 0, dsname, uniqueTag, vmViewId);
}
void ClientProxyMembershipID::initObjectVars(
    const char* hostname, uint8_t* hostAddr, uint32_t hostAddrLen,
    bool hostAddrLocalMem, uint32_t hostPort, const char* durableClientId,
    const uint32_t durableClntTimeOut, int32_t dcPort, int32_t vPID,
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
  m_memID.write(static_cast<int8_t>(GeodeTypeIdsImpl::FixedIDByte));
  m_memID.write(
      static_cast<int8_t>(GeodeTypeIdsImpl::InternalDistributedMember));
  m_memID.writeArrayLen(ADDRSIZE);
  // writing first 4 bytes of the address. This will be same until
  // IPV6 support is added in the client
  uint32_t temp;
  memcpy(&temp, hostAddr, 4);
  m_memID.writeInt(static_cast<int32_t>(temp));
  // m_memID.writeInt((int32_t)hostPort);
  m_memID.writeInt((int32_t)synch_counter);
  m_memID.write(static_cast<int8_t>(GeodeTypeIds::CacheableASCIIString));
  m_memID.writeASCII(hostname);
  m_memID.write(splitBrainFlag);  // splitbrain flags

  m_memID.writeInt(dcPort);

  m_memID.writeInt(vPID);
  m_memID.write(vmkind);
  m_memID.writeArrayLen(ROLEARRLENGTH);
  m_memID.write(static_cast<int8_t>(GeodeTypeIds::CacheableASCIIString));
  m_memID.writeASCII(dsname);
  m_memID.write(static_cast<int8_t>(GeodeTypeIds::CacheableASCIIString));
  m_memID.writeASCII(uniqueTag);

  if (durableClientId != nullptr && durableClntTimeOut != 0) {
    m_memID.write(static_cast<int8_t>(GeodeTypeIds::CacheableASCIIString));
    m_memID.writeASCII(durableClientId);
    CacheableInt32Ptr int32ptr = CacheableInt32::create(durableClntTimeOut);
    int32ptr->toData(m_memID);
  }
  writeVersion(Version::getOrdinal(), m_memID);
  uint32_t len;
  char* buf = (char*)m_memID.getBuffer(&len);
  m_memIDStr.append(buf, len);

  char PID[15] = {0};
  char Synch_Counter[15] = {0};
  ACE_OS::itoa(vPID, PID, 10);
  ACE_OS::itoa(synch_counter, Synch_Counter, 10);
  clientID.append(hostname);
  clientID.append("(");
  clientID.append(PID);
  clientID.append(":loner):");
  clientID.append(Synch_Counter);
  clientID.append(":");
  clientID.append(getUniqueTag());
  clientID.append(":");
  clientID.append(getDSName());
  // Hash key.

  // int offset = 0;
  for (uint32_t i = 0; i < getHostAddrLen(); i++) {
    char hostInfo[16] = {0};
    // offset += ACE_OS::snprintf(hostInfo + offset , 255 - offset, ":%x",
    // m_hostAddr[i]);
    ACE_OS::itoa(m_hostAddr[i], hostInfo, 16);
    m_hashKey.append(":");
    m_hashKey.append(hostInfo);
  }
  m_hashKey.append(":");
  char hostInfoPort[16] = {0};
  ACE_OS::itoa(getHostPort(), hostInfoPort, 10);
  //  offset += ACE_OS::snprintf(hostInfo + offset, 255 - offset , ":%d",
  //  getHostPort());
  m_hashKey.append(hostInfoPort);
  m_hashKey.append(":");
  m_hashKey.append(getDSName());
  m_hashKey.append(":");
  if (m_uniqueTag.size() != 0) {
    m_hashKey.append(getUniqueTag());
  } else {
    m_hashKey.append(":");
    char viewid[16] = {0};
    ACE_OS::itoa(m_vmViewId, viewid, 10);
    // offset += ACE_OS::snprintf(hostInfo + offset , 255 - offset , ":%d",
    // m_vmViewId);
    m_hashKey.append(viewid);
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

void ClientProxyMembershipID::toData(DataOutput& output) const {
  throw IllegalStateException("Member ID toData() not implemented.");
}

void ClientProxyMembershipID::fromData(DataInput& input) {
  // deserialization for PR FX HA
  uint8_t* hostAddr;
  int32_t len, hostPort, dcport, vPID, durableClntTimeOut;
  CacheableStringPtr hostname, dsName, uniqueTag, durableClientId;
  int8_t splitbrain, vmKind;

  len = input.readArrayLen();  // inetaddress len
  m_hostAddrLocalMem = true;
  /* adongre  - Coverity II
   * CID 29184: Out-of-bounds access (OVERRUN_DYNAMIC)
   */
  // hostAddr = new uint8_t(len);
  hostAddr = new uint8_t[len];

  input.readBytesOnly(hostAddr, len);  // inetaddress
  hostPort = input.readInt32();            // port
  hostname = input.readObject<CacheableString>();          // hostname
  splitbrain = input.read();             // splitbrain
  dcport = input.readInt32();              // port
  vPID = input.readInt32();                // pid
  vmKind = input.read();                 // vmkind
  auto aStringArray = CacheableStringArray::create();
  aStringArray->fromData(input);
  dsName = input.readObject<CacheableString>();            // name
  uniqueTag = input.readObject<CacheableString>();         // unique tag
  durableClientId = input.readObject<CacheableString>();   // durable client id
  durableClntTimeOut = input.readInt32();  // durable client timeout
  int32_t vmViewId = 0;
  readVersion(splitbrain, input);

  if (vmKind != ClientProxyMembershipID::LONER_DM_TYPE) {
    vmViewId = atoi(uniqueTag.get()->asChar());
    initObjectVars(hostname->asChar(), hostAddr, len, true, hostPort,
                   durableClientId->asChar(), durableClntTimeOut, dcport, vPID,
                   vmKind, splitbrain, dsName->asChar(), nullptr, vmViewId);
  } else {
    // initialize the object
    initObjectVars(hostname->asChar(), hostAddr, len, true, hostPort,
                   durableClientId->asChar(), durableClntTimeOut, dcport, vPID,
                   vmKind, splitbrain, dsName->asChar(), uniqueTag->asChar(),
                   0);
  }

  readAdditionalData(input);
}

Serializable* ClientProxyMembershipID::readEssentialData(DataInput& input) {
  uint8_t* hostAddr;
  int32_t len, hostPort, vmViewId = 0;
  CacheableStringPtr hostname, dsName, uniqueTag, vmViewIdstr;

  len = input.readArrayLen();  // inetaddress len
  m_hostAddrLocalMem = true;
  /* adongre - Coverity II
   * CID 29183: Out-of-bounds access (OVERRUN_DYNAMIC)
   */
  // hostAddr = new uint8_t(len);
  hostAddr = new uint8_t[len];

  input.readBytesOnly(hostAddr, len);  // inetaddress

  hostPort = input.readInt32();  // port
  // TODO: RVV get the host name from

  const uint8_t flag = input.read();

  const auto vmKind = input.read();  // vmkind

  if (vmKind == ClientProxyMembershipID::LONER_DM_TYPE) {
    uniqueTag = input.readObject<CacheableString>();  // unique tag
  } else {
    vmViewIdstr = input.readObject<CacheableString>();
    vmViewId = atoi(vmViewIdstr.get()->asChar());
  }

  dsName = input.readObject<CacheableString>();  // name

  if (vmKind != ClientProxyMembershipID::LONER_DM_TYPE) {
    // initialize the object with the values read and some dummy values
    initObjectVars("", hostAddr, len, true, hostPort, "", 0, DCPORT, 0, vmKind,
                   0, dsName->asChar(), nullptr, vmViewId);
  } else {
    // initialize the object with the values read and some dummy values
    initObjectVars("", hostAddr, len, true, hostPort, "", 0, DCPORT, 0, vmKind,
                   0, dsName->asChar(), uniqueTag->asChar(), vmViewId);
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
      LOGDEBUG("ClientProxyMembershipID::readVersion ordinal = %d ", input.readInt16());
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
