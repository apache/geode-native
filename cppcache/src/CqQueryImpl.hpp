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

#ifndef GEODE_CQQUERYIMPL_H_
#define GEODE_CQQUERYIMPL_H_

#include <geode/internal/geode_globals.hpp>

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
#include <geode/AuthenticatedView.hpp>

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
class CqQueryImpl : public CqQuery,
                    public std::enable_shared_from_this<CqQueryImpl> {
 protected:
  std::string m_cqName;
  std::string m_queryString;
  std::shared_ptr<SelectResults> m_cqResults;

 private:
  std::shared_ptr<Query> m_query;
  std::shared_ptr<CqAttributes> m_cqAttributes;
  CqAttributesMutator m_cqAttributesMutator;
  std::shared_ptr<CqService> m_cqService;
  std::string m_serverCqName;
  bool m_isDurable;

  // Stats counters
  std::shared_ptr<CqStatistics> m_stats;
  CqState m_cqState;
  CqOperation m_cqOperation;

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
  CqQueryImpl(
      const std::shared_ptr<CqService>& cqService, const std::string& cqName,
      const std::string& queryString,
      const std::shared_ptr<CqAttributes>& cqAttributes,
      statistics::StatisticsFactory* factory, const bool isDurable = false,
      const std::shared_ptr<UserAttributes>& userAttributesPtr = nullptr);

  virtual ~CqQueryImpl();

  /**
   * returns CQ name
   */
  const std::string& getName() const override;

  /**
   * sets the CqName.
   */
  void setName(std::string cqName);

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
  void close() override;

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
  const std::string& getQueryString() const override;

  /**
   * Return the query after replacing region names with parameters
   * @return the Query for the query string
   */
  std::shared_ptr<Query> getQuery() const override;

  /**
   * @see org.apache.geode.cache.query.CqQuery#getStatistics()
   */
  std::shared_ptr<CqStatistics> getStatistics() const override;

  CqQueryVsdStats& getVsdStats() {
    return *dynamic_cast<CqQueryVsdStats*>(m_stats.get());
  }

  std::shared_ptr<CqAttributes> getCqAttributes() const override;

  std::shared_ptr<Region> getCqBaseRegion();

  /**
   * Clears the resource used by CQ.
   * @throws CqException
   */
  void cleanup();

  /**
   * @return Returns the cqListeners.
   */
  void getCqListeners(std::vector<std::shared_ptr<CqListener>>& cqListener);

  /**
   * Start or resume executing the query.
   */
  void execute() override;

  void executeAfterFailover();

  /**
   * Execute CQ on endpoint after failover
   */
  GfErrType execute(TcrEndpoint* endpoint);

  /**
   * Start or resume executing the query.
   * Gets or updates the CQ results and returns them.
   */
  std::shared_ptr<CqResults> executeWithInitialResults(
      std::chrono::milliseconds timeout) override;

  /**
   * This is called when the new server comes-up.
   * Executes the CQ on the given endpoint.
   * @param endpoint
   */
  bool executeCq(TcrMessage::MsgType requestType);

  /**
   * Stop or pause executing the query.
   */
  void stop() override;

  /**
   * Return the state of this query.
   * @return STOPPED RUNNING or CLOSED
   */
  CqState getState() override;

  /**
   * Sets the state of the cq.
   * Server side method. Called during cq registration time.
   */
  void setCqState(CqState state);

  CqAttributesMutator getCqAttributesMutator() const override;

  /**
   * @return Returns the cqOperation.
   */
  CqOperation getCqOperation() const;

  /**
   * @param cqOperation The cqOperation to set.
   */
  void setCqOperation(CqOperation cqOperation);

  /**
   * Update CQ stats
   * @param cqEvent object
   */
  void updateStats(CqEvent& cqEvent);

  /**
   * Return true if the CQ is in running state
   * @return true if running, false otherwise
   */
  bool isRunning() const override;

  /**
   * Return true if the CQ is in Sstopped state
   * @return true if stopped, false otherwise
   */
  bool isStopped() const override;

  /**
   * Return true if the CQ is closed
   * @return true if closed, false otherwise
   */
  bool isClosed() const override;

  /**
   * Return true if the CQ is durable
   * @return true if durable, false otherwise
   */
  bool isDurable() const override;

  inline ThinClientBaseDM* getDM() { return m_tccdm; }

 private:
  void updateStats();
  mutable ACE_Recursive_Thread_Mutex m_mutex;
  void sendStopOrClose(TcrMessage::MsgType requestType);
  ThinClientBaseDM* m_tccdm;
  AuthenticatedView* m_authenticatedView;

  _GEODE_FRIEND_STD_SHARED_PTR(CqQueryImpl)
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CQQUERYIMPL_H_
