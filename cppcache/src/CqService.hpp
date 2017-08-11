#pragma once

#ifndef GEODE_CQSERVICE_H_
#define GEODE_CQSERVICE_H_

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
#include <geode/geode_types.hpp>
#include "TcrMessage.hpp"
#include "Queue.hpp"
#include <ace/ACE.h>
#include <ace/Condition_Recursive_Thread_Mutex.h>
#include <ace/Time_Value.h>
#include <ace/Guard_T.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Semaphore.h>
#include <geode/CacheableKey.hpp>
#include <geode/CqOperation.hpp>
#include <geode/CqQuery.hpp>
#include "MapWithLock.hpp"
#include <geode/DistributedSystem.hpp>
#include <map>
#include <string>
#include "Queue.hpp"
#include <ace/Task.h>
#include "ThinClientBaseDM.hpp"
#include "CqServiceVsdStats.hpp"

#include "NonCopyable.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

/**
 * @class CqService CqService.hpp
 *
 * Implements the CqService functionality.
 *
 */

class CPPCACHE_EXPORT CqService
    : private NonCopyable,
      private NonAssignable,
      public std::enable_shared_from_this<CqService> {
 private:
  ThinClientBaseDM* m_tccdm;
  statistics::StatisticsFactory* m_statisticsFactory;
  ACE_Recursive_Thread_Mutex m_mutex;
  std::string m_queryString;
  ACE_Semaphore m_notificationSema;

  bool m_running;
  MapOfCqQueryWithLock* m_cqQueryMap;

  CqServiceStatisticsPtr m_stats;

  inline bool noCq() const {
    MapOfRegionGuard guard(m_cqQueryMap->mutex());
    return (0 == m_cqQueryMap->current_size());
  }

 public:
  typedef std::vector<CqQueryPtr> query_container_type;

  /**
   * Constructor.
   */
  CqService(ThinClientBaseDM* tccdm, statistics::StatisticsFactory* statisticsFactory);
  ThinClientBaseDM* getDM() { return m_tccdm; }

  void receiveNotification(TcrMessage* msg);
  ~CqService();

  /**
   * Returns the state of the cqService.
   */
  bool checkAndAcquireLock();

  void updateStats();

  CqServiceVsdStats& getCqServiceVsdStats() {
    return *dynamic_cast<CqServiceVsdStats*>(m_stats.get());
  }

  /**
   * Constructs a new named continuous query, represented by an instance of
   * CqQuery. The CqQuery is not executed, however, until the execute method
   * is invoked on the CqQuery. The name of the query will be used
   * to identify this query in statistics archival.
   *
   * @param cqName the String name for this query
   * @param queryString the OQL query
   * @param cqAttributes the CqAttributes
   * @param isDurable true if the CQ is durable
   * @return the newly created CqQuery object
   * @throws CqExistsException if a CQ by this name already exists on this
   * client
   * @throws IllegalArgumentException if queryString or cqAttr is null
   * @throws IllegalStateException if this method is called from a cache
   *         server
   * @throws QueryInvalidException if there is a syntax error in the query
   * @throws CqException if failed to create cq, failure during creating
   *         managing cq metadata info.
   * @throws CqInvalidException if the query doesnot meet the CQ constraints.
   *   E.g.: Query string should refer only one region, join not supported.
   *         The query must be a SELECT statement.
   *         DISTINCT queries are not supported.
   *         Projections are not supported.
   *         Only one iterator in the FROM clause is supported, and it must be a
   * region path.
   *         Bind parameters in the query are not supported for the initial
   * release.
   *
   */
  CqQueryPtr newCq(const std::string& cqName, const std::string& queryString,
                   const CqAttributesPtr& cqAttributes,
                   const bool isDurable = false);

  /**
   * Adds the given CQ and cqQuery object into the CQ map.
   */
  void addCq(const std::string& cqName, CqQueryPtr& cq);

  /**
   * Removes given CQ from the cqMap..
   */
  void removeCq(const std::string& cqName);
  /**
   * Retrieve a CqQuery by name.
   * @return the CqQuery or null if not found
   */
  CqQueryPtr getCq(const std::string& cqName);

  /**
   * Clears the CQ Query Map.
   */
  void clearCqQueryMap();
  /**
   * Retrieve  all registered CQs
   */
  void getAllCqs(query_container_type& vec);
  /**
   * Executes all the cqs on this client.
   */
  void executeAllClientCqs(bool afterFailover = false);

  /**
   * Executes all CQs on the specified endpoint after failover.
   */
  GfErrType executeAllClientCqs(TcrEndpoint* endpoint);

  /**
   * Executes all the given cqs.
   */
  void executeCqs(query_container_type& cqs, bool afterFailover = false);

  /**
   * Executes all the given cqs on the specified endpoint after failover.
   */
  GfErrType executeCqs(query_container_type& cqs, TcrEndpoint* endpoint);

  /**
   * Stops all the cqs
   */
  void stopAllClientCqs();

  /**
   * Stops all the specified cqs.
   */
  void stopCqs(query_container_type& cqs);

  /**
   * Close all CQs executing in this client, and release resources
   * associated with executing CQs.
   * CqQuerys created by other client are unaffected.
   */
  void closeAllCqs();

  /**
   * Get statistics information for all CQs
   * @return the CqServiceStatistics
   */
  CqServiceStatisticsPtr getCqServiceStatistics();

  /**
   * Close the CQ Service after cleanup if any.
   *
   */
  void closeCqService();

  /**
   * Cleans up the CqService.
   */
  void cleanup();

  /*
   * Checks if CQ with the given name already exists.
   * @param cqName name of the CQ.
   * @return true if exists else false.
   */
  bool isCqExists(const std::string& cqName);
  /**
   * Invokes the CqListeners for the given CQs.
   * @param cqs list of cqs with the cq operation from the Server.
   * @param messageType base operation
   * @param key
   * @param value
   */
  void invokeCqListeners(const std::map<std::string, int>* cqs,
                         uint32_t messageType, CacheableKeyPtr key,
                         CacheablePtr value, CacheableBytesPtr deltaValue,
                         EventIdPtr eventId);
  /**
   * Returns the Operation for the given EnumListenerEvent type.
   * @param eventType
   * @return Operation
   */
  CqOperation::CqOperationType getOperation(int eventType);

  void closeCqs(query_container_type& cqs);

  /**
   * Gets all the durable CQs registered by this client.
   *
   * @return List of names of registered durable CQs, empty list if no durable
   * cqs.
   */
  CacheableArrayListPtr getAllDurableCqsFromServer();

  void invokeCqConnectedListeners(const std::string& poolName,
                                  const bool connected);
};

typedef std::shared_ptr<CqService> CqServicePtr;
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CQSERVICE_H_
