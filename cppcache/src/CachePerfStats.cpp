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

#include "CachePerfStats.hpp"

#include "statistics/StatisticDescriptor.hpp"

namespace apache {
namespace geode {
namespace client {
CachePerfStats::CachePerfStats(StatisticsFactory* factory) {
  auto statsType = factory->findType("CachePerfStats");

  if (statsType == nullptr) {
    const bool largerIsBetter = true;
    StatisticDescriptor** statDescArr = new StatisticDescriptor*[24];

    statDescArr[0] = factory->createIntCounter(
        "creates", "The total number of cache creates", "entries",
        largerIsBetter);
    statDescArr[1] = factory->createIntCounter(
        "puts", "The total number of cache puts", "entries", largerIsBetter);
    statDescArr[2] = factory->createIntCounter(
        "gets", "The total number of cache gets", "entries", largerIsBetter);
    statDescArr[3] = factory->createIntCounter(
        "hits", "The total number of cache hits", "entries", largerIsBetter);
    statDescArr[4] =
        factory->createIntCounter("misses", "The total number of cache misses",
                                  "entries", !largerIsBetter);
    statDescArr[5] = factory->createIntGauge(
        "entries", "The current number of cache entries", "entries",
        largerIsBetter);
    statDescArr[6] = factory->createIntCounter(
        "destroys", "The total number of cache destroys", "entries",
        largerIsBetter);
    statDescArr[7] = factory->createIntCounter(
        "overflows",
        "The total number of cache overflows to persistence backup", "entries",
        largerIsBetter);
    statDescArr[8] =
        factory->createIntCounter("retrieves",
                                  "The total number of cache entries fetched "
                                  "from persistence backup into the cache",
                                  "entries", largerIsBetter);
    statDescArr[9] = factory->createIntCounter(
        "cacheListenerCallsCompleted",
        "Total number of times a cache listener call has completed",
        "operations", largerIsBetter);
    statDescArr[10] =
        factory->createIntCounter("deltaPuts",
                                  "Total number of puts containing delta "
                                  "that have been sent from client to server",
                                  "entries", largerIsBetter);
    statDescArr[11] = factory->createIntCounter(
        "processedDeltaMessages",
        "Total number of messages containing delta received from server and "
        "processed after reception",
        "entries", largerIsBetter);
    statDescArr[12] = factory->createIntCounter(
        "deltaMessageFailures",
        "Total number of messages containing delta (received from server) "
        "but could not be processed after reception.",
        "entries", largerIsBetter);
    statDescArr[13] = factory->createLongCounter(
        "processedDeltaMessagesTime",
        "Total time spent applying delta (received from server) on existing "
        "values at client",
        "operations", largerIsBetter);
    statDescArr[14] = factory->createIntCounter(
        "tombstoneCount", "The total number of current tombstones", "entries",
        false);
    statDescArr[15] = factory->createLongCounter(
        "nonReplicatedTombstonesSize",
        "The total number of bytes consumed by tombstones for all regions in "
        "client processes",
        "bytes", false);
    statDescArr[16] = factory->createIntCounter(
        "conflatedEvents",
        "The number of conflicting events that have been elided and not "
        "passed on to event listeners",
        "operations", largerIsBetter);
    statDescArr[17] = factory->createIntCounter(
        "pdxInstanceDeserializations",
        "Total number of times getObject has been called on a PdxInstance.",
        "entries", largerIsBetter);
    statDescArr[18] =
        factory->createLongCounter("pdxInstanceDeserializationTime",
                                   "Total amount of time, in nanoseconds, "
                                   "spent deserializing PdxInstances by"
                                   "calling getObject.",
                                   "operations", !largerIsBetter);
    statDescArr[19] = factory->createIntCounter(
        "pdxInstanceCreations",
        "Total number of times a deserialization created a PdxInstance.",
        "entries", largerIsBetter);
    statDescArr[20] = factory->createIntCounter(
        "pdxSerializations", "Total number of pdx serializations.", "entries",
        largerIsBetter);
    statDescArr[21] = factory->createLongCounter(
        "pdxSerializedBytes",
        "Total number of bytes produced by pdx serialization.", "entries",
        !largerIsBetter);
    statDescArr[22] = factory->createIntCounter(
        "pdxDeserializations", "Total number of pdx deserializations.",
        "entries", largerIsBetter);
    statDescArr[23] = factory->createLongCounter(
        "pdxDeserializedBytes",
        "Total number of bytes read by pdx deserialization.", "entries",
        !largerIsBetter);

    statsType = factory->createType("CachePerfStats",
                                    "Statistics about native client cache",
                                    statDescArr, 24);
  }
  // Create Statistics object
  m_cachePerfStats =
      factory->createAtomicStatistics(statsType, "CachePerfStats");

  // get Id of Statistics Descriptors
  m_destroysId = statsType->nameToId("destroys");
  m_createsId = statsType->nameToId("creates");
  m_putsId = statsType->nameToId("puts");
  m_getsId = statsType->nameToId("gets");
  m_hitsId = statsType->nameToId("hits");
  m_missesId = statsType->nameToId("misses");
  m_entriesId = statsType->nameToId("entries");
  m_overflowsId = statsType->nameToId("overflows");
  m_retrievesId = statsType->nameToId("retrieves");
  m_numListeners = statsType->nameToId("cacheListenerCallsCompleted");
  m_deltaPut = statsType->nameToId("deltaPuts");
  m_deltaReceived = statsType->nameToId("processedDeltaMessages");
  m_deltaFailedOnReceive = statsType->nameToId("deltaMessageFailures");
  m_processedDeltaMessagesTime =
      statsType->nameToId("processedDeltaMessagesTime");
  m_tombstoneCount = statsType->nameToId("tombstoneCount");
  m_tombstoneSize = statsType->nameToId("nonReplicatedTombstonesSize");
  m_conflatedEvents = statsType->nameToId("conflatedEvents");
  m_pdxInstanceDeserializationsId =
      statsType->nameToId("pdxInstanceDeserializations");
  m_pdxInstanceDeserializationTimeId =
      statsType->nameToId("pdxInstanceDeserializationTime");
  m_pdxInstanceCreationsId = statsType->nameToId("pdxInstanceCreations");
  m_pdxSerializationsId = statsType->nameToId("pdxSerializations");
  m_pdxSerializedBytesId = statsType->nameToId("pdxSerializedBytes");
  m_pdxDeserializationsId = statsType->nameToId("pdxDeserializations");
  m_pdxDeserializedBytesId = statsType->nameToId("pdxDeserializedBytes");

  // Set initial value
  m_cachePerfStats->setInt(m_destroysId, 0);
  m_cachePerfStats->setInt(m_createsId, 0);
  m_cachePerfStats->setInt(m_putsId, 0);
  m_cachePerfStats->setInt(m_getsId, 0);
  m_cachePerfStats->setInt(m_hitsId, 0);
  m_cachePerfStats->setInt(m_missesId, 0);
  m_cachePerfStats->setInt(m_entriesId, 0);
  m_cachePerfStats->setInt(m_overflowsId, 0);
  m_cachePerfStats->setInt(m_retrievesId, 0);
  m_cachePerfStats->setInt(m_numListeners, 0);
  m_cachePerfStats->setInt(m_deltaPut, 0);
  m_cachePerfStats->setInt(m_deltaReceived, 0);
  m_cachePerfStats->setInt(m_deltaFailedOnReceive, 0);
  m_cachePerfStats->setLong(m_processedDeltaMessagesTime, 0);
  m_cachePerfStats->setInt(m_tombstoneCount, 0);
  m_cachePerfStats->setLong(m_tombstoneSize, 0);
  m_cachePerfStats->setInt(m_conflatedEvents, 0);
  m_cachePerfStats->setInt(m_pdxInstanceDeserializationsId, 0);
  m_cachePerfStats->setLong(m_pdxInstanceDeserializationTimeId, 0);
  m_cachePerfStats->setInt(m_pdxInstanceCreationsId, 0);
  m_cachePerfStats->setInt(m_pdxSerializationsId, 0);
  m_cachePerfStats->setLong(m_pdxSerializedBytesId, 0);
  m_cachePerfStats->setInt(m_pdxDeserializationsId, 0);
  m_cachePerfStats->setLong(m_pdxDeserializedBytesId, 0);
}

CachePerfStats::~CachePerfStats() { m_cachePerfStats = nullptr; }

void CachePerfStats::close() { m_cachePerfStats->close(); }

void CachePerfStats::incDestroys() {
  m_cachePerfStats->incInt(m_destroysId, 1);
}

void CachePerfStats::incCreates() { m_cachePerfStats->incInt(m_createsId, 1); }

void CachePerfStats::incPuts() { m_cachePerfStats->incInt(m_putsId, 1); }

void CachePerfStats::incGets() { m_cachePerfStats->incInt(m_getsId, 1); }

void CachePerfStats::incHits() { m_cachePerfStats->incInt(m_hitsId, 1); }

void CachePerfStats::incMisses() { m_cachePerfStats->incInt(m_missesId, 1); }

void CachePerfStats::incOverflows() {
  m_cachePerfStats->incInt(m_overflowsId, 1);
}

void CachePerfStats::incRetrieves() {
  m_cachePerfStats->incInt(m_retrievesId, 1);
}

void CachePerfStats::incEntries(int32_t delta) {
  m_cachePerfStats->incInt(m_entriesId, delta);
}

void CachePerfStats::incListenerCalls() {
  m_cachePerfStats->incInt(m_numListeners, 1);
}

void CachePerfStats::incDeltaPut() { m_cachePerfStats->incInt(m_deltaPut, 1); }

void CachePerfStats::incDeltaReceived() {
  m_cachePerfStats->incInt(m_deltaReceived, 1);
}

void CachePerfStats::incFailureOnDeltaReceived() {
  m_cachePerfStats->incInt(m_deltaFailedOnReceive, 1);
}

void CachePerfStats::incTimeSpentOnDeltaApplication(int32_t time) {
  m_cachePerfStats->incInt(m_processedDeltaMessagesTime, time);
}

void CachePerfStats::incTombstoneCount() {
  m_cachePerfStats->incInt(m_tombstoneCount, 1);
}
void CachePerfStats::decTombstoneCount() {
  m_cachePerfStats->incInt(m_tombstoneCount, -1);
}
void CachePerfStats::incTombstoneSize(int64_t size) {
  m_cachePerfStats->incLong(m_tombstoneSize, size);
}
void CachePerfStats::decTombstoneSize(int64_t size) {
  m_cachePerfStats->incLong(m_tombstoneSize, -size);
}
void CachePerfStats::incConflatedEvents() {
  m_cachePerfStats->incInt(m_conflatedEvents, 1);
}
int64_t CachePerfStats::getTombstoneSize() {
  return m_cachePerfStats->getLong(m_tombstoneSize);
}
int32_t CachePerfStats::getTombstoneCount() {
  return m_cachePerfStats->getInt(m_tombstoneCount);
}
int32_t CachePerfStats::getConflatedEvents() {
  return m_cachePerfStats->getInt(m_conflatedEvents);
}

void CachePerfStats::incPdxInstanceDeserializations() {
  m_cachePerfStats->incInt(m_pdxInstanceDeserializationsId, 1);
}

Statistics* CachePerfStats::getStat() { return m_cachePerfStats; }

int32_t CachePerfStats::getPdxInstanceDeserializationTimeId() {
  return m_pdxInstanceDeserializationTimeId;
}

void CachePerfStats::incPdxInstanceCreations() {
  m_cachePerfStats->incInt(m_pdxInstanceCreationsId, 1);
}

int32_t CachePerfStats::getPdxInstanceDeserializations() {
  return m_cachePerfStats->getInt(m_pdxInstanceDeserializationsId);
}

int32_t CachePerfStats::getPdxInstanceDeserializationTime() {
  return m_cachePerfStats->getInt(m_pdxInstanceDeserializationTimeId);
}

int32_t CachePerfStats::getPdxInstanceCreations() {
  return m_cachePerfStats->getInt(m_pdxInstanceCreationsId);
}

void CachePerfStats::incPdxSerialization(int32_t bytes) {
  m_cachePerfStats->incInt(m_pdxSerializationsId, 1);
  m_cachePerfStats->incLong(m_pdxSerializedBytesId, bytes);
}

int32_t CachePerfStats::getPdxSerializations() {
  return m_cachePerfStats->getInt(m_pdxSerializationsId);
}

int64_t CachePerfStats::getPdxSerializationBytes() {
  return m_cachePerfStats->getLong(m_pdxSerializedBytesId);
}

void CachePerfStats::incPdxDeSerialization(int32_t bytes) {
  m_cachePerfStats->incInt(m_pdxDeserializationsId, 1);
  m_cachePerfStats->incLong(m_pdxDeserializedBytesId, bytes);
}

int32_t CachePerfStats::getPdxDeSerializations() {
  return m_cachePerfStats->getInt(m_pdxDeserializationsId);
}

int64_t CachePerfStats::getPdxDeSerializationBytes() {
  return m_cachePerfStats->getLong(m_pdxDeserializedBytesId);
}
}  // namespace client
}  // namespace geode
}  // namespace apache
