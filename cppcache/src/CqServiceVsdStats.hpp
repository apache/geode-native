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

#ifndef GEODE_CQSERVICEVSDSTATS_H_
#define GEODE_CQSERVICEVSDSTATS_H_

#include <string>

#include <geode/CqServiceStatistics.hpp>
#include <geode/internal/geode_globals.hpp>

#include "statistics/Statistics.hpp"
#include "statistics/StatisticsFactory.hpp"
#include "util/concurrent/spinlock_mutex.hpp"

namespace apache {
namespace geode {
namespace client {

using statistics::StatisticDescriptor;
using statistics::Statistics;
using statistics::StatisticsType;
using util::concurrent::spinlock_mutex;

class APACHE_GEODE_EXPORT CqServiceVsdStats : public CqServiceStatistics {
 public:
  /** hold statistics for a cq. */
  CqServiceVsdStats(statistics::StatisticsFactory* factory,
                    const std::string& cqName = "CqServiceVsdStats");

  /** disable stat collection for this item. */
  ~CqServiceVsdStats() override;

  void close();
  void decNumCqsActive();

  void incNumCqsActive() const;
  uint32_t numCqsActive() const override;

  void incNumCqsCreated();
  uint32_t numCqsCreated() const override;

  uint32_t numCqsOnClient() const override;

  void incNumCqsClosed();
  uint32_t numCqsClosed() const override;

  void incNumCqsStopped();
  void decNumCqsStopped();
  uint32_t numCqsStopped() const override;

  void setNumCqsActive(uint32_t value);

  void setNumCqsOnClient(uint32_t value);

  void setNumCqsClosed(uint32_t value);

  void setNumCqsStopped(uint32_t value);

 private:
  Statistics* m_cqServiceVsdStats;

  int32_t m_numCqsActiveId;
  int32_t m_numCqsCreatedId;
  int32_t m_numCqsOnClientId;
  int32_t m_numCqsClosedId;
  int32_t m_numCqsStoppedId;

  static constexpr const char* STATS_NAME = "CqServiceStatistics";
  static constexpr const char* STATS_DESC = "Statistics for this cq Service";
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CQSERVICEVSDSTATS_H_
