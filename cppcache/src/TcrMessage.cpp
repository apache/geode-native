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

#include "TcrMessage.hpp"

#include <geode/CacheableBuiltins.hpp>
#include <geode/CacheableObjectArray.hpp>
#include <geode/SystemProperties.hpp>

#include "AutoDelete.hpp"
#include "BucketServerLocation.hpp"
#include "CacheRegionHelper.hpp"
#include "DataInputInternal.hpp"
#include "DataOutputInternal.hpp"
#include "DiskStoreId.hpp"
#include "DiskVersionTag.hpp"
#include "EventId.hpp"
#include "FixedPartitionAttributesImpl.hpp"
#include "SerializationRegistry.hpp"
#include "StackTrace.hpp"
#include "TSSTXStateWrapper.hpp"
#include "TXState.hpp"
#include "TcrChunkedContext.hpp"
#include "TcrConnection.hpp"
#include "TcrConnectionManager.hpp"
#include "ThinClientBaseDM.hpp"
#include "ThinClientPoolDM.hpp"
#include "ThinClientRegion.hpp"
#include "VersionTag.hpp"
#include "VersionedCacheableObjectPartList.hpp"
#include "util/JavaModifiedUtf8.hpp"
#include "util/string.hpp"

#ifdef _SOLARIS
#pragma error_messages(off, SEC_UNINITIALIZED_MEM_READ)
#endif

