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

#ifndef GEODE_CACHETRANSACTIONMANAGER_H_
#define GEODE_CACHETRANSACTIONMANAGER_H_

//#### Warning: DO NOT directly include Region.hpp, include Cache.hpp instead.

#include <chrono>
#include <memory>
#include "geode_globals.hpp"

namespace apache {
namespace geode {
namespace client {
class TransactionId;

class CPPCACHE_EXPORT CacheTransactionManager {
 public:
  /** Creates a new transaction and associates it with the current thread.
   *
   * @throws IllegalStateException if the thread is already associated with a
   * transaction
   *
   * @since 3.6
   */
  virtual void begin() = 0;

  /** Commit the transaction associated with the current thread. If
   *  the commit operation fails due to a conflict it will destroy
   *  the transaction state and throw a {@link
   *  CommitConflictException}.  If the commit operation succeeds,
   *  it returns after the transaction state has been merged with
   *  committed state.  When this method completes, the thread is no
   *  longer associated with a transaction.
   *
   * @throws IllegalStateException if the thread is not associated with a
   * transaction
   *
   * @throws CommitConflictException if the commit operation fails due to
   *   a write conflict.
   *
   * @throws TransactionDataNodeHasDepartedException if the node hosting the
   * transaction data has departed. This is only relevant for transaction that
   * involve PartitionedRegions.
   *
   * @throws TransactionDataNotColocatedException if at commit time, the data
   * involved in the transaction has moved away from the transaction hosting
   * node. This can only happen if rebalancing/recovery happens during a
   * transaction that involves a PartitionedRegion.
   *
   * @throws TransactionInDoubtException when Geode cannot tell which nodes
   * have applied the transaction and which have not. This only occurs if nodes
   * fail mid-commit, and only then in very rare circumstances.
   */
  virtual void commit() = 0;

  /** Roll back the transaction associated with the current thread. When
   *  this method completes, the thread is no longer associated with a
   *  transaction and the transaction context is destroyed.
   *
   * @since 3.6
   *
   * @throws IllegalStateException if the thread is not associated with a
   * transaction
   */
  virtual void rollback() = 0;

  /**
   * Suspends the transaction on the current thread. All subsequent operations
   * performed by this thread will be non-transactional. The suspended
   * transaction can be resumed by calling {@link #resume(TransactionId)}
   *
   * @return the transaction identifier of the suspended transaction or null if
   *         the thread was not associated with a transaction
   * @since 3.6.2
   */
  virtual std::shared_ptr<TransactionId> suspend() = 0;

  /**
   * On the current thread, resumes a transaction that was previously suspended
   * using {@link #suspend()}
   *
   * @param transactionId
   *          the transaction to resume
   * @throws IllegalStateException
   *           if the thread is associated with a transaction or if
   *           {@link #isSuspended(TransactionId)} would return false for the
   *           given transactionId
   * @since 3.6.2
   */
  virtual void resume(std::shared_ptr<TransactionId> transactionId) = 0;

  /**
   * This method can be used to determine if a transaction with the given
   * transaction identifier is currently suspended locally. This method does not
   * check other members for transaction status.
   *
   * @param transactionId
   * @return true if the transaction is in suspended state, false otherwise
   * @since 3.6.2
   * @see #exists(TransactionId)
   */
  virtual bool isSuspended(std::shared_ptr<TransactionId> transactionId) = 0;

  /**
   * On the current thread, resumes a transaction that was previously suspended
   * using {@link #suspend()}.
   *
   * This method is equivalent to
   * <pre>
   * if (isSuspended(txId)) {
   *   resume(txId);
   * }
   * </pre>
   * except that this action is performed atomically
   *
   * @param transactionId
   *          the transaction to resume
   * @return true if the transaction was resumed, false otherwise
   * @since 3.6.2
   */
  virtual bool tryResume(std::shared_ptr<TransactionId> transactionId) = 0;

  /**
   * On the current thread, resumes a transaction that was previously suspended
   * using {@link #suspend()}, or waits for the specified timeout interval if
   * the transaction has not been suspended. This method will return if:
   * <ul>
   * <li>Another thread suspends the transaction</li>
   * <li>Another thread calls commit/rollback on the transaction</li>
   * <li>This thread has waited for the specified timeout</li>
   * </ul>
   *
   * This method returns immediately if {@link #exists(TransactionId)} returns
   * false.
   *
   * @param transactionId
   *          the transaction to resume
   * @param waitTime
   *          the maximum time to wait
   * @return true if the transaction was resumed, false otherwise
   * @since 3.6.2
   * @see #tryResume(TransactionId)
   */
  virtual bool tryResume(std::shared_ptr<TransactionId> transactionId,
                         std::chrono::milliseconds waitTime) = 0;

  /**
   * Reports the existence of a transaction for the given transactionId. This
   * method can be used to determine if a transaction with the given transaction
   * identifier is currently in progress locally.
   *
   * @param transactionId
   *          the given transaction identifier
   * @return true if the transaction is in progress, false otherwise.
   * @since 3.6.2
   * @see #isSuspended(TransactionId)
   */
  virtual bool exists(std::shared_ptr<TransactionId> transactionId) = 0;

  /** Returns the transaction identifier for the current thread
   *
   * @return the transaction identifier or null if no transaction exists
   *
   * @since 3.6.2
   */
  virtual std::shared_ptr<TransactionId> getTransactionId() = 0;

  /** Reports the existence of a Transaction for this thread
   *
   * @return true if a transaction exists, false otherwise
   *
   * @since 3.6
   */
  virtual bool exists() = 0;

 protected:
  CacheTransactionManager();
  virtual ~CacheTransactionManager();
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHETRANSACTIONMANAGER_H_
