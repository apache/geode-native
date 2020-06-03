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

#ifndef GEODE_STATISTICS_STATISTICSMANAGER_H_
#define GEODE_STATISTICS_STATISTICSMANAGER_H_

#include <memory>
#include <mutex>
#include <vector>

#include <geode/ExceptionTypes.hpp>
#include <geode/internal/geode_globals.hpp>

#include "GeodeStatisticsFactory.hpp"
#include "Statistics.hpp"
#include "StatisticsTypeImpl.hpp"

namespace apache {
namespace geode {

namespace client {

class AdminRegion;
class CacheImpl;

}  // namespace client

namespace statistics {

class GeodeStatisticsFactory;
class HostStatSampler;

/**
 * Head Application Manager for Statistics Module.
 *
 */
class StatisticsManager {
  // interval at which the sampler will take a sample of Stats
  std::chrono::milliseconds m_sampleIntervalMs;

  // Statistics sampler
  std::unique_ptr<HostStatSampler> m_sampler;

  // Vector containing all the Stats objects
  std::vector<Statistics*> m_statsList;

  // Vector containing stats pointers which are not yet sampled.
  std::vector<Statistics*> m_newlyAddedStatsList;

  // Mutex to lock the list of Stats
  std::recursive_mutex m_statsListLock;

  std::shared_ptr<client::AdminRegion> m_adminRegion;

  std::unique_ptr<GeodeStatisticsFactory> m_statisticsFactory;

  void closeSampler();

 public:
  StatisticsManager(const char* filePath,
                    std::chrono::milliseconds sampleIntervalMs, bool enabled,
                    client::CacheImpl* cache, int64_t statFileLimit = 0,
                    int64_t statDiskSpaceLimit = 0);

  void RegisterAdminRegion(std::shared_ptr<client::AdminRegion> adminRegPtr);

  std::shared_ptr<client::AdminRegion> getAdminRegion();

  void forceSample();

  ~StatisticsManager();

  int32_t getStatListModCount();

  void addStatisticsToList(Statistics* stat);

  std::vector<Statistics*>& getStatsList();

  std::vector<Statistics*>& getNewlyAddedStatsList();

  std::recursive_mutex& getListMutex();

  /** Return the first instance that matches the type, or nullptr */
  Statistics* findFirstStatisticsByType(const StatisticsType* type);

  std::vector<Statistics*> findStatisticsByType(StatisticsType* type);

  std::vector<Statistics*> findStatisticsByTextId(char* textId);

  std::vector<Statistics*> findStatisticsByNumericId(int64_t numericId);

  Statistics* findStatisticsByUniqueId(int64_t uniqueId);

  static void deleteStatistics(Statistics*& stat);

  GeodeStatisticsFactory* getStatisticsFactory() const;
};  // class

}  // namespace statistics
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STATISTICS_STATISTICSMANAGER_H_
