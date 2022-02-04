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

#ifndef GEODE_DISTRIBUTEDSYSTEMIMPL_H_
#define GEODE_DISTRIBUTEDSYSTEMIMPL_H_

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <geode/internal/geode_globals.hpp>

#include "DistributedSystem.hpp"
#include "statistics/StatisticsManager.hpp"

namespace apache {
namespace geode {
namespace client {

class SystemProperties;

class DistributedSystemImpl;

using CliCallbackMethod = std::function<void(Cache&)>;

/**
 * @class DistributedSystemImpl DistributedSystemImpl.hpp
 * A "connection" to a Geode distributed system.
 * The connection will be through a (host, port) pair.
 */
class DistributedSystemImpl {
 public:
  static void setThreadName(const std::string& threadName, std::thread::id tid);
  static std::string getThreadName(std::thread::id id);

  /**
   * @brief destructor
   */
  virtual ~DistributedSystemImpl();

  /** Retrieve the MemberId used to create this Cache. */
  virtual void disconnect();

  virtual void connect();

  void logSystemInformation() const;

  virtual const std::string& getName() const;

  SystemProperties& getSystemProperties() const;

  std::string m_name;
  DistributedSystem* m_implementee;

  /**
   * @brief constructors
   */
  DistributedSystemImpl(std::string name, DistributedSystem* implementee,
                        std::unique_ptr<SystemProperties> sysProps);

  // acquire/release locks

  static void registerCliCallback(int appdomainId,
                                  CliCallbackMethod clicallback);

  static void unregisterCliCallback(int appdomainId);

  static void CallCliCallBack(Cache& cache);

 private:
  static std::recursive_mutex m_cliCallbackLock;
  static volatile bool m_isCliCallbackSet;
  static std::map<int, CliCallbackMethod> m_cliCallbackMap;
  std::unique_ptr<SystemProperties> m_sysProps;
  bool m_connected;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_DISTRIBUTEDSYSTEMIMPL_H_
