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

#include <geode/CqStatistics.hpp>
#include <geode/internal/geode_globals.hpp>

#include "statistics/Statistics.hpp"
#include "statistics/StatisticsFactory.hpp"

namespace apache {
namespace geode {
namespace client {

using statistics::StatisticDescriptor;
using statistics::Statistics;
using statistics::StatisticsType;

class CqQueryVsdStats : public CqStatistics {
 public:
  /** hold statistics for a cq. */
  CqQueryVsdStats(statistics::StatisticsFactory* factory,
                  const std::string& cqqueryName);

  /** disable stat collection for this item. */
  virtual ~CqQueryVsdStats() noexcept;

  void close() { m_cqQueryVsdStats->close(); }

  inline void incNumInserts() { m_cqQueryVsdStats->incInt(m_numInsertsId, 1); }

  inline void incNumUpdates() { m_cqQueryVsdStats->incInt(m_numUpdatesId, 1); }

  inline void incNumDeletes() { m_cqQueryVsdStats->incInt(m_numDeletesId, 1); }

  inline void incNumEvents() { m_cqQueryVsdStats->incInt(m_numEventsId, 1); }

  uint32_t numInserts() const override {
    return m_cqQueryVsdStats->getInt(m_numInsertsId);
  }
  uint32_t numUpdates() const override {
    return m_cqQueryVsdStats->getInt(m_numUpdatesId);
  }
  uint32_t numDeletes() const override {
    return m_cqQueryVsdStats->getInt(m_numDeletesId);
  }
  uint32_t numEvents() const override {
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
