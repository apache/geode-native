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
class ProxyRegion final : public Region {
 public:
  const std::string& getName() const final { return m_realRegion->getName(); }

  const std::string& getFullPath() const final {
    return m_realRegion->getFullPath();
  }

  std::shared_ptr<Region> getParentRegion() const final {
    return m_realRegion->getParentRegion();
  }

  const RegionAttributes& getAttributes() const final {
    return m_realRegion->getAttributes();
  }

  std::shared_ptr<AttributesMutator> getAttributesMutator() const final {
    throw UnsupportedOperationException("Region.getAttributesMutator()");
  }

  std::shared_ptr<CacheStatistics> getStatistics() const final {
    return m_realRegion->getStatistics();
  }

  void invalidateRegion(const std::shared_ptr<Serializable>&) final {
    throw UnsupportedOperationException("Region.invalidateRegion()");
  }

  void localInvalidateRegion(const std::shared_ptr<Serializable>&) final {
    throw UnsupportedOperationException("Region.localInvalidateRegion()");
  }

  void destroyRegion(
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) final {
    GuardUserAttributes gua(m_authenticatedView);
    m_realRegion->destroyRegion(aCallbackArgument);
  }

  void clear(
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) final {
    GuardUserAttributes gua(m_authenticatedView);
    m_realRegion->clear(aCallbackArgument);
  }

  void localClear(const std::shared_ptr<Serializable>&) final {
    throw UnsupportedOperationException("localClear()");
  }

  void localDestroyRegion(const std::shared_ptr<Serializable>&) final {
    throw UnsupportedOperationException("Region.localDestroyRegion()");
  }

  std::shared_ptr<Region> getSubregion(const std::string& path) final {
    LOG_DEBUG("ProxyRegion getSubregion");
    auto rPtr = std::static_pointer_cast<RegionInternal>(
        m_realRegion->getSubregion(path));

    if (rPtr == nullptr) {
      return std::move(rPtr);
    }

    return std::make_shared<ProxyRegion>(*m_authenticatedView, rPtr);
  }

  std::shared_ptr<Region> createSubregion(const std::string&,
                                          RegionAttributes) final {
    throw UnsupportedOperationException("createSubregion()");
  }

  std::vector<std::shared_ptr<Region>> subregions(const bool recursive) final {
    std::vector<std::shared_ptr<Region>> realVectorRegion =
        m_realRegion->subregions(recursive);
    std::vector<std::shared_ptr<Region>> proxyRegions(realVectorRegion.size());

    std::transform(realVectorRegion.begin(), realVectorRegion.end(),
                   std::back_inserter(proxyRegions),
                   [this](const std::shared_ptr<Region>& realRegion)
                       -> std::shared_ptr<ProxyRegion> {
                     return std::make_shared<ProxyRegion>(
                         *m_authenticatedView,
                         std::static_pointer_cast<RegionInternal>(realRegion));
                   });

    return proxyRegions;
  }

