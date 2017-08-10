/*
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

#ifndef GEODE_POOLSTATISTICS_H_
#define GEODE_POOLSTATISTICS_H_

#include <string>

#include <geode/geode_globals.hpp>
#include <geode/statistics/Statistics.hpp>
#include <geode/statistics/StatisticsFactory.hpp>

#include "statistics/StatisticsManager.hpp"

namespace apache {
namespace geode {
namespace client {

using statistics::StatisticDescriptor;
using statistics::StatisticsType;
using statistics::Statistics;
using util::concurrent::spinlock_mutex;

class PoolStats {
 public:
  /** hold statistics for a pool.. */
  PoolStats(statistics::StatisticsFactory* factory, const std::string& poolName);

  /** disable stat collection for this item. */
  virtual ~PoolStats();

  void close() { getStats()->close(); }

  void setLocators(int32_t curVal) { getStats()->setInt(m_locatorsId, curVal); }

  void setServers(int32_t curVal) { getStats()->setInt(m_serversId, curVal); }

  void setSubsServers(int32_t curVal) {
    getStats()->setInt(m_subsServsId, curVal);
  }

  void incLoctorRequests() { getStats()->incLong(m_locReqsId, 1); }

  void incLoctorResposes() { getStats()->incLong(m_locRespsId, 1); }

  void setCurPoolConnections(int32_t curVal) {
    getStats()->setInt(m_poolConnsId, curVal);
  }

  void incPoolConnects() { getStats()->incInt(m_connectsId, 1); }

  void incPoolDisconnects() { getStats()->incInt(m_disconnectsId, 1); }

  void incPoolDisconnects(int numConn) {
    getStats()->incInt(m_disconnectsId, numConn);
  }

  void incMinPoolSizeConnects() { getStats()->incInt(m_minPoolConnectsId, 1); }

  void incLoadCondConnects() { getStats()->incInt(m_loadCondConnectsId, 1); }

  void incIdleDisconnects() { getStats()->incInt(m_idleDisconnectsId, 1); }

  void incLoadCondDisconnects() {
    getStats()->incInt(m_loadCondDisconnectsId, 1);
  }

  void setCurWaitingConnections(int32_t curVal) {
    getStats()->setInt(m_waitingConnectionsId, curVal);
  }

  void incWaitingConnections() { getStats()->incInt(m_totalWaitingConnsId, 1); }

  void setCurClientOps(int32_t curVal) {
    getStats()->setInt(m_curClientOpsId, curVal);
  }

  void incSucceedClientOps() { getStats()->incInt(m_clientOpsSuccessId, 1); }

  void incFailedClientOps() { getStats()->incInt(m_clientOpsFailedId, 1); }

  void incTimeoutClientOps() { getStats()->incInt(m_clientOpsTimeoutId, 1); }

  void incReceivedBytes(int64_t value) {  // counter
    getStats()->incLong(m_receivedBytesId, value);
  }

  void incMessageBeingReceived() {  // counter
    getStats()->incLong(m_messagesBeingReceivedId, 1);
  }

  void incProcessedDeltaMessages() {  // counter
    getStats()->incLong(m_processedDeltaMessagesId, 1);
  }
  void incDeltaMessageFailures() {  // counter
    getStats()->incLong(m_deltaMessageFailuresId, 1);
  }
  void incProcessedDeltaMessagesTime(int64_t value) {  // counter
    getStats()->incLong(m_processedDeltaMessagesTimeId, value);
  }

  void incTotalWaitingConnTime(int64_t value) {  // counter
    getStats()->incLong(m_totalWaitingConnTimeId, value);
  }
  void incClientOpsSuccessTime(int64_t value) {  // counter
    getStats()->incLong(m_clientOpsSuccessTimeId, value);
  }
  void incQueryExecutionId() {  // counter
    getStats()->incInt(m_queryExecutionsId, 1);
  }
  void incQueryExecutionTimeId(int64_t value) {  // counter
    getStats()->incLong(m_queryExecutionTimeId, value);
  }
  inline apache::geode::statistics::Statistics* getStats() {
    return m_poolStats;
  }
  inline int32_t getTotalWaitingConnTimeId() {
    return m_totalWaitingConnTimeId;
  }

  inline int32_t getQueryExecutionTimeId() { return m_queryExecutionTimeId; }

 private:
  // volatile apache::geode::statistics::Statistics* m_poolStats;
  apache::geode::statistics::Statistics* m_poolStats;

  int32_t m_locatorsId;
  int32_t m_serversId;
  int32_t m_subsServsId;
  int32_t m_locReqsId;
  int32_t m_locRespsId;
  int32_t m_poolConnsId;
  int32_t m_connectsId;
  int32_t m_disconnectsId;
  int32_t m_minPoolConnectsId;
  int32_t m_loadCondConnectsId;
  int32_t m_idleDisconnectsId;
  int32_t m_loadCondDisconnectsId;
  int32_t m_waitingConnectionsId;
  int32_t m_totalWaitingConnsId;
  int32_t m_totalWaitingConnTimeId;
  int32_t m_curClientOpsId;
  int32_t m_clientOpsSuccessId;
  int32_t m_clientOpsSuccessTimeId;
  int32_t m_clientOpsFailedId;
  int32_t m_clientOpsTimeoutId;
  int32_t m_receivedBytesId;
  int32_t m_messagesBeingReceivedId;
  int32_t m_processedDeltaMessagesId;
  int32_t m_deltaMessageFailuresId;
  int32_t m_processedDeltaMessagesTimeId;
  int32_t m_queryExecutionsId;
  int32_t m_queryExecutionTimeId;

  static constexpr const char* STATS_NAME = "PoolStatistics";
  static constexpr const char* STATS_DESC = "Statistics for this pool";
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_POOLSTATISTICS_H_
