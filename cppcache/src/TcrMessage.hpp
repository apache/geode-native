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
#include <cinttypes>
#include <map>
#include <string>
#include <vector>

#include <geode/CacheableBuiltins.hpp>
#include <geode/CacheableKey.hpp>
#include <geode/CacheableString.hpp>
#include <geode/CqState.hpp>
#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/Region.hpp>
#include <geode/Serializable.hpp>
#include <geode/internal/geode_globals.hpp>

#include "EventIdMap.hpp"
#include "InterestResultPolicy.hpp"
#include "util/concurrent/binary_semaphore.hpp"

namespace apache {
namespace geode {
namespace client {

class ThinClientBaseDM;
class TcrConnection;
class TcrMessagePing;
class BucketServerLocation;
class EventId;
class FixedPartitionAttributesImpl;
class SerializationRegistry;
class VersionTag;
class VersionedCacheableObjectPartList;
class MemberListForVersionStamp;
class TcrChunkedResult;
class DSMemberForVersionStamp;

class TcrMessage {
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

  static bool isUserInitiativeOps(const TcrMessage& msg);

  static std::shared_ptr<VersionTag> readVersionTagPart(
      DataInput& input, uint16_t endpointMemId,
      MemberListForVersionStamp& memberListForVersionStamp);

  /* constructors */
  void setData(std::vector<char>, uint16_t memId,
               const SerializationRegistry& serializationRegistry,
               MemberListForVersionStamp& memberListForVersionStamp);

