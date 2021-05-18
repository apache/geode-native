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

#ifndef GEODE_THINCLIENTREGION_H_
#define GEODE_THINCLIENTREGION_H_

#include <mutex>
#include <unordered_map>

#include <geode/ResultCollector.hpp>
#include <geode/internal/functional.hpp>

#include "CacheableObjectPartList.hpp"
#include "ClientMetadataService.hpp"
#include "LocalRegion.hpp"
#include "Queue.hpp"
#include "RegionGlobalLocks.hpp"
#include "TcrChunkedContext.hpp"
#include "TcrMessage.hpp"

namespace apache {
namespace geode {
namespace client {

class ThinClientBaseDM;
class TcrEndpoint;

/**
 * @class ThinClientRegion ThinClientRegion.hpp
 *
 * This class manages all the functionalities related with thin client
 * region. It will inherit from DistributedRegion and overload some methods
 *
 */
class ThinClientRegion : public LocalRegion {
 public:
  /**
   * @brief constructor/initializer/destructor
   */
  ThinClientRegion(const std::string& name, CacheImpl* cache,
                   const std::shared_ptr<RegionInternal>& rPtr,
                   RegionAttributes attributes,
                   const std::shared_ptr<CacheStatistics>& stats,
                   bool shared = false);

  ThinClientRegion(const ThinClientRegion&) = delete;
  ThinClientRegion& operator=(const ThinClientRegion&) = delete;

  virtual void initTCR();
  ~ThinClientRegion() noexcept override;

  /** @brief Public Methods from Region
   */
  // Unhide function to prevent SunPro Warnings
  using RegionInternal::registerKeys;
  void registerKeys(const std::vector<std::shared_ptr<CacheableKey>>& keys,
                    bool isDurable = false, bool getInitialValues = false,
                    bool receiveValues = true) override;
  void unregisterKeys(
      const std::vector<std::shared_ptr<CacheableKey>>& keys) override;
  void registerAllKeys(bool isDurable = false, bool getInitialValues = false,
                       bool receiveValues = true) override;
  void unregisterAllKeys() override;
  void registerRegex(const std::string& regex, bool isDurable = false,
                     bool getInitialValues = false,
                     bool receiveValues = true) override;
  void unregisterRegex(const std::string& regex) override;
  std::vector<std::shared_ptr<CacheableKey>> serverKeys() override;
  void clear(const std::shared_ptr<Serializable>& aCallbackArgument =
                 nullptr) override;

  std::shared_ptr<SelectResults> query(
      const std::string& predicate,
      std::chrono::milliseconds timeout =
          DEFAULT_QUERY_RESPONSE_TIMEOUT) override;

  bool existsValue(const std::string& predicate,
                   std::chrono::milliseconds timeout =
                       DEFAULT_QUERY_RESPONSE_TIMEOUT) override;

  std::shared_ptr<Serializable> selectValue(
      const std::string& predicate,
      std::chrono::milliseconds timeout =
          DEFAULT_QUERY_RESPONSE_TIMEOUT) override;

  /** @brief Public Methods from RegionInternal
   *  These are all virtual methods
   */
  GfErrType putAllNoThrow_remote(
      const HashMapOfCacheable& map,
      std::shared_ptr<VersionedCacheableObjectPartList>& versionedObjPartList,
      std::chrono::milliseconds timeout = DEFAULT_RESPONSE_TIMEOUT,
      const std::shared_ptr<Serializable>& aCallbackArgument =
          nullptr) override;
  GfErrType removeAllNoThrow_remote(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      std::shared_ptr<VersionedCacheableObjectPartList>& versionedObjPartList,
      const std::shared_ptr<Serializable>& aCallbackArgument =
          nullptr) override;
  GfErrType registerKeys(TcrEndpoint* endpoint = nullptr,
                         const TcrMessage* request = nullptr,
                         TcrMessageReply* reply = nullptr);
  GfErrType unregisterKeys();
  void addKeys(const std::vector<std::shared_ptr<CacheableKey>>& keys,
               bool isDurable, bool receiveValues,
               InterestResultPolicy interestpolicy);
  void addRegex(const std::string& regex, bool isDurable, bool receiveValues,
                InterestResultPolicy interestpolicy);
  GfErrType findRegex(const std::string& regex);
  void clearRegex(const std::string& regex);

