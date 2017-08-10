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
#include "config.h"
#include <geode/geode_globals.hpp>

#include <geode/DistributedSystem.hpp>
#include "statistics/StatisticsManager.hpp"
#include <geode/SystemProperties.hpp>

#include <CppCacheLibrary.hpp>
#include <Utils.hpp>
#include <geode/Log.hpp>
#include <geode/CacheFactory.hpp>
#include <ace/OS.h>

#include <ExpiryTaskManager.hpp>
#include <CacheImpl.hpp>
#include <ace/Guard_T.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <geode/DataOutput.hpp>
#include <TcrMessage.hpp>
#include <DistributedSystemImpl.hpp>
#include <RegionStats.hpp>
#include <PoolStatistics.hpp>

#include <DiffieHellman.hpp>

#include "version.h"

using namespace apache::geode::client;
using namespace apache::geode::statistics;

ACE_Recursive_Thread_Mutex* g_disconnectLock = new ACE_Recursive_Thread_Mutex();

namespace {}  // namespace

namespace apache {
namespace geode {
namespace client {
void setLFH() {
#ifdef _WIN32
  static HINSTANCE kernelMod = nullptr;
  if (kernelMod == nullptr) {
    kernelMod = GetModuleHandle("kernel32");
    if (kernelMod != nullptr) {
      typedef BOOL(WINAPI * PHSI)(
          HANDLE HeapHandle, HEAP_INFORMATION_CLASS HeapInformationClass,
          PVOID HeapInformation, SIZE_T HeapInformationLength);
      typedef HANDLE(WINAPI * PGPH)();
      PHSI pHSI = nullptr;
      PGPH pGPH = nullptr;
      if ((pHSI = (PHSI)GetProcAddress(kernelMod, "HeapSetInformation")) !=
          nullptr) {
        // The LFH API is available
        /* Only set LFH for process heap; causes problems in C++ framework if
        set for all heaps
        HANDLE hProcessHeapHandles[1024];
        DWORD dwRet;
        ULONG heapFragValue = 2;

        dwRet= GetProcessHeaps( 1024, hProcessHeapHandles );
        for (DWORD i = 0; i < dwRet; i++)
        {
          HeapSetInformation( hProcessHeapHandles[i],
            HeapCompatibilityInformation, &heapFragValue, sizeof(heapFragValue)
        );
        }
        */
        HANDLE hProcessHeapHandle;
        ULONG heapFragValue = 2;
        if ((pGPH = (PGPH)GetProcAddress(kernelMod, "GetProcessHeap")) !=
            nullptr) {
          hProcessHeapHandle = pGPH();
          LOGCONFIG(
              "Setting Microsoft Windows' low-fragmentation heap for use as "
              "the main process heap.");
          pHSI(hProcessHeapHandle, HeapCompatibilityInformation, &heapFragValue,
               sizeof(heapFragValue));
        }
      }
    }
  }
#endif
}
}  // namespace client
}  // namespace geode
}  // namespace apache

DistributedSystem::DistributedSystem(
    const std::string& name, std::unique_ptr<StatisticsManager> statMngr,
    std::unique_ptr<SystemProperties> sysProps)
    : m_name(name),
      m_statisticsManager(std::move(statMngr)),
      m_sysProps(std::move(sysProps)),
      m_connected(false) {
  LOGDEBUG("DistributedSystem::DistributedSystem");
  if (strlen(m_sysProps->securityClientDhAlgo()) > 0) {
    DiffieHellman::initOpenSSLFuncPtrs();
  }
}
DistributedSystem::~DistributedSystem() {}

void DistributedSystem::logSystemInformation() {
  std::string gfcpp = CppCacheLibrary::getProductDir();
  LOGCONFIG("Using Geode Native Client Product Directory: %s", gfcpp.c_str());

  // Add version information, source revision, current directory etc.
  LOGCONFIG("Product version: %s",
            PRODUCT_VENDOR " " PRODUCT_NAME " " PRODUCT_VERSION
                           " (" PRODUCT_BITS ") " PRODUCT_BUILDDATE);
  LOGCONFIG("Source revision: %s", PRODUCT_SOURCE_REVISION);
  LOGCONFIG("Source repository: %s", PRODUCT_SOURCE_REPOSITORY);

  ACE_utsname u;
  ACE_OS::uname(&u);
  LOGCONFIG(
      "Running on: SystemName=%s Machine=%s Host=%s Release=%s Version=%s",
      u.sysname, u.machine, u.nodename, u.release, u.version);

#ifdef _WIN32
  const uint32_t pathMax = _MAX_PATH;
#else
  const uint32_t pathMax = PATH_MAX;
#endif
  ACE_TCHAR cwd[pathMax + 1];
  (void)ACE_OS::getcwd(cwd, pathMax);
  LOGCONFIG("Current directory: %s", cwd);
  LOGCONFIG("Current value of PATH: %s", ACE_OS::getenv("PATH"));
#ifndef _WIN32
  const char* ld_libpath = ACE_OS::getenv("LD_LIBRARY_PATH");
  LOGCONFIG("Current library path: %s",
            ld_libpath == nullptr ? "nullptr" : ld_libpath);
#endif
  // Log the Geode system properties
  m_sysProps->logSettings();
}

