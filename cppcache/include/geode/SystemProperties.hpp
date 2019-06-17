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

#ifndef GEODE_SYSTEMPROPERTIES_H_
#define GEODE_SYSTEMPROPERTIES_H_

#include "Properties.hpp"
#include "util/LogLevel.hpp"

/** @file
 */

namespace apache {
namespace geode {
namespace client {

/**
 * The SystemProperties class
 *
 *
 */

/**
 * A class for internal use that encapsulates the properties that can be
 * set from DistributedSystem::connect.
 *
 */
class APACHE_GEODE_EXPORT SystemProperties {
 public:
  /**
   * Constructor. Will set the default values first, and then overwrite with
   * the values found in the given Properties object (if any), and
   * then from the values in the given file (if it exists).
   *
   * If useMemType is true, use the given member type; if false, always set
   * member type to SERVER.
   */
  explicit SystemProperties(const std::shared_ptr<Properties>& propertiesPtr,
                            const std::string& configFile = "");

  SystemProperties(const SystemProperties& rhs) = delete;
  void operator=(const SystemProperties& rhs) = delete;

  /**
   * Destructor.
   */
  ~SystemProperties();

  /** print all settings to the process log. */
  void logSettings();

  uint32_t threadPoolSize() const;

  /**
   * Returns the sampling interval of the sampling thread.
   * This would be how often the statistics thread writes to disk.
   */
  const std::chrono::milliseconds statisticsSampleInterval() const;

  /**
   * Tells whether statistics needs to be archived or not.
   */
  bool statisticsEnabled() const;

  /**
   * Whether SSL is enabled for socket connections.
   */
  bool sslEnabled() const;

  /**
   * Whether time stats are enabled for the statistics.
   */
  bool getEnableTimeStatistics() const;

  /**
   * Returns the path of the private key file for SSL use.
   */
  const std::string& sslKeyStore() const;

  /**
   * Returns the client keystore password.
   */
  const std::string& sslKeystorePassword() const;

  /**
   * Returns the path of the public key file for SSL use.
   */
  const std::string& sslTrustStore() const;

  /**
   * Returns the name of the filename into which statistics would
   * be archived.
   */
  const std::string& statisticsArchiveFile() const;

  /**
   * Returns the name of the filename into which logging would
   * be done.
   */
  const std::string& logFilename() const;

  /**
   * Returns the log level at which logging would be done.
   */
  LogLevel logLevel() const;

  /**
   * Returns a boolean that specifies if heapLRULimit has been enabled for the
   * process. If enabled, the HeapLRULimit specifies the maximum amount of
   * memory
   * that values in a cache can use to store data before overflowing to disk or
   * destroying entries to ensure that the server process never runs out of
   * memory
   *
   */
  bool heapLRULimitEnabled() const;

  /**
   * Returns the HeapLRULimit value (in bytes), the maximum memory that values
   * in a cache can use to store data before overflowing to disk or destroying
   * entries to ensure that the server process never runs out of memory due to
   * cache memory usage
   *
   */
  size_t heapLRULimit() const;

  /**
   * Returns the HeapLRUDelta value (a percent value). This specifies the
   * percentage of entries the system will evict each time it detects that
   * it has exceeded the HeapLRULimit. Defaults to 10%
   */
  int32_t heapLRUDelta() const;

  /**
   * Returns the maximum socket buffer size to use
   */
  int32_t maxSocketBufferSize() const;

  /**
   * Returns the time between two consecutive pings to servers
   */
  const std::chrono::seconds& pingInterval() const;

  /**
   * Returns the time between two consecutive checks for redundancy for HA
   */
  const std::chrono::seconds& redundancyMonitorInterval() const;

  /**
   * Returns the periodic notify ack interval
   */
  const std::chrono::milliseconds& notifyAckInterval() const;

  /**
   * Returns the expiry time of an idle event id map entry for duplicate
   * notification checking
   */
  const std::chrono::milliseconds& notifyDupCheckLife() const;

  /**
   * Returns the durable client ID
   */
  const std::string& durableClientId() const;

  /**
   * Returns the durable timeout
   */
  const std::chrono::seconds& durableTimeout() const;

  /**
   * Returns the connect timeout used for server and locator handshakes
   */
  const std::chrono::milliseconds& connectTimeout() const;

  /**
   * Returns the connect wait timeout(in milliseconds) used for to connect to
   * server This is only applicable for linux
   */
  const std::chrono::milliseconds& connectWaitTimeout() const;

  /**
   * Returns the connect wait timeout(in milliseconds) used for to connect to
   * server This is only applicable for linux
   */
  const std::chrono::milliseconds& bucketWaitTimeout() const;

  /**
   * Returns client Queueconflation option
   */
  const std::string& conflateEvents() const;

  const std::string& name() const;

  const std::string& cacheXMLFile() const;

