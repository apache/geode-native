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

#ifndef GEODE_TCRCONNECTIONMANAGER_H_
#define GEODE_TCRCONNECTIONMANAGER_H_

#include <list>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <geode/internal/geode_globals.hpp>

#include "ExpiryTaskManager.hpp"
#include "Queue.hpp"
#include "Task.hpp"
#include "ThinClientRedundancyManager.hpp"
#include "util/synchronized_map.hpp"

namespace apache {
namespace geode {
namespace client {

class TcrConnection;
class TcrEndpoint;
class TcrMessage;
class CacheImpl;
class ThinClientBaseDM;
class ThinClientRegion;

/**
 * @brief transport data between caches
 */
class TcrConnectionManager {
 public:
  using endpoint_map_type = synchronized_map<
      std::unordered_map<std::string, std::shared_ptr<TcrEndpoint>>,
      std::recursive_mutex>;

  explicit TcrConnectionManager(CacheImpl* cache);
  ~TcrConnectionManager();
  void init(bool isPool = false);
  void startFailoverAndCleanupThreads(bool isPool = false);
  void connect(ThinClientBaseDM* distMng, std::vector<TcrEndpoint*>& endpoints,
               const std::unordered_set<std::string>& endpointStrs);
  void disconnect(ThinClientBaseDM* distMng,
                  std::vector<TcrEndpoint*>& endpoints,
                  bool keepEndpoints = false);
  int checkConnection(const ACE_Time_Value&, const void*);
  int checkRedundancy(const ACE_Time_Value&, const void*);
  int processEventIdMap(const ACE_Time_Value&, const void*);
  ExpiryTaskManager::id_type getPingTaskId();
  void close();

  void readyForEvents();

  // added netDown() and revive() for tests simulation of client crash and
  // network drop
  void netDown();
  void revive();
  void setClientCrashTEST() { TEST_DURABLE_CLIENT_CRASH = true; }
  volatile static bool TEST_DURABLE_CLIENT_CRASH;

  inline endpoint_map_type& getGlobalEndpoints() { return m_endpoints; }

  void getAllEndpoints(std::vector<TcrEndpoint*>& endpoints);
  int getNumEndPoints();

  GfErrType registerInterestAllRegions(TcrEndpoint* ep,
                                       const TcrMessage* request,
                                       TcrMessageReply* reply);
  GfErrType sendSyncRequestCq(TcrMessage& request, TcrMessageReply& reply);

  void addNotificationForDeletion(Task<TcrEndpoint>* notifyReceiver,
                                  TcrConnection* notifyConnection,
                                  ACE_Semaphore& notifyCleanupSema);

  void processMarker();

  bool getEndpointStatus(const std::string& endpoint);

  bool isDurable() { return m_isDurable; }
  bool haEnabled() { return m_redundancyManager->m_HAenabled; }
  CacheImpl* getCacheImpl() const { return m_cache; }

  GfErrType sendSyncRequestCq(TcrMessage& request, TcrMessageReply& reply,
                              TcrHADistributionManager* theHADM);
  GfErrType sendSyncRequestRegisterInterest(
      TcrMessage& request, TcrMessageReply& reply, bool attemptFailover = true,
      TcrEndpoint* endpoint = nullptr,
      TcrHADistributionManager* theHADM = nullptr,
      ThinClientRegion* region = nullptr);

  inline void triggerRedundancyThread() { m_redundancySema.release(); }

  inline void acquireRedundancyLock() {
    m_redundancyManager->acquireRedundancyLock();
    m_distMngrsLock.lock();
  }

  inline void releaseRedundancyLock() {
    m_redundancyManager->releaseRedundancyLock();
    m_distMngrsLock.unlock();
  }

  bool checkDupAndAdd(std::shared_ptr<EventId> eventid) {
    return m_redundancyManager->checkDupAndAdd(eventid);
  }

  std::recursive_mutex& getRedundancyLock() {
    return m_redundancyManager->getRedundancyLock();
  }

  GfErrType sendRequestToPrimary(TcrMessage& request, TcrMessageReply& reply) {
    return m_redundancyManager->sendRequestToPrimary(request, reply);
  }

  bool isNetDown() const { return m_isNetDown; }

 private:
  CacheImpl* m_cache;
  volatile bool m_initGuard;
  endpoint_map_type m_endpoints;

  // key is hostname:port
  std::list<ThinClientBaseDM*> m_distMngrs;
  std::recursive_mutex m_distMngrsLock;

  ACE_Semaphore m_failoverSema;
  std::unique_ptr<Task<TcrConnectionManager>> m_failoverTask;

  bool removeRefToEndpoint(TcrEndpoint* ep, bool keepEndpoint = false);
  TcrEndpoint* addRefToTcrEndpoint(std::string endpointName,
                                   ThinClientBaseDM* dm = nullptr);

  void initializeHAEndpoints(const char* endpointsStr);
  void removeHAEndpoints();

  ACE_Semaphore m_cleanupSema;
  std::unique_ptr<Task<TcrConnectionManager>> m_cleanupTask;

  ExpiryTaskManager::id_type m_pingTaskId;
  ExpiryTaskManager::id_type m_servermonitorTaskId;
  Queue<Task<TcrEndpoint>*> m_receiverReleaseList;
  Queue<TcrConnection*> m_connectionReleaseList;
  Queue<ACE_Semaphore*> m_notifyCleanupSemaList;

  ACE_Semaphore m_redundancySema;
  std::unique_ptr<Task<TcrConnectionManager>> m_redundancyTask;
  std::recursive_mutex m_notificationLock;
  bool m_isDurable;

  bool m_isNetDown;

  std::unique_ptr<ThinClientRedundancyManager> m_redundancyManager;

  void failover(std::atomic<bool>& isRunning);
  void redundancy(std::atomic<bool>& isRunning);

  void cleanNotificationLists();
  void cleanup(std::atomic<bool>& isRunning);

  // Disallow copy constructor and assignment operator.
  TcrConnectionManager(const TcrConnectionManager&);
  TcrConnectionManager& operator=(const TcrConnectionManager&);

  friend class ThinClientRedundancyManager;
  friend class DistManagersLockGuard;
  friend class ThinClientPoolDM;
  friend class ThinClientPoolHADM;
  static const char* NC_Redundancy;
  static const char* NC_Failover;
  static const char* NC_CleanUp;
};

// Guard class to acquire/release distManagers lock
class DistManagersLockGuard {
 private:
  TcrConnectionManager& m_tccm;

 public:
  explicit DistManagersLockGuard(TcrConnectionManager& tccm) : m_tccm(tccm) {
    m_tccm.m_distMngrsLock.lock();
  }

  ~DistManagersLockGuard() { m_tccm.m_distMngrsLock.unlock(); }
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TCRCONNECTIONMANAGER_H_
