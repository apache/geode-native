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

#ifndef GEODE_MAPENTRY_H_
#define GEODE_MAPENTRY_H_

#include <atomic>
#include <memory>
#include <utility>

#include <geode/CacheableKey.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/Serializable.hpp>
#include <geode/internal/geode_globals.hpp>

#include "CacheImpl.hpp"
#include "CacheableToken.hpp"
#include "ExpiryTaskManager.hpp"
#include "RegionInternal.hpp"
#include "VersionStamp.hpp"

namespace apache {
namespace geode {
namespace client {

class MapEntry;
class MapEntryImpl;
class LRUEntryProperties;
class CacheImpl;

/**
 * @brief This class encapsulates expiration specific properties for
 *   a MapEntry.
 */
class APACHE_GEODE_EXPORT ExpEntryProperties {
 public:
  typedef std::chrono::system_clock::time_point time_point;

  inline explicit ExpEntryProperties(ExpiryTaskManager* expiryTaskManager)
      : m_lastAccessTime(0),
        m_lastModifiedTime(0),
        m_expiryTaskId(-1),
        m_expiryTaskManager(expiryTaskManager) {
    // The reactor always gives +ve id while scheduling.
    // -1 will indicate that an expiry task has not been scheduled
    // for this entry. // TODO confirm
  }

  inline time_point getLastAccessTime() const {
    return time_point(std::chrono::system_clock::duration(m_lastAccessTime));
  }

  inline time_point getLastModifiedTime() const {
    return time_point(std::chrono::system_clock::duration(m_lastModifiedTime));
  }

  //  moved time initialization outside of constructor to avoid
  // the costly gettimeofday call in MapSegment spinlock
  inline void initStartTime() {
    time_point currentTime = std::chrono::system_clock::now();
    m_lastAccessTime = currentTime.time_since_epoch().count();
    m_lastModifiedTime = currentTime.time_since_epoch().count();
  }

  inline void updateLastAccessTime(time_point currTime) {
    m_lastAccessTime = currTime.time_since_epoch().count();
  }

  inline void updateLastModifiedTime(time_point currTime) {
    m_lastModifiedTime = currTime.time_since_epoch().count();
  }

  inline void setExpiryTaskId(ExpiryTaskManager::id_type id) {
    m_expiryTaskId = id;
  }

  inline ExpiryTaskManager::id_type getExpiryTaskId() const {
    return m_expiryTaskId;
  }

  inline void cancelExpiryTaskId(
      const std::shared_ptr<CacheableKey>& key) const {
    auto taskIdStr = std::to_string(m_expiryTaskId);
    LOGDEBUG("Cancelling expiration task for key [%s] with id [%s]",
             Utils::nullSafeToString(key).c_str(), taskIdStr.c_str());
    m_expiryTaskManager->cancelTask(m_expiryTaskId);
  }

 protected:
  // this constructor deliberately skips initializing any fields
  inline explicit ExpEntryProperties(bool)
      : m_lastAccessTime(0), m_lastModifiedTime(0) {}

 private:
  /** last access time in secs, 32bit.. */
  std::atomic<time_point::duration::rep> m_lastAccessTime;
  /** last modified time in secs, 32bit.. */
  std::atomic<time_point::duration::rep> m_lastModifiedTime;
  /** The expiry task id for this particular entry.. **/
  ExpiryTaskManager::id_type m_expiryTaskId;
  ExpiryTaskManager* m_expiryTaskManager;
};

/**
 * @brief Interface class for region mapped entry value.
 */
class APACHE_GEODE_EXPORT MapEntry {
 public:
  virtual ~MapEntry() {}

  virtual void getKey(std::shared_ptr<CacheableKey>& result) const = 0;
  virtual void getValue(std::shared_ptr<Cacheable>& result) const = 0;
  virtual void setValue(const std::shared_ptr<Cacheable>& value) = 0;
  virtual std::shared_ptr<MapEntryImpl> getImplPtr() = 0;

  virtual LRUEntryProperties& getLRUProperties() = 0;
  virtual ExpEntryProperties& getExpProperties() = 0;
  virtual VersionStamp& getVersionStamp() = 0;

  /**
   * Adds a tracker to this MapEntry for any updates.
   * Returns the current update sequence number of this entry.
   * Returns by reference any updated MapEntry to be rebound while an
   * unchanged return value implies this MapEntry should be continued with.
   * Note that the contract says that the return value should not be
   * touched if this MapEntry is to be continued with.
   *
   * The design of tracking is thus:
   * Each entry will have two fields, one indicating the current number of
   * operations tracking this entry, and second indicating the sequence
   * number for number of updates since tracking was started. The idea is
   * to start tracking an entry before going for a long remote operation
   * and check whether the update sequence has remained unchanged upon
   * return. If the entry has been updated in the interim period then
   * the current update is not applied. When the number of operations
   * tracking an entry goes down to zero, then the update sequence number
   * is also reset to zero.
   */
  virtual int addTracker(std::shared_ptr<MapEntry>& newEntry) = 0;

  /**
   * Removes a tracker for this MapEntry and returns a pair:
   *  1) the first element of the pair is a boolean that indicates whether
   *     or not the entry should be replaced with the underlying MapEntryImpl
   *     object in the map
   *  2) the second element is the updated number of trackers for this entry
   */
  virtual std::pair<bool, int> removeTracker() = 0;

  /**
   * Increment the number of updates to this entry.
   * Returns the current update sequence number of this entry.
   * Returns by reference any updated MapEntry to be rebound while an
   * unchanged return value implies this MapEntry should be continued with.
   * Note that the contract says that the return value should not be
   * touched if this MapEntry is to be continued with.
   */
  virtual int incrementUpdateCount(std::shared_ptr<MapEntry>& newEntry) = 0;

  /**
   * Get the current tracking number of this entry. A return value of zero
   * indicates that the entry is not being tracked.
   */
  virtual int getTrackingNumber() const = 0;

  /**
   * Get the current number of updates since tracking was started for
   * this entry.
   */
  virtual int getUpdateCount() const = 0;

  /**
   * Any cleanup required (e.g. removing from LRUList) for the entry.
   */
  virtual void cleanup(const CacheEventFlags eventFlags) = 0;

 protected:
  inline MapEntry() {}

  inline explicit MapEntry(bool) {}
};

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

  LRUEntryProperties& getLRUProperties() override {
    throw FatalInternalException(
        "MapEntry::getLRUProperties called for "
        "non-LRU MapEntry");
  }

  ExpEntryProperties& getExpProperties() override {
    throw FatalInternalException(
        "MapEntry::getExpProperties called for "
        "non-expiration MapEntry");
  }

  VersionStamp& getVersionStamp() override {
    throw FatalInternalException(
        "MapEntry::getVersionStamp called for "
        "non-versioned MapEntry");
  }

  void cleanup(const CacheEventFlags) override{};

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
  virtual ~VersionedMapEntryImpl() {}

  virtual VersionStamp& getVersionStamp() { return *this; }

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

#endif  // GEODE_MAPENTRY_H_
