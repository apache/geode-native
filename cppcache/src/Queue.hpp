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

#ifndef GEODE_QUEUE_H_
#define GEODE_QUEUE_H_

#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>

namespace apache {
namespace geode {
namespace client {

template <class T>
class Queue {
 public:
  /**
   * Constructor with parameter to specify whether the contained objects
   * should be deleted in the destructor, and maximum size of queue.
   */
  explicit Queue(bool deleteObjs = true, const uint32_t maxSize = 0)
      : m_deleteObjs(deleteObjs), m_maxSize(maxSize), m_closed(false) {}

  ~Queue() { close(); }

  T get() {
    std::unique_lock<decltype(m_mutex)> _guard(m_mutex);
    return getNoLock();
  }

  template <class _Rep, class _Period>
  T getFor(const std::chrono::duration<_Rep, _Period>& duration) {
    std::unique_lock<decltype(m_mutex)> _guard(m_mutex);
    auto&& mp = getNoLock();
    if (mp == nullptr) {
      m_cond.wait_for(_guard, duration, [this, &mp] {
        return !(m_closed || nullptr == (mp = getNoLock()));
      });
    }
    return mp;
  }

  bool put(T mp) {
    std::unique_lock<decltype(m_mutex)> _guard(m_mutex);
    if (m_maxSize > 0 && m_queue.size() >= m_maxSize) {
      return false;
    }
    return putNoLock(mp);
  }

  template <class _Rep, class _Period>
  bool putFor(T mp, const std::chrono::duration<_Rep, _Period>& duration) {
    if (m_maxSize > 0) {
      std::unique_lock<decltype(m_mutex)> _guard(m_mutex);
      if (m_queue.size() < m_maxSize) {
        return putNoLock(mp);
      }

      if (m_cond.wait_for(_guard, duration,
                          [this] { return m_queue.size() < m_maxSize; })) {
        return putNoLock(mp);
      }

      return false;
    } else {
      std::unique_lock<decltype(m_mutex)> _guard(m_mutex);
      return putNoLock(mp);
    }
  }

  void open() {
    std::unique_lock<decltype(m_mutex)> _guard(m_mutex);
    m_closed = false;
  }

  void close() {
    std::unique_lock<decltype(m_mutex)> _guard(m_mutex);

    if (m_deleteObjs) {
      while (m_queue.size() > 0) {
        auto&& mp = m_queue.back();
        m_queue.pop_back();
      }
    } else {
      m_queue.clear();
    }
    m_closed = true;
    m_cond.notify_all();
  }

  uint32_t size() {
    std::unique_lock<decltype(m_mutex)> _guard(m_mutex);
    return static_cast<uint32_t>(m_queue.size());
  }

  bool empty() { return (size() == 0); }

 private:
  inline T getNoLock() {
    T mp = nullptr;

    auto queueSize = static_cast<uint32_t>(m_queue.size());
    if (queueSize > 0) {
      mp = m_queue.back();
      m_queue.pop_back();
      // signal the waiting putter threads, if any
      if (m_maxSize > 0 && queueSize == m_maxSize) {
        m_cond.notify_one();
      }
    }
    return mp;
  }

  inline bool putNoLock(T mp) {
    if (!m_closed) {
      m_queue.push_front(mp);
      // signal the waiting getter threads, if any
      if (m_queue.size() == 1) {
        m_cond.notify_one();
      }
      return true;
    }
    return false;
  }

  std::deque<T> m_queue;
  std::recursive_mutex m_mutex;
  std::condition_variable_any m_cond;
  bool m_deleteObjs;
  const uint32_t m_maxSize;
  bool m_closed;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_QUEUE_H_
