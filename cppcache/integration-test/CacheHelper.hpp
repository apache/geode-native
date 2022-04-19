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

#ifndef GEODE_INTEGRATION_TEST_CACHEHELPER_H_
#define GEODE_INTEGRATION_TEST_CACHEHELPER_H_

#include <list>
#include <chrono>
#include <cstdlib>

#include <geode/SystemProperties.hpp>
#include <geode/PoolManager.hpp>

#include "TimeBomb.hpp"
#include "SerializationRegistry.hpp"
#include "DistributedSystemImpl.hpp"
#include "Utils.hpp"
#include "config.h"

#ifndef ROOT_NAME
#define ROOT_NAME "Root"
#endif

#ifndef ROOT_SCOPE
#define ROOT_SCOPE LOCAL
#endif

namespace apache {
namespace geode {
namespace client {

class CacheHelper {
 public:
  static CacheHelper* singleton;
  static std::list<std::string> staticConfigFileList;
  std::shared_ptr<Cache> cachePtr;
  std::shared_ptr<Region> rootRegionPtr;

  std::shared_ptr<Cache> getCache();

  static CacheHelper& getHelper();

  static void resetHelper();

  static std::string unitTestOutputFile();
  static int getNumLocatorListUpdates(const std::string& search);

  explicit CacheHelper(const char* member_id,
                       const std::shared_ptr<Properties>& configPtr = nullptr,
                       const bool noRootRegion = false);

  /** rootRegionPtr will still be null... */
  CacheHelper(const char* member_id, const std::string& cachexml,
              const std::shared_ptr<Properties>& configPtr = nullptr);

  explicit CacheHelper(const std::shared_ptr<Properties>& configPtr = nullptr,
                       const bool noRootRegion = false);

  explicit CacheHelper(const bool isThinclient,
                       const std::shared_ptr<Properties>& configPtr = nullptr,
                       const bool noRootRegion = false);

  CacheHelper(const bool isThinclient,
              const std::shared_ptr<AuthInitialize>& authInitialize,
              const std::shared_ptr<Properties>& configPtr = nullptr);

  CacheHelper(const bool isThinclient, bool pdxIgnoreUnreadFields,
              bool pdxReadSerialized,
              const std::shared_ptr<Properties>& configPtr = nullptr,
              const bool noRootRegion = false);

  CacheHelper(const bool isthinClient, const char* poolName,
              const std::string& locators, const char* serverGroup,
              const std::shared_ptr<Properties>& configPtr = nullptr,
              int redundancy = 0, bool clientNotification = false,
              int subscriptionAckInterval = -1, int connections = -1,
              int loadConditioningInterval = -1, bool isMultiuserMode = false,
              bool prSingleHop = false, bool threadLocal = false);

  explicit CacheHelper(const int redundancyLevel,
                       const std::shared_ptr<Properties>& configPtr = nullptr);

  virtual ~CacheHelper();

  void disconnect(bool keepalive = false);

  void createPlainRegion(const char* regionName,
                         std::shared_ptr<Region>& regionPtr);

  void createPlainRegion(const char* regionName,
                         std::shared_ptr<Region>& regionPtr, uint32_t size);

  void createLRURegion(const char* regionName,
                       std::shared_ptr<Region>& regionPtr);

  void createLRURegion(const char* regionName,
                       std::shared_ptr<Region>& regionPtr, uint32_t size);

  void createDistRegion(const char* regionName,
                        std::shared_ptr<Region>& regionPtr);

  void createDistRegion(const char* regionName,
                        std::shared_ptr<Region>& regionPtr, uint32_t size);

  std::shared_ptr<Region> getRegion(const std::string& name);

  std::shared_ptr<Region> createRegion(
      const std::string& name, bool ack, bool caching,
      const std::shared_ptr<CacheListener>& listener,
      bool clientNotificationEnabled = false, bool scopeLocal = false,
      bool concurrencyCheckEnabled = false, int32_t tombstonetimeout = -1);

