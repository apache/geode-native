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

#ifndef GEODE_EXPMAPENTRY_H_
#define GEODE_EXPMAPENTRY_H_

#include <geode/internal/geode_globals.hpp>

#include "ExpEntryProperties.hpp"
#include "MapEntryImpl.hpp"
#include "VersionStamp.hpp"

namespace apache {
namespace geode {
namespace client {
/**
 * @brief Hold region mapped entry value.
 * This subclass adds expiration times.
 */
class ExpMapEntry : public MapEntryImpl, public ExpEntryProperties {
 public:
  ~ExpMapEntry() noexcept override = default;

  ExpMapEntry(const ExpMapEntry&) = delete;
  ExpMapEntry& operator=(const ExpMapEntry&) = delete;

  ExpEntryProperties& getExpProperties() override { return *this; }

  virtual void cleanup(const CacheEventFlags eventFlags) override {
    if (!eventFlags.isExpiration()) {
      cancel_task();
    }
  }

  // this constructor deliberately skips touching or initializing any members
  inline explicit ExpMapEntry(bool)
      : MapEntryImpl(true), ExpEntryProperties(true) {}

  inline ExpMapEntry(ExpiryTaskManager* expiryTaskManager,
                     const std::shared_ptr<CacheableKey>& key)
      : MapEntryImpl(key), ExpEntryProperties(expiryTaskManager) {}
};

class VersionedExpMapEntry : public ExpMapEntry, public VersionStamp {
 public:
  inline VersionedExpMapEntry(ExpiryTaskManager* expiryTaskManager,
                              const std::shared_ptr<CacheableKey>& key)
      : ExpMapEntry(expiryTaskManager, key) {}

  inline explicit VersionedExpMapEntry(bool) : ExpMapEntry(true) {}

  ~VersionedExpMapEntry() noexcept override {}

  VersionedExpMapEntry(const VersionedExpMapEntry&) = delete;
  VersionedExpMapEntry& operator=(const VersionedExpMapEntry&) = delete;

  VersionStamp& getVersionStamp() override { return *this; }
};

class ExpEntryFactory : public EntryFactory {
 public:
  using EntryFactory::EntryFactory;

  ~ExpEntryFactory() noexcept override {}

  void newMapEntry(ExpiryTaskManager* expiryTaskManager,
                   const std::shared_ptr<CacheableKey>& key,
                   std::shared_ptr<MapEntryImpl>& result) const override;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_EXPMAPENTRY_H_
