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

#include "ThreadPool.hpp"

#include <geode/SystemProperties.hpp>

#include "CacheImpl.hpp"
#include "DistributedSystem.hpp"
#include "DistributedSystemImpl.hpp"

namespace apache {
namespace geode {
namespace client {

ThreadPoolWorker::ThreadPoolWorker(IThreadPool* manager)
    : manager_(manager), queue_(msg_queue()), shutdown_(0) {
#if defined(_MACOSX)
  threadId_ = nullptr;
#else
  threadId_ = 0;
#endif
}

ThreadPoolWorker::~ThreadPoolWorker() { shutDown(); }

int ThreadPoolWorker::perform(ACE_Method_Request* req) {
  ACE_TRACE(ACE_TEXT("Worker::perform"));
  return this->queue_.enqueue(req);
}

int ThreadPoolWorker::svc(void) {
  threadId_ = ACE_Thread::self();
  while (1) {
    ACE_Method_Request* request = this->queue_.dequeue();
    if (request == nullptr) {
      shutDown();
      break;
    }

    // Invoke the request
    request->call();

    // Return to work.
    this->manager_->returnToWork(this);
  }
  return 0;
}

int ThreadPoolWorker::shutDown(void) {
  if (shutdown_ != 1) {
    queue_.queue()->close();
    wait();
    shutdown_ = 1;
  }

  return shutdown_;
}

ACE_thread_t ThreadPoolWorker::threadId(void) { return threadId_; }

ThreadPool::ThreadPool(uint32_t threadPoolSize)
    : poolSize_(threadPoolSize), shutdown_(0) {
  activate();
}

ThreadPool::~ThreadPool() { shutDown(); }

int ThreadPool::perform(ACE_Method_Request* req) {
  return this->queue_.enqueue(req);
}

const char* ThreadPool::NC_Pool_Thread = "NC Pool Thread";
int ThreadPool::svc(void) {
  DistributedSystemImpl::setThreadName(NC_Pool_Thread);
  // Create pool when you get in the first time.
  createWorkerPool();
  while (!done()) {
    // Get the next message
    ACE_Method_Request* request = this->queue_.dequeue();
    if (request == nullptr) {
      shutDown();
      break;
    }
    // Choose a worker.
    ThreadPoolWorker* worker = chooseWorker();
    // Ask the worker to do the job.
    worker->perform(request);
  }
  return 0;
}

int ThreadPool::shutDown(void) {
  if (shutdown_ != 1) {
    queue_.queue()->close();
    wait();
    shutdown_ = 1;
  }

  return shutdown_;
}

int ThreadPool::returnToWork(ThreadPoolWorker* worker) {
  std::unique_lock<decltype(workersLock_)> lock(workersLock_);
  workers_.enqueue_tail(worker);
  workersCond_.notify_one();
  return 0;
}

ThreadPoolWorker* ThreadPool::chooseWorker(void) {
  std::unique_lock<decltype(workersLock_)> lock(workersLock_);
  if (workers_.is_empty()) {
    workersCond_.wait(lock, [this] { return !workers_.is_empty(); });
  }
  ThreadPoolWorker* worker;
  this->workers_.dequeue_head(worker);
  return worker;
}

int ThreadPool::createWorkerPool(void) {
  std::unique_lock<decltype(workersLock_)> lock(workersLock_);
  for (int i = 0; i < poolSize_; i++) {
    ThreadPoolWorker* worker;
    ACE_NEW_RETURN(worker, ThreadPoolWorker(this), -1);
    this->workers_.enqueue_tail(worker);
    worker->activate();
  }
  return 0;
}

int ThreadPool::done(void) { return (shutdown_ == 1); }

}  // namespace client
}  // namespace geode
}  // namespace apache
