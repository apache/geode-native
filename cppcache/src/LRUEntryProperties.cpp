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

#include "LRUEntryProperties.hpp"

// Bit mask for recently used
#define RECENTLY_USED_BITS 1u
// Bit mask for evicted
#define EVICTED_BITS 2u

namespace apache {
namespace geode {
namespace client {
LRUEntryProperties::LRUEntryProperties()
    : m_bits(0), m_persistenceInfo(nullptr) {}

void LRUEntryProperties::setRecentlyUsed() { m_bits |= RECENTLY_USED_BITS; }

void LRUEntryProperties::clearRecentlyUsed() { m_bits &= ~RECENTLY_USED_BITS; }

bool LRUEntryProperties::testRecentlyUsed() const {
  return (m_bits.load() & RECENTLY_USED_BITS) == RECENTLY_USED_BITS;
}

bool LRUEntryProperties::testEvicted() const {
  return (m_bits.load() & EVICTED_BITS) == EVICTED_BITS;
}

void LRUEntryProperties::setEvicted() { m_bits |= EVICTED_BITS; }

void LRUEntryProperties::clearEvicted() { m_bits &= ~EVICTED_BITS; }

const std::shared_ptr<void>& LRUEntryProperties::getPersistenceInfo() const {
  return m_persistenceInfo;
}

void LRUEntryProperties::setPersistenceInfo(
    const std::shared_ptr<void>& persistenceInfo) {
  m_persistenceInfo = persistenceInfo;
}

LRUEntryProperties::LRUEntryProperties(bool) {}
}  // namespace client
}  // namespace geode
}  // namespace apache
