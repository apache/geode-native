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

#include "InternalCacheTransactionManager2PCImpl.hpp"

#include <geode/ExceptionTypes.hpp>
#include <geode/TransactionId.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "CacheTransactionManagerImpl.hpp"
#include "TXCleaner.hpp"
#include "TcrConnectionManager.hpp"
#include "TcrMessage.hpp"
#include "ThinClientPoolDM.hpp"
#include "util/exception.hpp"

namespace apache {
namespace geode {
namespace client {

InternalCacheTransactionManager2PCImpl::InternalCacheTransactionManager2PCImpl(
    CacheImpl* cache)
    : CacheTransactionManagerImpl(cache) {}

InternalCacheTransactionManager2PCImpl::
    ~InternalCacheTransactionManager2PCImpl() {}

void InternalCacheTransactionManager2PCImpl::prepare() {
  try {
    auto txState = TSSTXStateWrapper::get().getTXState();
    if (!txState) {
      GfErrTypeThrowException(
          "Transaction is null, cannot prepare of a null transaction",
          GF_CACHE_ILLEGAL_STATE_EXCEPTION);
      return;  // never called
    }

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

    TcrMessageTxSynchronization requestCommitBefore(
        new DataOutput(
            tcr_dm->getConnectionManager().getCacheImpl()->createDataOutput()),
        BEFORE_COMMIT, txState->getTransactionId().getId(), STATUS_COMMITTED);

    TcrMessageReply replyCommitBefore(true, nullptr);
    GfErrType err =
        tcr_dm->sendSyncRequest(requestCommitBefore, replyCommitBefore);
    if (err != GF_NOERR) {
      GfErrTypeThrowException("Error while prepare", err);
    } else {
      switch (replyCommitBefore.getMessageType()) {
        case TcrMessage::REPLY:
          txState->setPrepared();
          break;
        case TcrMessage::EXCEPTION: {
          TXCleaner txCleaner(this);
          const auto& exceptionMsg = replyCommitBefore.getException();
          err = ThinClientRegion::handleServerException(
              "CacheTransactionManager::prepare", exceptionMsg);
          GfErrTypeThrowException("Commit Failed in prepare", err);
          break;
        }
        case TcrMessage::REQUEST_DATA_ERROR: {
          TXCleaner txCleaner(this);
          GfErrTypeThrowException("Commit Failed in prepare",
                                  GF_COMMIT_CONFLICT_EXCEPTION);
          break;
        }
        default: {
          TXCleaner txCleaner(this);
          LOG_ERROR("Unknown message type in prepare reply %d",
                    replyCommitBefore.getMessageType());
          GfErrTypeThrowException("Commit Failed in prepare", GF_MSG);
          break;
        }
      }
    }
  } catch (const Exception& ex) {
    LOG_ERROR("Unexpected exception during commit in prepare %s", ex.what());
    throw;
  }
}

void InternalCacheTransactionManager2PCImpl::commit() {
  LOG_FINEST("Committing");
  this->afterCompletion(STATUS_COMMITTED);
}

void InternalCacheTransactionManager2PCImpl::rollback() {
  LOG_FINEST("Rolling back");
  this->afterCompletion(STATUS_ROLLEDBACK);
}

void InternalCacheTransactionManager2PCImpl::afterCompletion(int32_t status) {
  try {
    auto txState = TSSTXStateWrapper::get().getTXState();

    if (!txState) {
      GfErrTypeThrowException(
          "Transaction is null, cannot commit a null transaction",
          GF_CACHE_ILLEGAL_STATE_EXCEPTION);
      return;  // never called
    }

    auto tcr_dm = getDM();
    // This is for the case when no cache operation/s is performed between
    // tx->begin() and tx->commit()/rollback(),
    // simply return without sending COMMIT message to server. tcr_dm is nullptr
    // implies no cache operation is performed.
    // Theres no need to call txCleaner.clean(); here, because TXCleaner
    // destructor is called which cleans ThreadLocal.
    if (!tcr_dm) {
      TXCleaner txCleaner(this);
      return;
    }

    if (!txState->isPrepared()) {
      // Fallback to default 1PC commit
      // The inherited 1PC implementation clears the transaction state
      switch (status) {
        case STATUS_COMMITTED:
          CacheTransactionManagerImpl::commit();
          break;
        case STATUS_ROLLEDBACK:
          CacheTransactionManagerImpl::rollback();
          break;
        default:
          GfErrTypeThrowException("Unknown command",
                                  GF_CACHE_ILLEGAL_STATE_EXCEPTION);
      }
      return;
    }

    // In 2PC we always clear the transaction state
    TXCleaner txCleaner(this);

    TcrMessageTxSynchronization requestCommitAfter(
        new DataOutput(
            tcr_dm->getConnectionManager().getCacheImpl()->createDataOutput()),
        AFTER_COMMIT, txState->getTransactionId().getId(), status);

    TcrMessageReply replyCommitAfter(true, nullptr);
    GfErrType err =
        tcr_dm->sendSyncRequest(requestCommitAfter, replyCommitAfter);

    if (err != GF_NOERR) {
      GfErrTypeThrowException("Error in 2PC commit", err);
    } else {
      switch (replyCommitAfter.getMessageType()) {
        case TcrMessage::RESPONSE: {
          auto commit = std::dynamic_pointer_cast<TXCommitMessage>(
              replyCommitAfter.getValue());
          if (commit) {
            // e.g. when afterCompletion(STATUS_ROLLEDBACK) called
            txCleaner.clean();
            commit->apply(this->getCache());
          }
          break;
        }
        case TcrMessage::EXCEPTION: {
          const auto& exceptionMsg = replyCommitAfter.getException();
          err = ThinClientRegion::handleServerException(
              "CacheTransactionManager::afterCompletion", exceptionMsg);
          GfErrTypeThrowException("2PC Commit Failed", err);
          break;
        }
        case TcrMessage::REQUEST_DATA_ERROR:
          GfErrTypeThrowException("2PC Commit Failed",
                                  GF_COMMIT_CONFLICT_EXCEPTION);
          break;
        default:
          GfErrTypeThrowException("2PC Commit Failed", GF_MSG);
          break;
      }
    }
  } catch (const Exception& ex) {
    LOG_ERROR("Unexpected exception during completing transaction %s",
              ex.what());
    throw;
  }
}
}  // namespace client
}  // namespace geode
}  // namespace apache
