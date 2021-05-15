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

#include "ExpiryTaskManager.hpp"

#include <future>

#include "DistributedSystemImpl.hpp"
#include "util/Log.hpp"

namespace {
const char *NC_ETM_Thread = "NC ETM Thread";
}

namespace apache {
namespace geode {
namespace client {

ExpiryTaskManager::ExpiryTaskManager()
    : running_(false),
      work_guard_(boost::asio::make_work_guard(io_context_)),
      last_task_id_(0) {}

ExpiryTaskManager::~ExpiryTaskManager() noexcept {
  if (running_) {
    stop();
  }
}

void ExpiryTaskManager::start() {
  if (running_) {
    throw IllegalStateException(
        "Tried to start ExpiryTaskManager when it was already running");
  }

  std::promise<bool> start_promise;
  auto start_future = start_promise.get_future();
  runner_ = std::thread{[this, &start_promise] {
    start_promise.set_value(true);
    DistributedSystemImpl::setThreadName(NC_ETM_Thread);

    LOG_FINE("ExpiryTaskManager thread is running.");
    io_context_.run();
    LOG_FINE("ExpiryTaskManager thread has stopped.");
  }};

  running_ = start_future.get();
}

void ExpiryTaskManager::stop() {
  {
    std::unique_lock<std::mutex> lock(mutex_);

    if (!running_) {
      throw IllegalStateException(
          "Tried to stop ExpiryTaskManager when it was not running");
    }

    LOG_DEBUG("Stopping ExpiryTaskManager...");

    work_guard_.reset();
    running_ = false;

    cancel_all();
  }

  runner_.join();
}

ExpiryTask::id_t ExpiryTaskManager::schedule(
    std::shared_ptr<ExpiryTask> task, const ExpiryTask::duration_t &delay,
    const ExpiryTask::duration_t &interval) {
  std::unique_lock<std::mutex> lock(mutex_);

  if (!running_) {
    LOG_DEBUG("Tried to add a task while ExpiryTaskManager is not running");
    return ExpiryTask::invalid();
  }

  auto task_id = last_task_id_++;
  if (task_id == ExpiryTask::invalid()) {
    last_task_id_ = 0;
    task_id = 0;
  }

  task->id(task_id).interval(interval);
  task_map_.emplace(task_id, std::move(task)).first->second->reset(delay);

  using apache::geode::internal::chrono::duration::to_string;
  LOG_DEBUG("Task %zu has been scheduled in %s with an interval of %s", task_id,
            to_string(delay).c_str(), to_string(interval).c_str());
  return task_id;
}

int32_t ExpiryTaskManager::reset(ExpiryTask::id_t task_id,
                                 const ExpiryTask::duration_t &delay) {
  std::unique_lock<std::mutex> lock(mutex_);
  auto &&iter = task_map_.find(task_id);
  if (iter == task_map_.end()) {
    return -1;
  }

  auto n = iter->second->reset(delay);
  return static_cast<int32_t>(n);
}

int32_t ExpiryTaskManager::cancel(ExpiryTask::id_t task_id) {
  std::unique_lock<std::mutex> lock(mutex_);
  auto &&iter = task_map_.find(task_id);
  if (iter == task_map_.end()) {
    return -1;
  }

  auto n = iter->second->cancel();
  task_map_.erase(iter);
  return static_cast<int32_t>(n);
}

void ExpiryTaskManager::cancel_all() {
  for (auto &&iter : task_map_) {
    iter.second->cancel();
  }

  task_map_.clear();
}

void ExpiryTaskManager::remove(ExpiryTask::id_t task_id) {
  std::unique_lock<std::mutex> lock(mutex_);
  task_map_.erase(task_id);
}
}  // namespace client
}  // namespace geode
}  // namespace apache
