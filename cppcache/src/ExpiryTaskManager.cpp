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

#include "ExpiryTaskManager.hpp"

#include "DistributedSystem.hpp"
#include "DistributedSystemImpl.hpp"
#include "config.h"
#include "util/Log.hpp"

#if defined(_WIN32)
#include <ace/WFMO_Reactor.h>
#endif
#if defined(WITH_ACE_Select_Reactor)
#include <ace/Select_Reactor.h>
#else
#include <ace/Dev_Poll_Reactor.h>
#endif

namespace apache {
namespace geode {
namespace client {

const char* ExpiryTaskManager::NC_ETM_Thread = "NC ETM Thread";

ExpiryTaskManager::ExpiryTaskManager() : m_reactorEventLoopRunning(false) {
  // NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.VirtualCall): ACE
  auto timer = new GF_Timer_Heap_ImmediateReset();
  m_timer = std::unique_ptr<GF_Timer_Heap_ImmediateReset>(timer);
#if defined(_WIN32)
  m_reactor = new ACE_Reactor(new ACE_WFMO_Reactor(nullptr, m_timer.get()), 1);
#elif defined(WITH_ACE_Select_Reactor)
  // NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.VirtualCall): ACE
  auto aceSelectReactor = new ACE_Select_Reactor(nullptr, m_timer.get());
  // NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.VirtualCall): ACE
  m_reactor = new ACE_Reactor(aceSelectReactor, 1);
#else
  m_reactor =
      new ACE_Reactor(new ACE_Dev_Poll_Reactor(nullptr, m_timer.get()) 1);
#endif
}

int ExpiryTaskManager::resetTask(ExpiryTaskManager::id_type id, uint32_t sec) {
  ACE_Time_Value interval(sec);
  return m_reactor->reset_timer_interval(id, interval);
}

int ExpiryTaskManager::cancelTask(ExpiryTaskManager::id_type id) {
  return m_reactor->cancel_timer(id, nullptr, 0);
}

int ExpiryTaskManager::svc() {
  DistributedSystemImpl::setThreadName(NC_ETM_Thread);
  LOGFINE("ExpiryTaskManager thread is running.");
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_reactorEventLoopRunning = true;
    m_condition.notify_all();
  }
  m_reactor->owner(ACE_OS::thr_self());
  m_reactor->run_reactor_event_loop();
  LOGFINE("ExpiryTaskManager thread has stopped.");
  return 0;
}

void ExpiryTaskManager::stopExpiryTaskManager() {
  std::unique_lock<std::mutex> lock(m_mutex);

  if (m_reactorEventLoopRunning) {
    m_reactor->end_reactor_event_loop();
    this->wait();
    m_reactorEventLoopRunning = false;
    m_condition.notify_all();
  }
}

void ExpiryTaskManager::begin() {
  this->activate();
  std::unique_lock<std::mutex> lock(m_mutex);
  m_condition.wait(lock, [this] { return m_reactorEventLoopRunning; });
}

ExpiryTaskManager::~ExpiryTaskManager() {
  stopExpiryTaskManager();

  delete m_reactor;
  m_reactor = nullptr;

  // NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.VirtualCall): ACE
}

}  // namespace client
}  // namespace geode
}  // namespace apache