  bool containsKeyOnServer(
      const std::shared_ptr<CacheableKey>& keyPtr) const override;
  bool containsValueForKey_remote(
      const std::shared_ptr<CacheableKey>& keyPtr) const override;
  std::vector<std::shared_ptr<CacheableKey>> getInterestList() const override;
  std::vector<std::shared_ptr<CacheableString>> getInterestListRegex()
      const override;

  void receiveNotification(const TcrMessage& msg);

  static GfErrType handleServerException(const std::string& func,
                                         const std::string& exceptionMsg);

  void acquireGlobals(bool failover) override;
  void releaseGlobals(bool failover) override;

  void localInvalidateFailover();

  inline ThinClientBaseDM* getDistMgr() const { return m_tcrdm.get(); }

  std::shared_ptr<CacheableVector> reExecuteFunction(
      const std::string& func, const std::shared_ptr<Cacheable>& args,
      std::shared_ptr<CacheableVector> routingObj, uint8_t getResult,
      std::shared_ptr<ResultCollector> rc, int32_t retryAttempts,
      std::shared_ptr<CacheableHashSet>& failedNodes,
      std::chrono::milliseconds timeout = DEFAULT_QUERY_RESPONSE_TIMEOUT);

  bool executeFunctionSH(
      const std::string& func, const std::shared_ptr<Cacheable>& args,
      uint8_t getResult, std::shared_ptr<ResultCollector> rc,
      const std::shared_ptr<ClientMetadataService::ServerToKeysMap>&
          locationMap,
      std::shared_ptr<CacheableHashSet>& failedNodes,
      std::chrono::milliseconds timeout = DEFAULT_QUERY_RESPONSE_TIMEOUT,
      bool allBuckets = false);

  void executeFunction(
      const std::string&, const std::shared_ptr<Cacheable>& args,
      std::shared_ptr<CacheableVector> routingObj, uint8_t getResult,
      std::shared_ptr<ResultCollector> rc, int32_t retryAttempts,
      std::chrono::milliseconds timeout = DEFAULT_QUERY_RESPONSE_TIMEOUT);

  GfErrType getFuncAttributes(const std::string& func,
                              std::shared_ptr<std::vector<int8_t>>* attr);

  ACE_RW_Thread_Mutex& getMataDataMutex() { return m_RegionMutex; }

  bool const& getMetaDataRefreshed() { return m_isMetaDataRefreshed; }

  void setMetaDataRefreshed(bool aMetaDataRefreshed) {
    m_isMetaDataRefreshed = aMetaDataRefreshed;
  }

  uint32_t size_remote() override;

  void txDestroy(const std::shared_ptr<CacheableKey>& key,
                 const std::shared_ptr<Serializable>& callBack,
                 std::shared_ptr<VersionTag> versionTag) override;
  void txInvalidate(const std::shared_ptr<CacheableKey>& key,
                    const std::shared_ptr<Serializable>& callBack,
                    std::shared_ptr<VersionTag> versionTag) override;
  void txPut(const std::shared_ptr<CacheableKey>& key,
             const std::shared_ptr<Cacheable>& value,
             const std::shared_ptr<Serializable>& callBack,
             std::shared_ptr<VersionTag> versionTag) override;

  void clearKeysOfInterest();

