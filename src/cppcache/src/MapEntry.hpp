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
#include <geode/geode_globals.hpp>
#include <geode/Cacheable.hpp>
#include <geode/CacheableKey.hpp>
#include <memory>
#include <geode/ExceptionTypes.hpp>

#include "CacheImpl.hpp"
#include "ExpiryTaskManager.hpp"
#include "RegionInternal.hpp"
#include "CacheableToken.hpp"
#include "VersionStamp.hpp"
#include <ace/OS.h>
#include <utility>

namespace apache {
namespace geode {
namespace client {
class CPPCACHE_EXPORT MapEntry;
typedef std::shared_ptr<MapEntry> MapEntryPtr;
class CPPCACHE_EXPORT MapEntryImpl;
typedef std::shared_ptr<MapEntryImpl> MapEntryImplPtr;

class CPPCACHE_EXPORT LRUEntryProperties;
class CacheImpl;

/**
 * @brief This class encapsulates expiration specific properties for
 *   a MapEntry.
 */
class CPPCACHE_EXPORT ExpEntryProperties {
 public:
  inline ExpEntryProperties(ExpiryTaskManager* expiryTaskManager)
      : m_lastAccessTime(0),
        m_lastModifiedTime(0),
        m_expiryTaskId(-1),
        m_expiryTaskManager(expiryTaskManager) {
    // The reactor always gives +ve id while scheduling.
    // -1 will indicate that an expiry task has not been scheduled
    // for this entry. // TODO confirm
  }

  inline uint32_t getLastAccessTime() const { return m_lastAccessTime; }

  inline uint32_t getLastModifiedTime() const { return m_lastModifiedTime; }

  //  moved time initialization outside of constructor to avoid
  // the costly gettimeofday call in MapSegment spinlock
  inline void initStartTime() {
    uint32_t currTime = static_cast<uint32_t>(ACE_OS::gettimeofday().sec());
    m_lastModifiedTime = currTime;
    m_lastAccessTime = currTime;
  }

  inline void updateLastAccessTime(uint32_t currTime) {
    m_lastAccessTime = currTime;
  }

  inline void updateLastModifiedTime(uint32_t currTime) {
    m_lastModifiedTime = currTime;
  }

  inline void setExpiryTaskId(long id) { m_expiryTaskId = id; }

  inline long getExpiryTaskId() const { return m_expiryTaskId; }

  inline void cancelExpiryTaskId(const CacheableKeyPtr& key) const {
    LOGDEBUG("Cancelling expiration task for key [%s] with id [%d]",
             Utils::getCacheableKeyString(key)->asChar(), m_expiryTaskId);
    m_expiryTaskManager->cancelTask(m_expiryTaskId);
  }

 protected:
  // this constructor deliberately skips initializing any fields
  inline explicit ExpEntryProperties(bool noInit) {}

 private:
  /** last access time in secs, 32bit.. */
  std::atomic<uint32_t> m_lastAccessTime;
  /** last modified time in secs, 32bit.. */
  std::atomic<uint32_t> m_lastModifiedTime;
  /** The expiry task id for this particular entry.. **/
  long m_expiryTaskId;
  ExpiryTaskManager* m_expiryTaskManager;
};

/**
 * @brief Interface class for region mapped entry value.
 */
class CPPCACHE_EXPORT MapEntry {
 public:
  static MapEntryPtr MapEntry_NullPointer;

  virtual ~MapEntry() {}

  virtual void getKey(CacheableKeyPtr& result) const = 0;
  virtual void getValue(CacheablePtr& result) const = 0;
  virtual void setValue(const CacheablePtr& value) = 0;
  virtual MapEntryImplPtr getImplPtr() = 0;

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
  virtual int addTracker(MapEntryPtr& newEntry) = 0;

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
  virtual int incrementUpdateCount(MapEntryPtr& newEntry) = 0;

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

  inline explicit MapEntry(bool noInit) {}
};

/**
 * @brief Hold region mapped entry value. subclass will hold lru flags.
 * Another holds expiration timestamps.
 */
class MapEntryImpl : public MapEntry,
                     public std::enable_shared_from_this<MapEntryImpl> {
 public:
  virtual ~MapEntryImpl() {}

  inline void getKeyI(CacheableKeyPtr& result) const { result = m_key; }

  inline void getValueI(CacheablePtr& result) const {
    // If value is destroyed, then this returns nullptr
    if (CacheableToken::isDestroyed(m_value)) {
      result = nullptr;
    } else {
      result = m_value;
    }
  }

  inline void setValueI(const CacheablePtr& value) { m_value = value; }

  virtual void getKey(CacheableKeyPtr& result) const { getKeyI(result); }

  virtual void getValue(CacheablePtr& result) const { getValueI(result); }

  virtual void setValue(const CacheablePtr& value) { setValueI(value); }

  virtual MapEntryImplPtr getImplPtr() { return shared_from_this(); }

  virtual LRUEntryProperties& getLRUProperties() {
    throw FatalInternalException(
        "MapEntry::getLRUProperties called for "
        "non-LRU MapEntry");
  }

  virtual ExpEntryProperties& getExpProperties() {
    throw FatalInternalException(
        "MapEntry::getExpProperties called for "
        "non-expiration MapEntry");
  }

  virtual VersionStamp& getVersionStamp() {
    throw FatalInternalException(
        "MapEntry::getVersionStamp called for "
        "non-versioned MapEntry");
  }
  virtual void cleanup(const CacheEventFlags eventFlags) {}

 protected:
  inline explicit MapEntryImpl(bool noInit)
      : MapEntry(true), m_value(nullptr), m_key(nullptr) {}

  inline MapEntryImpl(const CacheableKeyPtr& key) : MapEntry(), m_key(key) {}

  CacheablePtr m_value;
  CacheableKeyPtr m_key;

 private:
  // disabled
  MapEntryImpl(const MapEntryImpl&);
  MapEntryImpl& operator=(const MapEntryImpl&);
};

class CPPCACHE_EXPORT VersionedMapEntryImpl : public MapEntryImpl,
                                              public VersionStamp {
 public:
  virtual ~VersionedMapEntryImpl() {}

  virtual VersionStamp& getVersionStamp() { return *this; }

 protected:
  inline explicit VersionedMapEntryImpl(bool noInit) : MapEntryImpl(true) {}

  inline VersionedMapEntryImpl(const CacheableKeyPtr& key)
      : MapEntryImpl(key) {}

 private:
  // disabled
  VersionedMapEntryImpl(const VersionedMapEntryImpl&);
  VersionedMapEntryImpl& operator=(const VersionedMapEntryImpl&);
};

typedef std::shared_ptr<VersionedMapEntryImpl> VersionedMapEntryImplPtr;

class CPPCACHE_EXPORT EntryFactory {
 public:
  EntryFactory(const bool concurrencyChecksEnabled)
      : m_concurrencyChecksEnabled(concurrencyChecksEnabled) {}

  virtual ~EntryFactory() {}

  virtual void newMapEntry(ExpiryTaskManager* expiryTaskManager,
                           const CacheableKeyPtr& key,
                           MapEntryImplPtr& result) const;

 protected:
  bool m_concurrencyChecksEnabled;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_MAPENTRY_H_
