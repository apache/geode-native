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

#ifndef GEODE_THINCLIENTREDUNDANCYMANAGER_H_
#define GEODE_THINCLIENTREDUNDANCYMANAGER_H_

#include <atomic>
#include <chrono>
#include <list>
#include <memory>
#include <mutex>
#include <set>
#include <string>

#include "ErrType.hpp"
#include "EventIdMap.hpp"
#include "ExpiryTaskManager.hpp"
#include "ServerLocation.hpp"
#include "Task.hpp"
#include "TcrMessage.hpp"
#include "util/synchronized_map.hpp"

namespace apache {
namespace geode {
namespace client {

class TcrConnectionManager;
class TcrHADistributionManager;
class ThinClientRegion;
class ThinClientPoolHADM;
class TcrEndpoint;

class ThinClientRedundancyManager {
 public:
  bool m_globalProcessedMarker;

  GfErrType maintainRedundancyLevel(bool init = false,
                                    const TcrMessage* request = nullptr,
                                    TcrMessageReply* reply = nullptr,
                                    ThinClientRegion* region = nullptr);
  void initialize(int redundancyLevel);
  void close();
  void sendNotificationCloseMsgs();

  explicit ThinClientRedundancyManager(TcrConnectionManager* theConnManager,
                                       int redundencyLevel = 0,
                                       ThinClientPoolHADM* poolHADM = nullptr,
                                       bool sentReadyForEvents = false,
                                       bool globalProcessedMarker = false);
  GfErrType sendSyncRequestRegisterInterest(TcrMessage& request,
                                            TcrMessageReply& reply,
                                            bool attemptFailover,
                                            TcrEndpoint* endpoint,
                                            ThinClientBaseDM* theHADM,
                                            ThinClientRegion* region = nullptr);

  GfErrType sendSyncRequestCq(TcrMessage& request, TcrMessageReply& reply,
                              ThinClientBaseDM* theHADM);
  void readyForEvents();
  void startPeriodicAck();
  bool checkDupAndAdd(std::shared_ptr<EventId> eventid);
  void netDown();
  void acquireRedundancyLock() { m_redundantEndpointsLock.lock(); }
  void releaseRedundancyLock() { m_redundantEndpointsLock.unlock(); }
  bool allEndPointDiscon() { return m_IsAllEpDisCon; }
  void removeCallbackConnection(TcrEndpoint*);

  std::recursive_mutex& getRedundancyLock() { return m_redundantEndpointsLock; }

  GfErrType sendRequestToPrimary(TcrMessage& request, TcrMessageReply& reply);
  bool isSentReadyForEvents() const { return m_sentReadyForEvents; }

 private:
  using clock = std::chrono::steady_clock;
  using time_point = clock::time_point;

  // for selectServers
  volatile bool m_IsAllEpDisCon;
  int m_server;
  bool m_sentReadyForEvents;
  int m_redundancyLevel;
  bool m_loggedRedundancyWarning;
  ThinClientPoolHADM* m_poolHADM;
  std::vector<TcrEndpoint*> m_redundantEndpoints;
  std::vector<TcrEndpoint*> m_nonredundantEndpoints;
  std::recursive_mutex m_redundantEndpointsLock;
  TcrConnectionManager* m_theTcrConnManager;
  std::shared_ptr<CacheableStringArray> m_locators;
  std::shared_ptr<CacheableStringArray> m_servers;

  void removeEndpointsInOrder(std::vector<TcrEndpoint*>& destVector,
                              const std::vector<TcrEndpoint*>& srcVector);
  void addEndpointsInOrder(std::vector<TcrEndpoint*>& destVector,
                           const std::vector<TcrEndpoint*>& srcVector);
  GfErrType makePrimary(TcrEndpoint* ep, const TcrMessage* request,
                        TcrMessageReply* reply);
  GfErrType makeSecondary(TcrEndpoint* ep, const TcrMessage* request,
                          TcrMessageReply* reply);
  bool sendMakePrimaryMesg(TcrEndpoint* ep, const TcrMessage* request,
                           ThinClientRegion* region);
  bool readyForEvents(TcrEndpoint* primaryCandidate);
  void moveEndpointToLast(std::vector<TcrEndpoint*>& epVector,
                          TcrEndpoint* targetEp);

  synchronized_map<
      std::unordered_map<std::string, std::shared_ptr<TcrEndpoint>>,
      std::recursive_mutex>&
  updateAndSelectEndpoints();

  void getAllEndpoints(std::vector<TcrEndpoint*>& endpoints);
  // For 38196 Fix: Reorder End points.
  void insertEPInQueueSizeOrder(TcrEndpoint* ep,
                                std::vector<TcrEndpoint*>& endpoints);

  GfErrType createQueueEP(TcrEndpoint* ep, const TcrMessage* request,
                          TcrMessageReply* reply, bool isPrimary);
  GfErrType createPoolQueueEP(TcrEndpoint* ep, const TcrMessage* request,
                              TcrMessageReply* reply, bool isPrimary);

  inline bool isDurable();
  int processEventIdMap(const ACE_Time_Value&, const void*);
  std::unique_ptr<Task<ThinClientRedundancyManager>> m_periodicAckTask;
  ACE_Semaphore m_periodicAckSema;
  ExpiryTaskManager::id_type
      m_processEventIdMapTaskId;  // periodic check eventid map for notify ack
                                  // and/or expiry
  void periodicAck(std::atomic<bool>& isRunning);
  void doPeriodicAck();
  time_point m_nextAck;                    // next ack time
  std::chrono::milliseconds m_nextAckInc;  // next ack time increment
  volatile bool m_HAenabled;
  EventIdMap m_eventidmap;

  std::list<ServerLocation> selectServers(int howMany,
                                          std::set<ServerLocation> exclEndPts);

  friend class TcrConnectionManager;
  static const char* NC_PerodicACK;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_THINCLIENTREDUNDANCYMANAGER_H_
