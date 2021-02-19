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

namespace apache {
namespace geode {
namespace client {

using statistics::StatisticDescriptor;
using statistics::Statistics;
using statistics::StatisticsType;

class CqServiceVsdStats : public CqServiceStatistics {
 public:
  /** hold statistics for a cq. */
  explicit CqServiceVsdStats(statistics::StatisticsFactory* factory,
                             const std::string& cqName = "CqServiceVsdStats");

  /** disable stat collection for this item. */
  ~CqServiceVsdStats() noexcept override;

  void close() { m_cqServiceVsdStats->close(); }

  inline void incNumCqsActive() const {
    m_cqServiceVsdStats->incInt(m_numCqsActiveId, 1);
  }
  inline uint32_t numCqsActive() const override {
    return m_cqServiceVsdStats->getInt(m_numCqsActiveId);
  }

  inline void incNumCqsCreated() {
    m_cqServiceVsdStats->incInt(m_numCqsCreatedId, 1);
  }
  inline uint32_t numCqsCreated() const override {
    return m_cqServiceVsdStats->getInt(m_numCqsCreatedId);
  }

  inline uint32_t numCqsOnClient() const override {
    return m_cqServiceVsdStats->getInt(m_numCqsOnClientId);
  }

  inline void incNumCqsClosed() {
    m_cqServiceVsdStats->incInt(m_numCqsClosedId, 1);
  }
  inline uint32_t numCqsClosed() const override {
    return m_cqServiceVsdStats->getInt(m_numCqsClosedId);
  }

  inline void incNumCqsStopped() {
    m_cqServiceVsdStats->incInt(m_numCqsStoppedId, 1);
  }

  inline uint32_t numCqsStopped() const override {
    return m_cqServiceVsdStats->getInt(m_numCqsStoppedId);
  }

  inline void setNumCqsActive(uint32_t value) {
    m_cqServiceVsdStats->setInt(m_numCqsActiveId, value);
  }

  inline void setNumCqsOnClient(uint32_t value) {
    m_cqServiceVsdStats->setInt(m_numCqsOnClientId, value);
  }

  inline void setNumCqsStopped(uint32_t value) {
    m_cqServiceVsdStats->setInt(m_numCqsStoppedId, value);
  }

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
