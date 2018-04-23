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

#include <geode/CqAttributesFactory.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/CqAttributesMutator.hpp>

#include "CqQueryImpl.hpp"
#include "ResultSetImpl.hpp"
#include "StructSetImpl.hpp"
#include "ThinClientRegion.hpp"
#include "ReadWriteLock.hpp"
#include "ThinClientRegion.hpp"
#include "UserAttributes.hpp"
#include "util/bounds.hpp"
#include "util/Log.hpp"
#include "util/exception.hpp"

using namespace apache::geode::client;

CqQueryImpl::CqQueryImpl(
    const std::shared_ptr<CqService>& cqService, const std::string& cqName,
    const std::string& queryString,
    const std::shared_ptr<CqAttributes>& cqAttributes,
    StatisticsFactory* factory, const bool isDurable,
    const std::shared_ptr<UserAttributes>& userAttributesPtr)
    : m_cqName(cqName),
      m_queryString(queryString),
      m_cqAttributes(CqAttributesFactory(cqAttributes).create()),
      m_cqAttributesMutator(m_cqAttributes),
      m_cqService(cqService),
      // On Client Side serverCqName and cqName will be same.
      m_serverCqName(cqName),
      m_isDurable(isDurable),
      m_stats(std::make_shared<CqQueryVsdStats>(factory, m_cqName)),
      m_cqState(CqState::STOPPED),  // Initial state is stopped
      m_cqOperation(CqOperation::OP_TYPE_INVALID),
      m_tccdm(m_cqService->getDM()) {
  if (userAttributesPtr != nullptr) {
    m_authenticatedView = userAttributesPtr->getAuthenticatedView();
  } else {
    m_authenticatedView = nullptr;
  }
}

CqQueryImpl::~CqQueryImpl() {}
/**
 * returns CQ name
 */
void CqQueryImpl::updateStats() { m_cqService->updateStats(); }

const std::string& CqQueryImpl::getName() const { return m_cqName; }

/**
 * sets the CqName.
 */
void CqQueryImpl::setName(std::string cqName) {
  m_cqName = m_serverCqName = cqName;
}

/**
 * Initializes the CqQuery.
 * creates Query object, if its valid adds into repository.
 */
void CqQueryImpl::initCq() {
  addToCqMap();

  // Initialize the VSD statistics

  // Update statistics with CQ creation.
  auto& stats = m_cqService->getCqServiceVsdStats();
  // stats.incNumCqsStopped();
  stats.incNumCqsCreated();
  // stats.incNumCqsOnClient();
  updateStats();
}

/**
 * Closes the Query.
 *        On Client side, sends the cq close request to server.
 *        On Server side, takes care of repository cleanup.
 * @throws CqException
 */
void CqQueryImpl::close() { close(true); }

/**
 * Closes the Query.
 *        On Client side, sends the cq close request to server.
 * @param sendRequestToServer true to send the request to server.
 * @throws CqException
 */
void CqQueryImpl::close(bool sendRequestToServer) {
  // Check if the cq is already closed.
  if (isClosed()) {
    // throw CqClosedException("CQ is already closed, CqName : " + this.cqName);
    LOGFINE("CQ is already closed, CqName : %s", m_cqName.c_str());
    return;
  }

  GuardUserAttributes gua;
  if (m_authenticatedView != nullptr) {
    gua.setAuthenticatedView(m_authenticatedView);
  }
  LOGFINE("Started closing CQ CqName : %s", m_cqName.c_str());

  // bool isClosed = false;

  // Stat update.
  auto& stats = m_cqService->getCqServiceVsdStats();
  /*
  if (isRunning()) {
      stats.decNumCqsActive();
  }
  else if (isStopped()) {
      stats.decNumCqsStopped();
  }
  */
  setCqState(CqState::CLOSING);
  if (sendRequestToServer == true) {
    try {
      sendStopOrClose(TcrMessage::CLOSECQ_MSG_TYPE);
    } catch (...) {
      // ignore and move on
    }
  }

  // Set the state to close, and update stats
  setCqState(CqState::CLOSED);
  stats.incNumCqsClosed();

  // Invoke close on Listeners if any.
  if (m_cqAttributes) {
    auto cqListeners = m_cqAttributes->getCqListeners();

    if (!cqListeners.empty()) {
      LOGFINE(
          "Invoking CqListeners close() api for the CQ, CqName : %s  Number of "
          "CqListeners : %d",
          m_cqName.c_str(), cqListeners.size());

      for (auto& l : cqListeners) {
        try {
          l->close();
          // Handle client side exceptions.
        } catch (Exception& ex) {
          LOGWARN(
              "Exception occoured in the CqListener of the CQ, CqName : "
              "%sError : %s",
              m_cqName.c_str(), ex.what());
        }
      }
    }
  }

  removeFromCqMap();
  updateStats();
  LOGFINE("Successfully closed the CQ. %s", m_cqName.c_str());
}

