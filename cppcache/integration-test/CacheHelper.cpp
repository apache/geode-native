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
#include <fstream>
#include <regex>
#include <list>

#include <ace/OS.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Acceptor.h>

#include <geode/SystemProperties.hpp>
#include <geode/PoolManager.hpp>
#include <geode/internal/chrono/duration.hpp>

#include "CacheRegionHelper.hpp"
#include "DistributedSystemImpl.hpp"
#include "TimeBomb.hpp"
#include "Utils.hpp"
#include "CacheImpl.hpp"
#include "config.h"

#include "CacheHelper.hpp"
#define __DUNIT_NO_MAIN__
#include "fw_dunit.hpp"

#include <chrono>
#include <thread>

#ifndef ROOT_NAME
#define ROOT_NAME "Root"
#endif

#ifndef ROOT_SCOPE
#define ROOT_SCOPE LOCAL
#endif

#if defined(WIN32)
#define GFSH "gfsh.bat"
#define COPY_COMMAND "copy /y"
#define DELETE_COMMAND "del /f"
#define PATH_SEP "\\"
#else
#define GFSH "gfsh"
#define COPY_COMMAND "cp -f"
#define DELETE_COMMAND "rm -f"
#define PATH_SEP "/"
#endif

using namespace apache::geode::client;
using namespace apache::geode::internal::chrono::duration;

extern ClientCleanup gClientCleanup;

#define SEED 0
#define RANDOM_NUMBER_OFFSET 14000
#define RANDOM_NUMBER_DIVIDER 15000
std::shared_ptr<Cache> CacheHelper::getCache() { return cachePtr; }

CacheHelper& CacheHelper::getHelper() {
  if (singleton == nullptr) {
    singleton = new CacheHelper();
  }
  return *singleton;
}

CacheHelper::CacheHelper(const char* member_id,
                         const std::shared_ptr<Properties>& configPtr,
                         const bool noRootRegion) {
  auto pp = configPtr;
  if (pp == nullptr) {
    pp = Properties::create();
  }

  auto cacheFactory = CacheFactory(pp);
  cachePtr = std::make_shared<Cache>(cacheFactory.create());

  m_doDisconnect = false;

  if (noRootRegion) return;

  try {
    auto regionFactory =
        cachePtr->createRegionFactory(RegionShortcut::CACHING_PROXY);
    rootRegionPtr = regionFactory.create(ROOT_NAME);
  } catch (const RegionExistsException&) {
    rootRegionPtr = cachePtr->getRegion(ROOT_NAME);
  }

  showRegionAttributes(*rootRegionPtr->getAttributes());
}

/** rootRegionPtr will still be null... */
CacheHelper::CacheHelper(const char* member_id, const char* cachexml,
                         const std::shared_ptr<Properties>& configPtr) {
  auto pp = configPtr;
  if (pp == nullptr) {
    pp = Properties::create();
  }
  if (cachexml != nullptr) {
    std::string tmpXmlFile(cachexml);
    std::string newFile;
    CacheHelper::createDuplicateXMLFile(newFile, tmpXmlFile);
    pp->insert("cache-xml-file", newFile.c_str());
  }
  auto cacheFactory = CacheFactory(pp);
  cachePtr = std::make_shared<Cache>(cacheFactory.create());

  m_doDisconnect = false;
}

CacheHelper::CacheHelper(const std::shared_ptr<Properties>& configPtr,
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

  m_doDisconnect = false;

  if (noRootRegion) return;

  try {
    auto regionFactory =
        cachePtr->createRegionFactory(RegionShortcut::CACHING_PROXY);
    rootRegionPtr = regionFactory.create(ROOT_NAME);
  } catch (const RegionExistsException&) {
    rootRegionPtr = cachePtr->getRegion(ROOT_NAME);
  }

  showRegionAttributes(*rootRegionPtr->getAttributes());
}

CacheHelper::CacheHelper(const bool isThinclient,
                         const std::shared_ptr<AuthInitialize>& authInitialize,
                         const std::shared_ptr<Properties>& configPtr) {
  auto pp = configPtr;
  if (pp == nullptr) {
    pp = Properties::create();
  }
  try {
    LOG(" in cachehelper before createCacheFactory");
    auto cacheFactory = CacheFactory(pp).setAuthInitialize(authInitialize);
    cachePtr = std::make_shared<Cache>(cacheFactory.create());
    m_doDisconnect = false;
  } catch (const Exception& excp) {
    LOG("Geode exception while creating cache, logged in following line");
    LOG(excp.what());
  } catch (...) {
    LOG("Throwing exception while creating cache....");
  }
}

CacheHelper::CacheHelper(const bool isThinclient,
                         const std::shared_ptr<Properties>& configPtr,
                         const bool noRootRegion) {
  auto pp = configPtr;
  if (pp == nullptr) {
    pp = Properties::create();
  }
  try {
    LOG(" in cachehelper before createCacheFactory");
    auto cacheFactory = CacheFactory(pp);
    cachePtr = std::make_shared<Cache>(cacheFactory.create());
    m_doDisconnect = false;
  } catch (const Exception& excp) {
    LOG("Geode exception while creating cache, logged in following line");
    LOG(excp.what());
  } catch (...) {
    LOG("Throwing exception while creating cache....");
  }
}

CacheHelper::CacheHelper(const bool isThinclient, bool pdxIgnoreUnreadFields,
                         bool pdxReadSerialized,
                         const std::shared_ptr<Properties>& configPtr,
                         const bool noRootRegion) {
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
    m_doDisconnect = false;
  } catch (const Exception& excp) {
    LOG("Geode exception while creating cache, logged in following line");
    LOG(excp.what());
  } catch (...) {
    LOG("Throwing exception while creating cache....");
  }
}

CacheHelper::CacheHelper(const bool isthinClient, const char* poolName,
                         const char* locators, const char* serverGroup,
                         const std::shared_ptr<Properties>& configPtr,
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
    printf(" Setting pr-single-hop to prSingleHop = %d ", prSingleHop);
    printf("Setting threadLocal to %d ", threadLocal);
    if (locators) {
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
    printf("Setting connections to %d ", connections);
    if (connections >= 0) {
      poolFactory.setMinConnections(connections);
      poolFactory.setMaxConnections(connections);
    }
    if (subscriptionAckInterval != -1) {
      poolFactory.setSubscriptionAckInterval(
          std::chrono::milliseconds(subscriptionAckInterval));
    }

    poolFactory.create(poolName);

  } catch (const Exception& excp) {
    LOG("Geode exception while creating cache, logged in following line");
    LOG(excp.what());
  } catch (...) {
    LOG("Throwing exception while creating cache....");
  }
}

CacheHelper::CacheHelper(const int redundancyLevel,
                         const std::shared_ptr<Properties>& configPtr) {
  auto pp = configPtr;
  if (pp == nullptr) {
    pp = Properties::create();
  }

  auto cacheFac = CacheFactory(pp);
  cachePtr = std::make_shared<Cache>(cacheFac.create());
  m_doDisconnect = false;
}

CacheHelper::~CacheHelper() {
  // CacheHelper::cleanupTmpConfigFiles();
  disconnect();
}

