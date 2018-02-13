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

#ifndef GEODE_TCRMESSAGE_H_
#define GEODE_TCRMESSAGE_H_

#include <atomic>
#include <string>
#include <map>
#include <vector>

#include <ace/OS.h>

#include <geode/internal/geode_globals.hpp>
#include <geode/Serializable.hpp>
#include <geode/CacheableKey.hpp>
#include <geode/CacheableString.hpp>
#include <geode/DataOutput.hpp>
#include <geode/DataInput.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/CacheableBuiltins.hpp>
#include <geode/CqState.hpp>

#include "InterestResultPolicy.hpp"
#include "EventId.hpp"
#include "EventIdMap.hpp"
#include "TcrChunkedContext.hpp"
#include "GeodeTypeIdsImpl.hpp"
#include "BucketServerLocation.hpp"
#include "FixedPartitionAttributesImpl.hpp"
#include "VersionTag.hpp"
#include "VersionedCacheableObjectPartList.hpp"
#include "SerializationRegistry.hpp"

namespace apache {
namespace geode {
namespace client {

class TcrMessage;
class ThinClientRegion;
class ThinClientBaseDM;
class TcrMessageHelper;
class TcrConnection;
class TcrMessagePing;

class _GEODE_EXPORT TcrMessage {
 private:
  inline static void writeInt(uint8_t* buffer, uint16_t value);
  inline static void writeInt(uint8_t* buffer, uint32_t value);
  inline static void readInt(uint8_t* buffer, uint16_t* value);
  inline static void readInt(uint8_t* buffer, uint32_t* value);

 public:
  typedef enum {
    /* Server couldn't read message; handle it like a server side
       exception that needs retries */
    NOT_PUBLIC_API_WITH_TIMEOUT = -2,
    INVALID = -1,
    REQUEST = 0,
    RESPONSE /* 1 */,
    EXCEPTION /* 2 */,
    REQUEST_DATA_ERROR /* 3 */,
    DATA_NOT_FOUND_ERROR /* 4 Not in use */,
    PING /* 5 */,
    REPLY /* 6 */,
    PUT /* 7 */,
    PUT_DATA_ERROR /* 8 */,
    DESTROY /* 9 */,
    DESTROY_DATA_ERROR /* 10 */,
    DESTROY_REGION /* 11 */,
    DESTROY_REGION_DATA_ERROR /* 12 */,
    CLIENT_NOTIFICATION /* 13 */,
    UPDATE_CLIENT_NOTIFICATION /* 14 */,
    LOCAL_INVALIDATE /* 15 */,
    LOCAL_DESTROY /* 16 */,
    LOCAL_DESTROY_REGION /* 17 */,
    CLOSE_CONNECTION /* 18 */,
    PROCESS_BATCH /* 19 */,
    REGISTER_INTEREST /* 20 */,
    REGISTER_INTEREST_DATA_ERROR /* 21 */,
    UNREGISTER_INTEREST /* 22 */,
    UNREGISTER_INTEREST_DATA_ERROR /* 23 */,
    REGISTER_INTEREST_LIST /* 24 */,
    UNREGISTER_INTEREST_LIST /* 25 */,
    UNKNOWN_MESSAGE_TYPE_ERROR /* 26 */,
    LOCAL_CREATE /* 27 */,
    LOCAL_UPDATE /* 28 */,
    CREATE_REGION /* 29 */,
    CREATE_REGION_DATA_ERROR /* 30 */,
    MAKE_PRIMARY /* 31 */,
    RESPONSE_FROM_PRIMARY /* 32 */,
    RESPONSE_FROM_SECONDARY /* 33 */,
    QUERY /* 34 */,
    QUERY_DATA_ERROR /* 35 */,
    CLEAR_REGION /* 36 */,
    CLEAR_REGION_DATA_ERROR /* 37 */,
    CONTAINS_KEY /* 38 */,
    CONTAINS_KEY_DATA_ERROR /* 39 */,
    KEY_SET /* 40 */,
    KEY_SET_DATA_ERROR /* 41 */,
    EXECUTECQ_MSG_TYPE /* 42 */,
    EXECUTECQ_WITH_IR_MSG_TYPE /*43 */,
    STOPCQ_MSG_TYPE /*44*/,
    CLOSECQ_MSG_TYPE /*45 */,
    CLOSECLIENTCQS_MSG_TYPE /*46*/,
    CQDATAERROR_MSG_TYPE /*47 */,
    GETCQSTATS_MSG_TYPE /*48 */,
    MONITORCQ_MSG_TYPE /*49 */,
    CQ_EXCEPTION_TYPE /*50 */,
    REGISTER_INSTANTIATORS = 51 /* 51 */,
    PERIODIC_ACK = 52 /* 52 */,
    CLIENT_READY /* 53 */,
    CLIENT_MARKER /* 54 */,
    INVALIDATE_REGION /* 55 */,
    PUTALL /* 56 */,
    GET_ALL_DATA_ERROR = 58 /* 58 */,
    EXECUTE_REGION_FUNCTION = 59 /* 59 */,
    EXECUTE_REGION_FUNCTION_RESULT /* 60 */,
    EXECUTE_REGION_FUNCTION_ERROR /* 61 */,
    EXECUTE_FUNCTION /* 62 */,
    EXECUTE_FUNCTION_RESULT /* 63 */,
    EXECUTE_FUNCTION_ERROR /* 64 */,
    CLIENT_REGISTER_INTEREST = 65 /* 65 */,
    CLIENT_UNREGISTER_INTEREST = 66,
    REGISTER_DATASERIALIZERS = 67,
    REQUEST_EVENT_VALUE = 68,
    REQUEST_EVENT_VALUE_ERROR = 69,             /*69*/
    PUT_DELTA_ERROR = 70,                       /*70*/
    GET_CLIENT_PR_METADATA = 71,                /*71*/
    RESPONSE_CLIENT_PR_METADATA = 72,           /*72*/
    GET_CLIENT_PARTITION_ATTRIBUTES = 73,       /*73*/
    RESPONSE_CLIENT_PARTITION_ATTRIBUTES = 74,  /*74*/
    GET_CLIENT_PR_METADATA_ERROR = 75,          /*75*/
    GET_CLIENT_PARTITION_ATTRIBUTES_ERROR = 76, /*76*/
    USER_CREDENTIAL_MESSAGE = 77,
    REMOVE_USER_AUTH = 78,
    EXECUTE_REGION_FUNCTION_SINGLE_HOP = 79,
    QUERY_WITH_PARAMETERS = 80,
    SIZE = 81,
    SIZE_ERROR = 82,
    INVALIDATE = 83,
    INVALIDATE_ERROR = 84,
    COMMIT = 85,
    COMMIT_ERROR = 86,
    ROLLBACK = 87,
    TX_FAILOVER = 88,
    GET_ENTRY = 89,
    TX_SYNCHRONIZATION = 90,
    GET_FUNCTION_ATTRIBUTES = 91,
    GET_PDX_TYPE_BY_ID = 92,
    GET_PDX_ID_FOR_TYPE = 93,
    ADD_PDX_TYPE = 94,
    ADD_PDX_ENUM = 96,
    GET_PDX_ID_FOR_ENUM = 97,
    GET_PDX_ENUM_BY_ID = 98,
    SERVER_TO_CLIENT_PING = 99,
    // GATEWAY_RECEIVER_COMMAND = 99,
    GET_ALL_70 = 100,
    TOMBSTONE_OPERATION = 103,
    GETDURABLECQS_MSG_TYPE = 105,
    GET_DURABLE_CQS_DATA_ERROR = 106,
    GET_ALL_WITH_CALLBACK = 107,
    PUT_ALL_WITH_CALLBACK = 108,
    REMOVE_ALL = 109

  } MsgType;

