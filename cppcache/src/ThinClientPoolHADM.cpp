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

#include "ThinClientPoolHADM.hpp"

#include <geode/SystemProperties.hpp>

#include "ExpiryHandler_T.hpp"
#include "TcrConnectionManager.hpp"
#include "util/exception.hpp"

namespace apache {
namespace geode {
namespace client {
const char* ThinClientPoolHADM::NC_Redundancy = "NC Redundancy";
ThinClientPoolHADM::ThinClientPoolHADM(const char* name,
                                       std::shared_ptr<PoolAttributes> poolAttr,
                                       TcrConnectionManager& connManager)
    : ThinClientPoolDM(name, poolAttr, connManager),
      m_theTcrConnManager(connManager),
      m_redundancySema(0),
      m_redundancyTask(nullptr),
      m_servermonitorTaskId(-1) {
  m_redundancyManager = std::unique_ptr<ThinClientRedundancyManager>(
      new ThinClientRedundancyManager(
          &connManager, poolAttr->getSubscriptionRedundancy(), this));
}

void ThinClientPoolHADM::init() {
  // Pool DM should only be inited once.
  ThinClientPoolDM::init();

  startBackgroundThreads();
}

void ThinClientPoolHADM::startBackgroundThreads() {
  auto& props = m_connManager.getCacheImpl()
                    ->getDistributedSystem()
                    .getSystemProperties();

  m_redundancyManager->initialize(m_attrs->getSubscriptionRedundancy());
  //  Call maintain redundancy level, so primary is available for notification
  //  operations.
  GfErrType err = m_redundancyManager->maintainRedundancyLevel(true);

  ACE_Event_Handler* redundancyChecker =
      new ExpiryHandler_T<ThinClientPoolHADM>(
          this, &ThinClientPoolHADM::checkRedundancy);
  const auto redundancyMonitorInterval = props.redundancyMonitorInterval();

  m_servermonitorTaskId =
      m_connManager.getCacheImpl()->getExpiryTaskManager().scheduleExpiryTask(
          redundancyChecker, std::chrono::seconds(1), redundancyMonitorInterval,
          false);
  LOGFINE(
      "ThinClientPoolHADM::ThinClientPoolHADM Registered server "
      "monitor task with id = %ld, interval = %ld",
      m_servermonitorTaskId, redundancyMonitorInterval.count());

  if (ThinClientBaseDM::isFatalClientError(err)) {
    if (err == GF_CACHE_LOCATOR_EXCEPTION) {
      LOGWARN(
          "No locators were available during pool initialization with "
          "subscription redundancy.");
    } else {
      throwExceptionIfError("ThinClientPoolHADM::init", err);
    }
  }

  m_redundancyManager->startPeriodicAck();
  m_redundancyTask =
      std::unique_ptr<Task<ThinClientPoolHADM>>(new Task<ThinClientPoolHADM>(
          this, &ThinClientPoolHADM::redundancy, NC_Redundancy));
  m_redundancyTask->start();
}

GfErrType ThinClientPoolHADM::sendSyncRequest(TcrMessage& request,
                                              TcrMessageReply& reply,
                                              bool attemptFailover,
                                              bool isBGThread) {
  GfErrType err = GF_NOERR;

  int32_t type = request.getMessageType();

  if ((type == TcrMessage::EXECUTECQ_MSG_TYPE ||
       type == TcrMessage::STOPCQ_MSG_TYPE ||
       type == TcrMessage::CLOSECQ_MSG_TYPE ||
       type == TcrMessage::CLOSECLIENTCQS_MSG_TYPE ||
       type == TcrMessage::GETCQSTATS_MSG_TYPE ||
       type == TcrMessage::MONITORCQ_MSG_TYPE ||
       type == TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE ||
       type == TcrMessage::GETDURABLECQS_MSG_TYPE)) {
    if (m_destroyPending) return GF_NOERR;
    reply.setDM(this);
    err = sendSyncRequestCq(request, reply);
  } else {
    err = ThinClientPoolDM::sendSyncRequest(request, reply, attemptFailover,
                                            isBGThread);
  }
  return err;
}

bool ThinClientPoolHADM::registerInterestForHARegion(
    TcrEndpoint* ep, const TcrMessage* request, ThinClientHARegion& region) {
  return (region.registerKeys(ep, request) == GF_NOERR);
}

GfErrType ThinClientPoolHADM::sendSyncRequestRegisterInterestEP(
    TcrMessage& request, TcrMessageReply& reply, bool attemptFailover,
    TcrEndpoint* endpoint) {
  return ThinClientBaseDM::sendSyncRequestRegisterInterest(
      request, reply, attemptFailover, nullptr, endpoint);
}

GfErrType ThinClientPoolHADM::sendSyncRequestRegisterInterest(
    TcrMessage& request, TcrMessageReply& reply, bool attemptFailover,
    ThinClientRegion* region, TcrEndpoint* endpoint) {
  return m_redundancyManager->sendSyncRequestRegisterInterest(
      request, reply, attemptFailover, endpoint, this, region);
}

GfErrType ThinClientPoolHADM::sendSyncRequestCq(TcrMessage& request,
                                                TcrMessageReply& reply) {
  return m_redundancyManager->sendSyncRequestCq(request, reply, this);
}

bool ThinClientPoolHADM::preFailoverAction() { return true; }

bool ThinClientPoolHADM::postFailoverAction(TcrEndpoint*) {
  m_connManager.triggerRedundancyThread();
  return true;
}

void ThinClientPoolHADM::redundancy(std::atomic<bool>& isRunning) {
  LOGFINE("ThinClientPoolHADM: Starting maintain redundancy thread.");
  while (isRunning) {
    m_redundancySema.acquire();
    if (isRunning && !m_connManager.isNetDown()) {
      m_redundancyManager->maintainRedundancyLevel();
      while (m_redundancySema.tryacquire() != -1) {
        ;
      }
    }
  }
  LOGFINE("ThinClientPoolHADM: Ending maintain redundancy thread.");
}

int ThinClientPoolHADM::checkRedundancy(const ACE_Time_Value&, const void*) {
  m_redundancySema.release();
  return 0;
}

void ThinClientPoolHADM::destroy(bool keepAlive) {
  LOGDEBUG("ThinClientPoolHADM::destroy");
  if (!m_isDestroyed && !m_destroyPending) {
    checkRegions();

    if (m_remoteQueryServicePtr != nullptr) {
      m_remoteQueryServicePtr->close();
      m_remoteQueryServicePtr = nullptr;
    }

    stopPingThread();

    sendNotificationCloseMsgs();

    m_redundancyManager->close();

    m_destroyPendingHADM = true;
    ThinClientPoolDM::destroy(keepAlive);
  }
}

void ThinClientPoolHADM::sendNotificationCloseMsgs() {
  if (m_redundancyTask) {
    if (m_servermonitorTaskId >= 0) {
      m_connManager.getCacheImpl()->getExpiryTaskManager().cancelTask(
          m_servermonitorTaskId);
    }
    m_redundancyTask->stopNoblock();
    m_redundancySema.release();
    m_redundancyTask->wait();
    m_redundancyTask = nullptr;
    m_redundancyManager->sendNotificationCloseMsgs();
  }
}

GfErrType ThinClientPoolHADM::registerInterestAllRegions(
    TcrEndpoint* ep, const TcrMessage* request, TcrMessageReply* reply) {
  GfErrType err = GF_NOERR;
  GfErrType opErr = GF_NOERR;

  std::lock_guard<decltype(m_regionsLock)> guard(m_regionsLock);
  for (std::list<ThinClientRegion*>::iterator itr = m_regions.begin();
       itr != m_regions.end(); itr++) {
    if ((opErr = (*itr)->registerKeys(ep, request, reply)) != GF_NOERR) {
      if (err == GF_NOERR) {
        err = opErr;
      }
    }
  }
  return err;
}

bool ThinClientPoolHADM::checkDupAndAdd(std::shared_ptr<EventId> eventid) {
  return m_redundancyManager->checkDupAndAdd(eventid);
}

void ThinClientPoolHADM::processMarker() {
  // also set the static bool m_processedMarker for makePrimary messages
  m_redundancyManager->m_globalProcessedMarker = true;
}

void ThinClientPoolHADM::acquireRedundancyLock() {
  m_redundancyManager->acquireRedundancyLock();
}

void ThinClientPoolHADM::releaseRedundancyLock() {
  m_redundancyManager->releaseRedundancyLock();
}

std::recursive_mutex& ThinClientPoolHADM::getRedundancyLock() {
  return m_redundancyManager->getRedundancyLock();
}

GfErrType ThinClientPoolHADM::sendRequestToPrimary(TcrMessage& request,
                                                   TcrMessageReply& reply) {
  return m_redundancyManager->sendRequestToPrimary(request, reply);
}

bool ThinClientPoolHADM::isReadyForEvent() const {
  return m_redundancyManager->isSentReadyForEvents();
}

void ThinClientPoolHADM::addRegion(ThinClientRegion* theTCR) {
  std::lock_guard<decltype(m_regionsLock)> guard(m_regionsLock);
  m_regions.push_back(theTCR);
}
void ThinClientPoolHADM::addDisMessToQueue(ThinClientRegion* theTCR) {
  std::lock_guard<decltype(m_regionsLock)> guard(m_regionsLock);
  if (m_redundancyManager->allEndPointDiscon()) {
    theTCR->receiveNotification(TcrMessage::getAllEPDisMess());
  }
}
void ThinClientPoolHADM::removeRegion(ThinClientRegion* theTCR) {
  std::lock_guard<decltype(m_regionsLock)> guard(m_regionsLock);
  for (std::list<ThinClientRegion*>::iterator itr = m_regions.begin();
       itr != m_regions.end(); itr++) {
    if (*itr == theTCR) {
      m_regions.erase(itr);
      return;
    }
  }
}

void ThinClientPoolHADM::readyForEvents() {
  auto& sysProp = m_connManager.getCacheImpl()
                      ->getDistributedSystem()
                      .getSystemProperties();
  if (!sysProp.autoReadyForEvents()) {
    init();
  }

  auto&& durable = sysProp.durableClientId();
  if (!durable.empty()) {
    m_redundancyManager->readyForEvents();
  }
}

void ThinClientPoolHADM::netDown() {
  ThinClientPoolDM::netDown();

  {
    std::lock_guard<decltype(m_endpointsLock)> guard(m_endpointsLock);
    for (auto&& currItr : m_endpoints) {
      currItr.second->setConnectionStatus(false);
    }
  }

  m_redundancyManager->netDown();
}

void ThinClientPoolHADM::pingServerLocal() {
  auto& mutex = m_redundancyManager->getRedundancyLock();
  std::lock_guard<decltype(mutex)> guard(mutex);
  ThinClientPoolDM::pingServerLocal();
}

void ThinClientPoolHADM::removeCallbackConnection(TcrEndpoint* ep) {
  m_redundancyManager->removeCallbackConnection(ep);
}

void ThinClientPoolHADM::sendNotConMesToAllregions() {
  std::lock_guard<decltype(m_regionsLock)> guard(m_regionsLock);
  for (std::list<ThinClientRegion*>::iterator it = m_regions.begin();
       it != m_regions.end(); it++) {
    (*it)->receiveNotification(TcrMessage::getAllEPDisMess());
  }
}

std::shared_ptr<TcrEndpoint> ThinClientPoolHADM::createEP(
    const char* endpointName) {
  return std::make_shared<TcrPoolEndPoint>(
      endpointName, m_connManager.getCacheImpl(), m_connManager.m_failoverSema,
      m_connManager.m_cleanupSema, m_redundancySema, this);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
