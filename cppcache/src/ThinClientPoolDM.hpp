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

#ifndef GEODE_THINCLIENTPOOLDM_H_
#define GEODE_THINCLIENTPOOLDM_H_

#include <chrono>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include <ace/Semaphore.h>
#include <ace/Time_Value.h>

#include <geode/Pool.hpp>
#include <geode/ResultCollector.hpp>

#include "ConnectionQueue.hpp"
#include "ExecutionImpl.hpp"
#include "PoolAttributes.hpp"
#include "PoolStatistics.hpp"
#include "RemoteQueryService.hpp"
#include "TXState.hpp"
#include "Task.hpp"
#include "TcrPoolEndPoint.hpp"
#include "ThinClientBaseDM.hpp"
#include "ThinClientLocatorHelper.hpp"
#include "ThinClientRegion.hpp"
#include "ThinClientStickyManager.hpp"
#include "ThreadPool.hpp"
#include "UserAttributes.hpp"

namespace apache {
namespace geode {

namespace statistics {

class PoolStatsSampler;

}  // namespace statistics

namespace client {

class CacheImpl;
class FunctionExecution;
class ClientMetadataService;

class ThinClientPoolDM
    : public ThinClientBaseDM,
      public Pool,
      public ConnectionQueue<TcrConnection, std::recursive_mutex> {
 public:
  ThinClientPoolDM(const ThinClientPoolDM&) = delete;
  ThinClientPoolDM& operator=(const ThinClientPoolDM&) = delete;
  ThinClientPoolDM(const char* name, std::shared_ptr<PoolAttributes> poolAttrs,
                   TcrConnectionManager& connManager);

  void init() override;

  const std::string& getName() const override { return m_poolName; }

  GfErrType sendSyncRequest(TcrMessage& request, TcrMessageReply& reply,
                            bool attemptFailover = true,
                            bool isBGThread = false) override;
  virtual GfErrType sendSyncRequest(
      TcrMessage& request, TcrMessageReply& reply, bool attemptFailover,
      bool isBGThread,
      const std::shared_ptr<BucketServerLocation>& serverLocation);

  // Pool Specific Fns.
  const std::shared_ptr<CacheableStringArray> getLocators() const override;
  const std::shared_ptr<CacheableStringArray> getServers() override;
  void destroy(bool keepalive = false) override;
  bool isDestroyed() const override;
  std::shared_ptr<QueryService> getQueryService() override;
  virtual std::shared_ptr<QueryService> getQueryServiceWithoutCheck();
  bool isEndpointAttached(TcrEndpoint* ep) override;
  GfErrType sendRequestToAllServers(
      const char* func, uint8_t getResult, std::chrono::milliseconds timeout,
      std::shared_ptr<Cacheable> args, std::shared_ptr<ResultCollector>& rs,
      std::shared_ptr<CacheableString>& exceptionPtr);

  GfErrType sendRequestToEP(const TcrMessage& request, TcrMessageReply& reply,
                            TcrEndpoint* currentEndpoint) override;
  void addConnection(TcrConnection* conn);

  std::shared_ptr<TcrEndpoint> addEP(ServerLocation& serverLoc);

  std::shared_ptr<TcrEndpoint> addEP(const std::string& endpointName);
  virtual void pingServer(std::atomic<bool>& isRunning);
  virtual void updateLocatorList(std::atomic<bool>& isRunning);
  virtual void cliCallback(std::atomic<bool>& isRunning);
  virtual void pingServerLocal();

  ~ThinClientPoolDM() override;

  // void updateQueue(const char* regionPath) ;
  ClientProxyMembershipID* getMembershipId() { return m_memId.get(); }
  virtual void processMarker() {}
  bool checkDupAndAdd(std::shared_ptr<EventId> eventid) override;
  std::recursive_mutex& getPoolLock() { return mutex_; }
  void reducePoolSize(int num);
  void removeEPConnections(int numConn, bool triggerManagerConn = true);
  void removeEPConnections(TcrEndpoint* ep);
  GfErrType createPoolConnection(TcrConnection*& conn,
                                 std::set<ServerLocation>& excludeServers,
                                 bool& maxConnLimit,
                                 const TcrConnection* currentServer = nullptr);
  ThinClientLocatorHelper* getLocatorHelper() { return m_locHelper; }
  void releaseThreadLocalConnection() override;
  virtual void setThreadLocalConnection(TcrConnection* conn);
  bool excludeConnection(TcrConnection*, std::set<ServerLocation>&);
  void incRegionCount();
  void decRegionCount();

  virtual void setStickyNull(bool isBGThread) {
    if (!isBGThread) m_manager->setStickyConnection(nullptr, false);
  }

  virtual bool canItBeDeletedNoImpl(TcrConnection* conn);

  void updateNotificationStats(bool isDeltaSuccess,
                               std::chrono::nanoseconds timeInNanoSecond);

  bool isSecurityOn() override { return m_isSecurityOn || m_isMultiUserMode; }

  bool isMultiUserMode() override { return m_isMultiUserMode; }

  virtual void sendUserCacheCloseMessage(bool keepAlive);

  virtual inline PoolStats& getStats() { return *m_stats; }

  size_t getNumberOfEndPoints() const override { return m_endpoints.size(); }

  int32_t GetPDXIdForType(std::shared_ptr<Serializable> pdxType);

  std::shared_ptr<Serializable> GetPDXTypeById(int32_t typeId);

  void AddPdxType(std::shared_ptr<Serializable> pdxType, int32_t pdxTypeId);

  int32_t GetEnumValue(std::shared_ptr<Serializable> enumInfo);
  std::shared_ptr<Serializable> GetEnum(int32_t val);
  void AddEnum(std::shared_ptr<Serializable> enumInfo, int enumVal);

  // Tries to get connection to a endpoint. If no connection is available, it
  // tries
  // to create one. If it fails to create one,  it returns a connection to any
  // other
  // server after failing over the transaction to that server
  GfErrType getConnectionToAnEndPoint(std::string epNameStr,
                                      TcrConnection*& conn);

  const std::string getSNIProxyHostname() { return m_attrs->getSniProxyHost(); }
  uint16_t getSNIPort() { return m_attrs->getSniProxyPort(); }
  virtual inline bool isSticky() { return m_sticky; }
  virtual TcrEndpoint* getEndPoint(
      const std::shared_ptr<BucketServerLocation>& serverLocation,
      int8_t& version, std::set<ServerLocation>& excludeServers);

  ClientMetadataService* getClientMetaDataService() {
    return m_clientMetadataService.get();
  }
  void setPrimaryServerQueueSize(int queueSize) {
    m_primaryServerQueueSize = queueSize;
  }
  int getPrimaryServerQueueSize() const { return m_primaryServerQueueSize; }

 protected:
  ThinClientStickyManager* m_manager;
  std::vector<std::string> m_canonicalHosts;
  synchronized_map<
      std::unordered_map<std::string, std::shared_ptr<TcrEndpoint>>,
      std::recursive_mutex>
      m_endpoints;
  std::recursive_mutex m_endpointsLock;
  std::recursive_mutex m_endpointSelectionLock;
  std::string m_poolName;
  PoolStats* m_stats;
  bool m_sticky;
  void netDown();
  ACE_Semaphore m_updateLocatorListSema;
  ACE_Semaphore m_pingSema;
  ACE_Semaphore m_cliCallbackSema;
  volatile bool m_isDestroyed;
  volatile bool m_destroyPending;
  volatile bool m_destroyPendingHADM;
  void checkRegions();
  std::shared_ptr<RemoteQueryService> m_remoteQueryServicePtr;
  virtual void startBackgroundThreads();
  virtual void stopPingThread();
  virtual void stopUpdateLocatorListThread();
  virtual void stopCliCallbackThread();
  virtual void cleanStickyConnections(std::atomic<bool>& isRunning);
  virtual TcrConnection* getConnectionFromQueue(bool timeout, GfErrType* error,
                                                std::set<ServerLocation>&,
                                                bool& maxConnLimit);

  virtual void putInQueue(TcrConnection* conn, bool isBGThread,
                          bool isTransaction = false);

  GfErrType doFailover(TcrConnection* conn);

  virtual bool canItBeDeleted(TcrConnection* conn);

  virtual TcrConnection* getConnectionFromQueueW(
      GfErrType* error, std::set<ServerLocation>& excludeServers,
      bool isBGThread, TcrMessage& request, int8_t& version, bool& match,
      bool& connFound,
      const std::shared_ptr<BucketServerLocation>& serverLocation = nullptr);

  TcrConnection* getFromEP(TcrEndpoint* theEP);
  virtual TcrEndpoint* getSingleHopServer(
      TcrMessage& request, int8_t& version,
      std::shared_ptr<BucketServerLocation>& serverLocation,
      std::set<ServerLocation>& excludeServers);
  // Create pool connection to a specified endpoint.
  GfErrType createPoolConnectionToAEndPoint(TcrConnection*& conn,
                                            TcrEndpoint* theEP,
                                            bool& maxConnLimit,
                                            bool appThreadrequest = false);

 private:
  bool hasExpired(TcrConnection* conn);

  std::shared_ptr<Properties> getCredentials(TcrEndpoint* ep);
  GfErrType sendUserCredentials(std::shared_ptr<Properties> credentials,
                                TcrConnection*& conn, bool isBGThread,
                                bool& isServerException);

  // get endpoint using the endpoint string
  std::shared_ptr<TcrEndpoint> getEndpoint(const std::string& epNameStr);

  bool m_isSecurityOn;
  bool m_isMultiUserMode;

  TcrConnection* getUntil(std::chrono::microseconds& sec, GfErrType* error,
                          std::set<ServerLocation>& excludeServers,
                          bool& maxConnLimit) {
    bool isClosed;
    TcrConnection* mp =
        getNoGetLock(isClosed, error, excludeServers, maxConnLimit);

    if (mp == nullptr && !isClosed) {
      mp = getLockedFor(sec, isClosed, &excludeServers);
    }

    return mp;
  }

  TcrConnection* getNoGetLock(bool& isClosed, GfErrType* error,
                              std::set<ServerLocation>& excludeServers,
                              bool& maxConnLimit);
  bool exclude(TcrConnection* conn, std::set<ServerLocation>& excludeServers);
  void deleteAction() override { removeEPConnections(1); }

  std::string selectEndpoint(std::set<ServerLocation>&,
                             const TcrConnection* currentServer = nullptr);
  // TODO global - m_memId was volatile
  std::unique_ptr<ClientProxyMembershipID> m_memId;
  virtual std::shared_ptr<TcrEndpoint> createEP(const char* endpointName);
  virtual void removeCallbackConnection(TcrEndpoint*) {}

  bool excludeServer(std::string, std::set<ServerLocation>&);

  ThinClientLocatorHelper* m_locHelper;

  std::atomic<int32_t> m_poolSize;  // Actual Size of Pool
  int m_numRegions;

  // for selectEndpoint
  unsigned m_server;

  // Manage Connection thread
  ACE_Semaphore m_connSema;
  std::unique_ptr<Task<ThinClientPoolDM>> m_connManageTask;
  std::unique_ptr<Task<ThinClientPoolDM>> m_pingTask;
  std::unique_ptr<Task<ThinClientPoolDM>> m_updateLocatorListTask;
  std::unique_ptr<Task<ThinClientPoolDM>> m_cliCallbackTask;
  ExpiryTaskManager::id_type m_pingTaskId;
  ExpiryTaskManager::id_type m_updateLocatorListTaskId;
  ExpiryTaskManager::id_type m_connManageTaskId;
  void manageConnections(std::atomic<bool>& isRunning);
  int doPing(const ACE_Time_Value&, const void*);
  int doUpdateLocatorList(const ACE_Time_Value&, const void*);
  int doManageConnections(const ACE_Time_Value&, const void*);
  void manageConnectionsInternal(std::atomic<bool>& isRunning);
  void cleanStaleConnections(std::atomic<bool>& isRunning);
  void restoreMinConnections(std::atomic<bool>& isRunning);
  std::atomic<int32_t> m_clientOps;  // Actual Size of Pool
  std::unique_ptr<statistics::PoolStatsSampler> m_PoolStatsSampler;
  std::unique_ptr<ClientMetadataService> m_clientMetadataService;
  friend class CacheImpl;
  friend class ThinClientStickyManager;
  friend class FunctionExecution;
  static const char* NC_Ping_Thread;
  static const char* NC_MC_Thread;
  int m_primaryServerQueueSize;
  void removeEPFromMetadataIfError(const GfErrType& error,
                                   const TcrEndpoint* ep);
};

class FunctionExecution : public PooledWork<GfErrType> {
  ThinClientPoolDM* m_poolDM;
  TcrEndpoint* m_ep;
  const char* m_func;
  uint8_t m_getResult;
  std::chrono::milliseconds m_timeout;
  std::shared_ptr<Cacheable> m_args;
  GfErrType m_error;
  std::shared_ptr<ResultCollector>* m_rc;
  std::shared_ptr<std::recursive_mutex> m_resultCollectorLock;
  std::shared_ptr<CacheableString> exceptionPtr;
  std::shared_ptr<UserAttributes> m_userAttr;

