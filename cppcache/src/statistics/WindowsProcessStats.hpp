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

#ifndef GEODE_STATISTICS_WINDOWSPROCESSSTATS_H_
#define GEODE_STATISTICS_WINDOWSPROCESSSTATS_H_

#include <geode/geode_globals.hpp>
#include <geode/statistics/Statistics.hpp>
#include <geode/statistics/StatisticsType.hpp>
#include <geode/statistics/StatisticDescriptor.hpp>
#include <geode/statistics/StatisticsFactory.hpp>
#include <geode/ExceptionTypes.hpp>

#include "ProcessStats.hpp"
#include "GeodeStatisticsFactory.hpp"

using namespace apache::geode::client;

/** @file
 */

namespace apache {
namespace geode {
namespace statistics {
/**
 * <P>This class provides the interface for statistics about a
 * Windows operating system process that is using a Geode system.
 */

class CPPCACHE_EXPORT WindowsProcessStats : public ProcessStats {
 private:
  /** The Static Type for Windows Process Stats */
  StatisticsType* m_statsType;

  /** Ids of All Stats Desciptors for seting new values */
  int32_t handlesINT;
  int32_t priorityBaseINT;
  int32_t threadsINT;
  int32_t activeTimeLONG;
  int32_t pageFaultsLONG;
  int32_t pageFileSizeLONG;
  int32_t pageFileSizePeakLONG;
  int32_t privateSizeLONG;
  int32_t systemTimeLONG;
  int32_t userTimeLONG;
  int32_t virtualSizeLONG;
  int32_t virtualSizePeakLONG;
  int32_t workingSetSizeLONG;
  int32_t workingSetSizePeakLONG;
  int32_t cpuUsageINT;

  /** The underlying statistics */
  Statistics* stats;

  void createType(StatisticsFactory* statFactory);

 public:
  WindowsProcessStats(GeodeStatisticsFactory* statisticsFactory, int64_t pid,
                      const char* name);
  ~WindowsProcessStats();

  int64_t getProcessSize();
  int32_t getCpuUsage();
  int64_t getCPUTime();
  int32_t getNumThreads();
  int64_t getAllCpuTime();
  /**
   * Close Underline Statistics
   */
  void close();

  friend class HostStatHelperWin;
};
}  // namespace statistics
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STATISTICS_WINDOWSPROCESSSTATS_H_
