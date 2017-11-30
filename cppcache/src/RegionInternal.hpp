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

#ifndef GEODE_REGIONINTERNAL_H_
#define GEODE_REGIONINTERNAL_H_

#include <string>
#include <map>
#include <chrono>

#include <geode/Region.hpp>

#include "RegionStats.hpp"
#include "EventId.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * @class CacheEventFlags RegionInternal.hpp
 *
 * This class encapsulates the flags (e.g. notification, expiration, local)
 * for cache events for various NoThrow methods.
 *
 *
 */
class CacheEventFlags {
 private:
  uint8_t m_flags;
  static const uint8_t GF_NORMAL = 0x01;
  static const uint8_t GF_LOCAL = 0x02;
  static const uint8_t GF_NOTIFICATION = 0x04;
  static const uint8_t GF_NOTIFICATION_UPDATE = 0x08;
  static const uint8_t GF_EVICTION = 0x10;
  static const uint8_t GF_EXPIRATION = 0x20;
  static const uint8_t GF_CACHE_CLOSE = 0x40;
  static const uint8_t GF_NOCACHEWRITER = 0x80;

  // private constructor
  inline CacheEventFlags(const uint8_t flags) : m_flags(flags) {}

  // disable constructors and assignment
  CacheEventFlags();
  CacheEventFlags& operator=(const CacheEventFlags&);

 public:
  static const CacheEventFlags NORMAL;
  static const CacheEventFlags LOCAL;
  static const CacheEventFlags NOTIFICATION;
  static const CacheEventFlags NOTIFICATION_UPDATE;
  static const CacheEventFlags EVICTION;
  static const CacheEventFlags EXPIRATION;
  static const CacheEventFlags CACHE_CLOSE;
  static const CacheEventFlags NOCACHEWRITER;

  inline CacheEventFlags(const CacheEventFlags& flags)
      : m_flags(flags.m_flags) {}

  inline CacheEventFlags operator|(const CacheEventFlags& flags) const {
    return CacheEventFlags(m_flags | flags.m_flags);
  }

  inline uint32_t operator&(const CacheEventFlags& flags) const {
    return (m_flags & flags.m_flags);
  }

  inline bool operator==(const CacheEventFlags& flags) const {
    return (m_flags == flags.m_flags);
  }

  inline bool isNormal() const {
    return (m_flags & GF_NORMAL) > 0 ? true : false;
  }

  inline bool isLocal() const {
    return (m_flags & GF_LOCAL) > 0 ? true : false;
  }

  inline bool isNotification() const {
    return (m_flags & GF_NOTIFICATION) > 0 ? true : false;
  }

  inline bool isNotificationUpdate() const {
    return (m_flags & GF_NOTIFICATION_UPDATE) > 0 ? true : false;
  }

  inline bool isEviction() const {
    return (m_flags & GF_EVICTION) > 0 ? true : false;
  }

  inline bool isExpiration() const {
    return (m_flags & GF_EXPIRATION) > 0 ? true : false;
  }

  inline bool isCacheClose() const {
    return (m_flags & GF_CACHE_CLOSE) > 0 ? true : false;
  }

  inline bool isNoCacheWriter() const {
    return (m_flags & GF_NOCACHEWRITER) > 0 ? true : false;
  }

  inline bool isEvictOrExpire() const {
    return (m_flags & (GF_EVICTION | GF_EXPIRATION)) > 0 ? true : false;
  }

  // special optimized method for CacheWriter invocation condition
  inline bool invokeCacheWriter() const {
    return ((m_flags & (GF_NOTIFICATION | GF_EVICTION | GF_EXPIRATION |
                        GF_NOCACHEWRITER)) == 0x0);
  }
};

class TombstoneList;
class VersionTag;
class MapEntryImpl;
/**
 * @class RegionInternal RegionInternal.hpp
 *
 * This class specifies internal common interface for all regions.
 */
