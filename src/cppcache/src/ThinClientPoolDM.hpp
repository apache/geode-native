#pragma once

#ifndef GEODE_THINCLIENTPOOLDM_H_
#define GEODE_THINCLIENTPOOLDM_H_

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

#include <string>
#include "ThinClientBaseDM.hpp"
#include <geode/Pool.hpp>
#include "PoolAttributes.hpp"
#include "ThinClientLocatorHelper.hpp"
#include "RemoteQueryService.hpp"
#include <memory>
#include <set>
#include <vector>
#include "Task.hpp"
#include <ace/Semaphore.h>
#include "PoolStatistics.hpp"
#include "FairQueue.hpp"
#include "TcrPoolEndPoint.hpp"
#include "ThinClientRegion.hpp"
#include <geode/ResultCollector.hpp>
#include "ExecutionImpl.hpp"
#include "ClientMetadataService.hpp"
#include "ThreadPool.hpp"
#include "ThinClientStickyManager.hpp"
#include "TXState.hpp"

#include "NonCopyable.hpp"

namespace apache {
namespace geode {
namespace statistics {
class PoolStatsSampler;
}  // namespace statistics
}  // namespace geode
}  // namespace apache
using namespace apache::geode::statistics;
namespace apache {
namespace geode {
namespace client {
class CacheImpl;
class FunctionExecution;

/* adongre
 * CID 28731: Other violation (MISSING_COPY)
 * Class "apache::geode::client::ThinClientPoolDM" owns resources that are
 managed in its
 * constructor and destructor but has no user-written copy constructor.
 * FIX : Make the class no Copyablez

 * CID 28717: Other violation (MISSING_ASSIGN)
 * Class "apache::geode::client::ThinClientPoolDM" owns resources that are
 * managed in its constructor and destructor but has no user-written assignment
 operator.
 * Fix : Make the class Non Assinable
 */
class ThinClientPoolDM
    : public ThinClientBaseDM,
      public Pool,
      public FairQueue<TcrConnection, ACE_Recursive_Thread_Mutex>,
      private NonCopyable,
      private NonAssignable {
 public:
  ThinClientPoolDM(const char* name, PoolAttributesPtr poolAttrs,
                   TcrConnectionManager& connManager);

  virtual void init();

  const char* getName() const { return m_poolName.c_str(); }

  virtual GfErrType sendSyncRequest(TcrMessage& request, TcrMessageReply& reply,
                                    bool attemptFailover = true,
                                    bool isBGThread = false);
  GfErrType sendSyncRequest(TcrMessage& request, TcrMessageReply& reply,
                            bool attemptFailover, bool isBGThread,
                            const BucketServerLocationPtr& serverLocation);

  // Pool Specific Fns.
  virtual const CacheableStringArrayPtr getLocators() const;
  virtual const CacheableStringArrayPtr getServers();
  virtual void destroy(bool keepalive = false);
  virtual bool isDestroyed() const;
  virtual QueryServicePtr getQueryService();
  virtual QueryServicePtr getQueryServiceWithoutCheck();
  virtual bool isEndpointAttached(TcrEndpoint* ep);
  GfErrType sendRequestToAllServers(const char* func, uint8_t getResult,
                                    uint32_t timeout, CacheablePtr args,
                                    ResultCollectorPtr& rs,
                                    CacheableStringPtr& exceptionPtr);

  GfErrType sendRequestToEP(const TcrMessage& request, TcrMessageReply& reply,
                            TcrEndpoint* currentEndpoint);
  void addConnection(TcrConnection* conn);

  TcrEndpoint* addEP(ServerLocation& serverLoc);

  TcrEndpoint* addEP(const char* endpointName);
  virtual int pingServer(volatile bool& isRunning);
  virtual int updateLocatorList(volatile bool& isRunning);
  virtual int cliCallback(volatile bool& isRunning);
  virtual void pingServerLocal();

  virtual ~ThinClientPoolDM() {
    destroy();
    GF_SAFE_DELETE(m_locHelper);
    GF_SAFE_DELETE(m_stats);
    GF_SAFE_DELETE(m_clientMetadataService);
    GF_SAFE_DELETE(m_manager);
  }
  // void updateQueue(const char* regionPath) ;
  ClientProxyMembershipID* getMembershipId() { return m_memId.get(); }
  virtual void processMarker(){};
  virtual bool checkDupAndAdd(EventIdPtr eventid) {
    return m_connManager.checkDupAndAdd(eventid);
  }
  ACE_Recursive_Thread_Mutex& getPoolLock() { return getQueueLock(); }
  void reducePoolSize(int num);
  void removeEPConnections(int numConn, bool triggerManagerConn = true);
  void removeEPConnections(TcrEndpoint* ep);
  GfErrType createPoolConnection(TcrConnection*& conn,
                                 std::set<ServerLocation>& excludeServers,
                                 bool& maxConnLimit,
                                 const TcrConnection* currentServer = nullptr);
  ThinClientLocatorHelper* getLocatorHelper() volatile {
    return (ThinClientLocatorHelper*)m_locHelper;
  }
  virtual void releaseThreadLocalConnection();
  virtual void setThreadLocalConnection(TcrConnection* conn);
  bool excludeConnection(TcrConnection*, std::set<ServerLocation>&);
  void incRegionCount();
  void decRegionCount();

  virtual void setStickyNull(bool isBGThread) {
    if (!isBGThread) m_manager->setStickyConnection(nullptr, false);
  };
  virtual bool canItBeDeletedNoImpl(TcrConnection* conn) { return false; };
  void updateNotificationStats(bool isDeltaSuccess, long timeInNanoSecond);
  virtual bool isSecurityOn() { return m_isSecurityOn || m_isMultiUserMode; }
  virtual bool isMultiUserMode() { return m_isMultiUserMode; }

  virtual void sendUserCacheCloseMessage(bool keepAlive);

  virtual inline PoolStats& getStats() { return *(PoolStats*)m_stats; }

  size_t getNumberOfEndPoints() const { return m_endpoints.current_size(); }

  int32_t GetPDXIdForType(SerializablePtr pdxType);

  SerializablePtr GetPDXTypeById(int32_t typeId);

  void AddPdxType(SerializablePtr pdxType, int32_t pdxTypeId);

  int32_t GetEnumValue(SerializablePtr enumInfo);
  SerializablePtr GetEnum(int32_t val);
  void AddEnum(SerializablePtr enumInfo, int enumVal);

  // Tries to get connection to a endpoint. If no connection is available, it
  // tries
  // to create one. If it fails to create one,  it returns a connection to any
  // other
  // server after failing over the transaction to that server
  GfErrType getConnectionToAnEndPoint(std::string epNameStr,
                                      TcrConnection*& conn);

  virtual inline bool isSticky() { return m_sticky; }
  virtual TcrEndpoint* getEndPoint(
      const BucketServerLocationPtr& serverLocation, int8_t& version,
      std::set<ServerLocation>& excludeServers);

  ClientMetadataService* getClientMetaDataService() {
    return m_clientMetadataService;
  }
  void setPrimaryServerQueueSize(int queueSize) {
    m_primaryServerQueueSize = queueSize;
  }
  int getPrimaryServerQueueSize() const { return m_primaryServerQueueSize; }

 protected:
  ThinClientStickyManager* m_manager;
  std::vector<std::string> m_canonicalHosts;
  ACE_Map_Manager<std::string, TcrEndpoint*, ACE_Recursive_Thread_Mutex>
      m_endpoints;
  ACE_Recursive_Thread_Mutex m_endpointsLock;
  ACE_Recursive_Thread_Mutex m_endpointSelectionLock;
  std::string m_poolName;
  volatile PoolStats* m_stats;
  bool m_sticky;
  // PoolStats * m_stats;
  // PoolStatType* m_poolStatType;
  void netDown();
  ACE_Semaphore m_updateLocatorListSema;
  ACE_Semaphore m_pingSema;
  ACE_Semaphore m_cliCallbackSema;
  volatile bool m_isDestroyed;
  volatile bool m_destroyPending;
  volatile bool m_destroyPendingHADM;
  void checkRegions();
  RemoteQueryServicePtr m_remoteQueryServicePtr;
  virtual void startBackgroundThreads();
  virtual void stopPingThread();
  virtual void stopUpdateLocatorListThread();
  virtual void stopCliCallbackThread();
  virtual void cleanStickyConnections(volatile bool& isRunning);
  virtual TcrConnection* getConnectionFromQueue(bool timeout, GfErrType* error,
                                                std::set<ServerLocation>&,
                                                bool& maxConnLimit);
  virtual void putInQueue(TcrConnection* conn, bool isBGThread,
                          bool isTransaction = false) {
    if (isTransaction) {
      m_manager->setStickyConnection(conn, isTransaction);
    } else {
      put(conn, false);
    }
  };

  GfErrType doFailover(TcrConnection* conn);

  virtual bool canItBeDeleted(TcrConnection* conn);
  virtual TcrConnection* getConnectionFromQueueW(
      GfErrType* error, std::set<ServerLocation>& excludeServers,
      bool isBGThread, TcrMessage& request, int8_t& version, bool& match,
      bool& connFound,
      const BucketServerLocationPtr& serverLocation = nullptr) {
    TcrConnection* conn = nullptr;
    TcrEndpoint* theEP = nullptr;
    LOGDEBUG("prEnabled = %s, forSingleHop = %s %d",
             m_attrs->getPRSingleHopEnabled() ? "true" : "false",
             request.forSingleHop() ? "true" : "false",
             request.getMessageType());

    match = false;
    BucketServerLocationPtr slTmp = nullptr;
    if (request.forTransaction()) {
      bool connFound =
          m_manager->getStickyConnection(conn, error, excludeServers, true);
      TXState* txState = TSSTXStateWrapper::s_geodeTSSTXState->getTXState();
      if (*error == GF_NOERR && !connFound &&
          (txState == nullptr || txState->isDirty())) {
        *error = doFailover(conn);
      }

      if (*error != GF_NOERR) {
        return nullptr;
      }

      if (txState != nullptr) {
        txState->setDirty();
      }
    } else if (serverLocation != nullptr /*&& excludeServers.size() == 0*/) {
      theEP = getEndPoint(serverLocation, version, excludeServers);
    } else if (
        m_attrs->getPRSingleHopEnabled() /*&& excludeServers.size() == 0*/ &&
        request.forSingleHop() &&
        (request.getMessageType() != TcrMessage::GET_ALL_70) &&
        (request.getMessageType() != TcrMessage::GET_ALL_WITH_CALLBACK)) {
      theEP = getSingleHopServer(request, version, slTmp, excludeServers);
      if (theEP != nullptr) {
        // if all buckets are not initialized
        //  match = true;
      }
      if (slTmp != nullptr && m_clientMetadataService != nullptr) {
        if (m_clientMetadataService->isBucketMarkedForTimeout(
                request.getRegionName().c_str(), slTmp->getBucketId()) ==
            true) {
          *error = GF_CLIENT_WAIT_TIMEOUT;
          return nullptr;
        }
      }
      LOGDEBUG("theEP is %p", theEP);
    }
    bool maxConnLimit = false;
    if (theEP != nullptr) {
      conn = getFromEP(theEP);
      if (!conn) {
        LOGFINER("Creating connection to endpint as not found in pool ");
        *error =
            createPoolConnectionToAEndPoint(conn, theEP, maxConnLimit, true);
        if (*error == GF_CLIENT_WAIT_TIMEOUT ||
            *error == GF_CLIENT_WAIT_TIMEOUT_REFRESH_PRMETADATA) {
          if (m_clientMetadataService == nullptr ||
              request.getKey() == nullptr) {
            return nullptr;
          }
          RegionPtr region;
          m_connManager.getCacheImpl()->getRegion(
              request.getRegionName().c_str(), region);
          if (region != nullptr) {
            slTmp = nullptr;
            m_clientMetadataService
                ->markPrimaryBucketForTimeoutButLookSecondaryBucket(
                    region, request.getKey(), request.getValue(),
                    request.getCallbackArgument(), request.forPrimary(), slTmp,
                    version);
          }
          return nullptr;
        }
      }
    }
    if (conn == nullptr) {
      LOGDEBUG("conn not found");
      match = false;
      LOGDEBUG("looking For connection");
      conn = getConnectionFromQueue(true, error, excludeServers, maxConnLimit);
      LOGDEBUG("Connection Found");
    }

    if (maxConnLimit) {
      // we reach max connection limit, found connection but endpoint is
      // (not)different, no need to refresh pr-meta-data
      connFound = true;
    } else {
      // if server hints pr-meta-data refresh then refresh
      // anything else???
    }

    LOGDEBUG(
        "ThinClientPoolDM::getConnectionFromQueueW return conn = %p match = %d "
        "connFound=%d",
        conn, match, connFound);
    return conn;
  }
  TcrConnection* getFromEP(TcrEndpoint* theEP);
  virtual TcrEndpoint* getSingleHopServer(
      TcrMessage& request, int8_t& version,
      BucketServerLocationPtr& serverLocation,
      std::set<ServerLocation>& excludeServers);
  // Create pool connection to a specified endpoint.
  GfErrType createPoolConnectionToAEndPoint(TcrConnection*& conn,
                                            TcrEndpoint* theEP,
                                            bool& maxConnLimit,
                                            bool appThreadrequest = false);

 private:
  bool hasExpired(TcrConnection* conn);

  PropertiesPtr getCredentials(TcrEndpoint* ep);
  GfErrType sendUserCredentials(PropertiesPtr credentials, TcrConnection*& conn,
                                bool isBGThread, bool& isServerException);
  TcrConnection* getConnectionInMultiuserMode(UserAttributesPtr userAttribute);

  // get endpoint using the endpoint string
  TcrEndpoint* getEndPoint(std::string epNameStr);

  bool m_isSecurityOn;
  bool m_isMultiUserMode;

  TcrConnection* getUntil(int64_t& sec, GfErrType* error,
                          std::set<ServerLocation>& excludeServers,
                          bool& maxConnLimit) {
    bool isClosed;
    TcrConnection* mp =
        getNoGetLock(isClosed, error, excludeServers, maxConnLimit);

    if (mp == nullptr && !isClosed) {
      mp = getUntilWithToken(sec, isClosed, &excludeServers);
    }

    return mp;
  }

  TcrConnection* getNoGetLock(bool& isClosed, GfErrType* error,
                              std::set<ServerLocation>& excludeServers,
                              bool& maxConnLimit);
  bool exclude(TcrConnection* conn, std::set<ServerLocation>& excludeServers);
  void deleteAction() { removeEPConnections(1); }

  std::string selectEndpoint(std::set<ServerLocation>&,
                             const TcrConnection* currentServer = nullptr);
  // TODO global - m_memId was volatile
  std::unique_ptr<ClientProxyMembershipID> m_memId;
  virtual TcrEndpoint* createEP(const char* endpointName) {
    return new TcrPoolEndPoint(endpointName, m_connManager.getCacheImpl(),
                               m_connManager.m_failoverSema,
                               m_connManager.m_cleanupSema,
                               m_connManager.m_redundancySema, this);
  }
  virtual void removeCallbackConnection(TcrEndpoint*) {}

  bool excludeServer(std::string, std::set<ServerLocation>&);

  volatile ThinClientLocatorHelper* m_locHelper;

  std::atomic<int32_t> m_poolSize;  // Actual Size of Pool
  int m_numRegions;

  // for selectEndpoint
  unsigned m_server;

  // Manage Connection thread
  ACE_Semaphore m_connSema;
  Task<ThinClientPoolDM>* m_connManageTask;
  Task<ThinClientPoolDM>* m_pingTask;
  Task<ThinClientPoolDM>* m_updateLocatorListTask;
  Task<ThinClientPoolDM>* m_cliCallbackTask;
  long m_pingTaskId;
  long m_updateLocatorListTaskId;
  long m_connManageTaskId;
  int manageConnections(volatile bool& isRunning);
  int doPing(const ACE_Time_Value&, const void*);
  int doUpdateLocatorList(const ACE_Time_Value&, const void*);
  int doManageConnections(const ACE_Time_Value&, const void*);
  int manageConnectionsInternal(volatile bool& isRunning);
  void cleanStaleConnections(volatile bool& isRunning);
  void restoreMinConnections(volatile bool& isRunning);
  std::atomic<int32_t> m_clientOps;  // Actual Size of Pool
  statistics::PoolStatsSampler* m_PoolStatsSampler;
  ClientMetadataService* m_clientMetadataService;
  friend class CacheImpl;
  friend class ThinClientStickyManager;
  friend class FunctionExecution;
  static const char* NC_Ping_Thread;
  static const char* NC_MC_Thread;
  int m_primaryServerQueueSize;
};

typedef std::shared_ptr<ThinClientPoolDM> ThinClientPoolDMPtr;
class FunctionExecution : public PooledWork<GfErrType> {
  ThinClientPoolDM* m_poolDM;
  TcrEndpoint* m_ep;
  const char* m_func;
  uint8_t m_getResult;
  uint32_t m_timeout;
  CacheablePtr m_args;
  GfErrType m_error;
  ResultCollectorPtr* m_rc;
  std::shared_ptr<ACE_Recursive_Thread_Mutex> m_resultCollectorLock;
  CacheableStringPtr exceptionPtr;
  UserAttributesPtr m_userAttr;

