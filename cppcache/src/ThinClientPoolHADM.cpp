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

#include "CacheImpl.hpp"
#include "FunctionExpiryTask.hpp"
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
      redundancy_semaphore_(0),
      m_redundancyTask(nullptr),
      server_monitor_task_id_(ExpiryTask::invalid()) {
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

  const auto interval = props.redundancyMonitorInterval();
  auto& manager = m_connManager.getCacheImpl()->getExpiryTaskManager();
  auto task = std::make_shared<FunctionExpiryTask>(
      manager, [this]() { redundancy_semaphore_.release(); });

  server_monitor_task_id_ =
      manager.schedule(std::move(task), std::chrono::seconds(1), interval);

  LOGFINE(
      "ThinClientPoolHADM::ThinClientPoolHADM Registered server "
      "monitor task with id = %ld, interval = %ld",
      server_monitor_task_id_, interval.count());

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
    return sendSyncRequestCq(request, reply);
  } else {
    return ThinClientPoolDM::sendSyncRequest(request, reply, attemptFailover,
                                             isBGThread);
  }
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

  redundancy_semaphore_.acquire();
  while (isRunning) {
    if (!m_connManager.isNetDown()) {
      m_redundancyManager->maintainRedundancyLevel();
    }

    redundancy_semaphore_.acquire();
  }
  LOGFINE("ThinClientPoolHADM: Ending maintain redundancy thread.");
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
    auto& manager = m_connManager.getCacheImpl()->getExpiryTaskManager();
    manager.cancel(server_monitor_task_id_);
    m_redundancyTask->stopNoblock();
    redundancy_semaphore_.release();
    m_redundancyTask->wait();
    m_redundancyTask = nullptr;
    m_redundancyManager->sendNotificationCloseMsgs();
  }
}

GfErrType ThinClientPoolHADM::registerInterestAllRegions(
    TcrEndpoint* ep, const TcrMessage* request, TcrMessageReply* reply) {
  GfErrType err = GF_NOERR;

  std::lock_guard<decltype(m_regionsLock)> guard(m_regionsLock);
  for (const auto& region : m_regions) {
    auto opErr = region->registerKeys(ep, request, reply);
    if (err == GF_NOERR) {
      err = opErr;
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

void ThinClientPoolHADM::clearKeysOfInterestAllRegions() {
  std::lock_guard<decltype(m_regionsLock)> guard(m_regionsLock);
  for (auto region : m_regions) {
    region->clearKeysOfInterest();
  }
}

std::shared_ptr<TcrEndpoint> ThinClientPoolHADM::createEP(
    const char* endpointName) {
  return std::make_shared<TcrPoolEndPoint>(
      endpointName, m_connManager.getCacheImpl(),
      m_connManager.failover_semaphore_, m_connManager.cleanup_semaphore_,
      redundancy_semaphore_, this);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
