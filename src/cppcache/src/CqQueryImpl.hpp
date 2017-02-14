#pragma once

#ifndef GEODE_CQQUERYIMPL_H_
#define GEODE_CQQUERYIMPL_H_

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
#include <geode/gf_types.hpp>

#include <geode/CqResults.hpp>
#include <geode/CqQuery.hpp>
#include <geode/CqState.hpp>
#include "CqQueryVsdStats.hpp"
#include "CqService.hpp"
#include <geode/CqOperation.hpp>
#include <geode/CqAttributes.hpp>
#include <geode/Region.hpp>
#include "MapWithLock.hpp"
#include <string>
#include <ace/ACE.h>
#include <ace/Condition_Recursive_Thread_Mutex.h>
#include <ace/Time_Value.h>
#include <ace/Guard_T.h>
#include <ace/Recursive_Thread_Mutex.h>
#include "ProxyCache.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

/**
 * @class CqQueryImpl CqQueryImpl.hpp
 *
 * Represents the CqQuery object. Implements CqQuery API and CqAttributeMutator.
 *
 */
class CqQueryImpl : public CqQuery {
 protected:
  std::string m_cqName;
  std::string m_queryString;
  SelectResultsPtr m_cqResults;

 private:
  QueryPtr m_query;
  CqAttributesPtr m_cqAttributes;
  CqAttributesMutatorPtr m_cqAttributesMutator;
  CqServicePtr m_cqService;
  std::string m_serverCqName;
  bool m_isDurable;

  // Stats counters
  CqStatisticsPtr m_stats;
  CqState::StateType m_cqState;
  CqOperation::CqOperationType m_cqOperation;

  /* CQ Request Type - Start */
  //  unused
  /*
  enum {
     EXECUTE_REQUEST = 0,
     EXECUTE_INITIAL_RESULTS_REQUEST = 1,
     STOP_REQUEST = 2,
     CLOSE_REQUEST = 3,
     REDUNDANT_EXECUTE_REQUEST = 4
  } CqRequestType;
  */

  /* CQ Request type - End */

  /**
   * Constructor.
   */
 public:
  CqQueryImpl(CqServicePtr& cqService, std::string& cqName,
              std::string& queryString, CqAttributesPtr& cqAttributes,
              bool isDurable = false,
              UserAttributesPtr userAttributesPtr = NULLPTR);

  ~CqQueryImpl();

  /**
   * returns CQ name
   */
  const char* getName() const;

  /**
   * sets the CqName.
   */
  void setName(std::string& cqName);

  /**
   * Initializes the CqQuery.
   * creates Query object, if its valid adds into repository.
   */
  void initCq();

  /**
   * Closes the Query.
   *        On Client side, sends the cq close request to server.
   *        On Server side, takes care of repository cleanup.
   * @throws CqException
   */
  void close();

  /**
   * Closes the Query.
   *        On Client side, sends the cq close request to server.
   *        On Server side, takes care of repository cleanup.
   * @param sendRequestToServer true to send the request to server.
   * @throws CqException
   */
  void close(bool sendRequestToServer);

  /**
   * Store this CQ in the cqService's cqMap.
   * @throws CqException
   */
  void addToCqMap();

  /**
   * Removes the CQ from CQ repository.
   * @throws CqException
   */
  void removeFromCqMap();

  /**
   * Returns the QueryString of this CQ.
   */
  const char* getQueryString() const;

  /**
   * Return the query after replacing region names with parameters
   * @return the Query for the query string
   */
  QueryPtr getQuery() const;

  /**
   * @see org.apache.geode.cache.query.CqQuery#getStatistics()
   */
  const CqStatisticsPtr getStatistics() const;

  CqQueryVsdStats& getVsdStats() {
    return *dynamic_cast<CqQueryVsdStats*>(m_stats.ptr());
  }

  const CqAttributesPtr getCqAttributes() const;

  RegionPtr getCqBaseRegion();

  /**
   * Clears the resource used by CQ.
   * @throws CqException
   */
  void cleanup();

  /**
   * @return Returns the cqListeners.
   */
  void getCqListeners(VectorOfCqListener& cqListener);

  /**
   * Start or resume executing the query.
   */
  void execute();

  void executeAfterFailover();

  /**
   * Execute CQ on endpoint after failover
   */
  GfErrType execute(TcrEndpoint* endpoint);

  /**
   * Start or resume executing the query.
   * Gets or updates the CQ results and returns them.
   */
  CqResultsPtr executeWithInitialResults(uint32_t timeout);

  /**
   * This is called when the new server comes-up.
   * Executes the CQ on the given endpoint.
   * @param endpoint
   */
  bool executeCq(TcrMessage::MsgType requestType);

  /**
   * Stop or pause executing the query.
   */
  void stop();

  /**
   * Return the state of this query.
   * @return STOPPED RUNNING or CLOSED
   */
  CqState::StateType getState();

  /**
   * Sets the state of the cq.
   * Server side method. Called during cq registration time.
   */
  void setCqState(CqState::StateType state);

  const CqAttributesMutatorPtr getCqAttributesMutator() const;

  /**
   * @return Returns the cqOperation.
   */
  CqOperation::CqOperationType getCqOperation();

  /**
   * @param cqOperation The cqOperation to set.
   */
  void setCqOperation(CqOperation::CqOperationType cqOperation);

  /**
   * Update CQ stats
   * @param cqEvent object
   */
  void updateStats(CqEvent& cqEvent);

  /**
   * Return true if the CQ is in running state
   * @return true if running, false otherwise
   */
  bool isRunning();

  /**
   * Return true if the CQ is in Sstopped state
   * @return true if stopped, false otherwise
   */
  bool isStopped();

  /**
   * Return true if the CQ is closed
   * @return true if closed, false otherwise
   */
  bool isClosed();

  /**
   * Return true if the CQ is durable
   * @return true if durable, false otherwise
   */
  bool isDurable();

  inline ThinClientBaseDM* getDM() { return m_tccdm; }

 private:
  void updateStats();
  ACE_Recursive_Thread_Mutex m_mutex;
  void sendStopOrClose(TcrMessage::MsgType requestType);
  ThinClientBaseDM* m_tccdm;
  ProxyCachePtr m_proxyCache;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CQQUERYIMPL_H_
