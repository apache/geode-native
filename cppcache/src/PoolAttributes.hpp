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

#ifndef GEODE_POOLATTRIBUTES_H_
#define GEODE_POOLATTRIBUTES_H_

#include <chrono>
#include <string>
#include <vector>

#include <geode/ExceptionTypes.hpp>
#include <geode/internal/geode_globals.hpp>

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

class PoolAttributes {
 public:
  PoolAttributes();

  const std::chrono::milliseconds& getFreeConnectionTimeout() const;

  void setFreeConnectionTimeout(
      const std::chrono::milliseconds& connectionTimeout);

  const std::chrono::milliseconds& getLoadConditioningInterval() const;

  void setLoadConditioningInterval(
      const std::chrono::milliseconds& loadConditioningInterval);

  int getSocketBufferSize() const;

  void setSocketBufferSize(int bufferSize);

  const std::chrono::milliseconds& getReadTimeout() const;

  void setReadTimeout(const std::chrono::milliseconds& timeout);

  bool getThreadLocalConnectionSetting();

  void setThreadLocalConnectionSetting(bool isThreadLocal);

  int getMinConnections() const;

  void setMinConnections(int minConnections);

  int getMaxConnections() const;

  void setMaxConnections(int maxConnections);

  const std::chrono::milliseconds& getIdleTimeout() const;

  void setIdleTimeout(const std::chrono::milliseconds& idleTimeout);

  int getRetryAttempts() const;

  void setRetryAttempts(int retryAttempts);

  const std::chrono::milliseconds& getPingInterval() const;

  void setPingInterval(const std::chrono::milliseconds& pingInterval);

  const std::chrono::milliseconds& getUpdateLocatorListInterval() const;

  void setUpdateLocatorListInterval(
      const std::chrono::milliseconds& updateLocatorListInterval);

  const std::chrono::milliseconds& getStatisticInterval() const;

  void setStatisticInterval(
      const std::chrono::milliseconds& statisticInterval);

  const std::string& getServerGroup() const;

  void setServerGroup(std::string group);

  bool getSubscriptionEnabled() const;

  void setSubscriptionEnabled(bool enabled);

  int getSubscriptionRedundancy() const;

  void setSubscriptionRedundancy(int redundancy);

  const std::chrono::milliseconds& getSubscriptionMessageTrackingTimeout() const;

  void setSubscriptionMessageTrackingTimeout(
      const std::chrono::milliseconds& messageTrackingTimeout);

  const std::chrono::milliseconds& getSubscriptionAckInterval() const;

  void setSubscriptionAckInterval(
      const std::chrono::milliseconds& ackInterval);

  bool getPRSingleHopEnabled() const;

  void setPRSingleHopEnabled(bool enabled);

  bool getMultiuserSecureModeEnabled() const;

  void setMultiuserSecureModeEnabled(bool multiuserSecureMode);

  void addLocator(const std::string& host, int port);

  void addServer(const std::string& host, int port);

  std::shared_ptr<PoolAttributes> clone();

  /** Return true if all the attributes are equal to those of other. */
  bool operator==(const PoolAttributes& other) const;

 private:
  bool m_isThreadLocalConn;
  std::chrono::milliseconds m_freeConnTimeout;
  std::chrono::milliseconds m_loadCondInterval;
  int m_sockBufferSize;
  std::chrono::milliseconds m_readTimeout;
  int m_minConns;
  int m_maxConns;
  int m_retryAttempts;
  std::chrono::milliseconds m_statsInterval;
  int m_redundancy;
  std::chrono::milliseconds m_msgTrackTimeout;
  std::chrono::milliseconds m_subsAckInterval;

  std::chrono::milliseconds m_idleTimeout;
  std::chrono::milliseconds m_pingInterval;
  std::chrono::milliseconds m_updateLocatorListInterval;

  bool m_subsEnabled;
  bool m_multiuserSecurityMode;
  bool m_isPRSingleHopEnabled;

  std::string m_serverGrp;
  std::vector<std::string> m_initLocList;
  std::vector<std::string> m_initServList;

  static bool compareVectorOfStrings(
      const std::vector<std::string>& thisVector,
      const std::vector<std::string>& otherVector);

  friend class ThinClientPoolDM;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_POOLATTRIBUTES_H_
