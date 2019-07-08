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

#include "DistributedSystem.hpp"

#include <geode/CacheFactory.hpp>
#include <geode/DataOutput.hpp>
#include <geode/SystemProperties.hpp>
#include <geode/internal/geode_globals.hpp>

#include "CacheImpl.hpp"
#include "CppCacheLibrary.hpp"
#include "DistributedSystemImpl.hpp"
#include "ExpiryTaskManager.hpp"
#include "TcrMessage.hpp"
#include "Utils.hpp"
#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

DistributedSystem::DistributedSystem(const std::string& name,
                                     std::unique_ptr<SystemProperties> sysProps)
    : m_impl(new DistributedSystemImpl(name, this, std::move(sysProps))) {}

DistributedSystem::DistributedSystem(DistributedSystem&& moved)
    : m_impl(std::move(moved.m_impl)) {
  m_impl->m_implementee = this;
}

DistributedSystem::~DistributedSystem() noexcept = default;

DistributedSystem DistributedSystem::create(
    const std::string& _name, const std::shared_ptr<Properties>& configPtr) {
  // TODO global - Refactory out the static initialization
  // Trigger other library initialization.
  CppCacheLibrary::initLib();

  auto systemProperties =
      std::unique_ptr<SystemProperties>(new SystemProperties(configPtr));

  auto name = _name;
  if (name.empty()) {
    name = "NativeDS";
  }

  // Set client name via native client API
  auto&& propName = systemProperties->name();
  if (!propName.empty()) {
    name = propName;
  }

  // TODO global - keep global but setup once.
  auto&& logFilename = systemProperties->logFilename();
  if (!logFilename.empty()) {
    try {
      Log::close();
      Log::init(systemProperties->logLevel(), logFilename,
                systemProperties->logFileSizeLimit(),
                systemProperties->logDiskSpaceLimit());
    } catch (const GeodeIOException&) {
      Log::close();
      systemProperties = nullptr;
      throw;
    }
  } else {
    Log::init(systemProperties->logLevel());
  }

  try {
    CppCacheLibrary::getProductDir();
  } catch (const Exception&) {
    LOGERROR(
        "Unable to determine Product Directory. Please set the "
        "GEODE_NATIVE_HOME environment variable.");
    throw;
  }

  auto distributedSystem = DistributedSystem(name, std::move(systemProperties));

  LOGCONFIG("Starting the Geode Native Client");
  return distributedSystem;
}

void DistributedSystem::connect() { m_impl->connect(); }

void DistributedSystem::disconnect() { m_impl->disconnect(); }

SystemProperties& DistributedSystem::getSystemProperties() const {
  return m_impl->getSystemProperties();
}

const std::string& DistributedSystem::getName() const {
  return m_impl->getName();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
