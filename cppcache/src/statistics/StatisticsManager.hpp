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
#include <vector>

#include <geode/geode_globals.hpp>
#include <geode/statistics/Statistics.hpp>
#include <geode/ExceptionTypes.hpp>

#include "HostStatSampler.hpp"
#include "StatisticsTypeImpl.hpp"
#include "AdminRegion.hpp"
#include "GeodeStatisticsFactory.hpp"

namespace apache {
namespace geode {
namespace statistics {

class GeodeStatisticsFactory;

/**
 * Head Application Manager for Statistics Module.
 *
 */
class StatisticsManager {
 private:
  // interval at which the sampler will take a sample of Stats
  int32_t m_sampleIntervalMs;

  // Statistics sampler
  HostStatSampler* m_sampler;

  // Vector containing all the Stats objects
  std::vector<Statistics*> m_statsList;

  // Vector containing stats pointers which are not yet sampled.
  std::vector<Statistics*> m_newlyAddedStatsList;

  // Mutex to lock the list of Stats
  ACE_Recursive_Thread_Mutex m_statsListLock;

  AdminRegionPtr m_adminRegion;

  std::unique_ptr<GeodeStatisticsFactory> m_statisticsFactory;

  void closeSampler();

 public:
  StatisticsManager(const char* filePath, int64_t sampleIntervalMs,
                    bool enabled, Cache* cache, const char* durableClientId,
                    const uint32_t durableTimeout, int64_t statFileLimit = 0,
                    int64_t statDiskSpaceLimit = 0);

  void RegisterAdminRegion(AdminRegionPtr adminRegPtr) {
    m_adminRegion = adminRegPtr;
  }

  AdminRegionPtr getAdminRegion() { return m_adminRegion; }

  void forceSample();

  ~StatisticsManager();

  int32_t getStatListModCount();

  void addStatisticsToList(Statistics* stat);

  std::vector<Statistics*>& getStatsList();

  std::vector<Statistics*>& getNewlyAddedStatsList();

  ACE_Recursive_Thread_Mutex& getListMutex();

  /** Return the first instance that matches the type, or nullptr */
  Statistics* findFirstStatisticsByType(StatisticsType* type);

  std::vector<Statistics*> findStatisticsByType(StatisticsType* type);

  std::vector<Statistics*> findStatisticsByTextId(char* textId);

  std::vector<Statistics*> findStatisticsByNumericId(int64_t numericId);

  Statistics* findStatisticsByUniqueId(int64_t uniqueId);

  static void deleteStatistics(Statistics*& stat);

  GeodeStatisticsFactory* getStatisticsFactory() const {
    return m_statisticsFactory.get();
  }

};  // class

}  // namespace statistics
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STATISTICS_STATISTICSMANAGER_H_