 protected:
  GfErrType getNoThrow_remote(
      const std::shared_ptr<CacheableKey>& keyPtr,
      std::shared_ptr<Cacheable>& valPtr,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<VersionTag>& versionTag) override;
  GfErrType putNoThrow_remote(
      const std::shared_ptr<CacheableKey>& keyPtr,
      const std::shared_ptr<Cacheable>& cvalue,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<VersionTag>& versionTag, bool checkDelta = true) override;
  GfErrType createNoThrow_remote(
      const std::shared_ptr<CacheableKey>& keyPtr,
      const std::shared_ptr<Cacheable>& cvalue,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<VersionTag>& versionTag) override;
  GfErrType destroyNoThrow_remote(
      const std::shared_ptr<CacheableKey>& keyPtr,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<VersionTag>& versionTag) override;
  GfErrType removeNoThrow_remote(
      const std::shared_ptr<CacheableKey>& keyPtr,
      const std::shared_ptr<Cacheable>& cvalue,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<VersionTag>& versionTag) override;
  GfErrType removeNoThrowEX_remote(
      const std::shared_ptr<CacheableKey>& keyPtr,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<VersionTag>& versionTag) override;
  GfErrType invalidateNoThrow_remote(
      const std::shared_ptr<CacheableKey>& keyPtr,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<VersionTag>& versionTag) override;
  GfErrType getAllNoThrow_remote(
      const std::vector<std::shared_ptr<CacheableKey>>* keys,
      const std::shared_ptr<HashMapOfCacheable>& values,
      const std::shared_ptr<HashMapOfException>& exceptions,
      const std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>>&
          resultKeys,
      bool addToLocalCache,
      const std::shared_ptr<Serializable>& aCallbackArgument) override;
  GfErrType destroyRegionNoThrow_remote(
      const std::shared_ptr<Serializable>& aCallbackArgument) override;
  GfErrType registerKeysNoThrow(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      bool attemptFailover = true, TcrEndpoint* endpoint = nullptr,
      bool isDurable = false,
      InterestResultPolicy interestPolicy = InterestResultPolicy::NONE,
      bool receiveValues = true, TcrMessageReply* reply = nullptr);
  GfErrType unregisterKeysNoThrow(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      bool attemptFailover = true);
  GfErrType unregisterKeysNoThrowLocalDestroy(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      bool attemptFailover = true);
  GfErrType registerRegexNoThrow(
      const std::string& regex, bool attemptFailover = true,
      TcrEndpoint* endpoint = nullptr, bool isDurable = false,
      std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> resultKeys =
          nullptr,
      InterestResultPolicy interestPolicy = InterestResultPolicy::NONE,
      bool receiveValues = true, TcrMessageReply* reply = nullptr);
  GfErrType unregisterRegexNoThrow(const std::string& regex,
                                   bool attemptFailover = true);
  GfErrType unregisterRegexNoThrowLocalDestroy(const std::string& regex,
                                               bool attemptFailover = true);
  GfErrType clientNotificationHandler(const TcrMessage& msg);

  virtual void localInvalidateRegion_internal();

  virtual void localInvalidateForRegisterInterest(
      const std::vector<std::shared_ptr<CacheableKey>>& keys);

  InterestResultPolicy copyInterestList(
      std::vector<std::shared_ptr<CacheableKey>>& keysVector,
      std::unordered_map<std::shared_ptr<CacheableKey>, InterestResultPolicy>&
          interestList) const;
  void release(bool invokeCallbacks = true) override;

  GfErrType unregisterKeysBeforeDestroyRegion() override;

  bool isDurableClient() { return m_isDurableClnt; }
  std::shared_ptr<ThinClientBaseDM> m_tcrdm;
  std::recursive_mutex m_keysLock;
  mutable ACE_RW_Thread_Mutex m_rwDestroyLock;
  std::unordered_map<std::shared_ptr<CacheableKey>, InterestResultPolicy>
      m_interestList;
  std::unordered_map<std::string, InterestResultPolicy> m_interestListRegex;
  std::unordered_map<std::shared_ptr<CacheableKey>, InterestResultPolicy>
      m_durableInterestList;
  std::unordered_map<std::string, InterestResultPolicy>
      m_durableInterestListRegex;
  std::unordered_map<std::shared_ptr<CacheableKey>, InterestResultPolicy>
      m_interestListForUpdatesAsInvalidates;
  std::unordered_map<std::string, InterestResultPolicy>
      m_interestListRegexForUpdatesAsInvalidates;
  std::unordered_map<std::shared_ptr<CacheableKey>, InterestResultPolicy>
      m_durableInterestListForUpdatesAsInvalidates;
  std::unordered_map<std::string, InterestResultPolicy>
      m_durableInterestListRegexForUpdatesAsInvalidates;

  bool m_notifyRelease;
  std::mutex m_notificationMutex;

  bool m_isDurableClnt;

  virtual void handleMarker() {}

  virtual void destroyDM(bool keepEndpoints = false);
  virtual void setProcessedMarker(bool mark = true);

 private:
  bool isRegexRegistered(
      std::unordered_map<std::string, InterestResultPolicy>& interestListRegex,
      const std::string& regex, bool allKeys);
  GfErrType registerStoredRegex(
      TcrEndpoint*,
      std::unordered_map<std::string, InterestResultPolicy>& interestListRegex,
      bool isDurable = false, bool receiveValues = true);
  GfErrType unregisterStoredRegex(
      std::unordered_map<std::string, InterestResultPolicy>& interestListRegex);
  GfErrType unregisterStoredRegexLocalDestroy(
      std::unordered_map<std::string, InterestResultPolicy>& interestListRegex);
  void invalidateInterestList(
      std::unordered_map<std::shared_ptr<CacheableKey>, InterestResultPolicy>&
          interestList);
  GfErrType createOnServer(
      const std::shared_ptr<CacheableKey>& keyPtr,
      const std::shared_ptr<Cacheable>& cvalue,
      const std::shared_ptr<Serializable>& aCallbackArgument);
  GfErrType getNoThrow_FullObject(
      std::shared_ptr<EventId> eventId, std::shared_ptr<Cacheable>& fullObject,
      std::shared_ptr<VersionTag>& versionTag) override;

