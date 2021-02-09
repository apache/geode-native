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
#include <util/concurrent/spinlock_mutex.hpp>

#include <ace/Task.h>
#include <ace/Time_Value.h>
#include <ace/Guard_T.h>

namespace {  // NOLINT(google-build-namespaces)

using apache::geode::util::concurrent::spinlock_mutex;

DUNIT_TASK(s1p1, Basic)
  {
    spinlock_mutex s;
    { std::lock_guard<spinlock_mutex> lk(s); }
  }
END_TASK(Basic)

perf::Semaphore *triggerA;
perf::Semaphore *triggerB;
perf::Semaphore *triggerM;

spinlock_mutex lock;
ACE_Time_Value *btime;

class ThreadA : public ACE_Task_Base {
 public:
  ThreadA() : ACE_Task_Base() {}

  int svc() override {
    {
      std::lock_guard<spinlock_mutex> lk(lock);
      LOG("ThreadA: Acquired lock x.");
      triggerM->release();
      triggerA->acquire();
    }
    LOG("ThreadA: Released lock.");
    return 0;
  }
};

class ThreadB : public ACE_Task_Base {
 public:
  ThreadB() : ACE_Task_Base() {}

  int svc() override {
    triggerB->acquire();
    {
      std::lock_guard<spinlock_mutex> lk(lock);
      btime = new ACE_Time_Value(ACE_OS::gettimeofday());
      LOG("ThreadB: Acquired lock.");
      triggerM->release();
    }
    return 0;
  }
};

DUNIT_TASK(s1p1, TwoThreads)
  {
    triggerA = new perf::Semaphore(0);
    triggerB = new perf::Semaphore(0);
    triggerM = new perf::Semaphore(0);

    ThreadA *threadA = new ThreadA();
    ThreadB *threadB = new ThreadB();

    threadA->activate();
    threadB->activate();

    // A runs, locks the spinlock, and triggers me. B is idle.
    triggerM->acquire();
    // A is now idle, but holds lock. Tell B to acquire the lock
    ACE_Time_Value stime = ACE_OS::gettimeofday();
    triggerB->release();
    SLEEP(5000);
    // B will be stuck until we tell A to release it.
    triggerA->release();
    // wait until B tells us it has acquired the lock.
    triggerM->acquire();

    // Now diff btime (when B acquired the lock) and stime to see that it
    // took longer than the 5000 seconds before A released it.
    ACE_Time_Value delta = *btime - stime;
    char msg[1024];
    sprintf(msg, "acquire delay was %lu\n", delta.msec());
    LOG(msg);
    ASSERT(delta.msec() >= 4900, "Expected 5 second or more spinlock delay");
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