  std::shared_ptr<Region> createRegion(
      const std::string& name, bool ack, bool caching = true,
      const std::chrono::seconds& ettl = std::chrono::seconds::zero(),
      const std::chrono::seconds& eit = std::chrono::seconds::zero(),
      const std::chrono::seconds& rttl = std::chrono::seconds::zero(),
      const std::chrono::seconds& rit = std::chrono::seconds::zero(),
      int lel = 0, ExpirationAction action = ExpirationAction::DESTROY,
      const std::string& endpoints = {},
      bool clientNotificationEnabled = false);

  std::shared_ptr<Pool> createPool(
      const std::string& poolName, const std::string& locators,
      const std::string& serverGroup, int redundancy = 0,
      bool clientNotification = false,
      std::chrono::milliseconds subscriptionAckInterval =
          std::chrono::milliseconds::zero(),
      int connections = -1, int loadConditioningInterval = -1,
      bool isMultiuserMode = false);

  // this will create pool even endpoints and locatorhost has been not defined
  std::shared_ptr<Pool> createPool2(
      const std::string& poolName, const std::string& locators,
      const std::string& serverGroup, const std::string& servers = {},
      int redundancy = 0, bool clientNotification = false,
      int subscriptionAckInterval = -1, int connections = -1);

  void logPoolAttributes(std::shared_ptr<Pool>& pool);

  void createPoolWithLocators(
      const std::string& name, const std::string& locators = {},
      bool clientNotificationEnabled = false, int subscriptionRedundancy = -1,
      std::chrono::milliseconds subscriptionAckInterval =
          std::chrono::milliseconds::zero(),
      int connections = -1, bool isMultiuserMode = false,
      const std::string& serverGroup = {});

  std::shared_ptr<Region> createRegionAndAttachPool(
      const std::string& name, bool ack, const std::string& poolName,
      bool caching = true,
      const std::chrono::seconds& ettl = std::chrono::seconds::zero(),
      const std::chrono::seconds& eit = std::chrono::seconds::zero(),
      const std::chrono::seconds& rttl = std::chrono::seconds::zero(),
      const std::chrono::seconds& rit = std::chrono::seconds::zero(),
      int lel = 0, ExpirationAction action = ExpirationAction::DESTROY);

  std::shared_ptr<Region> createRegionAndAttachPool2(
      const char* name, bool ack, const char* poolName,
      const std::shared_ptr<PartitionResolver>& aResolver = nullptr,
      bool caching = true,
      const std::chrono::seconds& ettl = std::chrono::seconds::zero(),
      const std::chrono::seconds& eit = std::chrono::seconds::zero(),
      const std::chrono::seconds& rttl = std::chrono::seconds::zero(),
      const std::chrono::seconds& rit = std::chrono::seconds::zero(),
      int lel = 0, ExpirationAction action = ExpirationAction::DESTROY);

  static void addServerLocatorEPs(const std::string& epList, PoolFactory& pfPtr,
                                  bool poolLocators = true);

  std::shared_ptr<Region> createPooledRegion(
      const std::string& name, bool ack, const std::string& locators = {},
      const std::string& poolName = "__TEST_POOL1__", bool caching = true,
      bool clientNotificationEnabled = false,
      const std::chrono::seconds& ettl = std::chrono::seconds::zero(),
      const std::chrono::seconds& eit = std::chrono::seconds::zero(),
      const std::chrono::seconds& rttl = std::chrono::seconds::zero(),
      const std::chrono::seconds& rit = std::chrono::seconds::zero(),
      int lel = 0,
      const std::shared_ptr<CacheListener>& cacheListener = nullptr,
      ExpirationAction action = ExpirationAction::DESTROY);

  std::shared_ptr<Region> createPooledRegionConcurrencyCheckDisabled(
      const std::string& name, bool ack, const std::string& locators = {},
      const std::string& poolName = "__TEST_POOL1__", bool caching = true,
      bool clientNotificationEnabled = false,
      bool concurrencyCheckEnabled = true,
      const std::chrono::seconds& ettl = std::chrono::seconds::zero(),
      const std::chrono::seconds& eit = std::chrono::seconds::zero(),
      const std::chrono::seconds& rttl = std::chrono::seconds::zero(),
      const std::chrono::seconds& rit = std::chrono::seconds::zero(),
      int lel = 0,
      const std::shared_ptr<CacheListener>& cacheListener = nullptr,
      ExpirationAction action = ExpirationAction::DESTROY);

