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

#ifndef GEODE_THINCLIENTPOOLHADM_H_
#define GEODE_THINCLIENTPOOLHADM_H_

#include <mutex>

#include "PoolAttributes.hpp"
#include "ThinClientHARegion.hpp"
#include "ThinClientPoolDM.hpp"
#include "ThinClientRedundancyManager.hpp"

namespace apache {
namespace geode {
namespace client {

class TcrConnectionManager;

class ThinClientPoolHADM : public ThinClientPoolDM {
 public:
  ThinClientPoolHADM(const char* name, std::shared_ptr<PoolAttributes> poolAttr,
                     TcrConnectionManager& connManager);

  void init() override;

  ~ThinClientPoolHADM() override { destroy(); }

  GfErrType sendSyncRequest(TcrMessage& request, TcrMessageReply& reply,
                            bool attemptFailover = true,
                            bool isBGThread = false) override;

  bool registerInterestForHARegion(TcrEndpoint* ep, const TcrMessage* request,
                                   ThinClientHARegion& region);

  GfErrType sendSyncRequestRegisterInterestEP(TcrMessage& request,
                                              TcrMessageReply& reply,
                                              bool attemptFailover,
                                              TcrEndpoint* endpoint) override;

  GfErrType registerInterestAllRegions(TcrEndpoint* ep,
                                       const TcrMessage* request,
                                       TcrMessageReply* reply);

  virtual void destroy(bool keepAlive = false) override;

  void readyForEvents();

  void sendNotificationCloseMsgs();

  bool checkDupAndAdd(std::shared_ptr<EventId> eventid) override {
    return m_redundancyManager->checkDupAndAdd(eventid);
  }

  void processMarker() override {
    // also set the static bool m_processedMarker for makePrimary messages
    m_redundancyManager->m_globalProcessedMarker = true;
  }

  void netDown();

  void pingServerLocal() override;

  void acquireRedundancyLock() override {
    m_redundancyManager->acquireRedundancyLock();
  };
  void releaseRedundancyLock() override {
    m_redundancyManager->releaseRedundancyLock();
  };
  std::recursive_mutex& getRedundancyLock() override {
    return m_redundancyManager->getRedundancyLock();
  }

  GfErrType sendRequestToPrimary(TcrMessage& request, TcrMessageReply& reply) {
    return m_redundancyManager->sendRequestToPrimary(request, reply);
  }

  void triggerRedundancyThread() override { m_redundancySema.release(); }

  bool isReadyForEvent() const {
    return m_redundancyManager->isSentReadyForEvents();
  }

 protected:
  GfErrType sendSyncRequestRegisterInterest(
      TcrMessage& request, TcrMessageReply& reply, bool attemptFailover = true,
      ThinClientRegion* region = nullptr,
      TcrEndpoint* endpoint = nullptr) override;

  virtual GfErrType sendSyncRequestCq(TcrMessage& request,
                                      TcrMessageReply& reply);

  virtual bool preFailoverAction();

  virtual bool postFailoverAction(TcrEndpoint* endpoint);

  void startBackgroundThreads() override;

 private:
  // Disallow copy constructor and assignment operator.
  ThinClientRedundancyManager* m_redundancyManager;
  ThinClientPoolHADM(const ThinClientPoolHADM&);
  ThinClientPoolHADM& operator=(const ThinClientPoolHADM&) = delete;

  TcrConnectionManager& m_theTcrConnManager;
  ACE_Semaphore m_redundancySema;
  Task<ThinClientPoolHADM>* m_redundancyTask;

  int redundancy(volatile bool& isRunning);
  /*
  void stopNotificationThreads();
  */
  ExpiryTaskManager::id_type m_servermonitorTaskId;
  int checkRedundancy(const ACE_Time_Value&, const void*);

  TcrEndpoint* createEP(const char* endpointName) override;

  void removeCallbackConnection(TcrEndpoint*) override;

  std::list<ThinClientRegion*> m_regions;
  std::recursive_mutex m_regionsLock;
  void addRegion(ThinClientRegion* theTCR);
  void removeRegion(ThinClientRegion* theTCR);
  void sendNotConMesToAllregions();
  void addDisMessToQueue(ThinClientRegion* theTCR);

  friend class ThinClientHARegion;
  friend class TcrConnectionManager;
  friend class ThinClientRedundancyManager;
  static const char* NC_Redundancy;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_THINCLIENTPOOLHADM_H_
