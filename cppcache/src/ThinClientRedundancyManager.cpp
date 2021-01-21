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
/*
 * ThinClientRedundancyManager.cpp
 *
 *  Created on: Dec 1, 2008
 *      Author: abhaware
 */

#include "ThinClientRedundancyManager.hpp"

#include <algorithm>
#include <set>

#include <geode/AuthenticatedView.hpp>
#include <geode/SystemProperties.hpp>

#include "CacheImpl.hpp"
#include "ClientProxyMembershipID.hpp"
#include "ExpiryHandler_T.hpp"
#include "RemoteQueryService.hpp"
#include "ServerLocation.hpp"
#include "TcrHADistributionManager.hpp"
#include "ThinClientLocatorHelper.hpp"
#include "ThinClientPoolHADM.hpp"
#include "ThinClientRegion.hpp"
#include "UserAttributes.hpp"

namespace apache {
namespace geode {
namespace client {

const int MIN_RETRY_ATTEMPTS = 5;

const char* ThinClientRedundancyManager::NC_PerodicACK = "NC PerodicACK";

ThinClientRedundancyManager::ThinClientRedundancyManager(
    TcrConnectionManager* theConnManager, int redundancyLevel,
    ThinClientPoolHADM* poolHADM, bool sentReadyForEvents,
    bool globalProcessedMarker)
    : m_globalProcessedMarker(globalProcessedMarker),
      m_IsAllEpDisCon(false),
      m_server(0),
      m_sentReadyForEvents(sentReadyForEvents),
      m_redundancyLevel(redundancyLevel),
      m_loggedRedundancyWarning(false),
      m_poolHADM(poolHADM),
      m_theTcrConnManager(theConnManager),
      m_locators(nullptr),
      m_servers(nullptr),
      m_periodicAckTask(nullptr),
      m_processEventIdMapTaskId(-1),
      m_nextAckInc(0),
      m_HAenabled(false) {}

std::list<ServerLocation> ThinClientRedundancyManager::selectServers(
    int howMany, std::set<ServerLocation> exclEndPts) {
  LOGFINE("Selecting %d servers with %d in exclude list", howMany,
          exclEndPts.size());

  std::list<ServerLocation> outEndpoints;

  if (m_locators->length() > 0) {
    try {
      std::string additionalLoc;
      ClientProxyMembershipID* m_proxyID = m_poolHADM->getMembershipId();
      m_poolHADM->getLocatorHelper()->getEndpointForNewCallBackConn(
          *m_proxyID, outEndpoints, additionalLoc, howMany, exclEndPts,
          m_poolHADM->getServerGroup());
    } catch (const AuthenticationRequiredException&) {
      return outEndpoints;
    } catch (const NoAvailableLocatorsException&) {
      LOGFINE("No locators available");
      return outEndpoints;
    }
  } else if (m_servers->length() > 0) {
    if (howMany == -1) howMany = m_servers->length();
    for (int attempts = 0; attempts < m_servers->length() && howMany > 0;
         attempts++) {
      if (m_server >= m_servers->length()) {
        m_server = 0;
      }
      ServerLocation location(Utils::convertHostToCanonicalForm(
                                  (*m_servers)[m_server++]->value().c_str())
                                  .c_str());
      if (exclEndPts.find(location) != exclEndPts.end()) {
        // exclude this one
        continue;
      }
      outEndpoints.push_back(location);
      howMany--;
    }
  } else {
    throw IllegalStateException(
        "No locators or servers available to select from");
  }
  return outEndpoints;
}

GfErrType ThinClientRedundancyManager::maintainRedundancyLevel(
    bool init, const TcrMessage* request, TcrMessageReply* reply,
    ThinClientRegion* region) {
  // Preconditions:
  // 1. m_redundantEndpoints UNION m_nonredundantEndpionts = All Endpoints
  // 2. m_redundantEndpoints INTERSECTION m_nonredundantEndpoints = Empty
  GfErrType err = GF_NOTCON;
  // save any fatal errors that occur during maintain redundancy so
  // that we can send it back to the caller, to avoid missing out due
  // to nonfatal errors such as server not available
  GfErrType fatalError = GF_NOERR;
  bool fatal = false;
  std::lock_guard<decltype(m_redundantEndpointsLock)> guard(
      m_redundantEndpointsLock);
  bool isRedundancySatisfied = false;
  int secondaryCount = 0;
  bool isPrimaryConnected = false;
  bool isPrimaryAtBack = false;
  // TODO: isPrimaryAtBack can be removed by simplifying
  // removeEndpointsInOrder().

  std::vector<TcrEndpoint*>::iterator itRedundant =
      m_redundantEndpoints.begin();
  std::vector<TcrEndpoint*> tempRedundantEndpoints;
  std::vector<TcrEndpoint*> tempNonredundantEndpoints;

  // Redundancy level is maintained as follows using the following vectors:
  // m_redundantEndpoints, m_nonredundantEndpoints, tempRedundantEndpoints,
  // tempNonredundantEndpoints.
  // m_redundantEndpoints and m_nonredundantEndpoints contain the current
  // primary-secondary and non-redundant endpoints respectively.
  // tempRedundantEndpoints and tempNonredundantEndpoints are vectors that are
  // used to adjust the status of endpoints:
  // tempRedundantEndpoints: contains endpoints which have been changed from
  // nonredundant to either primary or secondary, i.e.
  // those endpoints whose status has been changed in order to satisfy
  // redundancy level.
  // tempNonredundantEndpoints: contains endpoints which were earlier redundant
  // but are now not connected. These endpoints will
  // be moved from the redundant list to the non redundant list.

  // Step 1: Scan all the endpoints in one pass and arrange them according to
  // connection status in the following order:
  // m_redundantEndpoints,tempRedundantEndpoints,m_nonredundantEndpoints,tempNonredundantEndpoints
  // The endpoints are maintained in order in the redundant endpoints lists.
  // This order is maintained when the
  // endpoints in the temporary list are moved into
  // m_redundantEndpoints/m_nonredundantEndpoints.
  // Note that although endpoint status may change, the endpoints are not
  // deleted from the lists (instead, the endpoints are
  // copied to temporary lists).
  // Scanning the endpoints is done in two stages:
  // 1. First scan the redundant endpoints to find the failed endpoints list,
  // whether redundancy has been satisfied.
  // 2. If redundancy has not been satisfied, scan the nonredundant list to find
  // available endpoints that can be made redundant.

  LOGDEBUG(
      "ThinClientRedundancyManager::maintainRedundancyLevel(): checking "
      "redundant list, size = %zu",
      m_redundantEndpoints.size());
  while (!isRedundancySatisfied && itRedundant != m_redundantEndpoints.end()) {
    if (!isPrimaryConnected) {
      if (itRedundant == m_redundantEndpoints.begin()) {
        if ((*itRedundant)->connected()) {
          isPrimaryConnected = true;
          if (m_redundancyLevel == 0) isRedundancySatisfied = true;
        } else {
          tempNonredundantEndpoints.push_back(*itRedundant);
        }
      } else {
        if (sendMakePrimaryMesg(*itRedundant, request, region)) {
          isPrimaryConnected = true;
        } else {
          tempNonredundantEndpoints.push_back(*itRedundant);
        }
      }
    } else {
      if ((*itRedundant)->connected()) {
        secondaryCount++;
        if (secondaryCount == m_redundancyLevel) {
          isRedundancySatisfied = true;
        }
      } else {
        tempNonredundantEndpoints.push_back(*itRedundant);
      }
    }
    ++itRedundant;
  }

  // If redundancy is not satisfied, find nonredundant endpoints that can be
  // made redundant.
  // For queue locators, fetch an initial list of endpoints which can host
  // queues.
  if (!isRedundancySatisfied && m_poolHADM && !init) {
    LOGDEBUG(
        "ThinClientRedundancyManager::maintainRedundancyLevel(): building "
        "nonredundant list via pool.");

    std::list<ServerLocation> outEndpoints;
    std::set<ServerLocation> exclEndPts;
    for (std::vector<TcrEndpoint*>::iterator itr = m_redundantEndpoints.begin();
         itr != m_redundantEndpoints.end(); itr++) {
      LOGDEBUG(
          "ThinClientRedundancyManager::maintainRedundancyLevel(): excluding "
          "endpoint %s from queue list.",
          (*itr)->name().c_str());
      ServerLocation serverLoc((*itr)->name());
      exclEndPts.insert(serverLoc);
    }

    m_nonredundantEndpoints.clear();
    int howMany = -1;
    if (m_locators != nullptr && m_locators->length() > 0 &&
        m_servers != nullptr && m_servers->length() == 0) {
      // if we are using locators only request the required number of servers.
      howMany = m_redundancyLevel - static_cast<int>(exclEndPts.size()) + 1;
    }
    outEndpoints = selectServers(howMany, exclEndPts);
    for (std::list<ServerLocation>::iterator it = outEndpoints.begin();
         it != outEndpoints.end(); it++) {
      auto ep = m_poolHADM->addEP(*it);
      LOGDEBUG(
          "ThinClientRedundancyManager::maintainRedundancyLevel(): Adding "
          "endpoint %s to nonredundant list.",
          ep->name().c_str());
      m_nonredundantEndpoints.push_back(ep.get());
    }
  }

  LOGDEBUG(
      "ThinClientRedundancyManager::maintainRedundancyLevel(): finding "
      "nonredundant endpoints, size = %zu",
      m_nonredundantEndpoints.size());
  std::vector<TcrEndpoint*>::iterator itNonredundant =
      m_nonredundantEndpoints.begin();
  while (!isRedundancySatisfied &&
         itNonredundant != m_nonredundantEndpoints.end()) {
    if (!isPrimaryConnected) {
      if (secondaryCount == m_redundancyLevel) {
        // 38196:Make last endpoint from the non redundant list as primary.
        if ((!init || *itNonredundant == m_nonredundantEndpoints.back()) &&
            (err = makePrimary(*itNonredundant, request, reply)) == GF_NOERR) {
          tempRedundantEndpoints.push_back(*itNonredundant);
          isRedundancySatisfied = true;
          isPrimaryConnected = true;
          isPrimaryAtBack = true;
        } else {
          if (ThinClientBaseDM::isFatalError(err)) {
            fatal = true;
            fatalError = err;
          }
        }
      } else {
        if ((err = makeSecondary(*itNonredundant, request, reply)) ==
            GF_NOERR) {
          tempRedundantEndpoints.push_back(*itNonredundant);
          secondaryCount++;
        } else {
          if (ThinClientBaseDM::isFatalError(err)) {
            fatal = true;
            fatalError = err;
          }
        }
      }
    } else {
      if ((err = makeSecondary(*itNonredundant, request, reply)) == GF_NOERR) {
        tempRedundantEndpoints.push_back(*itNonredundant);
        secondaryCount++;
        if (secondaryCount == m_redundancyLevel) {
          isRedundancySatisfied = true;
        }
      } else {
        if (ThinClientBaseDM::isFatalError(err)) {
          fatal = true;
          fatalError = err;
        }
      }
    }
    ++itNonredundant;
  }
  // Step 2: After one scan of the endpoints, if the redundancy level is
  // satisifed by changing status of endpoints, the primary
  // endpoint will be present in either m_redundantEndpoints or
  // tempRedundantEndpoints.
  // However, when redundancy level is not satisifed and following condition
  // holds true, a new secondary server (whose status
  // changed in Step 1) will be made primary:
  // A. No primary server was found, and
  // B. secondaryCount <= redundancy level.

  // 38196: Prefer Primary conversion from oldHA .
  TcrEndpoint* convertedPrimary = nullptr;
  if (init && !isRedundancySatisfied && !isPrimaryConnected) {
    bool oldHAEndPointPresent = false;
    for (std::vector<TcrEndpoint*>::iterator it =
             tempRedundantEndpoints.begin();
         it != tempRedundantEndpoints.end(); it++) {
      if ((*it)->getServerQueueStatus() != NON_REDUNDANT_SERVER) {
        oldHAEndPointPresent = true;
        break;
      }
    }
    // TODO: Post-38196fix, simplify durable client initialization by removing
    // constraint on primary position.

    //  holds the endpoints that are skipped by the oldHAEndPointPresent
    // check in the loop back
    std::vector<TcrEndpoint*> tempSkippedEndpoints;
    // warning: do not use unsigned type for index since .size() can return 0
    while (!tempRedundantEndpoints.empty()) {
      TcrEndpoint* ep = tempRedundantEndpoints.back();
      if (oldHAEndPointPresent &&
          ep->getServerQueueStatus() == NON_REDUNDANT_SERVER) {
        tempSkippedEndpoints.push_back(ep);
        tempRedundantEndpoints.pop_back();
        continue;
      }
      if (sendMakePrimaryMesg(ep, request, region)) {
        // Primary may be in middle If there are older nonredundant
        // ep in tempRedundantEndpoints
        isPrimaryAtBack = false;
        convertedPrimary = ep;
        isPrimaryConnected = true;
        break;
      } else {
        tempRedundantEndpoints.pop_back();
      }
    }
    //  push back the skipped endpoints into tempRedundantEndpoints
    while (!tempSkippedEndpoints.empty()) {
      TcrEndpoint* ep = tempSkippedEndpoints.back();
      tempSkippedEndpoints.pop_back();
      tempRedundantEndpoints.push_back(ep);
    }
  }
  if (!isRedundancySatisfied && !isPrimaryConnected) {
    // warning: do not use unsigned type for index since .size() can return 0
    while (!tempRedundantEndpoints.empty()) {
      TcrEndpoint* ep = tempRedundantEndpoints.back();
      if (sendMakePrimaryMesg(ep, request, region)) {
        isPrimaryAtBack = true;
        isPrimaryConnected = true;
        break;
      } else {
        tempRedundantEndpoints.pop_back();
      }
    }
  }

  // Step 3: Finally, create the new redundant and nonredundant lists. Copy from
  // m_redundantEndpointsList all the endpoints that were
  // marked as disconnected. Add in order all the new redundant endpoints (whose
  // status changed in Step 2) to m_redundantEndpoints.
  // If primary was at end of temporary list, move it to front on redundant
  // list.
  // Similarly, adjust the nonredundant list.

  removeEndpointsInOrder(m_redundantEndpoints, tempNonredundantEndpoints);
  removeEndpointsInOrder(m_nonredundantEndpoints, tempRedundantEndpoints);

  // 38196:for DurableReconnect case, primary may be in between, put it @ start.
  if (init && !isPrimaryAtBack && convertedPrimary != nullptr) {
    moveEndpointToLast(tempRedundantEndpoints, convertedPrimary);
    isPrimaryAtBack = true;
  }

  addEndpointsInOrder(m_redundantEndpoints, tempRedundantEndpoints);

  if (isPrimaryConnected && isPrimaryAtBack) {
    TcrEndpoint* primary = m_redundantEndpoints.back();
    m_redundantEndpoints.pop_back();
    m_redundantEndpoints.insert(m_redundantEndpoints.begin(), primary);
  }

  // Unregister DM for the new non-redundant endpoints
  for (std::vector<TcrEndpoint*>::const_iterator iter =
           tempNonredundantEndpoints.begin();
       iter != tempNonredundantEndpoints.end(); ++iter) {
    (*iter)->unregisterDM(true);
  }
  addEndpointsInOrder(m_nonredundantEndpoints, tempNonredundantEndpoints);

  // Postconditions:
  // 1. If redundancy level is satisifed, m_redundantEndpoints.size = r + 1,
  // m_redundantEndpoints[0] is primary.
  // 2. If redundancy level is not satisifed, m_redundantEndpoints.size <= r.
  // 3. If primary is connected, m_redundantEndpoints[0] is primary. ( Not
  // checked. To verify, We may have to modify
  //    TcrEndpoint class.)
  std::shared_ptr<RemoteQueryService> queryServicePtr;
  ThinClientPoolDM* poolDM = dynamic_cast<ThinClientPoolDM*>(m_poolHADM);
  if (poolDM) {
    queryServicePtr = std::dynamic_pointer_cast<RemoteQueryService>(
        poolDM->getQueryServiceWithoutCheck());
  }
  if (queryServicePtr != nullptr) {
    if (isPrimaryConnected) {
      // call CqStatusListener connect
      LOGDEBUG(
          "invoke invokeCqConnectedListeners for connected for CQ status "
          "listener");
      queryServicePtr->invokeCqConnectedListeners(poolDM, true);
    } else {
      // call CqStatusListener disconnect
      LOGDEBUG(
          "invoke invokeCqDisConnectedListeners for disconnected for CQ status "
          "listener");
      queryServicePtr->invokeCqConnectedListeners(poolDM, false);
    }
  }

  // Invariants:
  // 1. m_redundantEndpoints UNION m_nonredundantEndpionts = All Endpoints
  // 2. m_redundantEndpoints INTERSECTION m_nonredundantEndpoints = Empty

  // The global endpoint list does not change ever for HA so getAllEndpoints
  // result or redundantEndpoints/nonredundantEndpoints cannot have stale or
  // deleted endpoints

  if (m_poolHADM) {
    m_poolHADM->getStats().setSubsServers(
        static_cast<int32_t>(m_redundantEndpoints.size()));
  }

  if (isRedundancySatisfied) {
    m_IsAllEpDisCon = false;
    m_loggedRedundancyWarning = false;
    return GF_NOERR;
  } else if (isPrimaryConnected) {
    if (fatal && err != GF_NOERR) {
      return fatalError;
    }
    m_IsAllEpDisCon = false;
    if (m_redundancyLevel == -1) {
      LOGINFO("Current subscription redundancy level is %zu",
              m_redundantEndpoints.size() - 1);
      return GF_NOERR;
    }
    if (!m_loggedRedundancyWarning) {
      LOGWARN(
          "Requested subscription redundancy level %d is not satisfiable with "
          "%zu servers available",
          m_redundancyLevel, m_redundantEndpoints.size());
      m_loggedRedundancyWarning = true;
    }
    return GF_NOERR;
  } else {
    // save any fatal errors that occur during maintain redundancy so
    // that we can send it back to the caller, to avoid missing out due
    // to nonfatal errors such as server not available
    if (m_poolHADM && !m_IsAllEpDisCon) {
      m_poolHADM->sendNotConMesToAllregions();
      m_IsAllEpDisCon = true;
    }
    if (fatal && err != GF_NOERR) {
      return fatalError;
    }
    return err;
  }
}

void ThinClientRedundancyManager::removeEndpointsInOrder(
    std::vector<TcrEndpoint*>& destVector,
    const std::vector<TcrEndpoint*>& srcVector) {
  std::vector<TcrEndpoint*> tempDestVector;
  std::vector<TcrEndpoint*>::iterator itDest;
  std::vector<TcrEndpoint*>::const_iterator itSrc;

  itSrc = srcVector.begin();
  while ((itDest = destVector.begin()) != destVector.end()) {
    if ((itSrc != srcVector.end()) && (*itDest == *itSrc)) {
      destVector.erase(itDest);
      ++itSrc;
    } else {
      tempDestVector.push_back(*itDest);
      destVector.erase(itDest);
    }
  }
  destVector = tempDestVector;

  // Postconditions:
  // 1. size of destVector decreases by the size of srcVector
}

void ThinClientRedundancyManager::addEndpointsInOrder(
    std::vector<TcrEndpoint*>& destVector,
    const std::vector<TcrEndpoint*>& srcVector) {
  destVector.insert(destVector.end(), srcVector.begin(), srcVector.end());

  // Postconditions:
  // 1. Length of destVector increases by the length of srcVector
}

GfErrType ThinClientRedundancyManager::createQueueEP(TcrEndpoint* ep,
                                                     const TcrMessage* request,
                                                     TcrMessageReply* reply,
                                                     bool isPrimary) {
  LOGFINE("Recovering subscriptions on endpoint [%s]", ep->name().c_str());
  GfErrType err = GF_NOERR;
  if ((err = ep->registerDM(true, !isPrimary)) == GF_NOERR) {
    if ((err = m_theTcrConnManager->registerInterestAllRegions(
             ep, request, reply)) != GF_NOERR ||
        !readyForEvents(ep)) {
      ep->unregisterDM(true);
      if (err == GF_NOERR) {
        err = GF_NOTCON;
      }
    } else {
      // recover CQs
      CacheImpl* cache = m_theTcrConnManager->getCacheImpl();
      auto rqsService = std::dynamic_pointer_cast<RemoteQueryService>(
          cache->getQueryService(true));
      if (rqsService != nullptr) {
        try {
          err = rqsService->executeAllCqs(ep);
        } catch (const Exception& excp) {
          LOGFINE("Failed to recover CQs on endpoint[%s]: %s",
                  ep->name().c_str(), excp.what());
          ep->unregisterDM(true);
          err = GF_NOTCON;
        } catch (...) {
          LOGFINE("Failed to recover CQs on endpoint[%s]", ep->name().c_str());
          ep->unregisterDM(true);
          err = GF_NOTCON;
        }
      }
    }
  }
  LOGFINE("Done subscription recovery");
  return err;
}

GfErrType ThinClientRedundancyManager::createPoolQueueEP(
    TcrEndpoint* ep, const TcrMessage* request, TcrMessageReply* reply,
    bool isPrimary) {
  LOGFINE("Recovering subscriptions on endpoint [" + ep->name() +
          "] from pool " + m_poolHADM->getName());

  GfErrType err = GF_NOERR;
  if ((err = ep->registerDM(false, !isPrimary, false, m_poolHADM)) ==
      GF_NOERR) {
    if ((err = m_poolHADM->registerInterestAllRegions(ep, request, reply)) !=
            GF_NOERR ||
        !readyForEvents(ep)) {
      ep->unregisterDM(false);
      if (err == GF_NOERR) {
        err = GF_NOTCON;
      }
    } else {
      // recover CQs
      auto rqsService = std::dynamic_pointer_cast<RemoteQueryService>(
          m_poolHADM->getQueryServiceWithoutCheck());
      if (rqsService != nullptr) {
        try {
          err = rqsService->executeAllCqs(ep);
        } catch (const Exception& excp) {
          LOGFINE("Failed to recover CQs on endpoint[" + ep->name() + "]: %s" +
                  excp.getMessage());
          ep->unregisterDM(false);  // Argument is useless
          err = GF_NOTCON;
        } catch (...) {
          LOGFINE("Failed to recover CQs on endpoint[" + ep->name() + "]");
          ep->unregisterDM(false);  // Argument is useless
          err = GF_NOTCON;
        }
      }
    }
  }
  LOGFINE("Done subscription recovery");
  return err;
}

GfErrType ThinClientRedundancyManager::makePrimary(TcrEndpoint* ep,
                                                   const TcrMessage* request,
                                                   TcrMessageReply* reply) {
  if (m_poolHADM) {
    return createPoolQueueEP(ep, request, reply, true);
  } else {
    return createQueueEP(ep, request, reply, true);
  }
}

GfErrType ThinClientRedundancyManager::makeSecondary(TcrEndpoint* ep,
                                                     const TcrMessage* request,
                                                     TcrMessageReply* reply) {
  if (m_poolHADM) {
    return createPoolQueueEP(ep, request, reply, false);
  } else {
    return createQueueEP(ep, request, reply, false);
  }
}

void ThinClientRedundancyManager::initialize(int redundancyLevel) {
  LOGDEBUG(
      "ThinClientRedundancyManager::initialize(): initializing redundancy "
      "manager.");
  LOGFINE("Subscription redundancy level set to %d %s %s", redundancyLevel,
          m_poolHADM != nullptr ? "for pool" : "",
          m_poolHADM != nullptr ? m_poolHADM->getName().c_str() : "");
  m_redundancyLevel = redundancyLevel;
  m_HAenabled = (redundancyLevel > 0 || m_theTcrConnManager->isDurable() ||
                 ThinClientBaseDM::isDeltaEnabledOnServer());
  auto& sysProp = m_theTcrConnManager->getCacheImpl()
                      ->getDistributedSystem()
                      .getSystemProperties();
  if (m_poolHADM) {
    m_eventidmap.init(m_poolHADM->getSubscriptionMessageTrackingTimeout());
  } else {
    m_eventidmap.init(sysProp.notifyDupCheckLife());
  }

  if (m_HAenabled) {
    std::chrono::milliseconds interval;
    if (m_poolHADM) {
      //  Set periodic ack interval in seconds.
      interval = m_poolHADM->getSubscriptionAckInterval();

    } else {
      interval = sysProp.notifyAckInterval();
    }
    if (interval < std::chrono::milliseconds(100)) {
      interval = std::chrono::milliseconds(100);
    }
    m_nextAckInc = interval;
    m_nextAck = clock::now() + interval;
  }

  if (m_poolHADM) {
    m_locators = m_poolHADM->getLocators();
    if (m_locators->length() == 0) m_servers = m_poolHADM->getServers();
    if (m_locators->length() > 0) {
      for (int item = 0; item < m_locators->length(); item++) {
        LOGDEBUG("ThinClientRedundancyManager::initialize: adding locator %s",
                 (*m_locators)[item]->value().c_str());
      }
    } else if (m_servers->length() > 0) {
      RandGen randgen;
      m_server = randgen(m_servers->length());
    } else {
      throw IllegalStateException(
          "The redundancy manager's pool does not have locators or servers "
          "specified");
    }
  }
  getAllEndpoints(m_nonredundantEndpoints);
}

void ThinClientRedundancyManager::sendNotificationCloseMsgs() {
  std::lock_guard<decltype(m_redundantEndpointsLock)> guard(
      m_redundantEndpointsLock);

  for (auto&& endpoint : m_redundantEndpoints) {
    LOGDEBUG(
        "ThinClientRedundancyManager::sendNotificationCloseMsgs(): closing "
        "notification for endpoint %s",
        endpoint->name().c_str());
    endpoint->stopNoBlock();
  }

  for (auto&& endpoint : m_redundantEndpoints) {
    LOGDEBUG(
        "ThinClientRedundancyManager::sendNotificationCloseMsgs(): closing "
        "receiver for endpoint %s",
        endpoint->name().c_str());
    endpoint->stopNotifyReceiverAndCleanup();
  }
}

void ThinClientRedundancyManager::close() {
  LOGDEBUG("ThinClientRedundancyManager::close(): closing redundancy manager.");

  if (m_periodicAckTask) {
    if (m_processEventIdMapTaskId >= 0) {
      m_theTcrConnManager->getCacheImpl()->getExpiryTaskManager().cancelTask(
          m_processEventIdMapTaskId);
    }
    m_periodicAckTask->stopNoblock();
    m_periodicAckSema.release();
    m_periodicAckTask->wait();
    m_periodicAckTask = nullptr;
  }

  std::lock_guard<decltype(m_redundantEndpointsLock)> guard(
      m_redundantEndpointsLock);

  for (auto&& endpoint : m_redundantEndpoints) {
    LOGDEBUG(
        "ThinClientRedundancyManager::close(): unregistering from endpoint %s",
        endpoint->name().c_str());
    endpoint->unregisterDM(true);
  }

  m_redundantEndpoints.clear();
  m_nonredundantEndpoints.clear();
  m_theTcrConnManager = nullptr;
  m_globalProcessedMarker = false;
  m_sentReadyForEvents = false;
}

bool ThinClientRedundancyManager::readyForEvents(
    TcrEndpoint* primaryCandidate) {
  if (!m_theTcrConnManager->isDurable() || !m_sentReadyForEvents) {
    return true;
  }

  TcrMessageClientReady request(
      new DataOutput(m_theTcrConnManager->getCacheImpl()->createDataOutput()));
  TcrMessageReply reply(true, nullptr);

  GfErrType err = GF_NOTCON;
  if (m_poolHADM) {
    err = m_poolHADM->sendRequestToEP(request, reply, primaryCandidate);
  } else {
    err = ThinClientBaseDM::sendRequestToEndPoint(request, reply,
                                                  primaryCandidate);
  }
  if (err == GF_NOERR) {
    return true;
  } else {
    return false;
  }
}

void ThinClientRedundancyManager::moveEndpointToLast(
    std::vector<TcrEndpoint*>& epVector, TcrEndpoint* targetEp) {
  // Pre-condition

  // Remove Ep
  for (std::vector<TcrEndpoint*>::iterator it = epVector.begin();
       it != epVector.end(); it++) {
    if (targetEp == *it) {
      epVector.erase(it);
      break;
    }
  }
  // Push it @ end.
  epVector.push_back(targetEp);
}

bool ThinClientRedundancyManager::sendMakePrimaryMesg(TcrEndpoint* ep,
                                                      const TcrMessage*,
                                                      ThinClientRegion*) {
  if (!ep->connected()) {
    return false;
  }
  TcrMessageReply reply(false, nullptr);
  const TcrMessageMakePrimary makePrimaryRequest(
      new DataOutput(m_theTcrConnManager->getCacheImpl()->createDataOutput()),
      ThinClientRedundancyManager::m_sentReadyForEvents);

  LOGFINE("Making primary subscription endpoint %s", ep->name().c_str());
  GfErrType err = GF_NOTCON;
  if (m_poolHADM) {
    err = m_poolHADM->sendRequestToEP(makePrimaryRequest, reply, ep);
  } else {
    err =
        ThinClientBaseDM::sendRequestToEndPoint(makePrimaryRequest, reply, ep);
  }
  if (err == GF_NOERR) {
    /*  this causes keys to be added to the region even if the reg interest
     * is supposed to fail due to notauthorized exception then causing
     * subsequent maintainredundancy calls to fail for other ops like CQ
    if ( request != nullptr && region != nullptr ) {
      const std::vector<std::shared_ptr<CacheableKey>> * keys =
    request->getKeys( ); bool isDurable = request->isDurable( ); if ( keys ==
    nullptr || keys->empty( ) ) { const std::string& regex = request->getRegex(
    ); if ( !regex.empty( ) ) { region->addRegex( regex, isDurable );
        }
      } else {
        region->addKeys( *keys, isDurable );
      }
    }
    */
    return true;
  } else {
    return false;
  }
}

GfErrType ThinClientRedundancyManager::sendSyncRequestCq(
    TcrMessage& request, TcrMessageReply& reply, ThinClientBaseDM* theHADM) {
  LOGDEBUG("ThinClientRedundancyManager::sendSyncRequestCq msgType[%d]",
           request.getMessageType());
  std::lock_guard<decltype(m_redundantEndpointsLock)> guard(
      m_redundantEndpointsLock);

  GfErrType err = GF_NOERR;
  GfErrType opErr;
  TcrEndpoint* primaryEndpoint = nullptr;

  if (m_redundantEndpoints.size() >= 1) {
    LOGDEBUG(
        "ThinClientRedundancyManager::sendSyncRequestCq: to secondary "
        "size[%zu]",
        m_redundantEndpoints.size());
    std::vector<TcrEndpoint*>::iterator iter = m_redundantEndpoints.begin();
    LOGDEBUG("endpoint[%s]", (*iter)->name().c_str());
    for (++iter; iter != m_redundantEndpoints.end(); ++iter) {
      LOGDEBUG("endpoint[%s]", (*iter)->name().c_str());
      LOGDEBUG(
          "msgType[%d] ThinClientRedundancyManager::sendSyncRequestCq: to "
          "secondary",
          request.getMessageType());
      if (request.getMessageType() == TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE) {
        // Timeout for this message type is set like so...
        reply.setTimeout(
            dynamic_cast<ThinClientPoolDM*>(theHADM)->getReadTimeout());
        opErr = theHADM->sendRequestToEP(request, reply, *iter);
      } else {
        opErr = theHADM->sendRequestToEP(request, reply, *iter);
      }
      if (err == GF_NOERR) {
        err = opErr;
      }
    }
    primaryEndpoint = m_redundantEndpoints[0];
    LOGDEBUG("primary endpoint[%s]", primaryEndpoint->name().c_str());
  }

  int32_t attempts = static_cast<int32_t>(m_redundantEndpoints.size()) +
                     static_cast<int32_t>(m_nonredundantEndpoints.size());
  attempts = attempts < MIN_RETRY_ATTEMPTS
                 ? MIN_RETRY_ATTEMPTS
                 : attempts;  // at least 5 attempts if ep lists are small.

  AuthenticatedView* authenticatedView = nullptr;

  while (attempts--) {
    if (err != GF_NOERR || m_redundantEndpoints.empty()) {
      auto userAttr = UserAttributes::threadLocalUserAttributes;
      if (userAttr) {
        authenticatedView = userAttr->getAuthenticatedView();
      }
      err = maintainRedundancyLevel();
      // we continue on fatal error because MRL only tries a handshake without
      // sending a request (no params passed) so no need to check
      // isFatalClientError.
      if (theHADM->isFatalError(err) && m_redundantEndpoints.empty()) {
        continue;
      }
    }

    if (m_redundantEndpoints.empty()) {
      err = GF_NOTCON;
    } else {
      primaryEndpoint = m_redundantEndpoints[0];
      LOGDEBUG(
          "ThinClientRedundancyManager::sendSyncRequestCq: to primary [%s]",
          primaryEndpoint->name().c_str());
      GuardUserAttributes gua;
      if (authenticatedView != nullptr) {
        gua.setAuthenticatedView(authenticatedView);
      }
      err = theHADM->sendRequestToEP(request, reply, primaryEndpoint);
      if (err == GF_NOERR || err == GF_TIMEOUT ||
          ThinClientBaseDM::isFatalClientError(err)) {
        break;
      }
    }
  }

  // top level should only get NotConnectedException
  if (err == GF_IOERR) {
    err = GF_NOTCON;
  }

  return err;
}

GfErrType ThinClientRedundancyManager::sendSyncRequestRegisterInterest(
    TcrMessage& request, TcrMessageReply& reply, bool, TcrEndpoint* endpoint,
    ThinClientBaseDM* theHADM, ThinClientRegion* region) {
  LOGDEBUG("ThinClientRedundancyManager::sendSyncRequestRegisterInterest ");
  if (!endpoint) {
    std::lock_guard<decltype(m_redundantEndpointsLock)> guard(
        m_redundantEndpointsLock);

    auto err = GF_NOERR;
    auto opErr = GF_NOERR;
    TcrEndpoint* primaryEndpoint = nullptr;

    if (!m_redundantEndpoints.empty()) {
      for (auto&& redundantEndpoint : m_redundantEndpoints) {
        redundantEndpoint->setDM(request.getDM());
        opErr = theHADM->sendSyncRequestRegisterInterestEP(
            request, reply, false, redundantEndpoint);
        if (err == GF_NOERR) {
          err = opErr;
        }
      }
      primaryEndpoint = m_redundantEndpoints[0];
    }

    if ((request.getMessageType() == TcrMessage::REGISTER_INTEREST_LIST ||
         request.getMessageType() == TcrMessage::REGISTER_INTEREST ||
         request.getMessageType() == TcrMessage::UNREGISTER_INTEREST_LIST ||
         request.getMessageType() == TcrMessage::UNREGISTER_INTEREST) &&
        (err != GF_NOERR ||
         static_cast<int>(m_redundantEndpoints.size()) <= m_redundancyLevel)) {
      err = maintainRedundancyLevel(false, &request, &reply, region);
      if (theHADM->isFatalClientError(err)) {
        return err;
      }
    }

    // We need to ensure that primary registration is done at the end.
    // So we skip the primary registration when maintainRedundancyLevel()
    // has already done that, i.e. primary endpoint not the same as that
    // before calling maintainRedundancyLevel()

    if (m_redundantEndpoints.empty()) {
      err = GF_NOTCON;
    } else if (primaryEndpoint && primaryEndpoint == m_redundantEndpoints[0]) {
      for (size_t count = 0;
           count < m_redundantEndpoints.size() + m_nonredundantEndpoints.size();
           count++) {
        primaryEndpoint->setDM(request.getDM());
        opErr = theHADM->sendSyncRequestRegisterInterestEP(
            request, reply, false, primaryEndpoint);
        if (opErr == GF_NOERR) {
          break;
        } else {
          err = maintainRedundancyLevel(false, &request, &reply, region);
          primaryEndpoint = m_redundantEndpoints[0];
        }
      }
      if (theHADM->isFatalError(opErr)) {
        err = opErr;
      }
    }
    return err;
  } else {
    return theHADM->sendSyncRequestRegisterInterestEP(request, reply, false,
                                                      endpoint);
  }
}

synchronized_map<std::unordered_map<std::string, std::shared_ptr<TcrEndpoint>>,
                 std::recursive_mutex>&
ThinClientRedundancyManager::updateAndSelectEndpoints() {
  // 38196 Fix: For durable clients reconnect
  // 1. Get list of endpoints which have HA queue.
  // 2. Get HA endpoint with max queuesize;
  // 3. Move other HA endpoints at start of list in ascending order of
  // queuesize.
  // 4. Add HA endpoint with Max queueSize at the end.
  // Primary : Secondaries in ascending order of QSiz : Nonredundant : Secondary
  // with QSizemax
  // e.g. R[#70]:Old Primary,  R [#80] , R[#90], NR.... , R[#100]:New Primary
  // Exception: For R =0 ( or when no EP with Max queuesize ),
  //  Old primary would be considered as new. Hence it would be at the end

  if (m_poolHADM) {
    // fetch queue servers
    // send queue servers for sorting
    std::set<ServerLocation> exclEndPts;
    std::list<ServerLocation> outEndpoints;

    outEndpoints = selectServers(-1, exclEndPts);
    for (auto& it : outEndpoints) {
      m_poolHADM->addEP(it);
    }

    return m_poolHADM->m_endpoints;
  } else {
    return m_theTcrConnManager->m_endpoints;
  }
}

void ThinClientRedundancyManager::getAllEndpoints(
    std::vector<TcrEndpoint*>& endpoints) {
  std::shared_ptr<TcrEndpoint> maxQEp, primaryEp;

  auto& selectedEndpoints = updateAndSelectEndpoints();
  for (const auto& currItr : selectedEndpoints) {
    if (isDurable()) {
      auto ep = currItr.second;
      int32_t queueSize = 0;
      TcrConnection* statusConn = nullptr;
      auto status =
          ep->getFreshServerQueueStatus(queueSize, !m_poolHADM, statusConn);
      if (m_poolHADM && status != NON_REDUNDANT_SERVER) {
        m_poolHADM->addConnection(statusConn);
      }
      if (status == REDUNDANT_SERVER) {
        if (!maxQEp) {
          maxQEp = ep;
        } else if (ep->getServerQueueSize() > maxQEp->getServerQueueSize()) {
          insertEPInQueueSizeOrder(maxQEp.get(), endpoints);
          maxQEp = ep;
        } else {
          insertEPInQueueSizeOrder(ep.get(), endpoints);
        }
        LOGDEBUG(
            "ThinClientRedundancyManager::getAllEndpoints(): sorting "
            "endpoints, found redundant endpoint.");
      } else if (status == PRIMARY_SERVER) {
        // Primary should be unique
        primaryEp = ep;
        LOGDEBUG(
            "ThinClientRedundancyManager::getAllEndpoints(): sorting "
            "endpoints, found primary endpoint.");
      } else {
        endpoints.push_back(currItr.second.get());
        LOGDEBUG(
            "ThinClientRedundancyManager::getAllEndpoints(): sorting "
            "endpoints, found nonredundant endpoint.");
      }
    } else {
      endpoints.push_back(currItr.second.get());
    }
    //(*currItr)++;
  }

  // Add Endpoint with Max Queuesize at the last and Primary at first position
  if (isDurable()) {
    if (maxQEp) {
      endpoints.push_back(maxQEp.get());
      LOGDEBUG(
          "ThinClientRedundancyManager::getAllEndpoints(): sorting endpoints, "
          "pushing max-q endpoint at back.");
    }
    if (primaryEp) {
      if (m_redundancyLevel == 0 || !maxQEp) {
        endpoints.push_back(primaryEp.get());
        LOGDEBUG(
            "ThinClientRedundancyManager::getAllEndpoints(): sorting "
            "endpoints, pushing primary at back.");
      } else {
        endpoints.insert(endpoints.begin(), primaryEp.get());
        LOGDEBUG(
            "ThinClientRedundancyManager::getAllEndpoints(): sorting "
            "endpoints, inserting primary at head.");
      }
    }
  } else {
    RandGen randgen;
    std::random_shuffle(endpoints.begin(), endpoints.end(), randgen);
  }
}

void ThinClientRedundancyManager::insertEPInQueueSizeOrder(
    TcrEndpoint* inputEp, std::vector<TcrEndpoint*>& endpoints) {
  // need to sort out redundant ep in ascending order. other eps will be at
  // back.
  std::vector<TcrEndpoint*>::iterator it = endpoints.begin();
  while (it != endpoints.end()) {
    TcrEndpoint* thisEp = *it;
    if ((thisEp->getServerQueueSize() >= inputEp->getServerQueueSize()) ||
        (thisEp->getServerQueueStatus() != REDUNDANT_SERVER)) {
      break;
    }
    it++;
  }
  endpoints.insert(it, inputEp);
}

bool ThinClientRedundancyManager::isDurable() {
  return m_theTcrConnManager->isDurable();
}

void ThinClientRedundancyManager::readyForEvents() {
  TcrMessageClientReady request(
      new DataOutput(m_theTcrConnManager->getCacheImpl()->createDataOutput()));
  TcrMessageReply reply(true, nullptr);
  GfErrType result = GF_NOTCON;
  unsigned int epCount = 0;

  std::lock_guard<decltype(m_redundantEndpointsLock)> guard(
      m_redundantEndpointsLock);

  if (m_sentReadyForEvents) {
    throw IllegalStateException("Already called readyForEvents");
  }

  TcrEndpoint* primary = nullptr;
  if (m_redundantEndpoints.size() > 0) {
    primary = m_redundantEndpoints[0];
    if (m_poolHADM) {
      result = m_poolHADM->sendRequestToEP(request, reply, primary);
    } else {
      result = primary->send(request, reply);
    }
  }

  epCount++;

  while (result != GF_NOERR && epCount <= m_redundantEndpoints.size() +
                                              m_nonredundantEndpoints.size()) {
    TcrMessageReply tcrMessageReply(true, nullptr);
    maintainRedundancyLevel();
    if (m_redundantEndpoints.size() > 0) {
      primary = m_redundantEndpoints[0];
      if (m_poolHADM) {
        result = m_poolHADM->sendRequestToEP(request, tcrMessageReply, primary);
      } else {
        result = primary->send(request, tcrMessageReply);
      }
    }
    epCount++;
  }

  if (result != GF_NOERR) {
    throw NotConnectedException("No endpoints available");
  }

  m_sentReadyForEvents = true;
}

int ThinClientRedundancyManager::processEventIdMap(const ACE_Time_Value&,
                                                   const void*) {
  m_periodicAckSema.release();
  return 0;
}

void ThinClientRedundancyManager::periodicAck(std::atomic<bool>& isRunning) {
  while (isRunning) {
    m_periodicAckSema.acquire();
    if (isRunning) {
      doPeriodicAck();
      while (m_periodicAckSema.tryacquire() != -1) {
        ;
      }
    }
  }
}

void ThinClientRedundancyManager::doPeriodicAck() {
  LOGDEBUG(
      "ThinClientRedundancyManager::processEventIdMap( ): Examining eventid "
      "map.");
  // do periodic ack if HA is enabled and the time has come
  if (m_HAenabled && (m_nextAck < clock::now())) {
    LOGFINER("Doing periodic ack");
    m_nextAck += m_nextAckInc;

    auto entries = m_eventidmap.getUnAcked();
    auto count = entries.size();
    if (count > 0) {
      bool acked = false;

      std::lock_guard<decltype(m_redundantEndpointsLock)> guard(
          m_redundantEndpointsLock);

      auto endpoint = m_redundantEndpoints.begin();
      if (endpoint != m_redundantEndpoints.end()) {
        TcrMessagePeriodicAck request(
            new DataOutput(
                m_theTcrConnManager->getCacheImpl()->createDataOutput()),
            entries);
        TcrMessageReply reply(true, nullptr);

        GfErrType result = GF_NOERR;
        if (m_poolHADM) {
          result = m_poolHADM->sendRequestToEP(request, reply, *endpoint);
        } else {
          result = (*endpoint)->send(request, reply);
        };

        if (result == GF_NOERR && reply.getMessageType() == TcrMessage::REPLY) {
          LOGFINE(
              "Sent subscription ack message for %zu sources to endpoint %s",
              count, (*endpoint)->name().c_str());
          acked = true;
        } else {
          LOGWARN(
              "Failure sending subscription ack message for %zu sources to "
              "endpoint %s",
              count, (*endpoint)->name().c_str());
          LOGFINER("Ack result is %d and reply message type is %d", result,
                   reply.getMessageType());
        }
      } else {
        LOGWARN(
            "No subscription servers available for periodic ack for %zu "
            "sources",
            count);
      }

      if (!acked) {
        // clear entries' acked flag for next periodic ack
        m_eventidmap.clearAckedFlags(entries);
      }
    }
  }

  // check the event id map for expiry
  uint32_t expired = m_eventidmap.expire(m_HAenabled /* onlyacked */);

  if (expired > 0) {
    LOGFINE("Expired %d sources from subscription map", expired);
  }
}

void ThinClientRedundancyManager::startPeriodicAck() {
  m_periodicAckTask = std::unique_ptr<Task<ThinClientRedundancyManager>>(
      new Task<ThinClientRedundancyManager>(
          this, &ThinClientRedundancyManager::periodicAck, NC_PerodicACK));
  m_periodicAckTask->start();
  const auto& props = m_theTcrConnManager->getCacheImpl()
                          ->getDistributedSystem()
                          .getSystemProperties();
  // start the periodic ACK task handler
  auto periodicAckTask = new ExpiryHandler_T<ThinClientRedundancyManager>(
      this, &ThinClientRedundancyManager::processEventIdMap);
  m_processEventIdMapTaskId =
      m_theTcrConnManager->getCacheImpl()
          ->getExpiryTaskManager()
          .scheduleExpiryTask(periodicAckTask, m_nextAckInc, m_nextAckInc,
                              false);
  LOGFINE(
      "Registered subscription event "
      "periodic ack task with id = %ld, notify-ack-interval = %ld, "
      "notify-dupcheck-life = %ld, periodic ack is %sabled",
      m_processEventIdMapTaskId,
      (m_poolHADM ? m_poolHADM->getSubscriptionAckInterval()
                  : props.notifyAckInterval())
          .count(),
      (m_poolHADM ? m_poolHADM->getSubscriptionMessageTrackingTimeout()
                  : props.notifyDupCheckLife())
          .count(),
      m_HAenabled ? "en" : "dis");
}

// notification dup check with the help of eventidmap - called by
// ThinClientRegion
bool ThinClientRedundancyManager::checkDupAndAdd(
    std::shared_ptr<EventId> eventid) {
  EventIdMapEntry entry = EventIdMap::make(eventid);
  return m_eventidmap.put(entry.first, entry.second, true);
}

void ThinClientRedundancyManager::netDown() {
  std::lock_guard<decltype(m_redundantEndpointsLock)> guard(
      m_redundantEndpointsLock);

  if (!m_poolHADM) {
    m_nonredundantEndpoints.insert(m_nonredundantEndpoints.end(),
                                   m_redundantEndpoints.begin(),
                                   m_redundantEndpoints.end());
  }
  m_redundantEndpoints.clear();
}

void ThinClientRedundancyManager::removeCallbackConnection(TcrEndpoint* ep) {
  std::lock_guard<decltype(m_redundantEndpointsLock)> guard(
      m_redundantEndpointsLock);

  ep->unregisterDM(false, nullptr, true);
}
GfErrType ThinClientRedundancyManager::sendRequestToPrimary(
    TcrMessage& request, TcrMessageReply& reply) {
  std::lock_guard<decltype(m_redundantEndpointsLock)> guard(
      m_redundantEndpointsLock);
  GfErrType err = GF_NOTCON;
  for (size_t count = 0;
       count <= m_redundantEndpoints.size() + m_nonredundantEndpoints.size();
       count++) {
    if (m_poolHADM) {
      err =
          m_poolHADM->sendRequestToEP(request, reply, m_redundantEndpoints[0]);
    } else {
      err = m_redundantEndpoints[0]->send(request, reply);
    }
    if (err == GF_NOERR) break;
    maintainRedundancyLevel();
  }
  return err;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
