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
#include "FunctionExpiryTask.hpp"
#include "TcrConnection.hpp"
#include "TcrEndpoint.hpp"
#include "TcrHADistributionManager.hpp"
#include "ThinClientBaseDM.hpp"
#include "ThinClientHARegion.hpp"
#include "ThinClientRedundancyManager.hpp"
#include "ThinClientRegion.hpp"
#include "Utils.hpp"
#include "util/exception.hpp"

namespace apache {
namespace geode {
namespace client {

volatile bool TcrConnectionManager::TEST_DURABLE_CLIENT_CRASH = false;

const char *TcrConnectionManager::NC_Failover = "NC Failover";
const char *TcrConnectionManager::NC_CleanUp = "NC CleanUp";

TcrConnectionManager::TcrConnectionManager(CacheImpl *cache)
    : m_cache(cache),
      m_initGuard(false),
      failover_semaphore_(0),
      m_failoverTask(nullptr),
      cleanup_semaphore_(0),
      m_cleanupTask(nullptr),
      ping_task_id_(ExpiryTask::invalid()),
      // Create the queues with flag to not delete the objects
      notify_cleanup_semaphore_list_(false),
      redundancy_semaphore_(0),
      m_redundancyTask(nullptr),
      m_isDurable(false),
      m_isNetDown(false) {
  m_redundancyManager = std::unique_ptr<ThinClientRedundancyManager>(
      new ThinClientRedundancyManager(this));
}

void TcrConnectionManager::init(bool isPool) {
  if (!m_initGuard) {
    m_initGuard = true;
  } else {
    return;
  }
  auto &props = m_cache->getDistributedSystem().getSystemProperties();
  m_isDurable = !props.durableClientId().empty();

  auto interval = props.pingInterval();
  if (!isPool) {
    auto &expiry_manager = m_cache->getExpiryTaskManager();
    auto task = std::make_shared<FunctionExpiryTask>(
        expiry_manager, [this]() { ping_endpoints(); });

    ping_task_id_ = expiry_manager.schedule(std::move(task),
                                            std::chrono::seconds(10), interval);
    LOG_FINE(
        "TcrConnectionManager::TcrConnectionManager Registered ping "
        "task with id = %ld, interval = %ld",
        ping_task_id_, interval.count());
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
  LOG_FINE("TcrConnectionManager is closing");

  m_cache->getExpiryTaskManager().cancel(ping_task_id_);

  if (m_failoverTask != nullptr) {
    m_failoverTask->stopNoblock();
    failover_semaphore_.release();
    m_failoverTask->wait();
    m_failoverTask = nullptr;
  }

  LOG_FINE("TcrConnectionManager is closed");
}

void TcrConnectionManager::readyForEvents() {
  m_redundancyManager->readyForEvents();
}

TcrConnectionManager::~TcrConnectionManager() {
  if (m_cleanupTask != nullptr) {
    m_cleanupTask->stopNoblock();
    cleanup_semaphore_.release();
    m_cleanupTask->wait();
    // Clean notification lists if something remains in there; see bug #250
    cleanNotificationLists();
    m_cleanupTask = nullptr;

    // sanity cleanup of any remaining endpoints with warning; see bug #298
    //  cleanup of endpoints, when regions are destroyed via notification
    {
      auto &&guard = m_endpoints.make_lock();
      if (m_endpoints.size() > 0) {
        LOG_FINE("TCCM: endpoints remain in destructor");
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
      LOG_FINE(
          "TcrConnectionManager::connect(): Empty endpointstr vector "
          "passed to TCCM, will initialize endpoints list with all available "
          "endpoints (%zu).",
          m_endpoints.size());
      for (const auto &currItr : m_endpoints) {
        auto ep = currItr.second;
        ep->setNumRegions(ep->numRegions() + 1);
        LOG_FINER(
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
    ep = std::make_shared<TcrEndpoint>(endpointName, m_cache,
                                       failover_semaphore_, cleanup_semaphore_,
                                       redundancy_semaphore_, dm, false);
    m_endpoints.emplace(endpointName, ep);
  } else {
    ep = find->second;
  }
  ep->setNumRegions(ep->numRegions() + 1);

  LOG_FINER("TCCM: incremented region reference count for endpoint %s to %d",
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

  LOG_FINER("TCCM: decremented region reference count for endpoint %s to %d",
            ep->name().c_str(), ep->numRegions());

  if (0 == ep->numRegions()) {
    // this endpoint no longer used
    m_endpoints.erase(ep->name());
    LOG_FINE("delete endpoint %s", ep->name().c_str());
    _GEODE_SAFE_DELETE(ep);
    hasRemovedEndpoint = true;
  }
  return hasRemovedEndpoint;
}

void TcrConnectionManager::ping_endpoints() {
  auto &&guard = m_endpoints.make_lock();
  for (const auto &currItr : m_endpoints) {
    if (currItr.second->connected() && !m_isNetDown) {
      currItr.second->pingServer();
    }
  }
}

void TcrConnectionManager::failover(std::atomic<bool> &isRunning) {
  LOG_FINE("TcrConnectionManager: starting failover thread");

  failover_semaphore_.acquire();
  while (isRunning) {
    if (!m_isNetDown) {
      try {
        std::lock_guard<decltype(m_distMngrsLock)> guard(m_distMngrsLock);
        for (const auto &it : m_distMngrs) {
          it->failover();
        }
      } catch (const Exception &e) {
        LOG_ERROR(e.what());
      } catch (const std::exception &e) {
        LOG_ERROR(e.what());
      } catch (...) {
        LOG_ERROR(
            "Unexpected exception while failing over to a "
            "different endpoint");
      }
    }

    failover_semaphore_.acquire();
  }

  LOG_FINE("TcrConnectionManager: ending failover thread");
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
  std::lock_guard<decltype(m_distMngrsLock)> guard(m_distMngrsLock);

  for (const auto &it : m_distMngrs) {
    if (auto tcrHADM = dynamic_cast<TcrHADistributionManager *>(it)) {
      auto opErr = tcrHADM->registerInterestForRegion(ep, request, reply);
      if (err == GF_NOERR) {
        err = opErr;
      }
    }
  }
  return err;
}
GfErrType TcrConnectionManager::sendSyncRequestCq(TcrMessage &request,
                                                  TcrMessageReply &reply) {
  LOG_DEBUG("TcrConnectionManager::sendSyncRequestCq");
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
  LOG_FINE("Starting subscription maintain redundancy thread.");
  redundancy_semaphore_.acquire();

  while (isRunning) {
    if (!m_isNetDown) {
      m_redundancyManager->maintainRedundancyLevel();
    }

    redundancy_semaphore_.acquire();
  }
  LOG_FINE("Ending subscription maintain redundancy thread.");
}

void TcrConnectionManager::addNotificationForDeletion(
    Task<TcrEndpoint> *notifyReceiver, TcrConnection *notifyConnection,
    binary_semaphore &notifyCleanupSema) {
  std::lock_guard<decltype(m_notificationLock)> guard(m_notificationLock);
  m_connectionReleaseList.put(notifyConnection);
  m_receiverReleaseList.put(notifyReceiver);
  notify_cleanup_semaphore_list_.put(&notifyCleanupSema);
}

void TcrConnectionManager::cleanup(std::atomic<bool> &isRunning) {
  LOG_FINE("TcrConnectionManager: starting cleanup thread");

  cleanup_semaphore_.acquire();

  while (isRunning) {
    cleanNotificationLists();
    cleanup_semaphore_.acquire();
  }

  LOG_FINE("TcrConnectionManager: ending cleanup thread");
  //  Postcondition - all notification channels should be cleaned up by the end
  //  of this function.
}

void TcrConnectionManager::cleanNotificationLists() {
  Task<TcrEndpoint> *notifyReceiver;
  TcrConnection *notifyConnection;
  binary_semaphore *semaphore;

  while (true) {
    {
      std::lock_guard<decltype(m_notificationLock)> guard(m_notificationLock);
      notifyReceiver = m_receiverReleaseList.get();
      if (!notifyReceiver) break;
      notifyConnection = m_connectionReleaseList.get();
      semaphore = notify_cleanup_semaphore_list_.get();
    }
    notifyReceiver->wait();
    //_GEODE_SAFE_DELETE(notifyReceiver);
    _GEODE_SAFE_DELETE(notifyConnection);
    semaphore->release();
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
// TESTING: Disconnections of endpoint - return number of times that endpoint
// disconnected
int TcrConnectionManager::getNumberOfTimeEndpointDisconnected(
    const std::string &endpoint) {
  auto &&guard = m_endpoints.make_lock();
  for (auto &currItr : m_endpoints) {
    auto ep = currItr.second;
    const std::string epName = ep->name();
    if (epName == endpoint) return ep->numberOfTimesFailed();
  }
  throw IllegalStateException("Endpoint not found");
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
