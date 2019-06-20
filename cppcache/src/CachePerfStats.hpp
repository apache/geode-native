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

#ifndef GEODE_CACHEPERFSTATS_H_
#define GEODE_CACHEPERFSTATS_H_

#include <geode/internal/geode_globals.hpp>

#include "statistics/Statistics.hpp"
#include "statistics/StatisticsFactory.hpp"
#include "statistics/StatisticsManager.hpp"

namespace apache {
namespace geode {
namespace client {

using statistics::StatisticDescriptor;
using statistics::Statistics;
using statistics::StatisticsFactory;

/** hold statistics for cache.. */
class APACHE_GEODE_EXPORT CachePerfStats {
 public:
  explicit CachePerfStats(StatisticsFactory* factory);

  virtual ~CachePerfStats();

  void close();

  void incDestroys();

  void incCreates();

  void incPuts();

  void incGets();

  void incHits();

  void incMisses();

  void incOverflows();

  void incRetrieves();

  void incEntries(int32_t delta);

  void incListenerCalls();

  void incDeltaPut();

  void incDeltaReceived();

  void incFailureOnDeltaReceived();

  void incTimeSpentOnDeltaApplication(int32_t time);

  void incTombstoneCount();
  void decTombstoneCount();
  void incTombstoneSize(int64_t size);
  void decTombstoneSize(int64_t size);
  void incConflatedEvents();
  int64_t getTombstoneSize();
  int32_t getTombstoneCount();
  int32_t getConflatedEvents();

  void incPdxInstanceDeserializations();

  Statistics* getStat();

  int32_t getPdxInstanceDeserializationTimeId();

  void incPdxInstanceCreations();

  int32_t getPdxInstanceDeserializations();

  int32_t getPdxInstanceDeserializationTime();

  int32_t getPdxInstanceCreations();

  void incPdxSerialization(int32_t bytes);

  int32_t getPdxSerializations();

  int64_t getPdxSerializationBytes();

  void incPdxDeSerialization(int32_t bytes);

  int32_t getPdxDeSerializations();

  int64_t getPdxDeSerializationBytes();

 private:
  Statistics* m_cachePerfStats;

  int32_t m_destroysId;
  int32_t m_createsId;
  int32_t m_putsId;
  int32_t m_getsId;
  int32_t m_hitsId;
  int32_t m_missesId;
  int32_t m_entriesId;
  int32_t m_overflowsId;
  int32_t m_retrievesId;
  int32_t m_numListeners;
  int32_t m_deltaPut;
  int32_t m_deltaReceived;
  int32_t m_deltaFailedOnReceive;
  int32_t m_processedDeltaMessagesTime;
  int32_t m_tombstoneCount;
  int32_t m_tombstoneSize;
  int32_t m_conflatedEvents;
  int32_t m_pdxInstanceDeserializationsId;
  int32_t m_pdxInstanceDeserializationTimeId;
  int32_t m_pdxInstanceCreationsId;
  int32_t m_pdxSerializationsId;
  int32_t m_pdxSerializedBytesId;
  int32_t m_pdxDeserializationsId;
  int32_t m_pdxDeserializedBytesId;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHEPERFSTATS_H_
