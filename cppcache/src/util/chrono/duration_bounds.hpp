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

#ifndef GEODE_UTIL_CHRONO_DURATION_BOUNDS_H_
#define GEODE_UTIL_CHRONO_DURATION_BOUNDS_H_

#include <chrono>
#include <algorithm>

#include <geode/ExceptionTypes.hpp>

namespace apache {
namespace geode {
namespace util {
namespace chrono {
namespace duration {

using namespace apache::geode::client;

template <class LimitRep, class LimitPeriod,
          LimitRep lower = std::numeric_limits<LimitRep>::lowest(),
          LimitRep upper = std::numeric_limits<LimitRep>::max()>
struct assert_bounds {
  template <class Rep, class Period>
  inline void operator()(const std::chrono::duration<Rep, Period> value) const {
    constexpr auto min = std::chrono::duration<LimitRep, LimitPeriod>(lower);
    constexpr auto max = std::chrono::duration<LimitRep, LimitPeriod>(upper);
    if (value > max) {
      throw IllegalArgumentException("Duration exceeds maximum of " +
                                     to_string(max));
    } else if (value < min) {
      throw IllegalArgumentException("Duration exceeds minimum of " +
                                     to_string(min));
    }
  }
};

}  // namespace duration
}  // namespace chrono
}  // namespace util
}  // namespace geode
}  // namespace apache

#endif  // GEODE_UTIL_CHRONO_DURATION_BOUNDS_H_