std::unique_ptr<DistributedSystem> DistributedSystem::create(
    const std::string& _name, Cache* cache, const PropertiesPtr& configPtr) {
  // TODO global - Refactory out the static initialization
  // Trigger other library initialization.
  CppCacheLibrary::initLib();

  auto sysProps = std::unique_ptr<SystemProperties>(
      new SystemProperties(configPtr, nullptr));

  // TODO global - Refactor this to some process helper
  Exception::setStackTraces(sysProps->debugStackTraceEnabled());

  auto name = _name;
  if (name.empty()) {
    name = "NativeDS";
  }

  // Set client name via native client API
  const char* propName = sysProps->name();
  if (propName != nullptr && strlen(propName) > 0) {
    name = propName;
  }

  // TODO global - keep global but setup once.
  const char* logFilename = sysProps->logFilename();
  if (logFilename) {
    try {
      Log::close();
      Log::init(sysProps->logLevel(), logFilename, sysProps->logFileSizeLimit(),
                sysProps->logDiskSpaceLimit());
    } catch (const GeodeIOException&) {
      Log::close();
      sysProps = nullptr;
      throw;
    }
  } else {
    Log::setLogLevel(sysProps->logLevel());
  }

  try {
    std::string gfcpp = CppCacheLibrary::getProductDir();
  } catch (const Exception&) {
    LOGERROR(
        "Unable to determine Product Directory. Please set the "
        "GFCPP environment variable.");
    throw;
  }

  std::unique_ptr<StatisticsManager> statMngr;
  try {
    statMngr = std::unique_ptr<StatisticsManager>(new StatisticsManager(
        sysProps->statisticsArchiveFile(), sysProps->statisticsSampleInterval(),
        sysProps->statisticsEnabled(), cache, sysProps->durableClientId(),
        sysProps->durableTimeout(), sysProps->statsFileSizeLimit(),
        sysProps->statsDiskSpaceLimit()));
  } catch (const NullPointerException&) {
    Log::close();
    throw;
  }
  GF_D_ASSERT(m_statisticsManager != nullptr);

  auto distributedSystem = std::unique_ptr<DistributedSystem>(
      new DistributedSystem(name, std::move(statMngr), std::move(sysProps)));
  if (!distributedSystem) {
    throw NullPointerException("DistributedSystem::connect: new failed");
  }
  distributedSystem->m_impl =
      new DistributedSystemImpl(name.c_str(), distributedSystem.get());

  distributedSystem->logSystemInformation();
  LOGCONFIG("Starting the Geode Native Client");
  return distributedSystem;
}

void DistributedSystem::connect() {
  ACE_Guard<ACE_Recursive_Thread_Mutex> disconnectGuard(*g_disconnectLock);
  if (m_connected == true) {
    throw AlreadyConnectedException(
        "DistributedSystem::connect: already connected, call getInstance to "
        "get it");
  }

  try {
    m_impl->connect();
  } catch (const apache::geode::client::Exception& e) {
    LOGERROR("Exception caught during client initialization: %s",
             e.getMessage());
    std::string msg = "DistributedSystem::connect: caught exception: ";
    msg.append(e.getMessage());
    throw NotConnectedException(msg.c_str());
  } catch (const std::exception& e) {
    LOGERROR("Exception caught during client initialization: %s", e.what());
    std::string msg = "DistributedSystem::connect: caught exception: ";
    msg.append(e.what());
    throw NotConnectedException(msg.c_str());
  } catch (...) {
    LOGERROR("Unknown exception caught during client initialization");
    throw NotConnectedException(
        "DistributedSystem::connect: caught unknown exception");
  }

  m_connected = true;
}

void DistributedSystem::disconnect() {
  ACE_Guard<ACE_Recursive_Thread_Mutex> disconnectGuard(*g_disconnectLock);

  if (!m_connected) {
    throw NotConnectedException(
        "DistributedSystem::disconnect: connect "
        "not called");
  }

  if (m_impl) {
    m_impl->disconnect();
    delete m_impl;
    m_impl = nullptr;
  }

  LOGFINEST("Deleted DistributedSystemImpl");

  LOGCONFIG("Stopped the Geode Native Client");

  // TODO global - log stays global so lets move this
  Log::close();

  m_connected = false;
}

SystemProperties& DistributedSystem::getSystemProperties() const {
  return *m_sysProps;
}

const std::string& DistributedSystem::getName() const { return m_name; }
