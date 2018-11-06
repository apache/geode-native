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

#include "TXCleaner.hpp"

namespace apache {
namespace geode {
namespace client {

TXCleaner::TXCleaner(CacheTransactionManagerImpl* cacheTxMgr) {
  m_txState = TSSTXStateWrapper::get().getTXState();
  m_cacheTxMgr = cacheTxMgr;
}

TXCleaner::~TXCleaner() {
  clean();
  if (m_txState != nullptr) {
    m_txState->releaseStickyConnection();
    delete m_txState;
    m_txState = nullptr;
  }
}
void TXCleaner::clean() {
  if (m_txState != nullptr) {
    m_cacheTxMgr->removeTx(m_txState->getTransactionId().getId());
  }
  if (m_txState != nullptr) {
    TSSTXStateWrapper::get().setTXState(nullptr);
  }
}

TXState* TXCleaner::getTXState() {
  return TSSTXStateWrapper::get().getTXState();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