  void startProcessChunk(binary_semaphore& finalizeSema);
  // nullptr chunk means that this is the last chunk
  void processChunk(const std::vector<uint8_t>& chunk, int32_t chunkLen,
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

  const std::vector<std::shared_ptr<CacheableKey>>* getKeys() const;

  const std::string& getRegex() const;

  InterestResultPolicy getInterestResultPolicy() const;

  Pool* getPool() const;

  /**
   * Whether the request is meant to be
   * sent to PR primary node for single hop.
   */
  bool forPrimary() const;

  void initCqMap();

  bool forSingleHop() const;

  bool forTransaction() const;

  /* destroy the connection */
  virtual ~TcrMessage();

  const std::string& getRegionName() const;
  Region* getRegion() const;
  int32_t getMessageType() const;
  void setMessageType(int32_t msgType);

  /**
   * Set the msgType of the request that was made if this is a reply.
   */
  void setMessageTypeRequest(int32_t msgType);
  std::shared_ptr<CacheableKey> getKey() const;
  const std::shared_ptr<CacheableKey>& getKeyRef() const;
  std::shared_ptr<Cacheable> getValue() const;
  const std::shared_ptr<Cacheable>& getValueRef() const;
  std::shared_ptr<Cacheable> getCallbackArgument() const;
  const std::shared_ptr<Cacheable>& getCallbackArgumentRef() const;

  const std::map<std::string, int>* getCqs() const;
  bool getBoolValue() const;
  const std::string& getException();

  const char* getMsgData() const;
  size_t getMsgLength() const;
  std::shared_ptr<EventId> getEventId() const;

  int32_t getTransId() const;
  void setTransId(int32_t txId);

  std::chrono::milliseconds getTimeout() const;
  void setTimeout(std::chrono::milliseconds timeout);

  bool isDurable() const;
  bool receiveValues() const;
  bool hasCqPart() const;
  uint32_t getMessageTypeForCq() const;
  bool shouldIgnore() const;
  int8_t getMetaDataVersion() const;
  uint32_t getEntryNotFound() const;
  int8_t getserverGroupVersion() const;
  std::shared_ptr<std::vector<int8_t>> getFunctionAttributes();

  // set the DM for chunked response messages
  void setDM(ThinClientBaseDM* dm);
  ThinClientBaseDM* getDM();
  // set the chunked response handler
  void setChunkedResultHandler(TcrChunkedResult* chunkedResult);
  TcrChunkedResult* getChunkedResultHandler();

  DataInput* getDelta() const;
  //  getDeltaBytes( ) is called *only* by CqService, returns a CacheableBytes
  //  that
  // takes ownership of delta bytes.
  std::shared_ptr<CacheableBytes> getDeltaBytes();

  bool hasDelta() const;

  void addSecurityPart(int64_t connectionId, int64_t unique_id,
                       TcrConnection* conn);

  void addSecurityPart(int64_t connectionId, TcrConnection* conn);

  int64_t getConnectionId();

  int64_t getUniqueId();

  void createUserCredentialMessage(TcrConnection* conn);

  void readSecureObjectPart(DataInput& input, bool defaultString = false,
                            bool isChunk = false,
                            uint8_t isLastChunkWithSecurity = 0);

  void readUniqueIDObjectPart(DataInput& input);

  void setMetaRegion(bool isMetaRegion);
  bool isMetaRegion() const;

  int32_t getNumBuckets() const;

  const std::string& getColocatedWith() const;

  std::vector<std::vector<std::shared_ptr<BucketServerLocation>>>*
  getMetadata();

  std::vector<std::shared_ptr<FixedPartitionAttributesImpl>>* getFpaSet();

  std::shared_ptr<CacheableHashSet> getFailedNode();

  bool isCallBackArguement() const;

  void setCallBackArguement(bool aCallBackArguement);

  void setVersionTag(std::shared_ptr<VersionTag> versionTag);
  std::shared_ptr<VersionTag> getVersionTag() const;
  uint8_t hasResult() const;
  std::shared_ptr<CacheableHashMap> getTombstoneVersions() const;
  std::shared_ptr<CacheableHashSet> getTombstoneKeys() const;

  bool isFEAnotherHop();

 protected:
  TcrMessage();

  void handleSpecialFECase();
  void writeBytesOnly(const std::shared_ptr<Serializable>& se);
  std::shared_ptr<Serializable> readCacheableBytes(DataInput& input,
                                                   int lenObj);
  std::shared_ptr<Serializable> readCacheableString(DataInput& input,
                                                    int lenObj);

  TcrMessage(const TcrMessage&) = delete;
  TcrMessage& operator=(const TcrMessage&) = delete;

  // some private methods to handle things internally.
  void handleByteArrayResponse(
      std::vector<char> buffer, uint16_t endpointMemId,
      const SerializationRegistry& serializationRegistry,
      MemberListForVersionStamp& memberListForVersionStamp);
  void readObjectPart(DataInput& input, bool defaultString = false);
  void readFailedNodePart(DataInput& input);
  void readCallbackObjectPart(DataInput& input, bool defaultString = false);
  void readKeyPart(DataInput& input);
  void readBooleanPartAsObject(DataInput& input, bool* boolVal);
  void readIntPart(DataInput& input, uint32_t* intValue);
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
  void chunkSecurityHeader(int skipParts, const std::vector<uint8_t> bytes,
                           int32_t len, uint8_t isLastChunkAndSecurityHeader);

  void readEventIdPart(DataInput& input, bool skip = false,
                       int32_t parts = 1);  // skip num parts then read eventid

  void skipParts(DataInput& input, int32_t numParts = 1);
  const std::string readStringPart(DataInput& input);
  void readCqsPart(DataInput& input);
  void readHashMapForGCVersions(apache::geode::client::DataInput& input,
                                std::shared_ptr<CacheableHashMap>& value);
  void readHashSetForGCVersions(apache::geode::client::DataInput& input,
                                std::shared_ptr<CacheableHashSet>& value);
  std::shared_ptr<DSMemberForVersionStamp> readDSMember(
      apache::geode::client::DataInput& input);

  std::unique_ptr<DataOutput> m_request;
  /** the associated region that is handling processing of chunked responses */
  ThinClientBaseDM* m_tcdm;
  TcrChunkedResult* m_chunkedResult;
  const std::vector<std::shared_ptr<CacheableKey>>* m_keyList;
  const Region* m_region;
  std::chrono::milliseconds m_timeout;
  std::vector<std::vector<std::shared_ptr<BucketServerLocation>>>* m_metadata;
  std::map<std::string, int>* m_cqs;
  std::chrono::milliseconds m_messageResponseTimeout;
  std::unique_ptr<DataInput> m_delta;
  int8_t* m_deltaBytes;
  std::vector<std::shared_ptr<FixedPartitionAttributesImpl>>* m_fpaSet;
  std::shared_ptr<std::vector<int8_t>> m_functionAttributes;
  std::shared_ptr<CacheableBytes> m_connectionIDBytes;
  std::shared_ptr<Properties> m_creds;
  std::shared_ptr<CacheableKey> m_key;
  std::shared_ptr<Cacheable> m_value;
  std::shared_ptr<CacheableHashSet> m_failedNode;
  std::shared_ptr<Cacheable> m_callbackArgument;
  std::shared_ptr<VersionTag> m_versionTag;
  std::shared_ptr<EventId> m_eventid;
  std::shared_ptr<CacheableHashMap> m_tombstoneVersions;
  std::shared_ptr<CacheableHashSet> m_tombstoneKeys;
  std::string m_exceptionMessage;
  std::string m_regionName;
  std::string m_regex;
  std::string m_colocatedWith;
  int32_t m_securityHeaderLength;
  int32_t m_msgType;
  /** the msgType of the request if this TcrMessage is  a reply msg */
  int32_t m_msgTypeRequest;
  int32_t m_txId;
  int32_t m_bucketCount;
  uint32_t m_numCqPart;
  uint32_t m_msgTypeForCq;
  int32_t m_deltaBytesLen;
  uint32_t m_entryNotFound;
  bool m_feAnotherHop;
  bool m_isSecurityOn;
  uint8_t m_isLastChunkAndisSecurityHeader;
  bool m_isSecurityHeaderAdded;
  bool m_isMetaRegion;
  /** used only when decoding reply message, if false, decode header only */
  bool m_decodeAll;
  InterestResultPolicy m_interestPolicy;
  bool m_isDurable;
  bool m_receiveValues;
  bool m_hasCqsPart;
  bool m_isInterestListPassed;
  bool m_shouldIgnore;
  int8_t m_metaDataVersion;
  int8_t m_serverGroupVersion;
  bool m_boolValue;
  bool m_isCallBackArguement;
  uint8_t m_hasResult;

  friend class TcrMessageHelper;
};

class TcrMessageDestroyRegion : public TcrMessage {
 public:
  TcrMessageDestroyRegion(
      DataOutput* dataOutput, const Region* region,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::chrono::milliseconds messageResponsetimeout,
      ThinClientBaseDM* connectionDM);