 public:
  FunctionExecution() {
    m_poolDM = nullptr;
    m_ep = nullptr;
    m_func = nullptr;
    m_getResult = 0;
    m_timeout = 0;
    m_error = GF_NOERR;
    m_rc = nullptr;
    m_resultCollectorLock = nullptr;
    m_userAttr = nullptr;
  }

  ~FunctionExecution() {}

  CacheableStringPtr getException() { return exceptionPtr; }

  void setParameters(const char* func, uint8_t getResult, uint32_t timeout,
                     CacheablePtr args, TcrEndpoint* ep,
                     ThinClientPoolDM* poolDM,
                     const std::shared_ptr<ACE_Recursive_Thread_Mutex>& rCL,
                     ResultCollectorPtr* rs, UserAttributesPtr userAttr) {
    exceptionPtr = nullptr;
    m_resultCollectorLock = rCL;
    m_rc = rs;
    m_error = GF_NOTCON;
    m_func = func;
    m_getResult = getResult;
    m_timeout = timeout;
    m_args = args;
    m_ep = ep;
    m_poolDM = poolDM;
    m_userAttr = userAttr;

    // m_functionExecutionTask = new Task<FunctionExecution>(this,
    //&FunctionExecution::execute);
  }

  GfErrType execute(void) {
    // TSSUserAttributesWrapper::s_geodeTSSUserAttributes->setUserAttributes(m_userAttr);
    GuardUserAttribures gua;

    if (m_userAttr != nullptr) gua.setProxyCache(m_userAttr->getProxyCache());

    std::string funcName(m_func);
    TcrMessageExecuteFunction request(m_poolDM->getConnectionManager()
                                          .getCacheImpl()
                                          ->getCache()
                                          ->createDataOutput(),
                                      funcName, m_args, m_getResult, m_poolDM,
                                      m_timeout);
    TcrMessageReply reply(true, m_poolDM);
    ChunkedFunctionExecutionResponse* resultProcessor(
        new ChunkedFunctionExecutionResponse(reply, (m_getResult & 2), *m_rc,
                                             m_resultCollectorLock));
    reply.setChunkedResultHandler(resultProcessor);
    reply.setTimeout(m_timeout);
    reply.setDM(m_poolDM);

    LOGDEBUG(
        "ThinClientPoolDM::sendRequestToAllServer sendRequest on endpoint[%s]!",
        m_ep->name().c_str());

    m_error = m_poolDM->sendRequestToEP(request, reply, m_ep);
    m_error = m_poolDM->handleEPError(m_ep, reply, m_error);
    if (m_error != GF_NOERR) {
      if (m_error == GF_NOTCON || m_error == GF_IOERR) {

        delete resultProcessor;
        resultProcessor = nullptr;
        return GF_NOERR;  // if server is unavailable its not an error for
                          // functionexec OnServers() case
      }
      LOGDEBUG(
          "FunctionExecution::execute failed on endpoint[%s]!. Error = %d ",
          m_ep->name().c_str(), m_error);
      if (reply.getMessageType() == TcrMessage::EXCEPTION) {
        exceptionPtr = CacheableString::create(reply.getException());
      }

      delete resultProcessor;
      resultProcessor = nullptr;
      return m_error;
    } else if (reply.getMessageType() == TcrMessage::EXCEPTION ||
               reply.getMessageType() == TcrMessage::EXECUTE_FUNCTION_ERROR) {
      m_error = ThinClientRegion::handleServerException("Execute",
                                                        reply.getException());
      exceptionPtr = CacheableString::create(reply.getException());
    }
    if (resultProcessor->getResult() == true) {

    }
    delete resultProcessor;
    resultProcessor = nullptr;
    return m_error;
  }
};

class OnRegionFunctionExecution : public PooledWork<GfErrType> {
  BucketServerLocationPtr m_serverLocation;
  TcrMessage* m_request;
  TcrMessageReply* m_reply;
  bool m_isBGThread;
  ThinClientPoolDM* m_poolDM;
  const char* m_func;
  uint8_t m_getResult;
  uint32_t m_timeout;
  CacheablePtr m_args;
  CacheableHashSetPtr m_routingObj;
  ResultCollectorPtr m_rc;
  TcrChunkedResult* m_resultCollector;
  std::shared_ptr<ACE_Recursive_Thread_Mutex> m_resultCollectorLock;
  UserAttributesPtr m_userAttr;
  const Region* m_region;
  bool m_allBuckets;

