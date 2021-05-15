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

#include "TcrHADistributionManager.hpp"

#include <cassert>

#include <geode/ExceptionTypes.hpp>
#include <geode/internal/geode_globals.hpp>

#include "CacheImpl.hpp"
#include "RemoteQueryService.hpp"
#include "TcrEndpoint.hpp"
#include "TcrMessage.hpp"
#include "ThinClientHARegion.hpp"
#include "ThinClientRegion.hpp"
#include "Utils.hpp"

namespace apache {
namespace geode {
namespace client {

TcrHADistributionManager::TcrHADistributionManager(
    ThinClientRegion* theRegion, TcrConnectionManager& connManager)
    : ThinClientDistributionManager(connManager, theRegion),
      m_theTcrConnManager(connManager) {
  assert(theRegion != nullptr);
}

void TcrHADistributionManager::init() {
  // Calling base init().
  ThinClientDistributionManager::init();
}

bool TcrHADistributionManager::preFailoverAction() {
  return !m_region->isDestroyed();
}

bool TcrHADistributionManager::postFailoverAction(TcrEndpoint*) {
  // Trigger the redundancy thread.
  m_connManager.triggerRedundancyThread();
  return true;
}

GfErrType TcrHADistributionManager::registerInterestForRegion(
    TcrEndpoint* ep, const TcrMessage* request, TcrMessageReply* reply) {
  return m_region->registerKeys(ep, request, reply);
}

void TcrHADistributionManager::getEndpointNames(
    std::unordered_set<std::string>&) {}

GfErrType TcrHADistributionManager::sendRequestToEP(const TcrMessage& request,
                                                    TcrMessageReply& reply,
                                                    TcrEndpoint* endpoint) {
  LOG_DEBUG("TcrHADistributionManager::sendRequestToEP msgType[%d]",
            request.getMessageType());
  GfErrType err = GF_NOERR;
  reply.setDM(this);
  if (endpoint->connected()) {
    // err = ThinClientBaseDM::sendRequestToEndPoint( request, reply, endpoint
    // );
    err = ThinClientDistributionManager::sendRequestToEP(request, reply,
                                                         endpoint);
  } else {
    err = GF_NOTCON;
  }
  return err;
}
GfErrType TcrHADistributionManager::sendSyncRequestCq(TcrMessage& request,
                                                      TcrMessageReply& reply) {
  return m_connManager.sendSyncRequestCq(request, reply, this);
}

GfErrType TcrHADistributionManager::sendSyncRequestRegisterInterestEP(
    TcrMessage& request, TcrMessageReply& reply, bool attemptFailover,
    TcrEndpoint* endpoint) {
  return ThinClientBaseDM::sendSyncRequestRegisterInterest(
      request, reply, attemptFailover, nullptr, endpoint);
}

GfErrType TcrHADistributionManager::sendSyncRequestRegisterInterest(
    TcrMessage& request, TcrMessageReply& reply, bool attemptFailover,
    ThinClientRegion*, TcrEndpoint* endpoint) {
  return m_connManager.sendSyncRequestRegisterInterest(
      request, reply, attemptFailover, endpoint, this, m_region);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
