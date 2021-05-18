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

#ifndef GEODE_TRACKEDMAPENTRY_H_
#define GEODE_TRACKEDMAPENTRY_H_

#include "MapEntry.hpp"

namespace apache {
namespace geode {
namespace client {

class TrackedMapEntry final : public MapEntry {
 public:
  // Constructor should be invoked only when starting the tracking
  // of a MapEntry, so m_trackingNumber is initialized with 1.
  inline TrackedMapEntry(const std::shared_ptr<MapEntryImpl>& entry,
                         int trackingNumber, int updateCount)
      : m_entry(const_cast<std::shared_ptr<MapEntryImpl>&>(entry)),
        m_trackingNumber(trackingNumber),
        m_updateCount(updateCount) {}

  ~TrackedMapEntry() noexcept override = default;

  std::shared_ptr<MapEntryImpl> getImplPtr() final { return m_entry; }

  int addTracker(std::shared_ptr<MapEntry>&) final {
    ++m_trackingNumber;
    return m_updateCount;
  }

  std::pair<bool, int> removeTracker() final {
    if (m_trackingNumber > 0) {
      --m_trackingNumber;
    }
    if (m_trackingNumber == 0) {
      m_updateCount = 0;
      return std::make_pair(true, 0);
    }
    return std::make_pair(false, m_trackingNumber);
  }

  int incrementUpdateCount(std::shared_ptr<MapEntry>&) final {
    return ++m_updateCount;
  }

  int getTrackingNumber() const final { return m_trackingNumber; }

  int getUpdateCount() const final { return m_updateCount; }

  void getKey(std::shared_ptr<CacheableKey>& result) const final;

  void getValue(std::shared_ptr<Cacheable>& result) const final;

  void setValue(const std::shared_ptr<Cacheable>& value) final;

  LRUEntryProperties& getLRUProperties() final;

  ExpEntryProperties& getExpProperties() final;

  VersionStamp& getVersionStamp() final;

  void cleanup(const CacheEventFlags eventFlags) final;

 private:
  std::shared_ptr<MapEntryImpl> m_entry;
  int m_trackingNumber;
  int m_updateCount;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TRACKEDMAPENTRY_H_
