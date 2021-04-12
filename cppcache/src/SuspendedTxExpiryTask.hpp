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

#ifndef GEODE_SUSPENDEDTXEXPIRYTASK_H_
#define GEODE_SUSPENDEDTXEXPIRYTASK_H_

#include <geode/Cache.hpp>
#include <geode/internal/geode_globals.hpp>

#include "ExpiryTask.hpp"

namespace apache {
namespace geode {
namespace client {

class TransactionId;
class CacheTransactionManagerImpl;

/**
 * @class SuspendedTxExpiryTask
 *
 * The task gets triggered whenever a suspended transaction expires.
 *
 */
class SuspendedTxExpiryTask : public ExpiryTask {
 public:
  SuspendedTxExpiryTask(ExpiryTaskManager& expiry_manager,
                        CacheTransactionManagerImpl& tx_manager,
                        TransactionId& tx_id);

 protected:
  bool on_expire() override;

 protected:
  CacheTransactionManagerImpl& tx_manager_;
  TransactionId& tx_id_;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_SUSPENDEDTXEXPIRYTASK_H_
