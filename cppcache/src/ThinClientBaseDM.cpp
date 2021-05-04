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

#include "ThinClientBaseDM.hpp"

#include <chrono>

#include <geode/AuthenticatedView.hpp>

#include "CacheImpl.hpp"
#include "TcrConnectionManager.hpp"
#include "ThinClientRegion.hpp"
#include "UserAttributes.hpp"

namespace apache {
namespace geode {
namespace client {

volatile bool ThinClientBaseDM::s_isDeltaEnabledOnServer = true;
const char* ThinClientBaseDM::NC_ProcessChunk = "NC ProcessChunk";

ThinClientBaseDM::ThinClientBaseDM(TcrConnectionManager& connManager,
                                   ThinClientRegion* theRegion)
    : m_region(theRegion),
      m_connManager(connManager),
      m_initDone(false),
      m_clientNotification(false),
      m_chunks(true),
      m_chunkProcessor(nullptr) {}

ThinClientBaseDM::~ThinClientBaseDM() noexcept = default;

void ThinClientBaseDM::init() {
  const auto& systemProperties = m_connManager.getCacheImpl()
                                     ->getDistributedSystem()
                                     .getSystemProperties();

  if (systemProperties.enableChunkHandlerThread()) {
    startChunkProcessor();
  }

  m_initDone = true;
}

bool ThinClientBaseDM::isSecurityOn() {
  return m_connManager.getCacheImpl()->getAuthInitialize() != nullptr;
}

void ThinClientBaseDM::destroy(bool) {
  if (!m_initDone) {
    // nothing to be done
    return;
  }
  // stop the chunk processing thread
  stopChunkProcessor();
  m_initDone = false;
}

GfErrType ThinClientBaseDM::sendSyncRequestRegisterInterest(
    TcrMessage& request, TcrMessageReply& reply, bool attemptFailover,
    ThinClientRegion*, TcrEndpoint* endpoint) {
  GfErrType err;
  if (endpoint == nullptr) {
    err = sendSyncRequest(request, reply, attemptFailover);
  } else {
    reply.setDM(this);
    if (endpoint->connected()) {
      err = sendRequestToEP(request, reply, endpoint);
    } else {
      err = GF_NOTCON;
    }
  }

  if (err == GF_NOERR) {
    switch (reply.getMessageType()) {
      case TcrMessage::REPLY:
      case TcrMessage::RESPONSE:
      case TcrMessage::RESPONSE_FROM_PRIMARY:
      case TcrMessage::RESPONSE_FROM_SECONDARY:
        break;

      case TcrMessage::EXCEPTION:
        err = ThinClientRegion::handleServerException("registerInterest",
                                                      reply.getException());
        break;

      case TcrMessage::REGISTER_INTEREST_DATA_ERROR:
        err = GF_CACHESERVER_EXCEPTION;
        break;

      case TcrMessage::UNREGISTER_INTEREST_DATA_ERROR:
        err = GF_CACHESERVER_EXCEPTION;
        break;

      default:
        LOG_ERROR(
            "Unknown message type %d during register subscription interest",
            reply.getMessageType());
        err = GF_MSG;
        break;
    }
  }

  // top level should only see NotConnectedException
  if (err == GF_IOERR) {
    err = GF_NOTCON;
  }
  return err;
}

GfErrType ThinClientBaseDM::handleEPError(TcrEndpoint* ep,
                                          TcrMessageReply& reply,
                                          GfErrType error) {
  if (error == GF_NOERR) {
    if (reply.getMessageType() == TcrMessage::EXCEPTION) {
      const auto& exceptStr = reply.getException();
      if (!exceptStr.empty()) {
        bool markServerDead = unrecoverableServerError(exceptStr);
        bool cacheClosedEx =
            (exceptStr.find("org.apache.geode.cache.CacheClosedException") !=
             std::string::npos);
        bool doFailover =
            (markServerDead || cacheClosedEx || nonFatalServerError(exceptStr));
        if (doFailover) {
          LOG_FINE(
              "ThinClientDistributionManager::sendRequestToEP: retrying for "
              "server [" +
              ep->name() + "] exception: " + exceptStr);
          error = GF_NOTCON;
          if (markServerDead) {
            ep->setConnectionStatus(false);
          }
        }
      }
    }
  }
  return error;
}

GfErrType ThinClientBaseDM::sendRequestToEndPoint(const TcrMessage& request,
                                                  TcrMessageReply& reply,
                                                  TcrEndpoint* ep) {
  LOG_DEBUG("ThinClientBaseDM::sendRequestToEP: invoking endpoint send for: %s",
            ep->name().c_str());
  auto error = ep->send(request, reply);
  LOG_DEBUG(
      "ThinClientBaseDM::sendRequestToEP: completed endpoint send for: %s "
      "[error:%d]",
      ep->name().c_str(), error);
  return handleEPError(ep, reply, error);
}

/**
 * If we receive an exception back from the server, we should retry on
 * other servers for some exceptions. Some exceptions indicate the server
 * is no longer usable while others indicate a temporary condition on
 * the server so that need not be marked as dead.
 * This method is for exceptions when server should be marked as dead.
 */
bool ThinClientBaseDM::unrecoverableServerError(const std::string& exceptStr) {
  return ((exceptStr.find("org.apache.geode.distributed.ShutdownException") !=
           std::string::npos) ||
          (exceptStr.find("java.lang.OutOfMemoryError") != std::string::npos));
}

/**
 * If we receive an exception back from the server, we should retry on
 * other servers for some exceptions. Some exceptions indicate the server
 * is no longer usable while others indicate a temporary condition on
 * the server so that need not be marked as dead.
 * This method is for exceptions when server should *not* be marked as dead.
 */
bool ThinClientBaseDM::nonFatalServerError(const std::string& exceptStr) {
  return (
      (exceptStr.find("org.apache.geode.distributed.TimeoutException") !=
       std::string::npos) ||
      (exceptStr.find("org.apache.geode.ThreadInterruptedException") !=
       std::string::npos) ||
      (exceptStr.find("java.lang.IllegalStateException") != std::string::npos));
}

void ThinClientBaseDM::failover() {}

void ThinClientBaseDM::queueChunk(TcrChunkedContext* chunk) {
  LOG_DEBUG("ThinClientBaseDM::queueChunk");
  if (m_chunkProcessor == nullptr) {
    LOG_DEBUG("ThinClientBaseDM::queueChunk2");
    // process in same thread if no chunk processor thread
    chunk->handleChunk(true);
    _GEODE_SAFE_DELETE(chunk);
  } else if (!m_chunks.putFor(chunk, std::chrono::seconds(1))) {
    LOG_DEBUG("ThinClientBaseDM::queueChunk3");
    // if put in queue fails due to whatever reason then process in same thread
    LOG_FINE(
        "addChunkToQueue: timed out while adding to queue of "
        "unbounded size after waiting for 1 secs");
    chunk->handleChunk(true);
    _GEODE_SAFE_DELETE(chunk);
  } else {
    LOG_DEBUG("Adding message to ThinClientBaseDM::queueChunk");
  }
}

// the chunk processing thread
void ThinClientBaseDM::processChunks(std::atomic<bool>& isRunning) {
  TcrChunkedContext* chunk;
  LOG_FINE("Starting chunk process thread for region %s",
           (m_region ? m_region->getFullPath().c_str() : "(null)"));

  std::chrono::milliseconds wait_for_chunk{100};
  chunk = m_chunks.getFor(wait_for_chunk);

  while (isRunning) {
    if (chunk) {
      chunk->handleChunk(false);
      _GEODE_SAFE_DELETE(chunk);
    }

    chunk = m_chunks.getFor(wait_for_chunk);
  }

  if (chunk) {
    _GEODE_SAFE_DELETE(chunk);
  }

  LOG_FINE("Ending chunk process thread for region %s",
           (m_region ? m_region->getFullPath().c_str() : "(null)"));
}

// start the chunk processing thread
void ThinClientBaseDM::startChunkProcessor() {
  if (m_chunkProcessor == nullptr) {
    m_chunks.open();
    m_chunkProcessor =
        std::unique_ptr<Task<ThinClientBaseDM>>(new Task<ThinClientBaseDM>(
            this, &ThinClientBaseDM::processChunks, NC_ProcessChunk));
    m_chunkProcessor->start();
  }
}

// stop the chunk processing thread
void ThinClientBaseDM::stopChunkProcessor() {
  if (m_chunkProcessor) {
    m_chunkProcessor->stop();
    m_chunks.close();
    m_chunkProcessor = nullptr;
  }
}

void ThinClientBaseDM::beforeSendingRequest(const TcrMessage& request,
                                            TcrConnection* conn) {
  LOG_DEBUG(
      "ThinClientBaseDM::beforeSendingRequest %d  "
      "TcrMessage::isUserInitiativeOps(request) = %d ",
      request.isMetaRegion(), TcrMessage::isUserInitiativeOps(request));
  LOG_DEBUG(
      "ThinClientBaseDM::beforeSendingRequest %d this->isMultiUserMode() = %d "
      "messageType = %d ",
      this->isSecurityOn(), this->isMultiUserMode(), request.getMessageType());
  if (!(request.isMetaRegion()) && TcrMessage::isUserInitiativeOps(request) &&
      (this->isSecurityOn() || this->isMultiUserMode())) {
    int64_t connId;
    int64_t uniqueId = 0;
    if (!this->isMultiUserMode()) {
      connId = conn->getConnectionId();
      uniqueId = conn->getEndpointObject()->getUniqueId();
    } else {
      connId = conn->getConnectionId();
      if (!(request.getMessageType() == TcrMessage::USER_CREDENTIAL_MESSAGE)) {
        uniqueId = UserAttributes::threadLocalUserAttributes
                       ->getConnectionAttribute(conn->getEndpointObject())
                       ->getUniqueId();
      }
    }

    if (request.getMessageType() == TcrMessage::USER_CREDENTIAL_MESSAGE) {
      auto* req = const_cast<TcrMessage*>(&request);
      req->createUserCredentialMessage(conn);
      req->addSecurityPart(connId, conn);
    } else if (TcrMessage::isUserInitiativeOps(request)) {
      auto* req = const_cast<TcrMessage*>(&request);
      req->addSecurityPart(connId, uniqueId, conn);
    }
  }
}

void ThinClientBaseDM::afterSendingRequest(const TcrMessage& request,
                                           TcrMessageReply& reply,
                                           TcrConnection* conn) {
  LOG_DEBUG("ThinClientBaseDM::afterSendingRequest reply msgtype = %d ",
            reply.getMessageType());
  if (!reply.isMetaRegion() && TcrMessage::isUserInitiativeOps(request) &&
      (this->isSecurityOn() || this->isMultiUserMode())) {
    // need to handle encryption/decryption
    if (request.getMessageType() == TcrMessage::USER_CREDENTIAL_MESSAGE) {
      if (TcrMessage::RESPONSE == reply.getMessageType()) {
        if (this->isMultiUserMode()) {
          UserAttributes::threadLocalUserAttributes->setConnectionAttributes(
              conn->getEndpointObject(), reply.getUniqueId());
        } else {
          conn->getEndpointObject()->setUniqueId(reply.getUniqueId());
        }
      }
      conn->setConnectionId(reply.getConnectionId());
    } else if (TcrMessage::isUserInitiativeOps(request)) {
      // bugfix: if noack op then reuse previous security token.
      conn->setConnectionId(reply.getMessageType() == TcrMessage::INVALID
                                ? conn->getConnectionId()
                                : reply.getConnectionId());
    }
  }
}

GfErrType ThinClientBaseDM::sendSyncRequestRegisterInterestEP(TcrMessage&,
                                                              TcrMessageReply&,
                                                              bool,
                                                              TcrEndpoint*) {
  return GF_NOERR;
}

GfErrType ThinClientBaseDM::registerInterestForRegion(TcrEndpoint*,
                                                      const TcrMessage*,
                                                      TcrMessageReply*) {
  return GF_NOERR;
}

bool ThinClientBaseDM::isEndpointAttached(TcrEndpoint*) { return false; }

bool ThinClientBaseDM::checkDupAndAdd(std::shared_ptr<EventId> eventid) {
  return m_connManager.checkDupAndAdd(eventid);
}

std::recursive_mutex& ThinClientBaseDM::getRedundancyLock() {
  return m_connManager.getRedundancyLock();
}

bool ThinClientBaseDM::isNotAuthorizedException(
    const std::string& exceptionMsg) {
  if (exceptionMsg.find("org.apache.geode.security.NotAuthorizedException") !=
      std::string::npos) {
    LOG_DEBUG("isNotAuthorizedException() An exception (" + exceptionMsg +
              ") happened at remote server.");
    return true;
  }
  return false;
}

bool ThinClientBaseDM::isPutAllPartialResultException(
    const std::string& exceptionMsg) {
  if (exceptionMsg.find(
          "org.apache.geode.internal.cache.PutAllPartialResultException") !=
      std::string::npos) {
    LOG_DEBUG("isNotAuthorizedException() An exception (" + exceptionMsg +
              ") happened at remote server.");
    return true;
  }
  return false;
}

bool ThinClientBaseDM::isAuthRequireException(const std::string& exceptionMsg) {
  if (exceptionMsg.find(
          "org.apache.geode.security.AuthenticationRequiredException") !=
      std::string::npos) {
    LOG_DEBUG("isAuthRequireExcep() An exception (" + exceptionMsg +
              ") happened at remote server.");
    return true;
  }
  return false;
}

void ThinClientBaseDM::setDeltaEnabledOnServer(bool isDeltaEnabledOnServer) {
  s_isDeltaEnabledOnServer = isDeltaEnabledOnServer;
  LOG_FINE("Delta enabled on server: %s",
           s_isDeltaEnabledOnServer ? "true" : "false");
}

}  // namespace client
}  // namespace geode
}  // namespace apache
