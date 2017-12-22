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
 * A class for internal use, that encapsulates the properties that can be
 * set from DistributedSystem::connect.
 *
 */
class CPPCACHE_EXPORT SystemProperties {
 public:
  /**
   * Constructor. Will set the default values first, and then overwrite with
   * the values found in the given Properties object (if any), and
   * then from the values in the given file (if it exists).
   *
   * If useMemType is true, use the given member type; if false, always set
   * member type to SERVER.
   */
  SystemProperties(const std::shared_ptr<Properties>& propertiesPtr,
                   const std::string& configFile = "");

  SystemProperties(const SystemProperties& rhs) = delete;
  void operator=(const SystemProperties& rhs) = delete;

  /**
   * Destructor.
   */
  ~SystemProperties();

  /** print all settings to the process log. */
  void logSettings();

  const uint32_t threadPoolSize() const { return m_threadPoolSize; }

  /**
   * Returns the sampling interval of the sampling thread.
   * This would be how often the statistics thread writes to disk.
   */
  const std::chrono::milliseconds statisticsSampleInterval() const {
    return m_statisticsSampleInterval;
  }

  /**
   * Tells whether statistics needs to be archived or not.
   */
  bool statisticsEnabled() const { return m_statisticsEnabled; }

  /**
   * Whether SSL is enabled for socket connections.
   */
  bool sslEnabled() const { return m_sslEnabled; }

  /**
   * Whether time stats are enabled for the statistics.
   */
  bool getEnableTimeStatistics() const /*timestatisticsEnabled()*/
  {
    return m_timestatisticsEnabled;
  } /*m_timestatisticsEnabled*/

  /**
   * Returns the path of the private key file for SSL use.
   */
  const std::string& sslKeyStore() const { return m_sslKeyStore; }

  /**
   * Returns the client keystore password.
   */
  const std::string& sslKeystorePassword() const {
    return m_sslKeystorePassword;
  }

  /**
   * Returns the path of the public key file for SSL use.
   */
  const std::string& sslTrustStore() const { return m_sslTrustStore; }

  /**
   * Returns the name of the filename into which statistics would
   * be archived.
   */
  const std::string& statisticsArchiveFile() const {
    return m_statisticsArchiveFile;
  }

  /**
   * Returns the name of the filename into which logging would
   * be done.
   */
  const std::string& logFilename() const { return m_logFilename; }

  /**
   * Returns the log level at which logging would be done.
   */
  LogLevel logLevel() const { return m_logLevel; }

  /**
   * Returns  a boolean that specifies if heapLRULimit has been enabled for the
   * process. If enabled, the HeapLRULimit specifies the maximum amount of
   * memory
   * that values in a cache can use to store data before overflowing to disk or
   * destroying entries to ensure that the server process never runs out of
   * memory
   *
   */
  const bool heapLRULimitEnabled() const { return (m_heapLRULimit > 0); }

  /**
   * Returns  the HeapLRULimit value (in bytes), the maximum memory that values
   * in a cache can use to store data before overflowing to disk or destroying
   * entries to ensure that the server process never runs out of memory due to
   * cache memory usage
   *
   */
  const size_t heapLRULimit() const { return m_heapLRULimit; }

  /**
   * Returns  the HeapLRUDelta value (a percent value). This specifies the
   * percentage of entries the system will evict each time it detects that
   * it has exceeded the HeapLRULimit. Defaults to 10%
   */
  const int32_t heapLRUDelta() const { return m_heapLRUDelta; }
  /**
   * Returns  the maximum socket buffer size to use
   */
  const int32_t maxSocketBufferSize() const { return m_maxSocketBufferSize; }

  /**
   * Returns  the time between two consecutive ping to servers
   */
  const std::chrono::seconds pingInterval() const { return m_pingInterval; }
  /**
   * Returns  the time between two consecutive checks for redundancy for HA
   */
  const std::chrono::seconds redundancyMonitorInterval() const {
    return m_redundancyMonitorInterval;
  }

  /**
   * Returns the periodic notify ack interval
   */
  const std::chrono::milliseconds notifyAckInterval() const {
    return m_notifyAckInterval;
  }

  /**
   * Returns the expiry time of an idle event id map entry for duplicate
   * notification checking
   */
  const std::chrono::milliseconds notifyDupCheckLife() const {
    return m_notifyDupCheckLife;
  }

  /**
   * Returns the durable client ID
   */
  const std::string& durableClientId() const { return m_durableClientId; }

  /**
   * Returns the durable timeout
   */
  const std::chrono::seconds durableTimeout() const { return m_durableTimeout; }

  /**
   * Returns the connect timeout used for server and locator handshakes
   */
  const std::chrono::milliseconds connectTimeout() const {
    return m_connectTimeout;
  }

  /**
   * Returns the connect wait timeout(in millis) used for to connect to server
   * This is only applicable for linux
   */
  const std::chrono::milliseconds connectWaitTimeout() const {
    return m_connectWaitTimeout;
  }

  /**
   * Returns the connect wait timeout(in millis) used for to connect to server
   * This is only applicable for linux
   */
  const std::chrono::milliseconds bucketWaitTimeout() const {
    return m_bucketWaitTimeout;
  }

