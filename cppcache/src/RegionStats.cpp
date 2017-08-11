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

#include <geode/geode_globals.hpp>
#include <util/concurrent/spinlock_mutex.hpp>

#include "RegionStats.hpp"

namespace apache {
namespace geode {
namespace client {

using statistics::StatisticsFactory;
using util::concurrent::spinlock_mutex;

constexpr const char* RegionStats::STATS_NAME;
constexpr const char* RegionStats::STATS_DESC;

RegionStats::RegionStats(StatisticsFactory* factory,
                         const std::string& regionName) {
  auto statsType = factory->findType(STATS_NAME);

  if (!statsType) {
    const bool largerIsBetter = true;
    auto stats = new StatisticDescriptor*[25];
    stats[0] = factory->createIntCounter(
        "creates", "The total number of cache creates for this region",
        "entries", largerIsBetter);
    stats[1] = factory->createIntCounter(
        "puts", "The total number of cache puts for this region", "entries",
        largerIsBetter);
    stats[2] = factory->createIntCounter(
        "gets", "The total number of cache gets for this region", "entries",
        largerIsBetter);
    stats[3] = factory->createIntCounter(
        "hits", "The total number of cache hits for this region", "entries",
        largerIsBetter);
    stats[4] = factory->createIntCounter(
        "misses", "The total number of cache misses for this region", "entries",
        !largerIsBetter);
    stats[5] = factory->createIntGauge(
        "entries", "The current number of cache entries for this region",
        "entries", largerIsBetter);
    stats[6] = factory->createIntCounter(
        "destroys", "The total number of cache destroys for this region",
        "entries", largerIsBetter);
    stats[7] =
        factory->createIntCounter("overflows",
                                  "The total number of cache overflows for "
                                  "this region to persistence backup",
                                  "entries", largerIsBetter);
    stats[8] =
        factory->createIntCounter("retrieves",
                                  "The total number of cache entries fetched "
                                  "from persistence backup into the cache",
                                  "entries", largerIsBetter);
    stats[9] =
        factory->createIntCounter("metaDataRefreshCount",
                                  "The total number of times matadata is "
                                  "refreshed due to hoping observed",
                                  "entries", !largerIsBetter);
    stats[10] = factory->createIntCounter(
        "getAll", "The total number of cache getAll for this region", "entries",
        largerIsBetter);
    stats[11] = factory->createIntCounter(
        "putAll", "The total number of cache putAll for this region", "entries",
        largerIsBetter);
    stats[12] = factory->createLongCounter(
        "getTime", "Total time spent doing get operations for this region",
        "Nanoseconds", !largerIsBetter);
    stats[13] = factory->createLongCounter(
        "putTime", "Total time spent doing puts operations for this region",
        "Nanoseconds", !largerIsBetter);
    stats[14] = factory->createLongCounter(
        "putAllTime",
        "Total time spent doing putAlls operations for this region",
        "Nanoseconds", !largerIsBetter);
    stats[15] = factory->createLongCounter(
        "getAllTime",
        "Total time spent doing the getAlls operations for this region",
        "Nanoseconds", !largerIsBetter);

    stats[16] = factory->createIntCounter(
        "cacheLoaderCallsCompleted",
        "Total number of times a load has completed for this region", "entries",
        largerIsBetter);
    stats[17] = factory->createLongCounter(
        "cacheLoaderCallTIme",
        "Total time spent invoking the loaders for this region", "Nanoseconds",
        !largerIsBetter);
    stats[18] =
        factory->createIntCounter("cacheWriterCallsCompleted",
                                  "Total number of times a cache writer call "
                                  "has completed for this region",
                                  "entries", largerIsBetter);
    stats[19] = factory->createLongCounter(
        "cacheWriterCallTime", "Total time spent doing cache writer calls",
        "Nanoseconds", !largerIsBetter);
    stats[20] =
        factory->createIntCounter("cacheListenerCallsCompleted",
                                  "Total number of times a cache listener call "
                                  "has completed for this region",
                                  "entries", largerIsBetter);
    stats[21] = factory->createLongCounter(
        "cacheListenerCallTime",
        "Total time spent doing cache listener calls for this region",
        "Nanoseconds", !largerIsBetter);
    stats[22] =
        factory->createIntCounter("clears",
                                  "The total number of times a clear has been "
                                  "done on this cache for this region",
                                  "entries", !largerIsBetter);
    stats[23] = factory->createIntCounter(
        "removeAll", "The total number of cache removeAll for this region",
        "entries", largerIsBetter);
    stats[24] = factory->createLongCounter(
        "removeAllTime",
        "Total time spent doing removeAlls operations for this region",
        "Nanoseconds", !largerIsBetter);
    statsType = factory->createType(STATS_NAME, STATS_DESC, stats, 25);
  }

  m_destroysId = statsType->nameToId("destroys");
  m_createsId = statsType->nameToId("creates");
  m_putsId = statsType->nameToId("puts");
  m_putTimeId = statsType->nameToId("putTime");
  m_putAllId = statsType->nameToId("putAll");
  m_putAllTimeId = statsType->nameToId("putAllTime");
  m_removeAllId = statsType->nameToId("removeAll");
  m_removeAllTimeId = statsType->nameToId("removeAllTime");
  m_getsId = statsType->nameToId("gets");
  m_getTimeId = statsType->nameToId("getTime");
  m_getAllId = statsType->nameToId("getAll");
  m_getAllTimeId = statsType->nameToId("getAllTime");
  m_hitsId = statsType->nameToId("hits");
  m_missesId = statsType->nameToId("misses");
  m_entriesId = statsType->nameToId("entries");
  m_overflowsId = statsType->nameToId("overflows");
  m_retrievesId = statsType->nameToId("retrieves");
  m_metaDataRefreshId = statsType->nameToId("metaDataRefreshCount");
  m_LoaderCallsCompletedId = statsType->nameToId("cacheLoaderCallsCompleted");
  m_LoaderCallTimeId = statsType->nameToId("cacheLoaderCallTIme");
  m_WriterCallsCompletedId = statsType->nameToId("cacheWriterCallsCompleted");
  m_WriterCallTimeId = statsType->nameToId("cacheWriterCallTime");
  m_ListenerCallsCompletedId =
      statsType->nameToId("cacheListenerCallsCompleted");
  m_ListenerCallTimeId = statsType->nameToId("cacheListenerCallTime");
  m_clearsId = statsType->nameToId("clears");

  m_regionStats = factory->createAtomicStatistics(
      statsType, const_cast<char*>(regionName.c_str()));

  m_regionStats->setInt(m_destroysId, 0);
  m_regionStats->setInt(m_createsId, 0);
  m_regionStats->setInt(m_putsId, 0);
  m_regionStats->setInt(m_putTimeId, 0);
  m_regionStats->setInt(m_getsId, 0);
  m_regionStats->setInt(m_getTimeId, 0);
  m_regionStats->setInt(m_getAllId, 0);
  m_regionStats->setInt(m_getAllTimeId, 0);
  m_regionStats->setInt(m_putAllId, 0);
  m_regionStats->setInt(m_putAllTimeId, 0);
  m_regionStats->setInt(m_removeAllId, 0);
  m_regionStats->setInt(m_removeAllTimeId, 0);
  m_regionStats->setInt(m_hitsId, 0);
  m_regionStats->setInt(m_missesId, 0);
  m_regionStats->setInt(m_entriesId, 0);
  m_regionStats->setInt(m_overflowsId, 0);
  m_regionStats->setInt(m_retrievesId, 0);
  m_regionStats->setInt(m_metaDataRefreshId, 0);
  m_regionStats->setInt(m_LoaderCallsCompletedId, 0);
  m_regionStats->setInt(m_LoaderCallTimeId, 0);
  m_regionStats->setInt(m_WriterCallsCompletedId, 0);
  m_regionStats->setInt(m_WriterCallTimeId, 0);
  m_regionStats->setInt(m_ListenerCallsCompletedId, 0);
  m_regionStats->setInt(m_ListenerCallTimeId, 0);
  m_regionStats->setInt(m_clearsId, 0);
}

RegionStats::~RegionStats() {
  if (m_regionStats != nullptr) {
    // Don't Delete, Already closed, Just set nullptr
    // delete m_regionStats;
    m_regionStats = nullptr;
  }
}
}  // namespace client
}  // namespace geode
}  // namespace apache
