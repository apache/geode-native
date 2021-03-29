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

#ifndef GEODE_TCRPOOLENDPOINT_H_
#define GEODE_TCRPOOLENDPOINT_H_

#include "PoolStatistics.hpp"
#include "TcrEndpoint.hpp"

namespace apache {
namespace geode {
namespace client {

class ThinClientPoolDM;

class TcrPoolEndPoint : public TcrEndpoint {
 public:
  TcrPoolEndPoint(const std::string& name, CacheImpl* cache,
                  binary_semaphore& failoverSema, binary_semaphore& cleanupSema,
                  binary_semaphore& redundancySema, ThinClientPoolDM* dm);
  ThinClientPoolDM* getPoolHADM() const override;

  bool checkDupAndAdd(std::shared_ptr<EventId> eventid) override;
  void processMarker() override;
  std::shared_ptr<QueryService> getQueryService() override;
  void closeFailedConnection(TcrConnection*& conn) override;
  GfErrType registerDM(bool clientNotification, bool isSecondary = false,
                       bool isActiveEndpoint = false,
                       ThinClientBaseDM* distMgr = nullptr) override;
  void unregisterDM(bool clientNotification,
                    ThinClientBaseDM* distMgr = nullptr,
                    bool checkQueueHosted = false) override;
  using TcrEndpoint::handleIOException;
  bool handleIOException(const std::string& message, TcrConnection*& conn,
                         bool isBgThread = false) override;
  void handleNotificationStats(int64_t byteLength) override;
  ~TcrPoolEndPoint() override { m_dm = nullptr; }
  bool isMultiUserMode() override;

 protected:
  void closeNotification() override;
  void triggerRedundancyThread() override;

 private:
  ThinClientPoolDM* m_dm;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TCRPOOLENDPOINT_H_
