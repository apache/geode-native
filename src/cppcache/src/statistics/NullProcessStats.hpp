#pragma once

#ifndef GEODE_STATISTICS_NULLPROCESSSTATS_H_
#define GEODE_STATISTICS_NULLPROCESSSTATS_H_

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

#include <geode/geode_globals.hpp>
#include <geode/statistics/Statistics.hpp>
#include <geode/statistics/StatisticsType.hpp>
#include <geode/statistics/StatisticDescriptor.hpp>
#include "ProcessStats.hpp"
#include "HostStatHelper.hpp"

using namespace apache::geode::client;

/** @file
 */

namespace apache {
namespace geode {
namespace statistics {
/**
 * <P>This class provides the interface for statistics about a
 * Null operating system process that is using a Geode system.
 *
 */

class CPPCACHE_EXPORT NullProcessStats : public ProcessStats {
 public:
  NullProcessStats(int64_t pid, const char* name);
  ~NullProcessStats();

  int64_t getProcessSize();
  int32_t getCpuUsage();
  int64_t getCPUTime();
  int32_t getNumThreads();
  int64_t getAllCpuTime();
  void close();
};  // Class NullProcessStats
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STATISTICS_NULLPROCESSSTATS_H_
