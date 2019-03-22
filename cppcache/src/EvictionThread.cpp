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
#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

const char* EvictionThread::NC_Evic_Thread = "NC Evic Thread";

EvictionThread::EvictionThread(EvictionController* parent)
    : m_run(false), m_pParent(parent) {}

void EvictionThread::start() {
  m_run = true;
  m_thread = std::thread(&EvictionThread::svc, this);

  LOGFINE("Eviction Thread started");
}

void EvictionThread::stop() {
  m_run = false;
  m_queueCondition.notify_one();
  m_thread.join();

  m_queue.clear();

  LOGFINE("Eviction Thread stopped");
}

void EvictionThread::svc(void) {
  DistributedSystemImpl::setThreadName(NC_Evic_Thread);

  while (m_run) {
    std::unique_lock<std::mutex> lock(m_queueMutex);
    m_queueCondition.wait(lock, [this] { return !m_run || !m_queue.empty(); });

    while (!m_queue.empty()) {
      auto percentageToEvict = m_queue.front();
      m_queue.pop_front();
      if (0 != percentageToEvict) {
        m_pParent->evict(percentageToEvict);
      }
    }
  }
}

void EvictionThread::putEvictionInfo(int32_t info) {
  std::unique_lock<std::mutex> lock(m_queueMutex);
  m_queue.push_back(info);
  m_queueCondition.notify_one();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
