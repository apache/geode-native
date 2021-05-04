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

#include "TXState.hpp"

#include <geode/Cache.hpp>
#include <geode/CacheTransactionManager.hpp>

#include "CacheImpl.hpp"
#include "ThinClientPoolDM.hpp"
#include "TransactionalOperation.hpp"
#include "TssConnectionWrapper.hpp"
#include "util/exception.hpp"

namespace apache {
namespace geode {
namespace client {

TXState::TXState(CacheImpl* cacheImpl) {
  m_txId = TXId();
  m_closed = false;
  m_modSerialNum = 0;
  m_dirty = false;
  m_replay = false;
  m_prepared = false;
  m_cache = cacheImpl;
  /* adongre
   * Non-static class member "m_suspendedExpiryTaskId" is not initialized in
   * this constructor nor in any functions that it calls.
   */
  m_suspendedExpiryTaskId = 0;
  m_pooldm = nullptr;
}

TXState::~TXState() {}
TXId& TXState::getTransactionId() { return m_txId; }

int32_t TXState::nextModSerialNum() {
  m_modSerialNum += 1;
  return m_modSerialNum;
}
std::shared_ptr<Cacheable> TXState::replay(bool isRollback) {
  GfErrTypeThrowException("Replay is unsupported", GF_NOTSUP);
  int retryAttempts = 3;

  std::shared_ptr<Cacheable> result = nullptr;

  ReplayControl replayControl(this);
  m_dirty = false;
  // if retryAttempts < 0 we retry until there are no servers available
  for (int i = 0; (retryAttempts < 0) || (i < retryAttempts); i++) {
    // try {
    if (isRollback) {  // need to roll back these
      try {
        m_cache->getCacheTransactionManager()
            ->rollback();  // this.firstProxy.rollback(proxy.getTxId().getUniqId());
      } catch (const Exception& ex) {
        LOG_FINE(
            "caught exception when rolling back before retrying transaction "
            "%s",
            ex.what());
      }
    }
    m_txId = TXId();
    // LOG_FINE("retrying transaction after loss of state in server.  Attempt #"
    // + (i+1));
    try {
      for (const auto& operation : m_operations) {
        result = operation->replay(m_cache);
      }

      return result;
    } catch (const TransactionDataNodeHasDepartedException& ex) {
      LOG_DEBUG("Transaction exception:%s", ex.what());
      isRollback = false;
      // try again
    } catch (const TransactionDataRebalancedException& ex) {
      LOG_DEBUG("Transaction exception:%s", ex.what());
      isRollback = true;
      // try again
    }
  }

  GfErrTypeThrowException(
      "Unable to reestablish transaction context on servers", GF_EUNDEF);

  return nullptr;
}

void TXState::releaseStickyConnection() {
  // Since this is called during cleanup or through destructor, we should not
  // throw exception from here,
  // which can cause undefined cleanup.
  if (auto conn = TssConnectionWrapper::get().getConnection()) {
    if (auto dm = conn->getEndpointObject()->getPoolHADM()) {
      if (!dm->isSticky()) {
        LOG_FINE(
            "Release the sticky connection associated with the transaction");
        dm->releaseThreadLocalConnection();  // this will release connection now
      } else {
        LOG_FINE(
            "Pool with sticky connection - don't Release the sticky connection "
            "associated with the transaction");
        conn->setAndGetBeingUsed(false, false);  // just release connection now
      }
    }
  }
}
}  // namespace client
}  // namespace geode
}  // namespace apache
