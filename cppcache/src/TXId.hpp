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

#ifndef GEODE_TXID_H_
#define GEODE_TXID_H_

#include <atomic>

#include <geode/DataOutput.hpp>
#include <geode/TransactionId.hpp>

namespace apache {
namespace geode {
namespace client {

class TXId : public apache::geode::client::TransactionId {
 public:
  TXId();

  TXId& operator=(const TXId&);

  ~TXId() noexcept override;

  int32_t getId();

  // This method is only for testing and should not be used for any
  // other purpose. See TXIdTest.cpp for more details.
  static void setInitalTransactionIDValue(int32_t);

 private:
  int32_t m_TXId;
  static std::atomic<int32_t> m_transactionId;
  TXId(const TXId&);
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TXID_H_
