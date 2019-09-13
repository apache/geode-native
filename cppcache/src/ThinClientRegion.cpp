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

#include "ThinClientRegion.hpp"

#include <algorithm>
#include <limits>
#include <regex>

#include <geode/PoolManager.hpp>
#include <geode/Struct.hpp>
#include <geode/SystemProperties.hpp>
#include <geode/UserFunctionExecutionException.hpp>

#include "AutoDelete.hpp"
#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "DataInputInternal.hpp"
#include "PutAllPartialResultServerException.hpp"
#include "ReadWriteLock.hpp"
#include "RegionGlobalLocks.hpp"
#include "RemoteQuery.hpp"
#include "TcrConnectionManager.hpp"
#include "TcrDistributionManager.hpp"
#include "TcrEndpoint.hpp"
#include "ThinClientBaseDM.hpp"
#include "ThinClientPoolDM.hpp"
#include "UserAttributes.hpp"
#include "Utils.hpp"
#include "VersionedCacheableObjectPartList.hpp"
#include "util/bounds.hpp"
#include "util/exception.hpp"

namespace apache {
namespace geode {
namespace client {

static const std::regex PREDICATE_IS_FULL_QUERY_REGEX(
    "^\\s*(?:select|import)\\b", std::regex::icase);

void setThreadLocalExceptionMessage(const char* exMsg);

class PutAllWork : public PooledWork<GfErrType>,
                   private NonCopyable,
                   private NonAssignable {
  ThinClientPoolDM* m_poolDM;
  std::shared_ptr<BucketServerLocation> m_serverLocation;
  TcrMessage* m_request;
  TcrMessageReply* m_reply;
  MapOfUpdateCounters m_mapOfUpdateCounters;
  bool m_attemptFailover;
  bool m_isBGThread;
  std::shared_ptr<UserAttributes> m_userAttribute;
  const std::shared_ptr<Region> m_region;
  std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> m_keys;
  std::shared_ptr<HashMapOfCacheable> m_map;
  std::shared_ptr<VersionedCacheableObjectPartList> m_verObjPartListPtr;
  std::chrono::milliseconds m_timeout;
  std::shared_ptr<PutAllPartialResultServerException> m_papException;
  bool m_isPapeReceived;
  ChunkedPutAllResponse* m_resultCollector;
  // UNUSED const std::shared_ptr<Serializable>& m_aCallbackArgument;

 public:
  PutAllWork(
      ThinClientPoolDM* poolDM,
      const std::shared_ptr<BucketServerLocation>& serverLocation,
      const std::shared_ptr<Region>& region, bool attemptFailover,
      bool isBGThread, const std::shared_ptr<HashMapOfCacheable> map,
      const std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> keys,
      std::chrono::milliseconds timeout,
      const std::shared_ptr<Serializable>& aCallbackArgument)
      : m_poolDM(poolDM),
        m_serverLocation(serverLocation),
        m_attemptFailover(attemptFailover),
        m_isBGThread(isBGThread),
        m_userAttribute(nullptr),
        m_region(region),
        m_keys(keys),
        m_map(map),
        m_timeout(timeout),
        m_papException(nullptr),
        m_isPapeReceived(false)
  // UNUSED , m_aCallbackArgument(aCallbackArgument)
  {
    m_request = new TcrMessagePutAll(
        new DataOutput(m_region->getCache().createDataOutput()), m_region.get(),
        *m_map, m_timeout, m_poolDM, aCallbackArgument);
    m_reply = new TcrMessageReply(true, m_poolDM);

    // create new instanceof VCOPL
    std::recursive_mutex responseLock;
    m_verObjPartListPtr =
        std::make_shared<VersionedCacheableObjectPartList>(keys, responseLock);

    if (m_poolDM->isMultiUserMode()) {
      m_userAttribute = UserAttributes::threadLocalUserAttributes;
    }

    m_request->setTimeout(m_timeout);
    m_reply->setTimeout(m_timeout);
    m_resultCollector = new ChunkedPutAllResponse(
        m_region, *m_reply, responseLock, m_verObjPartListPtr);
    m_reply->setChunkedResultHandler(m_resultCollector);
  }

  ~PutAllWork() {
    delete m_request;
    delete m_reply;
    delete m_resultCollector;
  }

  TcrMessage* getReply() { return m_reply; }

  std::shared_ptr<HashMapOfCacheable> getPutAllMap() { return m_map; }

  std::shared_ptr<VersionedCacheableObjectPartList> getVerObjPartList() {
    return m_verObjPartListPtr;
  }

  ChunkedPutAllResponse* getResultCollector() { return m_resultCollector; }

  std::shared_ptr<BucketServerLocation> getServerLocation() {
    return m_serverLocation;
  }

  std::shared_ptr<PutAllPartialResultServerException> getPaPResultException() {
    return m_papException;
  }

  void init() {}
  GfErrType execute(void) {
    GuardUserAttributes gua;

    if (m_userAttribute != nullptr) {
      gua.setAuthenticatedView(m_userAttribute->getAuthenticatedView());
    }

    GfErrType err = GF_NOERR;
    err = m_poolDM->sendSyncRequest(*m_request, *m_reply, m_attemptFailover,
                                    m_isBGThread, m_serverLocation);

    // Set Version Tags
    LOGDEBUG(" m_verObjPartListPtr size = %d err = %d ",
             m_resultCollector->getList()->size(), err);
    m_verObjPartListPtr->setVersionedTagptr(
        m_resultCollector->getList()->getVersionedTagptr());

    if (err != GF_NOERR) {
      return err;
    } /*This can be GF_NOTCON, counterpart to java
                          ServerConnectivityException*/

    switch (m_reply->getMessageType()) {
      case TcrMessage::REPLY:
        break;
      case TcrMessage::RESPONSE:
        LOGDEBUG("PutAllwork execute err = %d ", err);
        break;
      case TcrMessage::EXCEPTION:
        // TODO::Check for the PAPException and READ
        // PutAllPartialResultServerException and set its member for later use.
        // set m_papException and m_isPapeReceived
        m_isPapeReceived = true;
        if (m_poolDM->isNotAuthorizedException(m_reply->getException())) {
          LOGDEBUG("received NotAuthorizedException");
          err = GF_AUTHENTICATION_FAILED_EXCEPTION;
        } else if (m_poolDM->isPutAllPartialResultException(
                       m_reply->getException())) {
          LOGDEBUG("received PutAllPartialResultException");
          err = GF_PUTALL_PARTIAL_RESULT_EXCEPTION;
        } else {
          LOGDEBUG("received unknown exception:%s", m_reply->getException());
          err = GF_PUTALL_PARTIAL_RESULT_EXCEPTION;
          // TODO should assign a new err code
        }

        break;
      case TcrMessage::PUT_DATA_ERROR:
        err = GF_CACHESERVER_EXCEPTION;
        break;
      default:
        LOGERROR("Unknown message type %d during region put-all",
                 m_reply->getMessageType());
        err = GF_NOTOBJ;
        break;
    }
    return err;
  }
};

class RemoveAllWork : public PooledWork<GfErrType>,
                      private NonCopyable,
                      private NonAssignable {
  ThinClientPoolDM* m_poolDM;
  std::shared_ptr<BucketServerLocation> m_serverLocation;
  TcrMessage* m_request;
  TcrMessageReply* m_reply;
  MapOfUpdateCounters m_mapOfUpdateCounters;
  bool m_attemptFailover;
  bool m_isBGThread;
  std::shared_ptr<UserAttributes> m_userAttribute;
  const std::shared_ptr<Region> m_region;
  const std::shared_ptr<Serializable>& m_aCallbackArgument;
  std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> m_keys;
  std::shared_ptr<VersionedCacheableObjectPartList> m_verObjPartListPtr;
  std::shared_ptr<PutAllPartialResultServerException> m_papException;
  bool m_isPapeReceived;
  ChunkedRemoveAllResponse* m_resultCollector;

 public:
  RemoveAllWork(
      ThinClientPoolDM* poolDM,
      const std::shared_ptr<BucketServerLocation>& serverLocation,
      const std::shared_ptr<Region>& region, bool attemptFailover,
      bool isBGThread,
      const std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> keys,
      const std::shared_ptr<Serializable>& aCallbackArgument)
      : m_poolDM(poolDM),
        m_serverLocation(serverLocation),
        m_attemptFailover(attemptFailover),
        m_isBGThread(isBGThread),
        m_userAttribute(nullptr),
        m_region(region),
        m_aCallbackArgument(aCallbackArgument),
        m_keys(keys),
        m_papException(nullptr),
        m_isPapeReceived(false) {
    m_request = new TcrMessageRemoveAll(
        new DataOutput(m_region->getCache().createDataOutput()), m_region.get(),
        *keys, m_aCallbackArgument, m_poolDM);
    m_reply = new TcrMessageReply(true, m_poolDM);
    // create new instanceof VCOPL
    std::recursive_mutex responseLock;
    m_verObjPartListPtr =
        std::make_shared<VersionedCacheableObjectPartList>(keys, responseLock);

    if (m_poolDM->isMultiUserMode()) {
      m_userAttribute = UserAttributes::threadLocalUserAttributes;
    }

    m_resultCollector = new ChunkedRemoveAllResponse(
        m_region, *m_reply, responseLock, m_verObjPartListPtr);
    m_reply->setChunkedResultHandler(m_resultCollector);
  }

  ~RemoveAllWork() {
    delete m_request;
    delete m_reply;
    delete m_resultCollector;
  }

  TcrMessage* getReply() { return m_reply; }

  std::shared_ptr<VersionedCacheableObjectPartList> getVerObjPartList() {
    return m_verObjPartListPtr;
  }

  ChunkedRemoveAllResponse* getResultCollector() { return m_resultCollector; }

  std::shared_ptr<BucketServerLocation> getServerLocation() {
    return m_serverLocation;
  }

  std::shared_ptr<PutAllPartialResultServerException> getPaPResultException() {
    return m_papException;
  }

  void init() {}
  GfErrType execute(void) {
    GuardUserAttributes gua;

    if (m_userAttribute != nullptr) {
      gua.setAuthenticatedView(m_userAttribute->getAuthenticatedView());
    }

    GfErrType err = GF_NOERR;
    err = m_poolDM->sendSyncRequest(*m_request, *m_reply, m_attemptFailover,
                                    m_isBGThread, m_serverLocation);

    // Set Version Tags
    LOGDEBUG(" m_verObjPartListPtr size = %d err = %d ",
             m_resultCollector->getList()->size(), err);
    m_verObjPartListPtr->setVersionedTagptr(
        m_resultCollector->getList()->getVersionedTagptr());

    if (err != GF_NOERR) {
      return err;
    } /*This can be GF_NOTCON, counterpart to java
                          ServerConnectivityException*/

    switch (m_reply->getMessageType()) {
      case TcrMessage::REPLY:
        break;
      case TcrMessage::RESPONSE:
        LOGDEBUG("RemoveAllWork execute err = %d ", err);
        break;
      case TcrMessage::EXCEPTION:
        // TODO::Check for the PAPException and READ
        // PutAllPartialResultServerException and set its member for later use.
        // set m_papException and m_isPapeReceived
        m_isPapeReceived = true;
        if (m_poolDM->isNotAuthorizedException(m_reply->getException())) {
          LOGDEBUG("received NotAuthorizedException");
          err = GF_AUTHENTICATION_FAILED_EXCEPTION;
        } else if (m_poolDM->isPutAllPartialResultException(
                       m_reply->getException())) {
          LOGDEBUG("received PutAllPartialResultException");
          err = GF_PUTALL_PARTIAL_RESULT_EXCEPTION;
        } else {
          LOGDEBUG("received unknown exception:%s", m_reply->getException());
          err = GF_PUTALL_PARTIAL_RESULT_EXCEPTION;
          // TODO should assign a new err code
        }

        break;
      case TcrMessage::PUT_DATA_ERROR:
        err = GF_CACHESERVER_EXCEPTION;
        break;
      default:
        LOGERROR("Unknown message type %d during region remove-all",
                 m_reply->getMessageType());
        err = GF_NOTOBJ;
        break;
    }
    return err;
  }
};

ThinClientRegion::ThinClientRegion(
    const std::string& name, CacheImpl* cacheImpl,
    const std::shared_ptr<RegionInternal>& rPtr, RegionAttributes attributes,
    const std::shared_ptr<CacheStatistics>& stats, bool shared)
    : LocalRegion(name, cacheImpl, rPtr, attributes, stats, shared),
      m_tcrdm(nullptr),
      m_notifyRelease(false),
      m_isMetaDataRefreshed(false) {
  m_transactionEnabled = true;
  m_isDurableClnt = !cacheImpl->getDistributedSystem()
                         .getSystemProperties()
                         .durableClientId()
                         .empty();
}

void ThinClientRegion::initTCR() {
  try {
    m_tcrdm = std::make_shared<TcrDistributionManager>(
        this, m_cacheImpl->tcrConnectionManager());
    m_tcrdm->init();
  } catch (const Exception& ex) {
    LOGERROR("Exception while initializing region: %s: %s",
             ex.getName().c_str(), ex.what());
    throw;
  }
}

void ThinClientRegion::registerKeys(
    const std::vector<std::shared_ptr<CacheableKey>>& keys, bool isDurable,
    bool getInitialValues, bool receiveValues) {
  auto pool = m_cacheImpl->getPoolManager().find(getAttributes().getPoolName());
  if (pool != nullptr) {
    if (!pool->getSubscriptionEnabled()) {
      LOGERROR(
          "Registering keys is supported "
          "only if subscription-enabled attribute is true for pool " +
          pool->getName());
      throw UnsupportedOperationException(
          "Registering keys is supported "
          "only if pool subscription-enabled attribute is true.");
    }
  }
  if (keys.empty()) {
    LOGERROR("Register keys list is empty");
    throw IllegalArgumentException(
        "Register keys "
        "keys vector is empty");
  }
  if (isDurable && !isDurableClient()) {
    LOGERROR(
        "Register keys durable flag is only applicable for durable clients");
    throw IllegalStateException(
        "Durable flag only applicable for "
        "durable clients");
  }
  if (getInitialValues && !m_regionAttributes.getCachingEnabled()) {
    LOGERROR(
        "Register keys getInitialValues flag is only applicable for caching"
        "clients");
    throw IllegalStateException(
        "getInitialValues flag only applicable for caching clients");
  }

  InterestResultPolicy interestPolicy = InterestResultPolicy::NONE;
  if (getInitialValues) {
    interestPolicy = InterestResultPolicy::KEYS_VALUES;
  }

  LOGDEBUG("ThinClientRegion::registerKeys : interestpolicy is %d",
           interestPolicy.ordinal);

  GfErrType err = registerKeysNoThrow(keys, true, nullptr, isDurable,
                                      interestPolicy, receiveValues);

  if (m_tcrdm->isFatalError(err)) {
    throwExceptionIfError("Region::registerKeys", err);
  }

  throwExceptionIfError("Region::registerKeys", err);
}

void ThinClientRegion::unregisterKeys(
    const std::vector<std::shared_ptr<CacheableKey>>& keys) {
  auto pool = m_cacheImpl->getPoolManager().find(getAttributes().getPoolName());
  if (pool != nullptr) {
    if (!pool->getSubscriptionEnabled()) {
      LOGERROR(
          "Unregister keys is supported "
          "only if subscription-enabled attribute is true for pool " +
          pool->getName());
      throw UnsupportedOperationException(
          "Unregister keys is supported "
          "only if pool subscription-enabled attribute is true.");
    }
  } else {
    if (!getAttributes().getClientNotificationEnabled()) {
      LOGERROR(
          "Unregister keys is supported "
          "only if region client-notification attribute is true.");
      throw UnsupportedOperationException(
          "Unregister keys is supported "
          "only if region client-notification attribute is true.");
    }
  }
  if (keys.empty()) {
    LOGERROR("Unregister keys list is empty");
    throw IllegalArgumentException(
        "Unregister keys "
        "keys vector is empty");
  }
  GfErrType err = unregisterKeysNoThrow(keys);
  throwExceptionIfError("Region::unregisterKeys", err);
}

void ThinClientRegion::registerAllKeys(bool isDurable, bool getInitialValues,
                                       bool receiveValues) {
  auto pool = m_cacheImpl->getPoolManager().find(getAttributes().getPoolName());
  if (pool != nullptr) {
    if (!pool->getSubscriptionEnabled()) {
      LOGERROR(
          "Register all keys is supported only "
          "if subscription-enabled attribute is true for pool " +
          pool->getName());
      throw UnsupportedOperationException(
          "Register all keys is supported only "
          "if pool subscription-enabled attribute is true.");
    }
  }
  if (isDurable && !isDurableClient()) {
    LOGERROR(
        "Register all keys durable flag is only applicable for durable "
        "clients");
    throw IllegalStateException(
        "Durable flag only applicable for durable clients");
  }

  if (getInitialValues && !m_regionAttributes.getCachingEnabled()) {
    LOGERROR(
        "Register all keys getInitialValues flag is only applicable for caching"
        "clients");
    throw IllegalStateException(
        "getInitialValues flag only applicable for caching clients");
  }

  InterestResultPolicy interestPolicy = InterestResultPolicy::NONE;
  if (getInitialValues) {
    interestPolicy = InterestResultPolicy::KEYS_VALUES;
  } else {
    interestPolicy = InterestResultPolicy::KEYS;
  }

  LOGDEBUG("ThinClientRegion::registerAllKeys : interestpolicy is %d",
           interestPolicy.ordinal);

  std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> resultKeys;
  //  if we need to fetch initial data, then we get the keys in
  // that call itself using the special GET_ALL message and do not need
  // to get the keys in the initial  register interest  call
  GfErrType err =
      registerRegexNoThrow(".*", true, nullptr, isDurable, resultKeys,
                           interestPolicy, receiveValues);

  if (m_tcrdm->isFatalError(err)) {
    throwExceptionIfError("Region::registerAllKeys", err);
  }

  // Get the entries from the server using a special GET_ALL message
  throwExceptionIfError("Region::registerAllKeys", err);
}

void ThinClientRegion::registerRegex(const std::string& regex, bool isDurable,
                                     bool getInitialValues,
                                     bool receiveValues) {
  auto pool = m_cacheImpl->getPoolManager().find(getAttributes().getPoolName());
  if (pool != nullptr) {
    if (!pool->getSubscriptionEnabled()) {
      LOGERROR(
          "Register regex is supported only if "
          "subscription-enabled attribute is true for pool " +
          pool->getName());
      throw UnsupportedOperationException(
          "Register regex is supported only if "
          "pool subscription-enabled attribute is true.");
    }
  }
  if (isDurable && !isDurableClient()) {
    LOGERROR("Register regex durable flag only applicable for durable clients");
    throw IllegalStateException(
        "Durable flag only applicable for durable clients");
  }

  if (regex.empty()) {
    throw IllegalArgumentException(
        "Region::registerRegex: Regex string is empty");
  }

  auto interestPolicy = InterestResultPolicy::NONE;
  if (getInitialValues) {
    interestPolicy = InterestResultPolicy::KEYS_VALUES;
  } else {
    interestPolicy = InterestResultPolicy::KEYS;
  }

  LOGDEBUG("ThinClientRegion::registerRegex : interestpolicy is %d",
           interestPolicy.ordinal);

  auto resultKeys2 =
      std::make_shared<std::vector<std::shared_ptr<CacheableKey>>>();

  //  if we need to fetch initial data for "allKeys" case, then we
  // get the keys in that call itself using the special GET_ALL message and
  // do not need to get the keys in the initial  register interest  call
  GfErrType err =
      registerRegexNoThrow(regex, true, nullptr, isDurable, resultKeys2,
                           interestPolicy, receiveValues);

  if (m_tcrdm->isFatalError(err)) {
    throwExceptionIfError("Region::registerRegex", err);
  }

  throwExceptionIfError("Region::registerRegex", err);
}

void ThinClientRegion::unregisterRegex(const std::string& regex) {
  auto pool = m_cacheImpl->getPoolManager().find(getAttributes().getPoolName());
  if (pool != nullptr) {
    if (!pool->getSubscriptionEnabled()) {
      LOGERROR(
          "Unregister regex is supported only if "
          "subscription-enabled attribute is true for pool " +
          pool->getName());
      throw UnsupportedOperationException(
          "Unregister regex is supported only if "
          "pool subscription-enabled attribute is true.");
    }
  }

  if (regex.empty()) {
    LOGERROR("Unregister regex string is empty");
    throw IllegalArgumentException("Unregister regex string is empty");
  }

  GfErrType err = unregisterRegexNoThrow(regex);
  throwExceptionIfError("Region::unregisterRegex", err);
}

void ThinClientRegion::unregisterAllKeys() {
  auto pool = m_cacheImpl->getPoolManager().find(getAttributes().getPoolName());
  if (pool != nullptr) {
    if (!pool->getSubscriptionEnabled()) {
      LOGERROR(
          "Unregister all keys is supported only if "
          "subscription-enabled attribute is true for pool " +
          pool->getName());
      throw UnsupportedOperationException(
          "Unregister all keys is supported only if "
          "pool subscription-enabled attribute is true.");
    }
  }
  GfErrType err = unregisterRegexNoThrow(".*");
  throwExceptionIfError("Region::unregisterAllKeys", err);
}

std::shared_ptr<SelectResults> ThinClientRegion::query(
    const std::string& predicate, std::chrono::milliseconds timeout) {
  util::PROTOCOL_OPERATION_TIMEOUT_BOUNDS(timeout);

  CHECK_DESTROY_PENDING(TryReadGuard, Region::query);

  if (predicate.empty()) {
    LOGERROR("Region query predicate string is empty");
    throw IllegalArgumentException("Region query predicate string is empty");
  }

  std::string squery;
  if (std::regex_search(predicate, PREDICATE_IS_FULL_QUERY_REGEX)) {
    squery = predicate;
  } else {
    squery = "select distinct * from ";
    squery += getFullPath();
    squery += " this where ";
    squery += predicate;
  }

  std::shared_ptr<RemoteQuery> queryPtr;

  if (auto poolDM = std::dynamic_pointer_cast<ThinClientPoolDM>(m_tcrdm)) {
    queryPtr = std::dynamic_pointer_cast<RemoteQuery>(
        poolDM->getQueryServiceWithoutCheck()->newQuery(squery.c_str()));
  } else {
    queryPtr = std::dynamic_pointer_cast<RemoteQuery>(
        m_cacheImpl->getQueryService()->newQuery(squery.c_str()));
  }

  return queryPtr->execute(timeout, "Region::query", m_tcrdm.get(), nullptr);
}

bool ThinClientRegion::existsValue(const std::string& predicate,
                                   std::chrono::milliseconds timeout) {
  util::PROTOCOL_OPERATION_TIMEOUT_BOUNDS(timeout);

  auto results = query(predicate, timeout);

  if (results == nullptr) {
    return false;
  }

  return results->size() > 0;
}

GfErrType ThinClientRegion::unregisterKeysBeforeDestroyRegion() {
  auto pool = m_cacheImpl->getPoolManager().find(getAttributes().getPoolName());
  if (pool != nullptr) {
    if (!pool->getSubscriptionEnabled()) {
      LOGDEBUG(
          "pool subscription-enabled attribute is false, No need to Unregister "
          "keys");
      return GF_NOERR;
    }
  }
  GfErrType err = GF_NOERR;
  GfErrType opErr = GF_NOERR;

  opErr = unregisterStoredRegexLocalDestroy(m_interestListRegex);
  err = opErr != GF_NOERR ? opErr : err;
  opErr = unregisterStoredRegexLocalDestroy(m_durableInterestListRegex);
  err = opErr != GF_NOERR ? opErr : err;
  opErr = unregisterStoredRegexLocalDestroy(
      m_interestListRegexForUpdatesAsInvalidates);
  err = opErr != GF_NOERR ? opErr : err;
  opErr = unregisterStoredRegexLocalDestroy(
      m_durableInterestListRegexForUpdatesAsInvalidates);
  err = opErr != GF_NOERR ? opErr : err;

  std::vector<std::shared_ptr<CacheableKey>> keysVec;
  copyInterestList(keysVec, m_interestList);
  opErr = unregisterKeysNoThrowLocalDestroy(keysVec, false);
  err = opErr != GF_NOERR ? opErr : err;

  std::vector<std::shared_ptr<CacheableKey>> keysVecDurable;
  copyInterestList(keysVecDurable, m_durableInterestList);
  opErr = unregisterKeysNoThrowLocalDestroy(keysVecDurable, false);
  err = opErr != GF_NOERR ? opErr : err;

  std::vector<std::shared_ptr<CacheableKey>> keysVecForUpdatesAsInvalidates;
  copyInterestList(keysVecForUpdatesAsInvalidates,
                   m_interestListForUpdatesAsInvalidates);
  opErr =
      unregisterKeysNoThrowLocalDestroy(keysVecForUpdatesAsInvalidates, false);
  err = opErr != GF_NOERR ? opErr : err;

  std::vector<std::shared_ptr<CacheableKey>>
      keysVecDurableForUpdatesAsInvalidates;
  copyInterestList(keysVecDurableForUpdatesAsInvalidates,
                   m_durableInterestListForUpdatesAsInvalidates);
  opErr = unregisterKeysNoThrowLocalDestroy(
      keysVecDurableForUpdatesAsInvalidates, false);
  err = opErr != GF_NOERR ? opErr : err;
  return err;
}
std::shared_ptr<Serializable> ThinClientRegion::selectValue(
    const std::string& predicate, std::chrono::milliseconds timeout) {
  auto results = query(predicate, timeout);

  if (results == nullptr || results->size() == 0) {
    return nullptr;
  }

  if (results->size() > 1) {
    throw QueryException("selectValue has more than one result");
  }

  return results->operator[](0);
}

std::vector<std::shared_ptr<CacheableKey>> ThinClientRegion::serverKeys() {
  CHECK_DESTROY_PENDING(TryReadGuard, Region::serverKeys);

  TcrMessageReply reply(true, m_tcrdm.get());
  TcrMessageKeySet request(new DataOutput(m_cacheImpl->createDataOutput()),
                           m_fullPath, m_tcrdm.get());
  reply.setMessageTypeRequest(TcrMessage::KEY_SET);
  std::vector<std::shared_ptr<CacheableKey>> serverKeys;
  ChunkedKeySetResponse resultCollector(request, serverKeys, reply);
  reply.setChunkedResultHandler(&resultCollector);

  GfErrType err = GF_NOERR;

  err = m_tcrdm->sendSyncRequest(request, reply);

  throwExceptionIfError("Region::serverKeys", err);

  switch (reply.getMessageType()) {
    case TcrMessage::RESPONSE: {
      // keyset result is handled by ChunkedKeySetResponse
      break;
    }
    case TcrMessage::EXCEPTION: {
      err = handleServerException("Region:serverKeys", reply.getException());
      break;
    }
    case TcrMessage::KEY_SET_DATA_ERROR: {
      LOGERROR("Region serverKeys: an error occurred on the server");
      err = GF_CACHESERVER_EXCEPTION;
      break;
    }
    default: {
      LOGERROR("Unknown message type %d during region serverKeys",
               reply.getMessageType());
      err = GF_MSG;
      break;
    }
  }
  throwExceptionIfError("Region::serverKeys", err);

  return serverKeys;
}

bool ThinClientRegion::containsKeyOnServer(
    const std::shared_ptr<CacheableKey>& keyPtr) const {
  GfErrType err = GF_NOERR;
  bool ret = false;

  /** @brief Create message and send to bridge server */

  TcrMessageContainsKey request(
      new DataOutput(m_cacheImpl->createDataOutput()), this, keyPtr,
      static_cast<std::shared_ptr<Serializable>>(nullptr), true, m_tcrdm.get());
  TcrMessageReply reply(true, m_tcrdm.get());
  reply.setMessageTypeRequest(TcrMessage::CONTAINS_KEY);
  err = m_tcrdm->sendSyncRequest(request, reply);

  switch (reply.getMessageType()) {
    case TcrMessage::RESPONSE:
      ret = reply.getBoolValue();
      break;

    case TcrMessage::EXCEPTION:
      err = handleServerException("Region::containsKeyOnServer:",
                                  reply.getException());
      break;

    case TcrMessage::REQUEST_DATA_ERROR:
      LOGERROR(
          "Region::containsKeyOnServer: read error occurred on the endpoint %s",
          m_tcrdm->getActiveEndpoint()->name().c_str());
      err = GF_CACHESERVER_EXCEPTION;
      break;

    default:
      LOGERROR("Unknown message type in Region::containsKeyOnServer %d",
               reply.getMessageType());
      err = GF_MSG;
      break;
  }

  auto rptr = CacheableBoolean::create(ret);

  rptr = std::dynamic_pointer_cast<CacheableBoolean>(handleReplay(err, rptr));
  throwExceptionIfError("Region::containsKeyOnServer ", err);
  return rptr->value();
}

bool ThinClientRegion::containsValueForKey_remote(
    const std::shared_ptr<CacheableKey>& keyPtr) const {
  GfErrType err = GF_NOERR;
  bool ret = false;

  /** @brief Create message and send to bridge server */

  TcrMessageContainsKey request(
      new DataOutput(m_cacheImpl->createDataOutput()), this, keyPtr,
      static_cast<std::shared_ptr<Serializable>>(nullptr), false,
      m_tcrdm.get());
  TcrMessageReply reply(true, m_tcrdm.get());
  reply.setMessageTypeRequest(TcrMessage::CONTAINS_KEY);
  err = m_tcrdm->sendSyncRequest(request, reply);
  // if ( err != GF_NOERR ) return ret;

  switch (reply.getMessageType()) {
    case TcrMessage::RESPONSE:
      ret = reply.getBoolValue();
      break;

    case TcrMessage::EXCEPTION:
      err = handleServerException("Region::containsValueForKey:",
                                  reply.getException());
      break;

    case TcrMessage::REQUEST_DATA_ERROR:
      LOGERROR(
          "Region::containsValueForKey: read error occurred on the endpoint %s",
          m_tcrdm->getActiveEndpoint()->name().c_str());
      err = GF_CACHESERVER_EXCEPTION;
      break;

    default:
      LOGERROR("Unknown message type in Region::containsValueForKey %d",
               reply.getMessageType());
      err = GF_MSG;
      break;
  }

  auto rptr = CacheableBoolean::create(ret);

  rptr = std::dynamic_pointer_cast<CacheableBoolean>(handleReplay(err, rptr));

  throwExceptionIfError("Region::containsValueForKey ", err);
  return rptr->value();
}

void ThinClientRegion::clear(
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  GfErrType err = GF_NOERR;
  err = localClearNoThrow(aCallbackArgument, CacheEventFlags::NORMAL);
  if (err != GF_NOERR) throwExceptionIfError("Region::clear", err);

  /** @brief Create message and send to bridge server */

  TcrMessageClearRegion request(new DataOutput(m_cacheImpl->createDataOutput()),
                                this, aCallbackArgument,
                                std::chrono::milliseconds(-1), m_tcrdm.get());
  TcrMessageReply reply(true, m_tcrdm.get());
  err = m_tcrdm->sendSyncRequest(request, reply);
  if (err != GF_NOERR) throwExceptionIfError("Region::clear", err);

  switch (reply.getMessageType()) {
    case TcrMessage::REPLY:
      LOGFINE("Region %s clear message sent to server successfully",
              m_fullPath.c_str());
      break;
    case TcrMessage::EXCEPTION:
      err = handleServerException("Region::clear:", reply.getException());
      break;

    case TcrMessage::CLEAR_REGION_DATA_ERROR:
      LOGERROR("Region clear read error occurred on the endpoint %s",
               m_tcrdm->getActiveEndpoint()->name().c_str());
      err = GF_CACHESERVER_EXCEPTION;
      break;

    default:
      LOGERROR("Unknown message type %d during region clear",
               reply.getMessageType());
      err = GF_MSG;
      break;
  }
  if (err == GF_NOERR) {
    err = invokeCacheListenerForRegionEvent(
        aCallbackArgument, CacheEventFlags::NORMAL, AFTER_REGION_CLEAR);
  }
  throwExceptionIfError("Region::clear", err);
}

GfErrType ThinClientRegion::getNoThrow_remote(
    const std::shared_ptr<CacheableKey>& keyPtr,
    std::shared_ptr<Cacheable>& valPtr,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    std::shared_ptr<VersionTag>& versionTag) {
  GfErrType err = GF_NOERR;

  /** @brief Create message and send to bridge server */

  TcrMessageRequest request(new DataOutput(m_cacheImpl->createDataOutput()),
                            this, keyPtr, aCallbackArgument, m_tcrdm.get());
  TcrMessageReply reply(true, m_tcrdm.get());
  err = m_tcrdm->sendSyncRequest(request, reply);
  if (err != GF_NOERR) return err;

  // put the object into local region
  switch (reply.getMessageType()) {
    case TcrMessage::RESPONSE: {
      valPtr = reply.getValue();
      versionTag = reply.getVersionTag();
      break;
    }
    case TcrMessage::EXCEPTION: {
      err = handleServerException("Region::get", reply.getException());
      break;
    }
    case TcrMessage::REQUEST_DATA_ERROR: {
      // LOGERROR("A read error occurred on the endpoint %s",
      //    m_tcrdm->getActiveEndpoint()->name().c_str());
      err = GF_CACHESERVER_EXCEPTION;
      break;
    }
    default: {
      LOGERROR("Unknown message type %d while getting entry from region",
               reply.getMessageType());
      err = GF_MSG;
      break;
    }
  }
  return err;
}

GfErrType ThinClientRegion::invalidateNoThrow_remote(
    const std::shared_ptr<CacheableKey>& keyPtr,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    std::shared_ptr<VersionTag>& versionTag) {
  GfErrType err = GF_NOERR;

  TcrMessageInvalidate request(new DataOutput(m_cacheImpl->createDataOutput()),
                               this, keyPtr, aCallbackArgument, m_tcrdm.get());
  TcrMessageReply reply(true, m_tcrdm.get());
  err = m_tcrdm->sendSyncRequest(request, reply);
  if (err != GF_NOERR) return err;

  // put the object into local region
  switch (reply.getMessageType()) {
    case TcrMessage::REPLY: {
      versionTag = reply.getVersionTag();
      break;
    }
    case TcrMessage::EXCEPTION: {
      err = handleServerException("Region::get", reply.getException());
      break;
    }
    case TcrMessage::INVALIDATE_ERROR: {
      err = GF_CACHESERVER_EXCEPTION;
      break;
    }
    default: {
      LOGERROR("Unknown message type %d while getting entry from region",
               reply.getMessageType());
      err = GF_MSG;
      break;
    }
  }
  return err;
}

GfErrType ThinClientRegion::putNoThrow_remote(
    const std::shared_ptr<CacheableKey>& keyPtr,
    const std::shared_ptr<Cacheable>& valuePtr,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    std::shared_ptr<VersionTag>& versionTag, bool checkDelta) {
  GfErrType err = GF_NOERR;
  // do TCR put
  // bool delta = valuePtr->hasDelta();
  bool delta = false;
  auto&& conFlationValue = getCacheImpl()
                               ->getDistributedSystem()
                               .getSystemProperties()
                               .conflateEvents();
  if (checkDelta && valuePtr && conFlationValue != "true" &&
      ThinClientBaseDM::isDeltaEnabledOnServer()) {
    auto&& temp = std::dynamic_pointer_cast<Delta>(valuePtr);
    delta = temp && temp->hasDelta();
  }
  TcrMessagePut request(new DataOutput(m_cacheImpl->createDataOutput()), this,
                        keyPtr, valuePtr, aCallbackArgument, delta,
                        m_tcrdm.get());
  TcrMessageReply* reply = new TcrMessageReply(true, m_tcrdm.get());
  err = m_tcrdm->sendSyncRequest(request, *reply);
  if (delta) {
    // Does not check whether success of failure..
    m_cacheImpl->getCachePerfStats().incDeltaPut();
    if (reply->getMessageType() == TcrMessage::PUT_DELTA_ERROR) {
      // Try without delta
      TcrMessagePut request(new DataOutput(m_cacheImpl->createDataOutput()),
                            this, keyPtr, valuePtr, aCallbackArgument, false,
                            m_tcrdm.get(), false, true);
      delete reply;
      reply = new TcrMessageReply(true, m_tcrdm.get());
      err = m_tcrdm->sendSyncRequest(request, *reply);
    }
  }
  if (err != GF_NOERR) return err;

  // put the object into local region
  switch (reply->getMessageType()) {
    case TcrMessage::REPLY: {
      versionTag = reply->getVersionTag();
      break;
    }
    case TcrMessage::EXCEPTION: {
      err = handleServerException("Region::put", reply->getException());
      break;
    }
    case TcrMessage::PUT_DATA_ERROR: {
      err = GF_CACHESERVER_EXCEPTION;
      break;
    }
    default: {
      LOGERROR("Unknown message type %d during region put reply",
               reply->getMessageType());
      err = GF_MSG;
    }
  }
  delete reply;
  reply = nullptr;
  return err;
}

GfErrType ThinClientRegion::createNoThrow_remote(
    const std::shared_ptr<CacheableKey>& keyPtr,
    const std::shared_ptr<Cacheable>& valuePtr,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    std::shared_ptr<VersionTag>& versionTag) {
  return putNoThrow_remote(keyPtr, valuePtr, aCallbackArgument, versionTag,
                           false);
}

GfErrType ThinClientRegion::destroyNoThrow_remote(
    const std::shared_ptr<CacheableKey>& keyPtr,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    std::shared_ptr<VersionTag>& versionTag) {
  GfErrType err = GF_NOERR;

  // do TCR destroy
  TcrMessageDestroy request(new DataOutput(m_cacheImpl->createDataOutput()),
                            this, keyPtr, nullptr, aCallbackArgument,
                            m_tcrdm.get());
  TcrMessageReply reply(true, m_tcrdm.get());
  err = m_tcrdm->sendSyncRequest(request, reply);
  if (err != GF_NOERR) return err;

  switch (reply.getMessageType()) {
    case TcrMessage::REPLY: {
      if (reply.getEntryNotFound() == 1) {
        err = GF_CACHE_ENTRY_NOT_FOUND;
      } else {
        LOGDEBUG("Remote key [%s] is destroyed successfully in region %s",
                 Utils::nullSafeToString(keyPtr).c_str(), m_fullPath.c_str());
      }
      versionTag = reply.getVersionTag();
      break;
    }
    case TcrMessage::EXCEPTION: {
      err = handleServerException("Region::destroy", reply.getException());
      break;
    }
    case TcrMessage::DESTROY_DATA_ERROR: {
      err = GF_CACHE_ENTRY_DESTROYED_EXCEPTION;
      break;
    }
    default: {
      LOGERROR("Unknown message type %d while destroying region entry",
               reply.getMessageType());
      err = GF_MSG;
    }
  }

  return err;
}  // destroyNoThrow_remote()

GfErrType ThinClientRegion::removeNoThrow_remote(
    const std::shared_ptr<CacheableKey>& keyPtr,
    const std::shared_ptr<Cacheable>& cvalue,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    std::shared_ptr<VersionTag>& versionTag) {
  GfErrType err = GF_NOERR;

  // do TCR remove
  TcrMessageDestroy request(new DataOutput(m_cacheImpl->createDataOutput()),
                            this, keyPtr, cvalue, aCallbackArgument,
                            m_tcrdm.get());
  TcrMessageReply reply(true, m_tcrdm.get());
  err = m_tcrdm->sendSyncRequest(request, reply);
  if (err != GF_NOERR) {
    return err;
  }
  switch (reply.getMessageType()) {
    case TcrMessage::REPLY: {
      if (reply.getEntryNotFound() == 1) {
        err = GF_ENOENT;
      } else {
        LOGDEBUG("Remote key [%s] is removed successfully in region %s",
                 Utils::nullSafeToString(keyPtr).c_str(), m_fullPath.c_str());
      }
      versionTag = reply.getVersionTag();
      break;
    }
    case TcrMessage::EXCEPTION: {
      err = handleServerException("Region::remove", reply.getException());
      break;
    }
    case TcrMessage::DESTROY_DATA_ERROR: {
      err = GF_CACHE_ENTRY_DESTROYED_EXCEPTION;
      break;
    }
    default: {
      LOGERROR("Unknown message type %d while removing region entry",
               reply.getMessageType());
      err = GF_MSG;
    }
  }
  return err;
}

GfErrType ThinClientRegion::removeNoThrowEX_remote(
    const std::shared_ptr<CacheableKey>& keyPtr,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    std::shared_ptr<VersionTag>& versionTag) {
  GfErrType err = GF_NOERR;

  // do TCR remove
  TcrMessageDestroy request(new DataOutput(m_cacheImpl->createDataOutput()),
                            this, keyPtr, nullptr, aCallbackArgument,
                            m_tcrdm.get());
  TcrMessageReply reply(true, m_tcrdm.get());
  err = m_tcrdm->sendSyncRequest(request, reply);
  if (err != GF_NOERR) {
    return err;
  }
  switch (reply.getMessageType()) {
    case TcrMessage::REPLY: {
      versionTag = reply.getVersionTag();
      if (reply.getEntryNotFound() == 1) {
        err = GF_ENOENT;
      } else {
        LOGDEBUG("Remote key [%s] is removed successfully in region %s",
                 Utils::nullSafeToString(keyPtr).c_str(), m_fullPath.c_str());
      }
      break;
    }
    case TcrMessage::EXCEPTION: {
      err = handleServerException("Region::removeEx", reply.getException());
      break;
    }
    case TcrMessage::DESTROY_DATA_ERROR: {
      err = GF_CACHE_ENTRY_DESTROYED_EXCEPTION;
      break;
    }
    default: {
      LOGERROR("Unknown message type %d while removing region entry",
               reply.getMessageType());
      err = GF_MSG;
    }
  }
  return err;
}

GfErrType ThinClientRegion::getAllNoThrow_remote(
    const std::vector<std::shared_ptr<CacheableKey>>* keys,
    const std::shared_ptr<HashMapOfCacheable>& values,
    const std::shared_ptr<HashMapOfException>& exceptions,
    const std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>>&
        resultKeys,
    bool addToLocalCache,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  GfErrType err = GF_NOERR;
  MapOfUpdateCounters updateCountMap;
  int32_t destroyTracker = 0;
  addToLocalCache = addToLocalCache && m_regionAttributes.getCachingEnabled();
  if (addToLocalCache && !m_regionAttributes.getConcurrencyChecksEnabled()) {
    // start tracking the entries
    if (keys == nullptr) {
      // track all entries with destroy tracking for non-existent entries
      destroyTracker = m_entries->addTrackerForAllEntries(updateCountMap, true);
    } else {
      for (const auto& key : *keys) {
        std::shared_ptr<Cacheable> oldValue;
        int updateCount =
            m_entries->addTrackerForEntry(key, oldValue, true, false, false);
        updateCountMap.emplace(key, updateCount);
      }
    }
  }
  // create the GET_ALL request
  TcrMessageGetAll request(new DataOutput(m_cacheImpl->createDataOutput()),
                           this, keys, m_tcrdm.get(), aCallbackArgument);

  TcrMessageReply reply(true, m_tcrdm.get());
  std::recursive_mutex responseLock;
  // need to check
  TcrChunkedResult* resultCollector(new ChunkedGetAllResponse(
      reply, this, keys, values, exceptions, resultKeys, updateCountMap,
      destroyTracker, addToLocalCache, responseLock));

  reply.setChunkedResultHandler(resultCollector);
  err = m_tcrdm->sendSyncRequest(request, reply);

  if (addToLocalCache && !m_regionAttributes.getConcurrencyChecksEnabled()) {
    // remove the tracking for remaining keys in case some keys do not have
    // values from server in GII
    for (MapOfUpdateCounters::const_iterator iter = updateCountMap.begin();
         iter != updateCountMap.end(); ++iter) {
      if (iter->second >= 0) {
        m_entries->removeTrackerForEntry(iter->first);
      }
    }
    // remove tracking for destroys
    if (destroyTracker > 0) {
      m_entries->removeDestroyTracking();
    }
  }
  delete resultCollector;
  if (err != GF_NOERR) {
    return err;
  }

  switch (reply.getMessageType()) {
    case TcrMessage::RESPONSE: {
      // nothing to be done; put in local region, if required,
      // is handled by ChunkedGetAllResponse
      break;
    }
    case TcrMessage::EXCEPTION: {
      err = handleServerException("Region:getAll", reply.getException());
      break;
    }
    case TcrMessage::GET_ALL_DATA_ERROR: {
      LOGERROR("Region get-all: a read error occurred on the endpoint %s",
               m_tcrdm->getActiveEndpoint()->name().c_str());
      err = GF_CACHESERVER_EXCEPTION;
      break;
    }
    default: {
      LOGERROR("Unknown message type %d during region get-all",
               reply.getMessageType());
      err = GF_MSG;
      break;
    }
  }
  return err;
}

GfErrType ThinClientRegion::singleHopPutAllNoThrow_remote(
    ThinClientPoolDM* tcrdm, const HashMapOfCacheable& map,
    std::shared_ptr<VersionedCacheableObjectPartList>& versionedObjPartList,
    std::chrono::milliseconds timeout,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  LOGDEBUG(" ThinClientRegion::singleHopPutAllNoThrow_remote map size = %zu",
           map.size());
  auto region = shared_from_this();

  auto error = GF_NOERR;
  /*Step-1::
   * populate the keys vector from the user Map and pass it to the
   * getServerToFilterMap to generate locationMap
   * If locationMap is nullptr try the old, existing putAll impl that may take
   * multiple n/w hops
   */
  auto userKeys = std::vector<std::shared_ptr<CacheableKey>>();
  for (const auto& iter : map) {
    userKeys.push_back(iter.first);
  }
  // last param in getServerToFilterMap() is false for putAll

  // LOGDEBUG("ThinClientRegion::singleHopPutAllNoThrow_remote keys.size() = %d
  // ", userKeys->size());
  auto locationMap = tcrdm->getClientMetaDataService()->getServerToFilterMap(
      userKeys, region, true);
  if (!locationMap) {
    // putAll with multiple hop implementation
    LOGDEBUG("locationMap is Null or Empty");

    return multiHopPutAllNoThrow_remote(map, versionedObjPartList, timeout,
                                        aCallbackArgument);
  }

  // set this flag that indicates putAll on PR is invoked with singlehop
  // enabled.
  m_isPRSingleHopEnabled = true;
  // LOGDEBUG("locationMap.size() = %d ", locationMap->size());

  /*Step-2
   *  a. create vector of PutAllWork
   *  b. locationMap<std::shared_ptr<BucketServerLocation>,
   * std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>> >>. Create
   * server specific filteredMap/subMap by populating all keys
   * (locationIter.second()) and its corr. values from the user Map.
   *  c. create new instance of PutAllWork, i.e worker with required params.
   *     //TODO:: Add details of each parameter later
   *  d. enqueue the worker for thread from threadPool to perform/run execute
   * method.
   *  e. insert the worker into the vector.
   */
  std::vector<std::shared_ptr<PutAllWork>> putAllWorkers;
  auto& threadPool = m_cacheImpl->getThreadPool();
  int locationMapIndex = 0;
  for (const auto& locationIter : *locationMap) {
    const auto& serverLocation = locationIter.first;
    if (serverLocation == nullptr) {
      LOGDEBUG("serverLocation is nullptr");
    }
    const auto& keys = locationIter.second;

    // Create server specific Sub-Map by iterating over keys.
    auto filteredMap = std::make_shared<HashMapOfCacheable>();
    if (keys != nullptr && keys->size() > 0) {
      for (const auto& key : *keys) {
        const auto& iter = map.find(key);
        if (iter != map.end()) {
          filteredMap->emplace(iter->first, iter->second);
        }
      }
    }

    auto worker = std::make_shared<PutAllWork>(
        tcrdm, serverLocation, region, true /*attemptFailover*/,
        false /*isBGThread*/, filteredMap, keys, timeout, aCallbackArgument);
    threadPool.perform(worker);
    putAllWorkers.push_back(worker);
    locationMapIndex++;
  }

  // TODO::CHECK, do we need to set following ..??
  // reply.setMessageType(TcrMessage::RESPONSE);

  int cnt = 1;

  /**
   * Step::3
   * a. Iterate over all vector of putAllWorkers and populate worker specific
   * information into the HashMap
   *    resultMap<std::shared_ptr<BucketServerLocation>,
   * std::shared_ptr<Serializable>>, 2nd part, Value can be a
   * std::shared_ptr<VersionedCacheableObjectPartList> or
   * std::shared_ptr<PutAllPartialResultServerException>.
   *    failedServers<std::shared_ptr<BucketServerLocation>,
   * std::shared_ptr<CacheableInt32>>, 2nd part, Value is a ErrorCode. b. delete
   * the worker
   */
  auto resultMap = ResultMap();
  auto failedServers = FailedServersMap();

  for (const auto& worker : putAllWorkers) {
    auto err =
        worker->getResult();  // wait() or blocking call for worker thread.
    LOGDEBUG("Error code :: %s:%d err = %d ", __FILE__, __LINE__, err);

    if (GF_NOERR == err) {
      // No Exception from server
      resultMap.emplace(worker->getServerLocation(),
                        worker->getResultCollector()->getList());
    } else {
      error = err;

      if (error == GF_PUTALL_PARTIAL_RESULT_EXCEPTION) {
        resultMap.emplace(worker->getServerLocation(),
                          worker->getPaPResultException());
      } else if (error == GF_NOTCON) {
        // Refresh the metadata in case of GF_NOTCON.
        tcrdm->getClientMetaDataService()->enqueueForMetadataRefresh(
            region->getFullPath(), 0);
      }
      failedServers.emplace(worker->getServerLocation(),
                            CacheableInt32::create(error));
    }

    LOGDEBUG("worker->getPutAllMap()->size() = %zu ",
             worker->getPutAllMap()->size());
    LOGDEBUG(
        "worker->getResultCollector()->getList()->getVersionedTagsize() = %d ",
        worker->getResultCollector()->getList()->getVersionedTagsize());

    // TODO::CHECK, why do we need following code... ??
    // TcrMessage* currentReply = worker->getReply();
    /*
    if(currentReply->getMessageType() != TcrMessage::REPLY)
    {
      reply.setMessageType(currentReply->getMessageType());
    }
    */

    cnt++;
  }
  /**
   * Step:4
   * a. create instance of std::shared_ptr<PutAllPartialResult> with total size=
   * map.size() b. Iterate over the resultMap and value for the particular
   * serverlocation is of type VersionedCacheableObjectPartList add keys and
   * versions. C. ToDO:: what if the value in the resultMap is of type
   * PutAllPartialResultServerException
   */
  std::recursive_mutex responseLock;
  auto result = std::make_shared<PutAllPartialResult>(
      static_cast<int>(map.size()), responseLock);
  LOGDEBUG(
      " TCRegion:: %s:%d  "
      "result->getSucceededKeysAndVersions()->getVersionedTagsize() = %d ",
      __FILE__, __LINE__,
      result->getSucceededKeysAndVersions()->getVersionedTagsize());
  LOGDEBUG(" TCRegion:: %s:%d resultMap->size() ", __FILE__, __LINE__,
           resultMap.size());
  for (const auto& resultMapIter : resultMap) {
    const auto& value = resultMapIter.second;

    if (const auto papException =
            std::dynamic_pointer_cast<PutAllPartialResultServerException>(
                value)) {
      // PutAllPartialResultServerException CASE:: value in resultMap is of type
      // PutAllPartialResultServerException.
      // TODO:: Add failedservers.keySet= all fialed servers, i.e list out all
      // keys in map failedServers,
      //       that is set view of the keys contained in failedservers map.
      // TODO:: need to read  papException and populate PutAllPartialResult.
      result->consolidate(papException->getResult());
    } else if (const auto list =
                   std::dynamic_pointer_cast<VersionedCacheableObjectPartList>(
                       value)) {
      // value in resultMap is of type VCOPL.
      result->addKeysAndVersions(list);
    } else {
      // ERROR CASE
      if (value) {
        LOGERROR(
            "ERROR:: ThinClientRegion::singleHopPutAllNoThrow_remote value "
            "could not Cast to either VCOPL or "
            "PutAllPartialResultServerException:%s",
            value->toString().c_str());
      } else {
        LOGERROR(
            "ERROR:: ThinClientRegion::singleHopPutAllNoThrow_remote value is "
            "nullptr");
      }
    }
  }

  /**
   * a. if PutAllPartialResult result does not contains any entry,  Iterate over
   * locationMap.
   * b. Create std::vector<std::shared_ptr<CacheableKey>>  succeedKeySet, and
   * keep adding set of keys (locationIter.second()) in locationMap for which
   * failedServers->contains(locationIter.first()is false.
   */

  LOGDEBUG("ThinClientRegion:: %s:%d failedServers->size() = %zu", __FILE__,
           __LINE__, failedServers.size());

  // if the partial result set doesn't already have keys (for tracking version
  // tags)
  // then we need to gather up the keys that we know have succeeded so far and
  // add them to the partial result set (See bug Id #955)
  if (!failedServers.empty()) {
    auto succeedKeySet =
        std::make_shared<std::vector<std::shared_ptr<CacheableKey>>>();
    if (result->getSucceededKeysAndVersions()->size() == 0) {
      for (const auto& locationIter : *locationMap) {
        if (failedServers.find(locationIter.first) != failedServers.end()) {
          for (const auto& i : *(locationIter.second)) {
            succeedKeySet->push_back(i);
          }
        }
      }
      result->addKeys(succeedKeySet);
    }
  }

  /**
   * a. Iterate over the failedServers map
   * c. if failedServer map contains "GF_PUTALL_PARTIAL_RESULT_EXCEPTION" then
   * continue, Do not retry putAll for corr. keys.
   * b. Retry for all the failed server.
   *    Generate a newSubMap by finding Keys specific to failedServers from
   * locationMap and finding their respective values from the usermap.
   */
  error = GF_NOERR;
  bool oneSubMapRetryFailed = false;
  for (const auto& failedServerIter : failedServers) {
    if (failedServerIter.second->value() ==
        GF_PUTALL_PARTIAL_RESULT_EXCEPTION) {  // serverLocation
      // will not retry for PutAllPartialResultException
      // but it means at least one sub map ever failed
      oneSubMapRetryFailed = true;
      error = GF_PUTALL_PARTIAL_RESULT_EXCEPTION;
      continue;
    }

    std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> failedKeys =
        nullptr;
    const auto& failedSerInLocMapIter =
        locationMap->find(failedServerIter.first);
    if (failedSerInLocMapIter != locationMap->end()) {
      failedKeys = failedSerInLocMapIter->second;
    }

    if (failedKeys == nullptr) {
      LOGERROR(
          "TCRegion::singleHopPutAllNoThrow_remote :: failedKeys are nullptr "
          "that is not valid");
    }

    auto newSubMap = std::make_shared<HashMapOfCacheable>();
    if (failedKeys && !failedKeys->empty()) {
      for (const auto& key : *failedKeys) {
        const auto& iter = map.find(key);
        if (iter != map.end()) {
          newSubMap->emplace(iter->first, iter->second);
        } else {
          LOGERROR(
              "DEBUG:: TCRegion.cpp singleHopPutAllNoThrow_remote KEY not "
              "found in user failedSubMap");
        }
      }
    }

    std::shared_ptr<VersionedCacheableObjectPartList> vcopListPtr;
    GfErrType errCode = multiHopPutAllNoThrow_remote(
        *newSubMap, vcopListPtr, timeout, aCallbackArgument);
    if (errCode == GF_NOERR) {
      result->addKeysAndVersions(vcopListPtr);
    } else if (errCode == GF_PUTALL_PARTIAL_RESULT_EXCEPTION) {
      oneSubMapRetryFailed = true;
      // TODO:: Commented it as papResultServerExc is nullptr this time
      //       UnComment it once you read papResultServerExc.
      // result->consolidate(papResultServerExc->getResult());
      error = errCode;
    } else /*if(errCode != GF_NOERR)*/ {
      oneSubMapRetryFailed = true;
      const auto& firstKey = newSubMap->begin()->first;
      std::shared_ptr<Exception> excptPtr = nullptr;
      // TODO:: formulat excptPtr from the errCode
      result->saveFailedKey(firstKey, excptPtr);
      error = errCode;
    }
  }

  if (!oneSubMapRetryFailed) {
    error = GF_NOERR;
  }
  versionedObjPartList = result->getSucceededKeysAndVersions();
  LOGDEBUG("singlehop versionedObjPartList = %d error=%d",
           versionedObjPartList->size(), error);

  return error;
}

GfErrType ThinClientRegion::multiHopPutAllNoThrow_remote(
    const HashMapOfCacheable& map,
    std::shared_ptr<VersionedCacheableObjectPartList>& versionedObjPartList,
    std::chrono::milliseconds timeout,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  // Multiple hop implementation
  LOGDEBUG("ThinClientRegion::multiHopPutAllNoThrow_remote ");
  auto err = GF_NOERR;

  // Construct request/reply for putAll
  TcrMessagePutAll request(new DataOutput(m_cacheImpl->createDataOutput()),
                           this, map, timeout, m_tcrdm.get(),
                           aCallbackArgument);
  TcrMessageReply reply(true, m_tcrdm.get());
  request.setTimeout(timeout);
  reply.setTimeout(timeout);

  std::recursive_mutex responseLock;
  versionedObjPartList =
      std::make_shared<VersionedCacheableObjectPartList>(this, responseLock);
  // need to check
  ChunkedPutAllResponse* resultCollector(new ChunkedPutAllResponse(
      shared_from_this(), reply, responseLock, versionedObjPartList));
  reply.setChunkedResultHandler(resultCollector);

  err = m_tcrdm->sendSyncRequest(request, reply);

  versionedObjPartList = resultCollector->getList();
  LOGDEBUG("multiple hop versionedObjPartList size = %d , err = %d  ",
           versionedObjPartList->size(), err);
  delete resultCollector;
  if (err != GF_NOERR) return err;
  LOGDEBUG(
      "ThinClientRegion::multiHopPutAllNoThrow_remote reply.getMessageType() = "
      "%d ",
      reply.getMessageType());
  switch (reply.getMessageType()) {
    case TcrMessage::REPLY:
      // LOGDEBUG("Map is written into remote server at region %s",
      // m_fullPath.c_str());
      break;
    case TcrMessage::RESPONSE:
      LOGDEBUG(
          "multiHopPutAllNoThrow_remote TcrMessage::RESPONSE %s, err = %d ",
          m_fullPath.c_str(), err);
      break;
    case TcrMessage::EXCEPTION:
      err = handleServerException("ThinClientRegion::putAllNoThrow",
                                  reply.getException());
      // TODO:: Do we need to read PutAllPartialServerException for multiple
      // hop.
      break;
    case TcrMessage::PUT_DATA_ERROR:
      // LOGERROR( "A write error occurred on the endpoint %s",
      // m_tcrdm->getActiveEndpoint( )->name( ).c_str( ) );
      err = GF_CACHESERVER_EXCEPTION;
      break;
    default:
      LOGERROR("Unknown message type %d during region put-all",
               reply.getMessageType());
      err = GF_NOTOBJ;
      break;
  }
  return err;
}

GfErrType ThinClientRegion::putAllNoThrow_remote(
    const HashMapOfCacheable& map,
    std::shared_ptr<VersionedCacheableObjectPartList>& versionedObjPartList,
    std::chrono::milliseconds timeout,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  LOGDEBUG("ThinClientRegion::putAllNoThrow_remote");

  if (auto poolDM = std::dynamic_pointer_cast<ThinClientPoolDM>(m_tcrdm)) {
    if (poolDM->getPRSingleHopEnabled() && poolDM->getClientMetaDataService() &&
        !TSSTXStateWrapper::get().getTXState()) {
      return singleHopPutAllNoThrow_remote(
          poolDM.get(), map, versionedObjPartList, timeout, aCallbackArgument);
    } else {
      return multiHopPutAllNoThrow_remote(map, versionedObjPartList, timeout,
                                          aCallbackArgument);
    }
  } else {
    LOGERROR("ThinClientRegion::putAllNoThrow_remote :: Pool Not Specified ");
    return GF_NOTSUP;
  }
}

GfErrType ThinClientRegion::singleHopRemoveAllNoThrow_remote(
    ThinClientPoolDM* tcrdm,
    const std::vector<std::shared_ptr<CacheableKey>>& keys,
    std::shared_ptr<VersionedCacheableObjectPartList>& versionedObjPartList,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  LOGDEBUG(
      " ThinClientRegion::singleHopRemoveAllNoThrow_remote keys size = %zu",
      keys.size());
  auto region = shared_from_this();
  GfErrType error = GF_NOERR;

  auto locationMap = tcrdm->getClientMetaDataService()->getServerToFilterMap(
      keys, region, true);
  if (!locationMap) {
    // removeAll with multiple hop implementation
    LOGDEBUG("locationMap is Null or Empty");
    return multiHopRemoveAllNoThrow_remote(keys, versionedObjPartList,
                                           aCallbackArgument);
  }

  // set this flag that indicates putAll on PR is invoked with singlehop
  // enabled.
  m_isPRSingleHopEnabled = true;
  LOGDEBUG("locationMap.size() = %zu ", locationMap->size());

  /*Step-2
   *  a. create vector of RemoveAllWork
   *  b. locationMap<std::shared_ptr<BucketServerLocation>,
   * std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>> >>. Create
   * server specific filteredMap/subMap by populating all keys
   * (locationIter.second()) and its corr. values from the user Map.
   *  c. create new instance of RemoveAllWork, i.e worker with required params.
   *     //TODO:: Add details of each parameter later
   *  d. enqueue the worker for thread from threadPool to perform/run execute
   * method.
   *  e. insert the worker into the vector.
   */
  std::vector<std::shared_ptr<RemoveAllWork>> removeAllWorkers;
  auto& threadPool = m_cacheImpl->getThreadPool();
  int locationMapIndex = 0;
  for (const auto& locationIter : *locationMap) {
    const auto& serverLocation = locationIter.first;
    if (serverLocation == nullptr) {
      LOGDEBUG("serverLocation is nullptr");
    }
    const auto& mappedkeys = locationIter.second;
    auto worker = std::make_shared<RemoveAllWork>(
        tcrdm, serverLocation, region, true /*attemptFailover*/,
        false /*isBGThread*/, mappedkeys, aCallbackArgument);
    threadPool.perform(worker);
    removeAllWorkers.push_back(worker);
    locationMapIndex++;
  }
  // TODO::CHECK, do we need to set following ..??
  // reply.setMessageType(TcrMessage::RESPONSE);

  int cnt = 1;

  /**
   * Step::3
   * a. Iterate over all vector of putAllWorkers and populate worker specific
   * information into the HashMap
   *    resultMap<std::shared_ptr<BucketServerLocation>,
   * std::shared_ptr<Serializable>>, 2nd part, Value can be a
   * std::shared_ptr<VersionedCacheableObjectPartList> or
   * std::shared_ptr<PutAllPartialResultServerException>.
   *    failedServers<std::shared_ptr<BucketServerLocation>,
   * std::shared_ptr<CacheableInt32>>, 2nd part, Value is a ErrorCode. b. delete
   * the worker
   */
  auto resultMap = ResultMap();
  auto failedServers = FailedServersMap();
  for (const auto& worker : removeAllWorkers) {
    auto err =
        worker->getResult();  // wait() or blocking call for worker thread.
    LOGDEBUG("Error code :: %s:%d err = %d ", __FILE__, __LINE__, err);

    if (GF_NOERR == err) {
      // No Exception from server
      resultMap.emplace(worker->getServerLocation(),
                        worker->getResultCollector()->getList());
    } else {
      error = err;

      if (error == GF_PUTALL_PARTIAL_RESULT_EXCEPTION) {
        resultMap.emplace(worker->getServerLocation(),
                          worker->getPaPResultException());
      } else if (error == GF_NOTCON) {
        // Refresh the metadata in case of GF_NOTCON.
        tcrdm->getClientMetaDataService()->enqueueForMetadataRefresh(
            region->getFullPath(), 0);
      }
      failedServers.emplace(worker->getServerLocation(),
                            CacheableInt32::create(error));
    }

    LOGDEBUG(
        "worker->getResultCollector()->getList()->getVersionedTagsize() = %d ",
        worker->getResultCollector()->getList()->getVersionedTagsize());

    cnt++;
  }
  /**
   * Step:4
   * a. create instance of std::shared_ptr<PutAllPartialResult> with total size=
   * map.size() b. Iterate over the resultMap and value for the particular
   * serverlocation is of type VersionedCacheableObjectPartList add keys and
   * versions. C. ToDO:: what if the value in the resultMap is of type
   * PutAllPartialResultServerException
   */
  std::recursive_mutex responseLock;
  auto result = std::make_shared<PutAllPartialResult>(
      static_cast<int>(keys.size()), responseLock);
  LOGDEBUG(
      " TCRegion:: %s:%d  "
      "result->getSucceededKeysAndVersions()->getVersionedTagsize() = %d ",
      __FILE__, __LINE__,
      result->getSucceededKeysAndVersions()->getVersionedTagsize());
  LOGDEBUG(" TCRegion:: %s:%d resultMap->size() ", __FILE__, __LINE__,
           resultMap.size());
  for (const auto& resultMapIter : resultMap) {
    const auto& value = resultMapIter.second;

    if (const auto papException =
            std::dynamic_pointer_cast<PutAllPartialResultServerException>(
                value)) {
      // PutAllPartialResultServerException CASE:: value in resultMap is of type
      // PutAllPartialResultServerException.
      // TODO:: Add failedservers.keySet= all fialed servers, i.e list out all
      // keys in map failedServers,
      //       that is set view of the keys contained in failedservers map.
      // TODO:: need to read  papException and populate PutAllPartialResult.
      result->consolidate(papException->getResult());
    } else if (const auto list =
                   std::dynamic_pointer_cast<VersionedCacheableObjectPartList>(
                       value)) {
      // value in resultMap is of type VCOPL.
      result->addKeysAndVersions(list);
    } else {
      // ERROR CASE
      if (value) {
        LOGERROR(
            "ERROR:: ThinClientRegion::singleHopRemoveAllNoThrow_remote value "
            "could not Cast to either VCOPL or "
            "PutAllPartialResultServerException:%s",
            value->toString().c_str());
      } else {
        LOGERROR(
            "ERROR:: ThinClientRegion::singleHopRemoveAllNoThrow_remote value "
            "is nullptr");
      }
    }
  }

  /**
   * a. if PutAllPartialResult result does not contains any entry,  Iterate over
   * locationMap.
   * b. Create std::vector<std::shared_ptr<CacheableKey>>  succeedKeySet, and
   * keep adding set of keys (locationIter.second()) in locationMap for which
   * failedServers->contains(locationIter.first()is false.
   */

  LOGDEBUG("ThinClientRegion:: %s:%d failedServers->size() = %zu", __FILE__,
           __LINE__, failedServers.size());

  // if the partial result set doesn't already have keys (for tracking version
  // tags)
  // then we need to gather up the keys that we know have succeeded so far and
  // add them to the partial result set (See bug Id #955)
  if (!failedServers.empty()) {
    auto succeedKeySet =
        std::make_shared<std::vector<std::shared_ptr<CacheableKey>>>();
    if (result->getSucceededKeysAndVersions()->size() == 0) {
      for (const auto& locationIter : *locationMap) {
        if (failedServers.find(locationIter.first) != failedServers.end()) {
          for (const auto& i : *(locationIter.second)) {
            succeedKeySet->push_back(i);
          }
        }
      }
      result->addKeys(succeedKeySet);
    }
  }

  /**
   * a. Iterate over the failedServers map
   * c. if failedServer map contains "GF_PUTALL_PARTIAL_RESULT_EXCEPTION" then
   * continue, Do not retry putAll for corr. keys.
   * b. Retry for all the failed server.
   *    Generate a newSubMap by finding Keys specific to failedServers from
   * locationMap and finding their respective values from the usermap.
   */
  error = GF_NOERR;
  bool oneSubMapRetryFailed = false;
  for (const auto& failedServerIter : failedServers) {
    if (failedServerIter.second->value() ==
        GF_PUTALL_PARTIAL_RESULT_EXCEPTION) {  // serverLocation
      // will not retry for PutAllPartialResultException
      // but it means at least one sub map ever failed
      oneSubMapRetryFailed = true;
      error = GF_PUTALL_PARTIAL_RESULT_EXCEPTION;
      continue;
    }

    std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> failedKeys =
        nullptr;
    const auto& failedSerInLocMapIter =
        locationMap->find(failedServerIter.first);
    if (failedSerInLocMapIter != locationMap->end()) {
      failedKeys = failedSerInLocMapIter->second;
    }

    if (failedKeys == nullptr) {
      LOGERROR(
          "TCRegion::singleHopRemoveAllNoThrow_remote :: failedKeys are "
          "nullptr "
          "that is not valid");
    }

    std::shared_ptr<VersionedCacheableObjectPartList> vcopListPtr;
    std::shared_ptr<PutAllPartialResultServerException> papResultServerExc =
        nullptr;
    GfErrType errCode = multiHopRemoveAllNoThrow_remote(
        *failedKeys, vcopListPtr, aCallbackArgument);
    if (errCode == GF_NOERR) {
      result->addKeysAndVersions(vcopListPtr);
    } else if (errCode == GF_PUTALL_PARTIAL_RESULT_EXCEPTION) {
      oneSubMapRetryFailed = true;
      error = errCode;
    } else /*if(errCode != GF_NOERR)*/ {
      oneSubMapRetryFailed = true;
      std::shared_ptr<Exception> excptPtr = nullptr;
      result->saveFailedKey(failedKeys->at(0), excptPtr);
      error = errCode;
    }
  }

  if (!oneSubMapRetryFailed) {
    error = GF_NOERR;
  }
  versionedObjPartList = result->getSucceededKeysAndVersions();
  LOGDEBUG("singlehop versionedObjPartList = %d error=%d",
           versionedObjPartList->size(), error);
  return error;
}

GfErrType ThinClientRegion::multiHopRemoveAllNoThrow_remote(
    const std::vector<std::shared_ptr<CacheableKey>>& keys,
    std::shared_ptr<VersionedCacheableObjectPartList>& versionedObjPartList,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  // Multiple hop implementation
  LOGDEBUG("ThinClientRegion::multiHopRemoveAllNoThrow_remote ");
  GfErrType err = GF_NOERR;

  // Construct request/reply for putAll
  TcrMessageRemoveAll request(new DataOutput(m_cacheImpl->createDataOutput()),
                              this, keys, aCallbackArgument, m_tcrdm.get());
  TcrMessageReply reply(true, m_tcrdm.get());

  std::recursive_mutex responseLock;
  versionedObjPartList =
      std::make_shared<VersionedCacheableObjectPartList>(this, responseLock);
  // need to check
  ChunkedRemoveAllResponse* resultCollector(new ChunkedRemoveAllResponse(
      shared_from_this(), reply, responseLock, versionedObjPartList));
  reply.setChunkedResultHandler(resultCollector);

  err = m_tcrdm->sendSyncRequest(request, reply);

  versionedObjPartList = resultCollector->getList();
  LOGDEBUG("multiple hop versionedObjPartList size = %d , err = %d  ",
           versionedObjPartList->size(), err);
  delete resultCollector;
  if (err != GF_NOERR) return err;
  LOGDEBUG(
      "ThinClientRegion::multiHopRemoveAllNoThrow_remote "
      "reply.getMessageType() = %d ",
      reply.getMessageType());
  switch (reply.getMessageType()) {
    case TcrMessage::REPLY:
      // LOGDEBUG("Map is written into remote server at region %s",
      // m_fullPath.c_str());
      break;
    case TcrMessage::RESPONSE:
      LOGDEBUG(
          "multiHopRemoveAllNoThrow_remote TcrMessage::RESPONSE %s, err = %d ",
          m_fullPath.c_str(), err);
      break;
    case TcrMessage::EXCEPTION:
      err = handleServerException("ThinClientRegion::putAllNoThrow",
                                  reply.getException());
      break;
    default:
      LOGERROR("Unknown message type %d during region remove-all",
               reply.getMessageType());
      err = GF_NOTOBJ;
      break;
  }
  return err;
}

GfErrType ThinClientRegion::removeAllNoThrow_remote(
    const std::vector<std::shared_ptr<CacheableKey>>& keys,
    std::shared_ptr<VersionedCacheableObjectPartList>& versionedObjPartList,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  LOGDEBUG("ThinClientRegion::removeAllNoThrow_remote");

  if (auto poolDM = std::dynamic_pointer_cast<ThinClientPoolDM>(m_tcrdm)) {
    if (poolDM->getPRSingleHopEnabled() && poolDM->getClientMetaDataService() &&
        !TSSTXStateWrapper::get().getTXState()) {
      return singleHopRemoveAllNoThrow_remote(
          poolDM.get(), keys, versionedObjPartList, aCallbackArgument);
    } else {
      return multiHopRemoveAllNoThrow_remote(keys, versionedObjPartList,
                                             aCallbackArgument);
    }
  } else {
    LOGERROR(
        "ThinClientRegion::removeAllNoThrow_remote :: Pool Not Specified ");
    return GF_NOTSUP;
  }
}

uint32_t ThinClientRegion::size_remote() {
  LOGDEBUG("ThinClientRegion::size_remote");
  GfErrType err = GF_NOERR;

  // do TCR size
  TcrMessageSize request(new DataOutput(m_cacheImpl->createDataOutput()),
                         m_fullPath.c_str());
  TcrMessageReply reply(true, m_tcrdm.get());
  err = m_tcrdm->sendSyncRequest(request, reply);

  if (err != GF_NOERR) {
    throwExceptionIfError("Region::size", err);
  }

  switch (reply.getMessageType()) {
    case TcrMessage::RESPONSE: {
      auto size = std::dynamic_pointer_cast<CacheableInt32>(reply.getValue());
      return size->value();
    }
    case TcrMessage::EXCEPTION:
      err =
          handleServerException("ThinClientRegion::size", reply.getException());
      break;
    case TcrMessage::SIZE_ERROR:
      err = GF_CACHESERVER_EXCEPTION;
      break;
    default:
      LOGERROR("Unknown message type %d during region size",
               reply.getMessageType());
      err = GF_NOTOBJ;
  }

  throwExceptionIfError("Region::size", err);
  return 0;
}

GfErrType ThinClientRegion::registerStoredRegex(
    TcrEndpoint* endpoint,
    std::unordered_map<std::string, InterestResultPolicy>& interestListRegex,
    bool isDurable, bool receiveValues) {
  GfErrType opErr = GF_NOERR;
  GfErrType retVal = GF_NOERR;

  for (std::unordered_map<std::string, InterestResultPolicy>::iterator it =
           interestListRegex.begin();
       it != interestListRegex.end(); ++it) {
    opErr = registerRegexNoThrow(it->first, false, endpoint, isDurable, nullptr,
                                 it->second, receiveValues);
    if (opErr != GF_NOERR) {
      retVal = opErr;
    }
  }

  return retVal;
}

GfErrType ThinClientRegion::registerKeys(TcrEndpoint* endpoint,
                                         const TcrMessage* request,
                                         TcrMessageReply* reply) {
  GfErrType err = GF_NOERR;
  GfErrType opErr = GF_NOERR;

  // called when failover to a different server
  opErr = registerStoredRegex(endpoint, m_interestListRegex);
  err = opErr != GF_NOERR ? opErr : err;
  opErr = registerStoredRegex(
      endpoint, m_interestListRegexForUpdatesAsInvalidates, false, false);
  err = opErr != GF_NOERR ? opErr : err;
  opErr = registerStoredRegex(endpoint, m_durableInterestListRegex, true);
  err = opErr != GF_NOERR ? opErr : err;
  opErr = registerStoredRegex(
      endpoint, m_durableInterestListRegexForUpdatesAsInvalidates, true, false);
  err = opErr != GF_NOERR ? opErr : err;

  std::vector<std::shared_ptr<CacheableKey>> keysVec;
  InterestResultPolicy interestPolicy =
      copyInterestList(keysVec, m_interestList);
  opErr = registerKeysNoThrow(keysVec, false, endpoint, false, interestPolicy);
  err = opErr != GF_NOERR ? opErr : err;

  std::vector<std::shared_ptr<CacheableKey>> keysVecForUpdatesAsInvalidates;
  interestPolicy = copyInterestList(keysVecForUpdatesAsInvalidates,
                                    m_interestListForUpdatesAsInvalidates);
  opErr = registerKeysNoThrow(keysVecForUpdatesAsInvalidates, false, endpoint,
                              false, interestPolicy, false);
  err = opErr != GF_NOERR ? opErr : err;

  std::vector<std::shared_ptr<CacheableKey>> keysVecDurable;
  interestPolicy = copyInterestList(keysVecDurable, m_durableInterestList);
  opErr = registerKeysNoThrow(keysVecDurable, false, endpoint, true,
                              interestPolicy);
  err = opErr != GF_NOERR ? opErr : err;

  std::vector<std::shared_ptr<CacheableKey>>
      keysVecDurableForUpdatesAsInvalidates;
  interestPolicy =
      copyInterestList(keysVecDurableForUpdatesAsInvalidates,
                       m_durableInterestListForUpdatesAsInvalidates);
  opErr = registerKeysNoThrow(keysVecDurableForUpdatesAsInvalidates, false,
                              endpoint, true, interestPolicy, false);
  err = opErr != GF_NOERR ? opErr : err;

  if (request != nullptr && request->getRegionName() == m_fullPath &&
      (request->getMessageType() == TcrMessage::REGISTER_INTEREST ||
       request->getMessageType() == TcrMessage::REGISTER_INTEREST_LIST)) {
    const std::vector<std::shared_ptr<CacheableKey>>* newKeysVec =
        request->getKeys();
    bool isDurable = request->isDurable();
    bool receiveValues = request->receiveValues();
    if (newKeysVec == nullptr || newKeysVec->empty()) {
      const std::string& newRegex = request->getRegex();
      if (!newRegex.empty()) {
        if (request->getRegionName() != m_fullPath) {
          reply = nullptr;
        }
        opErr = registerRegexNoThrow(
            newRegex, false, endpoint, isDurable, nullptr,
            request->getInterestResultPolicy(), receiveValues, reply);
        err = opErr != GF_NOERR ? opErr : err;
      }
    } else {
      opErr = registerKeysNoThrow(*newKeysVec, false, endpoint, isDurable,
                                  request->getInterestResultPolicy(),
                                  receiveValues, reply);
      err = opErr != GF_NOERR ? opErr : err;
    }
  }
  return err;
}

GfErrType ThinClientRegion::unregisterStoredRegex(
    std::unordered_map<std::string, InterestResultPolicy>& interestListRegex) {
  GfErrType opErr = GF_NOERR;
  GfErrType retVal = GF_NOERR;

  for (std::unordered_map<std::string, InterestResultPolicy>::iterator it =
           interestListRegex.begin();
       it != interestListRegex.end(); ++it) {
    opErr = unregisterRegexNoThrow(it->first, false);
    if (opErr != GF_NOERR) {
      retVal = opErr;
    }
  }

  return retVal;
}

GfErrType ThinClientRegion::unregisterStoredRegexLocalDestroy(
    std::unordered_map<std::string, InterestResultPolicy>& interestListRegex) {
  GfErrType opErr = GF_NOERR;
  GfErrType retVal = GF_NOERR;

  for (std::unordered_map<std::string, InterestResultPolicy>::iterator it =
           interestListRegex.begin();
       it != interestListRegex.end(); ++it) {
    opErr = unregisterRegexNoThrowLocalDestroy(it->first, false);
    if (opErr != GF_NOERR) {
      retVal = opErr;
    }
  }
  return retVal;
}

GfErrType ThinClientRegion::unregisterKeys() {
  GfErrType err = GF_NOERR;
  GfErrType opErr = GF_NOERR;

  // called when disconnect from a server
  opErr = unregisterStoredRegex(m_interestListRegex);
  err = opErr != GF_NOERR ? opErr : err;
  opErr = unregisterStoredRegex(m_durableInterestListRegex);
  err = opErr != GF_NOERR ? opErr : err;
  opErr = unregisterStoredRegex(m_interestListRegexForUpdatesAsInvalidates);
  err = opErr != GF_NOERR ? opErr : err;
  opErr =
      unregisterStoredRegex(m_durableInterestListRegexForUpdatesAsInvalidates);
  err = opErr != GF_NOERR ? opErr : err;

  std::vector<std::shared_ptr<CacheableKey>> keysVec;
  copyInterestList(keysVec, m_interestList);
  opErr = unregisterKeysNoThrow(keysVec, false);
  err = opErr != GF_NOERR ? opErr : err;

  std::vector<std::shared_ptr<CacheableKey>> keysVecDurable;
  copyInterestList(keysVecDurable, m_durableInterestList);
  opErr = unregisterKeysNoThrow(keysVecDurable, false);
  err = opErr != GF_NOERR ? opErr : err;

  std::vector<std::shared_ptr<CacheableKey>> keysVecForUpdatesAsInvalidates;
  copyInterestList(keysVecForUpdatesAsInvalidates,
                   m_interestListForUpdatesAsInvalidates);
  opErr = unregisterKeysNoThrow(keysVecForUpdatesAsInvalidates, false);
  err = opErr != GF_NOERR ? opErr : err;

  std::vector<std::shared_ptr<CacheableKey>>
      keysVecDurableForUpdatesAsInvalidates;
  copyInterestList(keysVecDurableForUpdatesAsInvalidates,
                   m_durableInterestListForUpdatesAsInvalidates);
  opErr = unregisterKeysNoThrow(keysVecDurableForUpdatesAsInvalidates, false);
  err = opErr != GF_NOERR ? opErr : err;

  return err;
}

GfErrType ThinClientRegion::destroyRegionNoThrow_remote(
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  GfErrType err = GF_NOERR;

  // do TCR destroyRegion
  TcrMessageDestroyRegion request(
      new DataOutput(m_cacheImpl->createDataOutput()), this, aCallbackArgument,
      std::chrono::milliseconds(-1), m_tcrdm.get());
  TcrMessageReply reply(true, m_tcrdm.get());
  err = m_tcrdm->sendSyncRequest(request, reply);
  if (err != GF_NOERR) return err;

  switch (reply.getMessageType()) {
    case TcrMessage::REPLY: {
      // LOGINFO("Region %s at remote is destroyed successfully",
      // m_fullPath.c_str());
      break;
    }
    case TcrMessage::EXCEPTION: {
      err =
          handleServerException("Region::destroyRegion", reply.getException());
      break;
    }
    case TcrMessage::DESTROY_REGION_DATA_ERROR: {
      err = GF_CACHE_REGION_DESTROYED_EXCEPTION;
      break;
    }
    default: {
      LOGERROR("Unknown message type %d during destroy region",
               reply.getMessageType());
      err = GF_MSG;
    }
  }
  return err;
}

GfErrType ThinClientRegion::registerKeysNoThrow(
    const std::vector<std::shared_ptr<CacheableKey>>& keys,
    bool attemptFailover, TcrEndpoint* endpoint, bool isDurable,
    InterestResultPolicy interestPolicy, bool receiveValues,
    TcrMessageReply* reply) {
  RegionGlobalLocks acquireLocksRedundancy(this, false);
  RegionGlobalLocks acquireLocksFailover(this);
  CHECK_DESTROY_PENDING_NOTHROW(TryReadGuard);
  GfErrType err = GF_NOERR;

  std::lock_guard<decltype(m_keysLock)> keysGuard(m_keysLock);
  if (keys.empty()) {
    return err;
  }

  TcrMessageReply replyLocal(true, m_tcrdm.get());
  bool needToCreateRC = true;
  if (reply == nullptr) {
    reply = &replyLocal;
  } else {
    needToCreateRC = false;
  }

  LOGDEBUG("ThinClientRegion::registerKeysNoThrow : interestpolicy is %d",
           interestPolicy.ordinal);

  TcrMessageRegisterInterestList request(
      new DataOutput(m_cacheImpl->createDataOutput()), this, keys, isDurable,
      getAttributes().getCachingEnabled(), receiveValues, interestPolicy,
      m_tcrdm.get());
  std::recursive_mutex responseLock;
  TcrChunkedResult* resultCollector = nullptr;
  if (interestPolicy.ordinal == InterestResultPolicy::KEYS_VALUES.ordinal) {
    auto values = std::make_shared<HashMapOfCacheable>();
    auto exceptions = std::make_shared<HashMapOfException>();
    MapOfUpdateCounters trackers;
    int32_t destroyTracker = 1;
    if (needToCreateRC) {
      resultCollector = (new ChunkedGetAllResponse(
          request, this, &keys, values, exceptions, nullptr, trackers,
          destroyTracker, true, responseLock));
      reply->setChunkedResultHandler(resultCollector);
    }
  } else {
    if (needToCreateRC) {
      resultCollector = (new ChunkedInterestResponse(request, nullptr, *reply));
      reply->setChunkedResultHandler(resultCollector);
    }
  }

  err = m_tcrdm->sendSyncRequestRegisterInterest(
      request, *reply, attemptFailover, this, endpoint);

  if (err == GF_NOERR /*|| err == GF_CACHE_REDUNDANCY_FAILURE*/) {
    if (reply->getMessageType() == TcrMessage::RESPONSE_FROM_SECONDARY &&
        endpoint) {
      LOGFINER(
          "registerKeysNoThrow - got response from secondary for "
          "endpoint %s, ignoring.",
          endpoint->name().c_str());
    } else if (attemptFailover) {
      addKeys(keys, isDurable, receiveValues, interestPolicy);
      if (!(interestPolicy.ordinal ==
            InterestResultPolicy::KEYS_VALUES.ordinal)) {
        localInvalidateForRegisterInterest(keys);
      }
    }
  }
  if (needToCreateRC) {
    delete resultCollector;
  }
  return err;
}

GfErrType ThinClientRegion::unregisterKeysNoThrow(
    const std::vector<std::shared_ptr<CacheableKey>>& keys,
    bool attemptFailover) {
  RegionGlobalLocks acquireLocksRedundancy(this, false);
  RegionGlobalLocks acquireLocksFailover(this);
  CHECK_DESTROY_PENDING_NOTHROW(TryReadGuard);
  GfErrType err = GF_NOERR;
  std::lock_guard<decltype(m_keysLock)> keysGuard(m_keysLock);
  TcrMessageReply reply(true, m_tcrdm.get());
  if (keys.empty()) {
    return err;
  }

  if (m_interestList.empty() && m_durableInterestList.empty() &&
      m_interestListForUpdatesAsInvalidates.empty() &&
      m_durableInterestListForUpdatesAsInvalidates.empty()) {
    // did not register any keys before.
    return GF_CACHE_ILLEGAL_STATE_EXCEPTION;
  }

  TcrMessageUnregisterInterestList request(
      new DataOutput(m_cacheImpl->createDataOutput()), this, keys, false, true,
      InterestResultPolicy::NONE, m_tcrdm.get());
  err = m_tcrdm->sendSyncRequestRegisterInterest(request, reply);
  if (err == GF_NOERR /*|| err == GF_CACHE_REDUNDANCY_FAILURE*/) {
    if (attemptFailover) {
      for (const auto& key : keys) {
        m_interestList.erase(key);
        m_durableInterestList.erase(key);
        m_interestListForUpdatesAsInvalidates.erase(key);
        m_durableInterestListForUpdatesAsInvalidates.erase(key);
      }
    }
  }
  return err;
}

GfErrType ThinClientRegion::unregisterKeysNoThrowLocalDestroy(
    const std::vector<std::shared_ptr<CacheableKey>>& keys,
    bool attemptFailover) {
  RegionGlobalLocks acquireLocksRedundancy(this, false);
  RegionGlobalLocks acquireLocksFailover(this);
  GfErrType err = GF_NOERR;
  std::lock_guard<decltype(m_keysLock)> keysGuard(m_keysLock);
  TcrMessageReply reply(true, m_tcrdm.get());
  if (keys.empty()) {
    return err;
  }

  if (m_interestList.empty() && m_durableInterestList.empty() &&
      m_interestListForUpdatesAsInvalidates.empty() &&
      m_durableInterestListForUpdatesAsInvalidates.empty()) {
    // did not register any keys before.
    return GF_CACHE_ILLEGAL_STATE_EXCEPTION;
  }

  TcrMessageUnregisterInterestList request(
      new DataOutput(m_cacheImpl->createDataOutput()), this, keys, false, true,
      InterestResultPolicy::NONE, m_tcrdm.get());
  err = m_tcrdm->sendSyncRequestRegisterInterest(request, reply);
  if (err == GF_NOERR) {
    if (attemptFailover) {
      for (const auto& key : keys) {
        m_interestList.erase(key);
        m_durableInterestList.erase(key);
        m_interestListForUpdatesAsInvalidates.erase(key);
        m_durableInterestListForUpdatesAsInvalidates.erase(key);
      }
    }
  }
  return err;
}

bool ThinClientRegion::isRegexRegistered(
    std::unordered_map<std::string, InterestResultPolicy>& interestListRegex,
    const std::string& regex, bool allKeys) {
  if (interestListRegex.find(".*") != interestListRegex.end() ||
      (!allKeys && interestListRegex.find(regex) != interestListRegex.end())) {
    return true;
  }
  return false;
}

GfErrType ThinClientRegion::registerRegexNoThrow(
    const std::string& regex, bool attemptFailover, TcrEndpoint* endpoint,
    bool isDurable,
    std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> resultKeys,
    InterestResultPolicy interestPolicy, bool receiveValues,
    TcrMessageReply* reply) {
  RegionGlobalLocks acquireLocksRedundancy(this, false);
  RegionGlobalLocks acquireLocksFailover(this);
  CHECK_DESTROY_PENDING_NOTHROW(TryReadGuard);
  GfErrType err = GF_NOERR;

  bool allKeys = (regex == ".*");
  std::lock_guard<decltype(m_keysLock)> keysGuard(m_keysLock);

  if (attemptFailover) {
    if ((isDurable &&
         (isRegexRegistered(m_durableInterestListRegex, regex, allKeys) ||
          isRegexRegistered(m_durableInterestListRegexForUpdatesAsInvalidates,
                            regex, allKeys))) ||
        (!isDurable &&
         (isRegexRegistered(m_interestListRegex, regex, allKeys) ||
          isRegexRegistered(m_interestListRegexForUpdatesAsInvalidates, regex,
                            allKeys)))) {
      return err;
    }
  }

  ChunkedInterestResponse* resultCollector = nullptr;
  ChunkedGetAllResponse* getAllResultCollector = nullptr;
  if (reply != nullptr) {
    // need to check
    resultCollector = dynamic_cast<ChunkedInterestResponse*>(
        reply->getChunkedResultHandler());
    if (resultCollector != nullptr) {
      resultKeys = resultCollector->getResultKeys();
    } else {
      getAllResultCollector = dynamic_cast<ChunkedGetAllResponse*>(
          reply->getChunkedResultHandler());
      resultKeys = getAllResultCollector->getResultKeys();
    }
  }

  bool isRCCreatedLocally = false;
  LOGDEBUG("ThinClientRegion::registerRegexNoThrow : interestpolicy is %d",
           interestPolicy.ordinal);

  // TODO:
  TcrMessageRegisterInterest request(
      new DataOutput(m_cacheImpl->createDataOutput()), m_fullPath,
      regex.c_str(), interestPolicy, isDurable,
      getAttributes().getCachingEnabled(), receiveValues, m_tcrdm.get());
  std::recursive_mutex responseLock;
  if (reply == nullptr) {
    TcrMessageReply replyLocal(true, m_tcrdm.get());
    auto values = std::make_shared<HashMapOfCacheable>();
    auto exceptions = std::make_shared<HashMapOfException>();

    reply = &replyLocal;
    if (interestPolicy.ordinal == InterestResultPolicy::KEYS_VALUES.ordinal) {
      MapOfUpdateCounters trackers;
      int32_t destroyTracker = 1;
      if (resultKeys == nullptr) {
        resultKeys =
            std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>>(
                new std::vector<std::shared_ptr<CacheableKey>>());
      }
      // need to check
      getAllResultCollector = (new ChunkedGetAllResponse(
          request, this, nullptr, values, exceptions, resultKeys, trackers,
          destroyTracker, true, responseLock));
      reply->setChunkedResultHandler(getAllResultCollector);
      isRCCreatedLocally = true;
    } else {
      isRCCreatedLocally = true;
      // need to check
      resultCollector =
          new ChunkedInterestResponse(request, resultKeys, replyLocal);
      reply->setChunkedResultHandler(resultCollector);
    }
    err = m_tcrdm->sendSyncRequestRegisterInterest(
        request, replyLocal, attemptFailover, this, endpoint);
  } else {
    err = m_tcrdm->sendSyncRequestRegisterInterest(
        request, *reply, attemptFailover, this, endpoint);
  }
  if (err == GF_NOERR /*|| err == GF_CACHE_REDUNDANCY_FAILURE*/) {
    if (reply->getMessageType() == TcrMessage::RESPONSE_FROM_SECONDARY &&
        endpoint) {
      LOGFINER(
          "registerRegexNoThrow - got response from secondary for "
          "endpoint %s, ignoring.",
          endpoint->name().c_str());
    } else if (attemptFailover) {
      addRegex(regex, isDurable, receiveValues, interestPolicy);
      if (interestPolicy.ordinal != InterestResultPolicy::KEYS_VALUES.ordinal) {
        if (allKeys) {
          localInvalidateRegion_internal();
        } else {
          const std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>>&
              keys = resultCollector != nullptr
                         ? resultCollector->getResultKeys()
                         : getAllResultCollector->getResultKeys();
          if (keys != nullptr) {
            localInvalidateForRegisterInterest(*keys);
          }
        }
      }
    }
  }

  if (isRCCreatedLocally == true) {
    if (resultCollector != nullptr) delete resultCollector;
    if (getAllResultCollector != nullptr) delete getAllResultCollector;
  }
  return err;
}

GfErrType ThinClientRegion::unregisterRegexNoThrow(const std::string& regex,
                                                   bool attemptFailover) {
  RegionGlobalLocks acquireLocksRedundancy(this, false);
  RegionGlobalLocks acquireLocksFailover(this);
  CHECK_DESTROY_PENDING_NOTHROW(TryReadGuard);
  GfErrType err = GF_NOERR;

  err = findRegex(regex);

  if (err == GF_NOERR) {
    TcrMessageReply reply(false, m_tcrdm.get());
    TcrMessageUnregisterInterest request(
        new DataOutput(m_cacheImpl->createDataOutput()), m_fullPath, regex,
        InterestResultPolicy::NONE, false, true, m_tcrdm.get());
    err = m_tcrdm->sendSyncRequestRegisterInterest(request, reply);
    if (err == GF_NOERR /*|| err == GF_CACHE_REDUNDANCY_FAILURE*/) {
      if (attemptFailover) {
        clearRegex(regex);
      }
    }
  }
  return err;
}

GfErrType ThinClientRegion::findRegex(const std::string& regex) {
  GfErrType err = GF_NOERR;
  std::lock_guard<decltype(m_keysLock)> keysGuard(m_keysLock);

  if (m_interestListRegex.find(regex) == m_interestListRegex.end() &&
      m_durableInterestListRegex.find(regex) ==
          m_durableInterestListRegex.end() &&
      m_interestListRegexForUpdatesAsInvalidates.find(regex) ==
          m_interestListRegexForUpdatesAsInvalidates.end() &&
      m_durableInterestListRegexForUpdatesAsInvalidates.find(regex) ==
          m_durableInterestListRegexForUpdatesAsInvalidates.end()) {
    return GF_CACHE_ILLEGAL_STATE_EXCEPTION;
  } else {
    return err;
  }
}

void ThinClientRegion::clearRegex(const std::string& regex) {
  std::lock_guard<decltype(m_keysLock)> keysGuard(m_keysLock);
  m_interestListRegex.erase(regex);
  m_durableInterestListRegex.erase(regex);
  m_interestListRegexForUpdatesAsInvalidates.erase(regex);
  m_durableInterestListRegexForUpdatesAsInvalidates.erase(regex);
}

GfErrType ThinClientRegion::unregisterRegexNoThrowLocalDestroy(
    const std::string& regex, bool attemptFailover) {
  GfErrType err = GF_NOERR;

  err = findRegex(regex);

  if (err == GF_NOERR) {
    TcrMessageReply reply(false, m_tcrdm.get());
    TcrMessageUnregisterInterest request(
        new DataOutput(m_cacheImpl->createDataOutput()), m_fullPath, regex,
        InterestResultPolicy::NONE, false, true, m_tcrdm.get());
    err = m_tcrdm->sendSyncRequestRegisterInterest(request, reply);
    if (err == GF_NOERR) {
      if (attemptFailover) {
        clearRegex(regex);
      }
    }
  }
  return err;
}

void ThinClientRegion::addKeys(
    const std::vector<std::shared_ptr<CacheableKey>>& keys, bool isDurable,
    bool receiveValues, InterestResultPolicy interestpolicy) {
  std::unordered_map<std::shared_ptr<CacheableKey>, InterestResultPolicy>&
      interestList =
          isDurable
              ? (receiveValues ? m_durableInterestList
                               : m_durableInterestListForUpdatesAsInvalidates)
              : (receiveValues ? m_interestList
                               : m_interestListForUpdatesAsInvalidates);

  for (const auto& key : keys) {
    interestList.insert(
        std::pair<std::shared_ptr<CacheableKey>, InterestResultPolicy>(
            key, interestpolicy));
  }
}

void ThinClientRegion::addRegex(const std::string& regex, bool isDurable,
                                bool receiveValues,
                                InterestResultPolicy interestpolicy) {
  std::unordered_map<std::shared_ptr<CacheableKey>, InterestResultPolicy>&
      interestList =
          isDurable
              ? (receiveValues ? m_durableInterestList
                               : m_durableInterestListForUpdatesAsInvalidates)
              : (receiveValues ? m_interestList
                               : m_interestListForUpdatesAsInvalidates);

  std::unordered_map<std::string, InterestResultPolicy>& interestListRegex =
      isDurable
          ? (receiveValues ? m_durableInterestListRegex
                           : m_durableInterestListRegexForUpdatesAsInvalidates)
          : (receiveValues ? m_interestListRegex
                           : m_interestListRegexForUpdatesAsInvalidates);

  if (regex == ".*") {
    interestListRegex.clear();
    interestList.clear();
  }

  interestListRegex.insert(
      std::pair<std::string, InterestResultPolicy>(regex, interestpolicy));
}

std::vector<std::shared_ptr<CacheableKey>> ThinClientRegion::getInterestList()
    const {
  auto nthis = const_cast<ThinClientRegion*>(this);
  RegionGlobalLocks acquireLocksRedundancy(nthis, false);
  RegionGlobalLocks acquireLocksFailover(nthis);
  CHECK_DESTROY_PENDING(TryReadGuard, getInterestList);
  std::lock_guard<decltype(m_keysLock)> keysGuard(nthis->m_keysLock);

  std::vector<std::shared_ptr<CacheableKey>> vlist;

  std::transform(std::begin(m_durableInterestList),
                 std::end(m_durableInterestList), std::back_inserter(vlist),
                 [](const decltype(m_durableInterestList)::value_type& e) {
                   return e.first;
                 });

  std::transform(
      std::begin(m_interestList), std::end(m_interestList),
      std::back_inserter(vlist),
      [](const decltype(m_interestList)::value_type& e) { return e.first; });

  return vlist;
}
std::vector<std::shared_ptr<CacheableString>>
ThinClientRegion::getInterestListRegex() const {
  auto nthis = const_cast<ThinClientRegion*>(this);
  RegionGlobalLocks acquireLocksRedundancy(nthis, false);
  RegionGlobalLocks acquireLocksFailover(nthis);
  CHECK_DESTROY_PENDING(TryReadGuard, getInterestListRegex);
  std::lock_guard<decltype(m_keysLock)> keysGuard(nthis->m_keysLock);

  std::vector<std::shared_ptr<CacheableString>> vlist;

  std::transform(std::begin(m_durableInterestListRegex),
                 std::end(m_durableInterestListRegex),
                 std::back_inserter(vlist),
                 [](const decltype(m_durableInterestListRegex)::value_type& e) {
                   return CacheableString::create(e.first.c_str());
                 });

  std::transform(std::begin(m_interestListRegex), std::end(m_interestListRegex),
                 std::back_inserter(vlist),
                 [](const decltype(m_interestListRegex)::value_type& e) {
                   return CacheableString::create(e.first.c_str());
                 });

  return vlist;
}

GfErrType ThinClientRegion::clientNotificationHandler(TcrMessage& msg) {
  GfErrType err = GF_NOERR;
  std::shared_ptr<Cacheable> oldValue;
  switch (msg.getMessageType()) {
    case TcrMessage::LOCAL_INVALIDATE: {
      LocalRegion::invalidateNoThrow(
          msg.getKey(), msg.getCallbackArgument(), -1,
          CacheEventFlags::NOTIFICATION | CacheEventFlags::LOCAL,
          msg.getVersionTag());
      break;
    }
    case TcrMessage::LOCAL_DESTROY: {
      err = LocalRegion::destroyNoThrow(
          msg.getKey(), msg.getCallbackArgument(), -1,
          CacheEventFlags::NOTIFICATION | CacheEventFlags::LOCAL,
          msg.getVersionTag());
      break;
    }
    case TcrMessage::CLEAR_REGION: {
      LOGDEBUG("remote clear region event for reigon[%s]",
               msg.getRegionName().c_str());
      err = localClearNoThrow(
          nullptr, CacheEventFlags::NOTIFICATION | CacheEventFlags::LOCAL);
      break;
    }
    case TcrMessage::LOCAL_DESTROY_REGION: {
      m_notifyRelease = true;
      err = LocalRegion::destroyRegionNoThrow(
          msg.getCallbackArgument(), true,
          CacheEventFlags::NOTIFICATION | CacheEventFlags::LOCAL);
      break;
    }
    case TcrMessage::LOCAL_CREATE:
      err = LocalRegion::putNoThrow(
          msg.getKey(), msg.getValue(), msg.getCallbackArgument(), oldValue, -1,
          CacheEventFlags::NOTIFICATION | CacheEventFlags::LOCAL,
          msg.getVersionTag());
      break;
    case TcrMessage::LOCAL_UPDATE: {
      //  for update set the NOTIFICATION_UPDATE to trigger the
      // afterUpdate event even if the key is not present in local cache
      err = LocalRegion::putNoThrow(
          msg.getKey(), msg.getValue(), msg.getCallbackArgument(), oldValue, -1,
          CacheEventFlags::NOTIFICATION | CacheEventFlags::NOTIFICATION_UPDATE |
              CacheEventFlags::LOCAL,
          msg.getVersionTag(), msg.getDelta(), msg.getEventId());
      break;
    }
    case TcrMessage::TOMBSTONE_OPERATION:
      LocalRegion::tombstoneOperationNoThrow(msg.getTombstoneVersions(),
                                             msg.getTombstoneKeys());
      break;
    default: {
      if (TcrMessage::getAllEPDisMess() == &msg) {
        setProcessedMarker(false);
        LocalRegion::invokeAfterAllEndPointDisconnected();
      } else {
        LOGERROR(
            "Unknown message type %d in subscription event handler; possible "
            "serialization mismatch",
            msg.getMessageType());
        err = GF_MSG;
      }
      break;
    }
  }

  // Update EventIdMap to mark event processed, Only for durable client.
  // In case of closing, don't send it as listener might not be invoked.
  if (!m_destroyPending && (m_isDurableClnt || msg.hasDelta()) &&
      TcrMessage::getAllEPDisMess() != &msg) {
    m_tcrdm->checkDupAndAdd(msg.getEventId());
  }

  return err;
}

GfErrType ThinClientRegion::handleServerException(const char* func,
                                                  const char* exceptionMsg) {
  GfErrType error = GF_NOERR;
  setThreadLocalExceptionMessage(exceptionMsg);
  if (strstr(exceptionMsg,
             "org.apache.geode.security.NotAuthorizedException") != nullptr) {
    error = GF_NOT_AUTHORIZED_EXCEPTION;
  } else if (strstr(exceptionMsg,
                    "org.apache.geode.cache.CacheWriterException") != nullptr) {
    error = GF_CACHE_WRITER_EXCEPTION;
  } else if (strstr(
                 exceptionMsg,
                 "org.apache.geode.security.AuthenticationFailedException") !=
             nullptr) {
    error = GF_AUTHENTICATION_FAILED_EXCEPTION;
  } else if (strstr(exceptionMsg,
                    "org.apache.geode.internal.cache.execute."
                    "InternalFunctionInvocationTargetException") != nullptr) {
    error = GF_FUNCTION_EXCEPTION;
  } else if (strstr(exceptionMsg,
                    "org.apache.geode.cache.CommitConflictException") !=
             nullptr) {
    error = GF_COMMIT_CONFLICT_EXCEPTION;
  } else if (strstr(exceptionMsg,
                    "org.apache.geode.cache."
                    "TransactionDataNodeHasDepartedException") != nullptr) {
    error = GF_TRANSACTION_DATA_NODE_HAS_DEPARTED_EXCEPTION;
  } else if (strstr(
                 exceptionMsg,
                 "org.apache.geode.cache.TransactionDataRebalancedException") !=
             nullptr) {
    error = GF_TRANSACTION_DATA_REBALANCED_EXCEPTION;
  } else if (strstr(
                 exceptionMsg,
                 "org.apache.geode.security.AuthenticationRequiredException") !=
             nullptr) {
    error = GF_AUTHENTICATION_REQUIRED_EXCEPTION;
  } else {
    error = GF_CACHESERVER_EXCEPTION;
  }

  if (error != GF_AUTHENTICATION_REQUIRED_EXCEPTION) {
    LOGERROR("%s: An exception (%s) happened at remote server.", func,
             exceptionMsg);
  } else {
    LOGFINER("%s: An exception (%s) happened at remote server.", func,
             exceptionMsg);
  }
  return error;
}

void ThinClientRegion::receiveNotification(TcrMessage* msg) {
  std::unique_lock<std::mutex> lock(m_notificationMutex, std::defer_lock);
  {
    TryReadGuard guard(m_rwLock, m_destroyPending);
    if (m_destroyPending) {
      if (msg != TcrMessage::getAllEPDisMess()) {
        _GEODE_SAFE_DELETE(msg);
      }
      return;
    }
    lock.lock();
  }

  if (msg->getMessageType() == TcrMessage::CLIENT_MARKER) {
    handleMarker();
  } else {
    clientNotificationHandler(*msg);
  }

  lock.unlock();
  if (TcrMessage::getAllEPDisMess() != msg) _GEODE_SAFE_DELETE(msg);
}

void ThinClientRegion::localInvalidateRegion_internal() {
  std::shared_ptr<MapEntryImpl> me;
  std::shared_ptr<Cacheable> oldValue;

  std::vector<std::shared_ptr<CacheableKey>> keysVec = keys_internal();
  for (const auto& key : keysVec) {
    std::shared_ptr<VersionTag> versionTag;
    m_entries->invalidate(key, me, oldValue, versionTag);
  }
}

void ThinClientRegion::invalidateInterestList(
    std::unordered_map<std::shared_ptr<CacheableKey>, InterestResultPolicy>&
        interestList) {
  std::shared_ptr<MapEntryImpl> me;
  std::shared_ptr<Cacheable> oldValue;

  if (!m_regionAttributes.getCachingEnabled()) {
    return;
  }
  for (const auto& iter : interestList) {
    std::shared_ptr<VersionTag> versionTag;
    m_entries->invalidate(iter.first, me, oldValue, versionTag);
  }
}

void ThinClientRegion::localInvalidateFailover() {
  CHECK_DESTROY_PENDING(TryReadGuard,
                        ThinClientRegion::localInvalidateFailover);

  //  No need to invalidate from the "m_xxxForUpdatesAsInvalidates" lists?
  if (m_interestListRegex.empty() && m_durableInterestListRegex.empty()) {
    invalidateInterestList(m_interestList);
    invalidateInterestList(m_durableInterestList);
  } else {
    localInvalidateRegion_internal();
  }
}

void ThinClientRegion::localInvalidateForRegisterInterest(
    const std::vector<std::shared_ptr<CacheableKey>>& keys) {
  CHECK_DESTROY_PENDING(TryReadGuard,
                        ThinClientRegion::localInvalidateForRegisterInterest);

  if (!m_regionAttributes.getCachingEnabled()) {
    return;
  }

  std::shared_ptr<Cacheable> oldValue;
  std::shared_ptr<MapEntryImpl> me;

  for (const auto& key : keys) {
    std::shared_ptr<VersionTag> versionTag;
    m_entries->invalidate(key, me, oldValue, versionTag);
    updateAccessAndModifiedTimeForEntry(me, true);
  }
}

InterestResultPolicy ThinClientRegion::copyInterestList(
    std::vector<std::shared_ptr<CacheableKey>>& keysVector,
    std::unordered_map<std::shared_ptr<CacheableKey>, InterestResultPolicy>&
        interestList) const {
  InterestResultPolicy interestPolicy = InterestResultPolicy::NONE;
  for (std::unordered_map<std::shared_ptr<CacheableKey>,
                          InterestResultPolicy>::const_iterator iter =
           interestList.begin();
       iter != interestList.end(); ++iter) {
    keysVector.push_back(iter->first);
    interestPolicy = iter->second;
  }
  return interestPolicy;
}

void ThinClientRegion::registerInterestGetValues(
    const char* method, const std::vector<std::shared_ptr<CacheableKey>>* keys,
    const std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>>&
        resultKeys) {
  auto exceptions = std::make_shared<HashMapOfException>();
  auto err = getAllNoThrow_remote(keys, nullptr, exceptions, resultKeys, true,
                                  nullptr);
  throwExceptionIfError(method, err);
  // log any exceptions here
  for (const auto& iter : *exceptions) {
    LOGWARN("%s Exception for key %s:: %s: %s", method,
            Utils::nullSafeToString(iter.first).c_str(),
            iter.second->getName().c_str(), iter.second->what());
  }
}

void ThinClientRegion::destroyDM(bool keepEndpoints) {
  if (m_tcrdm != nullptr) {
    m_tcrdm->destroy(keepEndpoints);
  }
}

void ThinClientRegion::release(bool invokeCallbacks) {
  if (m_released) {
    return;
  }

  std::unique_lock<std::mutex> lock(m_notificationMutex, std::defer_lock);
  if (!m_notifyRelease) {
    lock.lock();
  }

  destroyDM(invokeCallbacks);

  m_interestList.clear();
  m_interestListRegex.clear();
  m_durableInterestList.clear();
  m_durableInterestListRegex.clear();

  m_interestListForUpdatesAsInvalidates.clear();
  m_interestListRegexForUpdatesAsInvalidates.clear();
  m_durableInterestListForUpdatesAsInvalidates.clear();
  m_durableInterestListRegexForUpdatesAsInvalidates.clear();

  LocalRegion::release(invokeCallbacks);
}

ThinClientRegion::~ThinClientRegion() noexcept {
  TryWriteGuard guard(m_rwLock, m_destroyPending);
  if (!m_destroyPending) {
    release(false);
  }
}

void ThinClientRegion::acquireGlobals(bool isFailover) {
  if (isFailover) {
    m_tcrdm->acquireFailoverLock();
  }
}

void ThinClientRegion::releaseGlobals(bool isFailover) {
  if (isFailover) {
    m_tcrdm->releaseFailoverLock();
  }
}

void ThinClientRegion::executeFunction(
    const std::string& func, const std::shared_ptr<Cacheable>& args,
    std::shared_ptr<CacheableVector> routingObj, uint8_t getResult,
    std::shared_ptr<ResultCollector> rc, int32_t retryAttempts,
    std::chrono::milliseconds timeout) {
  int32_t attempt = 0;
  auto failedNodes = CacheableHashSet::create();
  // if pools retry attempts are not set then retry once on all available
  // endpoints
  if (retryAttempts == -1) {
    retryAttempts = static_cast<int32_t>(m_tcrdm->getNumberOfEndPoints());
  }

  bool reExecute = false;
  bool reExecuteForServ = false;

  do {
    TcrMessage* msg;
    if (reExecuteForServ) {
      msg = new TcrMessageExecuteRegionFunction(
          new DataOutput(m_cacheImpl->createDataOutput()), func, this, args,
          routingObj, getResult, failedNodes, timeout, m_tcrdm.get(),
          static_cast<int8_t>(1));
    } else {
      msg = new TcrMessageExecuteRegionFunction(
          new DataOutput(m_cacheImpl->createDataOutput()), func, this, args,
          routingObj, getResult, failedNodes, timeout, m_tcrdm.get(),
          static_cast<int8_t>(0));
    }
    TcrMessageReply reply(true, m_tcrdm.get());
    // need to check
    ChunkedFunctionExecutionResponse* resultCollector(
        new ChunkedFunctionExecutionResponse(reply, (getResult & 2) == 2, rc));
    reply.setChunkedResultHandler(resultCollector);
    reply.setTimeout(timeout);
    GfErrType err = GF_NOERR;
    err = m_tcrdm->sendSyncRequest(*msg, reply, !(getResult & 1));
    resultCollector->reset();
    delete msg;
    delete resultCollector;
    if (err == GF_NOERR &&
        (reply.getMessageType() == TcrMessage::EXCEPTION ||
         reply.getMessageType() == TcrMessage::EXECUTE_REGION_FUNCTION_ERROR)) {
      err = ThinClientRegion::handleServerException("Execute",
                                                    reply.getException());
    }

    if (ThinClientBaseDM::isFatalClientError(err)) {
      throwExceptionIfError("ExecuteOnRegion:", err);
    } else if (err != GF_NOERR) {
      if (err == GF_FUNCTION_EXCEPTION) {
        reExecute = true;
        rc->clearResults();
        std::shared_ptr<CacheableHashSet> failedNodesIds(reply.getFailedNode());
        failedNodes->clear();
        if (failedNodesIds) {
          LOGDEBUG(
              "ThinClientRegion::executeFunction with GF_FUNCTION_EXCEPTION "
              "failedNodesIds size = %zu ",
              failedNodesIds->size());
          failedNodes->insert(failedNodesIds->begin(), failedNodesIds->end());
        }
      } else if (err == GF_NOTCON) {
        attempt++;
        LOGDEBUG(
            "ThinClientRegion::executeFunction with GF_NOTCON retry attempt = "
            "%d ",
            attempt);
        if (attempt > retryAttempts) {
          throwExceptionIfError("ExecuteOnRegion:", err);
        }
        reExecuteForServ = true;
        rc->clearResults();
        failedNodes->clear();
      } else if (err == GF_TIMEOUT) {
        LOGINFO("function timeout. Name: %s, timeout: %z, params: %" PRIu8
                ", "
                "retryAttempts: %d ",
                func.c_str(), timeout.count(), getResult, retryAttempts);
        throwExceptionIfError("ExecuteOnRegion", GF_TIMEOUT);
      } else if (err == GF_CLIENT_WAIT_TIMEOUT ||
                 err == GF_CLIENT_WAIT_TIMEOUT_REFRESH_PRMETADATA) {
        LOGINFO(
            "function timeout, possibly bucket is not available or bucket "
            "blacklisted. Name: %s, timeout: %z, params: %" PRIu8
            ", retryAttempts: "
            "%d ",
            func.c_str(), timeout.count(), getResult, retryAttempts);
        throwExceptionIfError("ExecuteOnRegion", GF_CLIENT_WAIT_TIMEOUT);
      } else {
        LOGDEBUG("executeFunction err = %d ", err);
        throwExceptionIfError("ExecuteOnRegion:", err);
      }
    } else {
      reExecute = false;
      reExecuteForServ = false;
    }
  } while (reExecuteForServ);

  if (reExecute && (getResult & 1)) {
    reExecuteFunction(func, args, routingObj, getResult, rc, retryAttempts,
                      failedNodes, timeout);
  }
}
std::shared_ptr<CacheableVector> ThinClientRegion::reExecuteFunction(
    const std::string& func, const std::shared_ptr<Cacheable>& args,
    std::shared_ptr<CacheableVector> routingObj, uint8_t getResult,
    std::shared_ptr<ResultCollector> rc, int32_t retryAttempts,
    std::shared_ptr<CacheableHashSet>& failedNodes,
    std::chrono::milliseconds timeout) {
  int32_t attempt = 0;
  bool reExecute = true;
  // if pools retry attempts are not set then retry once on all available
  // endpoints
  if (retryAttempts == -1) {
    retryAttempts = static_cast<int32_t>(m_tcrdm->getNumberOfEndPoints());
  }

  do {
    reExecute = false;
    TcrMessageExecuteRegionFunction msg(
        new DataOutput(m_cacheImpl->createDataOutput()), func, this, args,
        routingObj, getResult, failedNodes, timeout, m_tcrdm.get(),
        static_cast<int8_t>(1));
    TcrMessageReply reply(true, m_tcrdm.get());
    // need to check
    ChunkedFunctionExecutionResponse* resultCollector(
        new ChunkedFunctionExecutionResponse(reply, (getResult & 2) == 2, rc));
    reply.setChunkedResultHandler(resultCollector);
    reply.setTimeout(timeout);

    GfErrType err = GF_NOERR;
    err = m_tcrdm->sendSyncRequest(msg, reply, !(getResult & 1));
    delete resultCollector;
    if (err == GF_NOERR &&
        (reply.getMessageType() == TcrMessage::EXCEPTION ||
         reply.getMessageType() == TcrMessage::EXECUTE_REGION_FUNCTION_ERROR)) {
      err = ThinClientRegion::handleServerException("Execute",
                                                    reply.getException());
    }

    if (ThinClientBaseDM::isFatalClientError(err)) {
      throwExceptionIfError("ExecuteOnRegion:", err);
    } else if (err != GF_NOERR) {
      if (err == GF_FUNCTION_EXCEPTION) {
        reExecute = true;
        rc->clearResults();
        std::shared_ptr<CacheableHashSet> failedNodesIds(reply.getFailedNode());
        failedNodes->clear();
        if (failedNodesIds) {
          LOGDEBUG(
              "ThinClientRegion::reExecuteFunction with GF_FUNCTION_EXCEPTION "
              "failedNodesIds size = %zu ",
              failedNodesIds->size());
          failedNodes->insert(failedNodesIds->begin(), failedNodesIds->end());
        }
      } else if ((err == GF_NOTCON) || (err == GF_CLIENT_WAIT_TIMEOUT) ||
                 (err == GF_CLIENT_WAIT_TIMEOUT_REFRESH_PRMETADATA)) {
        attempt++;
        LOGDEBUG(
            "ThinClientRegion::reExecuteFunction with GF_NOTCON OR TIMEOUT "
            "retry attempt "
            "= %d ",
            attempt);
        if (attempt > retryAttempts) {
          throwExceptionIfError("ExecuteOnRegion:", err);
        }
        reExecute = true;
        rc->clearResults();
        failedNodes->clear();
      } else if (err == GF_TIMEOUT) {
        LOGINFO("function timeout");
        throwExceptionIfError("ExecuteOnRegion", GF_CACHE_TIMEOUT_EXCEPTION);
      } else {
        LOGDEBUG("reExecuteFunction err = %d ", err);
        throwExceptionIfError("ExecuteOnRegion:", err);
      }
    }
  } while (reExecute);
  return nullptr;
}

bool ThinClientRegion::executeFunctionSH(
    const std::string& func, const std::shared_ptr<Cacheable>& args,
    uint8_t getResult, std::shared_ptr<ResultCollector> rc,
    const std::shared_ptr<ClientMetadataService::ServerToKeysMap>& locationMap,
    std::shared_ptr<CacheableHashSet>& failedNodes,
    std::chrono::milliseconds timeout, bool allBuckets) {
  bool reExecute = false;
  auto resultCollectorLock = std::make_shared<std::recursive_mutex>();
  const auto& userAttr = UserAttributes::threadLocalUserAttributes;
  std::vector<std::shared_ptr<OnRegionFunctionExecution>> feWorkers;
  auto& threadPool =
      CacheRegionHelper::getCacheImpl(&getCache())->getThreadPool();

  for (const auto& locationIter : *locationMap) {
    const auto& serverLocation = locationIter.first;
    const auto& routingObj = locationIter.second;
    auto worker = std::make_shared<OnRegionFunctionExecution>(
        func, this, args, routingObj, getResult, timeout,
        dynamic_cast<ThinClientPoolDM*>(m_tcrdm.get()), resultCollectorLock, rc,
        userAttr, false, serverLocation, allBuckets);
    threadPool.perform(worker);
    feWorkers.push_back(worker);
  }

  GfErrType abortError = GF_NOERR;

  for (auto worker : feWorkers) {
    auto err = worker->getResult();
    auto currentReply = worker->getReply();

    if (err == GF_NOERR &&
        (currentReply->getMessageType() == TcrMessage::EXCEPTION ||
         currentReply->getMessageType() ==
             TcrMessage::EXECUTE_REGION_FUNCTION_ERROR)) {
      err = ThinClientRegion::handleServerException(
          "Execute", currentReply->getException());
    }

    if (err != GF_NOERR) {
      if (err == GF_FUNCTION_EXCEPTION) {
        reExecute = true;
        if (auto poolDM =
                std::dynamic_pointer_cast<ThinClientPoolDM>(m_tcrdm)) {
          if (poolDM->getClientMetaDataService()) {
            poolDM->getClientMetaDataService()->enqueueForMetadataRefresh(
                this->getFullPath(), 0);
          }
        }
        worker->getResultCollector()->reset();
        {
          std::lock_guard<decltype(*resultCollectorLock)> guard(
              *resultCollectorLock);
          rc->clearResults();
        }
        std::shared_ptr<CacheableHashSet> failedNodeIds(
            currentReply->getFailedNode());
        if (failedNodeIds) {
          LOGDEBUG(
              "ThinClientRegion::executeFunctionSH with GF_FUNCTION_EXCEPTION "
              "failedNodeIds size = %zu ",
              failedNodeIds->size());
          failedNodes->insert(failedNodeIds->begin(), failedNodeIds->end());
        }
      } else if ((err == GF_NOTCON) || (err == GF_CLIENT_WAIT_TIMEOUT) ||
                 (err == GF_CLIENT_WAIT_TIMEOUT_REFRESH_PRMETADATA)) {
        reExecute = true;
        LOGINFO(
            "ThinClientRegion::executeFunctionSH with GF_NOTCON or "
            "GF_CLIENT_WAIT_TIMEOUT ");
        if (auto poolDM =
                std::dynamic_pointer_cast<ThinClientPoolDM>(m_tcrdm)) {
          if (poolDM->getClientMetaDataService()) {
            poolDM->getClientMetaDataService()->enqueueForMetadataRefresh(
                this->getFullPath(), 0);
          }
        }
        worker->getResultCollector()->reset();
        {
          std::lock_guard<decltype(*resultCollectorLock)> guard(
              *resultCollectorLock);
          rc->clearResults();
        }
      } else {
        if (ThinClientBaseDM::isFatalClientError(err)) {
          LOGERROR("ThinClientRegion::executeFunctionSH: Fatal Exception");
        } else {
          LOGWARN("ThinClientRegion::executeFunctionSH: Unexpected Exception");
        }

        if (abortError == GF_NOERR) {
          abortError = err;
        }
      }
    }
  }

  if (abortError != GF_NOERR) {
    throwExceptionIfError("ExecuteOnRegion:", abortError);
  }
  return reExecute;
}

GfErrType ThinClientRegion::getFuncAttributes(const std::string& func,
                                              std::vector<int8_t>** attr) {
  GfErrType err = GF_NOERR;

  // do TCR GET_FUNCTION_ATTRIBUTES
  LOGDEBUG("Tcrmessage request GET_FUNCTION_ATTRIBUTES ");
  TcrMessageGetFunctionAttributes request(
      new DataOutput(m_cacheImpl->createDataOutput()), func, m_tcrdm.get());
  TcrMessageReply reply(true, m_tcrdm.get());
  err = m_tcrdm->sendSyncRequest(request, reply);
  if (err != GF_NOERR) {
    return err;
  }
  switch (reply.getMessageType()) {
    case TcrMessage::RESPONSE: {
      *attr = reply.getFunctionAttributes();
      break;
    }
    case TcrMessage::EXCEPTION: {
      err = handleServerException("Region::GET_FUNCTION_ATTRIBUTES",
                                  reply.getException());
      break;
    }
    case TcrMessage::REQUEST_DATA_ERROR: {
      LOGERROR("Error message from server: " + reply.getValue()->toString());
      throw FunctionExecutionException(reply.getValue()->toString());
    }
    default: {
      LOGERROR("Unknown message type %d while getting function attributes.",
               reply.getMessageType());
      err = GF_MSG;
      break;
    }
  }
  return err;
}

GfErrType ThinClientRegion::getNoThrow_FullObject(
    std::shared_ptr<EventId> eventId, std::shared_ptr<Cacheable>& fullObject,
    std::shared_ptr<VersionTag>& versionTag) {
  TcrMessageRequestEventValue fullObjectMsg(
      new DataOutput(m_cacheImpl->createDataOutput()), eventId);
  TcrMessageReply reply(true, nullptr);

  GfErrType err = GF_NOTCON;
  err = m_tcrdm->sendSyncRequest(fullObjectMsg, reply, false, true);
  if (err == GF_NOERR) {
    fullObject = reply.getValue();
  }
  versionTag = reply.getVersionTag();
  return err;
}

void ThinClientRegion::txDestroy(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    std::shared_ptr<VersionTag> versionTag) {
  GfErrType err = destroyNoThrowTX(key, aCallbackArgument, -1,
                                   CacheEventFlags::NORMAL, versionTag);
  throwExceptionIfError("Region::destroyTX", err);
}

void ThinClientRegion::txInvalidate(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    std::shared_ptr<VersionTag> versionTag) {
  GfErrType err = invalidateNoThrowTX(key, aCallbackArgument, -1,
                                      CacheEventFlags::NORMAL, versionTag);
  throwExceptionIfError("Region::invalidateTX", err);
}

void ThinClientRegion::txPut(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Cacheable>& value,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    std::shared_ptr<VersionTag> versionTag) {
  std::shared_ptr<Cacheable> oldValue;
  int64_t sampleStartNanos = startStatOpTime();
  GfErrType err = putNoThrowTX(key, value, aCallbackArgument, oldValue, -1,
                               CacheEventFlags::NORMAL, versionTag);

  updateStatOpTime(m_regionStats->getStat(), m_regionStats->getPutTimeId(),
                   sampleStartNanos);
  throwExceptionIfError("Region::putTX", err);
}

void ThinClientRegion::setProcessedMarker(bool) {}

void ChunkedInterestResponse::reset() {
  if (m_resultKeys != nullptr && m_resultKeys->size() > 0) {
    m_resultKeys->clear();
  }
}

void ChunkedInterestResponse::handleChunk(const uint8_t* chunk,
                                          int32_t chunkLen,
                                          uint8_t isLastChunkWithSecurity,
                                          const CacheImpl* cacheImpl) {
  auto input =
      cacheImpl->createDataInput(chunk, chunkLen, m_replyMsg.getPool());

  uint32_t partLen;
  if (TcrMessageHelper::readChunkPartHeader(
          m_msg, input, DSCode::FixedIDDefault,
          static_cast<int32_t>(DSCode::CacheableArrayList),
          "ChunkedInterestResponse", partLen, isLastChunkWithSecurity) !=
      TcrMessageHelper::ChunkObjectType::OBJECT) {
    // encountered an exception part, so return without reading more
    m_replyMsg.readSecureObjectPart(input, false, true,
                                    isLastChunkWithSecurity);
    return;
  }

  if (m_resultKeys == nullptr) {
    m_resultKeys =
        std::make_shared<std::vector<std::shared_ptr<CacheableKey>>>();
  }
  serializer::readObject(input, *m_resultKeys);
  m_replyMsg.readSecureObjectPart(input, false, true, isLastChunkWithSecurity);
}

void ChunkedKeySetResponse::reset() {
  if (m_resultKeys.size() > 0) {
    m_resultKeys.clear();
  }
}

void ChunkedKeySetResponse::handleChunk(const uint8_t* chunk, int32_t chunkLen,
                                        uint8_t isLastChunkWithSecurity,
                                        const CacheImpl* cacheImpl) {
  auto input =
      cacheImpl->createDataInput(chunk, chunkLen, m_replyMsg.getPool());

  uint32_t partLen;
  if (TcrMessageHelper::readChunkPartHeader(
          m_msg, input, DSCode::FixedIDDefault,
          static_cast<int32_t>(DSCode::CacheableArrayList),
          "ChunkedKeySetResponse", partLen, isLastChunkWithSecurity) !=
      TcrMessageHelper::ChunkObjectType::OBJECT) {
    // encountered an exception part, so return without reading more
    m_replyMsg.readSecureObjectPart(input, false, true,
                                    isLastChunkWithSecurity);
    return;
  }

  serializer::readObject(input, m_resultKeys);
  m_replyMsg.readSecureObjectPart(input, false, true, isLastChunkWithSecurity);
}

void ChunkedQueryResponse::reset() {
  m_queryResults->clear();
  m_structFieldNames.clear();
}

void ChunkedQueryResponse::readObjectPartList(DataInput& input,
                                              bool isResultSet) {
  if (input.readBoolean()) {
    LOGERROR("Query response has keys which is unexpected.");
    throw IllegalStateException("Query response has keys which is unexpected.");
  }

  int32_t len = input.readInt32();

  for (int32_t index = 0; index < len; ++index) {
    if (input.read() == 2 /* for exception*/) {
      input.advanceCursor(input.readArrayLength());  // skipLen
      auto exMsgPtr = input.readString();
      throw IllegalStateException(exMsgPtr);
    } else {
      if (isResultSet) {
        std::shared_ptr<Cacheable> value;
        input.readObject(value);
        m_queryResults->push_back(value);
      } else {
        auto code = static_cast<DSCode>(input.read());
        if (code == DSCode::FixedIDByte) {
          auto arrayType = static_cast<DSFid>(input.read());
          if (arrayType != DSFid::CacheableObjectPartList) {
            LOGERROR(
                "Query response got unhandled message format %d while "
                "expecting struct set object part list; possible serialization "
                "mismatch",
                arrayType);
            throw MessageException(
                "Query response got unhandled message format while expecting "
                "struct set object part list; possible serialization mismatch");
          }
          readObjectPartList(input, true);
        } else {
          LOGERROR(
              "Query response got unhandled message format %" PRId8
              "while expecting "
              "struct set object part list; possible serialization mismatch",
              code);
          throw MessageException(
              "Query response got unhandled message format while expecting "
              "struct set object part list; possible serialization mismatch");
        }
      }
    }
  }
}

void ChunkedQueryResponse::handleChunk(const uint8_t* chunk, int32_t chunkLen,
                                       uint8_t isLastChunkWithSecurity,
                                       const CacheImpl* cacheImpl) {
  LOGDEBUG("ChunkedQueryResponse::handleChunk..");
  auto input = cacheImpl->createDataInput(chunk, chunkLen, m_msg.getPool());

  uint32_t partLen;
  auto objType = TcrMessageHelper::readChunkPartHeader(
      m_msg, input, DSCode::FixedIDByte,
      static_cast<int32_t>(DSFid::CollectionTypeImpl), "ChunkedQueryResponse",
      partLen, isLastChunkWithSecurity);
  if (objType == TcrMessageHelper::ChunkObjectType::EXCEPTION) {
    // encountered an exception part, so return without reading more
    m_msg.readSecureObjectPart(input, false, true, isLastChunkWithSecurity);
    return;
  } else if (objType == TcrMessageHelper::ChunkObjectType::NULL_OBJECT) {
    // special case for scalar result
    input.readInt32();  // ignored part length
    input.read();       // ignored is object
    auto intVal = std::dynamic_pointer_cast<CacheableInt32>(input.readObject());
    m_queryResults->push_back(intVal);
    m_msg.readSecureObjectPart(input, false, true, isLastChunkWithSecurity);
    return;
  }

  // ignoring parent classes for now
  // we will require to look at it once CQ is to be implemented.
  // skipping HashSet/StructSet
  // qhe: It was agreed upon that we'll use set for all kinds of results.
  // to avoid dealing with compare operator for user objets.
  // If the results on server are in a bag, or the user need to manipulate
  // the elements, then we have to revisit this issue.
  // For now, we'll live with duplicate records, hoping they do not cost much.
  skipClass(input);
  // skipping CollectionTypeImpl
  // skipClass(input); // no longer, since GFE 5.7

  input.read();  // this is Fixed ID byte (1)
  input.read();  // this is DataSerializable (45)
  input.read();  // this is Class 43
  const auto isStructTypeImpl = input.readString();

  if (isStructTypeImpl == "org.apache.geode.cache.query.Struct") {
    int32_t numOfFldNames = input.readArrayLength();
    bool skip = false;
    if (m_structFieldNames.size() != 0) {
      skip = true;
    }
    for (int i = 0; i < numOfFldNames; i++) {
      auto sptr = input.readString();
      if (!skip) {
        m_structFieldNames.push_back(sptr);
      }
    }
  }

  // skip the remaining part
  input.reset();
  // skip the whole part including partLen and isObj (4+1)
  input.advanceCursor(partLen + 5);

  input.readInt32();  // skip part length

  if (!input.read()) {
    LOGERROR(
        "Query response part is not an object; possible serialization "
        "mismatch");
    throw MessageException(
        "Query response part is not an object; possible serialization "
        "mismatch");
  }

  bool isResultSet = (m_structFieldNames.size() == 0);

  auto arrayType = static_cast<DSCode>(input.read());

  if (arrayType == DSCode::CacheableObjectArray) {
    int32_t arraySize = input.readArrayLength();
    skipClass(input);
    for (int32_t arrayItem = 0; arrayItem < arraySize; ++arrayItem) {
      std::shared_ptr<Serializable> value;
      if (isResultSet) {
        input.readObject(value);
        m_queryResults->push_back(value);
      } else {
        input.read();
        int32_t arraySize2 = input.readArrayLength();
        skipClass(input);
        for (int32_t index = 0; index < arraySize2; ++index) {
          input.readObject(value);
          m_queryResults->push_back(value);
        }
      }
    }
  } else if (arrayType == DSCode::FixedIDByte) {
    arrayType = static_cast<DSCode>(input.read());
    if (static_cast<int32_t>(arrayType) !=
        static_cast<int32_t>(DSFid::CacheableObjectPartList)) {
      LOGERROR(
          "Query response got unhandled message format %d while expecting "
          "object part list; possible serialization mismatch",
          arrayType);
      throw MessageException(
          "Query response got unhandled message format while expecting object "
          "part list; possible serialization mismatch");
    }
    readObjectPartList(input, isResultSet);
  } else {
    LOGERROR(
        "Query response got unhandled message format %d; possible "
        "serialization mismatch",
        arrayType);
    throw MessageException(
        "Query response got unhandled message format; possible serialization "
        "mismatch");
  }

  m_msg.readSecureObjectPart(input, false, true, isLastChunkWithSecurity);
}

void ChunkedQueryResponse::skipClass(DataInput& input) {
  auto classByte = static_cast<DSCode>(input.read());
  if (classByte == DSCode::Class) {
    // ignore string type id - assuming its a normal (under 64k) string.
    input.read();
    uint16_t classLen = input.readInt16();
    input.advanceCursor(classLen);
  } else {
    throw IllegalStateException(
        "ChunkedQueryResponse::skipClass: "
        "Did not get expected class header byte");
  }
}

void ChunkedFunctionExecutionResponse::reset() {
  // m_functionExecutionResults->clear();
}

void ChunkedFunctionExecutionResponse::handleChunk(
    const uint8_t* chunk, int32_t chunkLen, uint8_t isLastChunkWithSecurity,
    const CacheImpl* cacheImpl) {
  LOGDEBUG("ChunkedFunctionExecutionResponse::handleChunk");
  auto input = cacheImpl->createDataInput(chunk, chunkLen, m_msg.getPool());

  uint32_t partLen;

  TcrMessageHelper::ChunkObjectType arrayType;
  if ((arrayType = TcrMessageHelper::readChunkPartHeader(
           m_msg, input, "ChunkedFunctionExecutionResponse", partLen,
           isLastChunkWithSecurity)) ==
      TcrMessageHelper::ChunkObjectType::EXCEPTION) {
    // encountered an exception part, so return without reading more
    m_msg.readSecureObjectPart(input, false, true, isLastChunkWithSecurity);
    return;
  }

  if (m_getResult == false) {
    return;
  }

  if (static_cast<TcrMessageHelper::ChunkObjectType>(arrayType) ==
      TcrMessageHelper::ChunkObjectType::NULL_OBJECT) {
    LOGDEBUG("ChunkedFunctionExecutionResponse::handleChunk nullptr object");
    //	m_functionExecutionResults->push_back(nullptr);
    m_msg.readSecureObjectPart(input, false, true, isLastChunkWithSecurity);
    return;
  }

  auto startLen =
      input.getBytesRead() -
      1;  // from here need to look value part + memberid AND -1 for array type
  // iread adn gnore array length
  input.readArrayLength();

  // read a byte to determine whether to read exception part for sendException
  // or read objects.
  auto partType = static_cast<DSCode>(input.read());
  bool isExceptionPart = false;
  // See If partType is JavaSerializable
  const int CHUNK_HDR_LEN = 5;
  const int SECURE_PART_LEN = 5 + 8;
  bool readPart = true;
  LOGDEBUG(
      "ChunkedFunctionExecutionResponse::handleChunk chunkLen = %d & partLen = "
      "%d ",
      chunkLen, partLen);
  if (partType == DSCode::JavaSerializable) {
    isExceptionPart = true;
    // reset the input.
    input.reset();

    if (((isLastChunkWithSecurity & 0x02) &&
         (chunkLen - static_cast<int32_t>(partLen) <=
          CHUNK_HDR_LEN + SECURE_PART_LEN)) ||
        (((isLastChunkWithSecurity & 0x02) == 0) &&
         (chunkLen - static_cast<int32_t>(partLen) <= CHUNK_HDR_LEN))) {
      readPart = false;
      partLen = input.readInt32();
      input.advanceCursor(1);  // skip isObject byte
      input.advanceCursor(partLen);
    } else {
      // skip first part i.e JavaSerializable.
      TcrMessageHelper::skipParts(m_msg, input, 1);

      // read the second part which is string in usual manner, first its length.
      partLen = input.readInt32();

      // then isObject byte
      input.read();  // ignore iSobject

      startLen = input.getBytesRead();  // reset from here need to look value
      // part + memberid AND -1 for array type

      // Since it is contained as a part of other results, read arrayType which
      // is arrayList = 65.
      input.read();

      // read and ignore its len which is 2
      input.readArrayLength();
    }
  } else {
    // rewind cursor by 1 to what we had read a byte to determine whether to
    // read exception part or read objects.
    input.rewindCursor(1);
  }

  // Read either object or exception string from sendException.
  std::shared_ptr<Serializable> value;
  // std::shared_ptr<Cacheable> memberId;
  if (readPart) {
    input.readObject(value);
    // TODO: track this memberId for PrFxHa
    // input.readObject(memberId);
    auto objectlen = input.getBytesRead() - startLen;

    auto memberIdLen = partLen - objectlen;
    input.advanceCursor(memberIdLen);
    LOGDEBUG("function partlen = %d , objectlen = %z,  memberidlen = %z ",
             partLen, objectlen, memberIdLen);
    LOGDEBUG("function input.getBytesRemaining() = %z ",
             input.getBytesRemaining());

  } else {
    value = CacheableString::create("Function exception result.");
  }
  if (m_rc != nullptr) {
    std::shared_ptr<Cacheable> result = nullptr;
    if (isExceptionPart) {
      result = std::make_shared<UserFunctionExecutionException>(
          std::dynamic_pointer_cast<CacheableString>(value)->value());
    } else {
      result = value;
    }
    if (m_resultCollectorLock) {
      std::lock_guard<decltype(*m_resultCollectorLock)> guard(
          *m_resultCollectorLock);
      m_rc->addResult(result);
    } else {
      m_rc->addResult(result);
    }
  }

  m_msg.readSecureObjectPart(input, false, true, isLastChunkWithSecurity);
  //  m_functionExecutionResults->push_back(value);
}

void ChunkedGetAllResponse::reset() {
  m_keysOffset = 0;
  if (m_resultKeys != nullptr && m_resultKeys->size() > 0) {
    m_resultKeys->clear();
  }
}

// process a GET_ALL response chunk
void ChunkedGetAllResponse::handleChunk(const uint8_t* chunk, int32_t chunkLen,
                                        uint8_t isLastChunkWithSecurity,
                                        const CacheImpl* cacheImpl) {
  auto input = cacheImpl->createDataInput(chunk, chunkLen, m_msg.getPool());

  uint32_t partLen;
  if (TcrMessageHelper::readChunkPartHeader(
          m_msg, input, DSCode::FixedIDByte,
          static_cast<int32_t>(DSFid::VersionedObjectPartList),
          "ChunkedGetAllResponse", partLen, isLastChunkWithSecurity) !=
      TcrMessageHelper::ChunkObjectType::OBJECT) {
    // encountered an exception part, so return without reading more
    m_msg.readSecureObjectPart(input, false, true, isLastChunkWithSecurity);
    return;
  }

  VersionedCacheableObjectPartList objectList(
      m_keys, &m_keysOffset, m_values, m_exceptions, m_resultKeys, m_region,
      &m_trackerMap, m_destroyTracker, m_addToLocalCache, m_dsmemId,
      m_responseLock);

  objectList.fromData(input);

  m_msg.readSecureObjectPart(input, false, true, isLastChunkWithSecurity);
}

void ChunkedGetAllResponse::add(const ChunkedGetAllResponse* other) {
  if (m_values) {
    for (const auto& iter : *m_values) {
      m_values->emplace(iter.first, iter.second);
    }
  }

  if (m_exceptions) {
    m_exceptions->insert(other->m_exceptions->begin(),
                         other->m_exceptions->end());
  }

  for (const auto& iter : other->m_trackerMap) {
    m_trackerMap[iter.first] = iter.second;
  }

  if (m_resultKeys) {
    m_resultKeys->insert(m_resultKeys->end(), other->m_resultKeys->begin(),
                         other->m_resultKeys->end());
  }
}

void ChunkedPutAllResponse::reset() {
  if (m_list != nullptr && m_list->size() > 0) {
    m_list->getVersionedTagptr().clear();
  }
}

// process a PUT_ALL response chunk
void ChunkedPutAllResponse::handleChunk(const uint8_t* chunk, int32_t chunkLen,
                                        uint8_t isLastChunkWithSecurity,
                                        const CacheImpl* cacheImpl) {
  auto input = cacheImpl->createDataInput(chunk, chunkLen, m_msg.getPool());

  uint32_t partLen;
  TcrMessageHelper::ChunkObjectType chunkType;
  if ((chunkType = TcrMessageHelper::readChunkPartHeader(
           m_msg, input, DSCode::FixedIDByte,
           static_cast<int32_t>(DSFid::VersionedObjectPartList),
           "ChunkedPutAllResponse", partLen, isLastChunkWithSecurity)) ==
      TcrMessageHelper::ChunkObjectType::NULL_OBJECT) {
    LOGDEBUG("ChunkedPutAllResponse::handleChunk nullptr object");
    // No issues it will be empty in case of disabled caching.
    m_msg.readSecureObjectPart(input, false, true, isLastChunkWithSecurity);
    return;
  }

  if (chunkType == TcrMessageHelper::ChunkObjectType::OBJECT) {
    LOGDEBUG("ChunkedPutAllResponse::handleChunk object");
    std::recursive_mutex responseLock;
    auto vcObjPart = std::make_shared<VersionedCacheableObjectPartList>(
        dynamic_cast<ThinClientRegion*>(m_region.get()),
        m_msg.getChunkedResultHandler()->getEndpointMemId(), responseLock);
    vcObjPart->fromData(input);
    m_list->addAll(vcObjPart);
    m_msg.readSecureObjectPart(input, false, true, isLastChunkWithSecurity);
  } else {
    LOGDEBUG("ChunkedPutAllResponse::handleChunk BYTES PART");
    const auto byte0 = input.read();
    LOGDEBUG("ChunkedPutAllResponse::handleChunk single-hop bytes byte0 = %d ",
             byte0);
    const auto byte1 = input.read();
    m_msg.readSecureObjectPart(input, false, true, isLastChunkWithSecurity);

    auto pool = m_msg.getPool();
    if (pool != nullptr && !pool->isDestroyed() &&
        pool->getPRSingleHopEnabled()) {
      auto poolDM = dynamic_cast<ThinClientPoolDM*>(pool);
      if ((poolDM != nullptr) &&
          (poolDM->getClientMetaDataService() != nullptr) && (byte0 != 0)) {
        LOGFINE("enqueued region " + m_region->getFullPath() +
                " for metadata refresh for singlehop for PUTALL operation.");
        poolDM->getClientMetaDataService()->enqueueForMetadataRefresh(
            m_region->getFullPath(), byte1);
      }
    }
  }
}

void ChunkedRemoveAllResponse::reset() {
  if (m_list != nullptr && m_list->size() > 0) {
    m_list->getVersionedTagptr().clear();
  }
}

// process a REMOVE_ALL response chunk
void ChunkedRemoveAllResponse::handleChunk(const uint8_t* chunk,
                                           int32_t chunkLen,
                                           uint8_t isLastChunkWithSecurity,
                                           const CacheImpl* cacheImpl) {
  auto input = cacheImpl->createDataInput(chunk, chunkLen, m_msg.getPool());

  uint32_t partLen;
  TcrMessageHelper::ChunkObjectType chunkType;
  if ((chunkType = TcrMessageHelper::readChunkPartHeader(
           m_msg, input, DSCode::FixedIDByte,
           static_cast<int32_t>(DSFid::VersionedObjectPartList),
           "ChunkedRemoveAllResponse", partLen, isLastChunkWithSecurity)) ==
      TcrMessageHelper::ChunkObjectType::NULL_OBJECT) {
    LOGDEBUG("ChunkedRemoveAllResponse::handleChunk nullptr object");
    // No issues it will be empty in case of disabled caching.
    m_msg.readSecureObjectPart(input, false, true, isLastChunkWithSecurity);
    return;
  }

  if (chunkType == TcrMessageHelper::ChunkObjectType::OBJECT) {
    LOGDEBUG("ChunkedRemoveAllResponse::handleChunk object");
    std::recursive_mutex responseLock;
    auto vcObjPart = std::make_shared<VersionedCacheableObjectPartList>(
        dynamic_cast<ThinClientRegion*>(m_region.get()),
        m_msg.getChunkedResultHandler()->getEndpointMemId(), responseLock);
    vcObjPart->fromData(input);
    m_list->addAll(vcObjPart);
    m_msg.readSecureObjectPart(input, false, true, isLastChunkWithSecurity);
  } else {
    LOGDEBUG("ChunkedRemoveAllResponse::handleChunk BYTES PART");
    const auto byte0 = input.read();
    LOGDEBUG(
        "ChunkedRemoveAllResponse::handleChunk single-hop bytes byte0 = %d ",
        byte0);
    const auto byte1 = input.read();
    m_msg.readSecureObjectPart(input, false, true, isLastChunkWithSecurity);

    auto pool = m_msg.getPool();
    if (pool != nullptr && !pool->isDestroyed() &&
        pool->getPRSingleHopEnabled()) {
      auto poolDM = dynamic_cast<ThinClientPoolDM*>(pool);
      if ((poolDM != nullptr) &&
          (poolDM->getClientMetaDataService() != nullptr) && (byte0 != 0)) {
        LOGFINE("enqueued region " + m_region->getFullPath() +
                " for metadata refresh for singlehop for REMOVEALL operation.");
        poolDM->getClientMetaDataService()->enqueueForMetadataRefresh(
            m_region->getFullPath(), byte1);
      }
    }
  }
}

void ChunkedDurableCQListResponse::reset() {
  if (m_resultList) {
    m_resultList->clear();
  }
}

// handles the chunk response for GETDURABLECQS_MSG_TYPE
void ChunkedDurableCQListResponse::handleChunk(const uint8_t* chunk,
                                               int32_t chunkLen, uint8_t,
                                               const CacheImpl* cacheImpl) {
  auto input = cacheImpl->createDataInput(chunk, chunkLen, m_msg.getPool());

  // read and ignore part length
  input.readInt32();
  if (!input.readBoolean()) {
    // we're currently always expecting an object
    char exMsg[256];
    std::snprintf(
        exMsg, 255,
        "ChunkedDurableCQListResponse::handleChunk: part is not object");
    throw MessageException(exMsg);
  }

  input.advanceCursor(1);  // skip the CacheableArrayList type ID byte

  const auto stringParts = input.read();  // read the number of strings in the
                                          // message this is one byte

  for (int i = 0; i < stringParts; i++) {
    m_resultList->push_back(
        std::dynamic_pointer_cast<CacheableString>(input.readObject()));
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
