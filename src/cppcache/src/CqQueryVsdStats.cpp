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

#include <geode/geode_globals.hpp>

#include "CqQueryVsdStats.hpp"
//#include "StatisticsFactory.hpp"

#include <ace/Singleton.h>

#include <mutex>

#include "util/concurrent/spinlock_mutex.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace apache {
namespace geode {
namespace client {

using statistics::StatisticsFactory;
using util::concurrent::spinlock_mutex;
using std::lock_guard;

////////////////////////////////////////////////////////////////////////////////

spinlock_mutex CqQueryStatType::m_statTypeLock;

StatisticsType* CqQueryStatType::getStatType() {
  const bool largerIsBetter = true;
  lock_guard<spinlock_mutex> guard(m_statTypeLock);
  StatisticsFactory* factory = StatisticsFactory::getExistingInstance();
  GF_D_ASSERT(!!factory);

  StatisticsType* statsType = factory->findType("CqQueryStatistics");

  if (statsType == nullptr) {
    m_stats[0] = factory->createIntCounter(
        "inserts", "The total number of inserts this cq qurey", "entries",
        largerIsBetter);
    m_stats[1] = factory->createIntCounter(
        "updates", "The total number of updates for this cq query", "entries",
        largerIsBetter);
    m_stats[2] = factory->createIntCounter(
        "deletes", "The total number of deletes for this cq query", "entries",
        largerIsBetter);
    m_stats[3] = factory->createIntCounter(
        "events", "The total number of events for this cq query", "entries",
        largerIsBetter);

    statsType = factory->createType(statsName, statsDesc, m_stats, 4);

    m_numInsertsId = statsType->nameToId("inserts");
    m_numUpdatesId = statsType->nameToId("updates");
    m_numDeletesId = statsType->nameToId("deletes");
    m_numEventsId = statsType->nameToId("events");
  }

  return statsType;
}

CqQueryStatType& CqQueryStatType::getInstance() {
  // C++11 initializes statics threads safe
  static CqQueryStatType instance;
  return instance;
}

CqQueryStatType::CqQueryStatType()
    : m_numInsertsId(0),
      m_numUpdatesId(0),
      m_numDeletesId(0),
      m_numEventsId(0) {
  memset(m_stats, 0, sizeof(m_stats));
}

////////////////////////////////////////////////////////////////////////////////

// typedef ACE_Singleton<CqQueryVsdStatsInit, ACE_Thread_Mutex>
// TheCqQueryVsdStatsInit;

////////////////////////////////////////////////////////////////////////////////

CqQueryVsdStats::CqQueryVsdStats(const char* cqqueryName) {
  auto& regStatType = CqQueryStatType::getInstance();

  StatisticsType* statsType = regStatType.getStatType();

  GF_D_ASSERT(statsType != nullptr);

  StatisticsFactory* factory = StatisticsFactory::getExistingInstance();

  m_cqQueryVsdStats = factory->createAtomicStatistics(
      statsType, const_cast<char*>(cqqueryName));

  m_numInsertsId = regStatType.getNumInsertsId();
  m_numUpdatesId = regStatType.getNumUpdatesId();
  m_numDeletesId = regStatType.getNumDeletesId();
  m_numEventsId = regStatType.getNumEventsId();

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
