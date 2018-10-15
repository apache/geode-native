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

#include "EvictionThread.hpp"

#include <chrono>

#include "DistributedSystemImpl.hpp"
#include "EvictionController.hpp"

namespace apache {
namespace geode {
namespace client {

const char* EvictionThread::NC_Evic_Thread = "NC Evic Thread";
EvictionThread::EvictionThread(EvictionController* parent)
    : m_pParent(parent), m_run(false) {}

int EvictionThread::svc(void) {
  DistributedSystemImpl::setThreadName(NC_Evic_Thread);
  while (m_run) {
    processEvictions();
  }
  auto size = m_queue.size();
  for (decltype(size) i = 0; i < size; i++) {
    processEvictions();
  }
  return 1;
}

void EvictionThread::processEvictions() {
  auto percentageToEvict = m_queue.getFor(std::chrono::microseconds(1500));
  if (percentageToEvict != 0) {
    m_pParent->evict(percentageToEvict);
  }
}

void EvictionThread::putEvictionInfo(int32_t info) { m_queue.put(info); }

}  // namespace client
}  // namespace geode
}  // namespace apache