class RegionInternal : public Region {
 public:
  /**
   * @brief destructor
   */
  virtual ~RegionInternal();
  /** @brief Default implementation of Public Methods from Region
   */
  virtual void registerKeys(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      bool isDurable = false, bool getInitialValues = false,
      bool receiveValues = true) override;
  virtual void unregisterKeys(
      const std::vector<std::shared_ptr<CacheableKey>>& keys) override;
  virtual void registerAllKeys(bool isDurable = false,
                               bool getInitialValues = false,
                               bool receiveValues = true) override;
  virtual void unregisterAllKeys() override;

  virtual void registerRegex(const char* regex, bool isDurable = false,
                             bool getInitialValues = false,
                             bool receiveValues = true) override;
  virtual void unregisterRegex(const char* regex) override;

  virtual std::shared_ptr<SelectResults> query(
      const char* predicate, std::chrono::milliseconds timeout =
                                 DEFAULT_QUERY_RESPONSE_TIMEOUT) override;

  virtual bool existsValue(const char* predicate,
                           std::chrono::milliseconds timeout =
                               DEFAULT_QUERY_RESPONSE_TIMEOUT) override;

  virtual std::shared_ptr<Serializable> selectValue(
      const char* predicate, std::chrono::milliseconds timeout =
                                 DEFAULT_QUERY_RESPONSE_TIMEOUT) override;

  /** @brief Public Methods
   */
  virtual std::shared_ptr<PersistenceManager> getPersistenceManager() = 0;
  virtual void setPersistenceManager(std::shared_ptr<PersistenceManager>& pmPtr) = 0;

  virtual GfErrType getNoThrow(
      const std::shared_ptr<CacheableKey>& key,
      std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument) = 0;

  virtual HashMapOfCacheable getAll_internal(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      bool addToLocalCache) = 0;

  virtual GfErrType getAllNoThrow(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      const std::shared_ptr<HashMapOfCacheable>& values,
      const std::shared_ptr<HashMapOfException>& exceptions,
      const bool addToLocalCache,
      const std::shared_ptr<Serializable>& aCallbackArgument) = 0;
  virtual GfErrType putNoThrow(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<Cacheable>& oldValue, int updateCount,
      const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag,
      DataInput* delta = nullptr,
      std::shared_ptr<EventId> eventId = nullptr) = 0;
  virtual GfErrType createNoThrow(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
      const CacheEventFlags eventFlags,
      std::shared_ptr<VersionTag> versionTag) = 0;
  virtual GfErrType destroyNoThrow(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
      const CacheEventFlags eventFlags,
      std::shared_ptr<VersionTag> versionTag) = 0;
  virtual GfErrType removeNoThrow(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
      const CacheEventFlags eventFlags,
      std::shared_ptr<VersionTag> versionTag) = 0;
  virtual GfErrType invalidateNoThrow(
      const std::shared_ptr<CacheableKey>& keyPtr,
      const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
      const CacheEventFlags eventFlags,
      std::shared_ptr<VersionTag> versionTag) = 0;
  virtual GfErrType invalidateRegionNoThrow(
      const std::shared_ptr<Serializable>& aCallbackArgument,
      const CacheEventFlags eventFlags) = 0;
  virtual GfErrType destroyRegionNoThrow(
      const std::shared_ptr<Serializable>& aCallbackArgument,
      bool removeFromParent, const CacheEventFlags eventFlags) = 0;

  virtual void setRegionExpiryTask() = 0;
  virtual void acquireReadLock() = 0;
  virtual void releaseReadLock() = 0;
  // behaviors for attributes mutator
  virtual uint32_t adjustLruEntriesLimit(uint32_t limit) = 0;
  virtual ExpirationAction::Action adjustRegionExpiryAction(
      ExpirationAction::Action action) = 0;
  virtual ExpirationAction::Action adjustEntryExpiryAction(
      ExpirationAction::Action action) = 0;
  virtual std::chrono::seconds adjustRegionExpiryDuration(
      const std::chrono::seconds& duration) = 0;
  virtual std::chrono::seconds adjustEntryExpiryDuration(
      const std::chrono::seconds& duration) = 0;
  virtual void adjustCacheListener(const std::shared_ptr<CacheListener>& aListener) = 0;
  virtual void adjustCacheListener(const char* libpath,
                                   const char* factoryFuncName) = 0;
  virtual void adjustCacheLoader(const std::shared_ptr<CacheLoader>& aLoader) = 0;
  virtual void adjustCacheLoader(const char* libpath,
                                 const char* factoryFuncName) = 0;
  virtual void adjustCacheWriter(const std::shared_ptr<CacheWriter>& aWriter) = 0;
  virtual void adjustCacheWriter(const char* libpath,
                                 const char* factoryFuncName) = 0;