  ~TcrMessageDestroyRegion() override = default;
};

class TcrMessageClearRegion : public TcrMessage {
 public:
  TcrMessageClearRegion(DataOutput* dataOutput, const Region* region,
                        const std::shared_ptr<Serializable>& aCallbackArgument,
                        std::chrono::milliseconds messageResponsetimeout,
                        ThinClientBaseDM* connectionDM);

  ~TcrMessageClearRegion() override = default;
};

class TcrMessageQuery : public TcrMessage {
 public:
  TcrMessageQuery(DataOutput* dataOutput, const std::string& regionName,
                  std::chrono::milliseconds messageResponsetimeout,
                  ThinClientBaseDM* connectionDM);

  ~TcrMessageQuery() override = default;
};

class TcrMessageStopCQ : public TcrMessage {
 public:
  TcrMessageStopCQ(DataOutput* dataOutput, const std::string& regionName,
                   std::chrono::milliseconds messageResponsetimeout,
                   ThinClientBaseDM* connectionDM);

  ~TcrMessageStopCQ() override = default;
};

class TcrMessageCloseCQ : public TcrMessage {
 public:
  TcrMessageCloseCQ(DataOutput* dataOutput, const std::string& regionName,
                    std::chrono::milliseconds messageResponsetimeout,
                    ThinClientBaseDM* connectionDM);

