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

#ifndef GEODE_STATISTICS_POOLSTATSSAMPLER_H_
#define GEODE_STATISTICS_POOLSTATSSAMPLER_H_

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include <geode/internal/geode_globals.hpp>

namespace apache {
namespace geode {

namespace client {

class CacheImpl;
class ThinClientBaseDM;
class AdminRegion;
class ThinClientPoolDM;

}  // namespace client

namespace statistics {

using client::AdminRegion;
using client::CacheImpl;
using client::ThinClientPoolDM;

class StatisticsManager;
class GeodeStatisticsFactory;

class PoolStatsSampler {
 public:
  PoolStatsSampler() = delete;
  PoolStatsSampler(std::chrono::milliseconds sampleRate, CacheImpl* cache,
                   ThinClientPoolDM* distMan);
  PoolStatsSampler& operator=(const PoolStatsSampler&) = delete;
  PoolStatsSampler(const PoolStatsSampler& PoolStatsSampler) = delete;
  ~PoolStatsSampler() noexcept = default;

  void start();
  void stop();
  void svc(void);
  bool isRunning();

 private:
  void putStatsInAdminRegion();
  std::thread m_thread;
  std::atomic<bool> m_running;
  std::atomic<bool> m_stopRequested;
  std::chrono::milliseconds m_sampleRate;
  std::shared_ptr<AdminRegion> m_adminRegion;
  ThinClientPoolDM* m_distMan;
  static const char* NC_PSS_Thread;
  GeodeStatisticsFactory* m_statisticsFactory;
};

}  // namespace statistics

}  // namespace geode
}  // namespace apache

#endif  // GEODE_STATISTICS_POOLSTATSSAMPLER_H_