  std::shared_ptr<Region> createRegionDiscOverFlow(
      const std::string& name, bool caching = true,
      bool clientNotificationEnabled = false,
      const std::chrono::seconds& ettl = std::chrono::seconds::zero(),
      const std::chrono::seconds& eit = std::chrono::seconds::zero(),
      const std::chrono::seconds& rttl = std::chrono::seconds::zero(),
      const std::chrono::seconds& rit = std::chrono::seconds::zero(),
      int lel = 0, ExpirationAction action = ExpirationAction::DESTROY);

  std::shared_ptr<Region> createPooledRegionDiscOverFlow(
      const std::string& name, bool ack, const std::string& locators = {},
      const std::string& poolName = "__TEST_POOL1__", bool caching = true,
      bool clientNotificationEnabled = false,
      const std::chrono::seconds& ettl = std::chrono::seconds::zero(),
      const std::chrono::seconds& eit = std::chrono::seconds::zero(),
      const std::chrono::seconds& rttl = std::chrono::seconds::zero(),
      const std::chrono::seconds& rit = std::chrono::seconds::zero(),
      int lel = 0,
      const std::shared_ptr<CacheListener>& cacheListener = nullptr,
      ExpirationAction action = ExpirationAction::DESTROY);

  std::shared_ptr<Region> createPooledRegionSticky(
      const std::string& name, bool ack, const std::string& locators = {},
      const std::string& poolName = "__TEST_POOL1__", bool caching = true,
      bool clientNotificationEnabled = false,
      const std::chrono::seconds& ettl = std::chrono::seconds::zero(),
      const std::chrono::seconds& eit = std::chrono::seconds::zero(),
      const std::chrono::seconds& rttl = std::chrono::seconds::zero(),
      const std::chrono::seconds& rit = std::chrono::seconds::zero(),
      int lel = 0,
      const std::shared_ptr<CacheListener>& cacheListener = nullptr,
      ExpirationAction action = ExpirationAction::DESTROY);

  std::shared_ptr<Region> createPooledRegionStickySingleHop(
      const std::string& name, bool ack, const std::string& locators = {},
      const std::string& poolName = "__TEST_POOL1__", bool caching = true,
      bool clientNotificationEnabled = false,
      const std::chrono::seconds& ettl = std::chrono::seconds::zero(),
      const std::chrono::seconds& eit = std::chrono::seconds::zero(),
      const std::chrono::seconds& rttl = std::chrono::seconds::zero(),
      const std::chrono::seconds& rit = std::chrono::seconds::zero(),
      int lel = 0,
      const std::shared_ptr<CacheListener>& cacheListener = nullptr,
      ExpirationAction action = ExpirationAction::DESTROY);

  std::shared_ptr<Region> createSubregion(
      std::shared_ptr<Region>& parent, const char* name, bool ack, bool caching,
      const std::shared_ptr<CacheListener>& listener);

  std::shared_ptr<CacheableString> createCacheable(const char* value);

  void showKeys(std::vector<std::shared_ptr<CacheableKey>>& vecKeys);

  void showRegionAttributes(RegionAttributes attributes);

  std::shared_ptr<QueryService> getQueryService();

  /*
   * GFJAVA is the environment variable. user has to set GFJAVA variable as a
   * product build directory
   * path for java cache server or set as endpoints list for the remote server
   */

  static int staticHostPort1;
  static int staticHostPort2;
  static int staticHostPort3;
  static int staticHostPort4;

  static const std::string getTcrEndpoints(bool& isLocalServer,
                                           int numberOfServers = 1);

  static int staticLocatorHostPort1;
  static int staticLocatorHostPort2;
  static int staticLocatorHostPort3;

  static int staticJmxManagerPort;

  static std::string getstaticLocatorHostPort1();

  static std::string getstaticLocatorHostPort2();

  static std::string getLocatorHostPort(int locPort);

