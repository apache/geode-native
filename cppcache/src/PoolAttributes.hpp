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

  const std::chrono::milliseconds& getFreeConnectionTimeout() const {
    return m_freeConnTimeout;
  }

  void setFreeConnectionTimeout(
      const std::chrono::milliseconds& connectionTimeout) {
    m_freeConnTimeout = connectionTimeout;
  }

  const std::chrono::milliseconds& getLoadConditioningInterval() const {
    return m_loadCondInterval;
  }

  void setLoadConditioningInterval(
      const std::chrono::milliseconds& loadConditioningInterval) {
    m_loadCondInterval = loadConditioningInterval;
  }

  int getSocketBufferSize() const { return m_sockBufferSize; }

  void setSocketBufferSize(int bufferSize) { m_sockBufferSize = bufferSize; }

  const std::chrono::milliseconds& getReadTimeout() const {
    return m_readTimeout;
  }

  void setReadTimeout(const std::chrono::milliseconds& timeout) {
    m_readTimeout = timeout;
  }

  bool getThreadLocalConnectionSetting() { return m_isThreadLocalConn; }

  void setThreadLocalConnectionSetting(bool isThreadLocal) {
    m_isThreadLocalConn = isThreadLocal;
  }

  int getMinConnections() const { return m_minConns; }

  void setMinConnections(int minConnections) { m_minConns = minConnections; }

  int getMaxConnections() const { return m_maxConns; }

  void setMaxConnections(int maxConnections) { m_maxConns = maxConnections; }

  const std::chrono::milliseconds& getIdleTimeout() const {
    return m_idleTimeout;
  }

  void setIdleTimeout(const std::chrono::milliseconds& idleTimeout) {
    m_idleTimeout = idleTimeout;
  }

  int getRetryAttempts() const { return m_retryAttempts; }

  void setRetryAttempts(int retryAttempts) { m_retryAttempts = retryAttempts; }

  const std::chrono::milliseconds& getPingInterval() const {
    return m_pingInterval;
  }

  void setPingInterval(const std::chrono::milliseconds& pingInterval) {
    m_pingInterval = pingInterval;
  }

  const std::chrono::milliseconds& getUpdateLocatorListInterval() const {
    return m_updateLocatorListInterval;
  }

  void setUpdateLocatorListInterval(
      const std::chrono::milliseconds& updateLocatorListInterval) {
    m_updateLocatorListInterval = updateLocatorListInterval;
  }

  const std::chrono::milliseconds& getStatisticInterval() const {
    return m_statsInterval;
  }

  void setStatisticInterval(
      const std::chrono::milliseconds& statisticInterval) {
    m_statsInterval = statisticInterval;
  }

  const std::string& getSniProxyHost() const { return m_sniProxyHost; }

  void setSniProxyHost(const std::string& host) { m_sniProxyHost = host; }

  int getSniProxyPort() const { return m_sniProxyPort; }

  void setSniProxyPort(const int port) { m_sniProxyPort = port; }

  const std::string& getServerGroup() const { return m_serverGrp; }

  void setServerGroup(std::string group) { m_serverGrp = group; }

  bool getSubscriptionEnabled() const { return m_subsEnabled; }

  void setSubscriptionEnabled(bool enabled) { m_subsEnabled = enabled; }

  int getSubscriptionRedundancy() const { return m_redundancy; }

  void setSubscriptionRedundancy(int redundancy) { m_redundancy = redundancy; }

  const std::chrono::milliseconds& getSubscriptionMessageTrackingTimeout()
      const {
    return m_msgTrackTimeout;
  }

  void setSubscriptionMessageTrackingTimeout(
      const std::chrono::milliseconds& messageTrackingTimeout) {
    m_msgTrackTimeout = messageTrackingTimeout;
  }

  const std::chrono::milliseconds& getSubscriptionAckInterval() const {
    return m_subsAckInterval;
  }

  void setSubscriptionAckInterval(
      const std::chrono::milliseconds& ackInterval) {
    m_subsAckInterval = ackInterval;
  }

  bool getPRSingleHopEnabled() const { return m_isPRSingleHopEnabled; }

  void setPRSingleHopEnabled(bool enabled) { m_isPRSingleHopEnabled = enabled; }

  bool getMultiuserSecureModeEnabled() const { return m_multiuserSecurityMode; }

  void setMultiuserSecureModeEnabled(bool multiuserSecureMode) {
    m_multiuserSecurityMode = multiuserSecureMode;
  }

  void addLocator(const std::string& host, int port);

  void addServer(const std::string& host, int port);

  std::shared_ptr<PoolAttributes> clone();

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

  std::string m_sniProxyHost;
  int m_sniProxyPort;

  static bool compareVectorOfStrings(
      const std::vector<std::string>& thisVector,
      const std::vector<std::string>& otherVector);

  friend class ThinClientPoolDM;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_POOLATTRIBUTES_H_
