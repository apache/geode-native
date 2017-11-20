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
/*
 * CacheTransactionManagerImpl.cpp
 *
 *  Created on: 04-Feb-2011
 *      Author: ankurs
 */

#include "CacheTransactionManagerImpl.hpp"
#include <geode/TransactionId.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/PoolManager.hpp>

#include "TSSTXStateWrapper.hpp"
#include "TcrMessage.hpp"
#include "ThinClientBaseDM.hpp"
#include "ThinClientPoolDM.hpp"
#include "CacheRegionHelper.hpp"
#include "CacheImpl.hpp"
#include "TssConnectionWrapper.hpp"
#include "TXCleaner.hpp"

namespace apache {
namespace geode {
namespace client {

CacheTransactionManagerImpl::CacheTransactionManagerImpl(CacheImpl* cache)
    : m_cache(cache), m_txCond(m_suspendedTxLock) {}

CacheTransactionManagerImpl::~CacheTransactionManagerImpl() {}

void CacheTransactionManagerImpl::begin() {
  if (TSSTXStateWrapper::s_geodeTSSTXState->getTXState() != nullptr) {
    GfErrTypeThrowException("Transaction already in progress",
                            GF_CACHE_ILLEGAL_STATE_EXCEPTION);
  }
  TXState* txState = new TXState(m_cache->getCache());
  TSSTXStateWrapper::s_geodeTSSTXState->setTXState(txState);
  addTx(txState->getTransactionId()->getId());
}

void CacheTransactionManagerImpl::commit() {
  TXCleaner txCleaner(this);
  TXState* txState = txCleaner.getTXState();

  if (txState == nullptr) {
    GfErrTypeThrowException(
        "Transaction is null, cannot commit a null transaction",
        GF_CACHE_ILLEGAL_STATE_EXCEPTION);
  }

  TcrMessageCommit request(m_cache->getCache()->createDataOutput());
  TcrMessageReply reply(true, nullptr);

  ThinClientPoolDM* tcr_dm = getDM();
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
        const char* exceptionMsg = reply.getException();
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
        LOGERROR("Unknown message type in commit reply %d",
                 reply.getMessageType());
        GfErrTypeThrowException("Commit Failed", GF_MSG);
        break;
      }
    }
  }

  auto commit = std::static_pointer_cast<TXCommitMessage>(reply.getValue());
  txCleaner.clean();
  commit->apply(m_cache->getCache());

  /*
          if(m_writer != nullptr)
          {
                  try
                  {
                          std::shared_ptr<TransactionEvent> event(new
     TransactionEvent(txState->getTransactionId(),
     std::shared_ptr<Cache>(m_cache), commit->getEvents(m_cache)));
                          m_writer->beforeCommit(event);
                  } catch(const TransactionWriterException& ex)
                  {
                          noteCommitFailure(txState, commit);
                          GfErrTypeThrowException(ex.what(),
     GF_COMMIT_CONFLICT_EXCEPTION);
                  }
                  catch (const Exception& ex)
                  {
                          noteCommitFailure(txState, commit);
                          LOGERROR("Unexpected exception during writer callback
     %s", ex.what());
                          throw ex;
                  }
                  catch (...)
                  {
                          noteCommitFailure(txState, commit);
                          LOGERROR("Unexpected exception during writer
     callback");
                          throw;
                  }
          }
  */

