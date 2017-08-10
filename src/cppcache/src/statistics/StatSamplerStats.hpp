#pragma once

#ifndef GEODE_STATISTICS_STATSAMPLERSTATS_H_
#define GEODE_STATISTICS_STATSAMPLERSTATS_H_

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
#include <geode/statistics/StatisticDescriptor.hpp>
#include <geode/statistics/StatisticsType.hpp>
#include <geode/statistics/Statistics.hpp>
#include <geode/statistics/StatisticsFactory.hpp>

using namespace apache::geode::client;

/** @file
 */

namespace apache {
namespace geode {
namespace statistics {

class StatisticsFactory;
/**
 * Statistics related to the statistic sampler.
 */
class CPPCACHE_EXPORT StatSamplerStats {
 private:
  StatisticsType* samplerType;
  Statistics* samplerStats;
  int32_t sampleCountId;
  int32_t sampleTimeId;
  StatisticDescriptor** statDescriptorArr;

 public:
  StatSamplerStats(StatisticsFactory* statFactory);
  void tookSample(int64_t nanosSpentWorking);
  void close();
  void setInitialValues();
  ~StatSamplerStats();
};
}  // namespace statistics
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STATISTICS_STATSAMPLERSTATS_H_
