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

#include "CacheTransactionManagerImpl.hpp"

#include <geode/ExceptionTypes.hpp>
#include <geode/TransactionId.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "SuspendedTxExpiryTask.hpp"
#include "TSSTXStateWrapper.hpp"
#include "TXCleaner.hpp"
#include "TcrMessage.hpp"
#include "ThinClientBaseDM.hpp"
#include "ThinClientPoolDM.hpp"
#include "TssConnectionWrapper.hpp"
#include "util/exception.hpp"

namespace apache {
namespace geode {
namespace client {

CacheTransactionManagerImpl::CacheTransactionManagerImpl(CacheImpl* cache)
    : m_cache(cache) {}

CacheTransactionManagerImpl::~CacheTransactionManagerImpl() {}

void CacheTransactionManagerImpl::begin() {
  if (TSSTXStateWrapper::get().getTXState() != nullptr) {
    GfErrTypeThrowException("Transaction already in progress",
                            GF_CACHE_ILLEGAL_STATE_EXCEPTION);
  }
  auto txState = new TXState(m_cache);
  TSSTXStateWrapper::get().setTXState(txState);
  addTx(txState->getTransactionId().getId());
}

void CacheTransactionManagerImpl::commit() {
  TXCleaner txCleaner(this);
  auto txState = txCleaner.getTXState();

  if (txState == nullptr) {
    GfErrTypeThrowException(
        "Transaction is null, cannot commit a null transaction",
        GF_CACHE_ILLEGAL_STATE_EXCEPTION);
  }

  TcrMessageCommit request(new DataOutput(m_cache->createDataOutput()));
  TcrMessageReply reply(true, nullptr);

  auto tcr_dm = getDM();
  // This is for the case when no cache operation/s is performed between
  // tx->begin() and tx->commit()/rollback(),
  // simply return without sending COMMIT message to server. tcr_dm is nullptr
  // implies no cache operation is performed.
  // Theres no need to call txCleaner.clean(); here, because TXCleaner
  // destructor is called which cleans ThreadLocal.
  if (tcr_dm == nullptr) {
    return;
  }

  GfErrType err = tcr_dm->sendSyncRequest(request, reply);

  if (err != GF_NOERR) {
    // err = rollback(txState, false);
    //		noteCommitFailure(txState, nullptr);
    GfErrTypeThrowException("Error while committing", err);
  } else {
    switch (reply.getMessageType()) {
      case TcrMessage::RESPONSE: {
        break;
      }
      case TcrMessage::EXCEPTION: {
        //			noteCommitFailure(txState, nullptr);
        const auto& exceptionMsg = reply.getException();
        err = ThinClientRegion::handleServerException(
            "CacheTransactionManager::commit", exceptionMsg);
        GfErrTypeThrowException("Commit Failed", err);
        break;
      }
      case TcrMessage::COMMIT_ERROR: {
        //			noteCommitFailure(txState, nullptr);
        GfErrTypeThrowException("Commit Failed", GF_COMMIT_CONFLICT_EXCEPTION);
        break;
      }
      default: {
        //			noteCommitFailure(txState, nullptr);
        LOG_ERROR("Unknown message type in commit reply %d",
                  reply.getMessageType());
        GfErrTypeThrowException("Commit Failed", GF_MSG);
        break;
      }
    }
  }

  auto commit = std::dynamic_pointer_cast<TXCommitMessage>(reply.getValue());
  txCleaner.clean();
  commit->apply(m_cache->getCache());
}

void CacheTransactionManagerImpl::rollback() {
  TXCleaner txCleaner(this);
  TXState* txState = txCleaner.getTXState();

  if (txState == nullptr) {
    GfErrTypeThrowException("Thread does not have an active transaction",
                            GF_CACHE_ILLEGAL_STATE_EXCEPTION);
  }

  try {
    GfErrType err = rollback(txState, true);
    if (err != GF_NOERR) {
      throwExceptionIfError("Error while committing", err);
    }
  } catch (const Exception& ex) {
    // TODO: put a log message
    throw ex;
  } catch (...) {
    // TODO: put a log message
    throw;
  }
}

GfErrType CacheTransactionManagerImpl::rollback(TXState*, bool) {
  TcrMessageRollback request(new DataOutput(m_cache->createDataOutput()));
  TcrMessageReply reply(true, nullptr);
  GfErrType err = GF_NOERR;
  ThinClientPoolDM* tcr_dm = getDM();
  // This is for the case when no cache operation/s is performed between
  // tx->begin() and tx->commit()/rollback(),
  // simply return without sending COMMIT message to server. tcr_dm is nullptr
  // implies no cache operation is performed.
  // Theres no need to call txCleaner.clean(); here, because TXCleaner
  // destructor is called which cleans ThreadLocal.
  if (tcr_dm == nullptr) {
    return err;
  }
  err = tcr_dm->sendSyncRequest(request, reply);

  if (err == GF_NOERR) {
    switch (reply.getMessageType()) {
      case TcrMessage::REPLY: {
        break;
      }
      case TcrMessage::EXCEPTION: {
        break;
      }
      default: {
        break;
      }
    }
  }

  /*	if(err == GF_NOERR && callListener)
          {
  //	auto commit =
  std::static_pointer_cast<TXCommitMessage>(reply.getValue());
                  noteRollbackSuccess(txState, nullptr);
          }
  */
  return err;
}

ThinClientPoolDM* CacheTransactionManagerImpl::getDM() {
  if (auto conn = TssConnectionWrapper::get().getConnection()) {
    if (auto dm = conn->getEndpointObject()->getPoolHADM()) {
      return dm;
    }
  }
  return nullptr;
}

Cache* CacheTransactionManagerImpl::getCache() { return m_cache->getCache(); }
TransactionId& CacheTransactionManagerImpl::suspend() {
  // get the current state of the thread
  auto txState = TSSTXStateWrapper::get().getTXState();
  if (txState == nullptr) {
    LOG_FINE("Transaction not in progress. Returning nullptr transaction Id.");
    throw TransactionException("Transaction not in progress.");
  }

  // get the current connection that this transaction is using
  auto conn = TssConnectionWrapper::get().getConnection();
  if (!conn) {
    LOG_FINE(
        "Thread local connection is null. Returning nullptr transaction Id.");
    throw TransactionException("Thread local connection is null.");
  }

  // get the endpoint info from the connection
  TcrEndpoint* ep = conn->getEndpointObject();

  // store the endpoint info and the pool DM in the transaction state
  // this function setEPStr and setPoolDM is used only while suspending
  // the transaction. The info stored here is used during resume.
  txState->setEPStr(ep->name());
  txState->setPoolDM(ep->getPoolHADM());

  LOG_FINE(
      "suspended Release the sticky connection associated with the "
      "transaction");
  txState->releaseStickyConnection();

  // set the expiry handler for the suspended transaction
  auto timeout = m_cache->getDistributedSystem()
                     .getSystemProperties()
                     .suspendedTxTimeout();
  auto& manager = m_cache->getExpiryTaskManager();
  auto task = std::make_shared<SuspendedTxExpiryTask>(
      manager, *this, txState->getTransactionId());
  auto id = manager.schedule(std::move(task), timeout);
  txState->setSuspendedExpiryTaskId(id);

  // add the transaction state to the list of suspended transactions
  addSuspendedTx(txState->getTransactionId().getId(), txState);

  // set the current transaction state as null
  TSSTXStateWrapper::get().setTXState(nullptr);

  // return the transaction ID
  return static_cast<TransactionId&>(txState->getTransactionId());
}

void CacheTransactionManagerImpl::resume(TransactionId& transactionId) {
  // get the current state of the thread
  if (TSSTXStateWrapper::get().getTXState() != nullptr) {
    GfErrTypeThrowException("A transaction is already in progress",
                            GF_CACHE_ILLEGAL_STATE_EXCEPTION);
  }

  // get the transaction state of the suspended transaction
  TXState* txState =
      removeSuspendedTx((static_cast<TXId&>(transactionId)).getId());
  if (txState == nullptr) {
    GfErrTypeThrowException(
        "Could not get transaction state for the transaction id.",
        GF_CACHE_ILLEGAL_STATE_EXCEPTION);
  }

  resumeTxUsingTxState(txState);
}

bool CacheTransactionManagerImpl::isSuspended(TransactionId& transactionId) {
  return isSuspendedTx((static_cast<TXId&>(transactionId)).getId());
}
bool CacheTransactionManagerImpl::tryResume(TransactionId& transactionId) {
  return tryResume(transactionId, true);
}
bool CacheTransactionManagerImpl::tryResume(TransactionId& transactionId,
                                            bool cancelExpiryTask) {
  // get the current state of the thread
  if (TSSTXStateWrapper::get().getTXState() != nullptr) {
    LOG_FINE(
        "A transaction is already in progress. Cannot resume transaction.");
    return false;
  }

  // get the transaction state of the suspended transaction
  TXState* txState =
      removeSuspendedTx((static_cast<TXId&>(transactionId)).getId());
  if (txState == nullptr) return false;

  resumeTxUsingTxState(txState, cancelExpiryTask);
  return true;
}

bool CacheTransactionManagerImpl::tryResume(
    TransactionId& transactionId, std::chrono::milliseconds waitTime) {
  // get the current state of the thread
  if (TSSTXStateWrapper::get().getTXState() != nullptr) {
    LOG_FINE(
        "A transaction is already in progress. Cannot resume transaction.");
    return false;
  }

  if (!exists(transactionId)) return false;

  // get the transaction state of the suspended transaction
  TXState* txState =
      removeSuspendedTx((static_cast<TXId&>(transactionId)).getId(), waitTime);
  if (txState == nullptr) return false;

  resumeTxUsingTxState(txState);
  return true;
}

void CacheTransactionManagerImpl::resumeTxUsingTxState(TXState* txState,
                                                       bool cancelExpiryTask) {
  if (txState == nullptr) return;

  TcrConnection* conn;

  LOG_DEBUG("Resuming transaction for tid: %d",
            txState->getTransactionId().getId());

  if (cancelExpiryTask) {
    // cancel the expiry task for the transaction
    m_cache->getExpiryTaskManager().cancel(txState->getSuspendedExpiryTaskId());
  }

  // set the current state as the state of the suspended transaction
  TSSTXStateWrapper::get().setTXState(txState);

  LOG_FINE("Get connection for transaction id %d",
           txState->getTransactionId().getId());
  // get connection to the endpoint specified in the transaction state
  GfErrType error = txState->getPoolDM()->getConnectionToAnEndPoint(
      txState->getEPStr(), conn);
  if (conn == nullptr || error != GF_NOERR) {
    // throw an exception and set the current state as nullptr because
    // the transaction cannot be resumed
    TSSTXStateWrapper::get().setTXState(nullptr);
    GfErrTypeThrowException(
        "Could not get a connection for the transaction id.",
        GF_CACHE_ILLEGAL_STATE_EXCEPTION);
  } else {
    txState->getPoolDM()->setThreadLocalConnection(conn);
    LOG_FINE("Set the thread local connection for transaction id %d",
             txState->getTransactionId().getId());
  }
}

bool CacheTransactionManagerImpl::exists(TransactionId& transactionId) {
  return findTx((static_cast<TXId&>(transactionId)).getId());
}

bool CacheTransactionManagerImpl::exists() {
  return TSSTXStateWrapper::get().getTXState() != nullptr;
}

void CacheTransactionManagerImpl::addTx(int32_t txId) {
  std::unique_lock<decltype(m_txLock)> _guard(m_txLock);
  m_TXs.push_back(txId);
}

void CacheTransactionManagerImpl::removeTx(int32_t txId) {
  std::unique_lock<decltype(m_txLock)> _guard(m_txLock);
  m_TXs.erase(std::remove(m_TXs.begin(), m_TXs.end(), txId), m_TXs.end());
}
bool CacheTransactionManagerImpl::findTx(int32_t txId) {
  std::unique_lock<decltype(m_txLock)> _guard(m_txLock);
  return std::find(m_TXs.begin(), m_TXs.end(), txId) != m_TXs.end();
}

void CacheTransactionManagerImpl::addSuspendedTx(int32_t txId,
                                                 TXState* txState) {
  std::unique_lock<decltype(m_suspendedTxLock)> _guard(m_suspendedTxLock);
  m_suspendedTXs[txId] = txState;
  m_txCond.notify_all();
}

TXState* CacheTransactionManagerImpl::getSuspendedTx(int32_t txId) {
  std::unique_lock<decltype(m_suspendedTxLock)> _guard(m_suspendedTxLock);
  auto&& it = m_suspendedTXs.find(txId);
  if (it == m_suspendedTXs.end()) return nullptr;
  auto rettxState = (*it).second;
  return rettxState;
}

TXState* CacheTransactionManagerImpl::removeSuspendedTx(int32_t txId) {
  std::unique_lock<decltype(m_suspendedTxLock)> _guard(m_suspendedTxLock);
  auto&& it = m_suspendedTXs.find(txId);
  if (it == m_suspendedTXs.end()) return nullptr;
  auto rettxState = (*it).second;
  m_suspendedTXs.erase(it);
  return rettxState;
}

TXState* CacheTransactionManagerImpl::removeSuspendedTx(
    int32_t txId, std::chrono::milliseconds waitTime) {
  std::unique_lock<decltype(m_suspendedTxLock)> _guard(m_suspendedTxLock);

  auto txState = removeSuspendedTx(txId);
  if (txState == nullptr) {
    LOG_FINE("Wait for the connection to get suspended, Tid: %d", txId);
    m_txCond.wait_for(_guard, waitTime, [this, txId, &txState] {
      return nullptr != (txState = removeSuspendedTx(txId));
    });
  }

  return txState;
}

bool CacheTransactionManagerImpl::isSuspendedTx(int32_t txId) {
  std::unique_lock<decltype(m_suspendedTxLock)> _guard(m_suspendedTxLock);
  auto&& it = m_suspendedTXs.find(txId);
  if (it == m_suspendedTXs.end()) {
    return false;
  } else {
    return true;
  }
}

TransactionId& CacheTransactionManagerImpl::getTransactionId() {
  auto txState = TSSTXStateWrapper::get().getTXState();
  return txState->getTransactionId();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