  ~TcrMessageCloseCQ() override = default;
};

class TcrMessageQueryWithParameters : public TcrMessage {
 public:
  TcrMessageQueryWithParameters(
      DataOutput* dataOutput, const std::string& regionName,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<CacheableVector> paramList,
      std::chrono::milliseconds messageResponsetimeout,
      ThinClientBaseDM* connectionDM);

  ~TcrMessageQueryWithParameters() override = default;
};

class TcrMessageContainsKey : public TcrMessage {
 public:
  TcrMessageContainsKey(DataOutput* dataOutput, const Region* region,
                        const std::shared_ptr<CacheableKey>& key,
                        const std::shared_ptr<Serializable>& aCallbackArgument,
                        bool isContainsKey, ThinClientBaseDM* connectionDM);

  ~TcrMessageContainsKey() override = default;
};

class TcrMessageGetDurableCqs : public TcrMessage {
 public:
  TcrMessageGetDurableCqs(DataOutput* dataOutput,
                          ThinClientBaseDM* connectionDM);

  ~TcrMessageGetDurableCqs() override = default;
};

class TcrMessageRequest : public TcrMessage {
 public:
  TcrMessageRequest(DataOutput* dataOutput, const Region* region,
                    const std::shared_ptr<CacheableKey>& key,
                    const std::shared_ptr<Serializable>& aCallbackArgument,
                    ThinClientBaseDM* connectionDM = nullptr);

  ~TcrMessageRequest() override = default;
};

class TcrMessageInvalidate : public TcrMessage {
 public:
  TcrMessageInvalidate(DataOutput* dataOutput, const Region* region,
                       const std::shared_ptr<CacheableKey>& key,
                       const std::shared_ptr<Serializable>& aCallbackArgument,
                       ThinClientBaseDM* connectionDM = nullptr);

  ~TcrMessageInvalidate() override = default;
};

class TcrMessageDestroy : public TcrMessage {
 public:
  TcrMessageDestroy(DataOutput* dataOutput, const Region* region,
                    const std::shared_ptr<CacheableKey>& key,
                    const std::shared_ptr<Cacheable>& value,
                    bool isUserNullValue,
                    const std::shared_ptr<Serializable>& aCallbackArgument,
                    ThinClientBaseDM* connectionDM = nullptr);

  ~TcrMessageDestroy() override = default;
};

class TcrMessageRegisterInterestList : public TcrMessage {
 public:
  TcrMessageRegisterInterestList(
      DataOutput* dataOutput, const Region* region,
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      bool isDurable = false, bool isCachingEnabled = false,
      bool receiveValues = true,
      InterestResultPolicy interestPolicy = InterestResultPolicy::NONE,
      ThinClientBaseDM* connectionDM = nullptr);

  ~TcrMessageRegisterInterestList() override = default;
};

class TcrMessageUnregisterInterestList : public TcrMessage {
 public:
  TcrMessageUnregisterInterestList(
      DataOutput* dataOutput, const Region* region,
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      bool isDurable = false, ThinClientBaseDM* connectionDM = nullptr);

  ~TcrMessageUnregisterInterestList() override = default;
};

class TcrMessagePut : public TcrMessage {
 public:
  TcrMessagePut(DataOutput* dataOutput, const Region* region,
                const std::shared_ptr<CacheableKey>& key,
                const std::shared_ptr<Cacheable>& value,
                const std::shared_ptr<Serializable>& aCallbackArgument,
                bool isDelta = false, ThinClientBaseDM* connectionDM = nullptr,
                bool isMetaRegion = false, bool fullValueAfterDeltaFail = false,
                const char* regionName = nullptr);

  ~TcrMessagePut() override = default;
};

class TcrMessageCreateRegion : public TcrMessage {
 public:
  TcrMessageCreateRegion(DataOutput* dataOutput, const std::string& str1,
                         const std::string& str2, bool isDurable = false,
                         bool receiveValues = true,
                         ThinClientBaseDM* connectionDM = nullptr);