  /*try
  {
          TcrMessage requestCommitBefore(TcrMessage::TX_SYNCHRONIZATION,
BEFORE_COMMIT, txState->getTransactionId()->getId(), STATUS_COMMITTED);
          TcrMessage replyCommitBefore;
          err = tcr_dm->sendSyncRequest(requestCommitBefore, replyCommitBefore);
          if(err != GF_NOERR)
          {
                  GfErrTypeThrowException("Error while committing", err);
          } else {
                  switch (replyCommitBefore.getMessageType()) {
                  case TcrMessage::REPLY: {
                          break;
                  }
                  case TcrMessage::EXCEPTION: {
                          const char* exceptionMsg =
replyCommitBefore.getException();
                                                        err =
ThinClientRegion::handleServerException("CacheTransactionManager::commit",
                                                            exceptionMsg);
                                                        GfErrTypeThrowException("Commit
Failed", err);
                          break;
                  }
                  case TcrMessage::REQUEST_DATA_ERROR: {
                          GfErrTypeThrowException("Commit Failed",
GF_COMMIT_CONFLICT_EXCEPTION);
                          break;
                  }
                  default: {
                          LOGERROR("Unknown message type in commit reply %d",
reply.getMessageType());
                                                  GfErrTypeThrowException("Commit
Failed", GF_MSG);
                          break;
                  }
                  }
          }
  }
  catch (const Exception& ex)
  {
//		noteCommitFailure(txState, commit);
          LOGERROR("Unexpected exception during commit %s", ex.what());
          throw ex;
  }
  catch (...)
  {
//		noteCommitFailure(txState, commit);
          LOGERROR("Unexpected exception during writer callback");
          throw;
  }

  try{
          TcrMessage requestCommitAfter(TcrMessage::TX_SYNCHRONIZATION,
AFTER_COMMIT, txState->getTransactionId()->getId(), STATUS_COMMITTED);
          TcrMessage replyCommitAfter;
          txCleaner.clean();
          commit->apply(m_cache);
          err = tcr_dm->sendSyncRequest(requestCommitAfter, replyCommitAfter);

          if(err != GF_NOERR)
          {
                  GfErrTypeThrowException("Error while committing", err);
          } else {
                  switch (replyCommitAfter.getMessageType()) {
                  case TcrMessage::RESPONSE: {
                          //commit = replyCommitAfter.getValue();
                          break;
                  }
                  case TcrMessage::EXCEPTION: {
                          const char* exceptionMsg =
replyCommitAfter.getException();
                                                                                        err = ThinClientRegion::handleServerException("CacheTransactionManager::commit",
                                                                                            exceptionMsg);
                                                                                        GfErrTypeThrowException("Commit Failed", err);
                          break;
                  }
                  case TcrMessage::REQUEST_DATA_ERROR: {
                          GfErrTypeThrowException("Commit Failed",
GF_COMMIT_CONFLICT_EXCEPTION);
                          break;
                  }
                  default: {
                          GfErrTypeThrowException("Commit Failed", GF_MSG);
                          break;
                  }
                  }
          }
  }
  catch (const Exception& ex)
  {
//		noteCommitFailure(txState, commit);
          throw ex;
  }
  catch (...)
  {
//		noteCommitFailure(txState, commit);
          throw;
  }*/

  //	noteCommitSuccess(txState, commit);
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
      GfErrTypeToException("Error while committing", err);
    }
  } catch (const Exception& ex) {
    // TODO: put a log message
    throw ex;
  } catch (...) {
    // TODO: put a log message
    throw;
  }
}

GfErrType CacheTransactionManagerImpl::rollback(TXState* txState,
                                                bool callListener) {
  TcrMessageRollback request(m_cache->getCache()->createDataOutput());
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
      default: { break; }
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
  TcrConnection* conn =
      (*TssConnectionWrapper::s_geodeTSSConn)->getConnection();
  if (conn != nullptr) {
    ThinClientPoolDM* dm = conn->getEndpointObject()->getPoolHADM();
    if (dm != nullptr) {
      return dm;
    }
  }
  return nullptr;
}

