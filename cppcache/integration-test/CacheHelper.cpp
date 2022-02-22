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

#include <fstream>
#include <iostream>
#include <list>

#include <boost/process.hpp>
#include <boost/regex.hpp>

#include <geode/SystemProperties.hpp>
#include <geode/PoolManager.hpp>
#include <geode/internal/chrono/duration.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/RegionFactory.hpp>

#include "framework/GfshExecute.h"

#include "CacheRegionHelper.hpp"
#include "DistributedSystemImpl.hpp"
#include "TimeBomb.hpp"
#include "Utils.hpp"
#include "CacheImpl.hpp"

#include "CacheHelper.hpp"
#define __DUNIT_NO_MAIN__
#include "fw_dunit.hpp"

#include <chrono>
#include <thread>

#include <boost/asio.hpp>

#ifndef ROOT_NAME
#define ROOT_NAME "Root"
#endif

#ifndef ROOT_SCOPE
#define ROOT_SCOPE LOCAL
#endif

#if defined(WIN32)
#define COPY_COMMAND "copy /y"
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

extern ClientCleanup gClientCleanup;

namespace apache {
namespace geode {
namespace client {

#define RANDOM_NUMBER_OFFSET 14000
#define RANDOM_NUMBER_DIVIDER 15000
std::shared_ptr<Cache> CacheHelper::getCache() { return cachePtr; }

CacheHelper &CacheHelper::getHelper() {
  if (singleton == nullptr) {
    singleton = new CacheHelper();
  }
  return *singleton;
}

void CacheHelper::resetHelper() {
  if (singleton != nullptr) {
    delete singleton;
    singleton = nullptr;
  }
}

CacheHelper::CacheHelper(const char *,
                         const std::shared_ptr<Properties> &configPtr,
                         const bool noRootRegion) {
  auto pp = configPtr;
  if (pp == nullptr) {
    pp = Properties::create();
  }

  auto cacheFactory = CacheFactory(pp);
  cachePtr = std::make_shared<Cache>(cacheFactory.create());

  if (noRootRegion) return;

  try {
    auto regionFactory =
        cachePtr->createRegionFactory(RegionShortcut::CACHING_PROXY);
    rootRegionPtr = regionFactory.create(ROOT_NAME);
  } catch (const RegionExistsException &) {
    rootRegionPtr = cachePtr->getRegion(ROOT_NAME);
  }

  showRegionAttributes(rootRegionPtr->getAttributes());
}

/** rootRegionPtr will still be null... */
CacheHelper::CacheHelper(const char *, const std::string &cachexml,
                         const std::shared_ptr<Properties> &configPtr) {
  auto pp = configPtr;
  if (pp == nullptr) {
    pp = Properties::create();
  }
  if (!cachexml.empty()) {
    auto newFile = CacheHelper::createDuplicateXMLFile(cachexml);
    pp->insert("cache-xml-file", newFile.c_str());
  }
  auto cacheFactory = CacheFactory(pp);
  cachePtr = std::make_shared<Cache>(cacheFactory.create());
}

CacheHelper::CacheHelper(const std::shared_ptr<Properties> &configPtr,
                         const bool noRootRegion) {
  auto pp = configPtr;
  if (pp == nullptr) {
    pp = Properties::create();
  }

  auto cacheFactory = CacheFactory(pp);
  cachePtr = std::make_shared<Cache>(cacheFactory.create());

  auto poolFactory = cachePtr->getPoolManager().createFactory();
  addServerLocatorEPs("localhost:40404", poolFactory);
  poolFactory.create("__CACHE_HELPER_POOL__");

  if (noRootRegion) return;

  try {
    auto regionFactory =
        cachePtr->createRegionFactory(RegionShortcut::CACHING_PROXY);
    rootRegionPtr = regionFactory.create(ROOT_NAME);
  } catch (const RegionExistsException &) {
    rootRegionPtr = cachePtr->getRegion(ROOT_NAME);
  }

  showRegionAttributes(rootRegionPtr->getAttributes());
}

CacheHelper::CacheHelper(const bool,
                         const std::shared_ptr<AuthInitialize> &authInitialize,
                         const std::shared_ptr<Properties> &configPtr) {
  auto pp = configPtr;
  if (pp == nullptr) {
    pp = Properties::create();
  }
  try {
    LOG(" in cachehelper before createCacheFactory");
    auto cacheFactory = CacheFactory(pp).setAuthInitialize(authInitialize);
    cachePtr = std::make_shared<Cache>(cacheFactory.create());
  } catch (const Exception &excp) {
    LOG("Geode exception while creating cache, logged in following line");
    LOG(excp.what());
  } catch (...) {
    LOG("Throwing exception while creating cache....");
  }
}

CacheHelper::CacheHelper(const bool,
                         const std::shared_ptr<Properties> &configPtr,
                         const bool) {
  auto pp = configPtr;
  if (pp == nullptr) {
    pp = Properties::create();
  }
  LOG(" in cachehelper before createCacheFactory");
  auto cacheFactory = CacheFactory(pp);
  cachePtr = std::make_shared<Cache>(cacheFactory.create());
}

CacheHelper::CacheHelper(const bool, bool pdxIgnoreUnreadFields,
                         bool pdxReadSerialized,
                         const std::shared_ptr<Properties> &configPtr,
                         const bool) {
  auto pp = configPtr;
  if (pp == nullptr) {
    pp = Properties::create();
  }
  try {
    auto cfPtr = CacheFactory(pp);
    LOGINFO("pdxReadSerialized = %d ", pdxReadSerialized);
    LOGINFO("pdxIgnoreUnreadFields = %d ", pdxIgnoreUnreadFields);
    cfPtr.setPdxReadSerialized(pdxReadSerialized);
    cfPtr.setPdxIgnoreUnreadFields(pdxIgnoreUnreadFields);
    cachePtr = std::make_shared<Cache>(cfPtr.create());
  } catch (const Exception &excp) {
    LOG("Geode exception while creating cache, logged in following line");
    LOG(excp.what());
  } catch (...) {
    LOG("Throwing exception while creating cache....");
  }
}

CacheHelper::CacheHelper(const bool, const char *poolName,
                         const std::string &locators, const char *serverGroup,
                         const std::shared_ptr<Properties> &configPtr,
                         int redundancy, bool clientNotification,
                         int subscriptionAckInterval, int connections,
                         int loadConditioningInterval, bool isMultiuserMode,
                         bool prSingleHop, bool threadLocal) {
  auto pp = configPtr;
  if (pp == nullptr) {
    pp = Properties::create();
  }

  try {
    auto cacheFac = CacheFactory(pp);
    cachePtr = std::make_shared<Cache>(cacheFac.create());

    auto poolFactory = cachePtr->getPoolManager().createFactory();

    poolFactory.setPRSingleHopEnabled(prSingleHop);
    poolFactory.setThreadLocalConnections(threadLocal);
    std::cout << " Setting pr-single-hop to prSingleHop = " << prSingleHop
              << "\n";
    std::cout << "Setting threadLocal to " << threadLocal << "\n";
    if (!locators.empty()) {
      addServerLocatorEPs(locators, poolFactory);
      if (serverGroup) {
        poolFactory.setServerGroup(serverGroup);
      }
    }
    poolFactory.setSubscriptionRedundancy(redundancy);
    poolFactory.setSubscriptionEnabled(clientNotification);
    poolFactory.setMultiuserAuthentication(isMultiuserMode);
    if (loadConditioningInterval > 0) {
      poolFactory.setLoadConditioningInterval(
          std::chrono::milliseconds(loadConditioningInterval));
    }
    std::cout << "Setting connections to " << connections << "\n";
    if (connections >= 0) {
      poolFactory.setMinConnections(connections);
      poolFactory.setMaxConnections(connections);
    }
    if (subscriptionAckInterval != -1) {
      poolFactory.setSubscriptionAckInterval(
          std::chrono::milliseconds(subscriptionAckInterval));
    }

    poolFactory.create(poolName);

  } catch (const Exception &excp) {
    LOG("Geode exception while creating cache, logged in following line");
    LOG(excp.what());
  } catch (...) {
    LOG("Throwing exception while creating cache....");
  }
}

CacheHelper::CacheHelper(const int,
                         const std::shared_ptr<Properties> &configPtr) {
  auto pp = configPtr;
  if (pp == nullptr) {
    pp = Properties::create();
  }

  auto cacheFac = CacheFactory(pp);
  cachePtr = std::make_shared<Cache>(cacheFac.create());
}

CacheHelper::~CacheHelper() {
  // CacheHelper::cleanupTmpConfigFiles();
  disconnect();
}

void CacheHelper::disconnect(bool keepalive) {
  if (cachePtr == nullptr) {
    return;
  }

  LOG("Beginning cleanup after CacheHelper.");

  // rootRegionPtr->localDestroyRegion();
  rootRegionPtr = nullptr;
  LOG("Destroyed root region.");
  try {
    LOG("Closing cache.");
    if (cachePtr != nullptr) {
      cachePtr->close(keepalive);
    }
    LOG("Closing cache complete.");
  } catch (Exception &ex) {
    LOG("Exception thrown while closing cache: ");
    LOG(ex.what());
  } catch (...) {
    LOG("exception throw while closing cache");
  }

  LOG("Closed cache.");
  cachePtr = nullptr;
  singleton = nullptr;
  LOG("Finished cleanup after CacheHelper.");
}

void CacheHelper::createPlainRegion(const char *regionName,
                                    std::shared_ptr<Region> &regionPtr) {
  createPlainRegion(regionName, regionPtr, 10);
}

void CacheHelper::createPlainRegion(const char *regionName,
                                    std::shared_ptr<Region> &regionPtr,
                                    uint32_t size) {
  RegionAttributesFactory regionAttributesFactory;
  // set lru attributes...
  regionAttributesFactory.setLruEntriesLimit(0);     // no limit.
  regionAttributesFactory.setInitialCapacity(size);  // no limit.
  // then...
  auto regionAttributes = regionAttributesFactory.create();
  showRegionAttributes(regionAttributes);
  // This is using subregions (deprecated) so not placing the new cache API here
  regionPtr = rootRegionPtr->createSubregion(regionName, regionAttributes);
  ASSERT(regionPtr != nullptr, "failed to create region.");
}

void CacheHelper::createLRURegion(const char *regionName,
                                  std::shared_ptr<Region> &regionPtr) {
  createLRURegion(regionName, regionPtr, 10);
}
void CacheHelper::createLRURegion(const char *regionName,
                                  std::shared_ptr<Region> &regionPtr,
                                  uint32_t size) {
  RegionAttributesFactory regionAttributesFactory;
  // set lru attributes...
  regionAttributesFactory.setLruEntriesLimit(size);
  regionAttributesFactory.setInitialCapacity(size);
  // then...
  auto regionAttributes = regionAttributesFactory.create();
  showRegionAttributes(regionAttributes);
  // This is using subregions (deprecated) so not placing the new cache API here
  regionPtr = rootRegionPtr->createSubregion(regionName, regionAttributes);
  ASSERT(regionPtr != nullptr, "failed to create region.");
}

void CacheHelper::createDistRegion(const char *regionName,
                                   std::shared_ptr<Region> &regionPtr,
                                   uint32_t size) {
  RegionAttributesFactory regionAttributesFactory;
  // set lru attributes...
  regionAttributesFactory.setLruEntriesLimit(0);     // no limit.
  regionAttributesFactory.setInitialCapacity(size);  // no limit.
  // then...
  auto regionAttributes = regionAttributesFactory.create();
  showRegionAttributes(regionAttributes);
  // This is using subregions (deprecated) so not placing the new cache API here
  regionPtr = rootRegionPtr->createSubregion(regionName, regionAttributes);
  ASSERT(regionPtr != nullptr, "failed to create region.");
}
std::shared_ptr<Region> CacheHelper::getRegion(const std::string &name) {
  return cachePtr->getRegion(name);
}
std::shared_ptr<Region> CacheHelper::createRegion(
    const std::string &name, bool, bool caching,
    const std::shared_ptr<CacheListener> &listener, bool, bool,
    bool concurrencyCheckEnabled, int32_t) {
  RegionAttributesFactory regionAttributeFactory;
  regionAttributeFactory.setCachingEnabled(caching);
  if (listener != nullptr) {
    regionAttributeFactory.setCacheListener(listener);
  }
  if (concurrencyCheckEnabled) {
    regionAttributeFactory.setConcurrencyChecksEnabled(concurrencyCheckEnabled);
  }

  auto regionAttributes = regionAttributeFactory.create();

  CacheImpl *cacheImpl = CacheRegionHelper::getCacheImpl(cachePtr.get());
  std::shared_ptr<Region> regionPtr;
  cacheImpl->createRegion(name, regionAttributes, regionPtr);
  return regionPtr;
}
std::shared_ptr<Region> CacheHelper::createRegion(
    const std::string &name, bool, bool caching,
    const std::chrono::seconds &ettl, const std::chrono::seconds &eit,
    const std::chrono::seconds &rttl, const std::chrono::seconds &rit, int lel,
    ExpirationAction action, const std::string &, bool) {
  RegionAttributesFactory regionAttributeFactory;
  regionAttributeFactory.setCachingEnabled(caching);
  regionAttributeFactory.setLruEntriesLimit(lel);
  regionAttributeFactory.setEntryIdleTimeout(action, eit);
  regionAttributeFactory.setEntryTimeToLive(action, ettl);
  regionAttributeFactory.setRegionIdleTimeout(action, rit);
  regionAttributeFactory.setRegionTimeToLive(action, rttl);

  auto regionAttributes = regionAttributeFactory.create();

  CacheImpl *cacheImpl = CacheRegionHelper::getCacheImpl(cachePtr.get());
  std::shared_ptr<Region> regionPtr;
  cacheImpl->createRegion(name, regionAttributes, regionPtr);
  return regionPtr;
}
std::shared_ptr<Pool> CacheHelper::createPool(
    const std::string &poolName, const std::string &locators,
    const std::string &serverGroup, int redundancy, bool clientNotification,
    std::chrono::milliseconds subscriptionAckInterval, int connections,
    int loadConditioningInterval, bool isMultiuserMode) {
  auto poolFac = getCache()->getPoolManager().createFactory();

  addServerLocatorEPs(locators, poolFac);
  if (!serverGroup.empty()) {
    poolFac.setServerGroup(serverGroup);
  }

  poolFac.setSubscriptionRedundancy(redundancy);
  poolFac.setSubscriptionEnabled(clientNotification);
  poolFac.setMultiuserAuthentication(isMultiuserMode);
  // poolFac.setStatisticInterval(1000);
  if (loadConditioningInterval > 0) {
    poolFac.setLoadConditioningInterval(
        std::chrono::milliseconds(loadConditioningInterval));
  }

  if (connections >= 0) {
    poolFac.setMinConnections(connections);
    poolFac.setMaxConnections(connections);
  }
  if (subscriptionAckInterval > std::chrono::milliseconds::zero()) {
    poolFac.setSubscriptionAckInterval(subscriptionAckInterval);
  }

  return poolFac.create(poolName);
}

// this will create pool even endpoints and locatorhost has been not defined
std::shared_ptr<Pool> CacheHelper::createPool2(
    const std::string &poolName, const std::string &locators,
    const std::string &serverGroup, const std::string &servers, int redundancy,
    bool clientNotification, int subscriptionAckInterval, int connections) {
  auto poolFac = getCache()->getPoolManager().createFactory();

  if (!servers.empty()) {
    addServerLocatorEPs(servers, poolFac, false);
    // do region creation with end
  } else if (!locators.empty()) {
    addServerLocatorEPs(locators, poolFac);
    if (!serverGroup.empty()) {
      poolFac.setServerGroup(serverGroup);
    }
  }

  poolFac.setSubscriptionRedundancy(redundancy);
  poolFac.setSubscriptionEnabled(clientNotification);
  if (connections >= 0) {
    poolFac.setMinConnections(connections);
    poolFac.setMaxConnections(connections);
  }
  if (subscriptionAckInterval != -1) {
    poolFac.setSubscriptionAckInterval(
        std::chrono::milliseconds(subscriptionAckInterval));
  }

  return poolFac.create(poolName);
}

void CacheHelper::logPoolAttributes(std::shared_ptr<Pool> &pool) {
  using apache::geode::internal::chrono::duration::to_string;

  LOG("logPoolAttributes() entered");
  LOGINFO("CPPTEST: Pool attributes for pool %s are as follows" +
          pool->getName());
  LOGINFO("getFreeConnectionTimeout: " +
          to_string(pool->getFreeConnectionTimeout()));
  LOGINFO("getLoadConditioningInterval: " +
          to_string(pool->getLoadConditioningInterval()));
  LOGINFO("getSocketBufferSize: %d", pool->getSocketBufferSize());
  LOGINFO("getReadTimeout: " + to_string(pool->getReadTimeout()));
  LOGINFO("getMinConnections: %d", pool->getMinConnections());
  LOGINFO("getMaxConnections: %d", pool->getMaxConnections());
  LOGINFO("getIdleTimeout: " + to_string(pool->getIdleTimeout()));
  LOGINFO("getPingInterval: " + to_string(pool->getPingInterval()));
  LOGINFO("getStatisticInterval: " + to_string(pool->getStatisticInterval()));
  LOGINFO("getRetryAttempts: %d", pool->getRetryAttempts());
  LOGINFO("getSubscriptionEnabled: %s",
          pool->getSubscriptionEnabled() ? "true" : "false");
  LOGINFO("getSubscriptionRedundancy: %d", pool->getSubscriptionRedundancy());
  LOGINFO("getSubscriptionMessageTrackingTimeout: " +
          to_string(pool->getSubscriptionMessageTrackingTimeout()));
  LOGINFO("getSubscriptionAckInterval: " +
          to_string(pool->getSubscriptionAckInterval()));
  LOGINFO("getServerGroup: " + pool->getServerGroup());
  LOGINFO("getThreadLocalConnections: %s",
          pool->getThreadLocalConnections() ? "true" : "false");
  LOGINFO("getPRSingleHopEnabled: %s",
          pool->getPRSingleHopEnabled() ? "true" : "false");
}

void CacheHelper::createPoolWithLocators(
    const std::string &name, const std::string &locators,
    bool clientNotificationEnabled, int subscriptionRedundancy,
    std::chrono::milliseconds subscriptionAckInterval, int connections,
    bool isMultiuserMode, const std::string &serverGroup) {
  LOG("createPool() entered.");
  std::cout << " in createPoolWithLocators isMultiuserMode = "
            << isMultiuserMode << "\n";
  auto poolPtr = createPool(name, locators, serverGroup, subscriptionRedundancy,
                            clientNotificationEnabled, subscriptionAckInterval,
                            connections, -1, isMultiuserMode);
  ASSERT(poolPtr != nullptr, "Failed to create pool.");
  logPoolAttributes(poolPtr);
  LOG("Pool created.");
}
std::shared_ptr<Region> CacheHelper::createRegionAndAttachPool(
    const std::string &name, bool, const std::string &poolName, bool caching,
    const std::chrono::seconds &ettl, const std::chrono::seconds &eit,
    const std::chrono::seconds &rttl, const std::chrono::seconds &rit, int lel,
    ExpirationAction action) {
  RegionShortcut preDefRA = RegionShortcut::PROXY;
  if (caching) {
    preDefRA = RegionShortcut::CACHING_PROXY;
  }
  if (lel > 0) {
    preDefRA = RegionShortcut::CACHING_PROXY_ENTRY_LRU;
  }
  auto regionFactory = cachePtr->createRegionFactory(preDefRA);
  regionFactory.setLruEntriesLimit(lel);
  regionFactory.setEntryIdleTimeout(action, eit);
  regionFactory.setEntryTimeToLive(action, ettl);
  regionFactory.setRegionIdleTimeout(action, rit);
  regionFactory.setRegionTimeToLive(action, rttl);
  if (!poolName.empty()) {
    regionFactory.setPoolName(poolName);
  }
  return regionFactory.create(name);
}
std::shared_ptr<Region> CacheHelper::createRegionAndAttachPool2(
    const char *name, bool, const char *poolName,
    const std::shared_ptr<PartitionResolver> &aResolver, bool caching,
    const std::chrono::seconds &ettl, const std::chrono::seconds &eit,
    const std::chrono::seconds &rttl, const std::chrono::seconds &rit, int lel,
    ExpirationAction action) {
  RegionShortcut preDefRA = RegionShortcut::PROXY;
  if (caching) {
    preDefRA = RegionShortcut::CACHING_PROXY;
  }
  if (lel > 0) {
    preDefRA = RegionShortcut::CACHING_PROXY_ENTRY_LRU;
  }
  auto regionFactory = cachePtr->createRegionFactory(preDefRA);
  regionFactory.setLruEntriesLimit(lel);
  regionFactory.setEntryIdleTimeout(action, eit);
  regionFactory.setEntryTimeToLive(action, ettl);
  regionFactory.setRegionIdleTimeout(action, rit);
  regionFactory.setRegionTimeToLive(action, rttl);
  regionFactory.setPoolName(poolName);
  regionFactory.setPartitionResolver(aResolver);
  return regionFactory.create(name);
}

void CacheHelper::addServerLocatorEPs(const std::string &epList,
                                      PoolFactory &pf, bool poolLocators) {
  std::unordered_set<std::string> endpointNames;
  Utils::parseEndpointNamesString(epList, endpointNames);
  for (const auto &endpointName : endpointNames) {
    auto position = endpointName.find_first_of(":");
    if (position != std::string::npos) {
      auto hostname = endpointName.substr(0, position);
      auto portnumber = std::stoi(endpointName.substr(position + 1));
      if (poolLocators) {
        pf.addLocator(hostname, portnumber);
      } else {
        pf.addServer(hostname, portnumber);
      }
    }
  }
}

std::shared_ptr<Region> CacheHelper::createPooledRegion(
    const std::string &name, bool, const std::string &locators,
    const std::string &poolName, bool caching, bool clientNotificationEnabled,
    const std::chrono::seconds &ettl, const std::chrono::seconds &eit,
    const std::chrono::seconds &rttl, const std::chrono::seconds &rit, int lel,
    const std::shared_ptr<CacheListener> &cacheListener,
    ExpirationAction action) {
  auto poolFac = getCache()->getPoolManager().createFactory();
  poolFac.setSubscriptionEnabled(clientNotificationEnabled);

  if (!locators.empty()) {
    LOG("adding pool locators");
    addServerLocatorEPs(locators, poolFac);
  }

  if ((getCache()->getPoolManager().find(poolName)) ==
      nullptr) {  // Pool does not exist with the same name.
    auto pptr = poolFac.create(poolName);
  }

  RegionShortcut preDefRA = RegionShortcut::PROXY;
  if (caching) {
    preDefRA = RegionShortcut::CACHING_PROXY;
  }
  if (lel > 0) {
    preDefRA = RegionShortcut::CACHING_PROXY_ENTRY_LRU;
  }
  auto regionFactory = cachePtr->createRegionFactory(preDefRA);
  regionFactory.setLruEntriesLimit(lel);
  regionFactory.setEntryIdleTimeout(action, eit);
  regionFactory.setEntryTimeToLive(action, ettl);
  regionFactory.setRegionIdleTimeout(action, rit);
  regionFactory.setRegionTimeToLive(action, rttl);
  regionFactory.setPoolName(poolName);
  if (cacheListener != nullptr) {
    regionFactory.setCacheListener(cacheListener);
  }
  return regionFactory.create(name);
}
std::shared_ptr<Region> CacheHelper::createPooledRegionConcurrencyCheckDisabled(
    const std::string &name, bool, const std::string &locators,
    const std::string &poolName, bool caching, bool clientNotificationEnabled,
    bool concurrencyCheckEnabled, const std::chrono::seconds &ettl,
    const std::chrono::seconds &eit, const std::chrono::seconds &rttl,
    const std::chrono::seconds &rit, int lel,
    const std::shared_ptr<CacheListener> &cacheListener,
    ExpirationAction action) {
  auto poolFac = getCache()->getPoolManager().createFactory();
  poolFac.setSubscriptionEnabled(clientNotificationEnabled);

  LOG("adding pool locators");
  addServerLocatorEPs(locators, poolFac);

  if ((getCache()->getPoolManager().find(poolName)) ==
      nullptr) {  // Pool does not exist with the same name.
    auto pptr = poolFac.create(poolName);
  }

  RegionShortcut preDefRA = RegionShortcut::PROXY;
  if (caching) {
    preDefRA = RegionShortcut::CACHING_PROXY;
  }
  if (lel > 0) {
    preDefRA = RegionShortcut::CACHING_PROXY_ENTRY_LRU;
  }
  auto regionFactory = cachePtr->createRegionFactory(preDefRA);
  regionFactory.setLruEntriesLimit(lel);
  regionFactory.setEntryIdleTimeout(action, eit);
  regionFactory.setEntryTimeToLive(action, ettl);
  regionFactory.setRegionIdleTimeout(action, rit);
  regionFactory.setRegionTimeToLive(action, rttl);
  regionFactory.setConcurrencyChecksEnabled(concurrencyCheckEnabled);
  regionFactory.setPoolName(poolName);
  if (cacheListener != nullptr) {
    regionFactory.setCacheListener(cacheListener);
  }
  return regionFactory.create(name);
}
std::shared_ptr<Region> CacheHelper::createRegionDiscOverFlow(
    const std::string &name, bool caching, bool,
    const std::chrono::seconds &ettl, const std::chrono::seconds &eit,
    const std::chrono::seconds &rttl, const std::chrono::seconds &rit, int lel,
    ExpirationAction action) {
  RegionAttributesFactory regionAttributeFactory;
  regionAttributeFactory.setCachingEnabled(caching);
  regionAttributeFactory.setLruEntriesLimit(lel);
  regionAttributeFactory.setEntryIdleTimeout(action, eit);
  regionAttributeFactory.setEntryTimeToLive(action, ettl);
  regionAttributeFactory.setRegionIdleTimeout(action, rit);
  regionAttributeFactory.setRegionTimeToLive(action, rttl);
  regionAttributeFactory.setCloningEnabled(true);
  if (lel > 0) {
    regionAttributeFactory.setDiskPolicy(DiskPolicyType::OVERFLOWS);
    auto sqLiteProps = Properties::create();
    sqLiteProps->insert("PageSize", "65536");
    sqLiteProps->insert("MaxPageCount", "1073741823");
    std::string sqlite_dir =
        "SqLiteRegionData" + std::to_string(boost::this_process::get_id());
    sqLiteProps->insert("PersistenceDirectory", sqlite_dir.c_str());
    regionAttributeFactory.setPersistenceManager(
        "SqLiteImpl", "createSqLiteInstance", sqLiteProps);
  }

  auto regionAttributes = regionAttributeFactory.create();
  CacheImpl *cacheImpl = CacheRegionHelper::getCacheImpl(cachePtr.get());
  std::shared_ptr<Region> regionPtr;
  cacheImpl->createRegion(name, regionAttributes, regionPtr);
  return regionPtr;
}
std::shared_ptr<Region> CacheHelper::createPooledRegionDiscOverFlow(
    const std::string &name, bool, const std::string &locators,
    const std::string &poolName, bool caching, bool clientNotificationEnabled,
    const std::chrono::seconds &ettl, const std::chrono::seconds &eit,
    const std::chrono::seconds &rttl, const std::chrono::seconds &rit, int lel,
    const std::shared_ptr<CacheListener> &cacheListener,
    ExpirationAction action) {
  auto poolFac = getCache()->getPoolManager().createFactory();
  poolFac.setSubscriptionEnabled(clientNotificationEnabled);

  if (!locators.empty())  // with locator
  {
    LOG("adding pool locators");
    addServerLocatorEPs(locators, poolFac);
  }
  if ((getCache()->getPoolManager().find(poolName)) ==
      nullptr) {  // Pool does not exist with the same name.
    auto pptr = poolFac.create(poolName);
  }

  if (!caching) {
    LOG("createPooledRegionDiscOverFlow: setting caching=false does not make "
        "sense");
    FAIL(
        "createPooledRegionDiscOverFlow: setting caching=false does not make "
        "sense");
  }
  RegionShortcut preDefRA = RegionShortcut::CACHING_PROXY;
  if (lel > 0) {
    preDefRA = RegionShortcut::CACHING_PROXY_ENTRY_LRU;
  }
  auto regionFactory = cachePtr->createRegionFactory(preDefRA);
  regionFactory.setLruEntriesLimit(lel);
  regionFactory.setEntryIdleTimeout(action, eit);
  regionFactory.setEntryTimeToLive(action, ettl);
  regionFactory.setRegionIdleTimeout(action, rit);
  regionFactory.setRegionTimeToLive(action, rttl);
  regionFactory.setPoolName(poolName);
  regionFactory.setCloningEnabled(true);
  if (lel > 0) {
    regionFactory.setDiskPolicy(DiskPolicyType::OVERFLOWS);
    auto sqLiteProps = Properties::create();
    sqLiteProps->insert("PageSize", "65536");
    sqLiteProps->insert("MaxPageCount", "1073741823");
    std::string sqlite_dir =
        "SqLiteRegionData" + std::to_string(boost::this_process::get_id());
    sqLiteProps->insert("PersistenceDirectory", sqlite_dir.c_str());
    regionFactory.setPersistenceManager("SqLiteImpl", "createSqLiteInstance",
                                        sqLiteProps);
  }
  if (cacheListener != nullptr) {
    regionFactory.setCacheListener(cacheListener);
  }
  return regionFactory.create(name);
}

std::shared_ptr<Region> CacheHelper::createPooledRegionSticky(
    const std::string &name, bool, const std::string &locators,
    const std::string &poolName, bool caching, bool clientNotificationEnabled,
    const std::chrono::seconds &ettl, const std::chrono::seconds &eit,
    const std::chrono::seconds &rttl, const std::chrono::seconds &rit, int lel,
    const std::shared_ptr<CacheListener> &cacheListener,
    ExpirationAction action) {
  auto poolFac = getCache()->getPoolManager().createFactory();
  poolFac.setSubscriptionEnabled(clientNotificationEnabled);
  poolFac.setThreadLocalConnections(true);
  poolFac.setPRSingleHopEnabled(false);

  LOG("adding pool locators");
  addServerLocatorEPs(locators, poolFac);

  if ((getCache()->getPoolManager().find(poolName)) ==
      nullptr) {  // Pool does not exist with the same name.
    auto pptr = poolFac.create(poolName);
    LOG("createPooledRegionSticky logPoolAttributes");
    logPoolAttributes(pptr);
  }

  RegionShortcut preDefRA = RegionShortcut::PROXY;
  if (caching) {
    preDefRA = RegionShortcut::CACHING_PROXY;
  }
  if (lel > 0) {
    preDefRA = RegionShortcut::CACHING_PROXY_ENTRY_LRU;
  }
  auto regionFactory = cachePtr->createRegionFactory(preDefRA);
  regionFactory.setLruEntriesLimit(lel);
  regionFactory.setEntryIdleTimeout(action, eit);
  regionFactory.setEntryTimeToLive(action, ettl);
  regionFactory.setRegionIdleTimeout(action, rit);
  regionFactory.setRegionTimeToLive(action, rttl);
  regionFactory.setPoolName(poolName);
  if (cacheListener != nullptr) {
    regionFactory.setCacheListener(cacheListener);
  }
  return regionFactory.create(name);
}
std::shared_ptr<Region> CacheHelper::createPooledRegionStickySingleHop(
    const std::string &name, bool, const std::string &locators,
    const std::string &poolName, bool caching, bool clientNotificationEnabled,
    const std::chrono::seconds &ettl, const std::chrono::seconds &eit,
    const std::chrono::seconds &rttl, const std::chrono::seconds &rit, int lel,
    const std::shared_ptr<CacheListener> &cacheListener,
    ExpirationAction action) {
  LOG("createPooledRegionStickySingleHop");
  auto poolFac = getCache()->getPoolManager().createFactory();
  poolFac.setSubscriptionEnabled(clientNotificationEnabled);
  poolFac.setThreadLocalConnections(true);
  poolFac.setPRSingleHopEnabled(true);
  LOG("adding pool locators");
  addServerLocatorEPs(locators, poolFac);

  if ((getCache()->getPoolManager().find(poolName)) ==
      nullptr) {  // Pool does not exist with the same name.
    auto pptr = poolFac.create(poolName);
    LOG("createPooledRegionStickySingleHop logPoolAttributes");
    logPoolAttributes(pptr);
  }

  RegionShortcut preDefRA = RegionShortcut::PROXY;
  if (caching) {
    preDefRA = RegionShortcut::CACHING_PROXY;
  }
  if (lel > 0) {
    preDefRA = RegionShortcut::CACHING_PROXY_ENTRY_LRU;
  }
  auto regionFactory = cachePtr->createRegionFactory(preDefRA);
  regionFactory.setLruEntriesLimit(lel);
  regionFactory.setEntryIdleTimeout(action, eit);
  regionFactory.setEntryTimeToLive(action, ettl);
  regionFactory.setRegionIdleTimeout(action, rit);
  regionFactory.setRegionTimeToLive(action, rttl);
  regionFactory.setPoolName(poolName);
  if (cacheListener != nullptr) {
    regionFactory.setCacheListener(cacheListener);
  }
  return regionFactory.create(name);
}
std::shared_ptr<Region> CacheHelper::createSubregion(
    std::shared_ptr<Region> &parent, const char *name, bool, bool caching,
    const std::shared_ptr<CacheListener> &listener) {
  RegionAttributesFactory regionAttributeFactory;
  regionAttributeFactory.setCachingEnabled(caching);
  if (listener != nullptr) {
    regionAttributeFactory.setCacheListener(listener);
  }
  auto regionAttributes = regionAttributeFactory.create();

  return parent->createSubregion(name, regionAttributes);
}
std::shared_ptr<CacheableString> CacheHelper::createCacheable(
    const char *value) {
  return CacheableString::create(value);
}

void CacheHelper::showKeys(
    std::vector<std::shared_ptr<CacheableKey>> &vecKeys) {
  std::cout << "vecKeys.size() = " << vecKeys.size() << "\n";
  for (size_t i = 0; i < vecKeys.size(); i++) {
    std::cout << "key[" << i << "] -" << vecKeys.at(i)->toString() << "\n";
  }
  std::cout << std::flush;
}

void CacheHelper::showRegionAttributes(RegionAttributes attributes) {
  std::cout << "caching=" << (attributes.getCachingEnabled() ? "true" : "false")
            << "\n";
  std::cout << "Entry Time To Live = "
            << to_string(attributes.getEntryTimeToLive()) << "\n";
  std::cout << "Entry Idle Timeout = "
            << to_string(attributes.getEntryIdleTimeout()) << "\n";
  std::cout << "Region Time To Live = "
            << to_string(attributes.getRegionTimeToLive()) << "\n";
  std::cout << "Region Idle Timeout = "
            << to_string(attributes.getRegionIdleTimeout()) << "\n";
  std::cout << "Initial Capacity = " << attributes.getInitialCapacity() << "\n";
  std::cout << "Load Factor = " << attributes.getLoadFactor() << "\n";
  std::cout << "End Points = " << attributes.getEndpoints() << "\n";
}

std::shared_ptr<QueryService> CacheHelper::getQueryService() {
  return cachePtr->getQueryService();
}

const std::string CacheHelper::getTcrEndpoints(bool &isLocalServer,
                                               int numberOfServers) {
  static bool gflocalserver = false;
  static const auto gfjavaenv = Utils::getEnv("GFJAVA");

  ASSERT(!gfjavaenv.empty(),
         "Environment variable GFJAVA for java build directory is not set.");

  std::string gfendpoints;
  if (gfjavaenv.find(PATH_SEP) != std::string::npos) {
    gflocalserver = true;
    /* Support for multiple servers Max = 10*/
    switch (numberOfServers) {
      case 1:
        // gfendpoints = "localhost:24680";
        {
          gfendpoints = "localhost:";
          gfendpoints += std::to_string(CacheHelper::staticHostPort1);
        }
        break;
      case 2:
        // gfendpoints = "localhost:24680,localhost:24681";
        {
          gfendpoints = "localhost:";
          gfendpoints += std::to_string(CacheHelper::staticHostPort1);
          gfendpoints += ",localhost:";
          gfendpoints += std::to_string(CacheHelper::staticHostPort2);
        }
        break;
      case 3:
        // gfendpoints = "localhost:24680,localhost:24681,localhost:24682";
        {
          gfendpoints = "localhost:";
          gfendpoints += std::to_string(CacheHelper::staticHostPort1);
          gfendpoints += ",localhost:";
          gfendpoints += std::to_string(CacheHelper::staticHostPort2);
          gfendpoints += ",localhost:";
          gfendpoints += std::to_string(CacheHelper::staticHostPort3);
        }
        break;
      default:
        // ASSERT( ( numberOfServers <= 10 )," More than 10 servers not
        // supported");
        // TODO: need to support more servers, need to generate random ports
        // here
        ASSERT((numberOfServers <= 4), " More than 4 servers not supported");
        gfendpoints = "localhost:";
        gfendpoints += std::to_string(CacheHelper::staticHostPort1);
        gfendpoints += ",localhost:";
        gfendpoints += std::to_string(CacheHelper::staticHostPort2);
        gfendpoints += ",localhost:";
        gfendpoints += std::to_string(CacheHelper::staticHostPort3);
        gfendpoints += ",localhost:";
        gfendpoints += std::to_string(CacheHelper::staticHostPort4);
        break;
    }
  } else {
    gfendpoints = gfjavaenv;
  }

  isLocalServer = gflocalserver;
  std::cout << "getHostPort :: " << gfendpoints << "\n";

  return gfendpoints;
}

std::string CacheHelper::getstaticLocatorHostPort1() {
  return getLocatorHostPort(staticLocatorHostPort1);
}

std::string CacheHelper::getstaticLocatorHostPort2() {
  return getLocatorHostPort(staticLocatorHostPort2);
}

std::string CacheHelper::getLocatorHostPort(int locPort) {
  return "localhost:" + std::to_string(locPort);
}

std::string CacheHelper::getLocatorHostPort(bool &isLocator,
                                            bool &isLocalServer,
                                            int numberOfLocators) {
  static const auto gfjavaenv = Utils::getEnv("GFJAVA");
  static bool gflocator = false;
  static bool gflocalserver = false;

  ASSERT(!gfjavaenv.empty(),
         "Environment variable GFJAVA for java build directory is not set.");

  std::string gflchostport;
  if (gfjavaenv.find(PATH_SEP) != std::string::npos) {
    gflocator = true;
    gflocalserver = true;
    switch (numberOfLocators) {
      case 1:
        // gflchostport = "localhost:34756";
        {
          gflchostport = "localhost:";
          gflchostport += std::to_string(CacheHelper::staticLocatorHostPort1);
        }
        break;
      case 2:
        // gflchostport = "localhost:34756,localhost:34757";
        {
          gflchostport = "localhost:";
          gflchostport += std::to_string(CacheHelper::staticLocatorHostPort1);
          gflchostport += ",localhost:";
          gflchostport += std::to_string(CacheHelper::staticLocatorHostPort2);
        }
        break;
      default:
        // gflchostport = "localhost:34756,localhost:34757,localhost:34758";
        {
          gflchostport = "localhost:";
          gflchostport += std::to_string(CacheHelper::staticLocatorHostPort1);
          gflchostport += ",localhost:";
          gflchostport += std::to_string(CacheHelper::staticLocatorHostPort2);
          gflchostport += ",localhost:";
          gflchostport += std::to_string(CacheHelper::staticLocatorHostPort3);
        }
        break;
    }
  }

  isLocator = gflocator;
  isLocalServer = gflocalserver;
  std::cout << "getLocatorHostPort  :: " << gflchostport << "\n";

  return gflchostport;
}

void CacheHelper::cleanupServerInstances() {
  CacheHelper::cleanupTmpConfigFiles();
  if (staticServerInstanceList.size() > 0) {
    while (staticServerInstanceList.size() > 0) {
      int instance = staticServerInstanceList.front();

      staticServerInstanceList.remove(instance);  // for safety
      closeServer(instance);
    }
  }
}
void CacheHelper::initServer(int instance, const std::string &xml,
                             const std::string &locHostport,
                             const char * /*unused*/, bool ssl,
                             bool enableDelta, bool, bool testServerGC,
                             bool untrustedCert, bool useSecurityManager) {
  if (!isServerCleanupCallbackRegistered) {
    isServerCleanupCallbackRegistered = true;
    gClientCleanup.registerCallback(
        []() { CacheHelper::cleanupServerInstances(); });

    std::cout << "TimeBomb registered server cleanupcallback \n";
  }

  std::cout << "Inside initServer added\n";

  static const auto gfjavaenv = Utils::getEnv("GFJAVA");
  static auto gfLogLevel = Utils::getEnv("GFE_LOGLEVEL");
  static auto gfSecLogLevel = Utils::getEnv("GFE_SECLOGLEVEL");
  static const auto path = Utils::getEnv("TESTSRC");
  static const auto classpath = Utils::getEnv("GF_CLASSPATH");

  int portNum = 0;
  std::string currDir = boost::filesystem::current_path().string();

  ASSERT(!gfjavaenv.empty(),
         "Environment variable GFJAVA for java build directory is not set.");
  ASSERT(!path.empty(),
         "Environment variable TESTSRC for test source directory is not set.");

  if (gfLogLevel.empty()) {
    gfLogLevel = "config";
  }

  if (gfSecLogLevel.empty()) {
    gfSecLogLevel = "config";
  }

  if (gfjavaenv.find(PATH_SEP) == std::string::npos) {
    return;
  }

  std::string xmlFile = "";
  std::string sname = "GFECS";
  currDir += PATH_SEP;

  switch (instance) {
    case 0:
      // note: this need to take for multiple tests run
      xmlFile += "cacheserver.xml";
      break;
    case 1:
      xmlFile += "cacheserver.xml";
      portNum = CacheHelper::staticHostPort1;
      break;
    case 2:
      xmlFile += "cacheserver2.xml";
      portNum = CacheHelper::staticHostPort2;
      break;
    case 3:
      xmlFile += "cacheserver3.xml";
      portNum = CacheHelper::staticHostPort3;
      break;
    case 4:
      xmlFile += "cacheserver4.xml";
      portNum = CacheHelper::staticHostPort4;
      break;
    default: /* Support for any number of servers Max 10*/
      ASSERT((instance <= 10), " More than 10 servers not supported");
      ASSERT(!xml.empty(),
             "xml == nullptr : For server instance > 3 xml file is must");
      portNum = CacheHelper::staticHostPort4;
      break;
  }

  sname += std::to_string(portNum);
  currDir += sname;

  if (!xml.empty()) {
    xmlFile = xml;
  }

  std::string xmlFile_new;
  std::cout << " xml file name = " << xmlFile << "\n";
  xmlFile = CacheHelper::createDuplicateXMLFile(xmlFile);

  std::cout << "  creating dir = " << sname << "\n";
  boost::filesystem::create_directory(sname);

  int64_t defaultTombstone_timeout = 600000;
  int64_t defaultTombstone_gc_threshold = 100000;
  int64_t userTombstone_timeout = 1000;
  int64_t userTombstone_gc_threshold = 10;
  if (testServerGC) {
    boost::filesystem::create_directory("backupDirectory1");
    boost::filesystem::create_directory("backupDirectory2");
    boost::filesystem::create_directory("backupDirectory3");
    boost::filesystem::create_directory("backupDirectory4");
  }

  GfshExecute gfsh;
  auto server =
      gfsh.start()
          .server()
          .withClasspath(classpath)
          .withName(sname)
          .withCacheXMLFile(xmlFile)
          .withDir(currDir)
          .withPort(portNum)
          .withLogLevel(gfLogLevel)
          .withMaxHeap("1g")
          .withSystemProperty(
              "gemfire.tombstone-timeout",
              std::to_string(testServerGC ? userTombstone_timeout
                                          : defaultTombstone_timeout))
          .withSystemProperty(
              "gemfire.tombstone-gc-threshold",
              std::to_string(testServerGC ? userTombstone_gc_threshold
                                          : defaultTombstone_gc_threshold))
          .withSystemProperty("gemfire.security-log-level", gfSecLogLevel);

  if (useSecurityManager) {
    server.withUser("root").withPassword("root-password");
  }

  if (!locHostport.empty()) {
    server.withPropertiesFile(generateGeodeProperties(
        currDir, ssl, -1, 0, untrustedCert, useSecurityManager));
  }

  if (!enableDelta) {
    server.withSystemProperty("gemfire.delta-propagation", "false");
  }

  server.execute();

  staticServerInstanceList.push_back(instance);
  std::cout << "added server instance " << instance << "\n";
}

std::string CacheHelper::createDuplicateXMLFile(const std::string &source,
                                                int hostport1, int hostport2,
                                                int locport1, int locport2) {
  std::string dest = boost::filesystem::current_path().string();
  dest += PATH_SEP;
  dest += boost::filesystem::path{source}.filename().stem().string();
  dest += '.';
  dest += std::to_string(hostport1);
  dest += ".xml";

  std::string src = Utils::getEnv("TESTSRC");
  src += PATH_SEP;
  src += "resources";
  src += PATH_SEP;
  src += source;

  replacePortsInFile(hostport1, hostport2, CacheHelper::staticHostPort3,
                     CacheHelper::staticHostPort4, locport1, locport2, src,
                     dest);

  CacheHelper::staticConfigFileList.push_back(dest);
  std::cout << "createDuplicateXMLFile added file " << dest.c_str() << " "
            << CacheHelper::staticConfigFileList.size() << "\n";

  return dest;
}

// Need to avoid regex usage in Solaris Studio 12.4.
#ifdef _SOLARIS
// @Solaris 12.4 compiler is missing support for C++11 regex
void CacheHelper::replacePortsInFile(int hostPort1, int hostPort2,
                                     int hostPort3, int hostPort4, int locPort1,
                                     int locPort2, const std::string &inFile,
                                     const std::string &outFile) {
  std::ifstream in(inFile, std::ios::in | std::ios::binary);
  if (in) {
    std::string contents;
    contents.assign(std::istreambuf_iterator<char>(in),
                    std::istreambuf_iterator<char>());
    in.close();

    replaceInPlace(contents, "HOST_PORT1", std::to_string(hostPort1));
    replaceInPlace(contents, "HOST_PORT2", std::to_string(hostPort2));
    replaceInPlace(contents, "HOST_PORT3", std::to_string(hostPort3));
    replaceInPlace(contents, "HOST_PORT4", std::to_string(hostPort4));
    replaceInPlace(contents, "LOC_PORT1", std::to_string(locPort1));
    replaceInPlace(contents, "LOC_PORT2", std::to_string(locPort2));

    std::ofstream out(outFile, std::ios::out);
    out << contents;
    out.close();
  }
}

void CacheHelper::replaceInPlace(std::string &searchStr,
                                 const std::string &matchStr,
                                 const std::string &replaceStr) {
  size_t pos = 0;
  while ((pos = searchStr.find(matchStr, pos)) != std::string::npos) {
    searchStr.replace(pos, matchStr.length(), replaceStr);
    pos += replaceStr.length();
  }
}
#else
void CacheHelper::replacePortsInFile(int hostPort1, int hostPort2,
                                     int hostPort3, int hostPort4, int locPort1,
                                     int locPort2, const std::string &inFile,
                                     const std::string &outFile) {
  std::ifstream in(inFile, std::ios::in);
  if (in) {
    std::string contents;
    contents.assign(std::istreambuf_iterator<char>(in),
                    std::istreambuf_iterator<char>());
    in.close();

    contents = boost::regex_replace(contents, boost::regex("HOST_PORT1"),
                                    std::to_string(hostPort1));
    contents = boost::regex_replace(contents, boost::regex("HOST_PORT2"),
                                    std::to_string(hostPort2));
    contents = boost::regex_replace(contents, boost::regex("HOST_PORT3"),
                                    std::to_string(hostPort3));
    contents = boost::regex_replace(contents, boost::regex("HOST_PORT4"),
                                    std::to_string(hostPort4));
    contents = boost::regex_replace(contents, boost::regex("LOC_PORT1"),
                                    std::to_string(locPort1));
    contents = boost::regex_replace(contents, boost::regex("LOC_PORT2"),
                                    std::to_string(locPort2));

    std::ofstream out(outFile, std::ios::out);
    out << contents;
    out.close();
  }
}
#endif

std::string CacheHelper::createDuplicateXMLFile(const std::string &source) {
  return CacheHelper::createDuplicateXMLFile(
      source, CacheHelper::staticHostPort1, CacheHelper::staticHostPort2,
      CacheHelper::staticLocatorHostPort1, CacheHelper::staticLocatorHostPort2);
}

void CacheHelper::closeServer(int instance) {
  static const auto gfjavaenv = Utils::getEnv("GFJAVA");
  std::string currDir = boost::filesystem::current_path().string();

  ASSERT(!gfjavaenv.empty(),
         "Environment variable GFJAVA for java build directory is not set.");
  ASSERT(!currDir.empty(),
         "Current working directory could not be determined.");

  if (gfjavaenv.find(PATH_SEP) == std::string::npos) {
    return;
  }

  currDir += "/GFECS";
  switch (instance) {
    case 0:
      currDir += "0";
      break;
    case 1:
      currDir += std::to_string(CacheHelper::staticHostPort1);
      break;
    case 2:
      currDir += std::to_string(CacheHelper::staticHostPort2);
      break;
    case 3:
      currDir += std::to_string(CacheHelper::staticHostPort3);
      break;
    default: /* Support for any number of servers Max 10*/
      // ASSERT( ( instance <= 10 )," More than 10 servers not supported");
      // TODO: need to support more then three servers
      ASSERT((instance <= 4), " More than 4 servers not supported");
      currDir += std::to_string(CacheHelper::staticHostPort4);
      break;
  }

  try {
    GfshExecute gfsh;
    gfsh.stop().server().withDir(currDir).execute();
  } catch (const GfshExecuteException &) {
  }

  terminate_process_file(currDir + "/vf.gf.server.pid",
                         std::chrono::seconds(10));

  staticServerInstanceList.remove(instance);
}
// closing locator
void CacheHelper::closeLocator(int instance, bool) {
  static const auto gfjavaenv = Utils::getEnv("GFJAVA");
  static const auto testsrcenv = Utils::getEnv("TESTSRC");

  auto currDir = boost::filesystem::current_path().string();

  ASSERT(!gfjavaenv.empty(),
         "Environment variable GFJAVA for java build directory is not set.");
  ASSERT(!testsrcenv.empty(),
         "Environment variable TESTSRC for test source directory is not set.");

  std::string keystore = testsrcenv + "/keystore";
  if (gfjavaenv.find(PATH_SEP) == std::string::npos) {
    return;
  }

  currDir += PATH_SEP;
  currDir += "GFELOC";

  switch (instance) {
    case 1:
      currDir += std::to_string(CacheHelper::staticLocatorHostPort1);
      break;
    case 2:
      currDir += std::to_string(CacheHelper::staticLocatorHostPort2);
      break;
    case 3:
      currDir += std::to_string(CacheHelper::staticLocatorHostPort3);
      break;
    default: /* Support for any number of Locator Max 10*/
      // TODO://
      ASSERT((instance <= 3), " More than 3 servers not supported");
      currDir += std::to_string(instance);
      break;
  }

  try {
    GfshExecute gfsh;
    gfsh.stop().locator().withDir(currDir).execute();
  } catch (const GfshExecuteException &) {
  }

  terminate_process_file(currDir + "/vf.gf.locator.pid",
                         std::chrono::seconds(10));

  std::remove("test.geode.properties");
  staticLocatorInstanceList.remove(instance);
}

template <class Rep, class Period>
void CacheHelper::terminate_process_file(
    const std::string &pidFileName,
    const std::chrono::duration<Rep, Period> &duration) {
  auto timeout = std::chrono::system_clock::now() + duration;

  std::string pid;
  read_single_line(pidFileName, pid);
  if (pid.empty()) {
    return;
  }

  LOG("CacheHelper::terminate_process_file: process running. pidFileName=" +
      pidFileName + ", pid=" + pid);

  // Wait for process to terminate or timeout
  auto start = std::chrono::system_clock::now();
  while (std::chrono::system_clock::now() < timeout) {
    if (!file_exists(pidFileName)) {
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now() - start);
      LOG("CacheHelper::terminate_process_file: process exited. "
          "pidFileName=" +
          pidFileName + ", pid=" + pid +
          ", elapsed=" + std::to_string(elapsed.count()) + "ms");
      return;
    }
    std::this_thread::yield();
  }
  LOG("CacheHelper::terminate_process_file: timeout. pidFileName=" +
      pidFileName + ", pid=" + pid);

