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

#ifndef GEODE_INTERNAL_CHRONO_DURATION_H_
#define GEODE_INTERNAL_CHRONO_DURATION_H_

#include <chrono>
#include <iosfwd>
#include <ratio>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace apache {
namespace geode {
namespace internal {
namespace chrono {
namespace duration {

template <class Period>
struct _suffix {
  static constexpr char const* value = "<<unknown units>>";
};
template <>
struct _suffix<std::ratio<3600>> {
  static constexpr char const* value = "h";
};
template <>
struct _suffix<std::ratio<60>> {
  static constexpr char const* value = "min";
};
template <>
struct _suffix<std::ratio<1>> {
  static constexpr char const* value = "s";
};
template <>
struct _suffix<std::milli> {
  static constexpr char const* value = "ms";
};
template <>
struct _suffix<std::micro> {
  static constexpr char const* value = "us";
};
template <>
struct _suffix<std::nano> {
  static constexpr char const* value = "ns";
};

template <class T>
struct _is_duration : std::false_type {};

template <class Rep, class Period>
struct _is_duration<std::chrono::duration<Rep, Period>> : std::true_type {};

template <class Rep, class Period>
struct _is_duration<const std::chrono::duration<Rep, Period>> : std::true_type {
};

template <class Rep, class Period>
struct _is_duration<volatile std::chrono::duration<Rep, Period>>
    : std::true_type {};

template <class Rep, class Period>
struct _is_duration<const volatile std::chrono::duration<Rep, Period>>
    : std::true_type {};

/**
 * Converts std::chrono:duration from given unit to other where other duration
 * is no less than the given duration.
 *
 * For internal use only.
 */
template <class ToDuration, class Rep, class Period>
inline
    typename std::enable_if<_is_duration<ToDuration>::value, ToDuration>::type
    _ceil(const std::chrono::duration<Rep, Period>& duration) {
  ToDuration other = std::chrono::duration_cast<ToDuration>(duration);
  if (other < duration) {
    return other + ToDuration{1};
  }
  return other;
}

/**
 * Parses std::string into std::chrono::duration expecting same format as that
 * of C++14 std::chrono::duration literals.
 *
 * @returns std::chrono::duration for given input string where the resulting
 * std::chrono::duration will not be less than the parsed duration if the
 * units of std::chrono::duration are less the units of the given string.
 *
 * @tparam ToDuration std::chrono::duration type to parse into.
 */
template <class ToDuration = std::chrono::nanoseconds>
inline
    typename std::enable_if<_is_duration<ToDuration>::value, ToDuration>::type
    from_string(const std::string& string) {
  const auto& begin = string.begin();
  auto end = string.end();
  if (begin != end--) {
    if ('s' == *end) {
      if (begin != end--) {
        if ('m' == *end) {
          return _ceil<ToDuration>(std::chrono::milliseconds(
              std::stoll(string.substr(0, std::distance(begin, end)))));
        } else if ('u' == *end) {
          return _ceil<ToDuration>(std::chrono::microseconds(
              std::stoll(string.substr(0, std::distance(begin, end)))));
        } else if ('n' == *end) {
          return _ceil<ToDuration>(std::chrono::nanoseconds(
              std::stoll(string.substr(0, std::distance(begin, end)))));
        }
      }
      return _ceil<ToDuration>(std::chrono::seconds(
          std::stoll(string.substr(0, std::distance(begin, ++end)))));
    } else if ('h' == *end) {
      return std::chrono::hours(
          std::stoll(string.substr(0, std::distance(begin, end))));
    } else if ('n' == *end && begin != end-- && 'i' == *end && begin != end-- &&
               'm' == *end) {
      return _ceil<ToDuration>(std::chrono::minutes(
          std::stoll(string.substr(0, std::distance(begin, end)))));
    }
  }

  throw std::invalid_argument("Not a valid duration parsing '" + string + "'.");
}

/**
 * Converts std::chrono::duration to std::string.
 *
 * @tparam Rep
 * @tparam Period
 * @param duration to convert to std::string
 * @return std::string representing the given duration.
 */
template <class Rep, class Period>
inline std::string to_string(
    const std::chrono::duration<Rep, Period>& duration) {
  return std::to_string(duration.count()) + _suffix<Period>::value;
}

}  // namespace duration
}  // namespace chrono
}  // namespace internal
}  // namespace geode
}  // namespace apache

#endif /* GEODE_INTERNAL_CHRONO_DURATION_H_ */
