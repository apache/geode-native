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

#include "PoolStatistics.hpp"
//#include "StatisticsFactory.hpp"

#include <ace/Singleton.h>

#include <mutex>

#include "util/concurrent/spinlock_mutex.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace apache {
namespace geode {
namespace client {

using statistics::StatisticsFactory;
using statistics::StatisticsManager;
using util::concurrent::spinlock_mutex;

constexpr const char* PoolStats::STATS_NAME;
constexpr const char* PoolStats::STATS_DESC;

PoolStats::PoolStats(StatisticsFactory* factory, const std::string& poolName) {
  auto statsType = factory->findType(STATS_NAME);

  if (statsType == nullptr) {
    auto stats = new StatisticDescriptor*[27];

    stats[0] = factory->createIntGauge(
        "locators", "Current number of locators discovered", "locators");
    stats[1] = factory->createIntGauge(
        "servers", "Current number of servers discovered", "servers");
    stats[2] = factory->createIntGauge(
        "subscriptionServers",
        "Number of servers hosting this clients subscriptions", "servers");
    stats[3] = factory->createLongCounter(
        "locatorRequests",
        "Number of requests from this connection pool to a locator",
        "requests");
    stats[4] = factory->createLongCounter(
        "locatorResponses",
        "Number of responses from the locator to this connection pool",
        "responses");
    stats[5] = factory->createIntGauge(
        "poolConnections", "Current number of pool connections", "connections");
    stats[6] = factory->createIntCounter(
        "connects", "Total number of times a connection has been created.",
        "connects");
    stats[7] = factory->createIntCounter(
        "disconnects", "Total number of times a connection has been destroyed.",
        "disconnects");
    stats[8] = factory->createIntCounter(
        "minPoolSizeConnects",
        "Total number of connects done to maintain minimum pool size.",
        "connects");
    stats[9] = factory->createIntCounter(
        "loadConditioningConnects",
        "Total number of connects done due to load conditioning.", "connects");
    stats[10] = factory->createIntCounter(
        "idleDisconnects",
        "Total number of disconnects done due to idle expiration.",
        "disconnects");
    stats[11] = factory->createIntCounter(
        "loadConditioningDisconnects",
        "Total number of disconnects done due to load conditioning expiration.",
        "disconnects");
    stats[12] = factory->createIntGauge(
        "connectionWaitsInProgress",
        "Current number of threads waiting for a connection", "threads");
    stats[13] = factory->createIntCounter(
        "connectionWaits",
        "Total number of times a thread completed waiting for a connection (by "
        "timing out or by getting a connection).",
        "waits");
    stats[14] = factory->createLongCounter(
        "connectionWaitTime",
        "Total time (nanoseconds) spent waiting for a connection.",
        "nanoseconds");
    stats[15] = factory->createIntGauge(
        "clientOpsInProgress", "Current number of clientOps being executed",
        "clientOps");
    stats[16] = factory->createIntCounter(
        "clientOps", "Total number of clientOps completed successfully",
        "clientOps");
    stats[17] = factory->createLongCounter(
        "clientOpTime",
        "Total amount of time, in nanoseconds spent doing clientOps",
        "nanoseconds");
    stats[18] = factory->createIntCounter(
        "clientOpFailures",
        "Total number of clientOp attempts that have failed", "clientOps");
    stats[19] = factory->createIntCounter(
        "clientOpTimeouts",
        "Total number of clientOp attempts that have timed out", "clientOps");
    stats[20] = factory->createLongCounter(
        "receivedBytes", "Total number of bytes received from the server.",
        "bytes");
    stats[21] = factory->createLongCounter(
        "messagesBeingReceived",
        "Total number of message being received off the network.", "messages");
    stats[22] = factory->createLongCounter(
        "processedDeltaMessages",
        "Total number of delta message processed successfully", "messages");
    stats[23] = factory->createLongCounter(
        "deltaMessageFailures", "Total number of failures in processing delta",
        "messages");
    stats[24] = factory->createLongCounter(
        "processedDeltaMessagesTime", "Total time spent while processing Delta",
        "nanoseconds");
    stats[25] = factory->createIntCounter("queryExecutions",
                                          "Total number of queryExecutions",
                                          "queryExecutions");
    stats[26] = factory->createLongCounter(
        "queryExecutionTime",
        "Total time spent while processing queryExecution", "nanoseconds");

    statsType = factory->createType(STATS_NAME, STATS_DESC, stats, 27);
  }
  m_locatorsId = statsType->nameToId("locators");
  m_serversId = statsType->nameToId("servers");
  m_subsServsId = statsType->nameToId("subscriptionServers");
  m_locReqsId = statsType->nameToId("locatorRequests");
  m_locRespsId = statsType->nameToId("locatorResponses");
  m_poolConnsId = statsType->nameToId("poolConnections");
  m_connectsId = statsType->nameToId("connects");
  m_disconnectsId = statsType->nameToId("disconnects");
  m_minPoolConnectsId = statsType->nameToId("minPoolSizeConnects");
  m_loadCondConnectsId = statsType->nameToId("loadConditioningConnects");
  m_idleDisconnectsId = statsType->nameToId("idleDisconnects");
  m_loadCondDisconnectsId = statsType->nameToId("loadConditioningDisconnects");
  m_waitingConnectionsId = statsType->nameToId("connectionWaitsInProgress");
  m_totalWaitingConnsId = statsType->nameToId("connectionWaits");
  m_totalWaitingConnTimeId = statsType->nameToId("connectionWaitTime");
  m_curClientOpsId = statsType->nameToId("clientOpsInProgress");
  m_clientOpsSuccessId = statsType->nameToId("clientOps");
  m_clientOpsSuccessTimeId = statsType->nameToId("clientOpTime");
  m_clientOpsFailedId = statsType->nameToId("clientOpFailures");
  m_clientOpsTimeoutId = statsType->nameToId("clientOpTimeouts");
  m_receivedBytesId = statsType->nameToId("receivedBytes");
  m_messagesBeingReceivedId = statsType->nameToId("messagesBeingReceived");
  m_processedDeltaMessagesId = statsType->nameToId("processedDeltaMessages");
  m_deltaMessageFailuresId = statsType->nameToId("deltaMessageFailures");
  m_processedDeltaMessagesTimeId =
      statsType->nameToId("processedDeltaMessagesTime");
  m_queryExecutionsId = statsType->nameToId("queryExecutions");
  m_queryExecutionTimeId = statsType->nameToId("queryExecutionTime");

  m_poolStats = factory->createAtomicStatistics(statsType, poolName.c_str());

  getStats()->setInt(m_locatorsId, 0);
  getStats()->setInt(m_serversId, 0);
  getStats()->setInt(m_subsServsId, 0);
  getStats()->setLong(m_locReqsId, 0);
  getStats()->setLong(m_locRespsId, 0);
  getStats()->setInt(m_poolConnsId, 0);
  getStats()->setInt(m_connectsId, 0);
  getStats()->setInt(m_disconnectsId, 0);
  getStats()->setInt(m_minPoolConnectsId, 0);
  getStats()->setInt(m_loadCondConnectsId, 0);
  getStats()->setInt(m_idleDisconnectsId, 0);
  getStats()->setInt(m_loadCondDisconnectsId, 0);
  getStats()->setInt(m_waitingConnectionsId, 0);
  getStats()->setInt(m_totalWaitingConnsId, 0);
  getStats()->setLong(m_totalWaitingConnTimeId, 0);
  getStats()->setInt(m_curClientOpsId, 0);
  getStats()->setInt(m_clientOpsSuccessId, 0);
  getStats()->setLong(m_clientOpsSuccessTimeId, 0);
  getStats()->setInt(m_clientOpsFailedId, 0);
  getStats()->setInt(m_clientOpsTimeoutId, 0);
  getStats()->setInt(m_receivedBytesId, 0);
  getStats()->setInt(m_messagesBeingReceivedId, 0);
  getStats()->setInt(m_processedDeltaMessagesId, 0);
  getStats()->setInt(m_deltaMessageFailuresId, 0);
  getStats()->setInt(m_processedDeltaMessagesTimeId, 0);
  getStats()->setInt(m_queryExecutionsId, 0);
  getStats()->setLong(m_queryExecutionTimeId, 0);
}

PoolStats::~PoolStats() {
  if (m_poolStats != nullptr) {
    m_poolStats = nullptr;
  }
}
}  // namespace client
}  // namespace geode
}  // namespace apache