namespace apache {
namespace geode {
namespace client {
namespace {

constexpr size_t kHeaderLength = 17;

/**
 * come from Java InterestType.kREGULAR_EXPRESSION
 */
constexpr int32_t kREGULAR_EXPRESSION = 1;

constexpr int32_t kFlagEmpty = 0x01;
constexpr int32_t kFlagConcurrencyChecks = 0x02;

inline void readInt(uint8_t* buffer, uint16_t* value) {
  uint16_t tmp = *(buffer++);
  tmp = (tmp << 8) | *(buffer);
  *value = tmp;
}

inline void readInt(uint8_t* buffer, uint32_t* value) {
  uint32_t tmp = *(buffer++);
  tmp = (tmp << 8) | *(buffer++);
  tmp = (tmp << 8) | *(buffer++);
  tmp = (tmp << 8) | *(buffer++);
  *value = tmp;
}

inline void writeInt(uint8_t* buffer, uint16_t value) {
  *(buffer++) = static_cast<uint8_t>(value >> 8);
  *(buffer++) = static_cast<uint8_t>(value);
}

inline void writeInt(uint8_t* buffer, uint32_t value) {
  *(buffer++) = static_cast<uint8_t>(value >> 24);
  *(buffer++) = static_cast<uint8_t>(value >> 16);
  *(buffer++) = static_cast<uint8_t>(value >> 8);
  *(buffer++) = static_cast<uint8_t>(value);
}
}  // namespace

extern void setThreadLocalExceptionMessage(std::string);

bool TcrMessage::isUserInitiativeOps(const TcrMessage& msg) {
  int32_t msgType = msg.getMessageType();

  if (!msg.isMetaRegion() &&
      !(msgType == TcrMessage::PING || msgType == TcrMessage::PERIODIC_ACK ||
        msgType == TcrMessage::MAKE_PRIMARY ||
        msgType == TcrMessage::CLOSE_CONNECTION ||
        msgType == TcrMessage::CLIENT_READY || msgType == TcrMessage::INVALID ||
        msgType == TcrMessage::MONITORCQ_MSG_TYPE ||
        msgType == TcrMessage::GETCQSTATS_MSG_TYPE ||
        msgType == TcrMessage::REQUEST_EVENT_VALUE ||
        msgType == TcrMessage::GET_CLIENT_PR_METADATA ||
        msgType == TcrMessage::GET_CLIENT_PARTITION_ATTRIBUTES ||
        msgType == TcrMessage::GET_PDX_ID_FOR_TYPE ||
        msgType == TcrMessage::GET_PDX_TYPE_BY_ID ||
        msgType == TcrMessage::ADD_PDX_TYPE || msgType == TcrMessage::SIZE ||
        msgType == TcrMessage::TX_FAILOVER ||
        msgType == TcrMessage::GET_ENTRY ||
        msgType == TcrMessage::TX_SYNCHRONIZATION ||
        msgType == TcrMessage::GET_FUNCTION_ATTRIBUTES ||
        msgType == TcrMessage::ADD_PDX_ENUM ||
        msgType == TcrMessage::GET_PDX_ENUM_BY_ID ||
        msgType == TcrMessage::GET_PDX_ID_FOR_ENUM ||
        msgType == TcrMessage::COMMIT || msgType == TcrMessage::ROLLBACK)) {
    return true;
  }
  return false;
}

TcrMessage::TcrMessage()
    : m_request(nullptr),
      m_tcdm(nullptr),
      m_chunkedResult(nullptr),
      m_keyList(nullptr),
      m_region(nullptr),
      m_timeout(DEFAULT_TIMEOUT),
      m_metadata(),
      m_cqs(nullptr),
      m_messageResponseTimeout(-1),
      m_delta(nullptr),
      m_deltaBytes(nullptr),
      m_fpaSet(),
      m_functionAttributes(),
      m_connectionIDBytes(nullptr),
      m_creds(),
      m_key(),
      m_value(nullptr),
      m_failedNode(),
      m_callbackArgument(nullptr),
      m_versionTag(),
      m_eventid(nullptr),
      m_tombstoneVersions(),
      m_tombstoneKeys(),
      m_exceptionMessage(),
      m_regionName("INVALID_REGION_NAME"),
      m_regex(),
      m_colocatedWith(),
      m_securityHeaderLength(0),
      m_msgType(TcrMessage::INVALID),
      m_msgTypeRequest(0),
      m_txId(-1),
      m_bucketCount(0),
      m_numCqPart(0),
      m_msgTypeForCq(0),
      m_deltaBytesLen(0),
      m_entryNotFound(0),
      m_feAnotherHop(false),
      m_isSecurityOn(false),
      m_isLastChunkAndisSecurityHeader(0),
      m_isSecurityHeaderAdded(false),
      m_isMetaRegion(false),
      m_decodeAll(false),
      m_interestPolicy(0),
      m_isDurable(false),
      m_receiveValues(false),
      m_hasCqsPart(false),
      m_isInterestListPassed(false),
      m_shouldIgnore(false),
      m_metaDataVersion(0),
      m_serverGroupVersion(0),
      m_boolValue(0),
      m_isCallBackArguement(false),
      m_hasResult(0) {}

const std::vector<std::shared_ptr<CacheableKey>>* TcrMessage::getKeys() const {
  return m_keyList;
}

const std::string& TcrMessage::getRegex() const { return m_regex; }

InterestResultPolicy TcrMessage::getInterestResultPolicy() const {
  if (m_interestPolicy == 2) {
    return InterestResultPolicy::KEYS_VALUES;
  } else if (m_interestPolicy == 1) {
    return InterestResultPolicy::KEYS;
  } else {
    return InterestResultPolicy::NONE;
  }
}

bool TcrMessage::forPrimary() const {
  return m_msgType == TcrMessage::PUT || m_msgType == TcrMessage::DESTROY ||
         m_msgType == TcrMessage::EXECUTE_REGION_FUNCTION;
}

void TcrMessage::initCqMap() { m_cqs = new std::map<std::string, int>(); }

bool TcrMessage::forSingleHop() const {
  return m_msgType == TcrMessage::PUT || m_msgType == TcrMessage::DESTROY ||
         m_msgType == TcrMessage::REQUEST ||
         m_msgType == TcrMessage::GET_ALL_70 ||
         m_msgType == TcrMessage::GET_ALL_WITH_CALLBACK ||
         m_msgType == TcrMessage::EXECUTE_REGION_FUNCTION ||
         m_msgType == TcrMessage::PUTALL ||
         m_msgType == TcrMessage::PUT_ALL_WITH_CALLBACK;
}

bool TcrMessage::forTransaction() const { return m_txId != -1; }

bool TcrMessage::getBoolValue() const { return m_boolValue; }

const std::string& TcrMessage::getException() {
  m_exceptionMessage = Utils::nullSafeToString(m_value);
  return m_exceptionMessage;
}

bool TcrMessage::isDurable() const { return m_isDurable; }

bool TcrMessage::receiveValues() const { return m_receiveValues; }

bool TcrMessage::hasCqPart() const { return m_hasCqsPart; }

uint32_t TcrMessage::getMessageTypeForCq() const { return m_msgTypeForCq; }

bool TcrMessage::shouldIgnore() const { return m_shouldIgnore; }

int8_t TcrMessage::getMetaDataVersion() const { return m_metaDataVersion; }

uint32_t TcrMessage::getEntryNotFound() const { return m_entryNotFound; }

int8_t TcrMessage::getserverGroupVersion() const {
  return m_serverGroupVersion;
}

std::shared_ptr<std::vector<int8_t>> TcrMessage::getFunctionAttributes() {
  return m_functionAttributes;
}

void TcrMessage::setDM(ThinClientBaseDM* dm) { m_tcdm = dm; }

ThinClientBaseDM* TcrMessage::getDM() { return m_tcdm; }

// set the chunked response handler
void TcrMessage::setChunkedResultHandler(TcrChunkedResult* chunkedResult) {
  m_isLastChunkAndisSecurityHeader = 0x0;
  m_chunkedResult = chunkedResult;
}

TcrChunkedResult* TcrMessage::getChunkedResultHandler() {
  return m_chunkedResult;
}

DataInput* TcrMessage::getDelta() const { return m_delta.get(); }

//  getDeltaBytes( ) is called *only* by CqService, returns a CacheableBytes
//  that
// takes ownership of delta bytes.
std::shared_ptr<CacheableBytes> TcrMessage::getDeltaBytes() {
  if (m_deltaBytes == nullptr) {
    return nullptr;
  }
  auto retVal = CacheableBytes::create(
      std::vector<int8_t>(m_deltaBytes, m_deltaBytes + m_deltaBytesLen));
  _GEODE_SAFE_DELETE_ARRAY(m_deltaBytes);
  return retVal;
}

bool TcrMessage::hasDelta() const { return (m_delta != nullptr); }

void TcrMessage::setMetaRegion(bool isMetaRegion) {
  m_isMetaRegion = isMetaRegion;
}

bool TcrMessage::isMetaRegion() const { return m_isMetaRegion; }

int32_t TcrMessage::getNumBuckets() const { return m_bucketCount; }

const std::string& TcrMessage::getColocatedWith() const {
  return m_colocatedWith;
}

std::vector<std::vector<std::shared_ptr<BucketServerLocation>>>*
TcrMessage::getMetadata() {
  return m_metadata;
}

std::vector<std::shared_ptr<FixedPartitionAttributesImpl>>*
TcrMessage::getFpaSet() {
  return m_fpaSet;
}

std::shared_ptr<CacheableHashSet> TcrMessage::getFailedNode() {
  return m_failedNode;
}

bool TcrMessage::isCallBackArguement() const { return m_isCallBackArguement; }

void TcrMessage::setCallBackArguement(bool aCallBackArguement) {
  m_isCallBackArguement = aCallBackArguement;
}

void TcrMessage::setVersionTag(std::shared_ptr<VersionTag> versionTag) {
  m_versionTag = versionTag;
}
std::shared_ptr<VersionTag> TcrMessage::getVersionTag() const {
  return m_versionTag;
}

uint8_t TcrMessage::hasResult() const { return m_hasResult; }

std::shared_ptr<CacheableHashMap> TcrMessage::getTombstoneVersions() const {
  return m_tombstoneVersions;
}

std::shared_ptr<CacheableHashSet> TcrMessage::getTombstoneKeys() const {
  return m_tombstoneKeys;
}

void TcrMessage::writeInterestResultPolicyPart(InterestResultPolicy policy) {
  m_request->writeInt(static_cast<int32_t>(3));  // size
  m_request->write(static_cast<int8_t>(1));      // isObject
  m_request->write(static_cast<int8_t>(DSCode::FixedIDByte));
  m_request->write(static_cast<int8_t>(DSCode::InterestResultPolicy));
  m_request->write(static_cast<int8_t>(policy.getOrdinal()));
}

void TcrMessage::writeIntPart(int32_t intValue) {
  m_request->writeInt(static_cast<int32_t>(4));
  m_request->write(static_cast<int8_t>(0));
  m_request->writeInt(intValue);
}

void TcrMessage::writeBytePart(uint8_t byteValue) {
  m_request->writeInt(static_cast<int32_t>(1));
  m_request->write(static_cast<int8_t>(0));
  m_request->write(byteValue);
}

void TcrMessage::writeByteAndTimeOutPart(uint8_t byteValue,
                                         std::chrono::milliseconds timeout) {
  m_request->writeInt(static_cast<int32_t>(5));  // 1 (byte) + 4 (timeout)
  m_request->write(static_cast<int8_t>(0));
  m_request->write(byteValue);
  m_request->writeInt(static_cast<int32_t>(timeout.count()));
}

void TcrMessage::writeMillisecondsPart(std::chrono::milliseconds millis) {
  writeIntPart(static_cast<int32_t>(millis.count()));
}

void TcrMessage::readBooleanPartAsObject(DataInput& input, bool* boolVal) {
  int32_t lenObj = input.readInt32();
  const auto isObj = input.readBoolean();
  if (lenObj > 0) {
    if (isObj) {
      bool bVal = input.readNativeBool();
      *boolVal = bVal;
    }
  }
}

void TcrMessage::readOldValue(DataInput& input) {
  // read and ignore length
  input.readInt32();
  input.read();  // ignore isObj
  std::shared_ptr<Cacheable> value;
  input.readObject(value);  // we are not using this value currently
}

void TcrMessage::readPrMetaData(DataInput& input) {
  int32_t lenObj = input.readInt32();
  input.read();                      // ignore
  m_metaDataVersion = input.read();  // read refresh meta data byte
  if (lenObj == 2) {
    m_serverGroupVersion = input.read();
    LOGDEBUG("Single-hop m_serverGroupVersion in message reply is %d",
             m_serverGroupVersion);
  }
}
std::shared_ptr<VersionTag> TcrMessage::readVersionTagPart(
    DataInput& input, uint16_t endpointMemId,
    MemberListForVersionStamp& memberListForVersionStamp) {
  auto isObj = static_cast<DSCode>(input.read());
  std::shared_ptr<VersionTag> versionTag;

  if (isObj == DSCode::NullObj) return versionTag;

  if (isObj == DSCode::FixedIDByte) {
    versionTag = std::make_shared<VersionTag>(memberListForVersionStamp);
    if (static_cast<DSFid>(input.read()) == DSFid::VersionTag) {
      versionTag->fromData(input);
      versionTag->replaceNullMemberId(endpointMemId);
      return versionTag;
    }
  } else if (isObj == DSCode::FixedIDShort) {
    if (input.readInt16() == static_cast<int16_t>(DSFid::DiskVersionTag)) {
      DiskVersionTag* disk = new DiskVersionTag(memberListForVersionStamp);
      disk->fromData(input);
      versionTag.reset(disk);
      return versionTag;
    }
  }
  return versionTag;
}

void TcrMessage::readVersionTag(
    DataInput& input, uint16_t endpointMemId,
    MemberListForVersionStamp& memberListForVersionStamp) {
  int32_t lenObj = input.readInt32();
  input.read();  // ignore byte

  if (lenObj == 0) return;
  auto versionTag = TcrMessage::readVersionTagPart(input, endpointMemId,
                                                   memberListForVersionStamp);
  this->setVersionTag(versionTag);
}

void TcrMessage::readIntPart(DataInput& input, uint32_t* intValue) {
  uint32_t intLen = input.readInt32();
  if (intLen != 4) {
    throw Exception("int length should have been 4");
  }
  if (input.read()) throw Exception("Integer is not an object");
  *intValue = input.readInt32();
}

const std::string TcrMessage::readStringPart(DataInput& input) {
  auto stringLength = input.readInt32();
  if (input.read()) {
    throw Exception("String is not an object");
  }
  auto jmutf8 = internal::JavaModifiedUtf8::decode(
      reinterpret_cast<const char*>(input.currentBufferPosition()),
      stringLength);
  input.advanceCursor(stringLength);
  return to_utf8(jmutf8);
}

void TcrMessage::readCqsPart(DataInput& input) {
  m_cqs->clear();
  readIntPart(input, &m_numCqPart);
  for (uint32_t cqCnt = 0; cqCnt < m_numCqPart;) {
    auto cq = readStringPart(input);
    cqCnt++;
    int32_t cqOp;
    readIntPart(input, reinterpret_cast<uint32_t*>(&cqOp));
    cqCnt++;
    (*m_cqs)[cq] = cqOp;
  }
}

void TcrMessage::readCallbackObjectPart(DataInput& input, bool defaultString) {
  int32_t lenObj = input.readInt32();
  const auto isObj = input.readBoolean();
  if (lenObj > 0) {
    if (isObj) {
      input.readObject(m_callbackArgument);
    } else {
      if (defaultString) {
        m_callbackArgument = readCacheableString(input, lenObj);
      } else {
        m_callbackArgument = readCacheableBytes(input, lenObj);
      }
    }
  }
}

void TcrMessage::readObjectPart(DataInput& input, bool defaultString) {
  int32_t lenObj = input.readInt32();
  auto isObj = input.read();
  if (lenObj > 0) {
    if (isObj == 1) {
      input.readObject(m_value);
    } else {
      if (defaultString) {
        m_value = readCacheableString(input, lenObj);
      } else {
        m_value = readCacheableBytes(input, lenObj);
      }
    }
  } else if (lenObj == 0 && isObj == 2) {  // EMPTY BYTE ARRAY
    m_value = CacheableBytes::create();
  } else if (isObj == 0) {
    m_value = nullptr;
  }
}

void TcrMessage::readSecureObjectPart(DataInput& input, bool defaultString,
                                      bool isChunk,
                                      uint8_t isLastChunkWithSecurity) {
  LOGDEBUG(
      "TcrMessage::readSecureObjectPart isChunk = %d isLastChunkWithSecurity = "
      "%d",
      isChunk, isLastChunkWithSecurity);
  if (isChunk) {
    if (!(isLastChunkWithSecurity & 0x2)) {
      return;
    }
  }

  int32_t lenObj = input.readInt32();
  const auto isObj = input.readBoolean();
  LOGDEBUG(
      "TcrMessage::readSecureObjectPart lenObj = %d isObj = %d, "
      "m_msgTypeRequest = %d defaultString = %d ",
      lenObj, isObj, m_msgTypeRequest, defaultString);
  if (lenObj > 0) {
    if (isObj) {
      // TODO: ??
      input.readObject(m_value);
    } else {
      if (defaultString) {
        // TODO: ??
        // m_value = CacheableString::create(
        //   (char*)input.currentBufferPosition( ), lenObj );
        m_value = readCacheableString(input, lenObj);
      } else {
        LOGDEBUG("reading connectionid");
        // TODO: this will execute always
        // input.rea.readInt(&connectionId);
        // m_connectionIDBytes =
        // CacheableBytes::create(input.currentBufferPosition(), lenObj);
        // m_connectionIDBytes = readCacheableBytes(input, lenObj);
        m_connectionIDBytes = CacheableBytes::create(
            std::vector<int8_t>(input.currentBufferPosition(),
                                input.currentBufferPosition() + lenObj));
        input.advanceCursor(lenObj);
      }
    }
  }
  if (input.getBytesRemaining() != 0) {
    LOGERROR("readSecureObjectPart: we not read all bytes. Messagetype:%d",
             m_msgType);
    throw IllegalStateException("didn't read all bytes");
  }
}

void TcrMessage::readUniqueIDObjectPart(DataInput& input) {
  LOGDEBUG("TcrMessage::readUniqueIDObjectPart");

  int32_t lenObj = input.readInt32();
  const auto isObj = input.readBoolean();
  LOGDEBUG("TcrMessage::readUniqueIDObjectPart lenObj = %d isObj = %d", lenObj,
           isObj);
  if (lenObj > 0) {
    m_value = CacheableBytes::create(std::vector<int8_t>(
        input.currentBufferPosition(), input.currentBufferPosition() + lenObj));
    input.advanceCursor(lenObj);
  }
}

int64_t TcrMessage::getConnectionId() {
  if (m_connectionIDBytes) {
    auto di = m_tcdm->getConnectionManager().getCacheImpl()->createDataInput(
        reinterpret_cast<const uint8_t*>(m_connectionIDBytes->value().data()),
        m_connectionIDBytes->length());
    return di.readInt64();
  } else {
    LOGWARN("Returning 0 as internal connection ID msgtype = %d ", m_msgType);
    return 0;
  }
}

int64_t TcrMessage::getUniqueId() {
  if (auto cacheableBytes =
          std::dynamic_pointer_cast<CacheableBytes>(m_value)) {
    auto di = m_tcdm->getConnectionManager().getCacheImpl()->createDataInput(
        reinterpret_cast<const uint8_t*>(cacheableBytes->value().data()),
        cacheableBytes->length());
    return di.readInt64();
  }
  return 0;
}

void TcrMessage::readFailedNodePart(DataInput& input) {
  // read and ignore length
  input.readInt32();
  // read and ignore isObj
  input.readBoolean();
  m_failedNode = CacheableHashSet::create();
  input.read();  // ignore typeId
  // input.readDirectObject(m_failedNode, typeId);
  m_failedNode->fromData(input);
  LOGDEBUG("readFailedNodePart m_failedNode size = %zu", m_failedNode->size());
}

void TcrMessage::readKeyPart(DataInput& input) {
  int32_t lenObj = input.readInt32();
  const auto isObj = input.readBoolean();
  if (lenObj > 0) {
    if (isObj) {
      m_key = std::dynamic_pointer_cast<CacheableKey>(input.readObject());
    } else {
      m_key = std::dynamic_pointer_cast<CacheableKey>(
          readCacheableString(input, lenObj));
    }
  }
}

std::shared_ptr<Serializable> TcrMessage::readCacheableString(DataInput& input,
                                                              int lenObj) {
  auto decoded = internal::JavaModifiedUtf8::decode(
      reinterpret_cast<const char*>(input.currentBufferPosition()), lenObj);
  input.advanceCursor(lenObj);

  return CacheableString::create(decoded);
}

std::shared_ptr<Serializable> TcrMessage::readCacheableBytes(DataInput& input,
                                                             int lenObj) {
  if (lenObj <= 252) {  // 252 is java's ((byte)-4 && 0xFF)
    input.rewindCursor(1);
    uint8_t* buffer = const_cast<uint8_t*>(input.currentBufferPosition());
    buffer[0] = static_cast<uint8_t>(lenObj);
  } else if (lenObj <= 0xFFFF) {
    input.rewindCursor(3);
    uint8_t* buffer = const_cast<uint8_t*>(input.currentBufferPosition());
    buffer[0] = static_cast<uint8_t>(-2);
    writeInt(buffer + 1, static_cast<uint16_t>(lenObj));
  } else {
    input.rewindCursor(5);
    uint8_t* buffer = const_cast<uint8_t*>(input.currentBufferPosition());
    buffer[0] = static_cast<uint8_t>(-3);
    writeInt(buffer + 1, static_cast<uint32_t>(lenObj));
  }

  return input.readDirectObject(static_cast<int8_t>(DSCode::CacheableBytes));
}

bool TcrMessage::readExceptionPart(DataInput& input, uint8_t isLastChunk,
                                   bool skipFirstPart) {
  // Reading exception message sent from java cache server.
  // Read the first part which is serialized java exception and ignore it.
  // Then read the second part which is string and use it to construct the
  // exception.

  isLastChunk = (isLastChunk >> 5);
  LOGDEBUG("TcrMessage::readExceptionPart: isLastChunk = %d", isLastChunk);
  if (skipFirstPart == true) {
    skipParts(input, 1);
    isLastChunk--;
  }

  if (/*input.getBytesRemaining() > 0 */ isLastChunk > 0) {
    //  Do a best effort read for exception since it is possible
    // that this is invoked for non-exception chunk message which has
    // only one part.
    isLastChunk--;
    readObjectPart(input, true);
    // if (input.getBytesRemaining() > 0 && m_msgTypeRequest ==
    // TcrMessage::EXECUTE_REGION_FUNCTION && m_msgType !=
    // TcrMessage::EXCEPTION) {
    if (isLastChunk > 0) {
      readFailedNodePart(input);
      return true;  // 3 parts
    } else {
      return true;  // 2 parts
    }
  }
  return false;
}

void TcrMessage::writeObjectPart(
    const std::shared_ptr<Serializable>& se, bool isDelta, bool callToData,
    const std::vector<std::shared_ptr<CacheableKey>>* getAllKeyList) {
  //  no nullptr check since for some messages nullptr object may be valid
  uint32_t size = 0;
  // write a dummy size of 4 bytes.
  m_request->writeInt(static_cast<int32_t>(size));

  int8_t isObject = 1;

  // check if the type is a CacheableBytes
  if (auto cacheableBytes = std::dynamic_pointer_cast<CacheableBytes>(se)) {
    // for an emty byte array write EMPTY_BYTEARRAY_CODE(2) to is object
    auto byteArrLength = cacheableBytes->length();
    if (byteArrLength == 0) {
      isObject = 2;
      m_request->write(isObject);
      return;
    }
    isObject = 0;
  }

  if (isDelta) {
    m_request->write(static_cast<int8_t>(0));
  } else {
    m_request->write(isObject);
  }

  auto sizeBeforeWritingObj = m_request->getBufferLength();
  if (isDelta) {
    auto deltaPtr = std::dynamic_pointer_cast<Delta>(se);
    deltaPtr->toDelta(*m_request);
  } else if (isObject) {
    if (callToData) {
      m_request->getSerializationRegistry().serializeWithoutHeader(se,
                                                                   *m_request);
    } else {
      if (getAllKeyList != nullptr) {
        int8_t typeId = static_cast<int8_t>(DSCode::CacheableObjectArray);
        m_request->write(typeId);
        m_request->writeArrayLen(static_cast<int32_t>(getAllKeyList->size()));
        m_request->write(static_cast<int8_t>(DSCode::Class));
        m_request->writeString("java.lang.Object");
        for (const auto& key : *getAllKeyList) {
          m_request->writeObject(key);
        }
      } else {
        m_request->writeObject(se, isDelta);
      }
    }
  } else {
    // TODO::
    // CacheableBytes* rawByteArray = static_cast<CacheableBytes*>(se.get());
    // m_request->writeBytesOnly(rawByteArray->value(), rawByteArray->length());
    writeBytesOnly(se);
  }
  auto sizeAfterWritingObj = m_request->getBufferLength();
  auto sizeOfSerializedObj = sizeAfterWritingObj - sizeBeforeWritingObj;
  m_request->rewindCursor(sizeOfSerializedObj + 1 + 4);  //
  m_request->writeInt(static_cast<int32_t>(sizeOfSerializedObj));
  m_request->advanceCursor(sizeOfSerializedObj + 1);
}

void TcrMessage::writeBytesOnly(const std::shared_ptr<Serializable>& se) {
  auto cBufferLength = m_request->getBufferLength();
  uint8_t* startBytes = nullptr;
  m_request->writeObject(se);
  uint8_t* cursor =
      const_cast<uint8_t*>(m_request->getBuffer()) + cBufferLength;

  int pos = 1;  // one byte for typeid

  uint8_t code;
  code = cursor[pos++];

  if (code == 0xFF) {
    m_request->rewindCursor(2);
  } else {
    int32_t result = code;
    if (result > 252) {  // 252 is java's ((byte)-4 && 0xFF)
      if (code == 0xFE) {
        uint16_t val;
        readInt(cursor + pos, &val);
        startBytes = cursor + 4;
        result = val;
        m_request->rewindCursor(4);
      } else if (code == 0xFD) {
        uint32_t val;
        readInt(cursor + pos, &val);
        startBytes = cursor + 6;
        result = val;
        m_request->rewindCursor(6);
      }
    } else {
      startBytes = cursor + 2;
      m_request->rewindCursor(2);
    }
    for (int i = 0; i < result; i++) cursor[i] = startBytes[i];
  }
}

void TcrMessage::writeHeader(uint32_t msgType, uint32_t numOfParts) {
  int8_t earlyAck = 0x0;
  LOGDEBUG("TcrMessage::writeHeader m_isMetaRegion = %d", m_isMetaRegion);
  if (m_tcdm != nullptr) {
    if ((m_isSecurityOn =
             (m_tcdm->isSecurityOn() &&
              TcrMessage::isUserInitiativeOps(*this) && !m_isMetaRegion))) {
      earlyAck |= 0x2;
    }
  }

  LOGDEBUG("TcrMessage::writeHeader earlyAck = %d", earlyAck);

  m_request->writeInt(static_cast<int32_t>(msgType));
  m_request->writeInt(
      static_cast<int32_t>(0));  // write a dummy message len('0' here). At
                                 // the end write the length at the (buffer +
                                 // 4) offset.
  m_request->writeInt(static_cast<int32_t>(numOfParts));
  auto txState = TSSTXStateWrapper::get().getTXState();
  if (txState == nullptr) {
    m_txId = -1;
  } else {
    m_txId = txState->getTransactionId().getId();
  }
  m_request->writeInt(m_txId);

  // updateHeaderForRetry assumes that 16 bytes are written before earlyAck
  // byte. In case,
  // write header definition changes, updateHeaderForRetry should change
  // accordingly.
  m_request->write(earlyAck);
}

// Updates the early ack byte of the message to reflect that it is a retry op
// This function assumes that 16 bytes are written before earlyAck byte. In
// case,
// write header definition changes, this function should change accordingly.
void TcrMessage::updateHeaderForRetry() {
  uint8_t earlyAck = m_request->getValueAtPos(16);
  // set the isRetryBit
  m_request->updateValueAtPos(16, earlyAck | 0x4);
}

void TcrMessage::writeRegionPart(const std::string& regionName) {
  int32_t len = static_cast<int32_t>(regionName.length());
  m_request->writeInt(len);
  m_request->write(static_cast<int8_t>(0));  // isObject = 0
  m_request->writeBytesOnly(
      reinterpret_cast<int8_t*>(const_cast<char*>(regionName.c_str())), len);
}

void TcrMessage::writeStringPart(const std::string& str) {
  if (!str.empty()) {
    auto jmutf8 = internal::JavaModifiedUtf8::fromString(str);

    auto encodedLen = static_cast<int32_t>(jmutf8.length());
    m_request->writeInt(encodedLen);
    m_request->ensureCapacity(encodedLen);
    m_request->write(static_cast<int8_t>(0));  // isObject = 0 BYTE_CODE
    m_request->writeBytesOnly(reinterpret_cast<const int8_t*>(jmutf8.data()),
                              encodedLen);
  } else {
    m_request->writeInt(static_cast<uint16_t>(0));
  }
}

void TcrMessage::writeEventIdPart(int reserveSize,
                                  bool fullValueAfterDeltaFail) {
  EventId eid(true, reserveSize, fullValueAfterDeltaFail);  // set true so we
                                                            // auto-gen next
                                                            // per-thread
                                                            // sequence number
  //  Write EventId threadid and seqno.
  eid.writeIdsData(*m_request);
}

void TcrMessage::writeMessageLength() {
  auto totalLen = m_request->getBufferLength();
  auto msgLen = totalLen - kHeaderLength;
  m_request->rewindCursor(
      totalLen -
      4);  // msg len is written after the msg type which is of 4 bytes ...
  m_request->writeInt(static_cast<int32_t>(msgLen));
  m_request->advanceCursor(totalLen - 8);  // after writing 4 bytes for msg len
                                           // you are already 8 bytes ahead from
                                           // the beginning.
}

void TcrMessage::startProcessChunk(binary_semaphore& finalizeSema) {
  if (m_msgTypeRequest == TcrMessage::EXECUTECQ_MSG_TYPE ||
      m_msgTypeRequest == TcrMessage::STOPCQ_MSG_TYPE ||
      m_msgTypeRequest == TcrMessage::CLOSECQ_MSG_TYPE ||
      m_msgTypeRequest == TcrMessage::CLOSECLIENTCQS_MSG_TYPE ||
      m_msgTypeRequest == TcrMessage::GETCQSTATS_MSG_TYPE ||
      m_msgTypeRequest == TcrMessage::MONITORCQ_MSG_TYPE) {
    return;
  }
  if (m_chunkedResult == nullptr) {
    throw FatalInternalException(
        "TcrMessage::startProcessChunk: null "
        "result processor!");
  }
  switch (m_msgTypeRequest) {
    case TcrMessage::REGISTER_INTEREST:
    case TcrMessage::REGISTER_INTEREST_LIST:
    case TcrMessage::QUERY:
    case TcrMessage::QUERY_WITH_PARAMETERS:
    case TcrMessage::EXECUTE_FUNCTION:
    case TcrMessage::EXECUTE_REGION_FUNCTION:
    case TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP:
    case TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE:
    case TcrMessage::GETDURABLECQS_MSG_TYPE:
    case TcrMessage::KEY_SET:
    case TcrMessage::GET_ALL_70:
    case TcrMessage::GET_ALL_WITH_CALLBACK:
    case TcrMessage::PUTALL:
    case TcrMessage::PUT_ALL_WITH_CALLBACK:
    case TcrMessage::REMOVE_ALL: {
      m_chunkedResult->reset();
      break;
    }
    default: {
      LOGERROR(
          "Got unexpected request message type %d while starting to process "
          "response",
          m_msgTypeRequest);
      throw IllegalStateException(
          "Got unexpected request msg type while starting to process response");
    }
  }
  m_chunkedResult->setFinalizeSemaphore(&finalizeSema);
}

bool TcrMessage::isFEAnotherHop() { return m_feAnotherHop; }
void TcrMessage::handleSpecialFECase() {
  LOGDEBUG("handleSpecialFECase1 %d", this->m_isLastChunkAndisSecurityHeader);
  if ((this->m_isLastChunkAndisSecurityHeader & 0x01) == 0x01) {
    LOGDEBUG("handleSpecialFECase2 %d", this->m_isLastChunkAndisSecurityHeader);
    if (!((this->m_isLastChunkAndisSecurityHeader & 0x04) == 0x04)) {
      LOGDEBUG("handleSpecialFECase3 %d",
               this->m_isLastChunkAndisSecurityHeader);
      m_feAnotherHop = true;
    }
  }
}

void TcrMessage::processChunk(const std::vector<uint8_t>& chunk, int32_t len,
                              uint16_t endpointmemId,
                              const uint8_t isLastChunkAndisSecurityHeader) {
  // TODO: see if security header is there
  LOGDEBUG(
      "TcrMessage::processChunk isLastChunkAndisSecurityHeader = %d chunklen = "
      "%d m_msgType = %d",
      isLastChunkAndisSecurityHeader, len, m_msgType);

  this->m_isLastChunkAndisSecurityHeader = isLastChunkAndisSecurityHeader;
  handleSpecialFECase();

  if (m_tcdm == nullptr) {
    throw FatalInternalException("TcrMessage::processChunk: null DM!");
  }

  switch (m_msgType) {
    case TcrMessage::REPLY: {
      LOGDEBUG("processChunk - got reply for request %d", m_msgTypeRequest);
      chunkSecurityHeader(1, chunk, len, isLastChunkAndisSecurityHeader);
      break;
    }
    case TcrMessage::RESPONSE: {
      if (m_msgTypeRequest == TcrMessage::EXECUTECQ_MSG_TYPE ||
          m_msgTypeRequest == TcrMessage::STOPCQ_MSG_TYPE ||
          m_msgTypeRequest == TcrMessage::CLOSECQ_MSG_TYPE ||
          m_msgTypeRequest == TcrMessage::CLOSECLIENTCQS_MSG_TYPE ||
          m_msgTypeRequest == TcrMessage::GETCQSTATS_MSG_TYPE ||
          m_msgTypeRequest == TcrMessage::MONITORCQ_MSG_TYPE) {
        LOGDEBUG("processChunk - got CQ response for request %d",
                 m_msgTypeRequest);
        // TODO: do we need to do anything here
        break;
      } else if (m_msgTypeRequest == TcrMessage::PUTALL ||
                 m_msgTypeRequest == TcrMessage::PUT_ALL_WITH_CALLBACK) {
        TcrChunkedContext* chunkedContext = new TcrChunkedContext(
            chunk, len, m_chunkedResult, isLastChunkAndisSecurityHeader,
            m_tcdm->getConnectionManager().getCacheImpl());
        m_chunkedResult->setEndpointMemId(endpointmemId);
        m_tcdm->queueChunk(chunkedContext);
        if (chunk.empty()) {
          // last chunk -- wait for processing of all the chunks to complete
          m_chunkedResult->waitFinalize();
          auto ex = m_chunkedResult->getException();
          if (ex != nullptr) {
            throw *ex;
          }
        }
        break;
      }
      // fall-through for other cases
      if (m_chunkedResult != nullptr) {
        LOGDEBUG("tcrmessage in case22 ");
        TcrChunkedContext* chunkedContext = new TcrChunkedContext(
            chunk, len, m_chunkedResult, isLastChunkAndisSecurityHeader,
            m_tcdm->getConnectionManager().getCacheImpl());
        m_chunkedResult->setEndpointMemId(endpointmemId);
        m_tcdm->queueChunk(chunkedContext);
        if (chunk.empty()) {
          // last chunk -- wait for processing of all the chunks to complete
          m_chunkedResult->waitFinalize();
          //  Throw any exception during processing here.
          // Do not throw it immediately since we want to read the
          // full data from socket in any case.
          // Notice that TcrChunkedContext::handleChunk stops any
          // further processing as soon as an exception is encountered.
          // This can cause behaviour like partially filled cache in case
          // of populating cache with registerAllKeys(), so that should be
          // documented since rolling that back may not be a good idea either.
          if (const auto& ex = m_chunkedResult->getException()) {
            throw Exception(*ex);
          }
        }
      } else if (TcrMessage::CQ_EXCEPTION_TYPE == m_msgType ||
                 TcrMessage::CQDATAERROR_MSG_TYPE == m_msgType ||
                 TcrMessage::GET_ALL_DATA_ERROR == m_msgType) {
        if (!chunk.empty()) {
          chunkSecurityHeader(1, chunk, len, isLastChunkAndisSecurityHeader);
        }
      }
      break;
    }
    case TcrMessage::EXECUTE_REGION_FUNCTION_RESULT:
    case TcrMessage::EXECUTE_FUNCTION_RESULT:
    case TcrMessage::CQDATAERROR_MSG_TYPE:  // one part
    case TcrMessage::CQ_EXCEPTION_TYPE:     // one part
    case TcrMessage::RESPONSE_FROM_PRIMARY: {
      if (m_chunkedResult != nullptr) {
        LOGDEBUG("tcrmessage in case22 ");
        TcrChunkedContext* chunkedContext = new TcrChunkedContext(
            chunk, len, m_chunkedResult, isLastChunkAndisSecurityHeader,
            m_tcdm->getConnectionManager().getCacheImpl());
        m_chunkedResult->setEndpointMemId(endpointmemId);
        m_tcdm->queueChunk(chunkedContext);
        if (chunk.empty()) {
          // last chunk -- wait for processing of all the chunks to complete
          m_chunkedResult->waitFinalize();
          //  Throw any exception during processing here.
          // Do not throw it immediately since we want to read the
          // full data from socket in any case.
          // Notice that TcrChunkedContext::handleChunk stops any
          // further processing as soon as an exception is encountered.
          // This can cause behaviour like partially filled cache in case
          // of populating cache with registerAllKeys(), so that should be
          // documented since rolling that back may not be a good idea either.
          if (const auto& ex = m_chunkedResult->getException()) {
            throw Exception(*ex);
          }
        }
      } else if (TcrMessage::CQ_EXCEPTION_TYPE == m_msgType ||
                 TcrMessage::CQDATAERROR_MSG_TYPE == m_msgType ||
                 TcrMessage::GET_ALL_DATA_ERROR == m_msgType) {
        if (!chunk.empty()) {
          chunkSecurityHeader(1, chunk, len, isLastChunkAndisSecurityHeader);
        }
      }
      break;
    }
    case TcrMessage::REGISTER_INTEREST_DATA_ERROR:  // for  register interest
                                                    // error
    case EXECUTE_FUNCTION_ERROR:
    case EXECUTE_REGION_FUNCTION_ERROR: {
      if (!chunk.empty()) {
        // DeleteArray<const uint8_t> delChunk(bytes);
        //  DataInput input(bytes, len);
        // TODO: this not send two part...
        // looks like this is our exception so only one part will come
        // readExceptionPart(input, false);
        // readSecureObjectPart(input, false, true,
        // isLastChunkAndisSecurityHeader );
        chunkSecurityHeader(1, chunk, len, isLastChunkAndisSecurityHeader);
      }
      break;
    }
    case TcrMessage::EXCEPTION: {
      if (!chunk.empty()) {
        auto input =
            m_tcdm->getConnectionManager().getCacheImpl()->createDataInput(
                chunk.data(), len);
        readExceptionPart(input, isLastChunkAndisSecurityHeader);
        readSecureObjectPart(input, false, true,
                             isLastChunkAndisSecurityHeader);
      }
      break;
    }
    case TcrMessage::RESPONSE_FROM_SECONDARY: {
      // TODO: how many parts
      chunkSecurityHeader(1, chunk, len, isLastChunkAndisSecurityHeader);
      if (chunk.size()) {
        LOGFINEST("processChunk - got response from secondary, ignoring.");
      }
      break;
    }
    case TcrMessage::PUT_DATA_ERROR: {
      chunkSecurityHeader(1, chunk, len, isLastChunkAndisSecurityHeader);
      if (!chunk.empty()) {
        auto input =
            m_tcdm->getConnectionManager().getCacheImpl()->createDataInput(
                chunk.data(), len);
        auto errorString = readStringPart(input);

        if (!errorString.empty()) {
          errorString.erase(
              errorString.begin(),
              std::find_if(errorString.begin(), errorString.end(),
                           std::not1(std::ptr_fun<int, int>(std::isspace))));
          LOGDEBUG(
              "TcrMessage::%s: setting thread-local ex msg to \"%s\", %s, %d",
              __FUNCTION__, errorString.c_str(), __FILE__, __LINE__);
          setThreadLocalExceptionMessage(errorString.c_str());
        }
      }
      break;
    }
    case TcrMessage::GET_ALL_DATA_ERROR: {
      chunkSecurityHeader(1, chunk, len, isLastChunkAndisSecurityHeader);

      break;
    }
    default: {
      // TODO: how many parts what should we do here
      if (chunk.empty()) {
        LOGWARN(
            "Got unhandled message type %d while processing response, possible "
            "serialization mismatch",
            m_msgType);
        throw MessageException(
            "TcrMessage::processChunk: "
            "got unhandled message type");
      }
      break;
    }
  }
}

Pool* TcrMessage::getPool() const {
  if (m_region) {
    return m_region->getPool().get();
  }
  return nullptr;
}

void TcrMessage::chunkSecurityHeader(int skipPart,
                                     const std::vector<uint8_t> bytes,
                                     int32_t len,
                                     uint8_t isLastChunkAndSecurityHeader) {
  LOGDEBUG("TcrMessage::chunkSecurityHeader:: skipParts = %d", skipPart);
  if ((isLastChunkAndSecurityHeader & 0x3) == 0x3) {
    auto di = m_tcdm->getConnectionManager().getCacheImpl()->createDataInput(
        bytes.data(), len);
    skipParts(di, skipPart);
    readSecureObjectPart(di, false, true, isLastChunkAndSecurityHeader);
  }
}

void TcrMessage::handleByteArrayResponse(
    const char* bytearray, int32_t len, uint16_t endpointMemId,
    const SerializationRegistry& serializationRegistry,
    MemberListForVersionStamp& memberListForVersionStamp) {
  auto input = m_tcdm->getConnectionManager().getCacheImpl()->createDataInput(
      reinterpret_cast<uint8_t*>(const_cast<char*>(bytearray)), len, getPool());
  // TODO:: this need to make sure that pool is there
  //  if(m_tcdm == nullptr)
  //  throw IllegalArgumentException("Pool is nullptr in TcrMessage");
  m_msgType = input.readInt32();
  int32_t msglen;
  msglen = input.readInt32();
  int32_t numparts;
  numparts = input.readInt32();
  m_txId = input.readInt32();
  auto earlyack = input.read();
  LOGDEBUG(
      "handleByteArrayResponse m_msgType = %d m_isSecurityOn = %d requesttype "
      "=%d",
      m_msgType, m_isSecurityOn, m_msgTypeRequest);
  LOGDEBUG(
      "Message type=%d, length=%d, parts=%d, txid=%d and eack %d with data "
      "length=%d",
      m_msgType, msglen, numparts, m_txId, earlyack, len);

  // LOGFINE("Message type=%d, length=%d, parts=%d, txid=%d and eack %d with
  // data length=%d",
  // m_msgType, msglen, numparts, m_txId, earlyack, len);

  switch (m_msgType) {
    case TcrMessage::RESPONSE: {
      if (m_msgTypeRequest == TcrMessage::CONTAINS_KEY) {
        readBooleanPartAsObject(input, &m_boolValue);
      } else if (m_msgTypeRequest == TcrMessage::USER_CREDENTIAL_MESSAGE) {
        readUniqueIDObjectPart(input);
      } else if (m_msgTypeRequest == TcrMessage::GET_PDX_ID_FOR_TYPE ||
                 m_msgTypeRequest == TcrMessage::GET_PDX_ID_FOR_ENUM) {
        // int will come in response
        uint32_t typeId;
        readIntPart(input, &typeId);
        m_value = CacheableInt32::create(typeId);
      } else if (m_msgTypeRequest == TcrMessage::GET_PDX_TYPE_BY_ID) {
        // PdxType will come in response
        input.advanceCursor(sizeof(int32_t));  // ignore part size
        if (input.readBoolean()) {             // part type. If 1 is an object
          m_value = serializationRegistry.deserialize(
              input, static_cast<int8_t>(DSCode::PdxType));
        }
      } else if (m_msgTypeRequest == TcrMessage::GET_PDX_ENUM_BY_ID) {
        // PdxType will come in response
        input.advanceCursor(5);  // part header
        m_value = serializationRegistry.deserialize(input);
      } else if (m_msgTypeRequest == TcrMessage::GET_FUNCTION_ATTRIBUTES) {
        // read and ignore length
        input.readInt32();
        input.advanceCursor(1);  // ignore byte

        if (!m_functionAttributes) {
          m_functionAttributes = std::make_shared<std::vector<int8_t>>();
        }
        m_functionAttributes->push_back(input.read());
        m_functionAttributes->push_back(input.read());
        m_functionAttributes->push_back(input.read());
      } else if (m_msgTypeRequest == TcrMessage::REQUEST) {
        int32_t receivednumparts = 2;
        readObjectPart(input);
        uint32_t flag = 0;
        readIntPart(input, &flag);
        if (flag & 0x01) {
          readCallbackObjectPart(input);
          receivednumparts++;
        }

        if ((m_value == nullptr) && (flag & 0x08 /*VALUE_IS_INVALID*/)) {
          m_value = CacheableToken::invalid();
        }

        if (flag & 0x02) {
          readVersionTag(input, endpointMemId, memberListForVersionStamp);
          receivednumparts++;
        }

        if (flag & 0x04 /*KEY_NOT_PRESENT*/) {
          m_value = CacheableToken::tombstone();
        }

        if (numparts > receivednumparts) readPrMetaData(input);

      } else if (m_decodeAll) {
        readObjectPart(input);
        if (numparts == 2) {
          if (m_isCallBackArguement) {
            readCallbackObjectPart(input);
          } else {
            int32_t lenObj = input.readInt32();
            input.readBoolean();
            m_metaDataVersion = input.read();
            if (lenObj == 2) {
              m_serverGroupVersion = input.read();
              LOGDEBUG(
                  "Single-hop m_serverGroupVersion in message response is %d",
                  m_serverGroupVersion);
            }
          }
        } else if (numparts > 2) {
          skipParts(input, 1);
          int32_t lenObj = input.readInt32();
          input.readBoolean();
          m_metaDataVersion = input.read();
          LOGFINE("Single-hop metadata version in message response is %d",
                  m_metaDataVersion);
          if (lenObj == 2) {
            m_serverGroupVersion = input.read();
            LOGDEBUG(
                "Single-hop m_serverGroupVersion in message response is %d",
                m_serverGroupVersion);
          }
        }
      }
      break;
    }

    case TcrMessage::EXCEPTION: {
      uint8_t lastChunk = static_cast<uint8_t>(numparts);
      lastChunk = (lastChunk << 5);
      readExceptionPart(input, lastChunk);
      // if (m_isSecurityOn)
      // readSecureObjectPart( input );
      break;
    }

    case TcrMessage::INVALID: {
      // Read the string in the reply
      LOGWARN("Received invalid message type as reply from server");
      readObjectPart(input, true);
      break;
    }

    case TcrMessage::CLIENT_REGISTER_INTEREST:
    case TcrMessage::CLIENT_UNREGISTER_INTEREST:
    case TcrMessage::SERVER_TO_CLIENT_PING:
    case TcrMessage::REGISTER_INSTANTIATORS: {
      // ignore this
      m_shouldIgnore = true;
      break;
    }

    case TcrMessage::REGISTER_INTEREST_DATA_ERROR:
    case TcrMessage::UNREGISTER_INTEREST_DATA_ERROR:
    case TcrMessage::PUT_DATA_ERROR:
    case TcrMessage::KEY_SET_DATA_ERROR:
    case TcrMessage::DESTROY_REGION_DATA_ERROR:
    case TcrMessage::CLEAR_REGION_DATA_ERROR:
    case TcrMessage::CONTAINS_KEY_DATA_ERROR:
    case TcrMessage::PUT_DELTA_ERROR:
    case TcrMessage::REQUEST_DATA_ERROR: {
      m_value = std::make_shared<CacheableString>(readStringPart(input));
      break;
    }

    case TcrMessage::REPLY: {
      switch (m_msgTypeRequest) {
        case TcrMessage::PUT: {
          readPrMetaData(input);
          uint32_t flags = 0;
          readIntPart(input, &flags);
          if (flags & 0x01) {  //  has old value
            readOldValue(input);
          }
          if (flags & 0x04) {
            readVersionTag(input, endpointMemId, memberListForVersionStamp);
          }
          break;
        }
        case TcrMessage::INVALIDATE: {
          uint32_t flags = 0;
          readIntPart(input, &flags);
          if (flags & 0x01) {
            readVersionTag(input, endpointMemId, memberListForVersionStamp);
          }
          readPrMetaData(input);

          break;
        }
        case TcrMessage::DESTROY: {
          uint32_t flags = 0;
          readIntPart(input, &flags);
          if (flags & 0x01) {
            readVersionTag(input, endpointMemId, memberListForVersionStamp);
          }
          readPrMetaData(input);
          // skip the Destroy65.java response entryNotFound int part so
          // that the readSecureObjectPart() call below gets the security part
          // skipParts(input, 1);
          readIntPart(input, &m_entryNotFound);
          LOGDEBUG("Inside TcrMessage::REPLY::DESTROY m_entryNotFound = %d ",
                   m_entryNotFound);
          break;
        }
        case TcrMessage::PING:
        default: {
          readPrMetaData(input);
          break;
        }
      }
      break;
    }
    case TcrMessage::LOCAL_INVALIDATE:
    case TcrMessage::LOCAL_DESTROY: {
      int32_t regionLen = input.readInt32();
      input.advanceCursor(1);  // ignore byte
      char* regname = nullptr;
      regname = new char[regionLen + 1];
      DeleteArray<char> delRegName(regname);
      input.readBytesOnly(reinterpret_cast<int8_t*>(regname), regionLen);
      regname[regionLen] = '\0';
      m_regionName = regname;

      readKeyPart(input);

      // skipParts(input, 1); // skip callbackarg parts
      readCallbackObjectPart(input);
      readVersionTag(input, endpointMemId, memberListForVersionStamp);
      readBooleanPartAsObject(input, &m_isInterestListPassed);
      readBooleanPartAsObject(input, &m_hasCqsPart);
      if (m_hasCqsPart) {
        if (m_msgType == TcrMessage::LOCAL_INVALIDATE) {
          readIntPart(input, &m_msgTypeForCq);
        } else {
          m_msgTypeForCq = static_cast<uint32_t>(m_msgType);
        }
        // LOGINFO("got cq local local_invalidate/local_destroy read
        // m_hasCqsPart");
        readCqsPart(input);
      }

      // read eventid part
      readEventIdPart(input, false);

      break;
    }

    case TcrMessage::LOCAL_CREATE:
    case TcrMessage::LOCAL_UPDATE: {
      int32_t regionLen = input.readInt32();
      input.advanceCursor(1);  // ignore byte
      char* regname = nullptr;
      regname = new char[regionLen + 1];
      DeleteArray<char> delRegName(regname);
      input.readBytesOnly(reinterpret_cast<int8_t*>(regname), regionLen);
      regname[regionLen] = '\0';
      m_regionName = regname;

      readKeyPart(input);
      //  Read delta flag
      bool isDelta = false;
      readBooleanPartAsObject(input, &isDelta);
      if (isDelta) {
        m_deltaBytesLen = input.readInt32();

        input.advanceCursor(1);  // ignore byte
        m_deltaBytes = new int8_t[m_deltaBytesLen];
        input.readBytesOnly(m_deltaBytes, m_deltaBytesLen);
        m_delta = std::unique_ptr<DataInput>(new DataInput(
            m_tcdm->getConnectionManager().getCacheImpl()->createDataInput(
                reinterpret_cast<const uint8_t*>(m_deltaBytes),
                m_deltaBytesLen)));
      } else {
        readObjectPart(input);
      }

      // skip callbackarg part
      // skipParts(input, 1);
      readCallbackObjectPart(input);
      readVersionTag(input, endpointMemId, memberListForVersionStamp);
      readBooleanPartAsObject(input, &m_isInterestListPassed);
      readBooleanPartAsObject(input, &m_hasCqsPart);

      if (m_hasCqsPart) {
        // LOGINFO("got cq local_create/local_create");
        readCqsPart(input);
        m_msgTypeForCq = static_cast<uint32_t>(m_msgType);
      }

      // read eventid part
      readEventIdPart(input, false);
      _GEODE_SAFE_DELETE_ARRAY(regname);  // COVERITY ---> 30299 Resource leak

      break;
    }
    case TcrMessage::CLIENT_MARKER: {
      // dont skip (non-existent) callbackarg part, just read eventid part
      readEventIdPart(input, false);
      break;
    }

    case TcrMessage::LOCAL_DESTROY_REGION:
    case TcrMessage::CLEAR_REGION: {
      int32_t regionLen = input.readInt32();
      input.advanceCursor(1);  // ignore byte
      char* regname = nullptr;
      regname = new char[regionLen + 1];
      DeleteArray<char> delRegName(regname);
      input.readBytesOnly(reinterpret_cast<int8_t*>(regname), regionLen);
      regname[regionLen] = '\0';
      m_regionName = regname;
      // skip callbackarg part
      // skipParts(input, 1);
      readCallbackObjectPart(input);
      readBooleanPartAsObject(input, &m_hasCqsPart);
      if (m_hasCqsPart) {
        // LOGINFO("got cq region_destroy read m_hasCqsPart");
        readCqsPart(input);
      }
      // read eventid part
      readEventIdPart(input, false);
      break;
    }

    case TcrMessage::RESPONSE_CLIENT_PR_METADATA: {
      if (len == 17) {
        LOGDEBUG("RESPONSE_CLIENT_PR_METADATA len is 17");
        return;
      }
      m_metadata =
          new std::vector<std::vector<std::shared_ptr<BucketServerLocation>>>();
      for (int32_t i = 0; i < numparts; i++) {
        input.readInt32();          // ignore partlen
        input.read();               // ignore  isObj;
        auto bits8 = input.read();  // cacheable vector typeid
        LOGDEBUG("Expected typeID %d, got %d", DSCode::CacheableArrayList,
                 bits8);

        auto arrayLength = input.readArrayLength();  // array length
        LOGDEBUG("Array length = %d ", arrayLength);
        if (arrayLength > 0) {
          std::vector<std::shared_ptr<BucketServerLocation>>
              bucketServerLocations;
          for (int32_t index = 0; index < arrayLength; index++) {
            // ignore DS typeid, CLASS typeid, and string typeid
            input.advanceCursor(3);
            uint16_t classLen = input.readInt16();  // Read classLen
            input.advanceCursor(classLen);
            auto location = std::make_shared<BucketServerLocation>();
            location->fromData(input);
            LOGFINE("location contains %d\t%s\t%d\t%d\t%s",
                    location->getBucketId(), location->getServerName().c_str(),
                    location->getPort(), location->getVersion(),
                    (location->isPrimary() ? "true" : "false"));
            bucketServerLocations.push_back(location);
          }
          m_metadata->push_back(bucketServerLocations);
        }
        LOGFINER("Metadata size is %zu", m_metadata->size());
      }
      break;
    }

    case TcrMessage::GET_CLIENT_PR_METADATA_ERROR: {
      LOGERROR("Failed to get single-hop meta data");
      break;
    }

    case TcrMessage::RESPONSE_CLIENT_PARTITION_ATTRIBUTES: {
      input.readInt32();  // ignore partlen;
      input.read();       // ignore isObj;

      // PART1 = bucketCount
      m_bucketCount = input.readNativeInt32();

      auto partLength = input.readInt32();  // partlen;
      input.read();                         // ignore isObj;
      if (partLength > 0) {
        // PART2 = colocatedwith
        m_colocatedWith = input.readString();
      }

      if (numparts == 4) {
        partLength = input.readInt32();  // partlen;
        input.read();                    // ignore isObj;
        if (partLength > 0) {
          // PART3 = partitionresolvername
          input.readString();  // ignore
        }

        input.readInt32();  // ignore partlen;
        input.read();       // ignore isObj;
        input.read();       // ignore cacheable CacheableHashSet typeid

        auto arrayLength = input.readArrayLength();  // array length
        if (arrayLength > 0) {
          m_fpaSet =
              new std::vector<std::shared_ptr<FixedPartitionAttributesImpl>>();
          for (int32_t index = 0; index < arrayLength; index++) {
            input.advanceCursor(
                3);  // ignore DS typeid, CLASS typeid, string typeid
            auto classLen = input.readInt16();  // Read classLen
            input.advanceCursor(classLen);
            auto fpa = std::make_shared<FixedPartitionAttributesImpl>();
            fpa->fromData(input);  // PART4 = set of FixedAttributes.
            LOGDEBUG("fpa contains %d\t%s\t%d\t%d", fpa->getNumBuckets(),
                     fpa->getPartitionName().c_str(), fpa->isPrimary(),
                     fpa->getStartingBucketID());
            m_fpaSet->push_back(fpa);
          }
        }
      }
      break;
    }
    case TcrMessage::TOMBSTONE_OPERATION: {
      uint32_t tombstoneOpType;
      int32_t regionLen = input.readInt32();
      input.read();
      char* regname = nullptr;

      regname = new char[regionLen + 1];
      DeleteArray<char> delRegName(regname);
      input.readBytesOnly(reinterpret_cast<int8_t*>(regname), regionLen);
      regname[regionLen] = '\0';
      m_regionName = regname;
      readIntPart(input, &tombstoneOpType);  // partlen;
      // read and ignore length
      input.readInt32();
      // read and ignore isObj
      input.read();

      if (tombstoneOpType == 0) {
        if (m_tombstoneVersions == nullptr) {
          m_tombstoneVersions = CacheableHashMap::create();
        }
        readHashMapForGCVersions(input, m_tombstoneVersions);
      } else if (tombstoneOpType == 1) {
        if (m_tombstoneKeys == nullptr) {
          m_tombstoneKeys = CacheableHashSet::create();
        }
        // input.readObject(m_tombstoneKeys);
        readHashSetForGCVersions(input, m_tombstoneKeys);
      } else {
        LOGERROR("Failed to read the tombstone versions");
        break;
      }
      // readEventId Part
      readEventIdPart(input, false);
      break;
    }
    case TcrMessage::GET_CLIENT_PARTITION_ATTRIBUTES_ERROR: {
      LOGERROR("Failed to get server partitioned region attributes");
      break;
    }

    case TcrMessage::UNKNOWN_MESSAGE_TYPE_ERROR: {
      // do nothing
      break;
    }

    case TcrMessage::REQUEST_EVENT_VALUE_ERROR: {
      LOGERROR("Error while requesting full value for delta");
      break;
    }

    default:
      LOGERROR(
          "Unknown message type %d in response, possible serialization "
          "mismatch",
          m_msgType);
      std::stringstream ss;
      ss << boost::stacktrace::stacktrace();
      LOGERROR(ss.str().c_str());
      throw MessageException("handleByteArrayResponse: unknown message type");
  }
  LOGDEBUG("handleByteArrayResponse earlyack = %d ", earlyack);
  if (earlyack & 0x2) readSecureObjectPart(input);
}

TcrMessageDestroyRegion::TcrMessageDestroyRegion(
    DataOutput* dataOutput, const Region* region,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    std::chrono::milliseconds messageResponsetimeout,
    ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::DESTROY_REGION;
  m_tcdm = connectionDM;
  m_regionName =
      region == nullptr ? "INVALID_REGION_NAME" : region->getFullPath();
  m_region = region;
  m_timeout = DEFAULT_TIMEOUT;
  m_messageResponseTimeout = messageResponsetimeout;

  uint32_t numOfParts = 1;
  if (aCallbackArgument != nullptr) {
    ++numOfParts;
  }

  numOfParts++;

  if (m_messageResponseTimeout >= std::chrono::milliseconds::zero()) {
    numOfParts++;
  }
  writeHeader(m_msgType, numOfParts);
  writeRegionPart(m_regionName);
  writeEventIdPart();
  if (aCallbackArgument != nullptr) {
    writeObjectPart(aCallbackArgument);
  }
  if (m_messageResponseTimeout >= std::chrono::milliseconds::zero()) {
    writeMillisecondsPart(m_messageResponseTimeout);
  }

  writeMessageLength();
}

TcrMessageClearRegion::TcrMessageClearRegion(
    DataOutput* dataOutput, const Region* region,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    std::chrono::milliseconds messageResponsetimeout,
    ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::CLEAR_REGION;
  m_tcdm = connectionDM;
  m_regionName =
      region == nullptr ? "INVALID_REGION_NAME" : region->getFullPath();
  m_region = region;
  m_timeout = DEFAULT_TIMEOUT;
  m_messageResponseTimeout = messageResponsetimeout;

  m_isSecurityOn = false;
  m_isSecurityHeaderAdded = false;

  uint32_t numOfParts = 1;
  if (aCallbackArgument != nullptr) {
    ++numOfParts;
  }

  numOfParts++;

  if (m_messageResponseTimeout >= std::chrono::milliseconds::zero()) {
    numOfParts++;
  }
  writeHeader(m_msgType, numOfParts);
  writeRegionPart(m_regionName);
  writeEventIdPart();
  if (aCallbackArgument != nullptr) {
    writeObjectPart(aCallbackArgument);
  }
  if (m_messageResponseTimeout >= std::chrono::milliseconds::zero()) {
    writeMillisecondsPart(m_messageResponseTimeout);
  }

  writeMessageLength();
}

TcrMessageQuery::TcrMessageQuery(
    DataOutput* dataOutput, const std::string& regionName,
    std::chrono::milliseconds messageResponsetimeout,
    ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::QUERY;
  m_tcdm = connectionDM;
  m_regionName = regionName;  // this is querystri;
  m_timeout = DEFAULT_TIMEOUT;
  m_messageResponseTimeout = messageResponsetimeout;
  m_region = nullptr;
  uint32_t numOfParts = 1;

  numOfParts++;

  if (m_messageResponseTimeout >= std::chrono::milliseconds::zero()) {
    numOfParts++;
  }
  writeHeader(m_msgType, numOfParts);
  writeRegionPart(m_regionName);
  writeEventIdPart();
  if (m_messageResponseTimeout >= std::chrono::milliseconds::zero()) {
    writeMillisecondsPart(m_messageResponseTimeout);
  }
  writeMessageLength();
}

TcrMessageStopCQ::TcrMessageStopCQ(
    DataOutput* dataOutput, const std::string& regionName,
    std::chrono::milliseconds messageResponsetimeout,
    ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::STOPCQ_MSG_TYPE;
  m_tcdm = connectionDM;
  m_regionName = regionName;  // this is querystring
  m_timeout = DEFAULT_TIMEOUT;
  m_messageResponseTimeout = messageResponsetimeout;
  m_region = nullptr;
  m_isSecurityHeaderAdded = false;
  m_isMetaRegion = false;

  uint32_t numOfParts = 1;

  numOfParts++;

  if (m_messageResponseTimeout >= std::chrono::milliseconds::zero()) {
    numOfParts++;
  }

  writeHeader(m_msgType, numOfParts);
  writeRegionPart(m_regionName);
  writeEventIdPart();
  if (m_messageResponseTimeout >= std::chrono::milliseconds::zero()) {
    writeMillisecondsPart(m_messageResponseTimeout);
  }
  writeMessageLength();
}

TcrMessageCloseCQ::TcrMessageCloseCQ(
    DataOutput* dataOutput, const std::string& regionName,
    std::chrono::milliseconds messageResponsetimeout,
    ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::CLOSECQ_MSG_TYPE;
  m_tcdm = connectionDM;
  m_regionName = regionName;  // this is querystring
  m_timeout = DEFAULT_TIMEOUT;
  m_messageResponseTimeout = messageResponsetimeout;
  m_region = nullptr;
  uint32_t numOfParts = 1;

  numOfParts++;

  if (m_messageResponseTimeout >= std::chrono::milliseconds::zero()) {
    numOfParts++;
  }
  writeHeader(m_msgType, numOfParts);
  writeRegionPart(m_regionName);
  writeEventIdPart();
  if (m_messageResponseTimeout >= std::chrono::milliseconds::zero()) {
    writeMillisecondsPart(m_messageResponseTimeout);
  }
  writeMessageLength();
}

TcrMessageQueryWithParameters::TcrMessageQueryWithParameters(
    DataOutput* dataOutput, const std::string& regionName,
    const std::shared_ptr<Serializable>&,
    std::shared_ptr<CacheableVector> paramList,
    std::chrono::milliseconds messageResponsetimeout,
    ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::QUERY_WITH_PARAMETERS;
  m_tcdm = connectionDM;
  m_regionName = regionName;
  m_timeout = DEFAULT_TIMEOUT;
  m_messageResponseTimeout = messageResponsetimeout;
  m_region = nullptr;

  // Find out the numOfParts
  uint32_t numOfParts = 4 + static_cast<uint32_t>(paramList->size());
  writeHeader(m_msgType, numOfParts);
  // Part-1: Query String
  writeRegionPart(m_regionName);

  // Part-2: Number or length of the parameters
  writeIntPart(static_cast<uint32_t>(paramList->size()));

  // Part-3: X (COMPILE_QUERY_CLEAR_TIMEOUT) parameter
  writeIntPart(15);

  // Part-4: Request specific timeout
  if (m_messageResponseTimeout >= std::chrono::milliseconds::zero()) {
    writeMillisecondsPart(m_messageResponseTimeout);
  }
  // Part-5: Parameters
  if (paramList != nullptr) {
    for (const auto& value : *paramList) {
      writeObjectPart(value);
    }
  }
  writeMessageLength();
}

TcrMessageContainsKey::TcrMessageContainsKey(
    DataOutput* dataOutput, const Region* region,
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& aCallbackArgument, bool isContainsKey,
    ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::CONTAINS_KEY;
  m_tcdm = connectionDM;
  m_regionName =
      region == nullptr ? "INVALID_REGION_NAME" : region->getFullPath();
  m_region = region;
  m_timeout = DEFAULT_TIMEOUT;

  uint32_t numOfParts = 2;
  if (aCallbackArgument != nullptr) {
    ++numOfParts;
  }

  numOfParts++;

  if (key == nullptr) {
    throw IllegalArgumentException(
        "key passed to the constructor can't be nullptr");
  }

  writeHeader(m_msgType, numOfParts);
  writeRegionPart(m_regionName);
  writeObjectPart(key);
  // write 0 to indicate containskey (1 for containsvalueforkey)
  writeIntPart(isContainsKey ? 0 : 1);
  if (aCallbackArgument != nullptr) {
    writeObjectPart(aCallbackArgument);
  }
  writeMessageLength();
}

TcrMessageGetDurableCqs::TcrMessageGetDurableCqs(
    DataOutput* dataOutput, ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::GETDURABLECQS_MSG_TYPE;
  m_tcdm = connectionDM;
  m_timeout = DEFAULT_TIMEOUT;
  m_region = nullptr;
  // wrirting msgtype with part length =1
  writeHeader(m_msgType, 1);
  // the server expects at least 1 part, so writing a dummy byte part
  writeBytePart(0);
  writeMessageLength();
}

TcrMessageRequest::TcrMessageRequest(
    DataOutput* dataOutput, const Region* region,
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::REQUEST;
  m_tcdm = connectionDM;
  m_key = key;
  m_regionName =
      (region == nullptr ? "INVALID_REGION_NAME" : region->getFullPath());
  m_region = region;
  m_timeout = DEFAULT_TIMEOUT;

  uint32_t numOfParts = 2;
  if (aCallbackArgument != nullptr) {
    ++numOfParts;
  }

  numOfParts++;

  if (key == nullptr) {
    throw IllegalArgumentException(
        "key passed to the constructor can't be nullptr");
  }

  numOfParts--;  // no event id for request
  writeHeader(TcrMessage::REQUEST, numOfParts);
  writeRegionPart(m_regionName);
  writeObjectPart(key);
  if (aCallbackArgument != nullptr) {
    // set bool variable to true.
    m_isCallBackArguement = true;
    writeObjectPart(aCallbackArgument);
  }
  writeMessageLength();
}

TcrMessageInvalidate::TcrMessageInvalidate(
    DataOutput* dataOutput, const Region* region,
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::INVALIDATE;
  m_tcdm = connectionDM;
  m_key = key;
  m_regionName =
      (region == nullptr ? "INVALID_REGION_NAME" : region->getFullPath());
  m_region = region;
  m_timeout = DEFAULT_TIMEOUT;

  uint32_t numOfParts = 2;
  if (aCallbackArgument != nullptr) {
    ++numOfParts;
  }

  numOfParts++;

  if (key == nullptr) {
    throw IllegalArgumentException(
        "key passed to the constructor can't be nullptr");
  }

  writeHeader(TcrMessage::INVALIDATE, numOfParts);
  writeRegionPart(m_regionName);
  writeObjectPart(key);
  writeEventIdPart();
  if (aCallbackArgument != nullptr) {
    // set bool variable to true.
    m_isCallBackArguement = true;
    writeObjectPart(aCallbackArgument);
  }
  writeMessageLength();
}

TcrMessageDestroy::TcrMessageDestroy(
    DataOutput* dataOutput, const Region* region,
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Cacheable>& value,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::DESTROY;
  m_tcdm = connectionDM;
  m_key = key;
  m_regionName =
      (region == nullptr ? "INVALID_REGION_NAME" : region->getFullPath());
  m_region = region;
  m_timeout = DEFAULT_TIMEOUT;
  uint32_t numOfParts = 2;
  if (aCallbackArgument != nullptr) {
    ++numOfParts;
  }

  numOfParts++;

  if (key == nullptr) {
    throw IllegalArgumentException(
        "key passed to the constructor can't be nullptr");
  }

  if (value != nullptr) {
    numOfParts += 2;  // for GFE Destroy65.java
    writeHeader(TcrMessage::DESTROY, numOfParts);
    writeRegionPart(m_regionName);
    writeObjectPart(key);
    writeObjectPart(value);  // expectedOldValue part
    uint8_t removeByte = 8;  // OP_TYPE_DESTROY value from Operation.java
    auto removeBytePart = CacheableByte::create(removeByte);
    writeObjectPart(removeBytePart);  // operation part
    writeEventIdPart();
    if (aCallbackArgument != nullptr) {
      writeObjectPart(aCallbackArgument);
    }
    writeMessageLength();
  } else {
    numOfParts += 2;  // for GFE Destroy65.java
    writeHeader(TcrMessage::DESTROY, numOfParts);
    writeRegionPart(m_regionName);
    writeObjectPart(key);
    writeObjectPart(nullptr);  // expectedOldValue part
    writeObjectPart(nullptr);  // operation part
    writeEventIdPart();
    if (aCallbackArgument != nullptr) {
      writeObjectPart(aCallbackArgument);
    }
    writeMessageLength();
  }
}

TcrMessagePut::TcrMessagePut(
    DataOutput* dataOutput, const Region* region,
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Cacheable>& value,
    const std::shared_ptr<Serializable>& aCallbackArgument, bool isDelta,
    ThinClientBaseDM* connectionDM, bool isMetaRegion,
    bool fullValueAfterDeltaFail, const char* regionName) {
  m_request.reset(dataOutput);
  // m_securityHeaderLength = 0;
  m_isMetaRegion = isMetaRegion;
  m_msgType = TcrMessage::PUT;
  m_tcdm = connectionDM;
  m_key = key;
  m_regionName = region != nullptr ? region->getFullPath() : regionName;
  m_region = region;
  m_timeout = DEFAULT_TIMEOUT;

  // TODO check the number of parts in this constructor. doubt because in PUT
  // value can be nullptr also.
  uint32_t numOfParts = 5;
  if (aCallbackArgument != nullptr) {
    ++numOfParts;
  }

  numOfParts++;

  if (key == nullptr) {
    throw IllegalArgumentException(
        "key passed to the constructor can't be nullptr");
  }

  numOfParts++;
  writeHeader(m_msgType, numOfParts);
  writeRegionPart(m_regionName);
  writeObjectPart(nullptr);  // operation = null
  writeIntPart(0);           // flags = 0
  writeObjectPart(key);
  writeObjectPart(CacheableBoolean::create(isDelta));
  writeObjectPart(value, isDelta);
  writeEventIdPart(0, fullValueAfterDeltaFail);
  if (aCallbackArgument != nullptr) {
    writeObjectPart(aCallbackArgument);
  }
  writeMessageLength();
}

TcrMessageReply::TcrMessageReply(bool decodeAll,
                                 ThinClientBaseDM* connectionDM) {
  m_msgType = TcrMessage::INVALID;
  m_decodeAll = decodeAll;
  m_tcdm = connectionDM;

  if (connectionDM != nullptr) m_isSecurityOn = connectionDM->isSecurityOn();
}

TcrMessagePing::TcrMessagePing(std::unique_ptr<DataOutput> dataOutput) {
  m_msgType = TcrMessage::PING;
  m_request = std::move(dataOutput);
  writeHeader(m_msgType, 0);
  writeMessageLength();
}

TcrMessageCloseConnection::TcrMessageCloseConnection(
    std::unique_ptr<DataOutput> dataOutput, bool keepAlive) {
  m_msgType = TcrMessage::CLOSE_CONNECTION;
  m_request = std::move(dataOutput);
  writeHeader(m_msgType, 1);
  m_request->writeInt(static_cast<int32_t>(1));  // len
  m_request->writeBoolean(false);                // is obj
  m_request->writeBoolean(keepAlive);            // keepalive
  writeMessageLength();
}

TcrMessageClientMarker::TcrMessageClientMarker(DataOutput* dataOutput,
                                               bool decodeAll) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::CLIENT_MARKER;
  m_decodeAll = decodeAll;
}

TcrMessageRegisterInterestList::TcrMessageRegisterInterestList(
    DataOutput* dataOutput, const Region* region,
    const std::vector<std::shared_ptr<CacheableKey>>& keys, bool isDurable,
    bool isCachingEnabled, bool receiveValues,
    InterestResultPolicy interestPolicy, ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::REGISTER_INTEREST_LIST;
  m_tcdm = connectionDM;
  m_keyList = &keys;
  m_regionName =
      region == nullptr ? "INVALID_REGION_NAME" : region->getFullPath();
  m_region = region;
  m_timeout = DEFAULT_TIMEOUT;
  m_isDurable = isDurable;
  m_receiveValues = receiveValues;

  writeHeader(m_msgType, 6);

  // Part 1
  writeRegionPart(m_regionName);

  // Part 2
  writeInterestResultPolicyPart(interestPolicy);

  // Part 3
  writeBytePart(isDurable ? 1 : 0);  // keepalive

  // Part 4
  auto cal = CacheableArrayList::create();
  for (auto&& key : keys) {
    if (key == nullptr) {
      throw IllegalArgumentException(
          "keys in the interest list cannot be nullptr");
    }
    cal->push_back(key);
  }
  writeObjectPart(cal);

  // Part 5
  int8_t bytes[2];
  std::shared_ptr<CacheableBytes> byteArr = nullptr;
  bytes[0] = receiveValues ? 0 : 1;  // reveive values
  byteArr = CacheableBytes::create(std::vector<int8_t>(bytes, bytes + 1));
  writeObjectPart(byteArr);

  // Part 6
  bytes[0] = isCachingEnabled ? 1 : 0;  // region policy
  bytes[1] = 0;                         // serialize values
  byteArr = CacheableBytes::create(std::vector<int8_t>(bytes, bytes + 2));
  writeObjectPart(byteArr);

  writeMessageLength();
  m_interestPolicy = interestPolicy.ordinal;
}

TcrMessageUnregisterInterestList::TcrMessageUnregisterInterestList(
    DataOutput* dataOutput, const Region* region,
    const std::vector<std::shared_ptr<CacheableKey>>& keys, bool isDurable,
    bool receiveValues, InterestResultPolicy interestPolicy,
    ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::UNREGISTER_INTEREST_LIST;
  m_tcdm = connectionDM;
  m_keyList = &keys;
  m_regionName =
      region == nullptr ? "INVALID_REGION_NAME" : region->getFullPath();
  m_region = region;
  m_timeout = DEFAULT_TIMEOUT;
  m_isDurable = isDurable;
  m_receiveValues = receiveValues;

  auto numInItrestList = keys.size();
  assert(numInItrestList != 0);
  uint32_t numOfParts = 2 + static_cast<uint32_t>(numInItrestList);

  numOfParts += 2;
  writeHeader(m_msgType, numOfParts);
  writeRegionPart(m_regionName);
  writeBytePart(0);                  // isClosing
  writeBytePart(isDurable ? 1 : 0);  // keepalive

  writeIntPart(static_cast<int32_t>(numInItrestList));

  for (uint32_t i = 0; i < numInItrestList; i++) {
    if (keys[i] == nullptr) {
      throw IllegalArgumentException(
          "keys in the interest list cannot be nullptr");
    }
    writeObjectPart(keys[i]);
  }

  writeMessageLength();
  m_interestPolicy = interestPolicy.ordinal;
}

TcrMessageCreateRegion::TcrMessageCreateRegion(
    DataOutput* dataOutput, const std::string& str1, const std::string& str2,
    bool isDurable, bool receiveValues, ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::CREATE_REGION;
  m_tcdm = connectionDM;
  m_isDurable = isDurable;
  m_receiveValues = receiveValues;

  uint32_t numOfParts = 2;
  writeHeader(m_msgType, numOfParts);
  writeRegionPart(str1);  // parent region name
  writeRegionPart(str2);  // region name
  writeMessageLength();
  m_regionName = str2;
}

TcrMessageRegisterInterest::TcrMessageRegisterInterest(
    DataOutput* dataOutput, const std::string& str1, const std::string& str2,
    InterestResultPolicy interestPolicy, bool isDurable, bool isCachingEnabled,
    bool receiveValues, ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::REGISTER_INTEREST;
  m_tcdm = connectionDM;
  m_isDurable = isDurable;
  m_receiveValues = receiveValues;

  uint32_t numOfParts = 7;

  writeHeader(m_msgType, numOfParts);

  writeRegionPart(str1);                          // region name
  writeIntPart(kREGULAR_EXPRESSION);              // InterestType
  writeInterestResultPolicyPart(interestPolicy);  // InterestResultPolicy
  writeBytePart(isDurable ? 1 : 0);
  writeRegionPart(str2);  // regexp string

  int8_t bytes[2];
  std::shared_ptr<CacheableBytes> byteArr = nullptr;
  bytes[0] = receiveValues ? 0 : 1;
  byteArr = CacheableBytes::create(std::vector<int8_t>(bytes, bytes + 1));
  writeObjectPart(byteArr);
  bytes[0] = isCachingEnabled ? 1 : 0;  // region data policy
  bytes[1] = 0;                         // serializevalues
  byteArr = CacheableBytes::create(std::vector<int8_t>(bytes, bytes + 2));
  writeObjectPart(byteArr);

  writeMessageLength();
  m_regionName = str1;
  m_regex = str2;
  m_interestPolicy = interestPolicy.ordinal;
}

TcrMessageUnregisterInterest::TcrMessageUnregisterInterest(
    DataOutput* dataOutput, const std::string& str1, const std::string& str2,
    InterestResultPolicy interestPolicy, bool isDurable, bool receiveValues,
    ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::UNREGISTER_INTEREST;
  m_tcdm = connectionDM;
  m_isDurable = isDurable;
  m_receiveValues = receiveValues;

  uint32_t numOfParts = 3;
  numOfParts += 2;
  writeHeader(m_msgType, numOfParts);
  writeRegionPart(str1);              // region name
  writeIntPart(kREGULAR_EXPRESSION);  // InterestType
  writeRegionPart(str2);              // regexp string
  writeBytePart(0);                   // isClosing
  writeBytePart(isDurable ? 1 : 0);   // keepalive
  writeMessageLength();
  m_regionName = str1;
  m_regex = str2;
  m_interestPolicy = interestPolicy.ordinal;
}

TcrMessageTxSynchronization::TcrMessageTxSynchronization(DataOutput* dataOutput,
                                                         int ordinal, int txid,
                                                         int status) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::TX_SYNCHRONIZATION;