void CacheHelper::closePool(const char* poolName, bool keepAlive) {
  auto pool = getCache()->getPoolManager().find(poolName);
  pool->destroy(keepAlive);
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
  } catch (Exception& ex) {
    LOG("Exception thrown while closing cache: ");
    LOG(ex.what());
  } catch (...) {
    LOG("exception throw while closing cache");
  }

  LOG("Closed cache.");
  try {
    if (m_doDisconnect) {
      LOG("Disconnecting...");
      cachePtr->getDistributedSystem().disconnect();
      LOG("Finished disconnect.");
    }
  } catch (...) {
    LOG("Throwing exception while disconnecting....");
  }
  cachePtr = nullptr;
  singleton = nullptr;
  LOG("Finished cleanup after CacheHelper.");
}

void CacheHelper::createPlainRegion(const char* regionName,
                                    std::shared_ptr<Region>& regionPtr) {
  createPlainRegion(regionName, regionPtr, 10);
}

void CacheHelper::createPlainRegion(const char* regionName,
                                    std::shared_ptr<Region>& regionPtr,
                                    uint32_t size) {
  std::shared_ptr<RegionAttributes> regAttrs;
  AttributesFactory attrFactory;
  // set lru attributes...
  attrFactory.setLruEntriesLimit(0);     // no limit.
  attrFactory.setInitialCapacity(size);  // no limit.
  // then...
  regAttrs = attrFactory.createRegionAttributes();
  showRegionAttributes(*regAttrs);
  // This is using subregions (deprecated) so not placing the new cache API here
  regionPtr = rootRegionPtr->createSubregion(regionName, regAttrs);
  ASSERT(regionPtr != nullptr, "failed to create region.");
}

void CacheHelper::createLRURegion(const char* regionName,
                                  std::shared_ptr<Region>& regionPtr) {
  createLRURegion(regionName, regionPtr, 10);
}
void CacheHelper::createLRURegion(const char* regionName,
                                  std::shared_ptr<Region>& regionPtr,
                                  uint32_t size) {
  std::shared_ptr<RegionAttributes> regAttrs;
  AttributesFactory attrFactory;
  // set lru attributes...
  attrFactory.setLruEntriesLimit(size);
  attrFactory.setInitialCapacity(size);
  // then...
  regAttrs = attrFactory.createRegionAttributes();
  showRegionAttributes(*regAttrs);
  // This is using subregions (deprecated) so not placing the new cache API here
  regionPtr = rootRegionPtr->createSubregion(regionName, regAttrs);
  ASSERT(regionPtr != nullptr, "failed to create region.");
}

void CacheHelper::createDistRegion(const char* regionName,
                                   std::shared_ptr<Region>& regionPtr) {
  createDistRegion(regionName, regionPtr, 10);
}