  static bool isKeepAlive() { return *m_keepalive > 0; }
  static bool isUserInitiativeOps(const TcrMessage& msg) {
    int32_t msgType = msg.getMessageType();

    if (!msg.isMetaRegion() &&
        !(msgType == TcrMessage::PING || msgType == TcrMessage::PERIODIC_ACK ||
          msgType == TcrMessage::MAKE_PRIMARY ||
          msgType == TcrMessage::CLOSE_CONNECTION ||
          msgType == TcrMessage::CLIENT_READY ||
          msgType == TcrMessage::INVALID ||
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
  static std::shared_ptr<VersionTag> readVersionTagPart(
      DataInput& input, uint16_t endpointMemId,
      MemberListForVersionStamp& memberListForVersionStamp);

  /* constructors */
  void setData(const char* bytearray, int32_t len, uint16_t memId,
               const SerializationRegistry& serializationRegistry,
               MemberListForVersionStamp& memberListForVersionStamp);

  void startProcessChunk(ACE_Semaphore& finalizeSema);
  // nullptr chunk means that this is the last chunk
  void processChunk(const uint8_t* chunk, int32_t chunkLen,
                    uint16_t endpointmemId,
                    const uint8_t isLastChunkAndisSecurityHeader = 0x00);
  /* For creating a region on the java server */
  /* Note through this you can only create a sub region on the cache server */
  /* also for creating REGISTER_INTEREST regex request */
  // TcrMessage( TcrMessage::MsgType msgType, const std::string& str1,
  //  const std::string& str2, InterestResultPolicy interestPolicy =
  //  InterestResultPolicy::NONE, bool isDurable = false, bool isCachingEnabled
  //  = false, bool receiveValues = true, ThinClientBaseDM *connectionDM =
  //  nullptr);

  void InitializeGetallMsg(
      const std::shared_ptr<Serializable>& aCallbackArgument);
  // for multiuser cache close

  // Updates the early ack byte of the message to reflect that it is a retry op
  void updateHeaderForRetry();

  inline const std::vector<std::shared_ptr<CacheableKey>>* getKeys() const {
    return m_keyList;
  }

  inline const std::string& getRegex() const { return m_regex; }

  inline InterestResultPolicy getInterestResultPolicy() const {
    if (m_interestPolicy == 2) {
      return InterestResultPolicy::KEYS_VALUES;
    } else if (m_interestPolicy == 1) {
      return InterestResultPolicy::KEYS;
    } else {
      return InterestResultPolicy::NONE;
    }
  }

  const std::string& getPoolName() const;

  /**
   * Whether the request is meant to be
   * sent to PR primary node for single hop.
   */
  inline bool forPrimary() const {
    return m_msgType == TcrMessage::PUT || m_msgType == TcrMessage::DESTROY ||
           m_msgType == TcrMessage::EXECUTE_REGION_FUNCTION;
  }

  inline void initCqMap() { m_cqs = new std::map<std::string, int>(); }

  inline bool forSingleHop() const {
    return m_msgType == TcrMessage::PUT || m_msgType == TcrMessage::DESTROY ||
           m_msgType == TcrMessage::REQUEST ||
           m_msgType == TcrMessage::GET_ALL_70 ||
           m_msgType == TcrMessage::GET_ALL_WITH_CALLBACK ||
           m_msgType == TcrMessage::EXECUTE_REGION_FUNCTION ||
           m_msgType == TcrMessage::PUTALL ||
           m_msgType == TcrMessage::PUT_ALL_WITH_CALLBACK;
  }

  inline bool forTransaction() const { return m_txId != -1; }

  /*
  inline void getSingleHopFlags(bool& forSingleHop, bool& forPrimary) const
  {
    if (m_msgType == TcrMessage::PUT ||
         m_msgType == TcrMessage::DESTROY ||
         m_msgType == TcrMessage::REQUEST) {

           forSingleHop = true;

           if (m_msgType == TcrMessage::REQUEST) {
             forPrimary = false;
           } else {
             forPrimary = true;
           }

    } else {

      forSingleHop = false;
      forPrimary = false;

    }
  }
  */

  /* destroy the connection */
  virtual ~TcrMessage();

  const std::string& getRegionName() const;
  Region* getRegion() const;
  int32_t getMessageType() const;
  void setMessageType(int32_t msgType);
  void setMessageTypeRequest(int32_t msgType);  // the msgType of the request
                                                // that was made if this is a
                                                // reply
  int32_t getMessageTypeRequest() const;
  std::shared_ptr<CacheableKey> getKey() const;
  const std::shared_ptr<CacheableKey>& getKeyRef() const;
  std::shared_ptr<Cacheable> getValue() const;
  const std::shared_ptr<Cacheable>& getValueRef() const;
  std::shared_ptr<Cacheable> getCallbackArgument() const;
  const std::shared_ptr<Cacheable>& getCallbackArgumentRef() const;

  const std::map<std::string, int>* getCqs() const;
  bool getBoolValue() const { return m_boolValue; };
  inline const char* getException() {
    exceptionMessage = Utils::nullSafeToString(m_value);
    return exceptionMessage.c_str();
  }

  const char* getMsgData() const;
  const char* getMsgHeader() const;
  const char* getMsgBody() const;
  uint32_t getMsgLength() const;
  uint32_t getMsgBodyLength() const;
  std::shared_ptr<EventId> getEventId() const;

  int32_t getTransId() const;
  void setTransId(int32_t txId);

  std::chrono::milliseconds getTimeout() const;
  void setTimeout(std::chrono::milliseconds timeout);

  /* we need a static method to generate ping */
  /* The caller should not delete the message since it is global. */
  static TcrMessagePing* getPingMessage(Cache* cache);
  static TcrMessage* getAllEPDisMess();
  /* we need a static method to generate close connection message */
  /* The caller should not delete the message since it is global. */
  static TcrMessage* getCloseConnMessage(Cache* cache);
  static void setKeepAlive(bool keepalive);
  bool isDurable() const { return m_isDurable; }
  bool receiveValues() const { return m_receiveValues; }
  bool hasCqPart() const { return m_hasCqsPart; }
  uint32_t getMessageTypeForCq() const { return m_msgTypeForCq; }
  bool isInterestListPassed() const { return m_isInterestListPassed; }
  bool shouldIgnore() const { return m_shouldIgnore; }
  int8_t getMetaDataVersion() const { return m_metaDataVersion; }
  uint32_t getEntryNotFound() const { return m_entryNotFound; }
  int8_t getserverGroupVersion() const { return m_serverGroupVersion; }
  std::vector<int8_t>* getFunctionAttributes() { return m_functionAttributes; }

  // set the DM for chunked response messages
  void setDM(ThinClientBaseDM* dm) { m_tcdm = dm; }

  ThinClientBaseDM* getDM() { return m_tcdm; }
  // set the chunked response handler
  void setChunkedResultHandler(TcrChunkedResult* chunkedResult) {
    this->m_isLastChunkAndisSecurityHeader = 0x0;
    m_chunkedResult = chunkedResult;
  }
  TcrChunkedResult* getChunkedResultHandler() { return m_chunkedResult; }
  void setVersionedObjectPartList(
      std::shared_ptr<VersionedCacheableObjectPartList> versionObjPartListptr) {
    m_versionObjPartListptr = versionObjPartListptr;
  }

  std::shared_ptr<VersionedCacheableObjectPartList>
  getVersionedObjectPartList() {
    return m_versionObjPartListptr;
  }

  DataInput* getDelta() { return m_delta.get(); }

  //  getDeltaBytes( ) is called *only* by CqService, returns a CacheableBytes
  //  that
  // takes ownership of delta bytes.
  std::shared_ptr<CacheableBytes> getDeltaBytes() {
    if (m_deltaBytes == nullptr) {
      return nullptr;
    }
    auto retVal =
        CacheableBytes::create(std::vector<int8_t>(m_deltaBytes, m_deltaBytes + m_deltaBytesLen));
    m_deltaBytes = nullptr;
    return retVal;
  }

  bool hasDelta() { return (m_delta != nullptr); }

  void addSecurityPart(int64_t connectionId, int64_t unique_id,
                       TcrConnection* conn);

  void addSecurityPart(int64_t connectionId, TcrConnection* conn);

  int64_t getConnectionId(TcrConnection* conn);

  int64_t getUniqueId(TcrConnection* conn);

  void createUserCredentialMessage(TcrConnection* conn);

  void readSecureObjectPart(DataInput& input, bool defaultString = false,
                            bool isChunk = false,
                            uint8_t isLastChunkWithSecurity = 0);

  void readUniqueIDObjectPart(DataInput& input);

  void setMetaRegion(bool isMetaRegion) { m_isMetaRegion = isMetaRegion; }

  bool isMetaRegion() const { return m_isMetaRegion; }

  int32_t getNumBuckets() const { return m_bucketCount; }

  const std::string& getColocatedWith() const { return m_colocatedWith; }

  const std::string& getPartitionResolver() const {
    return m_partitionResolverName;
  }

  std::vector<std::vector<std::shared_ptr<BucketServerLocation>>>*
  getMetadata() {
    return m_metadata;
  }

  std::vector<std::shared_ptr<FixedPartitionAttributesImpl>>* getFpaSet() {
    return m_fpaSet;
  }

  std::shared_ptr<CacheableHashSet> getFailedNode() { return m_failedNode; }

  bool isCallBackArguement() const { return m_isCallBackArguement; }

  void setCallBackArguement(bool aCallBackArguement) {
    m_isCallBackArguement = aCallBackArguement;
  }

  void setBucketServerLocation(
      std::shared_ptr<BucketServerLocation> serverLocation) {
    m_bucketServerLocation = serverLocation;
  }
  void setVersionTag(std::shared_ptr<VersionTag> versionTag) {
    m_versionTag = versionTag;
  }
  std::shared_ptr<VersionTag> getVersionTag() { return m_versionTag; }
  uint8_t hasResult() const { return m_hasResult; }
  std::shared_ptr<CacheableHashMap> getTombstoneVersions() const {
    return m_tombstoneVersions;
  }
  std::shared_ptr<CacheableHashSet> getTombstoneKeys() const {
    return m_tombstoneKeys;
  }

  bool isFEAnotherHop();

 protected:
  TcrMessage()
      : m_feAnotherHop(false),
        m_connectionIDBytes(nullptr),
        isSecurityOn(false),
        m_isLastChunkAndisSecurityHeader(0),
        m_isSecurityHeaderAdded(false),
        m_creds(),
        m_securityHeaderLength(0),
        m_isMetaRegion(false),
        exceptionMessage(),
        m_request(nullptr),
        m_msgType(TcrMessage::INVALID),
        m_msgLength(-1),
        m_msgTypeRequest(0),
        m_txId(-1),
        m_decodeAll(false),
        m_tcdm(nullptr),
        m_chunkedResult(nullptr),
        m_keyList(nullptr),
        m_key(),
        m_value(nullptr),
        m_failedNode(),
        m_callbackArgument(nullptr),
        m_versionTag(),
        m_eventid(nullptr),
        m_regionName("INVALID_REGION_NAME"),
        m_region(nullptr),
        m_regex(),
        m_interestPolicy(0),
        m_timeout(15 /*DEFAULT_TIMEOUT_SECONDS*/),
        m_isDurable(false),
        m_receiveValues(false),
        m_hasCqsPart(false),
        m_isInterestListPassed(false),
        m_shouldIgnore(false),
        m_metaDataVersion(0),
        m_serverGroupVersion(0),
        m_bucketServerLocations(),
        m_metadata(),
        m_bucketCount(0),
        m_colocatedWith(),
        m_partitionResolverName(),
        m_vectorPtr(),
        m_numCqPart(0),
        m_msgTypeForCq(0),
        m_cqs(nullptr),
        m_messageResponseTimeout(-1),
        m_boolValue(0),
        m_delta(nullptr),
        m_deltaBytes(nullptr),
        m_deltaBytesLen(0),
        m_isCallBackArguement(false),
        m_bucketServerLocation(nullptr),
        m_entryNotFound(0),
        m_fpaSet(),
        m_functionAttributes(),
        m_hasResult(0),
        m_tombstoneVersions(),
        m_tombstoneKeys(),
        m_versionObjPartListptr() {}

  void handleSpecialFECase();
  bool m_feAnotherHop;
  void writeBytesOnly(const std::shared_ptr<Serializable>& se);
  std::shared_ptr<Serializable> readCacheableBytes(DataInput& input,
                                                   int lenObj);
  std::shared_ptr<Serializable> readCacheableString(DataInput& input,
                                                    int lenObj);

  static std::atomic<int32_t> m_transactionId;
  static uint8_t* m_keepalive;
  const static int m_flag_empty;
  const static int m_flag_concurrency_checks;

  std::shared_ptr<CacheableBytes> m_connectionIDBytes;
  bool isSecurityOn;
  uint8_t m_isLastChunkAndisSecurityHeader;
  bool m_isSecurityHeaderAdded;
  std::shared_ptr<Properties> m_creds;
  int32_t m_securityHeaderLength;
  bool m_isMetaRegion;

  std::string exceptionMessage;

  TcrMessage(const TcrMessage&) = delete;
  TcrMessage& operator=(const TcrMessage&) = delete;

  // some private methods to handle things internally.
  void handleByteArrayResponse(
      const char* bytearray, int32_t len, uint16_t endpointMemId,
      const SerializationRegistry& serializationRegistry,
      MemberListForVersionStamp& memberListForVersionStamp);
  void readObjectPart(DataInput& input, bool defaultString = false);
  void readFailedNodePart(DataInput& input, bool defaultString = false);
  void readCallbackObjectPart(DataInput& input, bool defaultString = false);
  void readKeyPart(DataInput& input);
  void readBooleanPartAsObject(DataInput& input, bool* boolVal);
  void readIntPart(DataInput& input, uint32_t* intValue);
  void readLongPart(DataInput& input, uint64_t* intValue);
  bool readExceptionPart(DataInput& input, uint8_t isLastChunk,
                         bool skipFirstPart = true);
  void readVersionTag(DataInput& input, uint16_t endpointMemId,
                      MemberListForVersionStamp& memberListForVersionStamp);
  void readOldValue(DataInput& input);
  void readPrMetaData(DataInput& input);
  void writeObjectPart(const std::shared_ptr<Serializable>& se,
                       bool isDelta = false, bool callToData = false,
                       const std::vector<std::shared_ptr<CacheableKey>>*
                           getAllKeyList = nullptr);
  void writeHeader(uint32_t msgType, uint32_t numOfParts);
  void writeRegionPart(const std::string& regionName);
  void writeStringPart(const std::string& str);
  void writeEventIdPart(int reserveSize = 0,
                        bool fullValueAfterDeltaFail = false);
  void writeMessageLength();
  void writeInterestResultPolicyPart(InterestResultPolicy policy);
  void writeIntPart(int32_t intValue);
  void writeBytePart(uint8_t byteValue);
  void writeMillisecondsPart(std::chrono::milliseconds millis);
  void writeByteAndTimeOutPart(uint8_t byteValue,
                               std::chrono::milliseconds timeout);
  void chunkSecurityHeader(int skipParts, const uint8_t* bytes, int32_t len,
                           uint8_t isLastChunkAndSecurityHeader);

  void readEventIdPart(DataInput& input, bool skip = false,
                       int32_t parts = 1);  // skip num parts then read eventid

  void skipParts(DataInput& input, int32_t numParts = 1);
  void readStringPart(DataInput& input, uint32_t* len, char** str);
  void readCqsPart(DataInput& input);
  void readHashMapForGCVersions(apache::geode::client::DataInput& input,
                                std::shared_ptr<CacheableHashMap>& value);
  void readHashSetForGCVersions(apache::geode::client::DataInput& input,
                                std::shared_ptr<CacheableHashSet>& value);
  std::shared_ptr<DSMemberForVersionStamp> readDSMember(
      apache::geode::client::DataInput& input);
  std::unique_ptr<DataOutput> m_request;
  int32_t m_msgType;
  int32_t m_msgLength;
  int32_t m_msgTypeRequest;  // the msgType of the request if this TcrMessage is
                             // a reply msg

  int32_t m_txId;
  bool m_decodeAll;  // used only when decoding reply message, if false, decode
                     // header only

  // the associated region that is handling processing of chunked responses
  ThinClientBaseDM* m_tcdm;
  TcrChunkedResult* m_chunkedResult;

  const std::vector<std::shared_ptr<CacheableKey>>* m_keyList;
  std::shared_ptr<CacheableKey> m_key;
  std::shared_ptr<Cacheable> m_value;
  std::shared_ptr<CacheableHashSet> m_failedNode;
  std::shared_ptr<Cacheable> m_callbackArgument;
  std::shared_ptr<VersionTag> m_versionTag;
  std::shared_ptr<EventId> m_eventid;
  std::string m_regionName;
  const Region* m_region;
  std::string m_regex;
  char m_interestPolicy;
  std::chrono::milliseconds m_timeout;
  bool m_isDurable;
  bool m_receiveValues;
  bool m_hasCqsPart;
  bool m_isInterestListPassed;
  bool m_shouldIgnore;
  int8_t m_metaDataVersion;
  int8_t m_serverGroupVersion;
  std::vector<std::shared_ptr<BucketServerLocation>> m_bucketServerLocations;
  std::vector<std::vector<std::shared_ptr<BucketServerLocation>>>* m_metadata;
  int32_t m_bucketCount;
  std::string m_colocatedWith;
  std::string m_partitionResolverName;
  std::shared_ptr<CacheableVector> m_vectorPtr;
  uint32_t m_numCqPart;
  uint32_t m_msgTypeForCq;  // new part since 7.0 for cq event message type.
  std::map<std::string, int>* m_cqs;
  std::chrono::milliseconds m_messageResponseTimeout;
  bool m_boolValue;
  std::unique_ptr<DataInput> m_delta;
  int8_t* m_deltaBytes;
  int32_t m_deltaBytesLen;
  bool m_isCallBackArguement;
  std::shared_ptr<BucketServerLocation> m_bucketServerLocation;
  uint32_t m_entryNotFound;
  std::vector<std::shared_ptr<FixedPartitionAttributesImpl>>* m_fpaSet;
  std::vector<int8_t>* m_functionAttributes;
  uint8_t m_hasResult;
  std::shared_ptr<CacheableHashMap> m_tombstoneVersions;
  std::shared_ptr<CacheableHashSet> m_tombstoneKeys;
  /*Added this member for PutALL versioning changes*/
  std::shared_ptr<VersionedCacheableObjectPartList> m_versionObjPartListptr;

  friend class TcrMessageHelper;
};

class TcrMessageDestroyRegion : public TcrMessage {
 public:
  TcrMessageDestroyRegion(
      std::unique_ptr<DataOutput> dataOutput, const Region* region,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::chrono::milliseconds messageResponsetimeout,
      ThinClientBaseDM* connectionDM);

  virtual ~TcrMessageDestroyRegion() {}

 private:
};

class TcrMessageClearRegion : public TcrMessage {
 public:
  TcrMessageClearRegion(std::unique_ptr<DataOutput> dataOutput,
                        const Region* region,
                        const std::shared_ptr<Serializable>& aCallbackArgument,
                        std::chrono::milliseconds messageResponsetimeout,
                        ThinClientBaseDM* connectionDM);

  virtual ~TcrMessageClearRegion() {}

 private:
};

class TcrMessageQuery : public TcrMessage {
 public:
  TcrMessageQuery(std::unique_ptr<DataOutput> dataOutput,
                  const std::string& regionName,
                  std::chrono::milliseconds messageResponsetimeout,
                  ThinClientBaseDM* connectionDM);

  virtual ~TcrMessageQuery() {}

 private:
};

class TcrMessageStopCQ : public TcrMessage {
 public:
  TcrMessageStopCQ(std::unique_ptr<DataOutput> dataOutput,
                   const std::string& regionName,
                   std::chrono::milliseconds messageResponsetimeout,
                   ThinClientBaseDM* connectionDM);

  virtual ~TcrMessageStopCQ() {}

 private:
};

class TcrMessageCloseCQ : public TcrMessage {
 public:
  TcrMessageCloseCQ(std::unique_ptr<DataOutput> dataOutput,
                    const std::string& regionName,
                    std::chrono::milliseconds messageResponsetimeout,
                    ThinClientBaseDM* connectionDM);

  virtual ~TcrMessageCloseCQ() {}

 private:
};

class TcrMessageQueryWithParameters : public TcrMessage {
 public:
  TcrMessageQueryWithParameters(
      std::unique_ptr<DataOutput> dataOutput, const std::string& regionName,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<CacheableVector> paramList,
      std::chrono::milliseconds messageResponsetimeout,
      ThinClientBaseDM* connectionDM);

  virtual ~TcrMessageQueryWithParameters() {}

 private:
};

class TcrMessageContainsKey : public TcrMessage {
 public:
  TcrMessageContainsKey(std::unique_ptr<DataOutput> dataOutput,
                        const Region* region,
                        const std::shared_ptr<CacheableKey>& key,
                        const std::shared_ptr<Serializable>& aCallbackArgument,
                        bool isContainsKey, ThinClientBaseDM* connectionDM);

  virtual ~TcrMessageContainsKey() {}

 private:
};

class TcrMessageGetDurableCqs : public TcrMessage {
 public:
  TcrMessageGetDurableCqs(std::unique_ptr<DataOutput> dataOutput,
                          ThinClientBaseDM* connectionDM);

  virtual ~TcrMessageGetDurableCqs() {}

 private:
};

class TcrMessageRequest : public TcrMessage {
 public:
  TcrMessageRequest(std::unique_ptr<DataOutput> dataOutput,
                    const Region* region,
                    const std::shared_ptr<CacheableKey>& key,
                    const std::shared_ptr<Serializable>& aCallbackArgument,
                    ThinClientBaseDM* connectionDM = nullptr);

  virtual ~TcrMessageRequest() {}

 private:
};

class TcrMessageInvalidate : public TcrMessage {
 public:
  TcrMessageInvalidate(std::unique_ptr<DataOutput> dataOutput,
                       const Region* region,
                       const std::shared_ptr<CacheableKey>& key,
                       const std::shared_ptr<Serializable>& aCallbackArgument,
                       ThinClientBaseDM* connectionDM = nullptr);

 private:
};

class TcrMessageDestroy : public TcrMessage {
 public:
  TcrMessageDestroy(std::unique_ptr<DataOutput> dataOutput,
                    const Region* region,
                    const std::shared_ptr<CacheableKey>& key,
                    const std::shared_ptr<Cacheable>& value,
                    const std::shared_ptr<Serializable>& aCallbackArgument,
                    ThinClientBaseDM* connectionDM = nullptr);

 private:
};

class TcrMessageRegisterInterestList : public TcrMessage {
 public:
  TcrMessageRegisterInterestList(
      std::unique_ptr<DataOutput> dataOutput, const Region* region,
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      bool isDurable = false, bool isCachingEnabled = false,
      bool receiveValues = true,
      InterestResultPolicy interestPolicy = InterestResultPolicy::NONE,
      ThinClientBaseDM* connectionDM = nullptr);

  virtual ~TcrMessageRegisterInterestList() {}

 private:
};

class TcrMessageUnregisterInterestList : public TcrMessage {
 public:
  TcrMessageUnregisterInterestList(
      std::unique_ptr<DataOutput> dataOutput, const Region* region,
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      bool isDurable = false, bool isCachingEnabled = false,
      bool receiveValues = true,
      InterestResultPolicy interestPolicy = InterestResultPolicy::NONE,
      ThinClientBaseDM* connectionDM = nullptr);

  virtual ~TcrMessageUnregisterInterestList() {}

 private:
};

class TcrMessagePut : public TcrMessage {
 public:
  TcrMessagePut(std::unique_ptr<DataOutput> dataOutput, const Region* region,
                const std::shared_ptr<CacheableKey>& key,
                const std::shared_ptr<Cacheable>& value,
                const std::shared_ptr<Serializable>& aCallbackArgument,
                bool isDelta = false, ThinClientBaseDM* connectionDM = nullptr,
                bool isMetaRegion = false, bool fullValueAfterDeltaFail = false,
                const char* regionName = nullptr);

  virtual ~TcrMessagePut() {}

 private:
};

class TcrMessageCreateRegion : public TcrMessage {
 public:
  TcrMessageCreateRegion(
      std::unique_ptr<DataOutput> dataOutput, const std::string& str1,
      const std::string& str2,
      InterestResultPolicy interestPolicy = InterestResultPolicy::NONE,
      bool isDurable = false, bool isCachingEnabled = false,
      bool receiveValues = true, ThinClientBaseDM* connectionDM = nullptr);

  virtual ~TcrMessageCreateRegion() {}

 private:
};

class TcrMessageRegisterInterest : public TcrMessage {
 public:
  TcrMessageRegisterInterest(
      std::unique_ptr<DataOutput> dataOutput, const std::string& str1,
      const std::string& str2,
      InterestResultPolicy interestPolicy = InterestResultPolicy::NONE,
      bool isDurable = false, bool isCachingEnabled = false,
      bool receiveValues = true, ThinClientBaseDM* connectionDM = nullptr);

  virtual ~TcrMessageRegisterInterest() {}

 private:
};

class TcrMessageUnregisterInterest : public TcrMessage {
 public:
  TcrMessageUnregisterInterest(
      std::unique_ptr<DataOutput> dataOutput, const std::string& str1,
      const std::string& str2,
      InterestResultPolicy interestPolicy = InterestResultPolicy::NONE,
      bool isDurable = false, bool isCachingEnabled = false,
      bool receiveValues = true, ThinClientBaseDM* connectionDM = nullptr);

  virtual ~TcrMessageUnregisterInterest() {}

 private:
};

class TcrMessageTxSynchronization : public TcrMessage {
 public:
  TcrMessageTxSynchronization(std::unique_ptr<DataOutput> dataOutput,
                              int ordinal, int txid, int status);

  virtual ~TcrMessageTxSynchronization() {}

 private:
};

class TcrMessageClientReady : public TcrMessage {
 public:
  TcrMessageClientReady(std::unique_ptr<DataOutput> dataOutput);

  virtual ~TcrMessageClientReady() {}

 private:
};

class TcrMessageCommit : public TcrMessage {
 public:
  TcrMessageCommit(std::unique_ptr<DataOutput> dataOutput);

  virtual ~TcrMessageCommit() {}

 private:
};

class TcrMessageRollback : public TcrMessage {
 public:
  TcrMessageRollback(std::unique_ptr<DataOutput> dataOutput);

  virtual ~TcrMessageRollback() {}

 private:
};

class TcrMessageTxFailover : public TcrMessage {
 public:
  TcrMessageTxFailover(std::unique_ptr<DataOutput> dataOutput);

  virtual ~TcrMessageTxFailover() {}

 private:
};

class TcrMessageMakePrimary : public TcrMessage {
 public:
  TcrMessageMakePrimary(std::unique_ptr<DataOutput> dataOutput,
                        bool processedMarker);

  virtual ~TcrMessageMakePrimary() {}

 private:
};

class TcrMessagePutAll : public TcrMessage {
 public:
  TcrMessagePutAll(std::unique_ptr<DataOutput> dataOutput, const Region* region,
                   const HashMapOfCacheable& map,
                   std::chrono::milliseconds messageResponsetimeout,
                   ThinClientBaseDM* connectionDM,
                   const std::shared_ptr<Serializable>& aCallbackArgument);

  virtual ~TcrMessagePutAll() {}

 private:
};

class TcrMessageRemoveAll : public TcrMessage {
 public:
  TcrMessageRemoveAll(std::unique_ptr<DataOutput> dataOutput,
                      const Region* region,
                      const std::vector<std::shared_ptr<CacheableKey>>& keys,
                      const std::shared_ptr<Serializable>& aCallbackArgument,
                      ThinClientBaseDM* connectionDM = nullptr);

  virtual ~TcrMessageRemoveAll() {}

 private:
};

class TcrMessageExecuteCq : public TcrMessage {
 public:
  TcrMessageExecuteCq(std::unique_ptr<DataOutput> dataOutput,
                      const std::string& str1, const std::string& str2,
                      CqState state, bool isDurable,
                      ThinClientBaseDM* connectionDM);

  virtual ~TcrMessageExecuteCq() {}

 private:
};

class TcrMessageExecuteCqWithIr : public TcrMessage {
 public:
  TcrMessageExecuteCqWithIr(std::unique_ptr<DataOutput> dataOutput,
                            const std::string& str1, const std::string& str2,
                            CqState state, bool isDurable,
                            ThinClientBaseDM* connectionDM);

  virtual ~TcrMessageExecuteCqWithIr() {}

 private:
};

class TcrMessageExecuteRegionFunction : public TcrMessage {
 public:
  TcrMessageExecuteRegionFunction(
      std::unique_ptr<DataOutput> dataOutput, const std::string& funcName,
      const Region* region, const std::shared_ptr<Cacheable>& args,
      std::shared_ptr<CacheableVector> routingObj, uint8_t getResult,
      std::shared_ptr<CacheableHashSet> failedNodes,
      std::chrono::milliseconds timeout,
      ThinClientBaseDM* connectionDM = nullptr, int8_t reExecute = 0);

  virtual ~TcrMessageExecuteRegionFunction() {}

 private:
};

class TcrMessageExecuteRegionFunctionSingleHop : public TcrMessage {
 public:
  TcrMessageExecuteRegionFunctionSingleHop(
      std::unique_ptr<DataOutput> dataOutput, const std::string& funcName,
      const Region* region, const std::shared_ptr<Cacheable>& args,
      std::shared_ptr<CacheableHashSet> routingObj, uint8_t getResult,
      std::shared_ptr<CacheableHashSet> failedNodes, bool allBuckets,
      std::chrono::milliseconds timeout, ThinClientBaseDM* connectionDM);

  virtual ~TcrMessageExecuteRegionFunctionSingleHop() {}

 private:
};

class TcrMessageGetClientPartitionAttributes : public TcrMessage {
 public:
  TcrMessageGetClientPartitionAttributes(std::unique_ptr<DataOutput> dataOutput,
                                         const char* regionName);

  virtual ~TcrMessageGetClientPartitionAttributes() {}

 private:
};

class TcrMessageGetClientPrMetadata : public TcrMessage {
 public:
  TcrMessageGetClientPrMetadata(std::unique_ptr<DataOutput> dataOutput,
                                const char* regionName);

  virtual ~TcrMessageGetClientPrMetadata() {}

 private:
};

class TcrMessageSize : public TcrMessage {
 public:
  TcrMessageSize(std::unique_ptr<DataOutput> dataOutput,
                 const char* regionName);

  virtual ~TcrMessageSize() {}

 private:
};

class TcrMessageUserCredential : public TcrMessage {
 public:
  TcrMessageUserCredential(std::unique_ptr<DataOutput> dataOutput,
                           std::shared_ptr<Properties> creds,
                           ThinClientBaseDM* connectionDM = nullptr);

  virtual ~TcrMessageUserCredential() {}

 private:
};

class TcrMessageRemoveUserAuth : public TcrMessage {
 public:
  TcrMessageRemoveUserAuth(std::unique_ptr<DataOutput> dataOutput,
                           bool keepAlive, ThinClientBaseDM* connectionDM);

  virtual ~TcrMessageRemoveUserAuth() {}

 private:
};

class TcrMessageGetPdxIdForType : public TcrMessage {
 public:
  TcrMessageGetPdxIdForType(std::unique_ptr<DataOutput> dataOutput,
                            const std::shared_ptr<Cacheable>& pdxType,
                            ThinClientBaseDM* connectionDM,
                            int32_t pdxTypeId = 0);

  virtual ~TcrMessageGetPdxIdForType() {}

 private:
};

class TcrMessageAddPdxType : public TcrMessage {
 public:
  TcrMessageAddPdxType(std::unique_ptr<DataOutput> dataOutput,
                       const std::shared_ptr<Cacheable>& pdxType,
                       ThinClientBaseDM* connectionDM, int32_t pdxTypeId = 0);

  virtual ~TcrMessageAddPdxType() {}

 private:
};

class TcrMessageGetPdxIdForEnum : public TcrMessage {
 public:
  TcrMessageGetPdxIdForEnum(std::unique_ptr<DataOutput> dataOutput,
                            const std::shared_ptr<Cacheable>& pdxType,
                            ThinClientBaseDM* connectionDM,
                            int32_t pdxTypeId = 0);

  virtual ~TcrMessageGetPdxIdForEnum() {}

 private:
};

class TcrMessageAddPdxEnum : public TcrMessage {
 public:
  TcrMessageAddPdxEnum(std::unique_ptr<DataOutput> dataOutput,
                       const std::shared_ptr<Cacheable>& pdxType,
                       ThinClientBaseDM* connectionDM, int32_t pdxTypeId = 0);

  virtual ~TcrMessageAddPdxEnum() {}

 private:
};

class TcrMessageGetPdxTypeById : public TcrMessage {
 public:
  TcrMessageGetPdxTypeById(std::unique_ptr<DataOutput> dataOutput,
                           int32_t typeId, ThinClientBaseDM* connectionDM);

  virtual ~TcrMessageGetPdxTypeById() {}

 private:
};

class TcrMessageGetPdxEnumById : public TcrMessage {
 public:
  TcrMessageGetPdxEnumById(std::unique_ptr<DataOutput> dataOutput,
                           int32_t typeId, ThinClientBaseDM* connectionDM);

  virtual ~TcrMessageGetPdxEnumById() {}

 private:
};

class TcrMessageGetFunctionAttributes : public TcrMessage {
 public:
  TcrMessageGetFunctionAttributes(std::unique_ptr<DataOutput> dataOutput,
                                  const std::string& funcName,
                                  ThinClientBaseDM* connectionDM = nullptr);

  virtual ~TcrMessageGetFunctionAttributes() {}

 private:
};

class TcrMessageKeySet : public TcrMessage {
 public:
  TcrMessageKeySet(std::unique_ptr<DataOutput> dataOutput,
                   const std::string& funcName,
                   ThinClientBaseDM* connectionDM = nullptr);

  virtual ~TcrMessageKeySet() {}

 private:
};

class TcrMessageRequestEventValue : public TcrMessage {
 public:
  TcrMessageRequestEventValue(std::unique_ptr<DataOutput> dataOutput,
                              std::shared_ptr<EventId> eventId);

  virtual ~TcrMessageRequestEventValue() {}

 private:
};

class TcrMessagePeriodicAck : public TcrMessage {
 public:
  TcrMessagePeriodicAck(std::unique_ptr<DataOutput> dataOutput,
                        const EventIdMapEntryList& entries);

  virtual ~TcrMessagePeriodicAck() {}

 private:
};

class TcrMessageUpdateClientNotification : public TcrMessage {
 public:
  TcrMessageUpdateClientNotification(std::unique_ptr<DataOutput> dataOutput,
                                     int32_t port);

  virtual ~TcrMessageUpdateClientNotification() {}

 private:
};

class TcrMessageGetAll : public TcrMessage {
 public:
  TcrMessageGetAll(
      std::unique_ptr<DataOutput> dataOutput, const Region* region,
      const std::vector<std::shared_ptr<CacheableKey>>* keys,
      ThinClientBaseDM* connectionDM = nullptr,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);

  virtual ~TcrMessageGetAll() {}

 private:
};

class TcrMessageExecuteFunction : public TcrMessage {
 public:
  TcrMessageExecuteFunction(std::unique_ptr<DataOutput> dataOutput,
                            const std::string& funcName,
                            const std::shared_ptr<Cacheable>& args,
                            uint8_t getResult, ThinClientBaseDM* connectionDM,
                            std::chrono::milliseconds timeout);

  virtual ~TcrMessageExecuteFunction() {}

 private:
};

class TcrMessagePing : public TcrMessage {
 public:
  TcrMessagePing(std::unique_ptr<DataOutput> dataOutput, bool decodeAll);

  virtual ~TcrMessagePing() {}

 private:
};

class TcrMessageCloseConnection : public TcrMessage {
 public:
  TcrMessageCloseConnection(std::unique_ptr<DataOutput> dataOutput,
                            bool decodeAll);

  virtual ~TcrMessageCloseConnection() {}

 private:
};

class TcrMessageClientMarker : public TcrMessage {
 public:
  TcrMessageClientMarker(std::unique_ptr<DataOutput> dataOutput,
                         bool decodeAll);

  virtual ~TcrMessageClientMarker() {}

 private:
};

class TcrMessageReply : public TcrMessage {
 public:
  TcrMessageReply(bool decodeAll, ThinClientBaseDM* connectionDM);

  virtual ~TcrMessageReply() {}
};

/**
 * Helper class to invoke some internal methods of TcrMessage. Add any
 * methods that response processor methods require to access here.
 */
class TcrMessageHelper {
 public:
  /**
   * result types returned by readChunkPartHeader
   */
  enum ChunkObjectType { OBJECT = 0, EXCEPTION = 1, NULL_OBJECT = 2 };

  /**
   * Tries to read an exception part and returns true if the exception
   * was successfully read.
   */
  inline static bool readExceptionPart(TcrMessage& msg, DataInput& input,
                                       uint8_t isLastChunk) {
    return msg.readExceptionPart(input, isLastChunk);
  }

  inline static void skipParts(TcrMessage& msg, DataInput& input,
                               int32_t numParts = 1) {
    msg.skipParts(input, numParts);
  }

  /**
   * Reads header of a chunk part. Returns true if header was successfully
   * read and false if it is a chunk exception part.
   * Throws a MessageException with relevant message if an unknown
   * message type is encountered in the header.
   */
  inline static ChunkObjectType readChunkPartHeader(
      TcrMessage& msg, DataInput& input, uint8_t expectedFirstType,
      int32_t expectedPartType, const char* methodName, uint32_t& partLen,
      uint8_t isLastChunk) {
    partLen = input.readInt32();
    const auto isObj = input.readBoolean();

    if (partLen == 0) {
      // special null object is case for scalar query result
      return NULL_OBJECT;
    } else if (!isObj) {
      // otherwise we're currently always expecting an object
      char exMsg[256];
      ACE_OS::snprintf(exMsg, 255,
                       "TcrMessageHelper::readChunkPartHeader: "
                       "%s: part is not object",
                       methodName);
      LOGDEBUG("%s ", exMsg);
      // throw MessageException(exMsg);
      return EXCEPTION;
    }

    uint8_t partType = input.read();
    int32_t compId = partType;

    //  ugly hack to check for exception chunk
    if (partType == GeodeTypeIdsImpl::JavaSerializable) {
      input.reset();
      if (TcrMessageHelper::readExceptionPart(msg, input, isLastChunk)) {
        msg.setMessageType(TcrMessage::EXCEPTION);
        return EXCEPTION;
      } else {
        char exMsg[256];
        ACE_OS::snprintf(
            exMsg, 255,
            "TcrMessageHelper::readChunkPartHeader: %s: cannot handle "
            "java serializable object from server",
            methodName);
        throw MessageException(exMsg);
      }
    } else if (partType == GeodeTypeIds::NullObj) {
      // special null object is case for scalar query result
      return NULL_OBJECT;
    }

    if (expectedFirstType > 0) {
      if (partType != expectedFirstType) {
        char exMsg[256];
        ACE_OS::snprintf(exMsg, 255,
                         "TcrMessageHelper::readChunkPartHeader: "
                         "%s: got unhandled object class = %d",
                         methodName, partType);
        throw MessageException(exMsg);
      }
      // This is for GETALL
      if (expectedFirstType == GeodeTypeIdsImpl::FixedIDShort) {
        compId = input.readInt16();
        ;
      }  // This is for QUERY or REGISTER INTEREST.
      else if (expectedFirstType == GeodeTypeIdsImpl::FixedIDByte ||
               expectedFirstType == 0) {
        partType = input.read();
        compId = partType;
      }
    }
    if (compId != expectedPartType) {
      char exMsg[256];
      ACE_OS::snprintf(exMsg, 255,
                       "TcrMessageHelper::readChunkPartHeader: "
                       "%s: got unhandled object type = %d",
                       methodName, compId);
      throw MessageException(exMsg);
    }
    return OBJECT;
  }
  inline static int8_t readChunkPartHeader(TcrMessage& msg, DataInput& input,
                                           const char* methodName,
                                           uint32_t& partLen,
                                           uint8_t isLastChunk) {
    partLen = input.readInt32();
    const auto isObj = input.readBoolean();

    if (partLen == 0) {
      // special null object is case for scalar query result
      return static_cast<int8_t>(NULL_OBJECT);
    } else if (!isObj) {
      // otherwise we're currently always expecting an object
      char exMsg[256];
      ACE_OS::snprintf(exMsg, 255,
                       "TcrMessageHelper::readChunkPartHeader: "
                       "%s: part is not object",
                       methodName);
      throw MessageException(exMsg);
    }

    const auto partType = input.read();
    //  ugly hack to check for exception chunk
    if (partType == GeodeTypeIdsImpl::JavaSerializable) {
      input.reset();
      if (TcrMessageHelper::readExceptionPart(msg, input, isLastChunk)) {
        msg.setMessageType(TcrMessage::EXCEPTION);
        return static_cast<int8_t>(EXCEPTION);
      } else {
        char exMsg[256];
        ACE_OS::snprintf(
            exMsg, 255,
            "TcrMessageHelper::readChunkPartHeader: %s: cannot handle "
            "java serializable object from server",
            methodName);
        throw MessageException(exMsg);
      }
    } else if (partType == GeodeTypeIds::NullObj) {
      // special null object is case for scalar query result
      return static_cast<int8_t>(NULL_OBJECT);
    }
    return partType;
  }
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TCRMESSAGE_H_