  writeHeader(m_msgType, ordinal == 1 ? 3 : 2);
  writeIntPart(ordinal);
  writeIntPart(txid);
  if (ordinal == 1) {
    writeIntPart(status);
  }

  writeMessageLength();
}

TcrMessageClientReady::TcrMessageClientReady(DataOutput* dataOutput) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::CLIENT_READY;

  writeHeader(m_msgType, 1);
  // the server expects at least 1 part, so writing a dummy
  writeBytePart(0);
  writeMessageLength();
}

TcrMessageCommit::TcrMessageCommit(DataOutput* dataOutput) {
  m_msgType = TcrMessage::COMMIT;
  m_request.reset(dataOutput);

  writeHeader(m_msgType, 1);
  // the server expects at least 1 part, so writing a dummy
  writeBytePart(0);
  writeMessageLength();
}

TcrMessageRollback::TcrMessageRollback(DataOutput* dataOutput) {
  m_msgType = TcrMessage::ROLLBACK;
  m_request.reset(dataOutput);

  writeHeader(m_msgType, 1);
  // the server expects at least 1 part, so writing a dummy
  writeBytePart(0);
  writeMessageLength();
}

TcrMessageTxFailover::TcrMessageTxFailover(DataOutput* dataOutput) {
  m_msgType = TcrMessage::TX_FAILOVER;
  m_request.reset(dataOutput);

  writeHeader(m_msgType, 1);
  // the server expects at least 1 part, so writing a dummy
  writeBytePart(0);
  writeMessageLength();
}

