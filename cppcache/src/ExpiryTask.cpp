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

#include "ExpiryTask.hpp"

#include "ExpiryTaskManager.hpp"

namespace apache {
namespace geode {
namespace client {

ExpiryTask::ExpiryTask(ExpiryTaskManager& manager)
    : id_{invalid()}, timer_{manager.io_context()}, manager_{manager} {}

int32_t ExpiryTask::reset(const std::chrono::nanoseconds& ns) {
  return reset(timer_t::clock_type::now() + ns);
}

int32_t ExpiryTask::reset(const time_point_t& at) {
  std::unique_lock<decltype(mutex_)> lock{mutex_};
  if (cancelled_) {
    return -1;
  }

  auto self = shared_from_this();
  auto n = timer_.expires_at(at);
  timer_.async_wait(
      [self](const boost::system::error_code& e) { self->on_callback(e); });
  return static_cast<int32_t>(n);
}

void ExpiryTask::on_callback(const boost::system::error_code& err) {
  if (cancelled_ || err) {
    return;
  }

  if (on_expire()) {
    if (periodic()) {
      reset(timer_.expiry() + interval_);
    } else {
      manager_.remove(id_);
    }
  }
}

int32_t ExpiryTask::cancel() {
  std::unique_lock<decltype(mutex_)> lock{mutex_};

  cancelled_ = true;
  return static_cast<int32_t>(timer_.cancel());
}

}  // namespace client
}  // namespace geode
}  // namespace apache
