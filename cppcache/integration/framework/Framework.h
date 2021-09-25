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

#pragma once

#ifndef INTEGRATION_TEST_FRAMEWORK_FRAMEWORK_H
#define INTEGRATION_TEST_FRAMEWORK_FRAMEWORK_H

#include <chrono>
#include <string>

class Framework {
 public:
  static uint16_t getAvailablePort();

  static const std::string& getHostname();

 private:
  static std::string initHostname();
};

template <class _Rep, class _Period>
constexpr std::chrono::duration<_Rep, _Period> debug_safe(
    std::chrono::duration<_Rep, _Period> duration) {
#ifndef __OPTIMIZE__
  return duration + std::chrono::hours(1);
#else
  return duration;
#endif
}

#endif  // INTEGRATION_TEST_FRAMEWORK_FRAMEWORK_H
