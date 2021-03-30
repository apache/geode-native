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

#include <iostream>

#include <ace/Synch.h>

#include "fw_helper.hpp"

class ThreadAcquire : public ACE_Task_Base {
 public:
  ThreadAcquire(ACE_Thread_Semaphore &sema, int acquireSecs)
      : ACE_Task_Base(),
        m_sema(sema),
        m_acquireSecs(acquireSecs),
        m_status(0) {}

  int svc() override {
    auto start = std::chrono::steady_clock::now();
    ACE_Time_Value expireAt =
        ACE_Time_Value{time(nullptr)} + ACE_Time_Value{m_acquireSecs};

    std::cout << "Thread acquiring lock at "
              << std::chrono::time_point_cast<std::chrono::milliseconds>(start)
                     .time_since_epoch()
                     .count()
              << "msecs" << std::endl;

    if (m_sema.acquire(expireAt) == 0) {
      auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::steady_clock::now() - start)
                          .count();
      std::cout << "Thread acquired lock after " << interval << "msecs"
                << std::endl;
      m_status = 0;
    } else {
      auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::steady_clock::now() - start)
                          .count();

      std::cout << "Thread failed to acquire lock after " << interval << "msecs"
                << std::endl;
      m_status = -1;
    }
    return m_status;
  }

  int getStatus() { return m_status; }

 private:
  ACE_Thread_Semaphore &m_sema;
  int m_acquireSecs;
  int m_status;
};

BEGIN_TEST(CheckTimedAcquire)
  {
    ACE_Thread_Semaphore sema(1);
    ThreadAcquire *thread = new ThreadAcquire(sema, 10);

    sema.acquire();
    thread->activate();

    LOG("Sleeping for 8 secs.");
    std::this_thread::sleep_for(std::chrono::seconds(8));
    ASSERT(thread->thr_count() == 1, "Expected thread to be running.");
    sema.release();
    SLEEP(50);  // Sleep for a few millis for the thread to end.
    ASSERT(thread->thr_count() == 0, "Expected no thread to be running.");
    ASSERT(thread->wait() == 0, "Expected successful end of thread.");
    ASSERT(thread->getStatus() == 0, "Expected zero exit status from thread.");

    delete thread;
  }
END_TEST(CheckTimedAcquire)

BEGIN_TEST(CheckTimedAcquireFail)
  {
    ACE_Thread_Semaphore sema(0);
    ThreadAcquire *thread = new ThreadAcquire(sema, 10);

    thread->activate();

    LOG("Sleeping for 8 secs.");
    std::this_thread::sleep_for(std::chrono::seconds(8));
    ASSERT(thread->thr_count() == 1, "Expected thread to be running.");
    std::this_thread::sleep_for(std::chrono::seconds(3));
    ASSERT(thread->thr_count() == 0, "Expected no thread to be running.");
    ASSERT(thread->wait() == 0, "Expected successful end of thread.");
    ASSERT(thread->getStatus() == -1,
           "Expected non-zero exit status from thread.");

    delete thread;
  }
END_TEST(CheckTimedAcquireFail)

BEGIN_TEST(CheckNoWait)
  {
    ACE_Thread_Semaphore sema(0);
    ThreadAcquire *thread = new ThreadAcquire(sema, 10);

    sema.release();
    thread->activate();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT(thread->thr_count() == 0, "Expected no thread to be running.");
    ASSERT(thread->wait() == 0, "Expected successful end of thread.");
    ASSERT(thread->getStatus() == 0, "Expected zero exit status from thread.");

    delete thread;
  }
END_TEST(CheckNoWait)

BEGIN_TEST(CheckResetAndTimedAcquire)
  {
    ACE_Thread_Semaphore sema(1);
    ThreadAcquire *thread = new ThreadAcquire(sema, 10);

    sema.acquire();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    sema.release();
    sema.release();
    sema.release();
    while (sema.tryacquire() != -1) {
    }
    thread->activate();

    LOG("Sleeping for 8 secs.");
    std::this_thread::sleep_for(std::chrono::seconds(8));
    ASSERT(thread->thr_count() == 1, "Expected thread to be running.");
    sema.release();
    SLEEP(50);  // Sleep for a few millis for the thread to end.
    ASSERT(thread->thr_count() == 0, "Expected no thread to be running.");
    ASSERT(thread->wait() == 0, "Expected successful end of thread.");
    ASSERT(thread->getStatus() == 0, "Expected zero exit status from thread.");

    delete thread;
  }
END_TEST(CheckResetAndTimedAcquire)
