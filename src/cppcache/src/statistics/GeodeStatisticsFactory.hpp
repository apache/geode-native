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

using namespace apache::geode::client;

/** @file
 */

namespace apache {
namespace geode {
namespace statistics {

class StatisticsManager;

/**
 * Geode's implementation of {@link StatisticsFactory}.
 *
 */
class GeodeStatisticsFactory : public StatisticsFactory {
 private:
  const char* m_name;

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

  const char* getName();

  int64_t getId();

  Statistics* createStatistics(StatisticsType* type);

  Statistics* createStatistics(StatisticsType* type, const char* textId);

  Statistics* createStatistics(StatisticsType* type, const char* textId,
                               int64_t numericId);

  Statistics* createOsStatistics(StatisticsType* type, const char* textId,
                                 int64_t numericId);

  Statistics* createAtomicStatistics(StatisticsType* type);

  Statistics* createAtomicStatistics(StatisticsType* type, const char* textId);

  Statistics* createAtomicStatistics(StatisticsType* type, const char* textId,
                                     int64_t numericId);

  StatisticsType* createType(const char* name, const char* description,
                             StatisticDescriptor** stats, int32_t statsLength);

  StatisticsType* findType(const char* name);

  StatisticDescriptor* createIntCounter(const char* name,
                                        const char* description,
                                        const char* units, bool largerBetter);

  StatisticDescriptor* createLongCounter(const char* name,
                                         const char* description,
                                         const char* units, bool largerBetter);

  StatisticDescriptor* createDoubleCounter(const char* name,
                                           const char* description,
                                           const char* units,
                                           bool largerBetter);

  StatisticDescriptor* createIntGauge(const char* name, const char* description,
                                      const char* units, bool largerBetter);

  StatisticDescriptor* createLongGauge(const char* name,
                                       const char* description,
                                       const char* units, bool largerBetter);

  StatisticDescriptor* createDoubleGauge(const char* name,
                                         const char* description,
                                         const char* units, bool largerBetter);

  /** Return the first instance that matches the type, or nullptr */
  Statistics* findFirstStatisticsByType(StatisticsType* type);
};

}  // namespace statistics
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STATISTICS_GEODESTATISTICSFACTORY_H_
