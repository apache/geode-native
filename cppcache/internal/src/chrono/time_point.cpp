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

#include "internal/chrono/time_point.hpp"

namespace apache {
namespace geode {
namespace util {
namespace chrono {
std::tm localtime(const time_t& time) {
  std::tm localtime;
#if defined(_WIN32)
  localtime_s(&localtime, &time);
#else
  localtime_r(&time, &localtime);
#endif
  return localtime;
}

std::tm localtime(const std::chrono::system_clock::time_point& time_point) {
  return localtime(std::chrono::system_clock::to_time_t(time_point));
}

std::string to_string(const time_t& time) {
  std::stringstream stringstream;
  const auto local = localtime(time);

#if defined(_WIN32)
  /*
  Windows does not allow for time_t to be negative (in the past). Any negative
  time_t will result in a tm with negative values. Converting this to a string
  will result in an assertion failure. The original string converstion would
  then result in something like -1/-1/1998. Rather than return a formatted but
  invalid time we should return a string indicating and invalid time failed to
  convert.

  TODO: replace with C++20 std::chrono::format or other date library
  */
  if (-1 == local.tm_hour) {
    return "invalid time";
  }
#endif

  stringstream << std::put_time(&local, "%c %Z");
  return stringstream.str();
}

std::string to_string(const std::chrono::system_clock::time_point& time_point) {
  return to_string(std::chrono::system_clock::to_time_t(time_point));
}

}  // namespace chrono
}  // namespace util
}  // namespace geode
}  // namespace apache
