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

#include "TXId.hpp"

namespace apache {
namespace geode {
namespace client {

std::atomic<int32_t> TXId::m_transactionId(0);

TXId::TXId() {
  // If m_transactionId has reached maximum value, then start again
  // from zero, and increment by one until first unused value is found.
  // This is done to avoid overflow of m_transactionId to negative value.
  auto current = m_transactionId.load();
  decltype(current) next;
  do {
    next = std::numeric_limits<decltype(current)>::max() == current
               ? 1
               : current + 1;
  } while (!m_transactionId.compare_exchange_strong(current, next));
  m_TXId = next;
}

TXId& TXId::operator=(const TXId& other) {
  m_TXId = other.m_TXId;
  return *this;
}

// This method is only for testing and should not be used for any
// other purpose. See TXIdTest.cpp for more details.
void TXId::setInitalTransactionIDValue(int32_t newTransactionID) {
  m_transactionId.exchange(newTransactionID);
}

TXId::~TXId() noexcept = default;

int32_t TXId::getId() { return m_TXId; }
}  // namespace client
}  // namespace geode
}  // namespace apache