  GfErrType singleHopPutAllNoThrow_remote(
      ThinClientPoolDM* tcrdm, const HashMapOfCacheable& map,
      std::shared_ptr<VersionedCacheableObjectPartList>& versionedObjPartList,
      std::chrono::milliseconds timeout = DEFAULT_RESPONSE_TIMEOUT,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  GfErrType multiHopPutAllNoThrow_remote(
      const HashMapOfCacheable& map,
      std::shared_ptr<VersionedCacheableObjectPartList>& versionedObjPartList,
      std::chrono::milliseconds timeout = DEFAULT_RESPONSE_TIMEOUT,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);

  GfErrType singleHopRemoveAllNoThrow_remote(
      ThinClientPoolDM* tcrdm,
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      std::shared_ptr<VersionedCacheableObjectPartList>& versionedObjPartList,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);
  GfErrType multiHopRemoveAllNoThrow_remote(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      std::shared_ptr<VersionedCacheableObjectPartList>& versionedObjPartList,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr);

  ACE_RW_Thread_Mutex m_RegionMutex;
  bool m_isMetaDataRefreshed;

  typedef std::unordered_map<
      std::shared_ptr<BucketServerLocation>, std::shared_ptr<Serializable>,
      dereference_hash<std::shared_ptr<BucketServerLocation>>,
      dereference_equal_to<std::shared_ptr<BucketServerLocation>>>
      ResultMap;
  typedef std::unordered_map<
      std::shared_ptr<BucketServerLocation>, std::shared_ptr<CacheableInt32>,
      dereference_hash<std::shared_ptr<BucketServerLocation>>,
      dereference_equal_to<std::shared_ptr<BucketServerLocation>>>
      FailedServersMap;
};

// Chunk processing classes

/**
 * Handle each chunk of the chunked interest registration response.
 *
 *
 */
class ChunkedInterestResponse : public TcrChunkedResult {
 private:
  TcrMessage& m_msg;
  TcrMessage& m_replyMsg;
  std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> m_resultKeys;

 public:
  inline ChunkedInterestResponse(
      TcrMessage& msg,
      const std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>>&
          resultKeys,
      TcrMessageReply& replyMsg)
      : TcrChunkedResult(),
        m_msg(msg),
        m_replyMsg(replyMsg),
        m_resultKeys(resultKeys) {}

  ChunkedInterestResponse(const ChunkedInterestResponse&) = delete;
  ChunkedInterestResponse& operator=(const ChunkedInterestResponse&) = delete;

  ~ChunkedInterestResponse() noexcept override = default;

  inline const std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>>&
  getResultKeys() const {
    return m_resultKeys;
  }

  void handleChunk(const uint8_t* chunk, int32_t chunkLen,
                   uint8_t isLastChunkWithSecurity,
                   const CacheImpl* cacheImpl) override;
  virtual void reset() override;
};

/**
 * Handle each chunk of the chunked query response.
 *
 *
 */
class ChunkedQueryResponse : public TcrChunkedResult {
 private:
  TcrMessage& m_msg;
  std::shared_ptr<CacheableVector> m_queryResults;
  std::vector<std::string> m_structFieldNames;

  void skipClass(DataInput& input);

 public:
  inline explicit ChunkedQueryResponse(TcrMessage& msg)
      : TcrChunkedResult(),
        m_msg(msg),
        m_queryResults(CacheableVector::create()) {}

  ChunkedQueryResponse(const ChunkedQueryResponse&) = delete;
  ChunkedQueryResponse& operator=(const ChunkedQueryResponse&) = delete;

  ~ChunkedQueryResponse() noexcept override = default;

  inline const std::shared_ptr<CacheableVector>& getQueryResults() const {
    return m_queryResults;
  }

  inline const std::vector<std::string>& getStructFieldNames() const {
    return m_structFieldNames;
  }

  void handleChunk(const uint8_t* chunk, int32_t chunkLen,
                   uint8_t isLastChunkWithSecurity,
                   const CacheImpl* cacheImpl) override;
  void reset() override;

