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

#ifndef GEODE_STATISTICS_GEODESTATISTICSFACTORY_H_
#define GEODE_STATISTICS_GEODESTATISTICSFACTORY_H_

#include <vector>

#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Map_Manager.h>

#include <geode/geode_globals.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/statistics/StatisticsFactory.hpp>

#include "StatisticsTypeImpl.hpp"
#include "StatisticsManager.hpp"

/** @file
 */

namespace apache {
namespace geode {
namespace statistics {

using namespace apache::geode::client;

class StatisticsManager;

/**
 * Geode's implementation of {@link StatisticsFactory}.
 *
 */
class GeodeStatisticsFactory : public StatisticsFactory {
 private:
  std::string m_name;

  int64_t m_id;

  StatisticsManager* m_statMngr;

  int64_t m_statsListUniqueId;  // Creates a unique id for each stats object in
                                // the list

  ACE_Recursive_Thread_Mutex m_statsListUniqueIdLock;

  /* Maps a stat name to its StatisticDescriptor*/
  ACE_Map_Manager<std::string, StatisticsTypeImpl*, ACE_Recursive_Thread_Mutex>
      statsTypeMap;

  StatisticsTypeImpl* addType(StatisticsTypeImpl* t);

 public:
  GeodeStatisticsFactory(StatisticsManager* statMngr);
  ~GeodeStatisticsFactory();

  const std::string& getName() const override;

  int64_t getId() const override;

  Statistics* createStatistics(StatisticsType* type) override;

  Statistics* createStatistics(StatisticsType* type,
                               const std::string& textId) override;

  Statistics* createStatistics(StatisticsType* type, const std::string& textId,
                               int64_t numericId) override;

  Statistics* createOsStatistics(StatisticsType* type,
                                 const std::string& textId, int64_t numericId);

  Statistics* createAtomicStatistics(StatisticsType* type) override;

  Statistics* createAtomicStatistics(StatisticsType* type,
                                     const std::string& textId) override;

  Statistics* createAtomicStatistics(StatisticsType* type,
                                     const std::string& textId,
                                     int64_t numericId) override;

  StatisticsType* createType(const std::string& name,
                             const std::string& description,
                             StatisticDescriptor** stats,
                             int32_t statsLength) override;

  StatisticsType* findType(const std::string& name) const override;

  StatisticDescriptor* createIntCounter(const std::string& name,
                                        const std::string& description,
                                        const std::string& units,
                                        bool largerBetter) override;

  StatisticDescriptor* createLongCounter(const std::string& name,
                                         const std::string& description,
                                         const std::string& units,
                                         bool largerBetter) override;

  StatisticDescriptor* createDoubleCounter(const std::string& name,
                                           const std::string& description,
                                           const std::string& units,
                                           bool largerBetter) override;

  StatisticDescriptor* createIntGauge(const std::string& name,
                                      const std::string& description,
                                      const std::string& units,
                                      bool largerBetter) override;

  StatisticDescriptor* createLongGauge(const std::string& name,
                                       const std::string& description,
                                       const std::string& units,
                                       bool largerBetter) override;

  StatisticDescriptor* createDoubleGauge(const std::string& name,
                                         const std::string& description,
                                         const std::string& units,
                                         bool largerBetter) override;

  Statistics* findFirstStatisticsByType(
      const StatisticsType* type) const override;
};

}  // namespace statistics
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STATISTICS_GEODESTATISTICSFACTORY_H_
