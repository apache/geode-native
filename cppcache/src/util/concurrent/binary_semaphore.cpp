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

#include "binary_semaphore.hpp"

namespace apache {
namespace geode {
namespace client {

binary_semaphore::binary_semaphore(bool released) : released_(released) {}

void binary_semaphore::release() {
  std::lock_guard<std::mutex> lock(mutex_);
  released_ = true;
  cv_.notify_one();
}

void binary_semaphore::acquire() {
  std::unique_lock<std::mutex> lock(mutex_);
  cv_.wait(lock, [this]() { return released_; });
  released_ = false;
}

bool binary_semaphore::try_acquire_for(
    const std::chrono::milliseconds& period) {
  std::unique_lock<std::mutex> lock(mutex_);
  if (!cv_.wait_for(lock, period, [this]() { return released_; })) {
    return false;
  }

  released_ = false;
  return true;
}
}  // namespace client
}  // namespace geode
}  // namespace apache
