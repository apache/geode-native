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

#include <cstdlib>
#include <string>
#include <thread>

#include <geode/CacheableKey.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/SystemProperties.hpp>
#include <geode/internal/chrono/duration.hpp>

#include "CppCacheLibrary.hpp"
#include "util/Log.hpp"

#if defined(_WIN32)
#include <windows.h>
#endif

namespace {

const char StatisticsSampleInterval[] = "statistic-sample-rate";
const char StatisticsEnabled[] = "statistic-sampling-enabled";
const char StatisticsArchiveFile[] = "statistic-archive-file";
const char LogFilename[] = "log-file";
const char LogLevelProperty[] = "log-level";

const char Name[] = "name";
const char ConnectionPoolSize[] = "connection-pool-size";

const char CacheXMLFile[] = "cache-xml-file";
const char LogFileSizeLimit[] = "log-file-size-limit";
const char LogDiskSpaceLimit[] = "log-disk-space-limit";
const char StatsFileSizeLimit[] = "archive-file-size-limit";
const char StatsDiskSpaceLimit[] = "archive-disk-space-limit";
const char HeapLRULimit[] = "heap-lru-limit";
const char HeapLRUDelta[] = "heap-lru-delta";
const char MaxSocketBufferSize[] = "max-socket-buffer-size";
const char PingInterval[] = "ping-interval";
const char RedundancyMonitorInterval[] = "redundancy-monitor-interval";
const char DisableShufflingEndpoint[] = "disable-shuffling-of-endpoints";
const char NotifyAckInterval[] = "notify-ack-interval";
const char NotifyDupCheckLife[] = "notify-dupcheck-life";
const char DurableClientId[] = "durable-client-id";
const char DurableTimeout[] = "durable-timeout";
const char ConnectTimeout[] = "connect-timeout";
const char ConnectWaitTimeout[] = "connect-wait-timeout";
const char BucketWaitTimeout[] = "bucket-wait-timeout";
const char ConflateEvents[] = "conflate-events";
const char SecurityClientDhAlgo[] = "security-client-dhalgo";
const char SecurityClientKsPath[] = "security-client-kspath";
const char AutoReadyForEvents[] = "auto-ready-for-events";
const char SslEnabled[] = "ssl-enabled";
const char TimeStatisticsEnabled[] = "enable-time-statistics";
const char SslKeyStore[] = "ssl-keystore";
const char SslTrustStore[] = "ssl-truststore";
const char SslKeystorePassword[] = "ssl-keystore-password";
const char ThreadPoolSize[] = "max-fe-threads";
const char SuspendedTxTimeout[] = "suspended-tx-timeout";
const char EnableChunkHandlerThread[] = "enable-chunk-handler-thread";
const char OnClientDisconnectClearPdxTypeIds[] =
    "on-client-disconnect-clear-pdxType-Ids";
const char TombstoneTimeoutInMSec[] = "tombstone-timeout";
const char DefaultConflateEvents[] = "server";

const char DefaultDurableClientId[] = "";
constexpr auto DefaultDurableTimeout = std::chrono::seconds(300);

constexpr auto DefaultConnectTimeout = std::chrono::seconds(59);
constexpr auto DefaultConnectWaitTimeout = std::chrono::seconds::zero();
constexpr auto DefaultBucketWaitTimeout = std::chrono::seconds::zero();

constexpr auto DefaultSamplingInterval = std::chrono::seconds(1);
constexpr auto DefaultSamplingEnabled = false;

const char DefaultStatArchive[] = "statArchive.gfs";
const char DefaultLogFilename[] = "";  // stdout...

const apache::geode::client::LogLevel DefaultLogLevel =
    apache::geode::client::LogLevel::Config;

const int DefaultConnectionPoolSize = 5;

const bool DefaultAutoReadyForEvents = true;
const bool DefaultSslEnabled = false;
const bool DefaultTimeStatisticsEnabled = false;  // or true;

const char DefaultSslKeyStore[] = "";
const char DefaultSslTrustStore[] = "";
const char DefaultSslKeystorePassword[] = "";
const char DefaultName[] = "";
const char DefaultCacheXMLFile[] = "";
const uint32_t DefaultLogFileSizeLimit = 0;     // = unlimited
const uint32_t DefaultLogDiskSpaceLimit = 0;    // = unlimited
const uint32_t DefaultStatsFileSizeLimit = 0;   // = unlimited
const uint32_t DefaultStatsDiskSpaceLimit = 0;  // = unlimited

const size_t DefaultHeapLRULimit = 0;    // = unlimited, disabled when it is 0
const int32_t DefaultHeapLRUDelta = 10;  // = unlimited, disabled when it is 0

const int32_t DefaultMaxSocketBufferSize = 65 * 1024;
constexpr auto DefaultPingInterval = std::chrono::seconds(10);
constexpr auto DefaultRedundancyMonitorInterval = std::chrono::seconds(10);
constexpr auto DefaultNotifyAckInterval = std::chrono::seconds(1);
constexpr auto DefaultNotifyDupCheckLife = std::chrono::seconds(300);
const char DefaultSecurityPrefix[] = "security-";
const uint32_t DefaultThreadPoolSize = std::thread::hardware_concurrency() * 2;
constexpr auto DefaultSuspendedTxTimeout = std::chrono::seconds(30);
constexpr auto DefaultTombstoneTimeout = std::chrono::seconds(480);
// not disable; all region api will use chunk handler thread
const bool DefaultEnableChunkHandlerThread = false;
const bool DefaultOnClientDisconnectClearPdxTypeIds = false;

}  // namespace