Cache* CacheTransactionManagerImpl::getCache() { return m_cache->getCache(); }
std::shared_ptr<TransactionId> CacheTransactionManagerImpl::suspend() {
  // get the current state of the thread
  TXState* txState = TSSTXStateWrapper::s_geodeTSSTXState->getTXState();
  if (txState == nullptr) {
    LOGFINE("Transaction not in progress. Returning nullptr transaction Id.");
    return nullptr;
  }

  // get the current connection that this transaction is using
  TcrConnection* conn =
      (*TssConnectionWrapper::s_geodeTSSConn)->getConnection();
  if (conn == nullptr) {
    LOGFINE(
        "Thread local connection is null. Returning nullptr transaction Id.");
    return nullptr;
  }

  // get the endpoint info from the connection
  TcrEndpoint* ep = conn->getEndpointObject();

  // store the endpoint info and the pool DM in the transaction state
  // this function setEPStr and setPoolDM is used only while suspending
  // the transaction. The info stored here is used during resume.
  txState->setEPStr(ep->name());
  txState->setPoolDM(ep->getPoolHADM());

  LOGFINE(
      "suspended Release the sticky connection associated with the "
      "transaction");
  txState->releaseStickyConnection();

  // set the expiry handler for the suspended transaction
  auto suspendedTxTimeout = m_cache->getDistributedSystem()
                                .getSystemProperties()
                                .suspendedTxTimeout();
  auto handler = new SuspendedTxExpiryHandler(this, txState->getTransactionId(),
                                              suspendedTxTimeout);
  long id = m_cache->getExpiryTaskManager()
                   .scheduleExpiryTask(handler, suspendedTxTimeout,
                                    std::chrono::seconds::zero(), false);
  txState->setSuspendedExpiryTaskId(id);

  // add the transaction state to the list of suspended transactions
  addSuspendedTx(txState->getTransactionId()->getId(), txState);

  // set the current transaction state as null
  TSSTXStateWrapper::s_geodeTSSTXState->setTXState(nullptr);

  // return the transaction ID
  return txState->getTransactionId();
}

void CacheTransactionManagerImpl::resume(std::shared_ptr<TransactionId> transactionId) {
  // get the current state of the thread
  if (TSSTXStateWrapper::s_geodeTSSTXState->getTXState() != nullptr) {
    GfErrTypeThrowException("A transaction is already in progress",
                            GF_CACHE_ILLEGAL_STATE_EXCEPTION);
  }

  // get the transaction state of the suspended transaction
  TXState* txState = removeSuspendedTx(
      (std::static_pointer_cast<TXId>(transactionId))->getId());
  if (txState == nullptr) {
    GfErrTypeThrowException(
        "Could not get transaction state for the transaction id.",
        GF_CACHE_ILLEGAL_STATE_EXCEPTION);
  }

  resumeTxUsingTxState(txState);
}

bool CacheTransactionManagerImpl::isSuspended(std::shared_ptr<TransactionId> transactionId) {
  return isSuspendedTx(
      (std::static_pointer_cast<TXId>(transactionId))->getId());
}
bool CacheTransactionManagerImpl::tryResume(std::shared_ptr<TransactionId> transactionId) {
  return tryResume(transactionId, true);
}
bool CacheTransactionManagerImpl::tryResume(
    std::shared_ptr<TransactionId> transactionId, bool cancelExpiryTask) {
  // get the current state of the thread
  if (TSSTXStateWrapper::s_geodeTSSTXState->getTXState() != nullptr) {
    LOGFINE("A transaction is already in progress. Cannot resume transaction.");
    return false;
  }

  // get the transaction state of the suspended transaction
  TXState* txState = removeSuspendedTx(
      (std::static_pointer_cast<TXId>(transactionId))->getId());
  if (txState == nullptr) return false;

  resumeTxUsingTxState(txState, cancelExpiryTask);
  return true;
}

bool CacheTransactionManagerImpl::tryResume(
    std::shared_ptr<TransactionId> transactionId, std::chrono::milliseconds waitTime) {
  // get the current state of the thread
  if (TSSTXStateWrapper::s_geodeTSSTXState->getTXState() != nullptr) {
    LOGFINE("A transaction is already in progress. Cannot resume transaction.");
    return false;
  }

  if (!exists(transactionId)) return false;

  // get the transaction state of the suspended transaction
  TXState* txState = removeSuspendedTx(
      (std::static_pointer_cast<TXId>(transactionId))->getId(), waitTime);
  if (txState == nullptr) return false;

  resumeTxUsingTxState(txState);
  return true;
}

