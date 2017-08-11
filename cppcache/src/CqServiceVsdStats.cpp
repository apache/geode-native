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

#include <ace/Thread_Mutex.h>
#include <ace/Singleton.h>
#include <mutex>
#include <geode/geode_globals.hpp>
#include <geode/statistics/StatisticsFactory.hpp>

#include "CqServiceVsdStats.hpp"

namespace apache {
namespace geode {
namespace client {

using statistics::StatisticsFactory;
using util::concurrent::spinlock_mutex;
using std::lock_guard;

constexpr const char* CqServiceVsdStats::STATS_NAME;
constexpr const char* CqServiceVsdStats::STATS_DESC;

CqServiceVsdStats::CqServiceVsdStats(StatisticsFactory* factory,
                                     const std::string& cqServiceName) {
  auto statsType = factory->findType(STATS_NAME);
  if (!statsType) {
    const bool largerIsBetter = true;
    auto stats = new StatisticDescriptor*[5];
    stats[0] = factory->createIntCounter(
        "CqsActive", "The total number of CqsActive this cq qurey", "entries",
        largerIsBetter);
    stats[1] = factory->createIntCounter(
        "CqsCreated", "The total number of CqsCreated for this cq Service",
        "entries", largerIsBetter);
    stats[2] = factory->createIntCounter(
        "CqsClosed", "The total number of CqsClosed for this cq Service",
        "entries", largerIsBetter);
    stats[3] = factory->createIntCounter(
        "CqsStopped", "The total number of CqsStopped for this cq Service",
        "entries", largerIsBetter);
    stats[4] = factory->createIntCounter(
        "CqsOnClient",
        "The total number of Cqs on the client for this cq Service", "entries",
        largerIsBetter);

    statsType = factory->createType(STATS_NAME, STATS_DESC, stats, 5);
  }

  m_cqServiceVsdStats =
      factory->createAtomicStatistics(statsType, cqServiceName.c_str());

  m_numCqsActiveId = statsType->nameToId("CqsActive");
  m_numCqsCreatedId = statsType->nameToId("CqsCreated");
  m_numCqsOnClientId = statsType->nameToId("CqsOnClient");
  m_numCqsClosedId = statsType->nameToId("CqsClosed");
  m_numCqsStoppedId = statsType->nameToId("CqsStopped");

  m_cqServiceVsdStats->setInt(m_numCqsActiveId, 0);
  m_cqServiceVsdStats->setInt(m_numCqsCreatedId, 0);
  m_cqServiceVsdStats->setInt(m_numCqsOnClientId, 0);
  m_cqServiceVsdStats->setInt(m_numCqsClosedId, 0);
  m_cqServiceVsdStats->setInt(m_numCqsStoppedId, 0);
}

CqServiceVsdStats::~CqServiceVsdStats() {
  if (m_cqServiceVsdStats != nullptr) {
    // Don't Delete, Already closed, Just set nullptr
    // delete m_CqServiceVsdStats;
    m_cqServiceVsdStats = nullptr;
  }
}
}  // namespace client
}  // namespace geode
}  // namespace apache