  /**
   * Returns the log-file-size-limit.
   */
  uint32_t logFileSizeLimit() const;

  /**
   * Returns the log-disk-space-limit.
   */
  uint32_t logDiskSpaceLimit() const;

  /**
   * Returns the stat-file-space-limit.
   */
  uint32_t statsFileSizeLimit() const;

  /**
   * Returns the stat-disk-size-limit.
   */
  uint32_t statsDiskSpaceLimit() const;

  uint32_t connectionPoolSize() const;

  void setjavaConnectionPoolSize(uint32_t size);

  /**
   * Returns true if chunk handler thread is enabled, false if not
   */
  bool enableChunkHandlerThread() const;

  /**
   * Enables or disables the chunk handler thread
   */
  void setEnableChunkHandlerThread(bool set);

  /**
   * Returns true if app wants to clear pdx type ids when client disconnect.
   * default is false.
   */
  bool onClientDisconnectClearPdxTypeIds() const;

  /**
   * Set to true if app wants to clear pdx type ids when client disconnect.
   * Default is false.
   */
  void setOnClientDisconnectClearPdxTypeIds(bool set);

  /** Return the security Diffie-Hellman secret key algorithm */
  const std::string& securityClientDhAlgo() const;

  /** Return the keystore (.pem file ) path */
  const std::string& securityClientKsPath() const;

  /** Returns securityPropertiesPtr.
   * @return  std::shared_ptr<Properties> value.
   */
  std::shared_ptr<Properties> getSecurityProperties() const;

  /** Checks whether list of endpoint is shuffled or not.
   * @return  bool value.
   */
  bool isEndpointShufflingDisabled() const;

  /**
   * Check whether Diffie-Hellman based credentials encryption is on.
   * @return bool flag to indicate whether DH for credentials is on.
   */
  bool isDhOn() const;

  /**
   * Whether a non durable client starts to receive and process
   * subscription events automatically.
   * If set to false then a non-durable client should call the
   * Cache::readyForEvents() method after all regions are created
   * and listeners attached for the client to start receiving events
   * whether the client is initialized programmatically or declaratively.
   * @return the value of the property.
   */
  bool autoReadyForEvents() const;

  /**
   * Returns the timeout after which suspended transactions are rolled back.
   */
  const std::chrono::seconds suspendedTxTimeout() const;

  /**
   * Returns the tombstone timeout
   */
  const std::chrono::milliseconds tombstoneTimeout() const;

 private:
  std::chrono::milliseconds m_statisticsSampleInterval;

  bool m_statisticsEnabled;

  std::string m_statisticsArchiveFile;

  std::string m_logFilename;

  LogLevel m_logLevel;

  int m_sessions;

  std::string m_name;

  bool m_disableShufflingEndpoint;

  std::string m_cacheXMLFile;

  uint32_t m_logFileSizeLimit;
  uint32_t m_logDiskSpaceLimit;

  uint32_t m_statsFileSizeLimit;
  uint32_t m_statsDiskSpaceLimit;

  uint32_t m_connectionPoolSize;

  int32_t m_heapLRULimit;
  int32_t m_heapLRUDelta;
  int32_t m_maxSocketBufferSize;
  std::chrono::seconds m_pingInterval;
  std::chrono::seconds m_redundancyMonitorInterval;

  std::chrono::milliseconds m_notifyAckInterval;
  std::chrono::milliseconds m_notifyDupCheckLife;

  std::shared_ptr<Properties> m_securityPropertiesPtr;

  std::string m_securityClientDhAlgo;
  std::string m_securityClientKsPath;

  std::string m_durableClientId;
  std::chrono::seconds m_durableTimeout;

  std::chrono::milliseconds m_connectTimeout;
  std::chrono::milliseconds m_connectWaitTimeout;
  std::chrono::milliseconds m_bucketWaitTimeout;

  bool m_autoReadyForEvents;

  bool m_sslEnabled;
  bool m_timestatisticsEnabled;
  std::string m_sslKeyStore;
  std::string m_sslTrustStore;

  std::string m_sslKeystorePassword;

  std::string m_conflateEvents;

  uint32_t m_threadPoolSize;
  std::chrono::seconds m_suspendedTxTimeout;
  std::chrono::milliseconds m_tombstoneTimeout;
  bool m_enableChunkHandlerThread;
  bool m_onClientDisconnectClearPdxTypeIds;

  /**
   * Processes the given property/value pair, saving
   * the results internally:
   */
  void processProperty(const std::string& property, const std::string& value);
  static bool parseBooleanProperty(const std::string& property,
                                   const std::string& value);
  template <class _Rep, class _Period>
  static void parseDurationProperty(
      const std::string& property, const std::string& value,
      std::chrono::duration<_Rep, _Period>& duration);

  [[noreturn]] static void throwError(const std::string& msg);

 public:
  friend class DistributedSystemImpl;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_SYSTEMPROPERTIES_H_
