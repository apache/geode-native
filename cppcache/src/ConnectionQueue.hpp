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

#ifndef GEODE_SYNCHRONIZEDQUEUE_H_
#define GEODE_SYNCHRONIZEDQUEUE_H_

#include <condition_variable>
#include <deque>
#include <mutex>

#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

template <class T, class _Mutex = std::mutex>
class ConnectionQueue {
 public:
  ConnectionQueue() : closed_(false) {}

  virtual ~ConnectionQueue() {}

  /** get without wait */
  T* getNoWait() {
    bool isClosed;
    return popLocked(isClosed);
  }

  /** wait sec time until notified */
  T* getUntil(const std::chrono::microseconds& sec) {
    bool isClosed;
    auto mp = popLocked(isClosed);

    if (!mp && !isClosed) {
      mp = getLockedFor(sec, isClosed, static_cast<void*>(nullptr));
    }
    return mp;
  }

  void put(T* mp, bool openQueue) {
    bool delMp = false;
    {
      std::lock_guard<_Mutex> _guard(mutex_);
      {
        if (openQueue || !closed_) {
          queue_.push_front(mp);
          closed_ = false;
        } else {
          delMp = true;
        }
      }
      condition_.notify_one();
    }
    if (delMp && mp) {
      mp->close();
      delete mp;
    }
  }

  size_t size() const {
    std::lock_guard<_Mutex> _guard(mutex_);
    return static_cast<uint32_t>(queue_.size());
  }

  bool empty() const {
    std::lock_guard<_Mutex> _guard(mutex_);
    return queue_.empty();
  }

  void close() {
    {
      std::lock_guard<_Mutex> _guard(mutex_);

      closed_ = true;
      LOG_DEBUG("Internal fair queue size while closing is %zu", queue_.size());
      while (!queue_.empty()) {
        auto mp = queue_.back();
        queue_.pop_back();
        mp->close();
        delete mp;
        deleteAction();
      }
    }
    LOG_DEBUG("ConnectionQueue::close( ): queue closed ");
    condition_.notify_all();
  }

  void reset() {
    std::lock_guard<_Mutex> _guard(mutex_);
    closed_ = false;
  }

 private:
  std::condition_variable_any condition_;
  bool closed_;

  T* popLocked(bool& isClosed) {
    std::lock_guard<_Mutex> _guard(mutex_);
    return popNoLock(isClosed);
  }

  bool exclude(T*, void*) { return false; }

 protected:
  std::deque<T*> queue_;
  mutable _Mutex mutex_;

  inline T* popNoLock(bool& isClosed) {
    T* mp = nullptr;

    isClosed = closed_;
    if (!isClosed && queue_.size() > 0) {
      mp = queue_.back();
      queue_.pop_back();
    }
    return mp;
  }

  template <typename U>
  T* getLockedFor(const std::chrono::microseconds& duration, bool& isClosed,
                  U* excludeList = nullptr) {
    const auto until = std::chrono::steady_clock::now() + duration;
    T* mp = nullptr;

    std::unique_lock<_Mutex> lock(mutex_);
    while (condition_.wait_until(
        lock, until, [&]() { return closed_ || !queue_.empty(); })) {
      mp = popNoLock(isClosed);
      if (mp && excludeList) {
        if (exclude(mp, excludeList)) {
          mp->close();
          _GEODE_SAFE_DELETE(mp);
          mp = nullptr;
          deleteAction();
          continue;
        }
      }

      break;
    }

    return mp;
  }

  virtual void deleteAction() {}
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_SYNCHRONIZEDQUEUE_H_