  ~TcrMessageCreateRegion() override = default;
};

class TcrMessageRegisterInterestRegex : public TcrMessage {
 public:
  TcrMessageRegisterInterestRegex(
      DataOutput* dataOutput, const std::string& regionName,
      const std::string& regex,
      InterestResultPolicy interestPolicy = InterestResultPolicy::NONE,
      bool isDurable = false, bool isCachingEnabled = false,
      bool receiveValues = true, ThinClientBaseDM* connectionDM = nullptr);

  ~TcrMessageRegisterInterestRegex() override = default;
};

class TcrMessageUnregisterInterestRegex : public TcrMessage {
 public:
  TcrMessageUnregisterInterestRegex(DataOutput* dataOutput,
                                    const std::string& regionName,
                                    const std::string& regex,
                                    bool isDurable = false,
                                    ThinClientBaseDM* connectionDM = nullptr);

  ~TcrMessageUnregisterInterestRegex() override = default;
};

class TcrMessageTxSynchronization : public TcrMessage {
 public:
  TcrMessageTxSynchronization(DataOutput* dataOutput, int ordinal, int txid,
                              int status);

  ~TcrMessageTxSynchronization() override = default;
};

class TcrMessageClientReady : public TcrMessage {
 public:
  explicit TcrMessageClientReady(DataOutput* dataOutput);

  ~TcrMessageClientReady() override = default;
};

class TcrMessageCommit : public TcrMessage {
 public:
  explicit TcrMessageCommit(DataOutput* dataOutput);

  ~TcrMessageCommit() override = default;
};

class TcrMessageRollback : public TcrMessage {
 public:
  explicit TcrMessageRollback(DataOutput* dataOutput);

  ~TcrMessageRollback() override = default;
};

class TcrMessageTxFailover : public TcrMessage {
 public:
  explicit TcrMessageTxFailover(DataOutput* dataOutput);

  ~TcrMessageTxFailover() override = default;
};

class TcrMessageMakePrimary : public TcrMessage {
 public:
  TcrMessageMakePrimary(DataOutput* dataOutput, bool processedMarker);

  ~TcrMessageMakePrimary() override = default;
};

class TcrMessagePutAll : public TcrMessage {
 public:
  TcrMessagePutAll(DataOutput* dataOutput, const Region* region,
                   const HashMapOfCacheable& map,
                   std::chrono::milliseconds messageResponsetimeout,
                   ThinClientBaseDM* connectionDM,
                   const std::shared_ptr<Serializable>& aCallbackArgument);

  ~TcrMessagePutAll() override = default;
};

class TcrMessageRemoveAll : public TcrMessage {
 public:
  TcrMessageRemoveAll(DataOutput* dataOutput, const Region* region,
                      const std::vector<std::shared_ptr<CacheableKey>>& keys,
                      const std::shared_ptr<Serializable>& aCallbackArgument,
                      ThinClientBaseDM* connectionDM = nullptr);

  ~TcrMessageRemoveAll() override = default;
};

class TcrMessageExecuteCq : public TcrMessage {
 public:
  TcrMessageExecuteCq(DataOutput* dataOutput, const std::string& str1,
                      const std::string& str2, CqState state, bool isDurable,
                      ThinClientBaseDM* connectionDM);

  ~TcrMessageExecuteCq() override = default;
};

class TcrMessageExecuteCqWithIr : public TcrMessage {
 public:
  TcrMessageExecuteCqWithIr(DataOutput* dataOutput, const std::string& str1,
                            const std::string& str2, CqState state,
                            bool isDurable, ThinClientBaseDM* connectionDM);

  ~TcrMessageExecuteCqWithIr() override = default;
};

class TcrMessageExecuteRegionFunction : public TcrMessage {
 public:
  TcrMessageExecuteRegionFunction(
      DataOutput* dataOutput, const std::string& funcName, const Region* region,
      const std::shared_ptr<Cacheable>& args,
      std::shared_ptr<CacheableVector> routingObj, uint8_t getResult,
      std::shared_ptr<CacheableHashSet> failedNodes,
      std::chrono::milliseconds timeout,
      ThinClientBaseDM* connectionDM = nullptr, int8_t reExecute = 0);

