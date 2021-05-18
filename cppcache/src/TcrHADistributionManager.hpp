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

#ifndef GEODE_TCRHADISTRIBUTIONMANAGER_H_
#define GEODE_TCRHADISTRIBUTIONMANAGER_H_

#include <geode/internal/geode_base.hpp>

#include "TcrConnectionManager.hpp"
#include "ThinClientDistributionManager.hpp"

namespace apache {
namespace geode {
namespace client {

class ThinClientRegion;
class ThinClientHARegion;
class TcrEndpoint;

/**
 * @brief Distribute data between caches
 */
class TcrHADistributionManager : public ThinClientDistributionManager {
 public:
  TcrHADistributionManager(ThinClientRegion* theRegion,
                           TcrConnectionManager& connManager);
  ~TcrHADistributionManager() override = default;
  TcrHADistributionManager(const TcrHADistributionManager&) = delete;
  TcrHADistributionManager& operator=(const TcrHADistributionManager&) = delete;

  void init() override;

  GfErrType registerInterestForRegion(TcrEndpoint* ep,
                                      const TcrMessage* request,
                                      TcrMessageReply* reply) override;

  GfErrType sendSyncRequestRegisterInterestEP(TcrMessage& request,
                                              TcrMessageReply& reply,
                                              bool attemptFailover,
                                              TcrEndpoint* endpoint) override;

  GfErrType sendRequestToEP(const TcrMessage& request, TcrMessageReply& reply,
                            TcrEndpoint* endpoint) override;

  ThinClientRegion* getRegion() { return m_region; }

  void acquireRedundancyLock() override {
    m_connManager.acquireRedundancyLock();
  }

  void releaseRedundancyLock() override {
    m_connManager.releaseRedundancyLock();
  }

 protected:
  GfErrType sendSyncRequestRegisterInterest(
      TcrMessage& request, TcrMessageReply& reply, bool attemptFailover = true,
      ThinClientRegion* region = nullptr,
      TcrEndpoint* endpoint = nullptr) override;

  virtual GfErrType sendSyncRequestCq(TcrMessage& request,
                                      TcrMessageReply& reply);

  void getEndpointNames(
      std::unordered_set<std::string>& endpointNames) override;

  bool preFailoverAction() override;

  bool postFailoverAction(TcrEndpoint* endpoint) override;

 private:
  TcrConnectionManager& m_theTcrConnManager;

  GfErrType sendRequestToPrimary(TcrMessage& request, TcrMessageReply& reply) {
    return m_theTcrConnManager.sendRequestToPrimary(request, reply);
  }

  friend class ThinClientHARegion;
  friend class TcrConnectionManager;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TCRHADISTRIBUTIONMANAGER_H_
