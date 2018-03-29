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

#include <geode/internal/geode_globals.hpp>
#include <geode/DistributedSystem.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/SystemProperties.hpp>
#include <geode/DataOutput.hpp>

#include "CppCacheLibrary.hpp"
#include "Utils.hpp"
#include "util/Log.hpp"
#include "ExpiryTaskManager.hpp"
#include "CacheImpl.hpp"
#include "TcrMessage.hpp"
#include "DistributedSystemImpl.hpp"

namespace apache {
namespace geode {
namespace client {

using namespace apache::geode::statistics;

DistributedSystem::DistributedSystem(const std::string& name,
                                     std::unique_ptr<SystemProperties> sysProps)
    : m_impl(new DistributedSystemImpl(name, this, std::move(sysProps))) {}

DistributedSystem::DistributedSystem(DistributedSystem&& moved)
    : m_impl(std::move(moved.m_impl)) {
  m_impl->m_implementee = this;
}

DistributedSystem::~DistributedSystem() = default;

DistributedSystem DistributedSystem::create(
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
    CppCacheLibrary::getProductDir();
  } catch (const Exception&) {
    LOGERROR(
        "Unable to determine Product Directory. Please set the "
        "GFCPP environment variable.");
    throw;
  }

  auto distributedSystem = DistributedSystem(name, std::move(sysProps));

  LOGCONFIG("Starting the Geode Native Client");
  return distributedSystem;
}

void DistributedSystem::connect(Cache* cache) { m_impl->connect(cache); }

void DistributedSystem::disconnect() { m_impl->disconnect(); }

SystemProperties& DistributedSystem::getSystemProperties() const {
  return m_impl->getSystemProperties();
}

const std::string& DistributedSystem::getName() const {
  return m_impl->getName();
}

statistics::StatisticsManager* DistributedSystem::getStatisticsManager() const {
  return m_impl->getStatisticsManager();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