/**
 * Store this CQ in the cqService's cqMap.
 * @throws CqException
 */
void CqQueryImpl::addToCqMap() {
  // Add CQ to the CQ repository
  try {
    LOGFINE("Adding to CQ Repository. CqName : %s Server CqName : %s",
            m_cqName.c_str(), m_serverCqName.c_str());
    std::shared_ptr<CqQuery> cq = shared_from_this();
    m_cqService->addCq(m_cqName, cq);
  } catch (Exception& ex) {
    std::string errMsg =
        "Failed to store Continuous Query in the repository. CqName: " +
        m_cqName + ex.what();
    LOGERROR(errMsg.c_str());
    throw CqException(errMsg.c_str());
  }
  LOGFINE("Stored CQ in the CQ repository. %s", m_cqName.c_str());
}

/**
 * Removes the CQ from CQ repository.
 * @throws CqException
 */
void CqQueryImpl::removeFromCqMap() {
  try {
    m_cqService->removeCq(m_cqName);
  } catch (Exception& ex) {
    std::string errMsg =
        "Failed to remove Continuous Query From the repository. CqName: " +
        m_cqName + " Error : " + ex.what();
    LOGERROR(errMsg.c_str());
    throw CqException(errMsg.c_str());
  }
  LOGFINE("Removed CQ from the CQ repository. CQ Name: %s", m_cqName.c_str());
}

/**
 * Returns the QueryString of this CQ.
 */
const std::string& CqQueryImpl::getQueryString() const { return m_queryString; }

/**
 * Return the query
 * @return the Query for the query string
 */
std::shared_ptr<Query> CqQueryImpl::getQuery() const { return m_query; }

/**
 * @see org.apache.geode.cache.query.CqQuery#getStatistics()
 */
std::shared_ptr<CqStatistics> CqQueryImpl::getStatistics() const {
  return m_stats;
}

std::shared_ptr<CqAttributes> CqQueryImpl::getCqAttributes() const {
  return m_cqAttributes;
}

/**
 * Clears the resource used by CQ.
 * @throws CqException
 */
void CqQueryImpl::cleanup() { removeFromCqMap(); }

/**
 * @return Returns the cqListeners.
 */
void CqQueryImpl::getCqListeners(
    CqAttributes::listener_container_type& cqListener) {
  cqListener = m_cqAttributes->getCqListeners();
}

