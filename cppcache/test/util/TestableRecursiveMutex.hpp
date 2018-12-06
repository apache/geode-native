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

#include <atomic>
#include <mutex>

class TestableRecursiveMutex {
 public:
  std::recursive_mutex mutex_;
  std::atomic<int32_t> recursive_depth_;
  std::atomic<int32_t> lock_count_;
  std::atomic<int32_t> unlock_count_;
  std::atomic<int32_t> try_lock_count_;

  TestableRecursiveMutex() noexcept
      : recursive_depth_(0),
        lock_count_(0),
        unlock_count_(0),
        try_lock_count_(0) {}

  void lock() {
    mutex_.lock();
    ++recursive_depth_;
    ++lock_count_;
  }

  void unlock() {
    mutex_.unlock();
    --recursive_depth_;
    ++unlock_count_;
  }

  bool try_lock() {
    bool locked = false;
    if ((locked = mutex_.try_lock())) {
      ++recursive_depth_;
    }

    ++try_lock_count_;
    return locked;
  }

  void resetCounters() {
    recursive_depth_ = 0;
    lock_count_ = 0;
    unlock_count_ = 0;
    try_lock_count_ = 0;
  }
};
