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

#ifndef GEODE_Task_H_
#define GEODE_Task_H_

#include <atomic>
#include <memory>
#include <thread>

#include "./util/Log.hpp"
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
class Task {
 public:
  typedef void (T::*Method)(std::atomic<bool>& isRunning);

  inline Task(T* target, Method method, const char* threadName)
      : target_(target),
        method_(method),
        threadName_(threadName),
        runnable_(false),
        appDomainContext_(createAppDomainContext()) {}

  inline ~Task() noexcept { stop(); }

  inline void start() {
    runnable_ = true;
    thread_ = std::thread(&Task::svc, this);
  }

  inline void stop() noexcept {
    stopNoblock();
    wait();
  }

  inline void stopNoblock() noexcept { runnable_ = false; }

  inline void wait() noexcept {
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  inline void svc(void) {
    Log::setThreadName(threadName_);

    if (appDomainContext_) {
      appDomainContext_->run(
          [this]() { (this->target_->*this->method_)(this->runnable_); });
    } else {
      (this->target_->*method_)(runnable_);
    }
  }

 private:
  std::thread thread_;
  T* target_;
  Method method_;
  const char* threadName_;
  std::atomic<bool> runnable_;
  std::unique_ptr<AppDomainContext> appDomainContext_;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_Task_H_