 public:
  FunctionExecution() {
    m_poolDM = nullptr;
    m_ep = nullptr;
    m_func = nullptr;
    m_getResult = 0;
    m_error = GF_NOERR;
    m_rc = nullptr;
    m_resultCollectorLock = nullptr;
    m_userAttr = nullptr;
  }

  ~FunctionExecution() {}

  std::shared_ptr<CacheableString> getException() { return exceptionPtr; }

  void setParameters(const char* func, uint8_t getResult,
                     std::chrono::milliseconds timeout,
                     std::shared_ptr<Cacheable> args, TcrEndpoint* ep,
                     ThinClientPoolDM* poolDM,
                     const std::shared_ptr<std::recursive_mutex>& rCL,
                     std::shared_ptr<ResultCollector>* rs,
                     std::shared_ptr<UserAttributes> userAttr) {
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
  }

  GfErrType execute(void);
};

class OnRegionFunctionExecution : public PooledWork<GfErrType> {
  std::shared_ptr<BucketServerLocation> m_serverLocation;
  TcrMessage* m_request;
  TcrMessageReply* m_reply;
  bool m_isBGThread;
  ThinClientPoolDM* m_poolDM;
  std::string m_func;
  uint8_t m_getResult;
  std::chrono::milliseconds m_timeout;
  std::shared_ptr<Cacheable> m_args;
  std::shared_ptr<CacheableHashSet> m_routingObj;
  std::shared_ptr<ResultCollector> m_rc;
  TcrChunkedResult* m_resultCollector;
  std::shared_ptr<std::recursive_mutex> m_resultCollectorLock;
  std::shared_ptr<UserAttributes> m_userAttr;
  const Region* m_region;
  bool m_allBuckets;