void CacheHelper::createDistRegion(const char* regionName,
                                   std::shared_ptr<Region>& regionPtr,
                                   uint32_t size) {
  std::shared_ptr<RegionAttributes> regAttrs;
  AttributesFactory attrFactory;
  // set lru attributes...
  attrFactory.setLruEntriesLimit(0);     // no limit.
  attrFactory.setInitialCapacity(size);  // no limit.
  // then...
  regAttrs = attrFactory.createRegionAttributes();
  showRegionAttributes(*regAttrs);
  // This is using subregions (deprecated) so not placing the new cache API here
  regionPtr = rootRegionPtr->createSubregion(regionName, regAttrs);
  ASSERT(regionPtr != nullptr, "failed to create region.");
}
std::shared_ptr<Region> CacheHelper::getRegion(const char* name) {
  return cachePtr->getRegion(name);
}
std::shared_ptr<Region> CacheHelper::createRegion(
    const char* name, bool ack, bool caching,
    const std::shared_ptr<CacheListener>& listener,
    bool clientNotificationEnabled, bool scopeLocal,
    bool concurrencyCheckEnabled, int32_t tombstonetimeout) {
  AttributesFactory af;
  af.setCachingEnabled(caching);
  if (listener != nullptr) {
    af.setCacheListener(listener);
  }
  if (concurrencyCheckEnabled) {
    af.setConcurrencyChecksEnabled(concurrencyCheckEnabled);
  }

  std::shared_ptr<RegionAttributes> rattrsPtr = af.createRegionAttributes();

  CacheImpl* cacheImpl = CacheRegionHelper::getCacheImpl(cachePtr.get());
  std::shared_ptr<Region> regionPtr;
  cacheImpl->createRegion(name, rattrsPtr, regionPtr);
  return regionPtr;
}
std::shared_ptr<Region> CacheHelper::createRegion(
    const char* name, bool ack, bool caching, const std::chrono::seconds& ettl,
    const std::chrono::seconds& eit, const std::chrono::seconds& rttl,
    const std::chrono::seconds& rit, int lel, ExpirationAction action,
    const char* endpoints, bool clientNotificationEnabled) {
  AttributesFactory af;
  af.setCachingEnabled(caching);
  af.setLruEntriesLimit(lel);
  af.setEntryIdleTimeout(action, eit);
  af.setEntryTimeToLive(action, ettl);
  af.setRegionIdleTimeout(action, rit);
  af.setRegionTimeToLive(action, rttl);

  std::shared_ptr<RegionAttributes> rattrsPtr = af.createRegionAttributes();

  CacheImpl* cacheImpl = CacheRegionHelper::getCacheImpl(cachePtr.get());
  std::shared_ptr<Region> regionPtr;
  cacheImpl->createRegion(name, rattrsPtr, regionPtr);
  return regionPtr;
}
std::shared_ptr<Pool> CacheHelper::createPool(
    const char* poolName, const char* locators, const char* serverGroup,
    int redundancy, bool clientNotification,
    std::chrono::milliseconds subscriptionAckInterval, int connections,
    int loadConditioningInterval, bool isMultiuserMode) {
  // printf(" in createPool isMultiuserMode = %d \n", isMultiuserMode);
  auto poolFac = getCache()->getPoolManager().createFactory();

  addServerLocatorEPs(locators, poolFac);
  if (serverGroup) {
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
    const char* poolName, const char* locators, const char* serverGroup,
    const char* servers, int redundancy, bool clientNotification,
    int subscriptionAckInterval, int connections) {
  auto poolFac = getCache()->getPoolManager().createFactory();

  if (servers != 0)  // with explicit server list
  {
    addServerLocatorEPs(servers, poolFac, false);
    // do region creation with end
  } else if (locators != 0)  // with locator
  {
    addServerLocatorEPs(locators, poolFac);
    if (serverGroup) {
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

void CacheHelper::logPoolAttributes(std::shared_ptr<Pool>& pool) {
  using namespace apache::geode::internal::chrono::duration;
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
    const char* name, const char* locators, bool clientNotificationEnabled,
    int subscriptionRedundancy,
    std::chrono::milliseconds subscriptionAckInterval, int connections,
    bool isMultiuserMode, const char* serverGroup) {
  LOG("createPool() entered.");
  printf(" in createPoolWithLocators isMultiuserMode = %d\n", isMultiuserMode);
  auto poolPtr = createPool(name, locators, serverGroup, subscriptionRedundancy,
                            clientNotificationEnabled, subscriptionAckInterval,
                            connections, -1, isMultiuserMode);
  ASSERT(poolPtr != nullptr, "Failed to create pool.");
  logPoolAttributes(poolPtr);
  LOG("Pool created.");
}
std::shared_ptr<Region> CacheHelper::createRegionAndAttachPool(
    const char* name, bool ack, const char* poolName, bool caching,
    const std::chrono::seconds& ettl, const std::chrono::seconds& eit,
    const std::chrono::seconds& rttl, const std::chrono::seconds& rit, int lel,
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
  if (poolName != nullptr) {
    regionFactory.setPoolName(poolName);
  }
  return regionFactory.create(name);
}
std::shared_ptr<Region> CacheHelper::createRegionAndAttachPool2(
    const char* name, bool ack, const char* poolName,
    const std::shared_ptr<PartitionResolver>& aResolver, bool caching,
    const std::chrono::seconds& ettl, const std::chrono::seconds& eit,
    const std::chrono::seconds& rttl, const std::chrono::seconds& rit, int lel,
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

void CacheHelper::addServerLocatorEPs(const char* epList, PoolFactory& pf,
                                      bool poolLocators) {
  std::unordered_set<std::string> endpointNames;
  Utils::parseEndpointNamesString(epList, endpointNames);
  for (const auto& endpointName : endpointNames) {
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

// void CacheHelper::addServerLocatorEPs(const char* epList,
//                                      std::shared_ptr<CacheFactory> cacheFac,
//                                      bool poolLocators) {
//  std::unordered_set<std::string> endpointNames;
//  Utils::parseEndpointNamesString(epList, endpointNames);
//  for (std::unordered_set<std::string>::iterator iter = endpointNames.begin();
//       iter != endpointNames.end(); ++iter) {
//    size_t position = (*iter).find_first_of(":");
//    if (position != std::string::npos) {
//      std::string hostname = (*iter).substr(0, position);
//      int portnumber = atoi(((*iter).substr(position + 1)).c_str());
//      if (poolLocators) {
//        getCache()->getPoolFactory()->addLocator(hostname.c_str(),
//        portnumber);
//      } else {
//        printf("ankur Server: %d", portnumber);
//        getCache()->getPoolFactory()->addServer(hostname.c_str(), portnumber);
//      }
//    }
//  }
//}
std::shared_ptr<Region> CacheHelper::createPooledRegion(
    const char* name, bool ack, const char* locators, const char* poolName,
    bool caching, bool clientNotificationEnabled,
    const std::chrono::seconds& ettl, const std::chrono::seconds& eit,
    const std::chrono::seconds& rttl, const std::chrono::seconds& rit, int lel,
    const std::shared_ptr<CacheListener>& cacheListener,
    ExpirationAction action) {
  auto poolFac = getCache()->getPoolManager().createFactory();
  poolFac.setSubscriptionEnabled(clientNotificationEnabled);

  if (locators) {
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
    const char* name, bool ack, const char* locators, const char* poolName,
    bool caching, bool clientNotificationEnabled, bool concurrencyCheckEnabled,
    const std::chrono::seconds& ettl, const std::chrono::seconds& eit,
    const std::chrono::seconds& rttl, const std::chrono::seconds& rit, int lel,
    const std::shared_ptr<CacheListener>& cacheListener,
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
    const char* name, bool caching, bool clientNotificationEnabled,
    const std::chrono::seconds& ettl, const std::chrono::seconds& eit,
    const std::chrono::seconds& rttl, const std::chrono::seconds& rit, int lel,
    ExpirationAction action) {
  AttributesFactory af;
  af.setCachingEnabled(caching);
  af.setLruEntriesLimit(lel);
  af.setEntryIdleTimeout(action, eit);
  af.setEntryTimeToLive(action, ettl);
  af.setRegionIdleTimeout(action, rit);
  af.setRegionTimeToLive(action, rttl);
  af.setCloningEnabled(true);
  if (lel > 0) {
    af.setDiskPolicy(DiskPolicyType::OVERFLOWS);
    auto sqLiteProps = Properties::create();
    sqLiteProps->insert("PageSize", "65536");
    sqLiteProps->insert("MaxPageCount", "1073741823");
    std::string sqlite_dir =
        "SqLiteRegionData" +
        std::to_string(static_cast<long long int>(ACE_OS::getpid()));
    sqLiteProps->insert("PersistenceDirectory", sqlite_dir.c_str());
    af.setPersistenceManager("SqLiteImpl", "createSqLiteInstance", sqLiteProps);
  }

  std::shared_ptr<RegionAttributes> rattrsPtr = af.createRegionAttributes();
  CacheImpl* cacheImpl = CacheRegionHelper::getCacheImpl(cachePtr.get());
  std::shared_ptr<Region> regionPtr;
  cacheImpl->createRegion(name, rattrsPtr, regionPtr);
  return regionPtr;
}
std::shared_ptr<Region> CacheHelper::createPooledRegionDiscOverFlow(
    const char* name, bool ack, const char* locators, const char* poolName,
    bool caching, bool clientNotificationEnabled,
    const std::chrono::seconds& ettl, const std::chrono::seconds& eit,
    const std::chrono::seconds& rttl, const std::chrono::seconds& rit, int lel,
    const std::shared_ptr<CacheListener>& cacheListener,
    ExpirationAction action) {
  auto poolFac = getCache()->getPoolManager().createFactory();
  poolFac.setSubscriptionEnabled(clientNotificationEnabled);

  if (locators)  // with locator
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
        "SqLiteRegionData" +
        std::to_string(static_cast<long long int>(ACE_OS::getpid()));
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
    const char* name, bool ack, const char* locators, const char* poolName,
    bool caching, bool clientNotificationEnabled,
    const std::chrono::seconds& ettl, const std::chrono::seconds& eit,
    const std::chrono::seconds& rttl, const std::chrono::seconds& rit, int lel,
    const std::shared_ptr<CacheListener>& cacheListener,
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
    const char* name, bool ack, const char* locators, const char* poolName,
    bool caching, bool clientNotificationEnabled,
    const std::chrono::seconds& ettl, const std::chrono::seconds& eit,
    const std::chrono::seconds& rttl, const std::chrono::seconds& rit, int lel,
    const std::shared_ptr<CacheListener>& cacheListener,
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
    std::shared_ptr<Region>& parent, const char* name, bool ack, bool caching,
    const std::shared_ptr<CacheListener>& listener) {
  AttributesFactory af;
  af.setCachingEnabled(caching);
  if (listener != nullptr) {
    af.setCacheListener(listener);
  }
  std::shared_ptr<RegionAttributes> rattrsPtr = af.createRegionAttributes();

  return parent->createSubregion(name, rattrsPtr);
}
std::shared_ptr<CacheableString> CacheHelper::createCacheable(
    const char* value) {
  return CacheableString::create(value);
}

void CacheHelper::showKeys(
    std::vector<std::shared_ptr<CacheableKey>>& vecKeys) {
  fprintf(stdout, "vecKeys.size() = %zd\n", vecKeys.size());
  for (size_t i = 0; i < vecKeys.size(); i++) {
    fprintf(stdout, "key[%zd] - %s\n", i, vecKeys.at(i)->toString().c_str());
  }
  fflush(stdout);
}

void CacheHelper::showRegionAttributes(RegionAttributes& attributes) {
  printf("caching=%s\n", attributes.getCachingEnabled() ? "true" : "false");
  printf("Entry Time To Live = %s\n",
         to_string(attributes.getEntryTimeToLive()).c_str());
  printf("Entry Idle Timeout = %s\n",
         to_string(attributes.getEntryIdleTimeout()).c_str());
  printf("Region Time To Live = %s\n",
         to_string(attributes.getRegionTimeToLive()).c_str());
  printf("Region Idle Timeout = %s\n",
         to_string(attributes.getRegionIdleTimeout()).c_str());
  printf("Initial Capacity = %d\n", attributes.getInitialCapacity());
  printf("Load Factor = %f\n", attributes.getLoadFactor());
  printf("End Points = %s\n", attributes.getEndpoints().c_str());
}
std::shared_ptr<QueryService> CacheHelper::getQueryService() {
  return cachePtr->getQueryService();
}

const char* CacheHelper::getTcrEndpoints(bool& isLocalServer,
                                         int numberOfServers) {
  static char* gfjavaenv = ACE_OS::getenv("GFJAVA");
  std::string gfendpoints;
  static bool gflocalserver = false;
  char tmp[100];

  if (gfendpoints.empty()) {
    ASSERT(gfjavaenv != nullptr,
           "Environment variable GFJAVA for java build directory is not set.");
    if ((ACE_OS::strchr(gfjavaenv, '\\') != nullptr) ||
        (ACE_OS::strchr(gfjavaenv, '/') != nullptr)) {
      gflocalserver = true;
      /* Support for multiple servers Max = 10*/
      switch (numberOfServers) {
        case 1:
          // gfendpoints = "localhost:24680";
          {
            gfendpoints = "localhost:";
            sprintf(tmp, "%d", CacheHelper::staticHostPort1);
            gfendpoints += tmp;
          }
          break;
        case 2:
          // gfendpoints = "localhost:24680,localhost:24681";
          {
            gfendpoints = "localhost:";
            sprintf(tmp, "%d", CacheHelper::staticHostPort1);
            gfendpoints += tmp;
            gfendpoints += ",localhost:";
            sprintf(tmp, "%d", CacheHelper::staticHostPort2);
            gfendpoints += tmp;
          }
          break;
        case 3:
          // gfendpoints = "localhost:24680,localhost:24681,localhost:24682";
          {
            gfendpoints = "localhost:";
            sprintf(tmp, "%d", CacheHelper::staticHostPort1);
            gfendpoints += tmp;
            gfendpoints += ",localhost:";
            sprintf(tmp, "%d", CacheHelper::staticHostPort2);
            gfendpoints += tmp;
            gfendpoints += ",localhost:";
            sprintf(tmp, "%d", CacheHelper::staticHostPort3);
            gfendpoints += tmp;
          }
          break;
        default:
          // ASSERT( ( numberOfServers <= 10 )," More than 10 servers not
          // supported");
          // TODO: need to support more servers, need to generate random ports
          // here
          ASSERT((numberOfServers <= 4), " More than 4 servers not supported");
          gfendpoints = "localhost:";
          sprintf(tmp, "%d", CacheHelper::staticHostPort1);
          gfendpoints += tmp;
          gfendpoints += ",localhost:";
          sprintf(tmp, "%d", CacheHelper::staticHostPort2);
          gfendpoints += tmp;
          gfendpoints += ",localhost:";
          sprintf(tmp, "%d", CacheHelper::staticHostPort3);
          gfendpoints += tmp;
          gfendpoints += ",localhost:";
          sprintf(tmp, "%d", CacheHelper::staticHostPort4);
          gfendpoints += tmp;
          /*gfendpoints = "localhost:24680";
          char temp[8];
          for(int i =1; i <= numberOfServers - 1; i++) {
           gfendpoints += ",localhost:2468";
           gfendpoints += ACE_OS::itoa(i,temp,10);
          }*/
          break;
      }
    } else {
      gfendpoints = gfjavaenv;
    }
  }
  isLocalServer = gflocalserver;
  printf("getHostPort :: %s \n", gfendpoints.c_str());
  return (new std::string(gfendpoints.c_str()))->c_str();
}

const char* CacheHelper::getstaticLocatorHostPort1() {
  return getLocatorHostPort(staticLocatorHostPort1);
}

const char* CacheHelper::getstaticLocatorHostPort2() {
  return getLocatorHostPort(staticLocatorHostPort2);
}

const char* CacheHelper::getLocatorHostPort(int locPort) {
  char tmp[128];
  std::string gfendpoints;
  gfendpoints = "localhost:";
  sprintf(tmp, "%d", locPort);
  gfendpoints += tmp;
  return (new std::string(gfendpoints.c_str()))->c_str();
  ;
}

const char* CacheHelper::getTcrEndpoints2(bool& isLocalServer,
                                          int numberOfServers) {
  static char* gfjavaenv = ACE_OS::getenv("GFJAVA");
  std::string gfendpoints;
  static bool gflocalserver = false;
  char tmp[128];

  if (gfendpoints.empty()) {
    if ((ACE_OS::strchr(gfjavaenv, '\\') != nullptr) ||
        (ACE_OS::strchr(gfjavaenv, '/') != nullptr)) {
      gflocalserver = true;
      /* Support for multiple servers Max = 10*/
      switch (numberOfServers) {
        case 1:
          // gfendpoints = "localhost:24680";
          {
            gfendpoints = "localhost:";
            sprintf(tmp, "%d", CacheHelper::staticHostPort1);
            gfendpoints += tmp;
          }
          break;
        case 2:
          // gfendpoints = "localhost:24680,localhost:24681";
          {
            gfendpoints = "localhost:";
            sprintf(tmp, "%d", CacheHelper::staticHostPort1);
            gfendpoints += tmp;
            gfendpoints += ",localhost:";
            sprintf(tmp, "%d", CacheHelper::staticHostPort2);
            gfendpoints += tmp;
          }
          break;
        case 3:
          // gfendpoints = "localhost:24680,localhost:24681,localhost:24682";
          {
            gfendpoints = "localhost:";
            sprintf(tmp, "%d", CacheHelper::staticHostPort1);
            gfendpoints += tmp;
            gfendpoints += ",localhost:";
            sprintf(tmp, "%d", CacheHelper::staticHostPort2);
            gfendpoints += tmp;
            gfendpoints += ",localhost:";
            sprintf(tmp, "%d", CacheHelper::staticHostPort3);
            gfendpoints += tmp;
          }
          break;
        case 4:
          // gfendpoints =
          // "localhost:24680,localhost:24681,localhost:24682,localhost:24683";
          {
            gfendpoints = "localhost:";
            sprintf(tmp, "%d", CacheHelper::staticHostPort1);
            gfendpoints += tmp;
            gfendpoints += ",localhost:";
            sprintf(tmp, "%d", CacheHelper::staticHostPort2);
            gfendpoints += tmp;
            gfendpoints += ",localhost:";
            sprintf(tmp, "%d", CacheHelper::staticHostPort3);
            gfendpoints += tmp;
            gfendpoints += ",localhost:";
            sprintf(tmp, "%d", CacheHelper::staticHostPort4);
            gfendpoints += tmp;
          }
          break;
        default:
          ASSERT((numberOfServers <= 10),
                 " More than 10 servers not supported");
          gfendpoints = "localhost:24680";
          char temp[8];
          for (int i = 1; i <= numberOfServers - 1; i++) {
            gfendpoints += ",localhost:2468";
            gfendpoints += ACE_OS::itoa(i, temp, 10);
          }
          break;
      }
    } else {
      gfendpoints = gfjavaenv;
    }
  }
  ASSERT(gfjavaenv != nullptr,
         "Environment variable GFJAVA for java build directory is not set.");
  isLocalServer = gflocalserver;
  return (new std::string(gfendpoints.c_str()))->c_str();
}

const char* CacheHelper::getLocatorHostPort(bool& isLocator,
                                            bool& isLocalServer,
                                            int numberOfLocators) {
  static char* gfjavaenv = ACE_OS::getenv("GFJAVA");
  static std::string gflchostport;
  static bool gflocator = false;
  static bool gflocalserver = false;
  char tmp[100];

  if (gflchostport.empty()) {
    if ((ACE_OS::strchr(gfjavaenv, '\\') != nullptr) ||
        (ACE_OS::strchr(gfjavaenv, '/') != nullptr)) {
      gflocator = true;
      gflocalserver = true;
      switch (numberOfLocators) {
        case 1:
          // gflchostport = "localhost:34756";
          {
            gflchostport = "localhost:";
            sprintf(tmp, "%d", CacheHelper::staticLocatorHostPort1);
            gflchostport += tmp;
          }
          break;
        case 2:
          // gflchostport = "localhost:34756,localhost:34757";
          {
            gflchostport = "localhost:";
            sprintf(tmp, "%d", CacheHelper::staticLocatorHostPort1);
            gflchostport += tmp;
            sprintf(tmp, "%d", CacheHelper::staticLocatorHostPort2);
            gflchostport += ",localhost:";
            gflchostport += tmp;
          }
          break;
        default:
          // gflchostport = "localhost:34756,localhost:34757,localhost:34758";
          {
            gflchostport = "localhost:";
            sprintf(tmp, "%d", CacheHelper::staticLocatorHostPort1);
            gflchostport += tmp;
            sprintf(tmp, "%d", CacheHelper::staticLocatorHostPort2);
            gflchostport += ",localhost:";
            gflchostport += tmp;
            sprintf(tmp, "%d", CacheHelper::staticLocatorHostPort3);
            gflchostport += ",localhost:";
            gflchostport += tmp;
          }
          break;
      }
    } else {
      gflchostport = "";
    }
  }
  ASSERT(gfjavaenv != nullptr,
         "Environment variable GFJAVA for java build directory is not set.");
  isLocator = gflocator;
  isLocalServer = gflocalserver;
  printf("getLocatorHostPort  :: %s  \n", gflchostport.c_str());
  return gflchostport.c_str();
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
void CacheHelper::initServer(int instance, const char* xml,
                             const char* locHostport, const char* authParam,
                             bool ssl, bool enableDelta, bool multiDS,
                             bool testServerGC, bool untrustedCert,
                             bool useSecurityManager) {
  if (!isServerCleanupCallbackRegistered &&
      gClientCleanup.registerCallback(&CacheHelper::cleanupServerInstances)) {
    isServerCleanupCallbackRegistered = true;
    printf("TimeBomb registered server cleanupcallback \n");
  }
  printf("Inside initServer added\n");
  if (authParam != nullptr) {
    printf("Inside initServer with authParam = %s\n", authParam);
  } else {
    printf("Inside initServer with authParam as nullptr\n");
    authParam = "";
  }
  static const char* gfjavaenv = ACE_OS::getenv("GFJAVA");
  static const char* gfLogLevel = ACE_OS::getenv("GFE_LOGLEVEL");
  static const char* gfSecLogLevel = ACE_OS::getenv("GFE_SECLOGLEVEL");
  static const char* path = ACE_OS::getenv("TESTSRC");
  static const char* mcastPort = ACE_OS::getenv("MCAST_PORT");
  static const char* mcastAddr = ACE_OS::getenv("MCAST_ADDR");
  static char* classpath = ACE_OS::getenv("GF_CLASSPATH");

  char cmd[2048];
  char tmp[128];
  char currWDPath[2048];
  int portNum;
  std::string currDir = ACE_OS::getcwd(currWDPath, 2048);

  ASSERT(gfjavaenv != nullptr,
         "Environment variable GFJAVA for java build directory is not set.");
  ASSERT(path != nullptr,
         "Environment variable TESTSRC for test source directory is not set.");
  ASSERT(mcastPort != nullptr,
         "Environment variable MCAST_PORT for multicast port is not set.");
  ASSERT(mcastAddr != nullptr,
         "Environment variable MCAST_ADDR for multicast address is not set.");
  ASSERT(!currDir.empty(),
         "Current working directory could not be determined.");
  if (gfLogLevel == nullptr || gfLogLevel[0] == '\0') {
    gfLogLevel = "config";
  }
  if (gfSecLogLevel == nullptr || gfSecLogLevel[0] == '\0') {
    gfSecLogLevel = "config";
  }

  if ((ACE_OS::strchr(gfjavaenv, '\\') == nullptr) &&
      (ACE_OS::strchr(gfjavaenv, '/') == nullptr)) {
    return;
  }

  std::string xmlFile = "";
  std::string sname = "GFECS";
  currDir += PATH_SEP;

  switch (instance) {
    case 0:
      // note: this need to take for multiple tests run
      xmlFile += "cacheserver.xml";
      sname += "0";
      if (multiDS) {
        mcastPort = "5431";
        mcastAddr = "224.10.11.";
      }
      break;
    case 1:
      xmlFile += "cacheserver.xml";
      sprintf(tmp, "%d", CacheHelper::staticHostPort1);
      sname += tmp;  // sname += "1";
      portNum = CacheHelper::staticHostPort1;

      if (multiDS) {
        mcastPort = "5431";
        mcastAddr = "224.10.11.";
      }
      break;
    case 2:
      xmlFile += "cacheserver2.xml";
      sprintf(tmp, "%d", CacheHelper::staticHostPort2);
      sname += tmp;  // sname += "3";
      portNum = CacheHelper::staticHostPort2;
      // sname += "2";
      if (multiDS) {
        mcastPort = "5431";
        mcastAddr = "224.10.11.";
      }
      break;
    case 3:
      xmlFile += "cacheserver3.xml";
      sprintf(tmp, "%d", CacheHelper::staticHostPort3);
      sname += tmp;  // sname += "3";
      portNum = CacheHelper::staticHostPort3;
      // sname += "3";
      if (multiDS) {
        mcastPort = "5433";
        mcastAddr = "224.10.11.";
      }
      break;
    case 4:
      xmlFile += "cacheserver4.xml";
      // sname += "4";
      sprintf(tmp, "%d", CacheHelper::staticHostPort4);
      sname += tmp;  // sname += "3";
      portNum = CacheHelper::staticHostPort4;
      if (multiDS) {
        mcastPort = "5433";
        mcastAddr = "224.10.11.";
      }
      break;
    default: /* Support for any number of servers Max 10*/
      ASSERT((instance <= 10), " More than 10 servers not supported");
      ASSERT(xml != nullptr,
             "xml == nullptr : For server instance > 3 xml file is must");
      char temp[8];
      portNum = CacheHelper::staticHostPort4;
      sname += ACE_OS::itoa(CacheHelper::staticHostPort4, temp, 10);
      break;
  }

  currDir += sname;

  if (xml != nullptr) {
    xmlFile = xml;
  }

  std::string xmlFile_new;
  printf(" xml file name = %s \n", xmlFile.c_str());
  CacheHelper::createDuplicateXMLFile(xmlFile_new, xmlFile);

  xmlFile = xmlFile_new;

  printf("  creating dir = %s \n", sname.c_str());
  ACE_OS::mkdir(sname.c_str());

  sprintf(cmd, "%s/bin/%s stop server --dir=%s 2>&1", gfjavaenv, GFSH,
          currDir.c_str());

  LOG(cmd);
  ACE_OS::system(cmd);
  std::string deltaProperty = "";
  if (!enableDelta) {
    deltaProperty = "delta-propagation=false";
  }
  long defaultTombstone_timeout = 600000;
  long defaultTombstone_gc_threshold = 100000;
  long userTombstone_timeout = 1000;
  long userTombstone_gc_threshold = 10;
  if (testServerGC) {
    ACE_OS::mkdir("backupDirectory1");
    ACE_OS::mkdir("backupDirectory2");
    ACE_OS::mkdir("backupDirectory3");
    ACE_OS::mkdir("backupDirectory4");
  }

  if (locHostport != nullptr) {  // check number of locator host port.
    std::string geodeProperties = generateGeodeProperties(
        currDir, ssl, -1, 0, untrustedCert, useSecurityManager);

    sprintf(
        cmd,
        "%s/bin/%s start server --classpath=%s --name=%s "
        "--cache-xml-file=%s %s --dir=%s --server-port=%d --log-level=%s "
        "--properties-file=%s %s %s "
        "--J=-Dgemfire.tombstone-timeout=%ld "
        "--J=-Dgemfire.tombstone-gc-hreshold=%ld "
        "--J=-Dgemfire.security-log-level=%s --J=-Xmx1024m --J=-Xms128m 2>&1",
        gfjavaenv, GFSH, classpath, sname.c_str(), xmlFile.c_str(),
        useSecurityManager ? "--user=root --password=root-password" : "",
        currDir.c_str(), portNum, gfLogLevel, geodeProperties.c_str(),
        authParam, deltaProperty.c_str(),
        testServerGC ? userTombstone_timeout : defaultTombstone_timeout,
        testServerGC ? userTombstone_gc_threshold
                     : defaultTombstone_gc_threshold,
        gfSecLogLevel);
  } else {
    sprintf(
        cmd,
        "%s/bin/%s start server --classpath=%s --name=%s "
        "--cache-xml-file=%s %s --dir=%s --server-port=%d --log-level=%s %s %s "
        "--J=-Dgemfire.tombstone-timeout=%ld "
        "--J=-Dgemfire.tombstone-gc-hreshold=%ld "
        "--J=-Dgemfire.security-log-level=%s --J=-Xmx1024m --J=-Xms128m 2>&1",
        gfjavaenv, GFSH, classpath, sname.c_str(), xmlFile.c_str(),
        useSecurityManager ? "--user=root --password=root-password" : "",
        currDir.c_str(), portNum, gfLogLevel, authParam, deltaProperty.c_str(),
        testServerGC ? userTombstone_timeout : defaultTombstone_timeout,
        testServerGC ? userTombstone_gc_threshold
                     : defaultTombstone_gc_threshold,
        gfSecLogLevel);
  }

  LOG(cmd);
  int e = ACE_OS::system(cmd);
  ASSERT(0 == e, "cmd failed");

  staticServerInstanceList.push_back(instance);
  printf("added server instance %d\n", instance);
}

void CacheHelper::createDuplicateXMLFile(std::string& originalFile,
                                         int hostport1, int hostport2,
                                         int locport1, int locport2) {
  char cmd[1024];
  char currWDPath[2048];
  std::string currDir = ACE_OS::getcwd(currWDPath, 2048);
  currDir += PATH_SEP;
  std::string testSrc = ACE_OS::getenv("TESTSRC");
  testSrc += PATH_SEP;
  testSrc += "resources";
  testSrc += PATH_SEP;

  // file name will have hostport1.xml(i.e. CacheHelper::staticHostPort1) as
  // suffix

  replacePortsInFile(
      hostport1, hostport2, CacheHelper::staticHostPort3,
      CacheHelper::staticHostPort4, locport1, locport2, testSrc + originalFile,
      currDir + originalFile + std::to_string(hostport1) + ".xml");

  // this file need to delete
  sprintf(cmd, "%s%s%d.xml", currDir.c_str(), originalFile.c_str(), hostport1);
  std::string s(cmd);
  CacheHelper::staticConfigFileList.push_back(s);

  printf("createDuplicateXMLFile added file %s %zd", cmd,
         CacheHelper::staticConfigFileList.size());
}

// Need to avoid regex usage in Solaris Studio 12.4.
#ifdef _SOLARIS
// @Solaris 12.4 compiler is missing support for C++11 regex
void CacheHelper::replacePortsInFile(int hostPort1, int hostPort2,
                                     int hostPort3, int hostPort4, int locPort1,
                                     int locPort2, const std::string& inFile,
                                     const std::string& outFile) {
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

void CacheHelper::replaceInPlace(std::string& searchStr,
                                 const std::string& matchStr,
                                 const std::string& replaceStr) {
  size_t pos = 0;
  while ((pos = searchStr.find(matchStr, pos)) != std::string::npos) {
    searchStr.replace(pos, matchStr.length(), replaceStr);
    pos += replaceStr.length();
  }
}
#else
void CacheHelper::replacePortsInFile(int hostPort1, int hostPort2,
                                     int hostPort3, int hostPort4, int locPort1,
                                     int locPort2, const std::string& inFile,
                                     const std::string& outFile) {
  std::ifstream in(inFile, std::ios::in | std::ios::binary);
  if (in) {
    std::string contents;
    contents.assign(std::istreambuf_iterator<char>(in),
                    std::istreambuf_iterator<char>());
    in.close();

    contents = std::regex_replace(contents, std::regex("HOST_PORT1"),
                                  std::to_string(hostPort1));
    contents = std::regex_replace(contents, std::regex("HOST_PORT2"),
                                  std::to_string(hostPort2));
    contents = std::regex_replace(contents, std::regex("HOST_PORT3"),
                                  std::to_string(hostPort3));
    contents = std::regex_replace(contents, std::regex("HOST_PORT4"),
                                  std::to_string(hostPort4));
    contents = std::regex_replace(contents, std::regex("LOC_PORT1"),
                                  std::to_string(locPort1));
    contents = std::regex_replace(contents, std::regex("LOC_PORT2"),
                                  std::to_string(locPort2));

    std::ofstream out(outFile, std::ios::out);
    out << contents;
    out.close();
  }
}
#endif

void CacheHelper::createDuplicateXMLFile(std::string& duplicateFile,
                                         std::string& originalFile) {
  CacheHelper::createDuplicateXMLFile(
      originalFile, CacheHelper::staticHostPort1, CacheHelper::staticHostPort2,
      CacheHelper::staticLocatorHostPort1, CacheHelper::staticLocatorHostPort2);

  char tmp[32];

  sprintf(tmp, "%d.xml", CacheHelper::staticHostPort1);

  char currWDPath[2048];
  duplicateFile = ACE_OS::getcwd(currWDPath, 2048);
  duplicateFile += PATH_SEP;
  duplicateFile += originalFile + tmp;
}

void CacheHelper::closeServer(int instance) {
  static char* gfjavaenv = ACE_OS::getenv("GFJAVA");

  char cmd[2048];
  char tmp[128];
  char currWDPath[2048];

  std::string currDir = ACE_OS::getcwd(currWDPath, 2048);

  ASSERT(gfjavaenv != nullptr,
         "Environment variable GFJAVA for java build directory is not set.");
  ASSERT(!currDir.empty(),
         "Current working directory could not be determined.");

  if ((ACE_OS::strchr(gfjavaenv, '\\') == nullptr) &&
      (ACE_OS::strchr(gfjavaenv, '/') == nullptr)) {
    return;
  }

  currDir += "/GFECS";
  switch (instance) {
    case 0:
      currDir += "0";
      break;
    case 1:
      sprintf(tmp, "%d", CacheHelper::staticHostPort1);
      currDir += tmp;  // currDir += "1";
      break;
    case 2:
      sprintf(tmp, "%d", CacheHelper::staticHostPort2);
      currDir += tmp;  // currDir += "2";
      break;
    case 3:
      sprintf(tmp, "%d", CacheHelper::staticHostPort3);
      currDir += tmp;  // currDir += "3";
      break;
    default: /* Support for any number of servers Max 10*/
      // ASSERT( ( instance <= 10 )," More than 10 servers not supported");
      // TODO: need to support more then three servers
      ASSERT((instance <= 4), " More than 4 servers not supported");
      char temp[8];
      currDir += ACE_OS::itoa(CacheHelper::staticHostPort4, temp, 10);
      break;
  }

  sprintf(cmd, "%s/bin/%s stop server --dir=%s 2>&1", gfjavaenv, GFSH,
          currDir.c_str());

  LOG(cmd);
  ACE_OS::system(cmd);

  terminate_process_file(currDir + "/vf.gf.server.pid",
                         std::chrono::seconds(10));

  staticServerInstanceList.remove(instance);
}
// closing locator
void CacheHelper::closeLocator(int instance, bool ssl) {
  static char* gfjavaenv = ACE_OS::getenv("GFJAVA");

  char cmd[2048];
  char currWDPath[2048];
  int portnum = 0;
  std::string currDir = ACE_OS::getcwd(currWDPath, 2048);
  std::string keystore = std::string(ACE_OS::getenv("TESTSRC")) + "/keystore";

  ASSERT(gfjavaenv != nullptr,
         "Environment variable GFJAVA for java build directory is not set.");
  ASSERT(!currDir.empty(),
         "Current working directory could not be determined.");
  if ((ACE_OS::strchr(gfjavaenv, '\\') == nullptr) &&
      (ACE_OS::strchr(gfjavaenv, '/') == nullptr)) {
    return;
  }

  currDir += PATH_SEP;
  currDir += "GFELOC";
  char tmp[100];

  switch (instance) {
    case 1:
      // portnum = 34756;
      portnum = CacheHelper::staticLocatorHostPort1;
      sprintf(tmp, "%d", CacheHelper::staticLocatorHostPort1);
      currDir += tmp;  // currDir += "1";
      break;
    case 2:
      // portnum = 34757;
      portnum = CacheHelper::staticLocatorHostPort2;
      sprintf(tmp, "%d", CacheHelper::staticLocatorHostPort2);
      currDir += tmp;
      // currDir += "2";
      break;
    case 3:
      // portnum = 34758;
      portnum = CacheHelper::staticLocatorHostPort3;
      sprintf(tmp, "%d", CacheHelper::staticLocatorHostPort3);
      currDir += tmp;
      // currDir += "3";
      break;
    default: /* Support for any number of Locator Max 10*/
      // TODO://
      ASSERT((instance <= 3), " More than 3 servers not supported");
      char temp[8];
      currDir += ACE_OS::itoa(instance, temp, 10);
      break;
  }

  sprintf(cmd, "%s/bin/%s stop locator --dir=%s", gfjavaenv, GFSH,
          currDir.c_str());
  LOG(cmd);
  ACE_OS::system(cmd);

  terminate_process_file(currDir + "/vf.gf.locator.pid",
                         std::chrono::seconds(10));

  sprintf(cmd, "%s .%stest.geode.properties", DELETE_COMMAND, PATH_SEP);
  LOG(cmd);
  ACE_OS::system(cmd);

  staticLocatorInstanceList.remove(instance);
}

template <class Rep, class Period>
void CacheHelper::terminate_process_file(
    const std::string& pidFileName,
    const std::chrono::duration<Rep, Period>& duration) {
  auto timeout = std::chrono::system_clock::now() + duration;

  std::string pid;
  read_single_line(pidFileName, pid);

  if (!pid.empty()) {
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
    LOG("ACE::terminate_process: pid=" + pid);
    ACE::terminate_process(std::stoi(pid));
  }
}

bool CacheHelper::file_exists(const std::string& fileName) {
  std::ifstream file(fileName);
  return file.is_open();
}

void CacheHelper::read_single_line(const std::string& fileName,
                                   std::string& str) {
  std::ifstream f(fileName);
  std::getline(f, str);
}

void CacheHelper::cleanupTmpConfigFiles() {
  std::list<std::string>::const_iterator its;

  char cmd[1024];
  for (its = CacheHelper::staticConfigFileList.begin();
       its != CacheHelper::staticConfigFileList.end(); ++its) {
    try {
      sprintf(cmd, "rm %s", its->c_str());
      LOG(cmd);
      ACE_OS::system(cmd);
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
void CacheHelper::initLocator(int instance, bool ssl, bool multiDS, int dsId,
                              int remoteLocator, bool untrustedCert,
                              bool useSecurityManager) {
  if (!isLocatorCleanupCallbackRegistered &&
      gClientCleanup.registerCallback(&CacheHelper::cleanupLocatorInstances)) {
    isLocatorCleanupCallbackRegistered = true;
  }
  static char* gfjavaenv = ACE_OS::getenv("GFJAVA");

  char cmd[2048];
  char currWDPath[2048];
  std::string currDir = ACE_OS::getcwd(currWDPath, 2048);
  //    std::string keystore = std::string(ACE_OS::getenv("TESTSRC")) +
  //    "/keystore";

  ASSERT(gfjavaenv != nullptr,
         "Environment variable GFJAVA for java build directory is not set.");
  ASSERT(!currDir.empty(),
         "Current working directory could not be determined.");

  if ((ACE_OS::strchr(gfjavaenv, '\\') == nullptr) &&
      (ACE_OS::strchr(gfjavaenv, '/') == nullptr)) {
    return;
  }
  std::string locDirname = "GFELOC";
  int portnum = 0;
  currDir += PATH_SEP;
  char tmp[100];
  switch (instance) {
    case 1:
      // portnum = 34756;
      portnum = CacheHelper::staticLocatorHostPort1;
      sprintf(tmp, "%d", CacheHelper::staticLocatorHostPort1);
      locDirname += tmp;
      // locDirname += "1";
      break;
    case 2:
      // portnum = 34757;
      portnum = CacheHelper::staticLocatorHostPort2;
      sprintf(tmp, "%d", CacheHelper::staticLocatorHostPort2);
      locDirname += tmp;
      // locDirname += "2";
      break;
    default:
      // portnum = 34758;
      portnum = CacheHelper::staticLocatorHostPort3;
      sprintf(tmp, "%d", CacheHelper::staticLocatorHostPort3);
      locDirname += tmp;
      // locDirname += "3";
      break;
  }

  currDir += locDirname;

  ACE_OS::mkdir(locDirname.c_str());

  std::string geodeFile = generateGeodeProperties(
      currDir, ssl, dsId, remoteLocator, untrustedCert, useSecurityManager);

  sprintf(cmd, "%s/bin/%s stop locator --dir=%s --properties-file=%s ",
          gfjavaenv, GFSH, currDir.c_str(), geodeFile.c_str());

  LOG(cmd);
  ACE_OS::system(cmd);

  static char* classpath = ACE_OS::getenv("GF_CLASSPATH");
  std::string propertiesFile =
      useSecurityManager
          ? std::string("--security-properties-file=") + geodeFile
          : std::string("--properties-file=") + geodeFile;
  sprintf(cmd,
          "%s/bin/%s start locator --name=%s --port=%d --dir=%s "
          "%s --http-service-port=0 --classpath=%s",
          gfjavaenv, GFSH, locDirname.c_str(), portnum, currDir.c_str(),
          propertiesFile.c_str(), classpath);

  LOG(cmd);
  ACE_OS::system(cmd);
  staticLocatorInstanceList.push_back(instance);
}

void CacheHelper::clearSecProp() {
  auto tmpSecProp = CacheHelper::getHelper()
                        .getCache()
                        ->getDistributedSystem()
                        .getSystemProperties()
                        .getSecurityProperties();
  tmpSecProp->remove("security-username");
  tmpSecProp->remove("security-password");
}
void CacheHelper::setJavaConnectionPoolSize(long size) {
  CacheHelper::getHelper()
      .getCache()
      ->getDistributedSystem()
      .getSystemProperties()
      .setjavaConnectionPoolSize(size);
}

bool CacheHelper::setSeed() {
  char* testName = ACE_OS::getenv("TESTNAME");

  int seed = hashcode(testName);

  printf("seed for process %d\n", seed);
  // The integration tests rely on the pseudo-random
  // number generator being seeded with a very particular
  // value specific to the test by way of the test name.
  // Whilst this approach is pessimal, it can not be
  // remedied as the test depend upon it.
  ACE_OS::srand(seed);
  return true;
}

int CacheHelper::hashcode(char* str) {
  if (str == nullptr) {
    return 0;
  }
  int localHash = 0;

  int prime = 31;
  char* data = str;
  for (int i = 0; i < 50 && (data[i] != '\0'); i++) {
    localHash = prime * localHash + data[i];
  }
  if (localHash > 0) return localHash;
  return -1 * localHash;
}

int CacheHelper::getRandomNumber() {
  // char * testName = ACE_OS::getenv( "TESTNAME" );

  // int seed = hashcode(testName);
  return (ACE_OS::rand() % RANDOM_NUMBER_DIVIDER) + RANDOM_NUMBER_OFFSET;
}

int CacheHelper::getRandomAvailablePort() {
  while (true) {
    int port = CacheHelper::getRandomNumber();
    ACE_INET_Addr addr(port, "localhost");
    ACE_SOCK_Acceptor acceptor;
    int result = acceptor.open(addr, 0, AF_INET);
    if (result == -1) {
      continue;
    } else {
      result = acceptor.close();
      if (result == -1) {
        continue;
      } else {
        return port;
      }
    }
  }
}

std::string CacheHelper::unitTestOutputFile() {
  char cwd[1024];
  if (!ACE_OS::getcwd(cwd, sizeof(cwd))) {
    throw Exception("Failed to get current working directory.");
  }

  std::string outputFile(cwd);
  outputFile += "/";
  outputFile += ACE_OS::getenv("TESTNAME");
  outputFile += ".log";

  return outputFile;
}

int CacheHelper::getNumLocatorListUpdates(const char* s) {
  std::string searchStr(s);
  std::string testFile = CacheHelper::unitTestOutputFile();
  FILE* fp = fopen(testFile.c_str(), "r");
  ASSERT(nullptr != fp, "Failed to open log file.");

  char buf[512];
  int numMatched = 0;
  while (fgets(buf, sizeof(buf), fp)) {
    std::string line(buf);
    if (line.find(searchStr) != std::string::npos) numMatched++;
  }
  return numMatched;
}

std::string CacheHelper::generateGeodeProperties(
    const std::string& path, const bool ssl, const int dsId,
    const int remoteLocator, const bool untrustedCert,
    const bool useSecurityManager) {
  char cmd[2048];
  std::string keystore = std::string(ACE_OS::getenv("TESTSRC")) + "/keystore";

  std::string geodeFile = path;
  geodeFile += "/test.geode.properties";
  sprintf(cmd, "%s %s%stest.geode.properties", DELETE_COMMAND, path.c_str(),
          PATH_SEP);
  LOG(cmd);
  ACE_OS::system(cmd);
  FILE* urandom = /*ACE_OS::*/
      fopen(geodeFile.c_str(), "w");
  char gemStr[258];
  sprintf(gemStr, "locators=localhost[%d],localhost[%d],localhost[%d]\n",
          CacheHelper::staticLocatorHostPort1,
          CacheHelper::staticLocatorHostPort2,
          CacheHelper::staticLocatorHostPort3);
  std::string msg = gemStr;

  msg += "log-level=config\n";
  msg += "mcast-port=0\n";
  msg += "enable-network-partition-detection=false\n";
  if (useSecurityManager) {
    msg += "security-manager=javaobject.SimpleSecurityManager\n";
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
      serverKeystore += "server_keystore.jks";
      serverTruststore += "server_truststore.jks";
      password += "gemstone";
    }
    msg += "jmx-manager-ssl-enabled=false\n";
    msg += "cluster-ssl-enabled=true\n";
    msg += "cluster-ssl-require-authentication=true\n";
    msg += "cluster-ssl-ciphers=TLS_RSA_WITH_AES_128_CBC_SHA\n";
    msg += "cluster-ssl-keystore-type=jks\n";
    msg += "cluster-ssl-keystore=" + keystore + "/" + serverKeystore.c_str() +
           "\n";
    msg += "cluster-ssl-keystore-password=" + password + "\n";
    msg += "cluster-ssl-truststore=" + keystore + "/" +
           serverTruststore.c_str() + "\n";
    msg += "cluster-ssl-truststore-password=" + password + "\n";
    msg += "security-username=xxxx\n";
    msg += "security-userPassword=yyyy \n";
  }
  if (remoteLocator != 0) {
    sprintf(gemStr, "distributed-system-id=%d\n remote-locators=localhost[%d]",
            dsId, remoteLocator);
  } else {
    sprintf(gemStr, "distributed-system-id=%d\n ", dsId);
  }
  msg += gemStr;

  /*ACE_OS::*/
  fwrite(msg.c_str(), msg.size(), 1, urandom);
  /*ACE_OS::*/
  fflush(urandom);
  /*ACE_OS::*/
  fclose(urandom);
  LOG(geodeFile.c_str());
  return geodeFile;
}