  ~TcrMessageExecuteRegionFunction() override = default;
};

class TcrMessageExecuteRegionFunctionSingleHop : public TcrMessage {
 public:
  TcrMessageExecuteRegionFunctionSingleHop(
      DataOutput* dataOutput, const std::string& funcName, const Region* region,
      const std::shared_ptr<Cacheable>& args,
      std::shared_ptr<CacheableHashSet> routingObj, uint8_t getResult,
      std::shared_ptr<CacheableHashSet> failedNodes, bool allBuckets,
      std::chrono::milliseconds timeout, ThinClientBaseDM* connectionDM);

  ~TcrMessageExecuteRegionFunctionSingleHop() override = default;
};

class TcrMessageGetClientPartitionAttributes : public TcrMessage {
 public:
  TcrMessageGetClientPartitionAttributes(DataOutput* dataOutput,
                                         const char* regionName);

  ~TcrMessageGetClientPartitionAttributes() override = default;
};

class TcrMessageGetClientPrMetadata : public TcrMessage {
 public:
  TcrMessageGetClientPrMetadata(DataOutput* dataOutput, const char* regionName);

  ~TcrMessageGetClientPrMetadata() override = default;
};

class TcrMessageSize : public TcrMessage {
 public:
  TcrMessageSize(DataOutput* dataOutput, const char* regionName);

  ~TcrMessageSize() override = default;
};

class TcrMessageUserCredential : public TcrMessage {
 public:
  TcrMessageUserCredential(DataOutput* dataOutput,
                           std::shared_ptr<Properties> creds,
                           ThinClientBaseDM* connectionDM = nullptr);

  ~TcrMessageUserCredential() override = default;
};

class TcrMessageRemoveUserAuth : public TcrMessage {
 public:
  TcrMessageRemoveUserAuth(DataOutput* dataOutput, bool keepAlive,
                           ThinClientBaseDM* connectionDM);

  ~TcrMessageRemoveUserAuth() override = default;
};

class TcrMessageGetPdxIdForType : public TcrMessage {
 public:
  TcrMessageGetPdxIdForType(DataOutput* dataOutput,
                            const std::shared_ptr<Cacheable>& pdxType,
                            ThinClientBaseDM* connectionDM);

  ~TcrMessageGetPdxIdForType() override = default;
};

class TcrMessageAddPdxType : public TcrMessage {
 public:
  TcrMessageAddPdxType(DataOutput* dataOutput,
                       const std::shared_ptr<Cacheable>& pdxType,
                       ThinClientBaseDM* connectionDM, int32_t pdxTypeId = 0);

  ~TcrMessageAddPdxType() override = default;
};

class TcrMessageGetPdxIdForEnum : public TcrMessage {
 public:
  TcrMessageGetPdxIdForEnum(DataOutput* dataOutput,
                            const std::shared_ptr<Cacheable>& pdxType,
                            ThinClientBaseDM* connectionDM);

  ~TcrMessageGetPdxIdForEnum() override = default;
};

class TcrMessageAddPdxEnum : public TcrMessage {
 public:
  TcrMessageAddPdxEnum(DataOutput* dataOutput,
                       const std::shared_ptr<Cacheable>& pdxType,
                       ThinClientBaseDM* connectionDM, int32_t pdxTypeId = 0);

  ~TcrMessageAddPdxEnum() override = default;
};

class TcrMessageGetPdxTypeById : public TcrMessage {
 public:
  TcrMessageGetPdxTypeById(DataOutput* dataOutput, int32_t typeId,
                           ThinClientBaseDM* connectionDM);

  ~TcrMessageGetPdxTypeById() override = default;
};

class TcrMessageGetPdxEnumById : public TcrMessage {
 public:
  TcrMessageGetPdxEnumById(DataOutput* dataOutput, int32_t typeId,
                           ThinClientBaseDM* connectionDM);

  ~TcrMessageGetPdxEnumById() override = default;
};

class TcrMessageGetFunctionAttributes : public TcrMessage {
 public:
  TcrMessageGetFunctionAttributes(DataOutput* dataOutput,
                                  const std::string& funcName,
                                  ThinClientBaseDM* connectionDM = nullptr);

