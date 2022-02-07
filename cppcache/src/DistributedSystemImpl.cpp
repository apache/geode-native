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

#include "DistributedSystemImpl.hpp"

#include "Utils.hpp"

#if defined(HAVE_PTHREAD_H)
#include <pthread.h>
#elif defined(_WIN32)
//#include <windows.h>
#endif

#include <boost/filesystem.hpp>

#include <geode/SystemProperties.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "CppCacheLibrary.hpp"
#include "PoolStatistics.hpp"
#include "RegionStats.hpp"
#include "config.h"
#include "util/Log.hpp"
#include "version.h"

namespace apache {
namespace geode {
namespace client {

volatile bool DistributedSystemImpl::m_isCliCallbackSet = false;
std::map<int, CliCallbackMethod> DistributedSystemImpl::m_cliCallbackMap;
std::recursive_mutex DistributedSystemImpl::m_cliCallbackLock;

DistributedSystemImpl::DistributedSystemImpl(
    std::string name, DistributedSystem* implementee,
    std::unique_ptr<SystemProperties> sysProps)
    : m_name(name),
      m_implementee(implementee),
      m_sysProps(std::move(sysProps)),
      m_connected(false) {
  logSystemInformation();
}

DistributedSystemImpl::~DistributedSystemImpl() {
  LOGFINE("Destroyed DistributedSystemImpl");
}

void DistributedSystemImpl::connect() {
  if (m_connected) {
    throw AlreadyConnectedException(
        "DistributedSystem::connect: already connected, call getInstance to "
        "get it");
  }

  m_connected = true;
}

void DistributedSystemImpl::logSystemInformation() const {
  auto productDir = CppCacheLibrary::getProductDir();
  LOGCONFIG("Using Geode Native Client Product Directory: " + productDir);

  // Add version information, source revision, current directory etc.
  LOGCONFIG("Product version: %s",
            PRODUCT_VENDOR " " PRODUCT_NAME " " PRODUCT_VERSION
                           " (" PRODUCT_BITS ") " PRODUCT_BUILDDATE);
  LOGCONFIG("Source revision: %s", PRODUCT_SOURCE_REVISION);
  LOGCONFIG("Source repository: %s", PRODUCT_SOURCE_REPOSITORY);

  auto sysinfo = Utils::getSystemInfo();
  LOGCONFIG("Running on: %s", sysinfo.c_str());
  LOGCONFIG("Current directory: %s",
            boost::filesystem::current_path().string().c_str());
  LOGCONFIG("Current value of PATH: %s", Utils::getEnv("PATH").c_str());
#ifndef _WIN32
  LOGCONFIG("Current library path: %s",
            Utils::getEnv("LD_LIBRARY_PATH").c_str());
#endif
  // Log the Geode system properties
  m_sysProps->logSettings();
}

void DistributedSystemImpl::disconnect() {
  if (!m_connected) {
    throw NotConnectedException(
        "DistributedSystem::disconnect: connect "
        "not called");
  }

  LOGCONFIG("Stopped the Geode Native Client");

  // TODO global - log stays global so lets move this
  Log::close();

  m_connected = false;
}

SystemProperties& DistributedSystemImpl::getSystemProperties() const {
  return *m_sysProps;
}

const std::string& DistributedSystemImpl::getName() const { return m_name; }

void DistributedSystemImpl::CallCliCallBack(Cache& cache) {
  std::lock_guard<decltype(m_cliCallbackLock)> disconnectGuard(
      m_cliCallbackLock);
  if (m_isCliCallbackSet == true) {
    for (const auto& iter : m_cliCallbackMap) {
      iter.second(cache);
    }
  }
}

void DistributedSystemImpl::registerCliCallback(int appdomainId,
                                                CliCallbackMethod clicallback) {
  std::lock_guard<decltype(m_cliCallbackLock)> disconnectGuard(
      m_cliCallbackLock);
  m_cliCallbackMap[appdomainId] = clicallback;
  m_isCliCallbackSet = true;
}

void DistributedSystemImpl::unregisterCliCallback(int appdomainId) {
  std::lock_guard<decltype(m_cliCallbackLock)> disconnectGuard(
      m_cliCallbackLock);
  auto iter = m_cliCallbackMap.find(appdomainId);
  if (iter != m_cliCallbackMap.end()) {
    m_cliCallbackMap.erase(iter);
    LOGFINE("Removing cliCallback %d", appdomainId);
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
