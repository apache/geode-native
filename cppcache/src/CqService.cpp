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

#include "CqService.hpp"

#include <sstream>

#include <geode/CqServiceStatistics.hpp>
#include <geode/CqStatusListener.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/SystemProperties.hpp>

#include "CqEventImpl.hpp"
#include "CqQueryImpl.hpp"
#include "DistributedSystem.hpp"
#include "ReadWriteLock.hpp"
#include "TcrConnectionManager.hpp"
#include "ThinClientPoolDM.hpp"
#include "util/exception.hpp"

namespace apache {
namespace geode {
namespace client {

CqService::CqService(ThinClientBaseDM* tccdm,
                     StatisticsFactory* statisticsFactory)
    : m_tccdm(tccdm),
      m_statisticsFactory(statisticsFactory),
      m_notificationSema(1),
      m_stats(std::make_shared<CqServiceVsdStats>(m_statisticsFactory)) {
  m_running = true;
  LOGDEBUG("CqService Started");
}
CqService::~CqService() noexcept { LOGDEBUG("CqService Destroyed"); }

void CqService::updateStats() {
  auto stats = std::dynamic_pointer_cast<CqServiceVsdStats>(m_stats);

  stats->setNumCqsActive(0);
  stats->setNumCqsStopped(0);

  auto&& lock = m_cqQueryMap.make_lock();

  stats->setNumCqsOnClient(static_cast<uint32_t>(m_cqQueryMap.size()));

  if (m_cqQueryMap.empty()) return;

  for (const auto& kv : m_cqQueryMap) {
    auto& cquery = kv.second;
    switch (cquery->getState()) {
      case CqState::RUNNING:
        stats->incNumCqsActive();
        break;
      case CqState::STOPPED:
        stats->incNumCqsStopped();
        break;
      case CqState::CLOSED:
      case CqState::CLOSING:
      case CqState::INVALID:
        break;
    }
  }
}

bool CqService::checkAndAcquireLock() {
  if (m_running) {
    m_notificationSema.acquire();
    if (m_running == false) {
      m_notificationSema.release();
      return false;
    }
    return true;
  } else {
    return false;
  }
}
std::shared_ptr<CqQuery> CqService::newCq(
    const std::string& cqName, const std::string& queryString,
    const std::shared_ptr<CqAttributes>& cqAttributes, bool isDurable) {
  if (queryString.empty()) {
    throw IllegalArgumentException("Null queryString is passed. ");
  } else if (cqAttributes == nullptr) {
    throw IllegalArgumentException("Null cqAttribute is passed. ");
  }

  // Check if the subscription is enabled on the pool
  auto pool = dynamic_cast<ThinClientPoolDM*>(m_tccdm);
  if (pool != nullptr && !pool->getSubscriptionEnabled()) {
    LOGERROR(
        "Cannot create CQ because subscription is not enabled on the pool.");
    throw IllegalStateException(
        "Cannot create CQ because subscription is not enabled on the pool.");
  }

  // check for durable client
  if (isDurable) {
    auto&& durableID = m_tccdm->getConnectionManager()
                           .getCacheImpl()
                           ->getDistributedSystem()
                           .getSystemProperties()
                           .durableClientId();
    if (durableID.empty()) {
      LOGERROR("Cannot create durable CQ because client is not durable.");
      throw IllegalStateException(
          "Cannot create durable CQ because client is not durable.");
    }
  }

  // Check if the given cq already exists.
  if (!cqName.empty() && isCqExists(cqName)) {
    throw CqExistsException(
        ("CQ with the given name already exists. CqName : " + cqName).c_str());
  }

  std::shared_ptr<UserAttributes> ua = nullptr;
  if (m_tccdm != nullptr && m_tccdm->isMultiUserMode()) {
    ua = UserAttributes::threadLocalUserAttributes;
  }

  auto cQuery = std::make_shared<CqQueryImpl>(
      shared_from_this(), cqName, queryString, cqAttributes,
      m_statisticsFactory, isDurable, ua);
  cQuery->initCq();
  return std::move(cQuery);
}

/**
 * Adds the given CQ and cqQuery object into the CQ map.
 */
void CqService::addCq(const std::string& cqName, std::shared_ptr<CqQuery>& cq) {
  auto result = m_cqQueryMap.emplace(cqName, cq);
  if (!result.second) {
    throw CqExistsException("CQ with given name already exists. ");
  }
}

/**
 * Removes given CQ from the cqMap..
 */
void CqService::removeCq(const std::string& cqName) {
  m_cqQueryMap.erase(cqName);
}

/**
 * Retrieve a CqQuery by name.
 * @return the CqQuery or null if not found
 */
std::shared_ptr<CqQuery> CqService::getCq(const std::string& cqName) {
  auto&& lock = m_cqQueryMap.make_lock();
  const auto& found = m_cqQueryMap.find(cqName);
  if (found == m_cqQueryMap.end()) {
    LOGWARN("Failed to get the specified CQ: %s", cqName.c_str());
  } else {
    return found->second;
  }
  return nullptr;
}

/**
 * Clears the CQ Query Map.
 */
void CqService::clearCqQueryMap() {
  LOGFINE("Cleaning clearCqQueryMap.");
  m_cqQueryMap.clear();
}

/**
 * Retrieve  all registered CQs
 */
CqService::query_container_type CqService::getAllCqs() {
  CqService::query_container_type cqVec;
  auto&& lock = m_cqQueryMap.make_lock();
  if (!m_cqQueryMap.empty()) {
    cqVec.reserve(m_cqQueryMap.size());
    for (auto& kv : m_cqQueryMap) {
      cqVec.push_back(kv.second);
    }
  }
  return cqVec;
}

/**
 * Executes all the cqs on this client.
 */
void CqService::executeAllClientCqs(bool afterFailover) {
  query_container_type cqVec = getAllCqs();
  executeCqs(cqVec, afterFailover);
}

/**
 * Executes all CQs on the specified endpoint after failover.
 */
GfErrType CqService::executeAllClientCqs(TcrEndpoint* endpoint) {
  query_container_type cqVec = getAllCqs();
  return executeCqs(cqVec, endpoint);
}

/**
 * Executes all the given cqs on the specified endpoint after failover.
 */
GfErrType CqService::executeCqs(query_container_type& cqs,
                                TcrEndpoint* endpoint) {
  if (cqs.empty()) {
    return GF_NOERR;
  }

  GfErrType err = GF_NOERR;
  GfErrType opErr = GF_NOERR;

  for (auto& cq : cqs) {
    if (!cq->isClosed() && cq->isRunning()) {
      opErr = std::static_pointer_cast<CqQueryImpl>(cq)->execute(endpoint);
      if (err == GF_NOERR) {
        err = opErr;
      }
    }
  }
  return err;
}

/**
 * Executes all the given cqs.
 */
void CqService::executeCqs(query_container_type& cqs, bool afterFailover) {
  if (cqs.empty()) {
    return;
  }
  std::string cqName;
  for (auto& cq : cqs) {
    if (!cq->isClosed() &&
        (cq->isStopped() || (cq->isRunning() && afterFailover))) {
      try {
        cqName = cq->getName();
        if (afterFailover) {
          std::static_pointer_cast<CqQueryImpl>(cq)->executeAfterFailover();
        } else {
          cq->execute();
        }
      } catch (QueryException& qe) {
        LOGFINE("%s", ("Failed to execute the CQ, CqName : " + cqName +
                       " Error : " + qe.what())
                          .c_str());
      } catch (CqClosedException& cce) {
        LOGFINE(("Failed to execute the CQ, CqName : " + cqName +
                 " Error : " + cce.what())
                    .c_str());
      }
    }
  }
}

/**
 * Stops all the cqs
 */
void CqService::stopAllClientCqs() {
  query_container_type cqVec = getAllCqs();
  // MapOfRegionGuard guard( m_cqQueryMap->mutex() );
  stopCqs(cqVec);
}

/**
 * Stops all the specified cqs.
 */
void CqService::stopCqs(query_container_type& cqs) {
  if (cqs.empty()) {
    return;
  }

  std::string cqName;
  for (auto cq : cqs) {
    if (!cq->isClosed() && cq->isRunning()) {
      try {
        cqName = cq->getName();
        cq->stop();
      } catch (QueryException& qe) {
        LOGFINE(("Failed to stop the CQ, CqName : " + cqName +
                 " Error : " + qe.what())
                    .c_str());
      } catch (CqClosedException& cce) {
        LOGFINE(("Failed to stop the CQ, CqName : " + cqName +
                 " Error : " + cce.what())
                    .c_str());
      }
    }
  }
}

void CqService::closeCqs(query_container_type& cqs) {
  LOGDEBUG("closeCqs() TcrMessage::isKeepAlive() = %d ",
           TcrMessage::isKeepAlive());
  if (!cqs.empty()) {
    std::string cqName;
    for (auto& cq : cqs) {
      try {
        auto cqi = std::static_pointer_cast<CqQueryImpl>(cq);
        cqName = cqi->getName();
        LOGDEBUG("closeCqs() cqname = %s isDurable = %d ", cqName.c_str(),
                 cqi->isDurable());
        if (!(cqi->isDurable() && TcrMessage::isKeepAlive())) {
          cqi->close(true);
        } else {
          cqi->close(false);
        }
      } catch (QueryException& qe) {
        LOGFINE(("Failed to close the CQ, CqName : " + cqName +
                 " Error : " + qe.what())
                    .c_str());
      } catch (CqClosedException& cce) {
        LOGFINE(("Failed to close the CQ, CqName : " + cqName +
                 " Error : " + cce.what())
                    .c_str());
      }
    }
  }
}

/**
 * Get statistics information for all CQs
 * @return the CqServiceStatistics
 */
std::shared_ptr<CqServiceStatistics> CqService::getCqServiceStatistics() {
  return m_stats;
}

/**
 * Close the CQ Service after cleanup if any.
 *
 */
void CqService::closeCqService() {
  if (m_running) {
    m_running = false;
    m_notificationSema.acquire();
    cleanup();
    m_notificationSema.release();
  }
}
void CqService::closeAllCqs() {
  LOGFINE("closeAllCqs()");
  query_container_type cqVec = getAllCqs();
  LOGFINE("closeAllCqs() 1");
  auto&& lock = m_cqQueryMap.make_lock();
  LOGFINE("closeAllCqs() 2");
  closeCqs(cqVec);
}

/**
 * Cleans up the CqService.
 */
void CqService::cleanup() {
  LOGFINE("Cleaning up CqService.");

  // Close All the CQs.
  // Need to take care when Clients are still connected...
  closeAllCqs();

  // Clear cqQueryMap.
  clearCqQueryMap();
}

/*
 * Checks if CQ with the given name already exists.
 * @param cqName name of the CQ.
 * @return true if exists else false.
 */
bool CqService::isCqExists(const std::string& cqName) {
  auto&& lock = m_cqQueryMap.make_lock();

  return m_cqQueryMap.find(cqName) != m_cqQueryMap.end();
}
void CqService::receiveNotification(TcrMessage* msg) {
  invokeCqListeners(msg->getCqs(), msg->getMessageTypeForCq(), msg->getKey(),
                    msg->getValue(), msg->getDeltaBytes(), msg->getEventId());
  _GEODE_SAFE_DELETE(msg);
  m_notificationSema.release();
}

/**
 * Invokes the CqListeners for the given CQs.
 * @param cqs list of cqs with the cq operation from the Server.
 * @param messageType base operation
 * @param key key to notify for
 * @param value associated value
 * @param deltaValue if delta, contains delta bytes
 * @param eventId event to send
 */
void CqService::invokeCqListeners(const std::map<std::string, int>* cqs,
                                  uint32_t messageType,
                                  std::shared_ptr<CacheableKey> key,
                                  std::shared_ptr<Cacheable> value,
                                  std::shared_ptr<CacheableBytes> deltaValue,
                                  std::shared_ptr<EventId> eventId) {
  LOGDEBUG("CqService::invokeCqListeners");
  for (const auto& kv : *cqs) {
    const auto cqName = kv.first;
    auto cQuery = getCq(cqName);
    auto cQueryImpl = std::dynamic_pointer_cast<CqQueryImpl>(cQuery);
    if (!(cQueryImpl && cQueryImpl->isRunning())) {
      LOGFINE("Unable to invoke CqListener, %s, CqName: %s",
              cQueryImpl ? "CQ not found" : "CQ is Not running",
              cqName.c_str());
      continue;
    }

    const auto cqOp = kv.second;

    // If Region destroy event, close the cq.
    if (cqOp == TcrMessage::DESTROY_REGION) {
      // The close will also invoke the listeners close().
      try {
        cQueryImpl->close(false);
      } catch (Exception& ex) {
        // handle?
        LOGFINE("Exception while invoking CQ listeners: %s", ex.what());
      }
      continue;
    }

    // Construct CqEvent.
    auto cqEvent =
        new CqEventImpl(cQuery, getOperation(messageType), getOperation(cqOp),
                        key, value, m_tccdm, deltaValue, eventId);

    // Update statistics
    cQueryImpl->updateStats(*cqEvent);

    // invoke CQ Listeners.
    for (auto l : cQueryImpl->getCqAttributes()->getCqListeners()) {
      try {
        // Check if the listener is not null, it could have been changed/reset
        // by the CqAttributeMutator.
        if (l) {
          if (cqEvent->getError() == true) {
            l->onError(*cqEvent);
          } else {
            l->onEvent(*cqEvent);
          }
        }
        // Handle client side exceptions.
      } catch (Exception& ex) {
        LOGWARN(("Exception in the CqListener of the CQ named " + cqName +
                 ", error: " + ex.what())
                    .c_str());
      }
    }
    delete cqEvent;
  }
}

void CqService::invokeCqConnectedListeners(const std::string& poolName,
                                           bool connected) {
  query_container_type vec = getAllCqs();
  for (auto& cQuery : vec) {
    const auto& cqName = cQuery->getName();
    auto cQueryImpl = std::dynamic_pointer_cast<CqQueryImpl>(cQuery);
    if (cQueryImpl == nullptr || !cQueryImpl->isRunning()) {
      LOGFINE("Unable to invoke CqStatusListener, %s, CqName: %s",
              (cQueryImpl == nullptr) ? "CQ not found" : "CQ is Not running",
              cqName.c_str());
      continue;
    }

    // Check cq pool to determine if the pool matches, if not continue.
    auto* poolDM = dynamic_cast<ThinClientPoolDM*>(cQueryImpl->getDM());
    if (poolDM != nullptr) {
      std::string pName = poolDM->getName();
      if (pName.compare(poolName) != 0) {
        continue;
      }
    }

    // invoke CQ Listeners.
    for (auto l : cQueryImpl->getCqAttributes()->getCqListeners()) {
      try {
        // Check if the listener is not null, it could have been changed/reset
        // by the CqAttributeMutator.
        if (auto statusLstr = std::dynamic_pointer_cast<CqStatusListener>(l)) {
          if (connected) {
            statusLstr->onCqConnected();
          } else {
            statusLstr->onCqDisconnected();
          }
        }
        // Handle client side exceptions.
      } catch (Exception& ex) {
        LOGWARN(("Exception in the CqStatusListener of the CQ named " + cqName +
                 ", error: " + ex.what())
                    .c_str());
      }
    }
  }
}

/**
 * Returns the Operation for the given EnumListenerEvent type.
 * @param eventType event type to translate to opType
 * @return Operation
 */
CqOperation CqService::getOperation(int eventType) {
  CqOperation op = CqOperation::OP_TYPE_INVALID;
  switch (eventType) {
    case TcrMessage::LOCAL_CREATE:
      op = CqOperation::OP_TYPE_CREATE;
      break;

    case TcrMessage::LOCAL_UPDATE:
      op = CqOperation::OP_TYPE_UPDATE;
      break;

    case TcrMessage::LOCAL_DESTROY:
      op = CqOperation::OP_TYPE_DESTROY;
      break;

    case TcrMessage::LOCAL_INVALIDATE:
      op = CqOperation::OP_TYPE_INVALIDATE;
      break;

    case TcrMessage::CLEAR_REGION:
      op = CqOperation::OP_TYPE_REGION_CLEAR;
      break;

      //      case TcrMessage::INVALIDATE_REGION :
      //        op = CqOperation::OP_TYPE_REGION_INVALIDATE;
      //        break;
  }
  return op;
}

/**
 * Gets all the durable CQs registered by this client.
 *
 * @return List of names of registered durable CQs, empty list if no durable
 * cqs.
 */
std::shared_ptr<CacheableArrayList> CqService::getAllDurableCqsFromServer() {
  TcrMessageGetDurableCqs msg(
      new DataOutput(
          m_tccdm->getConnectionManager().getCacheImpl()->createDataOutput()),
      m_tccdm);
  TcrMessageReply reply(true, m_tccdm);

  // intialize the chunked response hadler for durable cqs list
  ChunkedDurableCQListResponse* resultCollector =
      new ChunkedDurableCQListResponse(reply);
  reply.setChunkedResultHandler(
      static_cast<TcrChunkedResult*>(resultCollector));
  reply.setTimeout(DEFAULT_QUERY_RESPONSE_TIMEOUT);

  GfErrType err = GF_NOERR;
  err = m_tccdm->sendSyncRequest(msg, reply);
  if (err != GF_NOERR) {
    LOGDEBUG("CqService::getAllDurableCqsFromServer!!!!");
    throwExceptionIfError("CqService::getAllDurableCqsFromServer:", err);
  }
  if (reply.getMessageType() == TcrMessage::EXCEPTION ||
      reply.getMessageType() == TcrMessage::GET_DURABLE_CQS_DATA_ERROR) {
    err = ThinClientRegion::handleServerException(
        "CqService::getAllDurableCqsFromServer", reply.getException());
    if (err == GF_CACHESERVER_EXCEPTION) {
      std::stringstream message;
      message << "CqService::getAllDurableCqsFromServer: exception "
              << "at the server side: " << reply.getException();
      throw CqQueryException(message.str());
    } else {
      throwExceptionIfError("CqService::getAllDurableCqsFromServer", err);
    }
  }

  auto tmpRes = resultCollector->getResults();
  delete resultCollector;
  return tmpRes;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