  /**
   * Returns client Queueconflation option
   */
  const std::string& conflateEvents() const { return m_conflateEvents; }

  const std::string& name() const { return m_name; }

  const std::string& cacheXMLFile() const { return m_cacheXMLFile; }

  /**
   * Returns the log-file-size-limit.
   */
  const uint32_t logFileSizeLimit() const { return m_logFileSizeLimit; }

  /**
   * Returns the log-disk-space-limit.
   */
  const uint32_t logDiskSpaceLimit() const { return m_logDiskSpaceLimit; }

  /**
   * Returns the stat-file-space-limit.
   */
  const uint32_t statsFileSizeLimit() const { return m_statsFileSizeLimit; }

  /**
   * Returns the stat-disk-size-limit.
   */
  const uint32_t statsDiskSpaceLimit() const { return m_statsDiskSpaceLimit; }

  const uint32_t maxQueueSize() { return m_maxQueueSize; }

  const uint32_t javaConnectionPoolSize() const {
    return m_javaConnectionPoolSize;
  }
  void setjavaConnectionPoolSize(uint32_t size) {
    m_javaConnectionPoolSize = size;
  }

  /**
   * This can be call to know whether chunkhandler thread is disable for that
   * opertaion
   */
  bool disableChunkHandlerThread() const { return m_disableChunkHandlerThread; }

  /**
   * This can be call multiple time to disable chunkhandler thread for those
   * operations
   */
  void setDisableChunkHandlerThread(bool set) {
    m_disableChunkHandlerThread = set;
  }

  /**
   * returns true if app want to clear pdx type ids when client disconnect.
   * deafult is false.
   */
  bool onClientDisconnectClearPdxTypeIds() const {
    return m_onClientDisconnectClearPdxTypeIds;
  }

  /**
   * Set to true if app want to clear pdx type ids when client disconnect.
   * deafult is false.
   */
  void setOnClientDisconnectClearPdxTypeIds(bool set) {
    m_onClientDisconnectClearPdxTypeIds = set;
  }

  /** Return the security diffie hellman secret key algo */
  const std::string& securityClientDhAlgo() const {
    return m_securityClientDhAlgo;
  }

  /** Return the keystore (.pem file ) path */
  const std::string& securityClientKsPath() const {
    return m_securityClientKsPath;
  }

  /** Returns securityPropertiesPtr.
   * @return  std::shared_ptr<Properties> value.
   */
  std::shared_ptr<Properties> getSecurityProperties() const {
    return m_securityPropertiesPtr;
  }

  /** Checks whether list of endpoint is shuffeled or not.
   * @return  bool value.
   */
  inline bool isEndpointShufflingDisabled() const {
    return m_disableShufflingEndpoint;
  }

  /**
   * Check whether Diffie-Hellman based credentials encryption is on.
   * @return bool flag to indicate whether DH for credentials is on.
   */
  bool isDhOn() const { return !m_securityClientDhAlgo.empty(); }

  /**
   * Checks to see if this native client is being invoked as part of small
   * grid jobs; use this setting to disable some creation of threads and
   * reducing start/stop time. Note that this setting can cause improper
   * behaviour in some cases like:
   *  1) client that is setup in listening mode and a server failure may not
   *     lead to failover by client
   *  2) while shutting down the client will not send a proper CLOSE_CONNECTION
   *     message so server will report EOF exceptions and may detect client
   *     disconnect after quite some time
   * Also note that there may be some performance loss in queries and
   * Region::getAll due to unavailability of parallel processing threads.
   *
   * @return true if the "grid-client" property is set
   */
  inline bool isGridClient() const { return m_gridClient; }

  /**
   * This property checks whether C# client is running in multiple appdoamin or
   * not.
   * Default value is "false".
   */
  inline bool isAppDomainEnabled() const { return m_appDomainEnabled; }

  /**
   * Whether a non durable client starts to receive and process
   * subscription events automatically.
   * If set to false then a non durable client should call the
   * Cache::readyForEvents() method after all regions are created
   * and listeners attached for the client to start receiving events
   * whether the client is initialized programmatically or declaratively.
   * @return the value of the property.
   */
  inline bool autoReadyForEvents() const { return m_autoReadyForEvents; }

  /**
   * Returns the timeout after which suspended transactions are rolled back.
   */
  const std::chrono::seconds suspendedTxTimeout() const {
    return m_suspendedTxTimeout;
  }

  /**
   * Returns the tombstone timeout .
   */
  const std::chrono::milliseconds tombstoneTimeout() const {
    return m_tombstoneTimeout;
  }

 private:
  std::chrono::milliseconds m_statisticsSampleInterval;

  bool m_statisticsEnabled;

  bool m_appDomainEnabled;

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

  uint32_t m_maxQueueSize;
  uint32_t m_javaConnectionPoolSize;

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

  bool m_gridClient;

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
  bool m_disableChunkHandlerThread;
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

  static void throwError(const std::string& msg);

 public:
  friend class DistributedSystemImpl;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_SYSTEMPROPERTIES_H_
