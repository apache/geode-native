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

#include "TcrPoolEndPoint.hpp"

#include <geode/SystemProperties.hpp>

#include "CacheImpl.hpp"
#include "ThinClientPoolDM.hpp"

namespace apache {
namespace geode {
namespace client {

TcrPoolEndPoint::TcrPoolEndPoint(const std::string& name, CacheImpl* cache,
                                 binary_semaphore& failoverSema,
                                 binary_semaphore& cleanupSema,
                                 binary_semaphore& redundancySema,
                                 ThinClientPoolDM* dm)
    : TcrEndpoint(name, cache, failoverSema, cleanupSema, redundancySema, dm),
      m_dm(dm) {}
bool TcrPoolEndPoint::checkDupAndAdd(std::shared_ptr<EventId> eventid) {
  return m_dm->checkDupAndAdd(eventid);
}

void TcrPoolEndPoint::processMarker() { m_dm->processMarker(); }
std::shared_ptr<QueryService> TcrPoolEndPoint::getQueryService() {
  return m_dm->getQueryServiceWithoutCheck();
}
ThinClientPoolDM* TcrPoolEndPoint::getPoolHADM() const { return m_dm; }
void TcrPoolEndPoint::triggerRedundancyThread() {
  m_dm->triggerRedundancyThread();
}
void TcrPoolEndPoint::closeFailedConnection(TcrConnection*& conn) {
  if (!m_dm->getThreadLocalConnections()) closeConnection(conn);
}

bool TcrPoolEndPoint::isMultiUserMode() { return m_dm->isMultiUserMode(); }

void TcrPoolEndPoint::closeNotification() {
  LOG_FINE("TcrPoolEndPoint::closeNotification..");
  m_notifyReceiver->stopNoblock();
  m_notifyConnectionList.push_back(m_notifyConnection);
  m_notifyReceiverList.push_back(m_notifyReceiver.get());
  m_isQueueHosted = false;
}

GfErrType TcrPoolEndPoint::registerDM(bool, bool isSecondary, bool,
                                      ThinClientBaseDM*) {
  GfErrType err = GF_NOERR;
  std::lock_guard<decltype(m_dm->getPoolLock())> _guard(m_dm->getPoolLock());
  std::lock_guard<decltype(getQueueHostedMutex())> guardQueueHosted(
      getQueueHostedMutex());
  auto& sysProp = m_cacheImpl->getDistributedSystem().getSystemProperties();
  if (!connected()) {
    TcrConnection* newConn;
    if ((err = createNewConnection(newConn, false, false,
                                   sysProp.connectTimeout(), 0, connected())) !=
        GF_NOERR) {
      setConnected(false);
      return err;
    }
    m_dm->addConnection(newConn);
    setConnected(true);
  }

  LOG_FINEST(
      "TcrEndpoint::registerPoolDM( ): registering DM and notification "
      "channel for endpoint %s",
      name().c_str());

  if (m_numRegionListener == 0) {
    if ((err = createNewConnection(m_notifyConnection, true, isSecondary,
                                   sysProp.connectTimeout() * 3, 0)) !=
        GF_NOERR) {
      setConnected(false);
      LOG_WARN("Failed to start subscription channel for endpoint %s",
               name().c_str());
      return err;
    }
    m_notifyReceiver = std::unique_ptr<Task<TcrEndpoint>>(new Task<TcrEndpoint>(
        this, &TcrEndpoint::receiveNotification, NC_Notification));
    m_notifyReceiver->start();
  }
  ++m_numRegionListener;
  LOG_FINEST("Incremented notification count for endpoint %s to %d",
             name().c_str(), m_numRegionListener);

  m_isQueueHosted = true;
  setConnected(true);
  return err;
}
void TcrPoolEndPoint::unregisterDM(bool, ThinClientBaseDM*,
                                   bool checkQueueHosted) {
  std::lock_guard<decltype(getQueueHostedMutex())> guardQueueHosted(
      getQueueHostedMutex());

  if (checkQueueHosted && !m_isQueueHosted) {
    LOG_FINEST(
        "TcrEndpoint: unregistering pool DM, notification channel not present "
        "for %s",
        name().c_str());
    return;
  }

  LOG_FINEST(
      "TcrEndpoint: unregistering pool DM and closing notification "
      "channel for endpoint %s",
      name().c_str());
  std::lock_guard<decltype(m_notifyReceiverLock)> guard2(m_notifyReceiverLock);
  if (m_numRegionListener > 0 && --m_numRegionListener == 0) {
    closeNotification();
  }
  LOG_FINEST("Decremented notification count for endpoint %s to %d",
             name().c_str(), m_numRegionListener);
  LOG_FINEST("TcrEndpoint: unregisterPoolDM done for endpoint %s",
             name().c_str());
}

bool TcrPoolEndPoint::handleIOException(const std::string& message,
                                        TcrConnection*& conn, bool isBgThread) {
  if (!isBgThread) {
    m_dm->setStickyNull(false);
  }
  return TcrEndpoint::handleIOException(message, conn);
}

void TcrPoolEndPoint::handleNotificationStats(int64_t byteLength) {
  m_dm->getStats().incReceivedBytes(byteLength);
  m_dm->getStats().incMessageBeingReceived();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