  // Didn't exit on its own, kill it.
  LOG("terminate_process: pid=" + pid);

  boost::process::pid_t id =
      static_cast<boost::process::pid_t>(std::stoul(pid));
  boost::process::child process{id};
  process.terminate();
}

bool CacheHelper::file_exists(const std::string &fileName) {
  std::ifstream file(fileName);
  return file.is_open();
}

void CacheHelper::read_single_line(const std::string &fileName,
                                   std::string &str) {
  std::ifstream f(fileName);
  std::getline(f, str);
}

void CacheHelper::cleanupTmpConfigFiles() {
  for (auto its = CacheHelper::staticConfigFileList.begin();
       its != CacheHelper::staticConfigFileList.end(); ++its) {
    try {
      std::remove(its->c_str());
    } catch (...) {
    }
  }
}

void CacheHelper::cleanupLocatorInstances() {
  CacheHelper::cleanupTmpConfigFiles();
  if (staticLocatorInstanceList.size() > 0) {
    while (staticLocatorInstanceList.size() > 0) {
      int instance = staticLocatorInstanceList.front();

      staticLocatorInstanceList.remove(instance);  // for safety
      closeLocator(instance);                      // this will also remove
    }
  }
}

// starting locator
void CacheHelper::initLocator(int instance, bool ssl, bool, int dsId,
                              int remoteLocator, bool untrustedCert,
                              bool useSecurityManager) {
  static const auto gfjavaenv = Utils::getEnv("GFJAVA");

  if (!isLocatorCleanupCallbackRegistered) {
    isLocatorCleanupCallbackRegistered = true;

    gClientCleanup.registerCallback(
        []() { CacheHelper::cleanupLocatorInstances(); });
  }

  std::string currDir = boost::filesystem::current_path().string();

  ASSERT(!gfjavaenv.empty(),
         "Environment variable GFJAVA for java build directory is not set.");

  if (gfjavaenv.find(PATH_SEP) == std::string::npos) {
    return;
  }

  int portnum = 0;
  std::string locDirname = "GFELOC";

  currDir += PATH_SEP;

  switch (instance) {
    case 1:
      portnum = CacheHelper::staticLocatorHostPort1;
      break;
    case 2:
      portnum = CacheHelper::staticLocatorHostPort2;
      break;
    default:
      portnum = CacheHelper::staticLocatorHostPort3;
      break;
  }

  locDirname += std::to_string(portnum);

  int jmxManagerPort = CacheHelper::staticJmxManagerPort;

  currDir += locDirname;
  boost::filesystem::create_directory(locDirname);

  std::string geodeFile = generateGeodeProperties(
      currDir, ssl, dsId, remoteLocator, untrustedCert, useSecurityManager);

  auto classpath = Utils::getEnv("GF_CLASSPATH");

  GfshExecute gfsh;
  auto locator = gfsh.start()
                     .locator()
                     .withName(locDirname)
                     .withPort(portnum)
                     .withDir(currDir)
                     .withClasspath(classpath)
                     .withHttpServicePort(0)
                     .withJmxManagerPort(jmxManagerPort)
                     .withMaxHeap("256m");
  if (useSecurityManager) {
    locator.withSecurityPropertiesFile(geodeFile);
  } else {
    locator.withPropertiesFile(geodeFile);
  }
  locator.execute();

  staticLocatorInstanceList.push_back(instance);
}

void CacheHelper::setJavaConnectionPoolSize(uint32_t size) {
  CacheHelper::getHelper()
      .getCache()
      ->getSystemProperties()
      .setjavaConnectionPoolSize(size);
}

bool CacheHelper::setSeed() {
  static const auto testnameenv = Utils::getEnv("TESTNAME");
  ASSERT(!testnameenv.empty(),
         "Environment variable TESTNAME for test name is not set.");

  int seed = std::hash<std::string>{}(testnameenv);
  std::cout << "seed for process " << seed << "\n";
  // The integration tests rely on the pseudo-random
  // number generator being seeded with a very particular
  // value specific to the test by way of the test name.
  // Whilst this approach is pessimal, it can not be
  // remedied as the test depend upon it.
  std::srand(seed);
  return true;
}

int CacheHelper::hashcode(char *str) {
  if (str == nullptr) {
    return 0;
  }
  int localHash = 0;

  int prime = 31;
  char *data = str;
  for (int i = 0; i < 50 && (data[i] != '\0'); i++) {
    localHash = prime * localHash + data[i];
  }
  if (localHash > 0) return localHash;
  return -1 * localHash;
}

int CacheHelper::getRandomNumber() {
  return (std::rand() % RANDOM_NUMBER_DIVIDER) + RANDOM_NUMBER_OFFSET;
}

int CacheHelper::getRandomAvailablePort() {
  using boost::asio::io_service;
  using boost::asio::ip::tcp;
  namespace bip = boost::asio::ip;

  io_service service;
  bip::tcp::acceptor acceptor(service, bip::tcp::v4());

  int result = 0;
  while (result == 0) {
    uint16_t port = getRandomNumber();
    bip::tcp::endpoint ep{bip::address_v4::loopback(), port};

    try {
      acceptor.bind(ep);
      acceptor.listen();
      result = port;
    } catch (boost::system::system_error &e) {
      std::clog << "Error: " << e.what() << std::endl;
    }
  }

  return result;
}

std::string CacheHelper::unitTestOutputFile() {
  static const auto testnameenv = Utils::getEnv("TESTNAME");

  ASSERT(!testnameenv.empty(),
         "Environment variable TESTNAME for test name is not set.");

  std::string outputFile = boost::filesystem::current_path().string();
  outputFile += PATH_SEP;
  outputFile += testnameenv;
  outputFile += ".log";

  return outputFile;
}

int CacheHelper::getNumLocatorListUpdates(const std::string &search) {
  std::string testFile = CacheHelper::unitTestOutputFile();

  std::ifstream file{testFile};
  ASSERT(!file.fail(), "Failed to open log file.");

  std::string line;
  int numMatched = 0;
  while (std::getline(file, line)) {
    if (line.find(search) != std::string::npos) {
      ++numMatched;
    }
  }

  return numMatched;
}

std::string CacheHelper::generateGeodeProperties(
    const std::string &path, const bool ssl, const int dsId,
    const int remoteLocator, const bool untrustedCert,
    const bool useSecurityManager) {
  static const auto testnameenv = Utils::getEnv("TESTNAME");

  ASSERT(!testnameenv.empty(),
         "Environment variable TESTNAME for test name is not set.");

  std::string keystore = testnameenv + "/keystore";

  std::string geodeFile = path;
  geodeFile += "/test.geode.properties";

  std::ofstream file{geodeFile};

  file << "locators=localhost[" << CacheHelper::staticLocatorHostPort1
       << "],localhost[" << CacheHelper::staticLocatorHostPort2
       << "],localhost[" << CacheHelper::staticLocatorHostPort3 << "]"
       << std::endl;

  file << "log-level=config" << std::endl;
  file << "mcast-port=0" << std::endl;
  file << "enable-network-partition-detection=false" << std::endl;

  if (useSecurityManager) {
    file << "security-manager=javaobject.SimpleSecurityManager" << std::endl;
  }

  std::string serverKeystore;
  std::string serverTruststore;
  std::string password;

  if (ssl) {
    if (untrustedCert) {
      serverKeystore += "untrusted_server_keystore.jks";
      serverTruststore += "untrusted_server_truststore.jks";
      password += "secret";
    } else {
      serverKeystore += "server_keystore_chained.jks";
      serverTruststore += "server_truststore_chained_root.jks";
      password += "apachegeode";
    }
    file << "jmx-manager-ssl-enabled=false" << std::endl;
    file << "cluster-ssl-enabled=true" << std::endl;
    file << "cluster-ssl-require-authentication=false" << std::endl;
    file << "cluster-ssl-ciphers=TLS_RSA_WITH_AES_128_CBC_SHA" << std::endl;
    file << "cluster-ssl-keystore-type=jks" << std::endl;
    file << "cluster-ssl-keystore=" + keystore + PATH_SEP + serverKeystore
         << std::endl;
    file << "cluster-ssl-keystore-password=" + password + "" << std::endl;
    file << "cluster-ssl-truststore=" + keystore + PATH_SEP +
                serverTruststore.c_str() + ""
         << std::endl;
    file << "cluster-ssl-truststore-password=" + password + "" << std::endl;
    file << "security-username=xxxx" << std::endl;
    file << "security-userPassword=yyyy " << std::endl;
  }

  file << "distributed-system-id=" << dsId << std::endl;

  if (remoteLocator != 0) {
    file << "remote-locators=localhost[" << remoteLocator << "]";
  }

  file.close();

  LOG(geodeFile);
  return geodeFile;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
