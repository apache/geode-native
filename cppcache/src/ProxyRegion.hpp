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

#include <geode/geode_globals.hpp>
#include <geode/CacheableKey.hpp>
#include <geode/CacheableString.hpp>
#include <geode/CacheStatistics.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/CacheableString.hpp>
#include <geode/CacheableBuiltins.hpp>
#include <geode/RegionEntry.hpp>
#include <geode/CacheListener.hpp>
#include <geode/CacheWriter.hpp>
#include <geode/CacheLoader.hpp>
#include <geode/RegionAttributes.hpp>
#include <geode/AttributesMutator.hpp>
#include <geode/AttributesFactory.hpp>
#include <geode/CacheableKey.hpp>
#include <geode/Query.hpp>

#include "RegionInternal.hpp"
#include "ProxyCache.hpp"

namespace apache {
namespace geode {
namespace client {

class FunctionService;

/**
 * @class ProxyRegion ProxyRegion.hpp
 * This class wrapper around real region
 */
class CPPCACHE_EXPORT ProxyRegion : public Region {
 public:
  virtual const char* getName() const override { return m_realRegion->getName(); }

  virtual const char* getFullPath() const override {
    return m_realRegion->getFullPath();
  }

  virtual std::shared_ptr<Region> getParentRegion() const override {
    return m_realRegion->getParentRegion();
  }

  virtual std::shared_ptr<RegionAttributes> getAttributes() const override {
    return m_realRegion->getAttributes();
  }

  virtual std::shared_ptr<AttributesMutator> getAttributesMutator()
      const override {
    throw UnsupportedOperationException("Region.getAttributesMutator()");
  }

  virtual std::shared_ptr<CacheStatistics> getStatistics() const override {
    return m_realRegion->getStatistics();
  }

  virtual void invalidateRegion(const std::shared_ptr<Serializable>&
                                    aCallbackArgument = nullptr) override {
    throw UnsupportedOperationException("Region.invalidateRegion()");
  }

  virtual void localInvalidateRegion(const std::shared_ptr<Serializable>&
                                         aCallbackArgument = nullptr) override {
    throw UnsupportedOperationException("Region.localInvalidateRegion()");
  }

  virtual void destroyRegion(const std::shared_ptr<Serializable>&
                                 aCallbackArgument = nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    m_realRegion->destroyRegion(aCallbackArgument);
  }

  virtual void clear(const std::shared_ptr<Serializable>& aCallbackArgument =
                         nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    m_realRegion->clear(aCallbackArgument);
  }

  virtual void localClear(const std::shared_ptr<Serializable>&
                              aCallbackArgument = nullptr) override {
    throw UnsupportedOperationException("localClear()");
  }

  virtual void localDestroyRegion(const std::shared_ptr<Serializable>&
                                      aCallbackArgument = nullptr) override {
    throw UnsupportedOperationException("Region.localDestroyRegion()");
  }

  virtual std::shared_ptr<Region> getSubregion(const char* path) override {
    LOGDEBUG("ProxyRegion getSubregion");
    auto rPtr = std::static_pointer_cast<RegionInternal>(
        m_realRegion->getSubregion(path));

    if (rPtr == nullptr) return rPtr;

    return std::make_shared<ProxyRegion>(m_proxyCache, rPtr);
  }

  virtual std::shared_ptr<Region> createSubregion(
      const char* subregionName,
      const std::shared_ptr<RegionAttributes>& aRegionAttributes) override {
    throw UnsupportedOperationException("createSubregion()");
    return nullptr;
  }

  std::vector<std::shared_ptr<Region>> subregions(
      const bool recursive) override {
    std::vector<std::shared_ptr<Region>> realVectorRegion =
        m_realRegion->subregions(recursive);
    std::vector<std::shared_ptr<Region>> proxyRegions(realVectorRegion.size());

    std::transform(
        realVectorRegion.begin(), realVectorRegion.end(),
        std::back_inserter(proxyRegions),
        [this](const std::shared_ptr<Region>& realRegion)
            -> std::shared_ptr<ProxyRegion> {
          return std::make_shared<ProxyRegion>(
              m_proxyCache,
              std::static_pointer_cast<RegionInternal>(realRegion));
        });

    return proxyRegions;
  }