  static std::string getLocatorHostPort(bool& isLocator, bool& isLocalServer,
                                        int numberOfLocators = 0);

  static const std::string getTcrEndpoints2(bool& isLocalServer,
                                            int numberOfServers = 1);

  static std::list<int> staticServerInstanceList;
  static bool isServerCleanupCallbackRegistered;
  static void cleanupServerInstances();

  static void initServer(int instance, const std::string& xml = {},
                         const std::string& locHostport = {},
                         const char* authParam = nullptr, bool ssl = false,
                         bool enableDelta = true, bool multiDS = false,
                         bool testServerGC = false, bool untrustedCert = false,
                         bool useSecurityManager = false);

  static std::string createDuplicateXMLFile(const std::string& source,
                                            int hostport1, int hostport2,
                                            int locport1, int locport2);

  static std::string createDuplicateXMLFile(const std::string& source);

  static void closeServer(int instance);

  // closing locator
  static void closeLocator(int instance, bool ssl = false);
  template <class Rep, class Period>
  static void terminate_process_file(
      const std::string& pidFileName,
      const std::chrono::duration<Rep, Period>& duration);
  static bool file_exists(const std::string& fileName);
  static void read_single_line(const std::string& fileName, std::string& str);

  static void cleanupTmpConfigFiles();

  static void replacePortsInFile(int hostPort1, int hostPort2, int hostPort3,
                                 int hostPort4, int locPort1, int locPort2,
                                 const std::string& inFile,
                                 const std::string& outFile);

#ifdef _SOLARIS
  static void replaceInPlace(std::string& searchStr,
                             const std::string& matchStr,
                             const std::string& replaceStr);
#endif

  static std::list<int> staticLocatorInstanceList;
  static bool isLocatorCleanupCallbackRegistered;
  static void cleanupLocatorInstances();

  // starting locator
  static void initLocator(int instance, bool ssl = false, bool multiDS = false,
                          int dsId = -1, int remoteLocator = 0,
                          bool untrustedCert = false,
                          bool useSecurityManager = false);

  static void setJavaConnectionPoolSize(uint32_t size);

  static bool isSeedSet;
  static bool setSeed();

  static int hashcode(char* str);

  static int getRandomNumber();

  static int getRandomAvailablePort();

  static int staticMcastPort;
  static int staticMcastAddress;

 private:
  static std::string generateGeodeProperties(
      const std::string& path, const bool ssl = false, const int dsId = -1,
      const int remoteLocator = 0, const bool untrustedCert = false,
      const bool useSecurityManager = false);
};

#ifndef test_cppcache_utils_static
CacheHelper* CacheHelper::singleton = nullptr;
std::list<int> CacheHelper::staticServerInstanceList;
std::list<int> CacheHelper::staticLocatorInstanceList;
std::list<std::string> CacheHelper::staticConfigFileList;
bool CacheHelper::isServerCleanupCallbackRegistered = false;
bool CacheHelper::isLocatorCleanupCallbackRegistered = false;

bool CacheHelper::isSeedSet = CacheHelper::setSeed();
int CacheHelper::staticMcastAddress = CacheHelper::getRandomNumber() % 250 + 3;
int CacheHelper::staticMcastPort = CacheHelper::getRandomNumber();
int CacheHelper::staticHostPort1 = CacheHelper::getRandomAvailablePort();
int CacheHelper::staticHostPort2 = CacheHelper::getRandomAvailablePort();
int CacheHelper::staticHostPort3 = CacheHelper::getRandomAvailablePort();
int CacheHelper::staticHostPort4 = CacheHelper::getRandomAvailablePort();

int CacheHelper::staticLocatorHostPort1 = CacheHelper::getRandomAvailablePort();
int CacheHelper::staticLocatorHostPort2 = CacheHelper::getRandomAvailablePort();
int CacheHelper::staticLocatorHostPort3 = CacheHelper::getRandomAvailablePort();

int CacheHelper::staticJmxManagerPort = CacheHelper::getRandomAvailablePort();

#endif

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_INTEGRATION_TEST_CACHEHELPER_H_