// constructor for MAKE_PRIMARY message.
TcrMessageMakePrimary::TcrMessageMakePrimary(DataOutput* dataOutput,
                                             bool processedMarker) {
  m_msgType = TcrMessage::MAKE_PRIMARY;
  m_request.reset(dataOutput);

  writeHeader(m_msgType, 1);
  writeBytePart(processedMarker ? 1 : 0);  // boolean processedMarker
  writeMessageLength();
}

// constructor for PERIODIC_ACK of notified eventids
TcrMessagePeriodicAck::TcrMessagePeriodicAck(
    DataOutput* dataOutput, const EventIdMapEntryList& entries) {
  m_msgType = TcrMessage::PERIODIC_ACK;
  m_request.reset(dataOutput);

  uint32_t numParts = static_cast<uint32_t>(entries.size());
  writeHeader(m_msgType, numParts);
  for (EventIdMapEntryList::const_iterator entry = entries.begin();
       entry != entries.end(); ++entry) {
    auto src = entry->first;
    auto seq = entry->second;
    auto eid = EventId::create(src->getMemId(), src->getMemIdLen(),
                               src->getThrId(), seq->getSeqNum());
    writeObjectPart(eid);
  }
  writeMessageLength();
}

TcrMessagePutAll::TcrMessagePutAll(
    DataOutput* dataOutput, const Region* region, const HashMapOfCacheable& map,
    std::chrono::milliseconds messageResponsetimeout,
    ThinClientBaseDM* connectionDM,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  m_tcdm = connectionDM;
  m_regionName = region->getFullPath();
  m_region = region;
  m_messageResponseTimeout = messageResponsetimeout;
  m_request.reset(dataOutput);

  // TODO check the number of parts in this constructor. doubt because in PUT
  // value can be nullptr also.
  uint32_t numOfParts = 0;
  // bool skipCallBacks = false;

  if (aCallbackArgument != nullptr) {
    m_msgType = TcrMessage::PUT_ALL_WITH_CALLBACK;
    numOfParts = 6 + static_cast<uint32_t>(map.size()) * 2;
    // skipCallBacks = false;
  } else {
    m_msgType = TcrMessage::PUTALL;
    numOfParts = 5 + static_cast<uint32_t>(map.size()) * 2;
    // skipCallBacks = true;
  }

  // numOfParts++;

  if (m_messageResponseTimeout >= std::chrono::milliseconds::zero()) {
    numOfParts++;
  }

  writeHeader(m_msgType, numOfParts);
  writeRegionPart(m_regionName);
  writeEventIdPart(static_cast<uint32_t>(map.size()) - 1);
  // writeIntPart(skipCallBacks ? 0 : 1);
  writeIntPart(0);

  // Client putAll requests now send a flags int as part #0.  1==region has
  // datapolicy.EMPTY, 2==region has concurrency checks enabled.
  // Version tags are not sent back if dp is EMPTY or concurrency
  // checks are disabled.
  int flags = 0;
  if (!region->getAttributes().getCachingEnabled()) {
    flags |= kFlagEmpty;
    LOGDEBUG("TcrMessage::PUTALL datapolicy empty flags = %d ", flags);
  }
  if (region->getAttributes().getConcurrencyChecksEnabled()) {
    flags |= kFlagConcurrencyChecks;
    LOGDEBUG("TcrMessage::PUTALL ConcurrencyChecksEnabled flags = %d ", flags);
  }
  writeIntPart(flags);

  writeIntPart(static_cast<int32_t>(map.size()));

  if (aCallbackArgument != nullptr) {
    writeObjectPart(aCallbackArgument);
  }

  for (const auto& iter : map) {
    writeObjectPart(iter.first);
    writeObjectPart(iter.second);
  }

  if (m_messageResponseTimeout >= std::chrono::milliseconds::zero()) {
    writeMillisecondsPart(m_messageResponseTimeout);
  }
  writeMessageLength();
}

