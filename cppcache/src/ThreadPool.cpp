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

#include "DistributedSystemImpl.hpp"

namespace apache {
namespace geode {
namespace client {

const char* ThreadPool::NC_Pool_Thread = "NC Pool Thread";

ThreadPool::ThreadPool(size_t threadPoolSize)
    : shutdown_(false), appDomainContext_(createAppDomainContext()) {
  workers_.reserve(threadPoolSize);

  std::function<void()> executeWork = [this] {
    DistributedSystemImpl::setThreadName(NC_Pool_Thread);
    while (true) {
      std::unique_lock<decltype(queueMutex_)> lock(queueMutex_);
      queueCondition_.wait(lock,
                           [this] { return shutdown_ || !queue_.empty(); });

      if (shutdown_) {
        break;
      }

      auto work = queue_.front();
      queue_.pop_front();

      lock.unlock();

      try {
        work->call();
      } catch (...) {
        // ignore
      }
    }
  };

  if (appDomainContext_) {
    executeWork = [executeWork, this] { appDomainContext_->run(executeWork); };
  }

  for (size_t i = 0; i < threadPoolSize; i++) {
    workers_.emplace_back(executeWork);
  }
}

ThreadPool::~ThreadPool() { shutDown(); }

void ThreadPool::perform(std::shared_ptr<Callable> req) {
  {
    std::lock_guard<decltype(queueMutex_)> lock(queueMutex_);
    queue_.push_back(std::move(req));
    if (queue_.size() > 1) {
      return;
    }
  }

  queueCondition_.notify_all();
}

void ThreadPool::shutDown(void) {
  {
    std::lock_guard<decltype(queueMutex_)> lock(queueMutex_);
    if (shutdown_) {
      return;
    }
    shutdown_ = true;
  }

  queueCondition_.notify_all();

  for (auto& worker : workers_) {
    worker.join();
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
