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

#include <ace/OS.h>
#include <ace/Guard_T.h>
#include <ace/Recursive_Thread_Mutex.h>

#include <geode/geode_globals.hpp>
#include <geode/DistributedSystem.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/SystemProperties.hpp>
#include <geode/DataOutput.hpp>

#include "config.h"
#include "version.h"

#include "CppCacheLibrary.hpp"
#include "Utils.hpp"
#include "util/Log.hpp"
#include "statistics/StatisticsManager.hpp"
#include "ExpiryTaskManager.hpp"
#include "CacheImpl.hpp"
#include "TcrMessage.hpp"
#include "DistributedSystemImpl.hpp"
#include "RegionStats.hpp"
#include "PoolStatistics.hpp"
#include "CacheRegionHelper.hpp"
#include "DiffieHellman.hpp"

namespace apache {
namespace geode {
namespace client {

using namespace apache::geode::statistics;

DistributedSystem::DistributedSystem(
    const std::string& name,
    std::unique_ptr<SystemProperties> sysProps)
    : m_name(name),
      m_statisticsManager(nullptr),
      m_sysProps(std::move(sysProps)),
      m_connected(false) {
  LOGDEBUG("DistributedSystem::DistributedSystem");
  if (!m_sysProps->securityClientDhAlgo().empty()) {
    DiffieHellman::initOpenSSLFuncPtrs();
  }
}
DistributedSystem::~DistributedSystem() {}

void DistributedSystem::logSystemInformation() const {
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
    const std::string& _name, const std::shared_ptr<Properties>& configPtr) {
  // TODO global - Refactory out the static initialization
  // Trigger other library initialization.
  CppCacheLibrary::initLib();

  auto sysProps =
      std::unique_ptr<SystemProperties>(new SystemProperties(configPtr));

  auto name = _name;
  if (name.empty()) {
    name = "NativeDS";
  }

  // Set client name via native client API
  auto&& propName = sysProps->name();
  if (!propName.empty()) {
    name = propName;
  }

  // TODO global - keep global but setup once.
  auto&& logFilename = sysProps->logFilename();
  if (!logFilename.empty()) {
    try {
      Log::close();
      Log::init(sysProps->logLevel(), logFilename.c_str(),
                sysProps->logFileSizeLimit(), sysProps->logDiskSpaceLimit());
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

  auto distributedSystem = std::unique_ptr<DistributedSystem>(
      new DistributedSystem(name,  std::move(sysProps)));
  if (!distributedSystem) {
    throw NullPointerException("DistributedSystem::connect: new failed");
  }
  distributedSystem->m_impl =
      new DistributedSystemImpl(name.c_str(), distributedSystem.get());

  distributedSystem->logSystemInformation();
  LOGCONFIG("Starting the Geode Native Client");
  return distributedSystem;
}

void DistributedSystem::connect(Cache* cache) {
  if (m_connected == true) {
    throw AlreadyConnectedException(
        "DistributedSystem::connect: already connected, call getInstance to "
        "get it");
  }

  try {
    m_impl->connect();
  } catch (const apache::geode::client::Exception& e) {
    LOGERROR("Exception caught during client initialization: %s", e.what());
    std::string msg = "DistributedSystem::connect: caught exception: ";
    msg.append(e.what());
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

  auto cacheImpl = CacheRegionHelper::getCacheImpl(cache);
  try {
    m_statisticsManager =
        std::unique_ptr<StatisticsManager>(new StatisticsManager(
            m_sysProps->statisticsArchiveFile().c_str(),
            m_sysProps->statisticsSampleInterval(),
            m_sysProps->statisticsEnabled(), cacheImpl,
            m_sysProps->durableClientId().c_str(), m_sysProps->durableTimeout(),
            m_sysProps->statsFileSizeLimit(),
            m_sysProps->statsDiskSpaceLimit()));
    cacheImpl->m_cacheStats =
        new CachePerfStats(getStatisticsManager()->getStatisticsFactory());
  }
  catch (const NullPointerException&) {
    Log::close();
    throw;
  }

  m_connected = true;
}

void DistributedSystem::disconnect() {
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

}  // namespace client
}  // namespace geode
}  // namespace apache
