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
  LOG_FINE("Destroyed DistributedSystemImpl");
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
  LOG_CONFIG("Using Geode Native Client Product Directory: " + productDir);

  // Add version information, source revision, current directory etc.
  LOG_CONFIG("Product version: %s",
             PRODUCT_VENDOR " " PRODUCT_NAME " " PRODUCT_VERSION
                            " (" PRODUCT_BITS ") " PRODUCT_BUILDDATE);
  LOG_CONFIG("Source revision: %s", PRODUCT_SOURCE_REVISION);
  LOG_CONFIG("Source repository: %s", PRODUCT_SOURCE_REPOSITORY);

  auto sysinfo = Utils::getSystemInfo();
  LOG_CONFIG("Running on: %s", sysinfo.c_str());
  LOG_CONFIG("Current directory: %s",
             boost::filesystem::current_path().string().c_str());
  LOG_CONFIG("Current value of PATH: %s", Utils::getEnv("PATH").c_str());
#ifndef _WIN32
  LOG_CONFIG("Current library path: %s",
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

  LOG_CONFIG("Stopped the Geode Native Client");

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
    LOG_FINE("Removing cliCallback %d", appdomainId);
  }
}

void DistributedSystemImpl::setThreadName(const std::string& threadName) {
  if (threadName.empty()) {
    throw IllegalArgumentException("Thread name is empty.");
  }

#if defined(HAVE_pthread_setname_np)

  pthread_setname_np(threadName.c_str());

#elif defined(_WIN32)

  const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push, 8)
  typedef struct tagTHREADNAME_INFO {
    DWORD dwType;      // Must be 0x1000.
    LPCSTR szName;     // Pointer to name (in user addr space).
    DWORD dwThreadID;  // Thread ID (-1=caller thread).
    DWORD dwFlags;     // Reserved for future use, must be zero.
  } THREADNAME_INFO;
#pragma pack(pop)

  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = threadName.c_str();
  info.dwThreadID = -1;
  info.dwFlags = 0;

  __try {
    RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR),
                   (ULONG_PTR*)&info);
  } __except (EXCEPTION_EXECUTE_HANDLER) {
  }

#endif
}

}  // namespace client
}  // namespace geode
}  // namespace apache