  virtual RegionStats* getRegionStats() = 0;
  virtual bool cacheEnabled() = 0;
  virtual bool isDestroyed() const override = 0;
  virtual void evict(int32_t percentage) = 0;
  virtual CacheImpl* getCacheImpl() const = 0;
  virtual std::shared_ptr<TombstoneList> getTombstoneList();

  // KN: added now.
  virtual void updateAccessAndModifiedTime(bool modified) = 0;
  virtual void updateAccessAndModifiedTimeForEntry(
      std::shared_ptr<MapEntryImpl>& ptr, bool modified) = 0;
  std::shared_ptr<RegionEntry> createRegionEntry(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value);
  virtual void addDisMessToQueue(){};

  virtual void txDestroy(const std::shared_ptr<CacheableKey>& key,
                         const std::shared_ptr<Serializable>& callBack,
                         std::shared_ptr<VersionTag> versionTag);
  virtual void txInvalidate(const std::shared_ptr<CacheableKey>& key,
                            const std::shared_ptr<Serializable>& callBack,
                            std::shared_ptr<VersionTag> versionTag);
  virtual void txPut(const std::shared_ptr<CacheableKey>& key,
                     const std::shared_ptr<Cacheable>& value,
                     const std::shared_ptr<Serializable>& callBack,
                     std::shared_ptr<VersionTag> versionTag);
  inline bool isConcurrencyCheckEnabled() const {
    return m_regionAttributes->getConcurrencyChecksEnabled();
  }
  virtual const std::shared_ptr<Pool>& getPool() override = 0;

 protected:
  /**
   * @brief constructor
   */
  RegionInternal(Cache* cache, const std::shared_ptr<RegionAttributes>& attributes);

  void setLruEntriesLimit(uint32_t limit);
  void setRegionTimeToLiveExpirationAction(ExpirationAction::Action action);
  void setRegionIdleTimeoutExpirationAction(ExpirationAction::Action action);
  void setEntryTimeToLiveExpirationAction(ExpirationAction::Action action);
  void setEntryIdleTimeoutExpirationAction(ExpirationAction::Action action);
  void setRegionTimeToLive(const std::chrono::seconds& duration);
  void setRegionIdleTimeout(const std::chrono::seconds& duration);
  void setEntryTimeToLive(const std::chrono::seconds& duration);
  void setEntryIdleTimeout(const std::chrono::seconds& duration);
  void setCacheListener(const std::shared_ptr<CacheListener>& aListener);
  void setCacheListener(const char* libpath, const char* factoryFuncName);
  void setPartitionResolver(
      const std::shared_ptr<PartitionResolver>& aListener);
  void setPartitionResolver(const char* libpath, const char* factoryFuncName);
  void setCacheLoader(const std::shared_ptr<CacheLoader>& aLoader);
  void setCacheLoader(const char* libpath, const char* factoryFuncName);
  void setCacheWriter(const std::shared_ptr<CacheWriter>& aWriter);
  void setCacheWriter(const char* libpath, const char* factoryFuncName);
  void setEndpoints(const char* endpoints);
  void setClientNotificationEnabled(bool clientNotificationEnabled);

  std::shared_ptr<RegionAttributes> m_regionAttributes;

  inline bool entryExpiryEnabled() const {
    return m_regionAttributes->getEntryExpiryEnabled();
  }

  inline bool regionExpiryEnabled() const {
    return m_regionAttributes->getRegionExpiryEnabled();
  }

  RegionInternal(const RegionInternal&) = delete;
  RegionInternal& operator=(const RegionInternal&) = delete;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_REGIONINTERNAL_H_