TcrMessageRemoveAll::TcrMessageRemoveAll(
    DataOutput* dataOutput, const Region* region,
    const std::vector<std::shared_ptr<CacheableKey>>& keys,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    ThinClientBaseDM* connectionDM) {
  m_msgType = TcrMessage::REMOVE_ALL;
  m_tcdm = connectionDM;
  m_regionName = region->getFullPath();
  m_region = region;
  m_request.reset(dataOutput);

  // TODO check the number of parts in this constructor. doubt because in PUT
  // value can be nullptr also.
  uint32_t numOfParts = 5 + static_cast<uint32_t>(keys.size());

  if (m_messageResponseTimeout >= std::chrono::milliseconds::zero()) {
    numOfParts++;
  }
  writeHeader(m_msgType, numOfParts);
  writeRegionPart(m_regionName);
  writeEventIdPart(static_cast<int>(keys.size() - 1));

  // Client removeall requests now send a flags int as part #0.  1==region has
  // datapolicy.EMPTY, 2==region has concurrency checks enabled.
  // Version tags are not sent back if dp is EMPTY or concurrency
  // checks are disabled.
  int flags = 0;
  if (!region->getAttributes().getCachingEnabled()) {
    flags |= kFlagEmpty;
    LOGDEBUG("TcrMessage::REMOVE_ALL datapolicy empty flags = %d ", flags);
  }
  if (region->getAttributes().getConcurrencyChecksEnabled()) {
    flags |= kFlagConcurrencyChecks;
    LOGDEBUG("TcrMessage::REMOVE_ALL ConcurrencyChecksEnabled flags = %d ",
             flags);
  }
  writeIntPart(flags);
  writeObjectPart(aCallbackArgument);
  writeIntPart(static_cast<int32_t>(keys.size()));

  for (const auto& key : keys) {
    writeObjectPart(key);
  }
  writeMessageLength();
}