  void readObjectPartList(DataInput& input, bool isResultSet);
};

/**
 * Handle each chunk of the chunked function execution response.
 *
 *
 */
class ChunkedFunctionExecutionResponse : public TcrChunkedResult {
 private:
  TcrMessage& m_msg;
  // std::shared_ptr<CacheableVector>  m_functionExecutionResults;
  bool m_getResult;
  std::shared_ptr<ResultCollector> m_rc;
  std::shared_ptr<std::recursive_mutex> m_resultCollectorLock;

 public:
  inline ChunkedFunctionExecutionResponse(TcrMessage& msg, bool getResult,
                                          std::shared_ptr<ResultCollector> rc)
      : TcrChunkedResult(), m_msg(msg), m_getResult(getResult), m_rc(rc) {}

  inline ChunkedFunctionExecutionResponse(
      TcrMessage& msg, bool getResult, std::shared_ptr<ResultCollector> rc,
      const std::shared_ptr<std::recursive_mutex>& resultCollectorLock)
      : TcrChunkedResult(),
        m_msg(msg),
        m_getResult(getResult),
        m_rc(rc),
        m_resultCollectorLock(resultCollectorLock) {}

  ChunkedFunctionExecutionResponse(const ChunkedFunctionExecutionResponse&) =
      delete;
  ChunkedFunctionExecutionResponse& operator=(
      const ChunkedFunctionExecutionResponse&) = delete;

  ~ChunkedFunctionExecutionResponse() noexcept override = default;

  inline bool getResult() const { return m_getResult; }

  void handleChunk(const uint8_t* chunk, int32_t chunkLen,
                   uint8_t isLastChunkWithSecurity,
                   const CacheImpl* cacheImpl) override;
  void reset() override;
};

/**
 * Handle each chunk of the chunked getAll response.
 *
 *
 */
class ChunkedGetAllResponse : public TcrChunkedResult {
 private:
  TcrMessage& m_msg;
  ThinClientRegion* m_region;
  const std::vector<std::shared_ptr<CacheableKey>>* m_keys;
  std::shared_ptr<HashMapOfCacheable> m_values;
  std::shared_ptr<HashMapOfException> m_exceptions;
  std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> m_resultKeys;
  MapOfUpdateCounters& m_trackerMap;
  int32_t m_destroyTracker;
  bool m_addToLocalCache;
  uint32_t m_keysOffset;
  std::recursive_mutex& m_responseLock;

 public:
  inline ChunkedGetAllResponse(
      TcrMessage& msg, ThinClientRegion* region,
      const std::vector<std::shared_ptr<CacheableKey>>* keys,
      const std::shared_ptr<HashMapOfCacheable>& values,
      const std::shared_ptr<HashMapOfException>& exceptions,
      const std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>>&
          resultKeys,
      MapOfUpdateCounters& trackerMap, int32_t destroyTracker,
      bool addToLocalCache, std::recursive_mutex& responseLock)
      : TcrChunkedResult(),
        m_msg(msg),
        m_region(region),
        m_keys(keys),
        m_values(values),
        m_exceptions(exceptions),
        m_resultKeys(resultKeys),
        m_trackerMap(trackerMap),
        m_destroyTracker(destroyTracker),
        m_addToLocalCache(addToLocalCache),
        m_keysOffset(0),
        m_responseLock(responseLock) {}

  ChunkedGetAllResponse(const ChunkedGetAllResponse&) = delete;
  ChunkedGetAllResponse& operator=(const ChunkedGetAllResponse&) = delete;

  ~ChunkedGetAllResponse() noexcept override = default;

  void handleChunk(const uint8_t* chunk, int32_t chunkLen,
                   uint8_t isLastChunkWithSecurity,
                   const CacheImpl* cacheImpl) override;
  void reset() override;

  void add(const ChunkedGetAllResponse* other);
  bool getAddToLocalCache() { return m_addToLocalCache; }
  std::shared_ptr<HashMapOfCacheable> getValues() { return m_values; }
  std::shared_ptr<HashMapOfException> getExceptions() { return m_exceptions; }
  std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> getResultKeys() {
    return m_resultKeys;
  }
  MapOfUpdateCounters& getUpdateCounters() { return m_trackerMap; }
  std::recursive_mutex& getResponseLock() { return m_responseLock; }
};

/**
 * Handle each chunk of the chunked putAll response.
 */
class ChunkedPutAllResponse : public TcrChunkedResult {
 private:
  TcrMessage& m_msg;
  const std::shared_ptr<Region> m_region;
  std::recursive_mutex& m_responseLock;
  std::shared_ptr<VersionedCacheableObjectPartList> m_list;

