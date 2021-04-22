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
#include <condition_variable>
#include <util/concurrent/spinlock_mutex.hpp>

#include <ace/Task.h>
#include <ace/Time_Value.h>
#include <ace/Guard_T.h>

namespace {  // NOLINT(google-build-namespaces)

class semaphore {
 public:
  explicit semaphore(bool released) : released_(released) {}

  void release() {
    std::lock_guard<std::mutex> lock(mutex_);
    released_ = true;
    cv_.notify_one();
  }

  void acquire() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]() { return released_; });
    released_ = false;
  }

  semaphore& operator=(const semaphore& other) {
    released_ = other.released_;
    return *this;
  }

 protected:
  bool released_;
  std::mutex mutex_;
  std::condition_variable cv_;
};

using apache::geode::util::concurrent::spinlock_mutex;

DUNIT_TASK(s1p1, Basic)
  {
    spinlock_mutex s;
    { std::lock_guard<spinlock_mutex> lk(s); }
  }
END_TASK(Basic)

semaphore triggerA{0};
semaphore triggerB{0};
semaphore triggerM{0};

spinlock_mutex lock;
std::chrono::steady_clock::time_point btime;

class ThreadA : public ACE_Task_Base {
 public:
  ThreadA() : ACE_Task_Base() {}

  int svc() override {
    {
      std::lock_guard<spinlock_mutex> lk(lock);
      LOG("ThreadA: Acquired lock x.");
      triggerM.release();
      triggerA.acquire();
    }
    LOG("ThreadA: Released lock.");
    return 0;
  }
};

class ThreadB : public ACE_Task_Base {
 public:
  ThreadB() : ACE_Task_Base() {}

  int svc() override {
    triggerB.acquire();
    {
      std::lock_guard<spinlock_mutex> lk(lock);
      btime = std::chrono::steady_clock::now();
      LOG("ThreadB: Acquired lock.");
      triggerM.release();
    }
    return 0;
  }
};

DUNIT_TASK(s1p1, TwoThreads)
  {
    triggerA = semaphore{0};
    triggerB = semaphore{0};
    triggerM = semaphore{0};

    ThreadA* threadA = new ThreadA();
    ThreadB* threadB = new ThreadB();

    threadA->activate();
    threadB->activate();

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
    std::string msg = "acquire delay was " + std::to_string(delta);

    LOG(msg);
    ASSERT(delta >= 4900, "Expected 5 second or more spinlock delay");
    // Note the test is against 4900 instead of 5000 as there are some
    // measurement
    // issues. Often delta comes back as 4999 on linux.

    threadA->wait();
    delete threadA;
    threadB->wait();
    delete threadB;
  }
END_TASK(TwoThreads)

}  // namespace