TcrMessageGetAll::TcrMessageGetAll(
    DataOutput* dataOutput, const Region* region,
    const std::vector<std::shared_ptr<CacheableKey>>* keys,
    ThinClientBaseDM* connectionDM,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  m_msgType = TcrMessage::GET_ALL_70;
  m_tcdm = connectionDM;
  m_keyList = keys;
  m_callbackArgument = aCallbackArgument;
  m_regionName = region->getFullPath();
  m_region = region;
  m_request.reset(dataOutput);

  /*CacheableObjectArrayPtr keyArr = nullptr;
  if (keys != nullptr) {
    keyArr = CacheableObjectArray::create();
    for (int32_t index = 0; index < keys->size(); ++index) {
      keyArr->push_back(keys->operator[](index));
    }
  }*/
  if (m_callbackArgument != nullptr) {
    m_msgType = TcrMessage::GET_ALL_WITH_CALLBACK;
  } else {
    m_msgType = TcrMessage::GET_ALL_70;
  }

  writeHeader(m_msgType, 3);
  writeRegionPart(m_regionName);
  /*writeHeader(m_msgType, 2);
  writeRegionPart(regionName);
  writeObjectPart(keyArr);
  writeMessageLength();*/
}

void TcrMessage::InitializeGetallMsg(
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  /*CacheableObjectArrayPtr keyArr = nullptr;
  if (m_keyList != nullptr) {
    keyArr = CacheableObjectArray::create();
    for (int32_t index = 0; index < m_keyList->size(); ++index) {
      keyArr->push_back(m_keyList->operator[](index));
    }
  }*/
  // LOGINFO(" in InitializeGetallMsg %s ", m_regionName.c_str());
  // writeHeader(m_msgType, 2);
  // writeRegionPart(m_regionName);
  writeObjectPart(nullptr, false, false, m_keyList);  // will do manually
  if (aCallbackArgument != nullptr) {
    writeObjectPart(aCallbackArgument);
  } else {
    writeIntPart(0);
  }
  writeMessageLength();
}