GfErrType CqQueryImpl::execute(TcrEndpoint* endpoint) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_mutex);
  if (m_cqState != CqState::RUNNING) {
    return GF_NOERR;
  }

  GuardUserAttributes gua;
  if (m_authenticatedView != nullptr) {
    gua.setAuthenticatedView(m_authenticatedView);
  }

  LOGFINE("Executing CQ [%s]", m_cqName.c_str());

  TcrMessageExecuteCq request(m_cqService->getDM()
                                  ->getConnectionManager()
                                  .getCacheImpl()
                                  ->getCache()
                                  ->createDataOutput(),
                              m_cqName, m_queryString, CqState::RUNNING,
                              isDurable(), m_tccdm);
  TcrMessageReply reply(true, m_tccdm);

  GfErrType err = GF_NOERR;

  err = m_tccdm->sendRequestToEP(request, reply, endpoint);

  if (err != GF_NOERR) {
    // GfErrTypeToException("CqQuery::execute(endpoint)", err);
    return err;
  }

  if (reply.getMessageType() == TcrMessage::EXCEPTION ||
      reply.getMessageType() == TcrMessage::CQDATAERROR_MSG_TYPE ||
      reply.getMessageType() == TcrMessage::CQ_EXCEPTION_TYPE) {
    err = ThinClientRegion::handleServerException("CqQuery::execute(endpoint)",
                                                  reply.getException());
    /*
    if (err == GF_CACHESERVER_EXCEPTION) {
      throw CqQueryException("CqQuery::execute(endpoint): exception at the
    server side: ",
              reply.getException());
    }
    else {
      GfErrTypeToException("CqQuery::execute(endpoint)", err);
    }
    */
  }

  return err;
}

void CqQueryImpl::executeAfterFailover() {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_mutex);
  if (m_cqState != CqState::RUNNING) {
    return;
  }
  executeCq(TcrMessage::EXECUTECQ_MSG_TYPE);
}

void CqQueryImpl::execute() {
  GuardUserAttributes gua;
  if (m_authenticatedView != nullptr) {
    gua.setAuthenticatedView(m_authenticatedView);
  }

  ACE_Guard<ACE_Recursive_Thread_Mutex> guardRedundancy(
      *(m_tccdm->getRedundancyLock()));

  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_mutex);
  if (m_cqState == CqState::RUNNING) {
    throw IllegalStateException("CqQuery::execute: cq is already running");
  }
  executeCq(TcrMessage::EXECUTECQ_MSG_TYPE);
}

// for          EXECUTE_REQUEST or REDUNDANT_EXECUTE_REQUEST
bool CqQueryImpl::executeCq(TcrMessage::MsgType) {
  GuardUserAttributes gua;
  if (m_authenticatedView != nullptr) {
    gua.setAuthenticatedView(m_authenticatedView);
  }

  LOGDEBUG("CqQueryImpl::executeCq");
  TcrMessageExecuteCq msg(m_cqService->getDM()
                              ->getConnectionManager()
                              .getCacheImpl()
                              ->getCache()
                              ->createDataOutput(),
                          m_cqName, m_queryString, CqState::RUNNING,
                          isDurable(), m_tccdm);
  TcrMessageReply reply(true, m_tccdm);

  GfErrType err = GF_NOERR;
  err = m_tccdm->sendSyncRequest(msg, reply);
  if (err != GF_NOERR) {
    GfErrTypeToException("CqQuery::executeCq:", err);
  }
  if (reply.getMessageType() == TcrMessage::EXCEPTION ||
      reply.getMessageType() == TcrMessage::CQDATAERROR_MSG_TYPE ||
      reply.getMessageType() == TcrMessage::CQ_EXCEPTION_TYPE) {
    err = ThinClientRegion::handleServerException("CqQuery::executeCq",
                                                  reply.getException());
    if (err == GF_CACHESERVER_EXCEPTION) {
      throw CqQueryException(
          std::string("CqQuery::executeCq: exception at the server side: ") +
          reply.getException());
    } else {
      GfErrTypeToException("CqQuery::executeCq", err);
    }
  }
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_mutex);
  m_cqState = CqState::RUNNING;
  updateStats();
  return true;
}

