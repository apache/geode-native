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

#ifndef GEODE_THREADPOOL_H_
#define GEODE_THREADPOOL_H_

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

#include "AppDomainContext.hpp"

namespace apache {
namespace geode {
namespace client {

class Callable {
 public:
  virtual ~Callable() noexcept = default;
  virtual void call() = 0;
};

template <class T>
class PooledWork : public Callable {
 private:
  T m_retVal;
  std::recursive_mutex m_mutex;
  std::condition_variable_any m_cond;
  bool m_done;

 public:
  PooledWork() : m_mutex(), m_cond(), m_done(false) {}

  ~PooledWork() noexcept override = default;

  void call() override {
    T res = execute();

    std::lock_guard<decltype(m_mutex)> lock(m_mutex);

    m_retVal = res;
    m_done = true;
    m_cond.notify_all();
  }

  T getResult(void) {
    std::unique_lock<decltype(m_mutex)> lock(m_mutex);

    while (!m_done) {
      m_cond.wait(lock, [this] { return m_done; });
    }

    return m_retVal;
  }

 protected:
  virtual T execute(void) = 0;
};

class ThreadPool {
 public:
  explicit ThreadPool(size_t threadPoolSize);
  ~ThreadPool();

  void perform(std::shared_ptr<Callable> req);

  void shutDown(void);

 private:
  bool shutdown_;
  std::vector<std::thread> workers_;
  std::deque<std::shared_ptr<Callable>> queue_;
  std::mutex queueMutex_;
  std::condition_variable queueCondition_;
  static const char* NC_Pool_Thread;
  AppDomainContext* appDomainContext_;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_THREADPOOL_H_