namespace apache {
namespace geode {
namespace client {

SystemProperties::SystemProperties(
    const std::shared_ptr<Properties>& propertiesPtr,
    const std::string& configFile)
    : m_statisticsSampleInterval(DefaultSamplingInterval),
      m_statisticsEnabled(DefaultSamplingEnabled),
      m_statisticsArchiveFile(DefaultStatArchive),
      m_logFilename(DefaultLogFilename),
      m_logLevel(DefaultLogLevel),
      m_sessions(0 /* setup  later in processProperty */),
      m_name(DefaultName),
      m_disableShufflingEndpoint(false),
      m_cacheXMLFile(DefaultCacheXMLFile),
      m_logFileSizeLimit(DefaultLogFileSizeLimit),
      m_logDiskSpaceLimit(DefaultLogDiskSpaceLimit),
      m_statsFileSizeLimit(DefaultStatsFileSizeLimit),
      m_statsDiskSpaceLimit(DefaultStatsDiskSpaceLimit),
      m_connectionPoolSize(DefaultConnectionPoolSize),
      m_heapLRULimit(DefaultHeapLRULimit),
      m_heapLRUDelta(DefaultHeapLRUDelta),
      m_maxSocketBufferSize(DefaultMaxSocketBufferSize),
      m_pingInterval(DefaultPingInterval),
      m_redundancyMonitorInterval(DefaultRedundancyMonitorInterval),
      m_notifyAckInterval(DefaultNotifyAckInterval),
      m_notifyDupCheckLife(DefaultNotifyDupCheckLife),
      m_securityClientDhAlgo(),
      m_securityClientKsPath(),
      m_durableClientId(DefaultDurableClientId),
      m_durableTimeout(DefaultDurableTimeout),
      m_connectTimeout(DefaultConnectTimeout),
      m_connectWaitTimeout(DefaultConnectWaitTimeout),
      m_bucketWaitTimeout(DefaultBucketWaitTimeout),
      m_autoReadyForEvents(DefaultAutoReadyForEvents),
      m_sslEnabled(DefaultSslEnabled),
      m_timestatisticsEnabled(DefaultTimeStatisticsEnabled),
      m_sslKeyStore(DefaultSslKeyStore),
      m_sslTrustStore(DefaultSslTrustStore),
      m_sslKeystorePassword(DefaultSslKeystorePassword),
      m_conflateEvents(DefaultConflateEvents),
      m_threadPoolSize(DefaultThreadPoolSize),
      m_suspendedTxTimeout(DefaultSuspendedTxTimeout),
      m_tombstoneTimeout(DefaultTombstoneTimeout),
      m_enableChunkHandlerThread(DefaultEnableChunkHandlerThread),
      m_onClientDisconnectClearPdxTypeIds(
          DefaultOnClientDisconnectClearPdxTypeIds) {
  // now that defaults are set, consume files and override the defaults.
  class ProcessPropsVisitor : public Properties::Visitor {
    SystemProperties* m_sysProps;

   public:
    explicit ProcessPropsVisitor(SystemProperties* sysProps)
        : m_sysProps(sysProps) {}
    ~ProcessPropsVisitor() noexcept override = default;

    void visit(const std::shared_ptr<CacheableKey>& key,
               const std::shared_ptr<Cacheable>& value) override {
      auto property = key->toString();
      std::string val;
      if (value != nullptr) {
        val = value->toString();
      }
      m_sysProps->processProperty(property, val);
    }
  } processPropsVisitor(this);

  m_securityPropertiesPtr = Properties::create();
  auto givenConfigPtr = Properties::create();
  // Load the file from product tree.
  try {
    std::string defaultSystemProperties =
        CppCacheLibrary::getProductDir() + "/defaultSystem/geode.properties";
    givenConfigPtr->load(defaultSystemProperties);
  } catch (Exception&) {
    LOGERROR(
        "Unable to determine Product Directory. Please set the "
        "GEODE_NATIVE_HOME environment variable.");
    throw;
  }

  // Load the file from current directory.
  if (configFile.empty()) {
    givenConfigPtr->load("./geode.properties");
  } else {
    givenConfigPtr->load(configFile);
  }
  // process each loaded property.
  givenConfigPtr->foreach (processPropsVisitor);

  // Now consume any properties provided by the Properties object in code.
  if (propertiesPtr != nullptr) {
    propertiesPtr->foreach (processPropsVisitor);
  }
}

SystemProperties::~SystemProperties() {}

void SystemProperties::throwError(const std::string& msg) {
  LOGERROR(msg);
  throw GeodeConfigException(msg);
}

bool SystemProperties::parseBooleanProperty(const std::string& property,
                                            const std::string& value) {
  if (value == "false") {
    return false;
  } else if (value == "true") {
    return true;
  }

  throwError("SystemProperties: non-boolean " + property + "=" + value);
}

template <class _Rep, class _Period>
void SystemProperties::parseDurationProperty(
    const std::string& property, const std::string& value,
    std::chrono::duration<_Rep, _Period>& duration) {
  using apache::geode::internal::chrono::duration::from_string;
  try {
    duration = from_string<std::chrono::duration<_Rep, _Period>>(value);
  } catch (std::invalid_argument&) {
    throwError(("SystemProperties: non-duration " + property + "=" + value));
  }
}

void SystemProperties::processProperty(const std::string& property,
                                       const std::string& value) {
  if (property.compare(0, sizeof(DefaultSecurityPrefix) - 1,
                       DefaultSecurityPrefix) == 0) {
    m_securityPropertiesPtr->insert(property, value);

    if (property == SecurityClientDhAlgo) {
      throw IllegalArgumentException(
          "Diffie-Hellman based credentials encryption is not supported.");
    } else if (property == SecurityClientKsPath) {
      m_securityClientKsPath = value;
    }

    return;
  }

  if (property == ThreadPoolSize) {
    m_threadPoolSize = std::stoul(value);
  } else if (property == MaxSocketBufferSize) {
    m_maxSocketBufferSize = std::stol(value);
  } else if (property == PingInterval) {
    parseDurationProperty(property, std::string(value), m_pingInterval);
  } else if (property == RedundancyMonitorInterval) {
    parseDurationProperty(property, std::string(value),
                          m_redundancyMonitorInterval);
  } else if (property == NotifyAckInterval) {
    parseDurationProperty(property, std::string(value), m_notifyAckInterval);
  } else if (property == NotifyDupCheckLife) {
    parseDurationProperty(property, std::string(value), m_notifyDupCheckLife);
  } else if (property == StatisticsSampleInterval) {
    parseDurationProperty(property, std::string(value),
                          m_statisticsSampleInterval);
  } else if (property == DurableTimeout) {
    parseDurationProperty(property, std::string(value), m_durableTimeout);
  } else if (property == ConnectTimeout) {
    parseDurationProperty(property, std::string(value), m_connectTimeout);
  } else if (property == ConnectWaitTimeout) {
    parseDurationProperty(property, std::string(value), m_connectWaitTimeout);
  } else if (property == BucketWaitTimeout) {
    parseDurationProperty(property, std::string(value), m_bucketWaitTimeout);
  } else if (property == DisableShufflingEndpoint) {
    m_disableShufflingEndpoint = parseBooleanProperty(property, value);
  } else if (property == AutoReadyForEvents) {
    m_autoReadyForEvents = parseBooleanProperty(property, value);
  } else if (property == SslEnabled) {
    m_sslEnabled = parseBooleanProperty(property, value);
  } else if (property == TimeStatisticsEnabled) {
    m_timestatisticsEnabled = parseBooleanProperty(property, value);
  } else if (property == StatisticsEnabled) {
    m_statisticsEnabled = parseBooleanProperty(property, value);
  } else if (property == StatisticsArchiveFile) {
    m_statisticsArchiveFile = value;
  } else if (property == LogFilename) {
    m_logFilename = value;
  } else if (property == LogLevelProperty) {
    try {
      m_logLevel = Log::stringToLogLevel(value);
    } catch (IllegalArgumentException&) {
      throwError(
          ("SystemProperties: unknown log level " + property + "=" + value));
    }
  } else if (property == ConnectionPoolSize) {
    m_connectionPoolSize = std::stol(value);
  } else if (property == Name) {
    m_name = value;
  } else if (property == DurableClientId) {
    m_durableClientId = value;
  } else if (property == SslKeyStore) {
    m_sslKeyStore = value;
  } else if (property == SslTrustStore) {
    m_sslTrustStore = value;
  } else if (property == SslKeystorePassword) {
    m_sslKeystorePassword = value;
  } else if (property == ConflateEvents) {
    m_conflateEvents = value;
  } else if (property == CacheXMLFile) {
    m_cacheXMLFile = value;
  } else if (property == LogFileSizeLimit) {
    m_logFileSizeLimit = std::stol(value);
  } else if (property == LogDiskSpaceLimit) {
    m_logDiskSpaceLimit = std::stol(value);
  } else if (property == StatsFileSizeLimit) {
    m_statsFileSizeLimit = std::stol(value);
  } else if (property == StatsDiskSpaceLimit) {
    m_statsDiskSpaceLimit = std::stol(value);
  } else if (property == HeapLRULimit) {
    m_heapLRULimit = std::stol(value);
  } else if (property == HeapLRUDelta) {
    m_heapLRUDelta = std::stol(value);
  } else if (property == SuspendedTxTimeout) {
    parseDurationProperty(property, std::string(value), m_suspendedTxTimeout);
  } else if (property == TombstoneTimeoutInMSec) {
    parseDurationProperty(property, std::string(value), m_tombstoneTimeout);
  } else if (property == EnableChunkHandlerThread) {
    m_enableChunkHandlerThread = parseBooleanProperty(property, value);
  } else if (property == OnClientDisconnectClearPdxTypeIds) {
    m_onClientDisconnectClearPdxTypeIds = parseBooleanProperty(property, value);
  } else {
    throwError("SystemProperties: unknown property: " + property + "=" + value);
  }
}

void SystemProperties::logSettings() {
  using apache::geode::internal::chrono::duration::to_string;

  // *** PLEASE ADD IN ALPHABETICAL ORDER - USER VISIBLE ***

  std::string settings = "Geode Native Client System Properties:";

  settings += "\n  archive-disk-space-limit = ";
  settings += std::to_string(statsDiskSpaceLimit());

  settings += "\n  archive-file-size-limit = ";
  settings += std::to_string(statsFileSizeLimit());

  settings += "\n  auto-ready-for-events = ";
  settings += autoReadyForEvents() ? "true" : "false";

  settings += "\n  bucket-wait-timeout = ";
  settings += to_string(bucketWaitTimeout());

  settings += "\n  cache-xml-file = ";
  settings += cacheXMLFile();

  settings += "\n  conflate-events = ";
  settings += conflateEvents();

  settings += "\n  connect-timeout = ";
  settings += to_string(connectTimeout());

  settings += "\n  connection-pool-size = ";
  settings += std::to_string(connectionPoolSize());

  settings += "\n  connect-wait-timeout = ";
  settings += to_string(connectWaitTimeout());

  settings += "\n  enable-chunk-handler-thread = ";
  settings += enableChunkHandlerThread() ? "true" : "false";

  settings += "\n  disable-shuffling-of-endpoints = ";
  settings += isEndpointShufflingDisabled() ? "true" : "false";

  settings += "\n  durable-client-id = ";
  settings += durableClientId();

  settings += "\n  durable-timeout = ";
  settings += to_string(durableTimeout());

  // *** PLEASE ADD IN ALPHABETICAL ORDER - USER VISIBLE ***

  settings += "\n  enable-time-statistics = ";
  settings += getEnableTimeStatistics() ? "true" : "false";

  settings += "\n  heap-lru-delta = ";
  settings += std::to_string(heapLRUDelta());

  settings += "\n  heap-lru-limit = ";
  settings += std::to_string(heapLRULimit());

  settings += "\n  log-disk-space-limit = ";
  settings += std::to_string(logDiskSpaceLimit());

  settings += "\n  log-file = ";
  settings += logFilename();

  settings += "\n  log-file-size-limit = ";
  settings += std::to_string(logFileSizeLimit());

  settings += "\n  log-level = ";
  settings += Log::logLevelToString(logLevel());

  settings += "\n  max-fe-threads = ";
  settings += std::to_string(threadPoolSize());

  settings += "\n  max-socket-buffer-size = ";
  settings += std::to_string(maxSocketBufferSize());

  settings += "\n  notify-ack-interval = ";
  settings += to_string(notifyAckInterval());

  settings += "\n  notify-dupcheck-life = ";
  settings += to_string(notifyDupCheckLife());

  settings += "\n  on-client-disconnect-clear-pdxType-Ids = ";
  settings += onClientDisconnectClearPdxTypeIds() ? "true" : "false";

  // *** PLEASE ADD IN ALPHABETICAL ORDER - USER VISIBLE ***

  settings += "\n  ping-interval = ";
  settings += to_string(pingInterval());

  settings += "\n  redundancy-monitor-interval = ";
  settings += to_string(redundancyMonitorInterval());

  settings += "\n  security-client-kspath = ";
  settings += securityClientKsPath();

  settings += "\n  ssl-enabled = ";
  settings += sslEnabled() ? "true" : "false";

  settings += "\n  ssl-keystore = ";
  settings += sslKeyStore();

  settings += "\n  ssl-truststore = ";
  settings += sslTrustStore();

  settings += "\n  statistic-archive-file = ";
  settings += statisticsArchiveFile();

  settings += "\n  statistic-sampling-enabled = ";
  settings += statisticsEnabled() ? "true" : "false";

  settings += "\n  statistic-sample-rate = ";
  settings += to_string(statisticsSampleInterval());

  settings += "\n  suspended-tx-timeout = ";
  settings += to_string(suspendedTxTimeout());

  settings += "\n  tombstone-timeout = ";
  settings += to_string(tombstoneTimeout());

  // *** PLEASE ADD IN ALPHABETICAL ORDER - USER VISIBLE ***

  LOGCONFIG(settings);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
