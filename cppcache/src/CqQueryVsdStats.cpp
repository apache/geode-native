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


#include <mutex>

#include "util/concurrent/spinlock_mutex.hpp"

const char* cqStatsName = "CqQueryStatistics";
const char* cqStatsDesc = "Statistics for this cq query";
#include <ace/Thread_Mutex.h>
#include <ace/Singleton.h>

#include <geode/geode_globals.hpp>

#include "CqQueryVsdStats.hpp"

namespace apache {
namespace geode {
namespace client {

using statistics::StatisticsFactory;
using util::concurrent::spinlock_mutex;
using std::lock_guard;

constexpr const char* CqQueryVsdStats::STATS_NAME;
constexpr const char* CqQueryVsdStats::STATS_DESC;

CqQueryVsdStats::CqQueryVsdStats(StatisticsFactory* factory,
                                 const std::string& cqqueryName) {
  auto statsType = factory->findType(STATS_NAME);
  if (!statsType) {
    const bool largerIsBetter = true;
    auto stats = new StatisticDescriptor*[4];
    stats[0] = factory->createIntCounter(
        "inserts", "The total number of inserts this cq qurey", "entries",
        largerIsBetter);
    stats[1] = factory->createIntCounter(
        "updates", "The total number of updates for this cq query", "entries",
        largerIsBetter);
    stats[2] = factory->createIntCounter(
        "deletes", "The total number of deletes for this cq query", "entries",
        largerIsBetter);
    stats[3] = factory->createIntCounter(
        "events", "The total number of events for this cq query", "entries",
        largerIsBetter);

    statsType = factory->createType(STATS_NAME, STATS_DESC, stats, 4);
  }

  m_cqQueryVsdStats =
      factory->createAtomicStatistics(statsType, cqqueryName.c_str());

  m_numInsertsId = statsType->nameToId("inserts");
  m_numUpdatesId = statsType->nameToId("updates");
  m_numDeletesId = statsType->nameToId("deletes");
  m_numEventsId = statsType->nameToId("events");

  m_cqQueryVsdStats->setInt(m_numInsertsId, 0);
  m_cqQueryVsdStats->setInt(m_numUpdatesId, 0);
  m_cqQueryVsdStats->setInt(m_numDeletesId, 0);
  m_cqQueryVsdStats->setInt(m_numEventsId, 0);
}

CqQueryVsdStats::~CqQueryVsdStats() {
  if (m_cqQueryVsdStats != nullptr) {
    // Don't Delete, Already closed, Just set nullptr
    // delete m_CqQueryVsdStats;
    m_cqQueryVsdStats = nullptr;
  }
}
}  // namespace client
}  // namespace geode
}  // namespace apache