// for EXECUTE_INITIAL_RESULTS_REQUEST :
std::shared_ptr<CqResults> CqQueryImpl::executeWithInitialResults(
    std::chrono::milliseconds timeout) {
  util::PROTOCOL_OPERATION_TIMEOUT_BOUNDS(timeout);

  GuardUserAttributes gua;
  if (m_authenticatedView != nullptr) {
    gua.setAuthenticatedView(m_authenticatedView);
  }

  ACE_Guard<ACE_Recursive_Thread_Mutex> guardRedundancy(
      *(m_tccdm->getRedundancyLock()));

  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_mutex);
  if (m_cqState == CqState::RUNNING) {
    throw IllegalStateException(
        "CqQuery::executeWithInitialResults: cq is already running");
  }
  // QueryResult values;
  TcrMessageExecuteCqWithIr msg(m_cqService->getDM()
                                    ->getConnectionManager()
                                    .getCacheImpl()
                                    ->getCache()
                                    ->createDataOutput(),
                                m_cqName, m_queryString, CqState::RUNNING,
                                isDurable(), m_tccdm);

  TcrMessageReply reply(true, m_tccdm);
  auto resultCollector =
      std::unique_ptr<ChunkedQueryResponse>(new ChunkedQueryResponse(reply));
  reply.setChunkedResultHandler(resultCollector.get());
  reply.setTimeout(timeout);

  GfErrType err = GF_NOERR;
  err = m_tccdm->sendSyncRequest(msg, reply);
  if (err != GF_NOERR) {
    LOGDEBUG("CqQueryImpl::executeCqWithInitialResults errorred!!!!");
    GfErrTypeToException("CqQuery::executeCqWithInitialResults:", err);
  }
  if (reply.getMessageType() == TcrMessage::EXCEPTION ||
      reply.getMessageType() == TcrMessage::CQDATAERROR_MSG_TYPE ||
      reply.getMessageType() == TcrMessage::CQ_EXCEPTION_TYPE) {
    err = ThinClientRegion::handleServerException(
        "CqQuery::executeCqWithInitialResults", reply.getException());
    if (err == GF_CACHESERVER_EXCEPTION) {
      throw CqQueryException(
          std::string("CqQuery::executeWithInitialResults: exception ") +
          "at the server side: " + reply.getException());
    } else {
      GfErrTypeToException("CqQuery::executeWithInitialResults", err);
    }
  }
  m_cqState = CqState::RUNNING;
  updateStats();
  std::shared_ptr<CqResults> sr;
  auto&& values = resultCollector->getQueryResults();
  auto&& fieldNameVec = resultCollector->getStructFieldNames();
  auto sizeOfFieldNamesVec = fieldNameVec.size();
  if (sizeOfFieldNamesVec == 0) {
    LOGFINEST("Query::execute: creating ResultSet for query: %s",
              m_queryString.c_str());
    sr = std::dynamic_pointer_cast<CqResults>(
        std::make_shared<ResultSetImpl>(values));
  } else {
    if (values->size() % fieldNameVec.size() != 0) {
      throw MessageException(
          "Query::execute: Number of values coming "
          "from server has to be exactly divisible by field count");
    } else {
      LOGFINEST("Query::execute: creating StructSet for query: %s",
                m_queryString.c_str());
      sr = std::dynamic_pointer_cast<CqResults>(
          std::make_shared<StructSetImpl>(values, fieldNameVec));
    }
  }
  return sr;
}

/**
 * Stop or pause executing the query.
 */
