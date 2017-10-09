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

#include <geode/Cache.hpp>
#include <geode/CacheableKey.hpp>
#include <CacheableToken.hpp>

namespace apache {
namespace geode {
namespace client {

RegionEntry::RegionEntry(const RegionPtr& region, const CacheableKeyPtr& key,
                         const CacheablePtr& value)
    : m_region(region), m_key(key), m_value(value), m_destroyed(false) {}

RegionEntry::~RegionEntry() {}

CacheableKeyPtr RegionEntry::getKey() { return m_key; }

CacheablePtr RegionEntry::getValue() {
  return CacheableToken::isInvalid(m_value) ? nullptr : m_value;
}

RegionPtr RegionEntry::getRegion() { return m_region; }

CacheStatisticsPtr RegionEntry::getStatistics() {
  return m_statistics;
}

bool RegionEntry::isDestroyed() const { return m_destroyed; }

}  // namespace client
}  // namespace geode
}  // namespace apache
