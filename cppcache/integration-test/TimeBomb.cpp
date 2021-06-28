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

#include "TimeBomb.hpp"

#include <iostream>

TimeBomb::TimeBomb(const std::chrono::milliseconds& sleep,
                   std::function<void()> cleanup)
    : enabled_{false}, callback_{cleanup}, sleep_{sleep} {}

TimeBomb::~TimeBomb() noexcept {
  if (enabled_) {
    disarm();
  }
}

void TimeBomb::arm() {
  enabled_ = true;
  thread_ = std::thread{[this] { run(); }};
}

void TimeBomb::disarm() {
  enabled_ = false;
  cv_.notify_all();
  thread_.join();
}

void TimeBomb::run() {
  std::clog << "TimeBomb armed to trigger in " << sleep_.count() << " ms"
            << std::endl;
  {
    std::unique_lock<decltype(mutex_)> lock{mutex_};
    cv_.wait_for(lock, sleep_);
  }

  if (enabled_) {
    std::clog << "####### ERROR: TIMEBOMB WENT OFF, TEST TIMED OUT ########"
              << std::endl
              << std::flush;
    callback_();
    exit(-1);
  } else {
    std::clog << "###### TIMEBOMB Disabled ######" << std::endl << std::flush;
  }
}
