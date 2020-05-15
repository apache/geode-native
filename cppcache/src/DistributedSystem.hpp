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

#ifndef GEODE_DISTRIBUTEDSYSTEM_H_
#define GEODE_DISTRIBUTEDSYSTEM_H_

/**
 * @file
 */
#include <memory>

#include <geode/ExceptionTypes.hpp>
#include <geode/Properties.hpp>
#include <geode/internal/geode_globals.hpp>

namespace apache {
namespace geode {

namespace statistics {
class StatisticsManager;
}  // namespace statistics

namespace client {
/**
 * @class DistributedSystem DistributedSystem.hpp
 * DistributedSystem encapsulates this applications "connection" into the
 * Geode Java servers distributed system. In order to participate in the
 * Geode Java servers distributed system, each application needs to connect to
 * the DistributedSystem.
 * Each application can only be connected to one DistributedSystem.
 */
class SystemProperties;
class DistributedSystemImpl;
class CacheRegionHelper;

class APACHE_GEODE_EXPORT DistributedSystem {
  /**
   * @brief public methods
   */
 public:
  DistributedSystem() = delete;
  ~DistributedSystem() noexcept;
  DistributedSystem(const DistributedSystem&) = delete;
  DistributedSystem& operator=(const DistributedSystem&) = delete;
  DistributedSystem(DistributedSystem&&);
  DistributedSystem& operator=(DistributedSystem&&) = delete;

  /**
   * Initializes the Native Client system to be able to connect to the
   * Geode Java servers. If the name string is empty, then the default
   * "NativeDS" is used as the name of distributed system.
   * @throws IllegalStateException if GEODE_NATIVE_HOME variable is not set and
   *   product installation directory cannot be determined
   **/
  static DistributedSystem create(
      const std::string& name,
      const std::shared_ptr<Properties>& configPtr = nullptr);

  /**
   * @brief connects from the distributed system
   * @throws AlreadyConnectedException if this call has succeeded once before
   */
  void connect();

  /**
   * @brief disconnect from the distributed system
   * @throws IllegalStateException if not connected
   */
  void disconnect();

  /** Returns the SystemProperties that were used to create this instance of the
   *  DistributedSystem
   *  @return  SystemProperties
   */
  SystemProperties& getSystemProperties() const;

  /** Returns the name that identifies the distributed system instance
   * @return  name
   */
  const std::string& getName() const;

 protected:
  /**
   * @brief constructors
   */
  DistributedSystem(const std::string& name,
                    std::unique_ptr<SystemProperties> sysProps);

 private:
  std::unique_ptr<DistributedSystemImpl> m_impl;

  friend class CacheRegionHelper;
  friend class DistributedSystemImpl;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_DISTRIBUTEDSYSTEM_H_
