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

#include "ThinClientDistributionManager.hpp"

#include <algorithm>

#include <geode/AuthInitialize.hpp>
#include <geode/SystemProperties.hpp>

#include "CacheImpl.hpp"
#include "DistributedSystemImpl.hpp"
#include "TcrConnectionManager.hpp"
#include "TcrEndpoint.hpp"
#include "ThinClientRegion.hpp"
#include "util/exception.hpp"

namespace apache {
namespace geode {
namespace client {
ThinClientDistributionManager::ThinClientDistributionManager(
    TcrConnectionManager& connManager, ThinClientRegion* region)
    : ThinClientBaseDM(connManager, region), m_activeEndpoint(-1) {}

void ThinClientDistributionManager::init() {
  std::unordered_set<std::string> endpointNames;

  getEndpointNames(endpointNames);
  m_connManager.connect(this, m_endpoints, endpointNames);
  int32_t numEndpoints = static_cast<int32_t>(m_endpoints.size());
  std::vector<int> randIndex;
  for (int index = 0; index < numEndpoints; ++index) {
    randIndex.push_back(index);
  }
  RandGen randGen;
  std::random_shuffle(randIndex.begin(), randIndex.end(), randGen);
  int index = -1;
  GfErrType err = GF_NOERR;
  while (m_activeEndpoint < 0 && ++index < numEndpoints) {
    m_endpoints[randIndex[index]]->setDM(this);
    if ((err = m_endpoints[randIndex[index]]->registerDM(
             m_clientNotification, false, true)) == GF_NOERR) {
      m_activeEndpoint = randIndex[index];
      LOG_FINE("DM: Using endpoint %s",
               m_endpoints[m_activeEndpoint]->name().c_str());
    } else if (isFatalError(err)) {
      m_connManager.disconnect(this, m_endpoints);
      throwExceptionIfError("ThinClientDistributionManager::init", err);
    }
  }
  ThinClientBaseDM::init();
  m_initDone = true;
}
void ThinClientDistributionManager::destroy(bool keepAlive) {
  if (!m_initDone) {
    // nothing to be done
    return;
  }
  DistManagersLockGuard _guard(m_connManager);
  std::lock_guard<decltype(m_endpointsLock)> guard(m_endpointsLock);
  if (m_activeEndpoint >= 0) {
    m_endpoints[m_activeEndpoint]->unregisterDM(m_clientNotification, this);
  }
  LOG_FINEST(
      "ThinClientDistributionManager:: starting destroy for region %s",
      (m_region != nullptr ? m_region->getFullPath().c_str() : "(null)"));
  destroyAction();
  // stop the chunk processing thread
  stopChunkProcessor();
  if (Log::enabled(LogLevel::Finest)) {
    std::string endpointStr;
    for (size_t index = 0; index < m_endpoints.size(); ++index) {
      if (index != 0) {
        endpointStr.append(",");
      }
      endpointStr.append(m_endpoints[index]->name());
    }
    LOG_FINEST(
        "ThinClientDistributionManager: disconnecting endpoints %s from TCCM",
        endpointStr.c_str());
  }
  m_connManager.disconnect(this, m_endpoints, keepAlive);
  LOG_FINEST(
      "ThinClientDistributionManager: completed destroy for region %s",
      (m_region != nullptr ? m_region->getFullPath().c_str() : "(null)"));
  m_initDone = false;
}

void ThinClientDistributionManager::destroyAction() {}

void ThinClientDistributionManager::getEndpointNames(
    std::unordered_set<std::string>&) {}

bool ThinClientDistributionManager::isEndpointAttached(TcrEndpoint* ep) {
  for (std::vector<TcrEndpoint*>::const_iterator iter = m_endpoints.begin();
       iter != m_endpoints.end(); ++iter) {
    if (*iter == ep) {
      return true;
    }
  }
  return false;
}

GfErrType ThinClientDistributionManager::sendSyncRequest(TcrMessage& request,
                                                         TcrMessageReply& reply,
                                                         bool attemptFailover,
                                                         bool) {
  GfErrType error = GF_NOTCON;
  bool useActiveEndpoint = true;
  request.setDM(this);
  reply.setDM(this);
  if (request.getMessageType() == TcrMessage::GET_ALL_70 ||
      request.getMessageType() == TcrMessage::GET_ALL_WITH_CALLBACK) {
    request.InitializeGetallMsg(
        request.getCallbackArgument());  // now initialize getall msg
  }
  int currentEndpoint = m_activeEndpoint;
  if (currentEndpoint >= 0 && m_endpoints[currentEndpoint]->connected()) {
    LOG_DEBUG(
        "ThinClientDistributionManager::sendSyncRequest: trying to send on "
        "endpoint: %s",
        m_endpoints[currentEndpoint]->name().c_str());
    error = sendRequestToEP(request, reply, m_endpoints[currentEndpoint]);
    useActiveEndpoint = false;
    LOG_DEBUG(
        "ThinClientDistributionManager::sendSyncRequest: completed send on "
        "endpoint: %s [error:%d]",
        m_endpoints[currentEndpoint]->name().c_str(), error);
  }

  if (!attemptFailover || error == GF_NOERR) {
    return error;
  }

  bool doRand = true;
  std::vector<int> randIndex;
  int32_t type = request.getMessageType();
  bool forceSelect = false;

  // we need to forceSelect because endpoint connection status
  // is not set to false (in tcrendpoint::send) for a query or putall timeout
  if ((type == TcrMessage::QUERY || type == TcrMessage::QUERY_WITH_PARAMETERS ||
       type == TcrMessage::PUTALL ||
       type == TcrMessage::PUT_ALL_WITH_CALLBACK ||
       type == TcrMessage::EXECUTE_FUNCTION ||
       type == TcrMessage::EXECUTE_REGION_FUNCTION ||
       type == TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP ||
       type == TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE) &&
      error == GF_TIMEOUT) {
    forceSelect = true;
  }

  if (!isFatalError(error)) {
    std::lock_guard<decltype(m_endpointsLock)> guard(m_endpointsLock);
    GfErrType connErr = GF_NOERR;
    while (error != GF_NOERR && !isFatalError(error) &&
           (connErr = selectEndpoint(randIndex, doRand, useActiveEndpoint,
                                     forceSelect)) == GF_NOERR) {
      // if it's a query or putall and we had a timeout, just return with the
      // newly
      // selected endpoint without failover-retry
      if ((type == TcrMessage::QUERY ||
           type == TcrMessage::QUERY_WITH_PARAMETERS ||
           type == TcrMessage::PUTALL ||
           type == TcrMessage::PUT_ALL_WITH_CALLBACK ||
           type == TcrMessage::EXECUTE_FUNCTION ||
           type == TcrMessage::EXECUTE_REGION_FUNCTION ||
           type == TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP ||
           type == TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE) &&
          error == GF_TIMEOUT) {
        return error;
      }
      currentEndpoint = m_activeEndpoint;
      LOG_FINEST(
          "ThinClientDistributionManager::sendSyncRequest: trying send on new "
          "endpoint %s",
          m_endpoints[currentEndpoint]->name().c_str());
      error = sendRequestToEP(request, reply, m_endpoints[currentEndpoint]);
      if (error != GF_NOERR) {
        LOG_FINE(
            "ThinClientDistributionManager::sendSyncRequest: failed send on "
            "new endpoint %s for message "
            "type %d [error:%d]",
            m_endpoints[currentEndpoint]->name().c_str(),
            request.getMessageType(), error);
      } else {
        LOG_FINEST(
            "ThinClientDistributionManager::sendSyncRequest: completed send on "
            "new endpoint: %s",
            m_endpoints[currentEndpoint]->name().c_str());
      }
      useActiveEndpoint = false;
    }
    // : Top-level only sees NotConnectedException or TimeoutException
    if ((error == GF_NOERR && connErr != GF_NOERR) || error == GF_IOERR) {
      error = GF_NOTCON;
    }
  }
  return error;
}

void ThinClientDistributionManager::failover() {
  std::vector<int> randIndex;
  bool doRand = true;
  LOG_FINEST(
      "DM: invoked select endpoint via failover thread for region %s",
      (m_region != nullptr ? m_region->getFullPath().c_str() : "(null)"));
  selectEndpoint(randIndex, doRand);
}

inline GfErrType ThinClientDistributionManager::connectToEndpoint(int epIndex) {
  GfErrType err = GF_NOERR;
  TcrEndpoint* ep = m_endpoints[epIndex];
  ep->setDM(this);
  if ((err = ep->registerDM(m_clientNotification, false, true, this)) ==
      GF_NOERR) {
    LOG_FINE("DM: Attempting failover to endpoint %s", ep->name().c_str());
    if (postFailoverAction(ep)) {
      m_activeEndpoint = epIndex;
      LOG_FINE("DM: Failover to endpoint %s complete.", ep->name().c_str());
    } else {
      LOG_FINE("DM: Post failover action failed for endpoint %s",
               ep->name().c_str());
      ep->unregisterDM(m_clientNotification, this);
      err = GF_EUNDEF;
    }
  } else {
    LOG_FINE("DM: Could not connect to endpoint %s", ep->name().c_str());
  }
  return err;
}

GfErrType ThinClientDistributionManager::selectEndpoint(
    std::vector<int>& randIndex, bool& doRand, bool useActiveEndpoint,
    bool forceSelect) {
  // Preconditions
  // 1. Number of endpoints on which DM is registered <= 1

  GfErrType err = GF_NOERR;
  int currentEndpoint = m_activeEndpoint;

  if (currentEndpoint < 0 || !m_endpoints[currentEndpoint]->connected() ||
      forceSelect) {
    std::lock_guard<decltype(m_endpointsLock)> guard(m_endpointsLock);
    if (m_activeEndpoint < 0 || !m_endpoints[m_activeEndpoint]->connected() ||
        forceSelect) {  // double check
      currentEndpoint = m_activeEndpoint;
      if (m_activeEndpoint >= 0) {
        m_activeEndpoint = -1;
        m_endpoints[currentEndpoint]->unregisterDM(m_clientNotification, this);
        postUnregisterAction();
      }
      if (!preFailoverAction()) {
        return GF_NOERR;
      }
      int32_t numEndpoints = static_cast<int32_t>(m_endpoints.size());
      err = GF_EUNDEF;
      if (doRand) {
        RandGen randGen;
        for (int idx = 0; idx < numEndpoints; idx++) {
          if (useActiveEndpoint || idx != currentEndpoint) {
            randIndex.push_back(idx);
          }
        }
        std::random_shuffle(randIndex.begin(), randIndex.end(), randGen);
        doRand = false;
      }
      while (!randIndex.empty()) {
        int currIndex = randIndex[0];
        randIndex.erase(randIndex.begin());
        if ((err = connectToEndpoint(currIndex)) == GF_NOERR) {
          LOG_FINER("TCDM::selectEndpoint: Successfully selected endpoint %s",
                    m_endpoints[currIndex]->name().c_str());
          break;
        }
      }
    }
  }

  // Postconditions:
  // 1. If initial size of randIndex > 0 && failover was attempted,  final size
  // of randIndex < initial size of randIndex
  // 2. If CONN_NOERR, then m_activeEndpoint > -1, m_activeEndpoint should be
  // connected.
  // 3. Number of endpoints on which DM is registered <= 1
  return err;
}

void ThinClientDistributionManager::postUnregisterAction() {}

bool ThinClientDistributionManager::preFailoverAction() { return true; }

bool ThinClientDistributionManager::postFailoverAction(TcrEndpoint*) {
  return true;
}
std::shared_ptr<Properties> ThinClientDistributionManager::getCredentials(
    TcrEndpoint* ep) {
  auto cacheImpl = m_connManager.getCacheImpl();
  const auto& distributedSystem = cacheImpl->getDistributedSystem();
  const auto& tmpSecurityProperties =
      distributedSystem.getSystemProperties().getSecurityProperties();

  if (const auto& authInitialize = cacheImpl->getAuthInitialize()) {
    LOG_FINER(
        "ThinClientDistributionManager::getCredentials: acquired handle to "
        "authLoader, "
        "invoking getCredentials %s",
        ep->name().c_str());
    const auto& tmpAuthIniSecurityProperties = authInitialize->getCredentials(
        tmpSecurityProperties, ep->name().c_str());
    LOG_FINER("Done getting credentials");
    return tmpAuthIniSecurityProperties;
  }

  return nullptr;
}

GfErrType ThinClientDistributionManager::sendUserCredentials(
    std::shared_ptr<Properties> credentials, TcrEndpoint* ep) {
  LOG_DEBUG("ThinClientPoolDM::sendUserCredentials");

  GfErrType err = GF_NOERR;

  TcrMessageUserCredential request(
      new DataOutput(m_connManager.getCacheImpl()->createDataOutput()),
      credentials, this);

  TcrMessageReply reply(true, this);

  err = ep->send(request, reply);
  LOG_DEBUG(
      "ThinClientDistributionManager::sendUserCredentials: completed endpoint "
      "send for: %s [error:%d]",
      ep->name().c_str(), err);

  err = handleEPError(ep, reply, err);
  if (err == GF_IOERR) {
    err = GF_NOTCON;
  }

  if (err == GF_NOERR) {
    switch (reply.getMessageType()) {
      case TcrMessage::RESPONSE: {
        // nothing to be done;
        break;
      }
      case TcrMessage::EXCEPTION: {
        err = ThinClientRegion::handleServerException(
            "ThinClientDistributionManager::sendUserCredentials AuthException",
            reply.getException());
        break;
      }
      default: {
        LOG_ERROR(
            "Unknown message type %d during secure response, possible "
            "serialization mismatch",
            reply.getMessageType());
        err = GF_MSG;
        break;
      }
    }
    // throw exception if it is not authenticated
    // throwExceptionIfError("ThinClientDistributionManager::sendUserCredentials",
    // err);
  }

  return err;
}

GfErrType ThinClientDistributionManager::sendRequestToEP(
    const TcrMessage& request, TcrMessageReply& reply, TcrEndpoint* ep) {
  LOG_DEBUG(
      "ThinClientDistributionManager::sendRequestToEP: invoking endpoint send "
      "for: %s",
      ep->name().c_str());
  // retry for auth n times.
  // check if auth requie excep
  // if then then sendUserCreds message, if success
  // then send back original message
  int numberOftimesAuthTried = 3;

  if (!isSecurityOn()) numberOftimesAuthTried = 1;

  GfErrType error = GF_NOERR;

  while (numberOftimesAuthTried > 0) {
    ep->setDM(this);  // we are setting here before making request...
    if (isSecurityOn() && !ep->isAuthenticated()) {
      numberOftimesAuthTried--;
      error = this->sendUserCredentials(this->getCredentials(ep), ep);
    } else {
      numberOftimesAuthTried--;
    }

    if (error != GF_NOERR) return error;

    reply.setDM(this);
    error = ep->send(request, reply);
    LOG_DEBUG(
        "ThinClientDistributionManager::sendRequestToEP: completed endpoint "
        "send for: %s [error:%d]",
        ep->name().c_str(), error);
    error = handleEPError(ep, reply, error);
    if (error == GF_IOERR) {
      error = GF_NOTCON;
    }

    if (isSecurityOn() && error == GF_NOERR &&
        isAuthRequireException(reply.getException())) {
      ep->setAuthenticated(false);
      continue;
    }
    return error;
  }
  return error;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
