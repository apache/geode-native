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

#ifndef GEODE_REGIONSTATS_H_
#define GEODE_REGIONSTATS_H_

#include <string>

#include <geode/geode_globals.hpp>
#include <geode/statistics/Statistics.hpp>
#include <geode/statistics/StatisticsFactory.hpp>

namespace apache {
namespace geode {
namespace client {

using statistics::StatisticDescriptor;
using statistics::StatisticsType;
using statistics::Statistics;

class CPPCACHE_EXPORT RegionStats {
 public:
  /** hold statistics for a region.. */
  RegionStats(statistics::StatisticsFactory* factory, const std::string& regionName);

  /** disable stat collection for this item. */
  virtual ~RegionStats();

  void close() { m_regionStats->close(); }

  inline void incDestroys() { m_regionStats->incInt(m_destroysId, 1); }

  inline void incCreates() { m_regionStats->incInt(m_createsId, 1); }

  inline void incPuts() { m_regionStats->incInt(m_putsId, 1); }

  inline void incGets() { m_regionStats->incInt(m_getsId, 1); }

  inline void incGetAll() { m_regionStats->incInt(m_getAllId, 1); }

  inline void incPutAll() { m_regionStats->incInt(m_putAllId, 1); }

  inline void incRemoveAll() { m_regionStats->incInt(m_removeAllId, 1); }

  inline void incHits() { m_regionStats->incInt(m_hitsId, 1); }

  inline void incMisses() { m_regionStats->incInt(m_missesId, 1); }

  inline void incOverflows() { m_regionStats->incInt(m_overflowsId, 1); }

  inline void incRetrieves() { m_regionStats->incInt(m_retrievesId, 1); }

  inline void incMetaDataRefreshCount() {
    m_regionStats->incInt(m_metaDataRefreshId, 1);
  }

  inline void setEntries(int32_t entries) {
    m_regionStats->setInt(m_entriesId, entries);
  }

  inline void incLoaderCallsCompleted() {
    m_regionStats->incInt(m_LoaderCallsCompletedId, 1);
  }

  inline void incWriterCallsCompleted() {
    m_regionStats->incInt(m_WriterCallsCompletedId, 1);
  }

  inline void incListenerCallsCompleted() {
    m_regionStats->incInt(m_ListenerCallsCompletedId, 1);
  }

  inline void incClears() { m_regionStats->incInt(m_clearsId, 1); }

  inline void updateGetTime() { m_regionStats->incInt(m_clearsId, 1); }

  inline apache::geode::statistics::Statistics* getStat() {
    return m_regionStats;
  }

  inline int32_t getGetTimeId() { return m_getTimeId; }

  inline int32_t getPutTimeId() { return m_putTimeId; }

  inline int32_t getGetAllTimeId() { return m_getAllTimeId; }

  inline int32_t getPutAllTimeId() { return m_putAllTimeId; }

  inline int32_t getRemoveAllTimeId() { return m_removeAllTimeId; }

  inline int32_t getLoaderCallTimeId() { return m_LoaderCallTimeId; }

  inline int32_t getWriterCallTimeId() { return m_WriterCallTimeId; }

  inline int32_t getListenerCallTimeId() { return m_ListenerCallTimeId; }

  inline int32_t getClearsId() { return m_clearsId; }

 private:
  apache::geode::statistics::Statistics* m_regionStats;

  int32_t m_destroysId;
  int32_t m_createsId;
  int32_t m_putsId;
  int32_t m_putTimeId;
  int32_t m_putAllId;
  int32_t m_putAllTimeId;
  int32_t m_removeAllId;
  int32_t m_removeAllTimeId;
  int32_t m_getsId;
  int32_t m_getTimeId;
  int32_t m_getAllId;
  int32_t m_getAllTimeId;
  int32_t m_hitsId;
  int32_t m_missesId;
  int32_t m_entriesId;
  int32_t m_overflowsId;
  int32_t m_retrievesId;
  int32_t m_metaDataRefreshId;
  int32_t m_LoaderCallsCompletedId;
  int32_t m_LoaderCallTimeId;
  int32_t m_WriterCallsCompletedId;
  int32_t m_WriterCallTimeId;
  int32_t m_ListenerCallsCompletedId;
  int32_t m_ListenerCallTimeId;
  int32_t m_clearsId;

  static constexpr const char* STATS_NAME = "RegionStatistics";
  static constexpr const char* STATS_DESC = "Statistics for this region";
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_REGIONSTATS_H_
