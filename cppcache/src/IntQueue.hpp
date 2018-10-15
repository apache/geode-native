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

#ifndef GEODE_INTQUEUE_H_
#define GEODE_INTQUEUE_H_

#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>

namespace apache {
namespace geode {
namespace client {

template <class T>
class APACHE_GEODE_EXPORT IntQueue {
 public:
  inline IntQueue() = default;

  inline ~IntQueue() noexcept = default;

  T get() {
    std::unique_lock<decltype(m_mutex)> _guard(m_mutex);
    T mp = 0;
    getInternal(mp);
    return mp;
  }

  template <class _Rep, class _Period>
  T getFor(const std::chrono::duration<_Rep, _Period>& duration) {
    std::unique_lock<decltype(m_mutex)> _guard(m_mutex);
    T mp = 0;
    if (!getInternal(mp)) {
      if (m_cond.wait_for(_guard, duration,
                          [this] { return !m_queue.empty(); })) {
        mp = m_queue.back();
        m_queue.pop_back();
      }
    }
    return mp;
  }

  void put(T mp) {
    std::unique_lock<decltype(m_mutex)> _guard(m_mutex);
    m_queue.push_front(mp);
    if (m_queue.size() == 1) {
      m_cond.notify_one();
    }
  }

  size_t size() {
    std::unique_lock<decltype(m_mutex)> _guard(m_mutex);
    return m_queue.size();
  }

  void clear() {
    std::unique_lock<decltype(m_mutex)> _guard(m_mutex);
    m_queue.clear();
  }

  bool empty() {
    std::unique_lock<decltype(m_mutex)> _guard(m_mutex);
    return m_queue.empty();
  }

 private:
  inline bool getInternal(T& val) {
    if (m_queue.size() > 0) {
      val = m_queue.back();
      m_queue.pop_back();
      return true;
    }
    return false;
  }

  std::deque<T> m_queue;
  std::mutex m_mutex;
  std::condition_variable m_cond;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_INTQUEUE_H_
