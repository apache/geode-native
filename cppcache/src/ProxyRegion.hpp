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

#ifndef GEODE_PROXYREGION_H_
#define GEODE_PROXYREGION_H_

#include <algorithm>

#include <geode/AttributesMutator.hpp>
#include <geode/AuthenticatedView.hpp>
#include <geode/CacheListener.hpp>
#include <geode/CacheLoader.hpp>
#include <geode/CacheStatistics.hpp>
#include <geode/CacheWriter.hpp>
#include <geode/CacheableBuiltins.hpp>
#include <geode/CacheableKey.hpp>
#include <geode/CacheableString.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/Query.hpp>
#include <geode/RegionAttributes.hpp>
#include <geode/RegionAttributesFactory.hpp>
#include <geode/RegionEntry.hpp>
#include <geode/internal/geode_globals.hpp>

#include "RegionInternal.hpp"
#include "UserAttributes.hpp"

namespace apache {
namespace geode {
namespace client {

class FunctionService;

/**
 * @class ProxyRegion ProxyRegion.hpp
 * This class wrapper around real region
 */
class APACHE_GEODE_EXPORT ProxyRegion final : public Region {
 public:
  const std::string& getName() const override;

  const std::string& getFullPath() const override;

  std::shared_ptr<Region> getParentRegion() const override;

  const RegionAttributes& getAttributes() const override;

  std::shared_ptr<AttributesMutator> getAttributesMutator() const override;

  std::shared_ptr<CacheStatistics> getStatistics() const override;

  void invalidateRegion(const std::shared_ptr<Serializable>&) override;

  void localInvalidateRegion(const std::shared_ptr<Serializable>&) override;

  void destroyRegion(const std::shared_ptr<Serializable>& aCallbackArgument =
                         nullptr) override;

  void clear(const std::shared_ptr<Serializable>& aCallbackArgument =
                 nullptr) override;

  void localClear(const std::shared_ptr<Serializable>&) override;

  void localDestroyRegion(const std::shared_ptr<Serializable>&) override;

  std::shared_ptr<Region> getSubregion(const std::string& path) override;

  std::shared_ptr<Region> createSubregion(const std::string&,
                                          RegionAttributes) override;

  std::vector<std::shared_ptr<Region>> subregions(
      const bool recursive) override;

  std::shared_ptr<RegionEntry> getEntry(
      const std::shared_ptr<CacheableKey>& key) override;

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  std::shared_ptr<RegionEntry> getEntry(const KEYTYPE& key) {
    return getEntry(CacheableKey::create(key));
  }

  std::shared_ptr<Cacheable> get(const std::shared_ptr<CacheableKey>& key,
                                 const std::shared_ptr<Serializable>&
                                     aCallbackArgument = nullptr) override;

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  std::shared_ptr<Cacheable> get(
      const KEYTYPE& key,
      const std::shared_ptr<Serializable>& callbackArg = nullptr) {
    return get(CacheableKey::create(key), callbackArg);
  }

