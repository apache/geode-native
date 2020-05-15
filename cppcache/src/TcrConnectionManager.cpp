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

#include "TcrConnectionManager.hpp"

#include <chrono>
#include <set>
#include <thread>

#include <geode/SystemProperties.hpp>

#include "CacheImpl.hpp"
#include "ExpiryHandler_T.hpp"
#include "ExpiryTaskManager.hpp"
#include "RemoteQueryService.hpp"
#include "ServerLocation.hpp"
#include "TcrConnection.hpp"
#include "TcrEndpoint.hpp"
#include "TcrHADistributionManager.hpp"
#include "ThinClientBaseDM.hpp"
#include "ThinClientCacheDistributionManager.hpp"
#include "ThinClientHARegion.hpp"
#include "ThinClientLocatorHelper.hpp"
#include "ThinClientRedundancyManager.hpp"
#include "ThinClientRegion.hpp"
#include "Utils.hpp"
#include "util/exception.hpp"

namespace apache {
namespace geode {
namespace client {

volatile bool TcrConnectionManager::TEST_DURABLE_CLIENT_CRASH = false;

const char *TcrConnectionManager::NC_Redundancy = "NC Redundancy";
const char *TcrConnectionManager::NC_Failover = "NC Failover";
const char *TcrConnectionManager::NC_CleanUp = "NC CleanUp";

TcrConnectionManager::TcrConnectionManager(CacheImpl *cache)
    : m_cache(cache),
      m_initGuard(false),
      m_failoverSema(0),
      m_failoverTask(nullptr),
      m_cleanupSema(0),
      m_cleanupTask(nullptr),
      m_pingTaskId(-1),
      m_servermonitorTaskId(-1),
      // Create the queues with flag to not delete the objects
      m_notifyCleanupSemaList(false),
      m_redundancySema(0),
      m_redundancyTask(nullptr),
      m_isDurable(false),
      m_isNetDown(false) {
  m_redundancyManager = std::unique_ptr<ThinClientRedundancyManager>(
      new ThinClientRedundancyManager(this));
}

ExpiryTaskManager::id_type TcrConnectionManager::getPingTaskId() {
  return m_pingTaskId;
}
void TcrConnectionManager::init(bool isPool) {
  if (!m_initGuard) {
    m_initGuard = true;
  } else {
    return;
  }
  auto &props = m_cache->getDistributedSystem().getSystemProperties();
  m_isDurable = !props.durableClientId().empty();
  auto pingInterval = (props.pingInterval() / 2);
  if (!isPool) {
    ACE_Event_Handler *connectionChecker =
        new ExpiryHandler_T<TcrConnectionManager>(
            this, &TcrConnectionManager::checkConnection);
    m_pingTaskId = m_cache->getExpiryTaskManager().scheduleExpiryTask(
        connectionChecker, std::chrono::seconds(10), pingInterval, false);
    LOGFINE(
        "TcrConnectionManager::TcrConnectionManager Registered ping "
        "task with id = %ld, interval = %ld",
        m_pingTaskId, pingInterval.count());
  }

  m_redundancyManager->m_HAenabled = false;

  startFailoverAndCleanupThreads(isPool);
}

void TcrConnectionManager::startFailoverAndCleanupThreads(bool isPool) {
  if (!isPool && (m_failoverTask == nullptr || m_cleanupTask == nullptr)) {
    std::lock_guard<decltype(m_distMngrsLock)> _guard(m_distMngrsLock);
    if (!m_failoverTask) {
      m_failoverTask = std::unique_ptr<Task<TcrConnectionManager>>(
          new Task<TcrConnectionManager>(this, &TcrConnectionManager::failover,
                                         NC_Failover));
      m_failoverTask->start();
    }
    if (!m_cleanupTask) {
      if (m_redundancyManager->m_HAenabled) {
        m_redundancyManager->startPeriodicAck();
      }
      m_cleanupTask = std::unique_ptr<Task<TcrConnectionManager>>(
          new Task<TcrConnectionManager>(this, &TcrConnectionManager::cleanup,
                                         NC_CleanUp));
      m_cleanupTask->start();
    }
  }
}

void TcrConnectionManager::close() {
  LOGFINE("TcrConnectionManager is closing");

  if (m_pingTaskId > 0) {
    m_cache->getExpiryTaskManager().cancelTask(m_pingTaskId);
  }

  if (m_failoverTask != nullptr) {
    m_failoverTask->stopNoblock();
    m_failoverSema.release();
    m_failoverTask->wait();
    m_failoverTask = nullptr;
  }

  LOGFINE("TcrConnectionManager is closed");
}

void TcrConnectionManager::readyForEvents() {
  m_redundancyManager->readyForEvents();
}

TcrConnectionManager::~TcrConnectionManager() {
  if (m_cleanupTask != nullptr) {
    m_cleanupTask->stopNoblock();
    m_cleanupSema.release();
    m_cleanupTask->wait();
    // Clean notification lists if something remains in there; see bug #250
    cleanNotificationLists();
    m_cleanupTask = nullptr;

    // sanity cleanup of any remaining endpoints with warning; see bug #298
    //  cleanup of endpoints, when regions are destroyed via notification
    {
      auto &&guard = m_endpoints.make_lock();
      if (m_endpoints.size() > 0) {
        LOGFINE("TCCM: endpoints remain in destructor");
      }
    }
  }
  TcrConnectionManager::TEST_DURABLE_CLIENT_CRASH = false;
}

void TcrConnectionManager::connect(
    ThinClientBaseDM *distMng, std::vector<TcrEndpoint *> &endpoints,
    const std::unordered_set<std::string> &endpointStrs) {
  std::lock_guard<decltype(m_distMngrsLock)> guardDistMngrs(m_distMngrsLock);
  {
    auto &&endpointsGuard = m_endpoints.make_lock();
    int32_t numEndPoints = static_cast<int32_t>(endpointStrs.size());

    if (numEndPoints == 0) {
      LOGFINE(
          "TcrConnectionManager::connect(): Empty endpointstr vector "
          "passed to TCCM, will initialize endpoints list with all available "
          "endpoints (%zu).",
          m_endpoints.size());
      for (const auto &currItr : m_endpoints) {
        auto ep = currItr.second;
        ep->setNumRegions(ep->numRegions() + 1);
        LOGFINER(
            "TCCM 2: incremented region reference count for endpoint %s "
            "to %d",
            ep->name().c_str(), ep->numRegions());
        endpoints.push_back(ep.get());
      }
    } else {
      for (const auto &iter : endpointStrs) {
        auto ep = addRefToTcrEndpoint(iter, distMng);
        endpoints.push_back(ep);
      }
    }
  }

  m_distMngrs.push_back(distMng);

  // If a region/DM is joining after the marker has been
  // received then trigger it's marker flag.
  if (m_redundancyManager->m_globalProcessedMarker) {
    TcrHADistributionManager *tcrHADM =
        dynamic_cast<TcrHADistributionManager *>(distMng);
    if (tcrHADM != nullptr) {
      ThinClientHARegion *tcrHARegion =
          dynamic_cast<ThinClientHARegion *>(tcrHADM->m_region);
      tcrHARegion->setProcessedMarker();
    }
  }
}

TcrEndpoint *TcrConnectionManager::addRefToTcrEndpoint(std::string endpointName,
                                                       ThinClientBaseDM *dm) {
  std::shared_ptr<TcrEndpoint> ep;

  auto &&guard = m_endpoints.make_lock();
  const auto &find = m_endpoints.find(endpointName);
  if (find == m_endpoints.end()) {
    // this endpoint does not exist
    ep = std::make_shared<TcrEndpoint>(endpointName, m_cache, m_failoverSema,
                                       m_cleanupSema, m_redundancySema, dm,
                                       false);
    m_endpoints.emplace(endpointName, ep);
  } else {
    ep = find->second;
  }
  ep->setNumRegions(ep->numRegions() + 1);

  LOGFINER("TCCM: incremented region reference count for endpoint %s to %d",
           ep->name().c_str(), ep->numRegions());

  return ep.get();
}

void TcrConnectionManager::disconnect(ThinClientBaseDM *distMng,
                                      std::vector<TcrEndpoint *> &endpoints,
                                      bool keepEndpoints) {
  std::lock_guard<decltype(m_distMngrsLock)> guardDistMngrs(m_distMngrsLock);
  {
    auto &&guard = m_endpoints.make_lock();
    for (const auto &ep : endpoints) {
      removeRefToEndpoint(ep, keepEndpoints);
    }
  }

  m_distMngrs.remove(distMng);
}

bool TcrConnectionManager::removeRefToEndpoint(TcrEndpoint *ep,
                                               bool keepEndpoint) {
  bool hasRemovedEndpoint = false;

  if (keepEndpoint && (ep->numRegions() == 1)) {
    return false;
  }
  ep->setNumRegions(ep->numRegions() - 1);

  LOGFINER("TCCM: decremented region reference count for endpoint %s to %d",
           ep->name().c_str(), ep->numRegions());

  if (0 == ep->numRegions()) {
    // this endpoint no longer used
    m_endpoints.erase(ep->name());
    LOGFINE("delete endpoint %s", ep->name().c_str());
    _GEODE_SAFE_DELETE(ep);
    hasRemovedEndpoint = true;
  }
  return hasRemovedEndpoint;
}

int TcrConnectionManager::processEventIdMap(const ACE_Time_Value &currTime,
                                            const void *) {
  return m_redundancyManager->processEventIdMap(currTime, nullptr);
}

int TcrConnectionManager::checkConnection(const ACE_Time_Value &,
                                          const void *) {
  auto &&guard = m_endpoints.make_lock();
  for (const auto &currItr : m_endpoints) {
    if (currItr.second->connected() && !m_isNetDown) {
      currItr.second->pingServer();
    }
  }
  return 0;
}

int TcrConnectionManager::checkRedundancy(const ACE_Time_Value &,
                                          const void *) {
  m_redundancySema.release();
  return 0;
}

void TcrConnectionManager::failover(std::atomic<bool> &isRunning) {
  LOGFINE("TcrConnectionManager: starting failover thread");
  while (isRunning) {
    m_failoverSema.acquire();
    if (isRunning && !m_isNetDown) {
      try {
        std::lock_guard<decltype(m_distMngrsLock)> guard(m_distMngrsLock);
        for (const auto &it : m_distMngrs) {
          it->failover();
        }
        while (m_failoverSema.tryacquire() != -1) {
          ;
        }
      } catch (const Exception &e) {
        LOGERROR(e.what());
      } catch (const std::exception &e) {
        LOGERROR(e.what());
      } catch (...) {
        LOGERROR(
            "Unexpected exception while failing over to a "
            "different endpoint");
      }
    }
  }
  LOGFINE("TcrConnectionManager: ending failover thread");
}

void TcrConnectionManager::getAllEndpoints(
    std::vector<TcrEndpoint *> &endpoints) {
  auto &&guard = m_endpoints.make_lock();
  for (const auto &currItr : m_endpoints) {
    endpoints.push_back(currItr.second.get());
  }
}

int32_t TcrConnectionManager::getNumEndPoints() {
  return static_cast<int32_t>(m_endpoints.size());
}

GfErrType TcrConnectionManager::registerInterestAllRegions(
    TcrEndpoint *ep, const TcrMessage *request, TcrMessageReply *reply) {
  // Preconditions:
  // 1. m_distMngrs.size() > 1 (query distribution manager + 1 or more
  // TcrHADistributionManagers).

  GfErrType err = GF_NOERR;
  GfErrType opErr = GF_NOERR;
  std::lock_guard<decltype(m_distMngrsLock)> guard(m_distMngrsLock);
  std::list<ThinClientBaseDM *>::iterator begin = m_distMngrs.begin();
  std::list<ThinClientBaseDM *>::iterator end = m_distMngrs.end();
  for (std::list<ThinClientBaseDM *>::iterator it = begin; it != end; ++it) {
    TcrHADistributionManager *tcrHADM =
        dynamic_cast<TcrHADistributionManager *>(*it);
    if (tcrHADM != nullptr) {
      if ((opErr = tcrHADM->registerInterestForRegion(ep, request, reply)) !=
          GF_NOERR) {
        if (err == GF_NOERR) {
          err = opErr;
        }
      }
    }
  }
  return err;
}
GfErrType TcrConnectionManager::sendSyncRequestCq(TcrMessage &request,
                                                  TcrMessageReply &reply) {
  LOGDEBUG("TcrConnectionManager::sendSyncRequestCq");
  GfErrType err = GF_NOERR;
  // Preconditions:
  // 1. m_distMngrs.size() > 1 (query distribution manager + 1 or more
  // TcrHADistributionManagers).

  std::lock_guard<decltype(m_distMngrsLock)> guard(m_distMngrsLock);
  std::list<ThinClientBaseDM *>::iterator begin = m_distMngrs.begin();
  std::list<ThinClientBaseDM *>::iterator end = m_distMngrs.end();
  for (std::list<ThinClientBaseDM *>::iterator it = begin; it != end; ++it) {
    TcrHADistributionManager *tcrHADM =
        dynamic_cast<TcrHADistributionManager *>(*it);
    if (tcrHADM != nullptr) {
      return tcrHADM->sendSyncRequestCq(request, reply);
    }
  }
  return err;
}

void TcrConnectionManager::initializeHAEndpoints(const char *endpointsStr) {
  std::unordered_set<std::string> endpointsList;
  Utils::parseEndpointNamesString(endpointsStr, endpointsList);
  for (std::unordered_set<std::string>::iterator iter = endpointsList.begin();
       iter != endpointsList.end(); ++iter) {
    addRefToTcrEndpoint(*iter);
  }
  // Postconditions:
  // 1. endpointsList.size() > 0
}

void TcrConnectionManager::removeHAEndpoints() {
  auto &&guard = m_endpoints.make_lock();
  auto currItr = m_endpoints.begin();
  while (currItr != m_endpoints.end()) {
    if (removeRefToEndpoint(currItr->second.get())) {
      currItr = m_endpoints.begin();
    } else {
      currItr++;
    }
  }
}

void TcrConnectionManager::netDown() {
  m_isNetDown = true;

  //  sleep for 15 seconds to allow ping and redundancy threads to pause.
  std::this_thread::sleep_for(std::chrono::seconds(15));

  {
    auto &&guard = m_endpoints.make_lock();
    for (auto &currItr : m_endpoints) {
      currItr.second->setConnectionStatus(false);
    }
  }

  m_redundancyManager->netDown();
}

/* Need to do a get on unknown key after calling this Fn to restablish all
 * connection */
void TcrConnectionManager::revive() {
  m_isNetDown = false;

  //  sleep for 15 seconds to allow redundancy thread to reestablish
  //  connections.
  std::this_thread::sleep_for(std::chrono::seconds(15));
}

void TcrConnectionManager::redundancy(std::atomic<bool> &isRunning) {
  LOGFINE("Starting subscription maintain redundancy thread.");
  while (isRunning) {
    m_redundancySema.acquire();
    if (isRunning && !m_isNetDown) {
      m_redundancyManager->maintainRedundancyLevel();
      while (m_redundancySema.tryacquire() != -1) {
        ;
      }
    }
  }
  LOGFINE("Ending subscription maintain redundancy thread.");
}

void TcrConnectionManager::addNotificationForDeletion(
    Task<TcrEndpoint> *notifyReceiver, TcrConnection *notifyConnection,
    ACE_Semaphore &notifyCleanupSema) {
  std::lock_guard<decltype(m_notificationLock)> guard(m_notificationLock);
  m_connectionReleaseList.put(notifyConnection);
  m_receiverReleaseList.put(notifyReceiver);
  m_notifyCleanupSemaList.put(&notifyCleanupSema);
}

void TcrConnectionManager::cleanup(std::atomic<bool> &isRunning) {
  LOGFINE("TcrConnectionManager: starting cleanup thread");
  do {
    //  If we block on acquire, the queue must be empty (precondition).
    if (m_receiverReleaseList.size() == 0) {
      LOGDEBUG(
          "TcrConnectionManager::cleanup(): waiting to acquire cleanup "
          "semaphore.");
      m_cleanupSema.acquire();
    }
    cleanNotificationLists();

    while (m_cleanupSema.tryacquire() != -1) {
      ;
    }

  } while (isRunning);

  LOGFINE("TcrConnectionManager: ending cleanup thread");
  //  Postcondition - all notification channels should be cleaned up by the end
  //  of this function.
}

void TcrConnectionManager::cleanNotificationLists() {
  Task<TcrEndpoint> *notifyReceiver;
  TcrConnection *notifyConnection;
  ACE_Semaphore *notifyCleanupSema;

  while (true) {
    {
      std::lock_guard<decltype(m_notificationLock)> guard(m_notificationLock);
      notifyReceiver = m_receiverReleaseList.get();
      if (!notifyReceiver) break;
      notifyConnection = m_connectionReleaseList.get();
      notifyCleanupSema = m_notifyCleanupSemaList.get();
    }
    notifyReceiver->wait();
    //_GEODE_SAFE_DELETE(notifyReceiver);
    _GEODE_SAFE_DELETE(notifyConnection);
    notifyCleanupSema->release();
  }
}

void TcrConnectionManager::processMarker() {
  // also set the static bool m_processedMarker for makePrimary messages
  m_redundancyManager->m_globalProcessedMarker = true;
}

//  TESTING: Durable clients - return queue status of endpoing. Not thread safe.
bool TcrConnectionManager::getEndpointStatus(const std::string &endpoint) {
  for (auto &currItr : m_endpoints) {
    auto ep = currItr.second;
    const std::string epName = ep->name();
    if (epName == endpoint) return ep->getServerQueueStatusTEST();
  }
  return false;
}

GfErrType TcrConnectionManager::sendSyncRequestCq(
    TcrMessage &request, TcrMessageReply &reply,
    TcrHADistributionManager *theHADM) {
  return m_redundancyManager->sendSyncRequestCq(request, reply, theHADM);
}

GfErrType TcrConnectionManager::sendSyncRequestRegisterInterest(
    TcrMessage &request, TcrMessageReply &reply, bool attemptFailover,
    TcrEndpoint *endpoint, TcrHADistributionManager *theHADM,
    ThinClientRegion *region) {
  return m_redundancyManager->sendSyncRequestRegisterInterest(
      request, reply, attemptFailover, endpoint, theHADM, region);
}
}  // namespace client
}  // namespace geode
}  // namespace apache