 public:
  OnRegionFunctionExecution(
      const char* func, const Region* region, CacheablePtr args,
      CacheableHashSetPtr routingObj, uint8_t getResult, uint32_t timeout,
      ThinClientPoolDM* poolDM,
      const std::shared_ptr<ACE_Recursive_Thread_Mutex>& rCL,
      ResultCollectorPtr rs, UserAttributesPtr userAttr, bool isBGThread,
      const BucketServerLocationPtr& serverLocation, bool allBuckets)
      : m_serverLocation(serverLocation),
        m_isBGThread(isBGThread),
        m_poolDM(poolDM),
        m_func(func),
        m_getResult(getResult),
        m_timeout(timeout),
        m_args(args),
        m_routingObj(routingObj),
        m_rc(rs),
        m_resultCollectorLock(rCL),
        m_userAttr(userAttr),
        m_region(region),
        m_allBuckets(allBuckets) {
    std::string funcName(m_func);

    m_request = new TcrMessageExecuteRegionFunctionSingleHop(
        m_poolDM->getConnectionManager()
            .getCacheImpl()
            ->getCache()
            ->createDataOutput(),
        funcName, m_region, m_args, m_routingObj, m_getResult, nullptr,
        m_allBuckets, timeout, m_poolDM);
    m_reply = new TcrMessageReply(true, m_poolDM);
    m_resultCollector = new ChunkedFunctionExecutionResponse(
        *m_reply, (m_getResult & 2), m_rc, m_resultCollectorLock);
    m_reply->setChunkedResultHandler(m_resultCollector);
    m_reply->setTimeout(m_timeout);
    m_reply->setDM(m_poolDM);
  }

  ~OnRegionFunctionExecution() {
    delete m_request;
    delete m_reply;
    delete m_resultCollector;
  }

  TcrMessage* getReply() { return m_reply; }

  CacheableHashSetPtr getFailedNode() { return m_reply->getFailedNode(); }

  ChunkedFunctionExecutionResponse* getResultCollector() {
    return static_cast<ChunkedFunctionExecutionResponse*>(m_resultCollector);
  }

  GfErrType execute(void) {
    GuardUserAttribures gua;

    if (m_userAttr != nullptr) gua.setProxyCache(m_userAttr->getProxyCache());

    return m_poolDM->sendSyncRequest(*m_request, *m_reply, !(m_getResult & 1),
                                     m_isBGThread, m_serverLocation);
  }
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_THINCLIENTPOOLDM_H_