  virtual std::shared_ptr<RegionEntry> getEntry(
      const std::shared_ptr<CacheableKey>& key) override {
    return m_realRegion->getEntry(key);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline std::shared_ptr<RegionEntry> getEntry(const KEYTYPE& key) {
    return getEntry(createKey(key));
  }

  virtual std::shared_ptr<Cacheable> get(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument =
          nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    return m_realRegion->get(key, aCallbackArgument);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline std::shared_ptr<Cacheable> get(
      const KEYTYPE& key,
      const std::shared_ptr<Serializable>& callbackArg = nullptr) {
    return get(createKey(key), callbackArg);
  }

  virtual void put(const std::shared_ptr<CacheableKey>& key,
                   const std::shared_ptr<Cacheable>& value,
                   const std::shared_ptr<Serializable>& aCallbackArgument =
                       nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    return m_realRegion->put(key, value, aCallbackArgument);
  }

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline void put(const KEYTYPE& key, const VALUETYPE& value,
                  const std::shared_ptr<Serializable>& arg = nullptr) {
    put(createKey(key), createValue(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void put(const KEYTYPE& key, const std::shared_ptr<Cacheable>& value,
                  const std::shared_ptr<Serializable>& arg = nullptr) {
    put(createKey(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline void put(const std::shared_ptr<CacheableKey>& key,
                  const VALUETYPE& value,
                  const std::shared_ptr<Serializable>& arg = nullptr) {
    put(key, createValue(value), arg);
  }

  virtual void putAll(
      const HashMapOfCacheable& map,
      std::chrono::milliseconds timeout = DEFAULT_RESPONSE_TIMEOUT,
      const std::shared_ptr<Serializable>& aCallbackArgument =
          nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    return m_realRegion->putAll(map, timeout, aCallbackArgument);
  }

  virtual void localPut(const std::shared_ptr<CacheableKey>& key,
                        const std::shared_ptr<Cacheable>& value,
                        const std::shared_ptr<Serializable>& aCallbackArgument =
                            nullptr) override {
    throw UnsupportedOperationException("Region.localPut()");
  }

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline void localPut(const KEYTYPE& key, const VALUETYPE& value,
                       const std::shared_ptr<Serializable>& arg = nullptr) {
    localPut(createKey(key), createValue(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void localPut(const KEYTYPE& key,
                       const std::shared_ptr<Cacheable>& value,
                       const std::shared_ptr<Serializable>& arg = nullptr) {
    localPut(createKey(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline void localPut(const std::shared_ptr<CacheableKey>& key,
                       const VALUETYPE& value,
                       const std::shared_ptr<Serializable>& arg = nullptr) {
    localPut(key, createValue(value), arg);
  }

  virtual void create(const std::shared_ptr<CacheableKey>& key,
                      const std::shared_ptr<Cacheable>& value,
                      const std::shared_ptr<Serializable>& aCallbackArgument =
                          nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    m_realRegion->create(key, value, aCallbackArgument);
  }

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline void create(const KEYTYPE& key, const VALUETYPE& value,
                     const std::shared_ptr<Serializable>& arg = nullptr) {
    create(createKey(key), createValue(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void create(const KEYTYPE& key,
                     const std::shared_ptr<Cacheable>& value,
                     const std::shared_ptr<Serializable>& arg = nullptr) {
    create(createKey(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline void create(const std::shared_ptr<CacheableKey>& key,
                     const VALUETYPE& value,
                     const std::shared_ptr<Serializable>& arg = nullptr) {
    create(key, createValue(value), arg);
  }

  virtual void localCreate(const std::shared_ptr<CacheableKey>& key,
                           const std::shared_ptr<Cacheable>& value,
                           const std::shared_ptr<Serializable>&
                               aCallbackArgument = nullptr) override {
    throw UnsupportedOperationException("Region.localCreate()");
  }

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline void localCreate(const KEYTYPE& key, const VALUETYPE& value,
                          const std::shared_ptr<Serializable>& arg = nullptr) {
    localCreate(createKey(key), createValue(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void localCreate(const KEYTYPE& key,
                          const std::shared_ptr<Cacheable>& value,
                          const std::shared_ptr<Serializable>& arg = nullptr) {
    localCreate(createKey(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline void localCreate(const std::shared_ptr<CacheableKey>& key,
                          const VALUETYPE& value,
                          const std::shared_ptr<Serializable>& arg = nullptr) {
    localCreate(key, createValue(value), arg);
  }

  virtual void invalidate(const std::shared_ptr<CacheableKey>& key,
                          const std::shared_ptr<Serializable>&
                              aCallbackArgument = nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    m_realRegion->invalidate(key, aCallbackArgument);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void invalidate(const KEYTYPE& key,
                         const std::shared_ptr<Serializable>& arg = nullptr) {
    invalidate(createKey(key), arg);
  }

  virtual void localInvalidate(const std::shared_ptr<CacheableKey>& key,
                               const std::shared_ptr<Serializable>&
                                   aCallbackArgument = nullptr) override {
    throw UnsupportedOperationException("Region.localInvalidate()");
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void localInvalidate(
      const KEYTYPE& key, const std::shared_ptr<Serializable>& arg = nullptr) {
    localInvalidate(createKey(key), arg);
  }

  virtual void destroy(const std::shared_ptr<CacheableKey>& key,
                       const std::shared_ptr<Serializable>& aCallbackArgument =
                           nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    m_realRegion->destroy(key, aCallbackArgument);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void destroy(const KEYTYPE& key,
                      const std::shared_ptr<Serializable>& arg = nullptr) {
    destroy(createKey(key), arg);
  }

  virtual void localDestroy(const std::shared_ptr<CacheableKey>& key,
                            const std::shared_ptr<Serializable>&
                                aCallbackArgument = nullptr) override {
    throw UnsupportedOperationException("Region.localDestroy()");
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void localDestroy(const KEYTYPE& key,
                           const std::shared_ptr<Serializable>& arg = nullptr) {
    localDestroy(createKey(key), arg);
  }

  virtual bool remove(const std::shared_ptr<CacheableKey>& key,
                      const std::shared_ptr<Cacheable>& value,
                      const std::shared_ptr<Serializable>& aCallbackArgument =
                          nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    return m_realRegion->remove(key, value, aCallbackArgument);
  }

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline bool remove(const KEYTYPE& key, const VALUETYPE& value,
                     const std::shared_ptr<Serializable>& arg = nullptr) {
    return remove(createKey(key), createValue(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline bool remove(const KEYTYPE& key,
                     const std::shared_ptr<Cacheable>& value,
                     const std::shared_ptr<Serializable>& arg = nullptr) {
    return remove(createKey(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline bool remove(const std::shared_ptr<CacheableKey>& key,
                     const VALUETYPE& value,
                     const std::shared_ptr<Serializable>& arg = nullptr) {
    return remove(key, createValue(value), arg);
  }

  virtual bool removeEx(const std::shared_ptr<CacheableKey>& key,
                        const std::shared_ptr<Serializable>& aCallbackArgument =
                            nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    return m_realRegion->removeEx(key, aCallbackArgument);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline bool removeEx(const KEYTYPE& key,
                       const std::shared_ptr<Serializable>& arg = nullptr) {
    return removeEx(createKey(key), arg);
  }

  virtual bool localRemove(const std::shared_ptr<CacheableKey>& key,
                           const std::shared_ptr<Cacheable>& value,
                           const std::shared_ptr<Serializable>&
                               aCallbackArgument = nullptr) override {
    throw UnsupportedOperationException("Region.localRemove()");
    return false;
  }

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline bool localRemove(const KEYTYPE& key, const VALUETYPE& value,
                          const std::shared_ptr<Serializable>& arg = nullptr) {
    return localRemove(createKey(key), createValue(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline bool localRemove(const KEYTYPE& key,
                          const std::shared_ptr<Cacheable>& value,
                          const std::shared_ptr<Serializable>& arg = nullptr) {
    return localRemove(createKey(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline bool localRemove(const std::shared_ptr<CacheableKey>& key,
                          const VALUETYPE& value,
                          const std::shared_ptr<Serializable>& arg = nullptr) {
    return localRemove(key, createValue(value), arg);
  }

  virtual bool localRemoveEx(const std::shared_ptr<CacheableKey>& key,
                             const std::shared_ptr<Serializable>&
                                 aCallbackArgument = nullptr) override {
    throw UnsupportedOperationException("Region.localRemoveEx()");
    return false;
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline bool localRemoveEx(
      const KEYTYPE& key, const std::shared_ptr<Serializable>& arg = nullptr) {
    return localRemoveEx(createKey(key), arg);
  }

  /**
   * Return all the keys in the local process for this region. This includes
   * keys for which the entry is invalid.
   */
  virtual std::vector<std::shared_ptr<CacheableKey>> keys() override {
    throw UnsupportedOperationException("Region.keys()");
    return std::vector<std::shared_ptr<CacheableKey>>();
  }

  virtual std::vector<std::shared_ptr<CacheableKey>> serverKeys() override {
    GuardUserAttribures gua(m_proxyCache);
    return m_realRegion->serverKeys();
  }

  virtual std::vector<std::shared_ptr<Cacheable>> values() override {
    throw UnsupportedOperationException("Region.values()");
  }

  virtual std::vector<std::shared_ptr<RegionEntry>> entries(
      bool recursive) override {
    throw UnsupportedOperationException("Region.entries()");
  }

  virtual std::shared_ptr<RegionService> getRegionService() const override {
    return std::shared_ptr<RegionService>(m_proxyCache);
  }

  virtual bool isDestroyed() const override {
    return m_realRegion->isDestroyed();
  }

  virtual bool containsValueForKey(
      const std::shared_ptr<CacheableKey>& keyPtr) const override {
    throw UnsupportedOperationException("Region.containsValueForKey()");
    return false;
  }

  /**
   * Convenience method allowing key to be a const char*
   * This operations checks for the value in the local cache .
   * It is not propagated to the Geode cache server
   * to which it is connected.
   */
  template <class KEYTYPE>
  inline bool containsValueForKey(const KEYTYPE& key) const {
    return containsValueForKey(createKey(key));
  }

  virtual bool containsKey(
      const std::shared_ptr<CacheableKey>& keyPtr) const override {
    throw UnsupportedOperationException("Region.containsKey()");
    return false;
  }

  virtual bool containsKeyOnServer(
      const std::shared_ptr<CacheableKey>& keyPtr) const override {
    GuardUserAttribures gua(m_proxyCache);
    return m_realRegion->containsKeyOnServer(keyPtr);
  }

  virtual std::vector<std::shared_ptr<CacheableKey>> getInterestList()
      const override {
    throw UnsupportedOperationException("Region.getInterestList()");
  }

  virtual std::vector<std::shared_ptr<CacheableString>> getInterestListRegex()
      const override {
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
    return containsKey(createKey(key));
  }

  virtual void registerKeys(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      bool isDurable = false, bool getInitialValues = false,
      bool receiveValues = true) override {
    throw UnsupportedOperationException("Region.registerKeys()");
  }

  virtual void unregisterKeys(
      const std::vector<std::shared_ptr<CacheableKey>>& keys) override {
    throw UnsupportedOperationException("Region.unregisterKeys()");
  }

  virtual void registerAllKeys(bool isDurable = false,
                               bool getInitialValues = false,
                               bool receiveValues = true) override {
    throw UnsupportedOperationException("Region.registerAllKeys()");
  }

  virtual void unregisterAllKeys() override {
    throw UnsupportedOperationException("Region.unregisterAllKeys()");
  }

  virtual void registerRegex(const char* regex, bool isDurable = false,
                             bool getInitialValues = false,
                             bool receiveValues = true) override {
    throw UnsupportedOperationException("Region.registerRegex()");
  }

  virtual void unregisterRegex(const char* regex) override {
    throw UnsupportedOperationException("Region.unregisterRegex()");
  }

  virtual HashMapOfCacheable getAll(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      const std::shared_ptr<Serializable>& aCallbackArgument =
          nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    return m_realRegion->getAll_internal(keys, aCallbackArgument, false);
  }

  virtual std::shared_ptr<SelectResults> query(
      const char* predicate, std::chrono::milliseconds timeout =
                                 DEFAULT_QUERY_RESPONSE_TIMEOUT) override {
    GuardUserAttribures gua(m_proxyCache);
    return m_realRegion->query(predicate, timeout);
  }

  virtual bool existsValue(const char* predicate,
                           std::chrono::milliseconds timeout =
                               DEFAULT_QUERY_RESPONSE_TIMEOUT) override {
    GuardUserAttribures gua(m_proxyCache);
    return m_realRegion->existsValue(predicate, timeout);
  }

  virtual std::shared_ptr<Serializable> selectValue(
      const char* predicate, std::chrono::milliseconds timeout =
                                 DEFAULT_QUERY_RESPONSE_TIMEOUT) override {
    GuardUserAttribures gua(m_proxyCache);
    return m_realRegion->selectValue(predicate, timeout);
  }

  virtual void removeAll(const std::vector<std::shared_ptr<CacheableKey>>& keys,
                         const std::shared_ptr<Serializable>&
                             aCallbackArgument = nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    m_realRegion->removeAll(keys, aCallbackArgument);
  }

  virtual uint32_t size() override { return m_realRegion->size(); }

  virtual const std::shared_ptr<Pool>& getPool() override {
    return m_realRegion->getPool();
  }

  ProxyRegion(const std::shared_ptr<ProxyCache>& proxyCache,
              const std::shared_ptr<RegionInternal>& realRegion)
      : Region(&realRegion->getCache()) {
    m_proxyCache = proxyCache;
    m_realRegion = realRegion;
  }

  virtual ~ProxyRegion() {}

  ProxyRegion(const ProxyRegion&) = delete;
  ProxyRegion& operator=(const ProxyRegion&) = delete;

 private:
  std::shared_ptr<ProxyCache> m_proxyCache;
  std::shared_ptr<RegionInternal> m_realRegion;
  friend class FunctionService;

  FRIEND_STD_SHARED_PTR(ProxyRegion)
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PROXYREGION_H_