void CacheTransactionManagerImpl::resumeTxUsingTxState(TXState* txState,
                                                       bool cancelExpiryTask) {
  if (txState == nullptr) return;

  TcrConnection* conn;

  LOGDEBUG("Resuming transaction for tid: %d",
           txState->getTransactionId()->getId());

  if (cancelExpiryTask) {
    // cancel the expiry task for the transaction
    m_cache->getExpiryTaskManager().cancelTask(
        txState->getSuspendedExpiryTaskId());
  } else {
    m_cache->getExpiryTaskManager().resetTask(
        txState->getSuspendedExpiryTaskId(), std::chrono::seconds(0));
  }

  // set the current state as the state of the suspended transaction
  TSSTXStateWrapper::s_geodeTSSTXState->setTXState(txState);

  LOGFINE("Get connection for transaction id %d",
          txState->getTransactionId()->getId());
  // get connection to the endpoint specified in the transaction state
  GfErrType error = txState->getPoolDM()->getConnectionToAnEndPoint(
      txState->getEPStr(), conn);
  if (conn == nullptr || error != GF_NOERR) {
    // throw an exception and set the current state as nullptr because
    // the transaction cannot be resumed
    TSSTXStateWrapper::s_geodeTSSTXState->setTXState(nullptr);
    GfErrTypeThrowException(
        "Could not get a connection for the transaction id.",
        GF_CACHE_ILLEGAL_STATE_EXCEPTION);
  } else {
    txState->getPoolDM()->setThreadLocalConnection(conn);
    LOGFINE("Set the thread local connection for transaction id %d",
            txState->getTransactionId()->getId());
  }
}

bool CacheTransactionManagerImpl::exists(std::shared_ptr<TransactionId> transactionId) {
  return findTx((std::static_pointer_cast<TXId>(transactionId))->getId());
}

bool CacheTransactionManagerImpl::exists() {
  return TSSTXStateWrapper::s_geodeTSSTXState->getTXState() != nullptr;
}

void CacheTransactionManagerImpl::addTx(int32_t txId) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_txLock);
  m_TXs.push_back(txId);
}

bool CacheTransactionManagerImpl::removeTx(int32_t txId) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_txLock);

  for (std::vector<int32_t>::iterator iter = m_TXs.begin(); iter != m_TXs.end();
       ++iter) {
    if (*iter == txId) {
      m_TXs.erase(iter);
      return true;
    }
  }
  return false;
}
bool CacheTransactionManagerImpl::findTx(int32_t txId) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_txLock);

  for (std::vector<int32_t>::iterator iter = m_TXs.begin(); iter != m_TXs.end();
       ++iter) {
    if (*iter == txId) return true;
  }
  return false;
}

void CacheTransactionManagerImpl::addSuspendedTx(int32_t txId,
                                                 TXState* txState) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_suspendedTxLock);
  m_suspendedTXs[txId] = txState;
  // signal if some thread is waiting for this transaction to suspend
  m_txCond.broadcast();
}

TXState* CacheTransactionManagerImpl::getSuspendedTx(int32_t txId) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_suspendedTxLock);
  std::map<int32_t, TXState*>::iterator it = m_suspendedTXs.find(txId);
  if (it == m_suspendedTXs.end()) return nullptr;
  TXState* rettxState = (*it).second;
  return rettxState;
}

TXState* CacheTransactionManagerImpl::removeSuspendedTx(int32_t txId) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_suspendedTxLock);
  std::map<int32_t, TXState*>::iterator it = m_suspendedTXs.find(txId);
  if (it == m_suspendedTXs.end()) return nullptr;
  TXState* rettxState = (*it).second;
  m_suspendedTXs.erase(it);
  return rettxState;
}
TXState* CacheTransactionManagerImpl::removeSuspendedTx(
    int32_t txId, std::chrono::milliseconds waitTime) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_suspendedTxLock);
  TXState* txState = nullptr;
  ACE_Time_Value currTime(ACE_OS::gettimeofday());
  ACE_Time_Value stopAt(currTime);
  stopAt += waitTime;

  do {
    txState = removeSuspendedTx(txId);
    if (txState == nullptr) {
      LOGFINE("Wait for the connection to get suspended, Tid: %d", txId);
      m_txCond.wait(&stopAt);
    }

  } while (txState == nullptr && findTx(txId) &&
           (currTime = ACE_OS::gettimeofday()) < stopAt);
  return txState;
}

