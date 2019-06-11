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

#ifndef GEODE_TXSTATE_H_
#define GEODE_TXSTATE_H_

#include <string>
#include <vector>

#include "ExpiryTaskManager.hpp"
#include "TXId.hpp"
#include "TransactionalOperation.hpp"

namespace apache {
namespace geode {
namespace client {

class ThinClientPoolDM;
class TXState {
 public:
  TXState(CacheImpl* cacheImpl);
  virtual ~TXState();

  TXId& getTransactionId();
  bool isDirty();
  void setDirty();
  bool isReplay();
  bool isPrepared();
  void setPrepared();
  std::shared_ptr<Cacheable> replay(bool isRollback);
  void releaseStickyConnection();

  // This variable is used only when the transaction is suspended and resumed.
  // For using this variable somewhere else, care needs to be taken
  std::string getEPStr();
  void setEPStr(std::string ep);

  ThinClientPoolDM* getPoolDM();
  void setPoolDM(ThinClientPoolDM* dm);
  void setSuspendedExpiryTaskId(
      ExpiryTaskManager::id_type suspendedExpiryTaskId);
  ExpiryTaskManager::id_type getSuspendedExpiryTaskId();

 private:
  void startReplay();
  void endReplay();

 private:
  TXId m_txId;
  /**
   * Used to hand out modification serial numbers used to preserve
   * the order of operation done by this transaction.
   */
  int32_t m_modSerialNum;
  // A map of transaction state by Region
  bool m_closed;
  bool m_dirty;
  bool m_prepared;
  // This variable is used only when the transaction is suspended and resumed.
  // For using this variable somewhere else, care needs to be taken
  std::string epNameStr;
  int32_t nextModSerialNum();
  bool m_replay;
  std::vector<std::shared_ptr<TransactionalOperation>> m_operations;
  CacheImpl* m_cache;
  ThinClientPoolDM* m_pooldm;
  ExpiryTaskManager::id_type m_suspendedExpiryTaskId;
  class ReplayControl {
   public:
    ReplayControl(TXState* txState);
    virtual ~ReplayControl();

   private:
    TXState* m_txState;
  };
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TXSTATE_H_
