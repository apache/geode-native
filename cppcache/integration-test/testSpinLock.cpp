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

#include "fw_dunit.hpp"

#include <mutex>

#include "internal/concurrent/binary_semaphore.hpp"
#include "internal/concurrent/spinlock_mutex.hpp"

namespace {  // NOLINT(google-build-namespaces)

using apache::geode::client::binary_semaphore;
using apache::geode::util::concurrent::spinlock_mutex;

DUNIT_TASK(s1p1, Basic)
  {
    spinlock_mutex s;
    { std::lock_guard<spinlock_mutex> lk(s); }
  }
END_TASK(Basic)

spinlock_mutex lock;
std::chrono::steady_clock::time_point btime;

class ThreadA {
 public:
  ThreadA(binary_semaphore& triggerA, binary_semaphore& triggerM)
      : triggerA_{triggerA}, triggerM_{triggerM} {}

  ~ThreadA() { stop(); }

  void start() {
    thread_ = std::thread{[this]() { run(); }};
  }

  void stop() {
    if (thread_.joinable()) {
      thread_.join();
    }
  }

 protected:
  void run() {
    std::lock_guard<spinlock_mutex> lk(lock);
    LOG("ThreadA: Acquired lock x.");
    triggerM_.release();
    triggerA_.acquire();

    LOG("ThreadA: Released lock.");
  }

 protected:
  std::thread thread_;
  binary_semaphore& triggerA_;
  binary_semaphore& triggerM_;
};

class ThreadB {
 public:
  ThreadB(binary_semaphore& triggerB, binary_semaphore& triggerM)
      : triggerB_{triggerB}, triggerM_{triggerM} {}

  ~ThreadB() { stop(); }

  void start() {
    thread_ = std::thread{[this]() { run(); }};
  }

  void stop() {
    if (thread_.joinable()) {
      thread_.join();
    }
  }

 protected:
  void run() {
    triggerB_.acquire();

    std::lock_guard<spinlock_mutex> lk(lock);
    btime = std::chrono::steady_clock::now();
    LOG("ThreadB: Acquired lock.");
    triggerM_.release();
  }

 protected:
  std::thread thread_;
  binary_semaphore& triggerB_;
  binary_semaphore& triggerM_;
};

DUNIT_TASK(s1p1, TwoThreads)
  {
    binary_semaphore triggerA{0};
    binary_semaphore triggerB{0};
    binary_semaphore triggerM{0};

    ThreadA threadA{triggerA, triggerM};
    ThreadB threadB{triggerB, triggerM};

    threadA.start();
    threadB.start();

    // A runs, locks the spinlock, and triggers me. B is idle.
    triggerM.acquire();
    // A is now idle, but holds lock. Tell B to acquire the lock
    auto stime = std::chrono::steady_clock::now();
    triggerB.release();
    SLEEP(5000);
    // B will be stuck until we tell A to release it.
    triggerA.release();
    // wait until B tells us it has acquired the lock.
    triggerM.acquire();

    // Now diff btime (when B acquired the lock) and stime to see that it
    // took longer than the 5000 seconds before A released it.
    auto delta =
        std::chrono::duration_cast<std::chrono::milliseconds>(btime - stime)
            .count();

    LOG("acquire delay was " + std::to_string(delta));
    ASSERT(delta >= 4900, "Expected 5 second or more spinlock delay");
    // Note the test is against 4900 instead of 5000 as there are some
    // measurement
    // issues. Often delta comes back as 4999 on linux.

    threadA.stop();
    threadB.stop();
  }
END_TASK(TwoThreads)

}  // namespace
