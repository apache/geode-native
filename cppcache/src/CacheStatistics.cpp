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

namespace apache {
namespace geode {
namespace client {

CacheStatistics::CacheStatistics()
    : m_lastAccessTime(0), m_lastModifiedTime(0) {}

void CacheStatistics::setLastModifiedTime(time_point lmt) {
  m_lastModifiedTime = lmt.time_since_epoch().count();
}

void CacheStatistics::setLastAccessedTime(time_point lat) {
  m_lastAccessTime = lat.time_since_epoch().count();
}

CacheStatistics::time_point CacheStatistics::getLastModifiedTime() const {
  return time_point(std::chrono::system_clock::duration(m_lastModifiedTime));
}

CacheStatistics::time_point CacheStatistics::getLastAccessedTime() const {
  return time_point(std::chrono::system_clock::duration(m_lastAccessTime));
}

}  // namespace client
}  // namespace geode
}  // namespace apache