bool CacheTransactionManagerImpl::isSuspendedTx(int32_t txId) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> _guard(m_suspendedTxLock);
  std::map<int32_t, TXState*>::iterator it = m_suspendedTXs.find(txId);
  if (it == m_suspendedTXs.end()) {
    return false;
  } else {
    return true;
  }
} std::shared_ptr<TransactionId> CacheTransactionManagerImpl::getTransactionId() {
  TXState* txState = TSSTXStateWrapper::s_geodeTSSTXState->getTXState();
  if (txState == nullptr) {
    return nullptr;
  } else {
    return txState->getTransactionId();
  }
}
/*
void CacheTransactionManagerImpl::setWriter(std::shared_ptr<TransactionWriter>
writer)
{
        m_writer = writer;
}
 std::shared_ptr<TransactionWriter> CacheTransactionManagerImpl::getWriter()
{
        return m_writer;
}


void
CacheTransactionManagerImpl::addListener(std::shared_ptr<TransactionListener>
aListener)
{
        if(aListener == nullptr)
        {
                GfErrTypeThrowException("Trying to add null listener.",
GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION);
        }
        if(!m_listeners.contains(aListener))
        {
                m_listeners.insert(aListener);
        }
}

void
CacheTransactionManagerImpl::removeListener(std::shared_ptr<TransactionListener>
aListener)
{
        if(aListener == nullptr)
        {
                GfErrTypeThrowException("Trying to remove null listener.",
GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION);
        }
        if(m_listeners.erase(aListener))
        {
                aListener->close();
        }
}

void CacheTransactionManagerImpl::noteCommitFailure(TXState* txState, const
std::shared_ptr<TXCommitMessage>& commitMessage)
{
        VectorOfEntryEvent events;
        if(commitMessage!= nullptr)
        {
                events = commitMessage->getEvents(m_cache);
        }
        std::shared_ptr<TransactionEvent> event(new
TransactionEvent(txState->getTransactionId(), std::shared_ptr<Cache>(m_cache),
events));

        for(HashSetOfSharedBase::Iterator iter = m_listeners.begin();
m_listeners.end() != iter; iter++)
        {
               auto listener =
std::static_pointer_cast<TransactionListener>(*iter);
                listener->afterFailedCommit(event);
        }
}

void CacheTransactionManagerImpl::noteCommitSuccess(TXState* txState, const
std::shared_ptr<TXCommitMessage>& commitMessage)
{
        VectorOfEntryEvent events;
                if(commitMessage!= nullptr)
                {
                        events = commitMessage->getEvents(m_cache);
                }
                std::shared_ptr<TransactionEvent> event(new
TransactionEvent(txState->getTransactionId(), std::shared_ptr<Cache>(m_cache),
events));

        for(HashSetOfSharedBase::Iterator iter = m_listeners.begin();
m_listeners.end() != iter; iter++)
        {
               auto listener =
std::static_pointer_cast<std::shared_ptr<TransactionListener>>(*iter);
                listener->afterCommit(event);
        }
}

void CacheTransactionManagerImpl::noteRollbackSuccess(TXState* txState, const
std::shared_ptr<TXCommitMessage>& commitMessage)
{
        VectorOfEntryEvent events;
        if(commitMessage!= nullptr)
        {
                events = commitMessage->getEvents(m_cache);
        }
        std::shared_ptr<TransactionEvent> event(new
TransactionEvent(txState->getTransactionId(), std::shared_ptr<Cache>(m_cache),
events));

        for(HashSetOfSharedBase::Iterator iter = m_listeners.begin();
m_listeners.end() != iter; iter++)
        {
               auto listener =
std::static_pointer_cast<TransactionListener>(*iter);
                listener->afterRollback(event);
        }
}
*/
}  // namespace client
}  // namespace geode
}  // namespace apache