TcrMessageExecuteCq::TcrMessageExecuteCq(DataOutput* dataOutput,
                                         const std::string& str1,
                                         const std::string& str2, CqState state,
                                         bool isDurable,
                                         ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);

  m_msgType = TcrMessage::EXECUTECQ_MSG_TYPE;
  m_tcdm = connectionDM;
  m_isDurable = isDurable;

  uint32_t numOfParts = 5;
  writeHeader(m_msgType, numOfParts);
  writeStringPart(str1);  // cqName
  writeStringPart(str2);  // query string

  writeIntPart(static_cast<int32_t>(state));  // cq state
  writeBytePart(isDurable ? 1 : 0);
  //  hard-coding region data policy to 1
  // This Part will be removed when server-side changes are made to remove
  // CQ dependency on region data policy. After the changes, set numOfParts
  // to 4 (currently 5).
  writeBytePart(1);
  writeMessageLength();
  m_regionName = str1;
  m_regex = str2;
}

TcrMessageExecuteCqWithIr::TcrMessageExecuteCqWithIr(
    DataOutput* dataOutput, const std::string& str1, const std::string& str2,
    CqState state, bool isDurable, ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);

  m_msgType = TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE;
  m_tcdm = connectionDM;
  m_isDurable = isDurable;

  uint32_t numOfParts = 5;
  writeHeader(m_msgType, numOfParts);
  writeStringPart(str1);  // cqName
  writeStringPart(str2);  // query string

  writeIntPart(static_cast<int32_t>(state));  // cq state
  writeBytePart(isDurable ? 1 : 0);
  //  hard-coding region data policy to 1
  // This Part will be removed when server-side changes are made to remove
  // CQ dependency on region data policy. After the changes, set numOfParts
  // to 4 (currently 5).
  writeBytePart(1);
  writeMessageLength();
  m_regionName = str1;
  m_regex = str2;
}

TcrMessageExecuteFunction::TcrMessageExecuteFunction(
    DataOutput* dataOutput, const std::string& funcName,
    const std::shared_ptr<Cacheable>& args, uint8_t getResult,
    ThinClientBaseDM* connectionDM, std::chrono::milliseconds timeout) {
  m_request.reset(dataOutput);

  m_msgType = TcrMessage::EXECUTE_FUNCTION;
  m_tcdm = connectionDM;
  m_hasResult = getResult;

  uint32_t numOfParts = 3;
  writeHeader(m_msgType, numOfParts);
  writeByteAndTimeOutPart(getResult, timeout);
  writeRegionPart(funcName);  // function name string
  writeObjectPart(args);
  writeMessageLength();
}

TcrMessageExecuteRegionFunction::TcrMessageExecuteRegionFunction(
    DataOutput* dataOutput, const std::string& funcName, const Region* region,
    const std::shared_ptr<Cacheable>& args,
    std::shared_ptr<CacheableVector> routingObj, uint8_t getResult,
    std::shared_ptr<CacheableHashSet> failedNodes,
    std::chrono::milliseconds timeout, ThinClientBaseDM* connectionDM,
    int8_t reExecute) {
  m_request.reset(dataOutput);

  m_msgType = TcrMessage::EXECUTE_REGION_FUNCTION;
  m_tcdm = connectionDM;
  m_regionName =
      region == nullptr ? "INVALID_REGION_NAME" : region->getFullPath();
  m_region = region;
  m_hasResult = getResult;

  if (routingObj && routingObj->size() == 1) {
    LOGDEBUG("setting up key");
    m_key = std::dynamic_pointer_cast<CacheableKey>(routingObj->at(0));
  }

  uint32_t numOfParts =
      6 + (!routingObj ? 0 : static_cast<uint32_t>(routingObj->size()));
  numOfParts +=
      2;  // for the FunctionHA isReExecute and removedNodesSize parts.
  if (failedNodes != nullptr) {
    numOfParts++;
  }
  writeHeader(m_msgType, numOfParts);
  writeByteAndTimeOutPart(getResult, timeout);
  writeRegionPart(m_regionName);
  writeRegionPart(funcName);  // function name string
  writeObjectPart(args);
  // klug for MemberMappedArgs
  writeObjectPart(nullptr);
  writeBytePart(reExecute);  // FunctionHA isReExecute = false
  if (routingObj) {
    writeIntPart(static_cast<int32_t>(routingObj->size()));
    for (const auto& value : *routingObj) {
      writeObjectPart(value);
    }
  } else {
    writeIntPart(0);
  }
  if (failedNodes) {
    writeIntPart(static_cast<int32_t>(failedNodes->size()));
    writeObjectPart(failedNodes);
  } else {
    writeIntPart(0);  // FunctionHA removedNodesSize = 0
  }
  writeMessageLength();
}

TcrMessageExecuteRegionFunctionSingleHop::
    TcrMessageExecuteRegionFunctionSingleHop(
        DataOutput* dataOutput, const std::string& funcName,
        const Region* region, const std::shared_ptr<Cacheable>& args,
        std::shared_ptr<CacheableHashSet> routingObj, uint8_t getResult,
        std::shared_ptr<CacheableHashSet> failedNodes, bool allBuckets,
        std::chrono::milliseconds timeout, ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);

  m_msgType = TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP;
  m_tcdm = connectionDM;
  m_regionName =
      region == nullptr ? "INVALID_REGION_NAME" : region->getFullPath();
  m_region = region;
  m_hasResult = getResult;

  uint32_t numOfParts =
      6 + (routingObj ? static_cast<int32_t>(routingObj->size()) : 0);
  numOfParts +=
      2;  // for the FunctionHA isReExecute and removedNodesSize parts.
  if (failedNodes != nullptr) {
    numOfParts++;
  }
  writeHeader(m_msgType, numOfParts);
  writeByteAndTimeOutPart(getResult, timeout);
  writeRegionPart(m_regionName);
  writeRegionPart(funcName);  // function name string
  writeObjectPart(args);
  // klug for MemberMappedArgs
  writeObjectPart(nullptr);
  writeBytePart(allBuckets ? 1 : 0);
  if (routingObj) {
    writeIntPart(static_cast<int32_t>(routingObj->size()));
    if (allBuckets) {
      LOGDEBUG("All Buckets so putting IntPart for buckets = %zu",
               routingObj->size());
      for (const auto& itr : *routingObj) {
        writeIntPart(std::dynamic_pointer_cast<CacheableInt32>(itr)->value());
      }
    } else {
      LOGDEBUG("putting keys as withFilter called, routing Keys size = %zu",
               routingObj->size());
      for (const auto& itr : *routingObj) {
        writeObjectPart(itr);
      }
    }
  } else {
    writeIntPart(0);
  }
  if (failedNodes) {
    writeIntPart(static_cast<int32_t>(failedNodes->size()));
    writeObjectPart(failedNodes);
  } else {
    writeIntPart(0);  // FunctionHA removedNodesSize = 0
  }
  writeMessageLength();
}

TcrMessageGetClientPartitionAttributes::TcrMessageGetClientPartitionAttributes(
    DataOutput* dataOutput, const char* regionName) {
  m_request.reset(dataOutput);

  m_msgType = TcrMessage::GET_CLIENT_PARTITION_ATTRIBUTES;
  writeHeader(m_msgType, 1);
  writeRegionPart(regionName);
  writeMessageLength();
}

TcrMessageGetClientPrMetadata::TcrMessageGetClientPrMetadata(
    DataOutput* dataOutput, const char* regionName) {
  m_request.reset(dataOutput);

  m_msgType = TcrMessage::GET_CLIENT_PR_METADATA;
  writeHeader(m_msgType, 1);
  writeRegionPart(regionName);
  writeMessageLength();
}

TcrMessageSize::TcrMessageSize(DataOutput* dataOutput, const char* regionName) {
  m_request.reset(dataOutput);

  m_msgType = TcrMessage::SIZE;
  writeHeader(m_msgType, 1);
  writeRegionPart(regionName);
  writeMessageLength();
}

TcrMessageUserCredential::TcrMessageUserCredential(
    DataOutput* dataOutput, std::shared_ptr<Properties> creds,
    ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);

  m_msgType = TcrMessage::USER_CREDENTIAL_MESSAGE;
  m_tcdm = connectionDM;

  /*
   * First part will be connection-id ( may be in encrypted form) to avoid
   * replay attack
   * Second part will be credentails (may be in encrypted form)
   */
  m_creds = creds;
  /*LOGDEBUG("Tcrmessage sending creds to server");
  writeHeader(msgType, numOfParts);
  writeObjectPart(creds);
  writeMessageLength();
  LOGDEBUG("TcrMessage addsp = %s ",
  Utils::convertBytesToString(m_request->getBuffer(),
  m_request->getBufferLength())->value().c_str());*/
}

TcrMessageRemoveUserAuth::TcrMessageRemoveUserAuth(
    DataOutput* dataOutput, bool keepAlive, ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);

  m_msgType = TcrMessage::REMOVE_USER_AUTH;
  m_tcdm = connectionDM;
  LOGDEBUG("Tcrmessage sending REMOVE_USER_AUTH message to server");
  writeHeader(m_msgType, 1);
  // adding dummy part as server has check for numberofparts > 0
  int8_t dummy = 0;
  if (keepAlive) dummy = 1;
  auto cbp = CacheableBytes::create(std::vector<int8_t>(&dummy, &dummy + 1));
  writeObjectPart(cbp, false);
  writeMessageLength();
  LOGDEBUG("TcrMessage REMOVE_USER_AUTH = %s ",
           Utils::convertBytesToString(m_request->getBuffer(),
                                       m_request->getBufferLength())
               .c_str());
}

void TcrMessage::createUserCredentialMessage(TcrConnection*) {
  m_request->reset();
  m_isSecurityHeaderAdded = false;
  writeHeader(m_msgType, 1);

  auto dOut = m_tcdm->getConnectionManager().getCacheImpl()->createDataOutput(
      getPool());

  if (m_creds != nullptr) m_creds->toData(dOut);

  auto credBytes = CacheableBytes::create(std::vector<int8_t>(
      dOut.getBuffer(), dOut.getBuffer() + dOut.getBufferLength()));
  writeObjectPart(credBytes);

  writeMessageLength();
  LOGDEBUG("TcrMessage::createUserCredentialMessage  msg = %s ",
           Utils::convertBytesToString(m_request->getBuffer(),
                                       m_request->getBufferLength())
               .c_str());
}

void TcrMessage::addSecurityPart(int64_t connectionId, int64_t unique_id,
                                 TcrConnection* conn) {
  LOGDEBUG("TcrMessage::addSecurityPart m_isSecurityHeaderAdded = %d ",
           m_isSecurityHeaderAdded);
  if (m_isSecurityHeaderAdded) {
    m_request->rewindCursor(m_securityHeaderLength);
    writeMessageLength();
    m_securityHeaderLength = 0;
    m_isSecurityHeaderAdded = false;
  }
  m_isSecurityHeaderAdded = true;
  LOGDEBUG("addSecurityPart( , ) ");
  auto dOutput =
      m_tcdm->getConnectionManager().getCacheImpl()->createDataOutput(
          getPool());

  dOutput.writeInt(connectionId);
  dOutput.writeInt(unique_id);

  auto bytes = CacheableBytes::create(std::vector<int8_t>(
      dOutput.getBuffer(), dOutput.getBuffer() + dOutput.getBufferLength()));

  LOGDEBUG("TcrMessage::addSecurityPart [%p] length = %" PRId32
           ", encrypted ID = %s ",
           conn, bytes->length(),
           Utils::convertBytesToString(bytes->value().data(), bytes->length())
               .c_str());

  writeObjectPart(bytes);
  writeMessageLength();
  m_securityHeaderLength = 4 + 1 + bytes->length();
}

void TcrMessage::addSecurityPart(int64_t connectionId, TcrConnection*) {
  LOGDEBUG("TcrMessage::addSecurityPart m_isSecurityHeaderAdded = %d ",
           m_isSecurityHeaderAdded);
  if (m_isSecurityHeaderAdded) {
    m_request->rewindCursor(m_securityHeaderLength);
    writeMessageLength();
    m_securityHeaderLength = 0;
    m_isSecurityHeaderAdded = false;
  }
  m_isSecurityHeaderAdded = true;
  LOGDEBUG("TcrMessage::addSecurityPart only connid");
  auto dOutput =
      m_tcdm->getConnectionManager().getCacheImpl()->createDataOutput(
          getPool());

  dOutput.writeInt(connectionId);

  auto bytes = CacheableBytes::create(std::vector<int8_t>(
      dOutput.getBuffer(), dOutput.getBuffer() + dOutput.getBufferLength()));

  writeObjectPart(bytes);
  writeMessageLength();
  m_securityHeaderLength = 4 + 1 + bytes->length();
  LOGDEBUG("TcrMessage addspCC = %s ",
           Utils::convertBytesToString(m_request->getBuffer(),
                                       m_request->getBufferLength())
               .c_str());
}

TcrMessageRequestEventValue::TcrMessageRequestEventValue(
    DataOutput* dataOutput, std::shared_ptr<EventId> eventId) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::REQUEST_EVENT_VALUE;

  uint32_t numOfParts = 1;
  writeHeader(m_msgType, numOfParts);
  writeObjectPart(eventId);
  writeMessageLength();
}

TcrMessageGetPdxIdForType::TcrMessageGetPdxIdForType(
    DataOutput* dataOutput, const std::shared_ptr<Cacheable>& pdxType,
    ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::GET_PDX_ID_FOR_TYPE;
  m_tcdm = connectionDM;

  LOGDEBUG("Tcrmessage sending GET_PDX_ID_FOR_TYPE message to server");
  writeHeader(m_msgType, 1);
  writeObjectPart(pdxType, false, true);
  writeMessageLength();
  LOGDEBUG("TcrMessage GET_PDX_ID_FOR_TYPE = %s ",
           Utils::convertBytesToString(m_request->getBuffer(),
                                       m_request->getBufferLength())
               .c_str());
}

TcrMessageAddPdxType::TcrMessageAddPdxType(
    DataOutput* dataOutput, const std::shared_ptr<Cacheable>& pdxType,
    ThinClientBaseDM* connectionDM, int32_t pdxTypeId) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::ADD_PDX_TYPE;
  m_tcdm = connectionDM;

  LOGDEBUG("Tcrmessage sending ADD_PDX_TYPE message to server");
  writeHeader(m_msgType, 2);
  writeObjectPart(pdxType, false, true);
  writeIntPart(pdxTypeId);
  writeMessageLength();
  LOGDEBUG("TcrMessage ADD_PDX_TYPE id = %d = %s ", pdxTypeId,
           Utils::convertBytesToString(m_request->getBuffer(),
                                       m_request->getBufferLength())
               .c_str());
}

