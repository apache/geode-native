#pragma once

#ifndef GEODE_THREADPOOL_H_
#define GEODE_THREADPOOL_H_

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
/*
 * ThreadPool.hpp
 *
 *  Created on: 16-Mar-2010
 *      Author: ankurs
 */

#include <ace/Task.h>
#include <ace/Method_Request.h>
//#include <ace/Future.h>
#include <ace/Activation_Queue.h>
#include <ace/Condition_T.h>
#include <ace/Singleton.h>
#include <ace/Guard_T.h>
#include <mutex>
#include <condition_variable>
namespace apache {
namespace geode {
namespace client {

template <class T>
class PooledWork : public ACE_Method_Request {
 private:
  T m_retVal;
  std::recursive_mutex m_mutex;
  std::condition_variable_any m_cond;
  bool m_done;

 public:
  PooledWork() : m_mutex(), m_cond(), m_done(false) {}

  virtual ~PooledWork() {}

  virtual int call(void) {
    T res = execute();

    std::lock_guard<decltype(m_mutex)> lock(m_mutex);

    m_retVal = res;
    m_done = true;
    m_cond.notify_all();

    return 0;
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

template <typename S, typename R = int>
class PooledWorkFP : public PooledWork<R> {
 public:
  typedef R (S::*OPERATION)(void);
  PooledWorkFP(S* op_handler, OPERATION op)
      : op_handler_(op_handler), m_op(op) {}
  virtual ~PooledWorkFP() {}

 protected:
  virtual R execute(void) { return (this->op_handler_->*m_op)(); }

 private:
  S* op_handler_;
  OPERATION m_op;
};

class ThreadPoolWorker;

class IThreadPool {
 public:
  virtual int returnToWork(ThreadPoolWorker* worker) = 0;
  virtual ~IThreadPool() {}
};

class ThreadPoolWorker : public ACE_Task<ACE_MT_SYNCH> {
 public:
  ThreadPoolWorker(IThreadPool* manager);
  virtual ~ThreadPoolWorker();
  int perform(ACE_Method_Request* req);
  int shutDown(void);

  virtual int svc(void);
  ACE_thread_t threadId(void);

 private:
  IThreadPool* manager_;
  ACE_thread_t threadId_;
  ACE_Activation_Queue queue_;
  int shutdown_;
};

class ThreadPool : public ACE_Task_Base, IThreadPool {
  friend class ACE_Singleton<ThreadPool, ACE_Recursive_Thread_Mutex>;

 public:
  ThreadPool(uint32_t threadPoolSize);
  virtual ~ThreadPool();
  int perform(ACE_Method_Request* req);
  int svc(void);
  int shutDown(void);
  virtual int returnToWork(ThreadPoolWorker* worker);

 private:
  ThreadPoolWorker* chooseWorker(void);
  int createWorkerPool(void);
  int done(void);
  ACE_thread_t threadId(ThreadPoolWorker* worker);

 private:
  int poolSize_;
  int shutdown_;
  ACE_Thread_Mutex workersLock_;
  ACE_Condition<ACE_Thread_Mutex> workersCond_;
  ACE_Unbounded_Queue<ThreadPoolWorker*> workers_;
  ACE_Activation_Queue queue_;
  static const char* NC_Pool_Thread;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_THREADPOOL_H_
