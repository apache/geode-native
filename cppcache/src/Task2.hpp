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

#ifndef GEODE_Task2_H_
#define GEODE_Task2_H_

#include <atomic>
#include <memory>
#include <thread>

#include "AppDomainContext.hpp"
#include "DistributedSystemImpl.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * Re-implementation of the Task class based on std::thread rather then
 * ACE_Task.
 */
template <class T>
class Task2 {
 public:
  /// Handle timeout events.
  typedef void (T::*OPERATION)(std::atomic<bool>& isRunning);

  // op_handler is the receiver of the timeout event. timeout is the method to
  // be executed by op_handler_
  inline Task2(T* op_handler, OPERATION op, const char* tn)
      : op_handler_(op_handler),
        m_op(op),
        m_run(false),
        m_threadName(tn),
        m_appDomainContext(createAppDomainContext()) {}

  inline ~Task2() noexcept = default;

  inline void start() {
    m_run = true;
    m_thread = std::thread(&Task2::svc, this);
  }

  inline void stop() {
    if (m_run) {
      m_run = false;
      m_thread.join();
    }
  }

  inline void stopNoblock() { m_run = false; }

  inline void svc(void) {
    DistributedSystemImpl::setThreadName(m_threadName);

    if (m_appDomainContext) {
      m_appDomainContext->run(
          [this]() { (this->op_handler_->*this->m_op)(this->m_run); });
    } else {
      (this->op_handler_->*m_op)(m_run);
    }
  }

  inline void wait() { m_thread.join(); }

 private:
  std::thread m_thread;
  T* op_handler_;
  OPERATION m_op;
  std::atomic<bool> m_run;
  const char* m_threadName;
  std::unique_ptr<AppDomainContext> m_appDomainContext;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_Task2_H_