TcrMessageGetPdxIdForEnum::TcrMessageGetPdxIdForEnum(
    DataOutput* dataOutput, const std::shared_ptr<Cacheable>& pdxType,
    ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::GET_PDX_ID_FOR_ENUM;
  m_tcdm = connectionDM;

  LOGDEBUG("Tcrmessage sending GET_PDX_ID_FOR_ENUM message to server");
  writeHeader(m_msgType, 1);
  writeObjectPart(pdxType, false, false);
  writeMessageLength();
  LOGDEBUG("TcrMessage GET_PDX_ID_FOR_ENUM = %s ",
           Utils::convertBytesToString(m_request->getBuffer(),
                                       m_request->getBufferLength())
               .c_str());
}

TcrMessageAddPdxEnum::TcrMessageAddPdxEnum(
    DataOutput* dataOutput, const std::shared_ptr<Cacheable>& pdxType,
    ThinClientBaseDM* connectionDM, int32_t pdxTypeId) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::ADD_PDX_ENUM;
  m_tcdm = connectionDM;

  LOGDEBUG("Tcrmessage sending ADD_PDX_ENUM message to server");
  writeHeader(m_msgType, 2);
  writeObjectPart(pdxType, false, false);
  writeIntPart(pdxTypeId);
  writeMessageLength();
  LOGDEBUG("TcrMessage ADD_PDX_ENUM id = %d = %s ", pdxTypeId,
           Utils::convertBytesToString(m_request->getBuffer(),
                                       m_request->getBufferLength())
               .c_str());
}

TcrMessageGetPdxTypeById::TcrMessageGetPdxTypeById(
    DataOutput* dataOutput, int32_t typeId, ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::GET_PDX_TYPE_BY_ID;
  m_tcdm = connectionDM;

  LOGDEBUG("Tcrmessage sending GET_PDX_TYPE_BY_ID message to server");
  writeHeader(m_msgType, 1);
  m_request->writeInt(4);
  m_request->writeBoolean(false);
  m_request->writeInt(typeId);
  writeMessageLength();
  LOGDEBUG("TcrMessage GET_PDX_TYPE_BY_ID = %s ",
           Utils::convertBytesToString(m_request->getBuffer(),
                                       m_request->getBufferLength())
               .c_str());
}

TcrMessageGetPdxEnumById::TcrMessageGetPdxEnumById(
    DataOutput* dataOutput, int32_t typeId, ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::GET_PDX_ENUM_BY_ID;
  m_tcdm = connectionDM;

  LOGDEBUG("Tcrmessage sending GET_PDX_ENUM_BY_ID message to server");
  writeHeader(m_msgType, 1);
  m_request->writeInt(4);
  m_request->writeBoolean(false);
  m_request->writeInt(typeId);
  writeMessageLength();
  LOGDEBUG("TcrMessage GET_PDX_ENUM_BY_ID = %s ",
           Utils::convertBytesToString(m_request->getBuffer(),
                                       m_request->getBufferLength())
               .c_str());
}

TcrMessageGetFunctionAttributes::TcrMessageGetFunctionAttributes(
    DataOutput* dataOutput, const std::string& funcName,
    ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::GET_FUNCTION_ATTRIBUTES;
  m_tcdm = connectionDM;

  uint32_t numOfParts = 1;
  writeHeader(m_msgType, numOfParts);
  writeRegionPart(funcName);  // function name string
  writeMessageLength();
}

TcrMessageKeySet::TcrMessageKeySet(DataOutput* dataOutput,
                                   const std::string& funcName,
                                   ThinClientBaseDM* connectionDM) {
  m_request.reset(dataOutput);
  m_msgType = TcrMessage::KEY_SET;
  m_tcdm = connectionDM;

  uint32_t numOfParts = 1;
  writeHeader(m_msgType, numOfParts);
  writeRegionPart(funcName);  // function name string
  writeMessageLength();
}

void TcrMessage::setData(const char* bytearray, int32_t len, uint16_t memId,
                         const SerializationRegistry& serializationRegistry,
                         MemberListForVersionStamp& memberListForVersionStamp) {
  if (m_request == nullptr) {
    m_request = std::unique_ptr<DataOutput>(new DataOutput(
        m_tcdm->getConnectionManager().getCacheImpl()->createDataOutput(
            getPool())));
  }
  if (bytearray) {
    DeleteArray<const char> delByteArr(bytearray);
    handleByteArrayResponse(bytearray, len, memId, serializationRegistry,
                            memberListForVersionStamp);
  }
}

TcrMessage::~TcrMessage() {
  _GEODE_SAFE_DELETE(m_cqs);
  /* adongre
   * CID 29167: Non-array delete for scalars (DELETE_ARRAY)
   * Coverity - II
   */
  // _GEODE_SAFE_DELETE( m_deltaBytes );
  _GEODE_SAFE_DELETE_ARRAY(m_deltaBytes);
}

const std::string& TcrMessage::getRegionName() const { return m_regionName; }

Region* TcrMessage::getRegion() const { return const_cast<Region*>(m_region); }

int32_t TcrMessage::getMessageType() const { return m_msgType; }

void TcrMessage::setMessageType(int32_t msgType) { m_msgType = msgType; }

void TcrMessage::setMessageTypeRequest(int32_t msgType) {
  m_msgTypeRequest = msgType;
}

const std::map<std::string, int>* TcrMessage::getCqs() const { return m_cqs; }
std::shared_ptr<CacheableKey> TcrMessage::getKey() const { return m_key; }

const std::shared_ptr<CacheableKey>& TcrMessage::getKeyRef() const {
  return m_key;
}
std::shared_ptr<Cacheable> TcrMessage::getValue() const { return m_value; }

const std::shared_ptr<Cacheable>& TcrMessage::getValueRef() const {
  return m_value;
}
std::shared_ptr<Cacheable> TcrMessage::getCallbackArgument() const {
  return m_callbackArgument;
}

const std::shared_ptr<Cacheable>& TcrMessage::getCallbackArgumentRef() const {
  return m_callbackArgument;
}

const char* TcrMessage::getMsgData() const {
  return reinterpret_cast<const char*>(m_request->getBuffer());
}

size_t TcrMessage::getMsgLength() const { return m_request->getBufferLength(); }

std::shared_ptr<EventId> TcrMessage::getEventId() const { return m_eventid; }

int32_t TcrMessage::getTransId() const { return m_txId; }

void TcrMessage::setTransId(int32_t txId) { m_txId = txId; }

std::chrono::milliseconds TcrMessage::getTimeout() const { return m_timeout; }

void TcrMessage::setTimeout(std::chrono::milliseconds timeout) {
  m_timeout = timeout;
}

void TcrMessage::skipParts(DataInput& input, int32_t numParts) {
  while (numParts > 0) {
    numParts--;
    int32_t partLen = input.readInt32();
    LOGDEBUG("TcrMessage::skipParts partLen= %d ", partLen);
    input.advanceCursor(partLen + 1);  // Skip the whole part including "isObj"
  }
}

void TcrMessage::readEventIdPart(DataInput& input, bool skip, int32_t parts) {
  // skip requested number of parts
  if (skip) {
    skipParts(input, parts);
  }

  // read the eventid part
  // read and ignore length
  input.readInt32();
  // read and ignore isObj
  input.read();

  m_eventid = std::dynamic_pointer_cast<EventId>(input.readObject());
}
std::shared_ptr<DSMemberForVersionStamp> TcrMessage::readDSMember(
    apache::geode::client::DataInput& input) {
  uint8_t typeidLen = input.read();
  if (typeidLen == 1) {
    auto typeidofMember = static_cast<DSCode>(input.read());
    if (typeidofMember != DSCode::InternalDistributedMember) {
      throw Exception(
          "Reading DSMember. Expecting type id 92 for "
          "InternalDistributedMember. ");
    }

    auto memId =
        std::shared_ptr<ClientProxyMembershipID>(new ClientProxyMembershipID());
    memId->fromData(input);
    return std::shared_ptr<DSMemberForVersionStamp>(memId);
  } else if (typeidLen == 2) {
    auto typeidofMember = input.readInt16();
    if (typeidofMember != static_cast<int16_t>(DSFid::DiskStoreId)) {
      throw Exception(
          "Reading DSMember. Expecting type id 2133 for DiskStoreId. ");
    }
    DiskStoreId* diskStore = new DiskStoreId();
    diskStore->fromData(input);
    return std::shared_ptr<DSMemberForVersionStamp>(diskStore);
  } else {
    throw Exception(
        "Reading DSMember. Expecting type id length as either one or two "
        "byte.");
  }
}
void TcrMessage::readHashMapForGCVersions(
    apache::geode::client::DataInput& input,
    std::shared_ptr<CacheableHashMap>& value) {
  uint8_t hashmaptypeid = input.read();
  if (hashmaptypeid != static_cast<int8_t>(DSCode::CacheableHashMap)) {
    throw Exception(
        "Reading HashMap For GC versions. Expecting type id of hash map. ");
  }
  int32_t len = input.readArrayLength();

  if (len > 0) {
    std::shared_ptr<CacheableKey> key;
    std::shared_ptr<Cacheable> val;
    for (int32_t index = 0; index < len; index++) {
      key = readDSMember(input);
      // read and ignore versionType
      input.read();

      auto valVersion = CacheableInt64::create(input.readInt64());
      auto keyPtr = std::dynamic_pointer_cast<CacheableKey>(key);
      auto valVersionPtr = std::dynamic_pointer_cast<Cacheable>(valVersion);

      if (value) {
        value->emplace(keyPtr, valVersionPtr);
      } else {
        throw Exception(
            "Inserting values in HashMap For GC versions. value must not be "
            "nullptr. ");
      }
    }
  }
}

void TcrMessage::readHashSetForGCVersions(
    apache::geode::client::DataInput& input,
    std::shared_ptr<CacheableHashSet>& value) {
  auto hashsettypeid = input.read();
  if (hashsettypeid != static_cast<int8_t>(DSCode::CacheableHashSet)) {
    throw Exception(
        "Reading HashSet For GC versions. Expecting type id of hash set. ");
  }

  auto len = input.readArrayLength();
  for (decltype(len) index = 0; index < len; index++) {
    auto keyPtr = std::dynamic_pointer_cast<CacheableKey>(input.readObject());
    value->insert(keyPtr);
  }
}

bool TcrMessageHelper::readExceptionPart(TcrMessage& msg, DataInput& input,
                                         uint8_t isLastChunk) {
  return msg.readExceptionPart(input, isLastChunk);
}

void TcrMessageHelper::skipParts(TcrMessage& msg, DataInput& input,
                                 int32_t numParts) {
  msg.skipParts(input, numParts);
}

TcrMessageHelper::ChunkObjectType TcrMessageHelper::readChunkPartHeader(
    TcrMessage& msg, DataInput& input, DSCode expectedFirstType,
    int32_t expectedPartType, const char* methodName, uint32_t& partLen,
    uint8_t isLastChunk) {
  partLen = input.readInt32();
  const auto isObj = input.readBoolean();

  if (partLen == 0) {
    // special null object is case for scalar query result
    return ChunkObjectType::NULL_OBJECT;
  } else if (!isObj) {
    // otherwise we're currently always expecting an object
    char exMsg[256];
    std::snprintf(exMsg, sizeof(exMsg),
                  "TcrMessageHelper::readChunkPartHeader: "
                  "%s: part is not object",
                  methodName);
    LOGDEBUG("%s ", exMsg);
    return ChunkObjectType::EXCEPTION;
  }

  auto rawByte = input.read();
  auto partType = static_cast<DSCode>(rawByte);
  auto compId = static_cast<int32_t>(partType);

  //  ugly hack to check for exception chunk
  if (partType == DSCode::JavaSerializable) {
    input.reset();
    if (TcrMessageHelper::readExceptionPart(msg, input, isLastChunk)) {
      msg.setMessageType(TcrMessage::EXCEPTION);
      return ChunkObjectType::EXCEPTION;
    } else {
      char exMsg[256];
      std::snprintf(exMsg, sizeof(exMsg),
                    "TcrMessageHelper::readChunkPartHeader: %s: cannot handle "
                    "java serializable object from server",
                    methodName);
      throw MessageException(exMsg);
    }
  } else if (partType == DSCode::NullObj) {
    // special null object is case for scalar query result
    return ChunkObjectType::NULL_OBJECT;
  }

  // TODO enum - wtf?
  if (expectedFirstType > DSCode::FixedIDDefault) {
    if (partType != expectedFirstType) {
      char exMsg[256];
      std::snprintf(exMsg, sizeof(exMsg),
                    "TcrMessageHelper::readChunkPartHeader: "
                    "%s: got unhandled object class = %" PRId8,
                    methodName, static_cast<int8_t>(partType));
      throw MessageException(exMsg);
    }
    // This is for GETALL
    if (expectedFirstType == DSCode::FixedIDShort) {
      compId = input.readInt16();
    }  // This is for QUERY or REGISTER INTEREST.
    else if (expectedFirstType == DSCode::FixedIDByte) {
      compId = input.read();
    }
  }
  if (compId != expectedPartType) {
    char exMsg[256];
    std::snprintf(exMsg, sizeof(exMsg),
                  "TcrMessageHelper::readChunkPartHeader: "
                  "%s: got unhandled object type = %d, expected = %d, raw = %d",
                  methodName, compId, expectedPartType, rawByte);
    throw MessageException(exMsg);
  }
  return ChunkObjectType::OBJECT;
}

TcrMessageHelper::ChunkObjectType TcrMessageHelper::readChunkPartHeader(
    TcrMessage& msg, DataInput& input, const char* methodName,
    uint32_t& partLen, uint8_t isLastChunk) {
  partLen = input.readInt32();
  const auto isObj = input.readBoolean();

  if (partLen == 0) {
    // special null object is case for scalar query result
    return ChunkObjectType::NULL_OBJECT;
  } else if (!isObj) {
    // otherwise we're currently always expecting an object
    char exMsg[256];
    std::snprintf(exMsg, 255,
                  "TcrMessageHelper::readChunkPartHeader: "
                  "%s: part is not object",
                  methodName);
    throw MessageException(exMsg);
  }

  const auto partType = static_cast<const DSCode>(input.read());
  //  ugly hack to check for exception chunk
  if (partType == DSCode::JavaSerializable) {
    input.reset();
    if (TcrMessageHelper::readExceptionPart(msg, input, isLastChunk)) {
      msg.setMessageType(TcrMessage::EXCEPTION);
      return ChunkObjectType::EXCEPTION;
    } else {
      char exMsg[256];
      std::snprintf(exMsg, 255,
                    "TcrMessageHelper::readChunkPartHeader: %s: cannot handle "
                    "java serializable object from server",
                    methodName);
      throw MessageException(exMsg);
    }
  } else if (partType == DSCode::NullObj) {
    // special null object is case for scalar query result
    return ChunkObjectType::NULL_OBJECT;
  }
  return ChunkObjectType::OBJECT;
}

#ifdef _SOLARIS
#pragma error_messages(default, SEC_UNINITIALIZED_MEM_READ)
#endif

}  // namespace client
}  // namespace geode
}  // namespace apache
