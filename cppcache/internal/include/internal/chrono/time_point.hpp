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

#ifndef GEODE_UTIL_CHRONO_TIME_POINT_H_
#define GEODE_UTIL_CHRONO_TIME_POINT_H_

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace apache {
namespace geode {
namespace util {
namespace chrono {

/**
 * Wrapper around platform specific thread safe localtime functions.
 * @param time to get local time for
 * @return local time
 */
std::tm localtime(const time_t& time);

/**
 * Wrapper around platform specific thread safe localtime functions.
 * @param time_point to get local time for
 * @return local time
 */
std::tm localtime(const std::chrono::system_clock::time_point& time_point);

/**
 * Produces string representation for given time.
 * @param time to get string for
 * @return string representation of given time
 */
std::string to_string(const time_t& time);

/**
 * Produces string representation for given time_point.
 * @param time_point to get string for
 * @return string representation of given time_point
 */
std::string to_string(const std::chrono::system_clock::time_point& time_point);

}  // namespace chrono
}  // namespace util
}  // namespace geode
}  // namespace apache

#endif /* GEODE_UTIL_CHRONO_TIME_POINT_H_ */
