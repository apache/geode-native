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

#ifndef GEODE_MAPENTRYIMPL_H_
#define GEODE_MAPENTRYIMPL_H_

#include <atomic>
#include <memory>
#include <utility>

#include <geode/CacheableKey.hpp>
#include <geode/Serializable.hpp>
#include <geode/internal/geode_globals.hpp>

#include "CacheableToken.hpp"
#include "MapEntry.hpp"
#include "RegionInternal.hpp"
#include "VersionStamp.hpp"

namespace apache {
namespace geode {
namespace client {

class CacheImpl;
class ExpEntryProperties;
class ExpiryTaskManager;
class LRUEntryProperties;

/**
 * @brief Hold region mapped entry value. subclass will hold lru flags.
 * Another holds expiration timestamps.
 */
class MapEntryImpl : public MapEntry,
                     public std::enable_shared_from_this<MapEntryImpl> {
 public:
  ~MapEntryImpl() override = default;
  MapEntryImpl(const MapEntryImpl&) = delete;
  MapEntryImpl& operator=(const MapEntryImpl&) = delete;

  inline void getKeyI(std::shared_ptr<CacheableKey>& result) const {
    result = m_key;
  }

  inline void getValueI(std::shared_ptr<Cacheable>& result) const {
    // If value is destroyed, then this returns nullptr
    if (CacheableToken::isDestroyed(m_value)) {
      result = nullptr;
    } else {
      result = m_value;
    }
  }

  inline void setValueI(const std::shared_ptr<Cacheable>& value) {
    m_value = value;
  }

  void getKey(std::shared_ptr<CacheableKey>& result) const override {
    getKeyI(result);
  }

  void getValue(std::shared_ptr<Cacheable>& result) const override {
    getValueI(result);
  }

  void setValue(const std::shared_ptr<Cacheable>& value) override {
    setValueI(value);
  }

  std::shared_ptr<MapEntryImpl> getImplPtr() override {
    return shared_from_this();
  }

  LRUEntryProperties& getLRUProperties() override;

  ExpEntryProperties& getExpProperties() override;

  VersionStamp& getVersionStamp() override;

  void cleanup(const CacheEventFlags) override {}

 protected:
  inline explicit MapEntryImpl(bool) : MapEntry(true) {}

  inline explicit MapEntryImpl(const std::shared_ptr<CacheableKey>& key)
      : MapEntry(), m_key(key) {}

  std::shared_ptr<Cacheable> m_value;
  std::shared_ptr<CacheableKey> m_key;
};

class APACHE_GEODE_EXPORT VersionedMapEntryImpl : public MapEntryImpl,
                                                  public VersionStamp {
 public:
  ~VersionedMapEntryImpl() override = default;

  VersionStamp& getVersionStamp() override { return *this; }

 protected:
  inline explicit VersionedMapEntryImpl(bool) : MapEntryImpl(true) {}

  inline explicit VersionedMapEntryImpl(
      const std::shared_ptr<CacheableKey>& key)
      : MapEntryImpl(key) {}

 private:
  // disabled
  VersionedMapEntryImpl(const VersionedMapEntryImpl&);
  VersionedMapEntryImpl& operator=(const VersionedMapEntryImpl&);
};

class APACHE_GEODE_EXPORT EntryFactory {
 public:
  explicit EntryFactory(const bool concurrencyChecksEnabled)
      : m_concurrencyChecksEnabled(concurrencyChecksEnabled) {}

  virtual ~EntryFactory() {}

  virtual void newMapEntry(ExpiryTaskManager* expiryTaskManager,
                           const std::shared_ptr<CacheableKey>& key,
                           std::shared_ptr<MapEntryImpl>& result) const;

 protected:
  bool m_concurrencyChecksEnabled;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_MAPENTRYIMPL_H_
