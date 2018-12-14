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

ThreadPool::ThreadPool(size_t threadPoolSize) : shutdown_(false) {
  workers_.reserve(threadPoolSize);
  for (size_t i = 0; i < threadPoolSize; i++) {
    workers_.emplace_back([this] {
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
    });
  }
}

ThreadPool::~ThreadPool() { shutDown(); }

void ThreadPool::perform(std::shared_ptr<Callable> req) {
  std::unique_lock<decltype(queueMutex_)> lock(queueMutex_);
  auto wasEmpty = queue_.empty();
  queue_.push_back(std::move(req));
  lock.unlock();

  if (wasEmpty) {
    queueCondition_.notify_all();
  }
}

void ThreadPool::shutDown(void) {
  if (!shutdown_.exchange(true)) {
    queueCondition_.notify_all();
    for (auto& worker : workers_) {
      worker.join();
    }
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
