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

#ifndef GEODE_CACHETRANSACTIONMANAGERIMPL_H_
#define GEODE_CACHETRANSACTIONMANAGERIMPL_H_

#include <condition_variable>
#include <mutex>

#include <geode/CacheTransactionManager.hpp>

#include "TXCommitMessage.hpp"
#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

enum status { STATUS_COMMITTED = 3, STATUS_ROLLEDBACK = 4 };
enum commitOp { BEFORE_COMMIT, AFTER_COMMIT };

class ThinClientPoolDM;

class CacheTransactionManagerImpl : public virtual CacheTransactionManager {
 public:
  explicit CacheTransactionManagerImpl(CacheImpl* cache);
  virtual ~CacheTransactionManagerImpl() override;

  virtual void begin() override;
  virtual void commit() override;
  virtual void rollback() override;
  virtual bool exists() override;
  virtual TransactionId& suspend() override;
  virtual void resume(TransactionId& transactionId) override;
  virtual bool isSuspended(TransactionId& transactionId) override;
  virtual bool tryResume(TransactionId& transactionId) override;
  bool tryResume(TransactionId& transactionId, bool cancelExpiryTask);
  virtual bool tryResume(TransactionId& transactionId,
                         std::chrono::milliseconds waitTime) override;
  virtual bool exists(TransactionId& transactionId) override;

  virtual TransactionId& getTransactionId() override;

  TXState* getSuspendedTx(int32_t txId);

 protected:
  ThinClientPoolDM* getDM();
  Cache* getCache();

 private:
  CacheImpl* m_cache;

  void resumeTxUsingTxState(TXState* txState, bool cancelExpiryTask = true);
  GfErrType rollback(TXState* txState, bool callListener);
  void addSuspendedTx(int32_t txId, TXState* txState);
  TXState* removeSuspendedTx(int32_t txId);
  TXState* removeSuspendedTx(int32_t txId, std::chrono::milliseconds waitTime);
  bool isSuspendedTx(int32_t txId);
  void addTx(int32_t txId);
  void removeTx(int32_t txId);
  bool findTx(int32_t txId);

  std::map<int32_t, TXState*> m_suspendedTXs;
  std::recursive_mutex m_suspendedTxLock;
  std::vector<int32_t> m_TXs;
  std::recursive_mutex m_txLock;
  std::condition_variable_any m_txCond;

  friend class TXCleaner;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHETRANSACTIONMANAGERIMPL_H_
