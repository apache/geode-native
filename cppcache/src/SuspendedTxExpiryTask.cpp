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
#include "SuspendedTxExpiryTask.hpp"

#include "CacheImpl.hpp"
#include "CacheTransactionManagerImpl.hpp"

namespace apache {
namespace geode {
namespace client {

SuspendedTxExpiryTask::SuspendedTxExpiryTask(
    ExpiryTaskManager& expiry_manager, CacheTransactionManagerImpl& tx_manager,
    TransactionId& tx_id)
    : ExpiryTask(expiry_manager), tx_manager_(tx_manager), tx_id_(tx_id) {}

bool SuspendedTxExpiryTask::on_expire() {
  LOG_DEBUG("Entered SuspendedTxExpiryTask");
  try {
    // resume the transaction and rollback it
    if (tx_manager_.tryResume(tx_id_, false)) {
      tx_manager_.rollback();
    }
  } catch (...) {
    // Ignore whatever exception comes
    LOG_FINE(
        "Error while rollbacking expired suspended transaction. Ignoring the "
        "error");
  }
  return 0;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
