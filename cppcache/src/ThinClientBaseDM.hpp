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

#pragma once

#ifndef GEODE_THINCLIENTBASEDM_H_
#define GEODE_THINCLIENTBASEDM_H_

#include <memory>
#include <vector>

#include <geode/internal/geode_globals.hpp>

#include "Queue.hpp"
#include "Task.hpp"
#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

class TcrMessage;
class ThinClientRegion;
class TcrEndpoint;
class TcrConnectionManager;
class TcrMessageReply;
class TcrChunkedContext;
class EventId;

/**
 * @brief Distribute data between caches
 */
class ThinClientBaseDM {
 public:
  ThinClientBaseDM(TcrConnectionManager& connManager, ThinClientRegion* region);
  virtual ~ThinClientBaseDM() = 0;

  virtual void init();
  virtual void destroy(bool keepalive = false);

  virtual GfErrType sendSyncRequest(TcrMessage& request, TcrMessageReply& reply,
                                    bool attemptFailover = true,
                                    bool isBGThrad = false) = 0;

  virtual GfErrType sendSyncRequestRegisterInterest(
      TcrMessage& request, TcrMessageReply& reply, bool attemptFailover = true,
      ThinClientRegion* theRegion = nullptr, TcrEndpoint* endpoint = nullptr);

  virtual GfErrType sendSyncRequestRegisterInterestEP(TcrMessage& request,
                                                      TcrMessageReply& reply,
                                                      bool attemptFailover,
                                                      TcrEndpoint* endpoint);

  virtual void failover();

  virtual void acquireFailoverLock();
  virtual void releaseFailoverLock();
  virtual void acquireRedundancyLock();
  virtual void releaseRedundancyLock();
  virtual void triggerRedundancyThread();
  virtual bool isSecurityOn();

  virtual bool isMultiUserMode();

  virtual void beforeSendingRequest(const TcrMessage& request,
                                    TcrConnection* conn);
  virtual void afterSendingRequest(const TcrMessage& request,
                                   TcrMessageReply& reply, TcrConnection* conn);

  virtual GfErrType registerInterestForRegion(TcrEndpoint* ep,
                                              const TcrMessage* request,
                                              TcrMessageReply* reply);

  static bool isFatalError(GfErrType err);

  static bool isFatalClientError(GfErrType err);

  // add a new chunk to the queue
  void queueChunk(TcrChunkedContext* chunk);

  virtual bool isEndpointAttached(TcrEndpoint* ep);

  static GfErrType sendRequestToEndPoint(const TcrMessage& request,
                                         TcrMessageReply& reply,
                                         TcrEndpoint* currentEndpoint);

  virtual GfErrType sendRequestToEP(const TcrMessage& request,
                                    TcrMessageReply& reply,
                                    TcrEndpoint* currentEndpoint) = 0;

  virtual TcrEndpoint* getActiveEndpoint();

  virtual bool checkDupAndAdd(std::shared_ptr<EventId> eventid);

  virtual std::recursive_mutex& getRedundancyLock();

  static bool isDeltaEnabledOnServer();

  static void setDeltaEnabledOnServer(bool isDeltaEnabledOnServer);
  TcrConnectionManager& getConnectionManager() const;

  virtual size_t getNumberOfEndPoints() const;

  bool isNotAuthorizedException(const char* exceptionMsg);

  bool isPutAllPartialResultException(const char* exceptionMsg);

 protected:
  bool isAuthRequireException(const char* exceptionMsg);

  ThinClientRegion* m_region;

  // methods for the chunk processing thread
  void processChunks(std::atomic<bool>& isRunning);
  void startChunkProcessor();
  void stopChunkProcessor();

 private:
  // Disallow copy constructor and assignment operator.
  ThinClientBaseDM(const ThinClientBaseDM&);
  ThinClientBaseDM& operator=(const ThinClientBaseDM&);

 protected:
  static bool unrecoverableServerError(const char* exceptStr);
  static bool nonFatalServerError(const char* exceptStr);
  static GfErrType handleEPError(TcrEndpoint* ep, TcrMessageReply& reply,
                                 GfErrType error);

  TcrConnectionManager& m_connManager;
  // flag to indicate whether initialization completed successfully
  bool m_initDone;
  bool m_clientNotification;

  Queue<TcrChunkedContext*> m_chunks;
  std::unique_ptr<Task<ThinClientBaseDM>> m_chunkProcessor;

 private:
  static volatile bool s_isDeltaEnabledOnServer;
  static const char* NC_ProcessChunk;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_THINCLIENTBASEDM_H_
