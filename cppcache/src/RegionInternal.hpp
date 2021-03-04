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

#include <chrono>
#include <map>
#include <string>

#include <geode/Region.hpp>

#include "ErrType.hpp"
#include "EventId.hpp"
#include "HashMapOfException.hpp"
#include "RegionStats.hpp"

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
  uint16_t m_flags;
  static const uint16_t GF_NORMAL = 0x01;
  static const uint16_t GF_LOCAL = 0x02;
  static const uint16_t GF_NOTIFICATION = 0x04;
  static const uint16_t GF_NOTIFICATION_UPDATE = 0x08;
  static const uint16_t GF_EVICTION = 0x10;
  static const uint16_t GF_EXPIRATION = 0x20;
  static const uint16_t GF_CACHE_CLOSE = 0x40;
  static const uint16_t GF_NOCACHEWRITER = 0x80;
  static const uint16_t GF_NOCALLBACKS = 0x100;

  inline explicit CacheEventFlags(const uint16_t flags) : m_flags(flags) {}

 public:
  static const CacheEventFlags NORMAL;
  static const CacheEventFlags LOCAL;
  static const CacheEventFlags NOTIFICATION;
  static const CacheEventFlags NOTIFICATION_UPDATE;
  static const CacheEventFlags EVICTION;
  static const CacheEventFlags EXPIRATION;
  static const CacheEventFlags CACHE_CLOSE;
  static const CacheEventFlags NOCACHEWRITER;
  static const CacheEventFlags NOCALLBACKS;

  inline CacheEventFlags(const CacheEventFlags& flags) = default;

  CacheEventFlags() = delete;
  CacheEventFlags& operator=(const CacheEventFlags&) = delete;

  inline CacheEventFlags operator|(const CacheEventFlags& flags) const {
    return CacheEventFlags(m_flags | flags.m_flags);
  }

  inline uint32_t operator&(const CacheEventFlags& flags) const {
    return (m_flags & flags.m_flags);
  }

  inline bool operator==(const CacheEventFlags& flags) const {
    return (m_flags == flags.m_flags);
  }

  inline bool isNormal() const { return (m_flags & GF_NORMAL) > 0; }

  inline bool isLocal() const { return (m_flags & GF_LOCAL) > 0; }

  inline bool isNotification() const { return (m_flags & GF_NOTIFICATION) > 0; }

  inline bool isNotificationUpdate() const {
    return (m_flags & GF_NOTIFICATION_UPDATE) > 0;
  }

  inline bool isEviction() const { return (m_flags & GF_EVICTION) > 0; }

  inline bool isExpiration() const { return (m_flags & GF_EXPIRATION) > 0; }

  inline bool isCacheClose() const { return (m_flags & GF_CACHE_CLOSE) > 0; }

  inline bool isNoCacheWriter() const {
    return (m_flags & GF_NOCACHEWRITER) > 0;
  }

  inline bool isNoCallbacks() const { return (m_flags & GF_NOCALLBACKS) > 0; }

  inline bool isEvictOrExpire() const {
    return (m_flags & (GF_EVICTION | GF_EXPIRATION)) > 0;
  }

  // special optimized method for CacheWriter invocation condition
  inline bool invokeCacheWriter() const {
    return ((m_flags & (GF_NOTIFICATION | GF_EVICTION | GF_EXPIRATION |
                        GF_NOCACHEWRITER | GF_NOCALLBACKS)) == 0x0);
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
  RegionInternal(const RegionInternal&) = delete;
  RegionInternal& operator=(const RegionInternal&) = delete;

  ~RegionInternal() noexcept override;

  void registerKeys(const std::vector<std::shared_ptr<CacheableKey>>& keys,
                    bool isDurable = false, bool getInitialValues = false,
                    bool receiveValues = true) override;

  void unregisterKeys(
      const std::vector<std::shared_ptr<CacheableKey>>& keys) override;

  void registerAllKeys(bool isDurable = false, bool getInitialValues = false,
                       bool receiveValues = true) override;
  void unregisterAllKeys() override;

  void registerRegex(const std::string& regex, bool isDurable = false,
                     bool getInitialValues = false,
                     bool receiveValues = true) override;
  virtual void unregisterRegex(const std::string& regex) override;

  virtual std::shared_ptr<SelectResults> query(
      const std::string& predicate,
      std::chrono::milliseconds timeout =
          DEFAULT_QUERY_RESPONSE_TIMEOUT) override;

  bool existsValue(const std::string& predicate,
                   std::chrono::milliseconds timeout =
                       DEFAULT_QUERY_RESPONSE_TIMEOUT) override;

  std::shared_ptr<Serializable> selectValue(
      const std::string& predicate,
      std::chrono::milliseconds timeout =
          DEFAULT_QUERY_RESPONSE_TIMEOUT) override;

  /** @brief Public Methods
   */
  virtual std::shared_ptr<PersistenceManager> getPersistenceManager() = 0;
  virtual void setPersistenceManager(
      std::shared_ptr<PersistenceManager>& pmPtr) = 0;

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
  virtual ExpirationAction adjustRegionExpiryAction(
      ExpirationAction action) = 0;
  virtual ExpirationAction adjustEntryExpiryAction(ExpirationAction action) = 0;
  virtual std::chrono::seconds adjustRegionExpiryDuration(
      const std::chrono::seconds& duration) = 0;
  virtual std::chrono::seconds adjustEntryExpiryDuration(
      const std::chrono::seconds& duration) = 0;
  virtual void adjustCacheListener(
      const std::shared_ptr<CacheListener>& aListener) = 0;
  virtual void adjustCacheListener(const std::string& libpath,
                                   const std::string& factoryFuncName) = 0;
  virtual void adjustCacheLoader(
      const std::shared_ptr<CacheLoader>& aLoader) = 0;
  virtual void adjustCacheLoader(const std::string& libpath,
                                 const std::string& factoryFuncName) = 0;
  virtual void adjustCacheWriter(
      const std::shared_ptr<CacheWriter>& aWriter) = 0;
  virtual void adjustCacheWriter(const std::string& libpath,
                                 const std::string& factoryFuncName) = 0;

  virtual RegionStats* getRegionStats() = 0;
  virtual bool cacheEnabled() = 0;
  bool isDestroyed() const override = 0;
  virtual void evict(float percentage) = 0;
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
    return m_regionAttributes.getConcurrencyChecksEnabled();
  }
  const std::shared_ptr<Pool>& getPool() const override = 0;

 protected:
  RegionInternal(CacheImpl* cache, RegionAttributes attributes);

  void setLruEntriesLimit(uint32_t limit);
  void setRegionTimeToLiveExpirationAction(ExpirationAction action);
  void setRegionIdleTimeoutExpirationAction(ExpirationAction action);
  void setEntryTimeToLiveExpirationAction(ExpirationAction action);
  void setEntryIdleTimeoutExpirationAction(ExpirationAction action);
  void setRegionTimeToLive(const std::chrono::seconds& duration);
  void setRegionIdleTimeout(const std::chrono::seconds& duration);
  void setEntryTimeToLive(const std::chrono::seconds& duration);
  void setEntryIdleTimeout(const std::chrono::seconds& duration);
  void setCacheListener(const std::shared_ptr<CacheListener>& aListener);
  void setCacheListener(const std::string& libpath,
                        const std::string& factoryFuncName);
  void setPartitionResolver(
      const std::shared_ptr<PartitionResolver>& aListener);
  void setPartitionResolver(const std::string& libpath,
                            const std::string& factoryFuncName);
  void setCacheLoader(const std::shared_ptr<CacheLoader>& aLoader);
  void setCacheLoader(const std::string& libpath,
                      const std::string& factoryFuncName);
  void setCacheWriter(const std::shared_ptr<CacheWriter>& aWriter);
  void setCacheWriter(const std::string& libpath,
                      const std::string& factoryFuncName);
  void setEndpoints(const std::string& endpoints);
  void setClientNotificationEnabled(bool clientNotificationEnabled);

  RegionAttributes m_regionAttributes;

  inline bool entryExpiryEnabled() const {
    return m_regionAttributes.getEntryExpiryEnabled();
  }

  inline bool regionExpiryEnabled() const {
    return m_regionAttributes.getRegionExpiryEnabled();
  }
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_REGIONINTERNAL_H_