 public:
  OnRegionFunctionExecution(
      std::string func, const Region* region, std::shared_ptr<Cacheable> args,
      std::shared_ptr<CacheableHashSet> routingObj, uint8_t getResult,
      std::chrono::milliseconds timeout, ThinClientPoolDM* poolDM,
      const std::shared_ptr<std::recursive_mutex>& rCL,
      std::shared_ptr<ResultCollector> rs,
      std::shared_ptr<UserAttributes> userAttr, bool isBGThread,
      const std::shared_ptr<BucketServerLocation>& serverLocation,
      bool allBuckets);

  ~OnRegionFunctionExecution() {
    delete m_request;
    delete m_reply;
    delete m_resultCollector;
  }

  TcrMessage* getReply() { return m_reply; }

  std::shared_ptr<CacheableHashSet> getFailedNode() {
    return m_reply->getFailedNode();
  }

  ChunkedFunctionExecutionResponse* getResultCollector() {
    return static_cast<ChunkedFunctionExecutionResponse*>(m_resultCollector);
  }

  GfErrType execute(void) {
    GuardUserAttributes gua;

    if (m_userAttr) {
      gua.setAuthenticatedView(m_userAttr->getAuthenticatedView());
    }

    return m_poolDM->sendSyncRequest(*m_request, *m_reply, !(m_getResult & 1),
                                     m_isBGThread, m_serverLocation);
  }
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_THINCLIENTPOOLDM_H_
