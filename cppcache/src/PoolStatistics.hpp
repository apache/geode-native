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

#include <geode/internal/geode_globals.hpp>

#include "statistics/Statistics.hpp"
#include "statistics/StatisticsFactory.hpp"
#include "statistics/StatisticsManager.hpp"
#include "util/concurrent/spinlock_mutex.hpp"

namespace apache {
namespace geode {
namespace client {

using statistics::StatisticDescriptor;
using statistics::Statistics;
using statistics::StatisticsType;
using util::concurrent::spinlock_mutex;

class PoolStats {
 public:
  /** hold statistics for a pool.. */
  PoolStats(statistics::StatisticsFactory* factory,
            const std::string& poolName);

  /** disable stat collection for this item. */
  virtual ~PoolStats();

  void close();

  void setLocators(int32_t curVal);

  void setServers(int32_t curVal);

  void setSubsServers(int32_t curVal);

  void incLoctorRequests();

  void incLoctorResposes();

  void setCurPoolConnections(int32_t curVal);

  void incPoolConnects();

  void incPoolDisconnects();

  void incPoolDisconnects(int numConn);

  void incMinPoolSizeConnects();

  void incLoadCondConnects();

  void incIdleDisconnects();

  void incLoadCondDisconnects();

  void setCurWaitingConnections(int32_t curVal);

  void incWaitingConnections();

  void setCurClientOps(int32_t curVal);

  void incSucceedClientOps();

  void incFailedClientOps();

  void incTimeoutClientOps();

  void incReceivedBytes(int64_t value);

  void incMessageBeingReceived();

  void incProcessedDeltaMessages();
  void incDeltaMessageFailures();
  void incProcessedDeltaMessagesTime(int64_t value);

  void incTotalWaitingConnTime(int64_t value);
  void incClientOpsSuccessTime(int64_t value);
  void incQueryExecutionId();
  void incQueryExecutionTimeId(int64_t value);
  apache::geode::statistics::Statistics* getStats();
  int32_t getTotalWaitingConnTimeId();

  int32_t getQueryExecutionTimeId();

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
