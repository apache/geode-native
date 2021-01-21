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

#include <geode/CacheStatistics.hpp>

namespace {
std::chrono::system_clock::time_point convert_steady_to_system_tp(
    std::chrono::steady_clock::time_point tp) {
  return std::chrono::system_clock::now() +
         std::chrono::duration_cast<std::chrono::system_clock::duration>(
             std::chrono::steady_clock::now() - tp);
}
}  // namespace

namespace apache {
namespace geode {
namespace client {

CacheStatistics::CacheStatistics() : last_accessed_{0}, last_modified_{0} {}

CacheStatistics::~CacheStatistics() = default;

void CacheStatistics::setLastModifiedTime(time_point tp) {
  last_modified_ = tp.time_since_epoch().count();
}

void CacheStatistics::setLastAccessedTime(time_point tp) {
  last_accessed_ = tp.time_since_epoch().count();
}

std::chrono::system_clock::time_point CacheStatistics::getLastModifiedTime()
    const {
  return convert_steady_to_system_tp(getLastModifiedSteadyTime());
}

std::chrono::system_clock::time_point CacheStatistics::getLastAccessedTime()
    const {
  return convert_steady_to_system_tp(getLastAccessedSteadyTime());
}

CacheStatistics::time_point CacheStatistics::getLastModifiedSteadyTime() const {
  return time_point{time_point::duration{last_modified_}};
}

CacheStatistics::time_point CacheStatistics::getLastAccessedSteadyTime() const {
  return time_point{time_point::duration{last_accessed_}};
}
}  // namespace client
}  // namespace geode
}  // namespace apache