  ~TcrMessageGetFunctionAttributes() override = default;
};

class TcrMessageKeySet : public TcrMessage {
 public:
  TcrMessageKeySet(DataOutput* dataOutput, const std::string& funcName,
                   ThinClientBaseDM* connectionDM = nullptr);

  ~TcrMessageKeySet() override = default;
};

class TcrMessageRequestEventValue : public TcrMessage {
 public:
  TcrMessageRequestEventValue(DataOutput* dataOutput,
                              std::shared_ptr<EventId> eventId);

  ~TcrMessageRequestEventValue() override = default;
};

class TcrMessagePeriodicAck : public TcrMessage {
 public:
  TcrMessagePeriodicAck(DataOutput* dataOutput,
                        const EventIdMapEntryList& entries);

  ~TcrMessagePeriodicAck() override = default;
};

class TcrMessageGetAll : public TcrMessage {
 public:
  TcrMessageGetAll(
      DataOutput* dataOutput, const Region* region,
      const std::vector<std::shared_ptr<CacheableKey>>* keys,
      ThinClientBaseDM* connectionDM = nullptr,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);

  ~TcrMessageGetAll() override = default;
};

class TcrMessageExecuteFunction : public TcrMessage {
 public:
  TcrMessageExecuteFunction(DataOutput* dataOutput, const std::string& funcName,
                            const std::shared_ptr<Cacheable>& args,
                            uint8_t getResult, ThinClientBaseDM* connectionDM,
                            std::chrono::milliseconds timeout);

  ~TcrMessageExecuteFunction() override = default;
};

class TcrMessagePing : public TcrMessage {
 public:
  explicit TcrMessagePing(std::unique_ptr<DataOutput> dataOutput);

  ~TcrMessagePing() override = default;
};

class TcrMessageCloseConnection : public TcrMessage {
 public:
  TcrMessageCloseConnection(std::unique_ptr<DataOutput> dataOutput,
                            bool keepAlive);

  ~TcrMessageCloseConnection() override = default;
};

class TcrMessageClientMarker : public TcrMessage {
 public:
  TcrMessageClientMarker(DataOutput* dataOutput, bool decodeAll);

  ~TcrMessageClientMarker() override = default;
};

class TcrMessageReply : public TcrMessage {
 public:
  TcrMessageReply(bool decodeAll, ThinClientBaseDM* connectionDM);

  ~TcrMessageReply() override = default;
};

class TcrMessageAllEndpointsDisconnectedMarker : public TcrMessage {
 public:
  TcrMessageAllEndpointsDisconnectedMarker() = default;
};

/**
 * Helper class to invoke some internal methods of TcrMessage. Add any
 * methods that response processor methods require to access here.
 */
class TcrMessageHelper {
 public:
  TcrMessageHelper() = delete;

  /**
   * result types returned by readChunkPartHeader
   */
  enum class ChunkObjectType { OBJECT, EXCEPTION, NULL_OBJECT };

  /**
   * Tries to read an exception part and returns true if the exception
   * was successfully read.
   */
  static bool readExceptionPart(TcrMessage& msg, DataInput& input,
                                uint8_t isLastChunk);

  static void skipParts(TcrMessage& msg, DataInput& input,
                        int32_t numParts = 1);

  /**
   * Reads header of a chunk part. Returns true if header was successfully
   * read and false if it is a chunk exception part.
   * Throws a MessageException with relevant message if an unknown
   * message type is encountered in the header.
   */
  static ChunkObjectType readChunkPartHeader(TcrMessage& msg, DataInput& input,
                                             DSCode expectedFirstType,
                                             int32_t expectedPartType,
                                             const char* methodName,
                                             uint32_t& partLen,
                                             uint8_t isLastChunk);

  static ChunkObjectType readChunkPartHeader(TcrMessage& msg, DataInput& input,
                                             const char* methodName,
                                             uint32_t& partLen,
                                             uint8_t isLastChunk);
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TCRMESSAGE_H_