  void put(const std::shared_ptr<CacheableKey>& key,
           const std::shared_ptr<Cacheable>& value,
           const std::shared_ptr<Serializable>& aCallbackArgument =
               nullptr) override;

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  void put(const KEYTYPE& key, const VALUETYPE& value,
           const std::shared_ptr<Serializable>& arg = nullptr) {
    put(CacheableKey::create(key), Serializable::create(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  void put(const KEYTYPE& key, const std::shared_ptr<Cacheable>& value,
           const std::shared_ptr<Serializable>& arg = nullptr) {
    put(CacheableKey::create(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  void put(const std::shared_ptr<CacheableKey>& key, const VALUETYPE& value,
           const std::shared_ptr<Serializable>& arg = nullptr) {
    put(key, Serializable::create(value), arg);
  }

  void putAll(const HashMapOfCacheable& map,
              std::chrono::milliseconds timeout = DEFAULT_RESPONSE_TIMEOUT,
              const std::shared_ptr<Serializable>& aCallbackArgument =
                  nullptr) override;

  void localPut(const std::shared_ptr<CacheableKey>&,
                const std::shared_ptr<Cacheable>&,
                const std::shared_ptr<Serializable>&) override;

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  void localPut(const KEYTYPE& key, const VALUETYPE& value,
                const std::shared_ptr<Serializable>& arg = nullptr) {
    localPut(CacheableKey::create(key), Serializable::create(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  void localPut(const KEYTYPE& key, const std::shared_ptr<Cacheable>& value,
                const std::shared_ptr<Serializable>& arg = nullptr) {
    localPut(CacheableKey::create(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  void localPut(const std::shared_ptr<CacheableKey>& key,
                const VALUETYPE& value,
                const std::shared_ptr<Serializable>& arg = nullptr) {
    localPut(key, Serializable::create(value), arg);
  }

  void create(const std::shared_ptr<CacheableKey>& key,
              const std::shared_ptr<Cacheable>& value,
              const std::shared_ptr<Serializable>& aCallbackArgument =
                  nullptr) override;

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  void create(const KEYTYPE& key, const VALUETYPE& value,
              const std::shared_ptr<Serializable>& arg = nullptr) {
    create(CacheableKey::create(key), Serializable::create(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  void create(const KEYTYPE& key, const std::shared_ptr<Cacheable>& value,
              const std::shared_ptr<Serializable>& arg = nullptr) {
    create(CacheableKey::create(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  void create(const std::shared_ptr<CacheableKey>& key, const VALUETYPE& value,
              const std::shared_ptr<Serializable>& arg = nullptr) {
    create(key, Serializable::create(value), arg);
  }

  void localCreate(const std::shared_ptr<CacheableKey>&,
                   const std::shared_ptr<Cacheable>&,
                   const std::shared_ptr<Serializable>&) override;

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  void localCreate(const KEYTYPE& key, const VALUETYPE& value,
                   const std::shared_ptr<Serializable>& arg = nullptr) {
    localCreate(CacheableKey::create(key), Serializable::create(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  void localCreate(const KEYTYPE& key, const std::shared_ptr<Cacheable>& value,
                   const std::shared_ptr<Serializable>& arg = nullptr) {
    localCreate(CacheableKey::create(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  void localCreate(const std::shared_ptr<CacheableKey>& key,
                   const VALUETYPE& value,
                   const std::shared_ptr<Serializable>& arg = nullptr) {
    localCreate(key, Serializable::create(value), arg);
  }

  void invalidate(const std::shared_ptr<CacheableKey>& key,
                  const std::shared_ptr<Serializable>& aCallbackArgument =
                      nullptr) override;

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  void invalidate(const KEYTYPE& key,
                  const std::shared_ptr<Serializable>& arg = nullptr) {
    invalidate(CacheableKey::create(key), arg);
  }

  void localInvalidate(const std::shared_ptr<CacheableKey>&,
                       const std::shared_ptr<Serializable>&) override;

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  void localInvalidate(const KEYTYPE& key,
                       const std::shared_ptr<Serializable>& arg = nullptr) {
    localInvalidate(CacheableKey::create(key), arg);
  }

  void destroy(const std::shared_ptr<CacheableKey>& key,
               const std::shared_ptr<Serializable>& aCallbackArgument =
                   nullptr) override;

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  void destroy(const KEYTYPE& key,
               const std::shared_ptr<Serializable>& arg = nullptr) {
    destroy(CacheableKey::create(key), arg);
  }

  void localDestroy(const std::shared_ptr<CacheableKey>&,
                    const std::shared_ptr<Serializable>&) override;

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  void localDestroy(const KEYTYPE& key,
                    const std::shared_ptr<Serializable>& arg = nullptr);

  bool remove(const std::shared_ptr<CacheableKey>& key,
              const std::shared_ptr<Cacheable>& value,
              const std::shared_ptr<Serializable>& aCallbackArgument =
                  nullptr) override;

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  bool remove(const KEYTYPE& key, const VALUETYPE& value,
              const std::shared_ptr<Serializable>& arg = nullptr) {
    return remove(CacheableKey::create(key), Serializable::create(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  bool remove(const KEYTYPE& key, const std::shared_ptr<Cacheable>& value,
              const std::shared_ptr<Serializable>& arg = nullptr) {
    return remove(CacheableKey::create(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  bool remove(const std::shared_ptr<CacheableKey>& key, const VALUETYPE& value,
              const std::shared_ptr<Serializable>& arg = nullptr) {
    return remove(key, Serializable::create(value), arg);
  }

  bool removeEx(const std::shared_ptr<CacheableKey>& key,
                const std::shared_ptr<Serializable>& aCallbackArgument =
                    nullptr) override;

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  bool removeEx(const KEYTYPE& key,
                const std::shared_ptr<Serializable>& arg = nullptr) {
    return removeEx(CacheableKey::create(key), arg);
  }

  bool localRemove(const std::shared_ptr<CacheableKey>&,
                   const std::shared_ptr<Cacheable>&,
                   const std::shared_ptr<Serializable>&) override;

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  bool localRemove(const KEYTYPE& key, const VALUETYPE& value,
                   const std::shared_ptr<Serializable>& arg = nullptr) {
    return localRemove(CacheableKey::create(key), Serializable::create(value),
                       arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  bool localRemove(const KEYTYPE& key, const std::shared_ptr<Cacheable>& value,
                   const std::shared_ptr<Serializable>& arg = nullptr) {
    return localRemove(CacheableKey::create(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  bool localRemove(const std::shared_ptr<CacheableKey>& key,
                   const VALUETYPE& value,
                   const std::shared_ptr<Serializable>& arg = nullptr) {
    return localRemove(key, Serializable::create(value), arg);
  }

  bool localRemoveEx(const std::shared_ptr<CacheableKey>&,
                     const std::shared_ptr<Serializable>&) override;

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  bool localRemoveEx(const KEYTYPE& key,
                     const std::shared_ptr<Serializable>& arg = nullptr) {
    return localRemoveEx(CacheableKey::create(key), arg);
  }

  /**
   * Return all the keys in the local process for this region. This includes
   * keys for which the entry is invalid.
   */
  std::vector<std::shared_ptr<CacheableKey>> keys() override;

  std::vector<std::shared_ptr<CacheableKey>> serverKeys() override;

  std::vector<std::shared_ptr<Cacheable>> values() override;

  std::vector<std::shared_ptr<RegionEntry>> entries(bool) override;

  RegionService& getRegionService() const override;

  bool isDestroyed() const override;

  bool containsValueForKey(const std::shared_ptr<CacheableKey>&) const override;

  /**
   * Convenience method allowing key to be a const char*
   * This operations checks for the value in the local cache .
   * It is not propagated to the Geode cache server
   * to which it is connected.
   */
  template <class KEYTYPE>
  bool containsValueForKey(const KEYTYPE& key) const {
    return containsValueForKey(CacheableKey::create(key));
  }

  bool containsKey(const std::shared_ptr<CacheableKey>&) const override;

  bool containsKeyOnServer(
      const std::shared_ptr<CacheableKey>& keyPtr) const override;

  std::vector<std::shared_ptr<CacheableKey>> getInterestList() const override;

  std::vector<std::shared_ptr<CacheableString>> getInterestListRegex()
      const override;

  /**
   * Convenience method allowing key to be a const char*
   * This operations checks for the key in the local cache .
   * It is not propagated to the Geode cache server
   * to which it is connected.
   */
  template <class KEYTYPE>
  bool containsKey(const KEYTYPE& key) const {
    return containsKey(CacheableKey::create(key));
  }

  void registerKeys(const std::vector<std::shared_ptr<CacheableKey>>&, bool,
                    bool, bool) override;

  void unregisterKeys(
      const std::vector<std::shared_ptr<CacheableKey>>&) override;

  void registerAllKeys(bool, bool, bool) override;

  void unregisterAllKeys() override;

  void registerRegex(const std::string&, bool, bool, bool) override;

  void unregisterRegex(const std::string&) override;

  HashMapOfCacheable getAll(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      const std::shared_ptr<Serializable>& aCallbackArgument =
          nullptr) override;

  std::shared_ptr<SelectResults> query(
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

  void removeAll(const std::vector<std::shared_ptr<CacheableKey>>& keys,
                 const std::shared_ptr<Serializable>& aCallbackArgument =
                     nullptr) override;

  uint32_t size() override;

  const std::shared_ptr<Pool>& getPool() const override;

  ProxyRegion(AuthenticatedView& authenticatedView,
              const std::shared_ptr<RegionInternal>& realRegion);

  ~ProxyRegion() override = default;

  ProxyRegion(const ProxyRegion&) = delete;
  ProxyRegion& operator=(const ProxyRegion&) = delete;

 private:
  AuthenticatedView* m_authenticatedView;
  std::shared_ptr<RegionInternal> m_realRegion;
  friend class FunctionService;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PROXYREGION_H_