 public:
  inline ChunkedPutAllResponse(
      const std::shared_ptr<Region>& region, TcrMessage& msg,
      std::recursive_mutex& responseLock,
      std::shared_ptr<VersionedCacheableObjectPartList>& list)
      : TcrChunkedResult(),
        m_msg(msg),
        m_region(region),
        m_responseLock(responseLock),
        m_list(list) {}

  ChunkedPutAllResponse(const ChunkedPutAllResponse&) = delete;
  ChunkedPutAllResponse& operator=(const ChunkedPutAllResponse&) = delete;

  ~ChunkedPutAllResponse() noexcept override = default;

  void handleChunk(const uint8_t* chunk, int32_t chunkLen,
                   uint8_t isLastChunkWithSecurity,
                   const CacheImpl* cacheImpl) override;
  void reset() override;
  std::shared_ptr<VersionedCacheableObjectPartList> getList() { return m_list; }
  std::recursive_mutex& getResponseLock() { return m_responseLock; }
};

/**
 * Handle each chunk of the chunked removeAll response.
 */
class ChunkedRemoveAllResponse : public TcrChunkedResult {
 private:
  TcrMessage& m_msg;
  const std::shared_ptr<Region> m_region;
  std::shared_ptr<VersionedCacheableObjectPartList> m_list;

 public:
  inline ChunkedRemoveAllResponse(
      const std::shared_ptr<Region>& region, TcrMessage& msg,
      std::shared_ptr<VersionedCacheableObjectPartList>& list)
      : TcrChunkedResult(), m_msg(msg), m_region(region), m_list(list) {}

  ChunkedRemoveAllResponse(const ChunkedRemoveAllResponse&) = delete;
  ChunkedRemoveAllResponse& operator=(const ChunkedRemoveAllResponse&) = delete;

  ~ChunkedRemoveAllResponse() noexcept override = default;

  void handleChunk(const uint8_t* chunk, int32_t chunkLen,
                   uint8_t isLastChunkWithSecurity,
                   const CacheImpl* cacheImpl) override;
  void reset() override;
  std::shared_ptr<VersionedCacheableObjectPartList> getList() { return m_list; }
};

/**
 * Handle each chunk of the chunked interest registration response.
 *
 *
 */
class ChunkedKeySetResponse : public TcrChunkedResult {
 private:
  TcrMessage& m_msg;
  TcrMessage& m_replyMsg;
  std::vector<std::shared_ptr<CacheableKey>>& m_resultKeys;

 public:
  inline ChunkedKeySetResponse(
      TcrMessage& msg, std::vector<std::shared_ptr<CacheableKey>>& resultKeys,
      TcrMessageReply& replyMsg)
      : TcrChunkedResult(),
        m_msg(msg),
        m_replyMsg(replyMsg),
        m_resultKeys(resultKeys) {}

  ChunkedKeySetResponse(const ChunkedKeySetResponse&) = delete;
  ChunkedKeySetResponse& operator=(const ChunkedKeySetResponse&) = delete;

  ~ChunkedKeySetResponse() noexcept override = default;

  void handleChunk(const uint8_t* chunk, int32_t chunkLen,
                   uint8_t isLastChunkWithSecurity,
                   const CacheImpl* cacheImpl) override;
  void reset() override;
};

class ChunkedDurableCQListResponse : public TcrChunkedResult {
 private:
  TcrMessage& m_msg;
  std::shared_ptr<CacheableArrayList> m_resultList;

 public:
  inline explicit ChunkedDurableCQListResponse(TcrMessage& msg)
      : TcrChunkedResult(),
        m_msg(msg),
        m_resultList(CacheableArrayList::create()) {}
  inline std::shared_ptr<CacheableArrayList> getResults() {
    return m_resultList;
  }

  ChunkedDurableCQListResponse(const ChunkedDurableCQListResponse&) = delete;
  ChunkedDurableCQListResponse& operator=(const ChunkedDurableCQListResponse&) =
      delete;

  ~ChunkedDurableCQListResponse() noexcept override = default;

  void handleChunk(const uint8_t* chunk, int32_t chunkLen,
                   uint8_t isLastChunkWithSecurity,
                   const CacheImpl* cacheImpl) override;
  void reset() override;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_THINCLIENTREGION_H_
