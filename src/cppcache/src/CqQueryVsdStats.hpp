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

#ifndef GEODE_CQQUERYVSDSTATS_H_
#define GEODE_CQQUERYVSDSTATS_H_

#include <string>

#include <geode/geode_globals.hpp>
#include <geode/statistics/Statistics.hpp>
#include <geode/statistics/StatisticsFactory.hpp>
#include <geode/CqStatistics.hpp>

#include "util/concurrent/spinlock_mutex.hpp"

namespace apache {
namespace geode {
namespace client {

using statistics::StatisticDescriptor;
using statistics::StatisticsType;
using statistics::Statistics;
using util::concurrent::spinlock_mutex;

class CPPCACHE_EXPORT CqQueryVsdStats : public CqStatistics {
 public:
  /** hold statistics for a cq. */
  CqQueryVsdStats(statistics::StatisticsFactory* factory, const std::string& cqqueryName);

  /** disable stat collection for this item. */
  virtual ~CqQueryVsdStats();

  void close() { m_cqQueryVsdStats->close(); }

  inline void incNumInserts() { m_cqQueryVsdStats->incInt(m_numInsertsId, 1); }

  inline void incNumUpdates() { m_cqQueryVsdStats->incInt(m_numUpdatesId, 1); }

  inline void incNumDeletes() { m_cqQueryVsdStats->incInt(m_numDeletesId, 1); }

  inline void incNumEvents() { m_cqQueryVsdStats->incInt(m_numEventsId, 1); }

  inline uint32_t numInserts() const {
    return m_cqQueryVsdStats->getInt(m_numInsertsId);
  }
  inline uint32_t numUpdates() const {
    return m_cqQueryVsdStats->getInt(m_numUpdatesId);
  }
  inline uint32_t numDeletes() const {
    return m_cqQueryVsdStats->getInt(m_numDeletesId);
  }
  inline uint32_t numEvents() const {
    return m_cqQueryVsdStats->getInt(m_numEventsId);
  }

 private:
  Statistics* m_cqQueryVsdStats;

  int32_t m_numInsertsId;
  int32_t m_numUpdatesId;
  int32_t m_numDeletesId;
  int32_t m_numEventsId;

  static constexpr const char* STATS_NAME = "CqQueryStatistics";
  static constexpr const char* STATS_DESC = "Statistics for this cq query";
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CQQUERYVSDSTATS_H_
