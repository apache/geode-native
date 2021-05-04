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

#include "ThinClientCacheDistributionManager.hpp"

#include <algorithm>

#include <geode/ExceptionTypes.hpp>

#include "CacheImpl.hpp"
#include "RemoteQueryService.hpp"
#include "TcrConnectionManager.hpp"
#include "TcrEndpoint.hpp"
#include "TcrMessage.hpp"
#include "ThinClientBaseDM.hpp"
#include "ThinClientRegion.hpp"

namespace apache {
namespace geode {
namespace client {

ThinClientCacheDistributionManager::ThinClientCacheDistributionManager(
    TcrConnectionManager& connManager)
    : ThinClientDistributionManager(connManager, nullptr) {}

void ThinClientCacheDistributionManager::init() {
  LOG_DEBUG("ThinClientCacheDistributionManager::init");
  if (m_connManager.getNumEndPoints() == 0) {
    throw IllegalStateException("No endpoints defined for query.");
  }
  ThinClientBaseDM::init();
}

GfErrType ThinClientCacheDistributionManager::sendSyncRequestCq(
    TcrMessage& request, TcrMessageReply& reply) {
  preFailoverAction();

  reply.setDM(this);

  std::lock_guard<decltype(m_endpointsLock)> guard(m_endpointsLock);

  // Return best effort result: If CQ succeeds on ANY server return no-error
  // even if
  // other servers might fail since this method is called in non-HA with
  // failover case.

  GfErrType err = GF_NOTCON;
  GfErrType opErr = GF_NOERR;

  for (std::vector<TcrEndpoint*>::iterator ep = m_endpoints.begin();
       ep != m_endpoints.end(); ++ep) {
    if ((*ep)->connected()) {
      (*ep)->setDM(this);
      opErr = sendRequestToEP(
          request, reply,
          *ep);  // this will go to ThinClientDistributionManager
      if (opErr == GF_NOERR ||
          (ThinClientBaseDM::isFatalClientError(opErr) && err != GF_NOERR)) {
        err = opErr;
      }
    }
  }

  // This should return only either NOERR (takes precedence), NOTCON or a
  // "client fatal error" such as NotAuthorized.
  return err;
}

GfErrType ThinClientCacheDistributionManager::sendSyncRequest(
    TcrMessage& request, TcrMessageReply& reply, bool attemptFailover, bool) {
  GfErrType err = GF_NOERR;
  int32_t type = request.getMessageType();
  if (m_connManager.haEnabled() &&
      (type == TcrMessage::EXECUTECQ_MSG_TYPE ||
       type == TcrMessage::STOPCQ_MSG_TYPE ||
       type == TcrMessage::CLOSECQ_MSG_TYPE ||
       type == TcrMessage::CLOSECLIENTCQS_MSG_TYPE ||
       type == TcrMessage::GETCQSTATS_MSG_TYPE ||
       type == TcrMessage::MONITORCQ_MSG_TYPE ||
       type == TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE ||
       type == TcrMessage::GETDURABLECQS_MSG_TYPE)) {
    err = m_connManager.sendSyncRequestCq(request, reply);
  } else if ((type == TcrMessage::EXECUTECQ_MSG_TYPE ||
              type == TcrMessage::STOPCQ_MSG_TYPE ||
              type == TcrMessage::CLOSECQ_MSG_TYPE ||
              type == TcrMessage::CLOSECLIENTCQS_MSG_TYPE ||
              type == TcrMessage::GETCQSTATS_MSG_TYPE ||
              type == TcrMessage::MONITORCQ_MSG_TYPE ||
              type == TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE ||
              type == TcrMessage::GETDURABLECQS_MSG_TYPE)) {
    err = sendSyncRequestCq(request, reply);
  } else {
    err = ThinClientDistributionManager::sendSyncRequest(request, reply,
                                                         attemptFailover);
  }
  return err;
}

GfErrType ThinClientCacheDistributionManager::sendRequestToPrimary(
    TcrMessage& request, TcrMessageReply& reply) {
  GfErrType err = GF_NOTCON;
  if (m_connManager.haEnabled()) {
    err = m_connManager.sendRequestToPrimary(request, reply);
  } else {
    //  Call sendSyncRequest() with failover enabled.
    // TODO: Must ensure that active endpoint is correctly tracked so that
    // request is sent to an endpoint
    // for which callback connection is present.
    err = ThinClientDistributionManager::sendSyncRequest(request, reply, true);
  }
  return err;
}

bool ThinClientCacheDistributionManager::preFailoverAction() {
  if (!m_initDone) {
    // nothing to be done if not initialized
    return true;
  }
  //  take the global endpoint lock so that the global endpoints list
  // does not change while we are (possibly) adding endpoint to this endpoints
  // list and incrementing the reference count of endpoint
  auto&& guard = m_connManager.getGlobalEndpoints().make_lock();
  //  This method is called at the time of failover to refresh
  // the list of endpoints.
  std::vector<TcrEndpoint*> currentGlobalEndpointsList;
  m_connManager.getAllEndpoints(currentGlobalEndpointsList);

  //  Update local list with new endpoints.
  std::vector<TcrEndpoint*> newEndpointsList;
  for (const auto& it : currentGlobalEndpointsList) {
    bool found = false;
    for (const auto& currIter : m_endpoints) {
      if (currIter == it) {
        found = true;
        break;
      }
    }
    if (!found) newEndpointsList.push_back(it);
  }

  for (const auto& ep : newEndpointsList) {
    m_endpoints.push_back(ep);
    ep->setNumRegions(ep->numRegions() + 1);
    LOG_FINER(
        "TCCDM: incremented region reference count for endpoint %s "
        "to %d",
        ep->name().c_str(), ep->numRegions());
  }

  return true;
}
bool ThinClientCacheDistributionManager::postFailoverAction(
    TcrEndpoint* endpoint) {
  LOG_DEBUG("ThinClientCacheDistributionManager : executeAllCqs");
  if (m_connManager.haEnabled()) {
    LOG_DEBUG(
        "ThinClientCacheDistributionManager : executeAllCqs HA: case, done "
        "else where");
    return true;
  }

  CacheImpl* cache = m_connManager.getCacheImpl();

  if (cache == nullptr) {
    LOG_ERROR("Client not initialized for failover");
    return false;
  }
  try {
    auto rqsService = std::dynamic_pointer_cast<RemoteQueryService>(
        cache->getQueryService(true));
    rqsService->executeAllCqs(true);
  } catch (const Exception& excp) {
    LOG_WARN(
        "Failed to recover CQs during failover attempt to endpoint[%s]: %s",
        endpoint->name().c_str(), excp.what());
    return false;
  } catch (...) {
    LOG_WARN("Failed to recover CQs during failover attempt to endpoint[%s]",
             endpoint->name().c_str());
    return false;
  }
  return true;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