  std::shared_ptr<RegionEntry> getEntry(
      const std::shared_ptr<CacheableKey>& key) final {
    return m_realRegion->getEntry(key);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline std::shared_ptr<RegionEntry> getEntry(const KEYTYPE& key) {
    return getEntry(CacheableKey::create(key));
  }

  std::shared_ptr<Cacheable> get(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) final {
    GuardUserAttributes gua(m_authenticatedView);
    return m_realRegion->get(key, aCallbackArgument);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline std::shared_ptr<Cacheable> get(
      const KEYTYPE& key,
      const std::shared_ptr<Serializable>& callbackArg = nullptr) {
    return get(CacheableKey::create(key), callbackArg);
  }

  void put(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) final {
    GuardUserAttributes gua(m_authenticatedView);
    return m_realRegion->put(key, value, aCallbackArgument);
  }

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline void put(const KEYTYPE& key, const VALUETYPE& value,
                  const std::shared_ptr<Serializable>& arg = nullptr) {
    put(CacheableKey::create(key), Serializable::create(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void put(const KEYTYPE& key, const std::shared_ptr<Cacheable>& value,
                  const std::shared_ptr<Serializable>& arg = nullptr) {
    put(CacheableKey::create(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline void put(const std::shared_ptr<CacheableKey>& key,
                  const VALUETYPE& value,
                  const std::shared_ptr<Serializable>& arg = nullptr) {
    put(key, Serializable::create(value), arg);
  }

  void putAll(
      const HashMapOfCacheable& map,
      std::chrono::milliseconds timeout = DEFAULT_RESPONSE_TIMEOUT,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) final {
    GuardUserAttributes gua(m_authenticatedView);
    return m_realRegion->putAll(map, timeout, aCallbackArgument);
  }

  void localPut(const std::shared_ptr<CacheableKey>&,
                const std::shared_ptr<Cacheable>&,
                const std::shared_ptr<Serializable>&) final {
    throw UnsupportedOperationException("Region.localPut()");
  }

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline void localPut(const KEYTYPE& key, const VALUETYPE& value,
                       const std::shared_ptr<Serializable>& arg = nullptr) {
    localPut(CacheableKey::create(key), Serializable::create(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void localPut(const KEYTYPE& key,
                       const std::shared_ptr<Cacheable>& value,
                       const std::shared_ptr<Serializable>& arg = nullptr) {
    localPut(CacheableKey::create(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline void localPut(const std::shared_ptr<CacheableKey>& key,
                       const VALUETYPE& value,
                       const std::shared_ptr<Serializable>& arg = nullptr) {
    localPut(key, Serializable::create(value), arg);
  }

  void create(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) final {
    GuardUserAttributes gua(m_authenticatedView);
    m_realRegion->create(key, value, aCallbackArgument);
  }

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline void create(const KEYTYPE& key, const VALUETYPE& value,
                     const std::shared_ptr<Serializable>& arg = nullptr) {
    create(CacheableKey::create(key), Serializable::create(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void create(const KEYTYPE& key,
                     const std::shared_ptr<Cacheable>& value,
                     const std::shared_ptr<Serializable>& arg = nullptr) {
    create(CacheableKey::create(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline void create(const std::shared_ptr<CacheableKey>& key,
                     const VALUETYPE& value,
                     const std::shared_ptr<Serializable>& arg = nullptr) {
    create(key, Serializable::create(value), arg);
  }

  void localCreate(const std::shared_ptr<CacheableKey>&,
                   const std::shared_ptr<Cacheable>&,
                   const std::shared_ptr<Serializable>&) final {
    throw UnsupportedOperationException("Region.localCreate()");
  }

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline void localCreate(const KEYTYPE& key, const VALUETYPE& value,
                          const std::shared_ptr<Serializable>& arg = nullptr) {
    localCreate(CacheableKey::create(key), Serializable::create(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void localCreate(const KEYTYPE& key,
                          const std::shared_ptr<Cacheable>& value,
                          const std::shared_ptr<Serializable>& arg = nullptr) {
    localCreate(CacheableKey::create(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline void localCreate(const std::shared_ptr<CacheableKey>& key,
                          const VALUETYPE& value,
                          const std::shared_ptr<Serializable>& arg = nullptr) {
    localCreate(key, Serializable::create(value), arg);
  }

  void invalidate(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) final {
    GuardUserAttributes gua(m_authenticatedView);
    m_realRegion->invalidate(key, aCallbackArgument);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void invalidate(const KEYTYPE& key,
                         const std::shared_ptr<Serializable>& arg = nullptr) {
    invalidate(CacheableKey::create(key), arg);
  }

  void localInvalidate(const std::shared_ptr<CacheableKey>&,
                       const std::shared_ptr<Serializable>&) final {
    throw UnsupportedOperationException("Region.localInvalidate()");
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void localInvalidate(
      const KEYTYPE& key, const std::shared_ptr<Serializable>& arg = nullptr) {
    localInvalidate(CacheableKey::create(key), arg);
  }

  void destroy(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) final {
    GuardUserAttributes gua(m_authenticatedView);
    m_realRegion->destroy(key, aCallbackArgument);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void destroy(const KEYTYPE& key,
                      const std::shared_ptr<Serializable>& arg = nullptr) {
    destroy(CacheableKey::create(key), arg);
  }

  void localDestroy(const std::shared_ptr<CacheableKey>&,
                    const std::shared_ptr<Serializable>&) final {
    throw UnsupportedOperationException("Region.localDestroy()");
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void localDestroy(const KEYTYPE& key,
                           const std::shared_ptr<Serializable>& arg = nullptr) {
    localDestroy(CacheableKey::create(key), arg);
  }

  bool remove(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) final {
    GuardUserAttributes gua(m_authenticatedView);
    return m_realRegion->remove(key, value, aCallbackArgument);
  }

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline bool remove(const KEYTYPE& key, const VALUETYPE& value,
                     const std::shared_ptr<Serializable>& arg = nullptr) {
    return remove(CacheableKey::create(key), Serializable::create(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline bool remove(const KEYTYPE& key,
                     const std::shared_ptr<Cacheable>& value,
                     const std::shared_ptr<Serializable>& arg = nullptr) {
    return remove(CacheableKey::create(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline bool remove(const std::shared_ptr<CacheableKey>& key,
                     const VALUETYPE& value,
                     const std::shared_ptr<Serializable>& arg = nullptr) {
    return remove(key, Serializable::create(value), arg);
  }

  bool removeEx(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) final {
    GuardUserAttributes gua(m_authenticatedView);
    return m_realRegion->removeEx(key, aCallbackArgument);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline bool removeEx(const KEYTYPE& key,
                       const std::shared_ptr<Serializable>& arg = nullptr) {
    return removeEx(CacheableKey::create(key), arg);
  }

  bool localRemove(const std::shared_ptr<CacheableKey>&,
                   const std::shared_ptr<Cacheable>&,
                   const std::shared_ptr<Serializable>&) final {
    throw UnsupportedOperationException("Region.localRemove()");
  }

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline bool localRemove(const KEYTYPE& key, const VALUETYPE& value,
                          const std::shared_ptr<Serializable>& arg = nullptr) {
    return localRemove(CacheableKey::create(key), Serializable::create(value),
                       arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline bool localRemove(const KEYTYPE& key,
                          const std::shared_ptr<Cacheable>& value,
                          const std::shared_ptr<Serializable>& arg = nullptr) {
    return localRemove(CacheableKey::create(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline bool localRemove(const std::shared_ptr<CacheableKey>& key,
                          const VALUETYPE& value,
                          const std::shared_ptr<Serializable>& arg = nullptr) {
    return localRemove(key, Serializable::create(value), arg);
  }

  bool localRemoveEx(const std::shared_ptr<CacheableKey>&,
                     const std::shared_ptr<Serializable>&) final {
    throw UnsupportedOperationException("Region.localRemoveEx()");
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline bool localRemoveEx(
      const KEYTYPE& key, const std::shared_ptr<Serializable>& arg = nullptr) {
    return localRemoveEx(CacheableKey::create(key), arg);
  }

  /**
   * Return all the keys in the local process for this region. This includes
   * keys for which the entry is invalid.
   */
  std::vector<std::shared_ptr<CacheableKey>> keys() final {
    throw UnsupportedOperationException("Region.keys()");
  }

  std::vector<std::shared_ptr<CacheableKey>> serverKeys() final {
    GuardUserAttributes gua(m_authenticatedView);
    return m_realRegion->serverKeys();
  }

  std::vector<std::shared_ptr<Cacheable>> values() final {
    throw UnsupportedOperationException("Region.values()");
  }

  std::vector<std::shared_ptr<RegionEntry>> entries(bool) final {
    throw UnsupportedOperationException("Region.entries()");
  }

  RegionService& getRegionService() const final { return *m_authenticatedView; }

  bool isDestroyed() const final { return m_realRegion->isDestroyed(); }

  bool containsValueForKey(const std::shared_ptr<CacheableKey>&) const final {
    throw UnsupportedOperationException("Region.containsValueForKey()");
  }

  /**
   * Convenience method allowing key to be a const char*
   * This operations checks for the value in the local cache .
   * It is not propagated to the Geode cache server
   * to which it is connected.
   */
  template <class KEYTYPE>
  inline bool containsValueForKey(const KEYTYPE& key) const {
    return containsValueForKey(CacheableKey::create(key));
  }

  bool containsKey(const std::shared_ptr<CacheableKey>&) const final {
    throw UnsupportedOperationException("Region.containsKey()");
  }

  bool containsKeyOnServer(
      const std::shared_ptr<CacheableKey>& keyPtr) const final {
    GuardUserAttributes gua(m_authenticatedView);
    return m_realRegion->containsKeyOnServer(keyPtr);
  }

  std::vector<std::shared_ptr<CacheableKey>> getInterestList() const final {
    throw UnsupportedOperationException("Region.getInterestList()");
  }

  std::vector<std::shared_ptr<CacheableString>> getInterestListRegex()
      const final {
    throw UnsupportedOperationException("Region.getInterestListRegex()");
  }

  /**
   * Convenience method allowing key to be a const char*
   * This operations checks for the key in the local cache .
   * It is not propagated to the Geode cache server
   * to which it is connected.
   */
  template <class KEYTYPE>
  inline bool containsKey(const KEYTYPE& key) const {
    return containsKey(CacheableKey::create(key));
  }

  void registerKeys(const std::vector<std::shared_ptr<CacheableKey>>&, bool,
                    bool, bool) final {
    throw UnsupportedOperationException("Region.registerKeys()");
  }

  void unregisterKeys(const std::vector<std::shared_ptr<CacheableKey>>&) final {
    throw UnsupportedOperationException("Region.unregisterKeys()");
  }

  void registerAllKeys(bool, bool, bool) final {
    throw UnsupportedOperationException("Region.registerAllKeys()");
  }

  void unregisterAllKeys() final {
    throw UnsupportedOperationException("Region.unregisterAllKeys()");
  }

  void registerRegex(const std::string&, bool, bool, bool) final {
    throw UnsupportedOperationException("Region.registerRegex()");
  }

  void unregisterRegex(const std::string&) final {
    throw UnsupportedOperationException("Region.unregisterRegex()");
  }

  HashMapOfCacheable getAll(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) final {
    GuardUserAttributes gua(m_authenticatedView);
    return m_realRegion->getAll_internal(keys, aCallbackArgument, false);
  }

  std::shared_ptr<SelectResults> query(
      const std::string& predicate, std::chrono::milliseconds timeout =
                                        DEFAULT_QUERY_RESPONSE_TIMEOUT) final {
    GuardUserAttributes gua(m_authenticatedView);
    return m_realRegion->query(predicate, timeout);
  }

  bool existsValue(const std::string& predicate,
                   std::chrono::milliseconds timeout =
                       DEFAULT_QUERY_RESPONSE_TIMEOUT) final {
    GuardUserAttributes gua(m_authenticatedView);
    return m_realRegion->existsValue(predicate, timeout);
  }

  std::shared_ptr<Serializable> selectValue(
      const std::string& predicate, std::chrono::milliseconds timeout =
                                        DEFAULT_QUERY_RESPONSE_TIMEOUT) final {
    GuardUserAttributes gua(m_authenticatedView);
    return m_realRegion->selectValue(predicate, timeout);
  }

  void removeAll(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) final {
    GuardUserAttributes gua(m_authenticatedView);
    m_realRegion->removeAll(keys, aCallbackArgument);
  }

  uint32_t size() final { return m_realRegion->size(); }

  const std::shared_ptr<Pool>& getPool() const final {
    return m_realRegion->getPool();
  }

  ProxyRegion(AuthenticatedView& authenticatedView,
              const std::shared_ptr<RegionInternal>& realRegion)
      : Region(authenticatedView.m_cacheImpl) {
    m_authenticatedView = &authenticatedView;
    m_realRegion = realRegion;
  }

  ~ProxyRegion() final = default;

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