void CqQueryImpl::stop() {
  if (isClosed()) {
    throw CqClosedException(("CQ is closed, CqName : " + m_cqName).c_str());
  }

  GuardUserAttributes gua;
  if (m_authenticatedView != nullptr) {
    gua.setAuthenticatedView(m_authenticatedView);
  }

  if (!(isRunning())) {
    throw IllegalStateException(
        ("CQ is not in running state, stop CQ does not apply, CqName : " +
         m_cqName)
            .c_str());
  }

  sendStopOrClose(TcrMessage::STOPCQ_MSG_TYPE);
  /*
  CqServiceVsdStats & stats = m_cqService->getCqServiceVsdStats();
  stats.decNumCqsActive();
  */
  setCqState(CqState::STOPPED);
  // stats.incNumCqsStopped();
  updateStats();
}
void CqQueryImpl::sendStopOrClose(TcrMessage::MsgType requestType) {
  GfErrType err = GF_NOERR;
  TcrMessageReply reply(true, m_tccdm);

  if (requestType == TcrMessage::STOPCQ_MSG_TYPE) {
    TcrMessageStopCQ msg(m_cqService->getDM()
                             ->getConnectionManager()
                             .getCacheImpl()
                             ->getCache()
                             ->createDataOutput(),
                         m_cqName, std::chrono::milliseconds(-1), m_tccdm);
    err = m_tccdm->sendSyncRequest(msg, reply);
  } else if (requestType == TcrMessage::CLOSECQ_MSG_TYPE) {
    TcrMessageCloseCQ msg(m_cqService->getDM()
                              ->getConnectionManager()
                              .getCacheImpl()
                              ->getCache()
                              ->createDataOutput(),
                          m_cqName, std::chrono::milliseconds(-1), m_tccdm);
    err = m_tccdm->sendSyncRequest(msg, reply);
  }

  if (err != GF_NOERR) {
    GfErrTypeToException("CqQuery::stop/close:", err);
  }
  if (reply.getMessageType() == TcrMessage::EXCEPTION ||
      reply.getMessageType() == TcrMessage::CQDATAERROR_MSG_TYPE ||
      reply.getMessageType() == TcrMessage::CQ_EXCEPTION_TYPE) {
    err = ThinClientRegion::handleServerException("CqQuery::stop/close",
                                                  reply.getException());
    if (err == GF_CACHESERVER_EXCEPTION) {
      throw CqQueryException(
          std::string("CqQuery::stop/close: exception at the server side: ") +
          reply.getException());
    } else {
      GfErrTypeToException("CqQuery::stop/close", err);
    }
  }
}

/**
 * Return the state of this query.
 * @return STOPPED RUNNING or CLOSED
 */
CqState CqQueryImpl::getState() { return m_cqState; }

/**
 * Sets the state of the cq.
 * Server side method. Called during cq registration time.
 */
void CqQueryImpl::setCqState(CqState state) {
  if (isClosed()) {
    throw CqClosedException(("CQ is closed, CqName : " + m_cqName).c_str());
  }
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_mutex);
  m_cqState = state;
}

CqAttributesMutator CqQueryImpl::getCqAttributesMutator() const {
  return m_cqAttributesMutator;
}

/**
 * @return Returns the cqOperation.
 */
CqOperation CqQueryImpl::getCqOperation() const { return m_cqOperation; }

/**
 * @param cqOperation The cqOperation to set.
 */
void CqQueryImpl::setCqOperation(CqOperation cqOperation) {
  m_cqOperation = cqOperation;
}

/**
 * Update CQ stats
 * @param cqEvent object
 */
void CqQueryImpl::updateStats(CqEvent& cqEvent) {
  auto stats = std::static_pointer_cast<CqQueryVsdStats>(m_stats);
  stats->incNumEvents();
  switch (cqEvent.getQueryOperation()) {
    case CqOperation::OP_TYPE_CREATE:
      stats->incNumInserts();
      break;
    case CqOperation::OP_TYPE_UPDATE:
      stats->incNumUpdates();
      break;
    case CqOperation::OP_TYPE_DESTROY:
      stats->incNumDeletes();
      break;
    default:
      break;
  }
}

/**
 * Return true if the CQ is in running state
 * @return true if running, false otherwise
 */
bool CqQueryImpl::isRunning() const {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_mutex);
  return m_cqState == CqState::RUNNING;
}

/**
 * Return true if the CQ is in Sstopped state
 * @return true if stopped, false otherwise
 */
bool CqQueryImpl::isStopped() const {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_mutex);
  return m_cqState == CqState::STOPPED ||
         (m_authenticatedView && m_authenticatedView->isClosed());
}

/**
 * Return true if the CQ is closed
 * @return true if closed, false otherwise
 */
bool CqQueryImpl::isClosed() const {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_mutex);
  return m_cqState == CqState::CLOSED ||
         (m_authenticatedView && m_authenticatedView->isClosed());
}

/**
 * Return true if the CQ is durable
 * @return true if durable, false otherwise
 */
bool CqQueryImpl::isDurable() const { return m_isDurable; }
