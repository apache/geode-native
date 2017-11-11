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
#include <geode/geode_types.hpp>
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

  virtual RegionPtr getParentRegion() const override {
    return m_realRegion->getParentRegion();
  }

  virtual RegionAttributesPtr getAttributes() const override {
    return m_realRegion->getAttributes();
  }

  virtual AttributesMutatorPtr getAttributesMutator() const override {
    throw UnsupportedOperationException("Region.getAttributesMutator()");
  }

  virtual CacheStatisticsPtr getStatistics() const override {
    return m_realRegion->getStatistics();
  }

  virtual void invalidateRegion(
      const SerializablePtr& aCallbackArgument = nullptr) override {
    throw UnsupportedOperationException("Region.invalidateRegion()");
  }

  virtual void localInvalidateRegion(
      const SerializablePtr& aCallbackArgument = nullptr) override {
    throw UnsupportedOperationException("Region.localInvalidateRegion()");
  }

  virtual void destroyRegion(
      const SerializablePtr& aCallbackArgument = nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    m_realRegion->destroyRegion(aCallbackArgument);
  }

  virtual void clear(
      const SerializablePtr& aCallbackArgument = nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    m_realRegion->clear(aCallbackArgument);
  }

  virtual void localClear(
      const SerializablePtr& aCallbackArgument = nullptr) override {
    throw UnsupportedOperationException("localClear()");
  }

  virtual void localDestroyRegion(
      const SerializablePtr& aCallbackArgument = nullptr) override {
    throw UnsupportedOperationException("Region.localDestroyRegion()");
  }

  virtual RegionPtr getSubregion(const char* path) override {
    LOGDEBUG("ProxyRegion getSubregion");
    auto rPtr = std::static_pointer_cast<RegionInternal>(
        m_realRegion->getSubregion(path));

    if (rPtr == nullptr) return rPtr;

    return std::make_shared<ProxyRegion>(m_proxyCache, rPtr);
  }

  virtual RegionPtr createSubregion(
      const char* subregionName,
      const RegionAttributesPtr& aRegionAttributes) override {
    throw UnsupportedOperationException("createSubregion()");
    return nullptr;
  }

  VectorOfRegion subregions(const bool recursive) override {
    VectorOfRegion realVectorRegion = m_realRegion->subregions(recursive);
    VectorOfRegion proxyRegions(realVectorRegion.size());

    std::transform(
        realVectorRegion.begin(), realVectorRegion.end(),
        std::back_inserter(proxyRegions),
        [this](const RegionPtr& realRegion) -> std::shared_ptr<ProxyRegion> {
          return std::make_shared<ProxyRegion>(
              m_proxyCache,
              std::static_pointer_cast<RegionInternal>(realRegion));
        });

    return proxyRegions;
  }

  virtual RegionEntryPtr getEntry(const CacheableKeyPtr& key) override {
    return m_realRegion->getEntry(key);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline RegionEntryPtr getEntry(const KEYTYPE& key) {
    return getEntry(createKey(key));
  }

  virtual CacheablePtr get(
      const CacheableKeyPtr& key,
      const SerializablePtr& aCallbackArgument = nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    return m_realRegion->get(key, aCallbackArgument);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline CacheablePtr get(const KEYTYPE& key,
                          const SerializablePtr& callbackArg = nullptr) {
    return get(createKey(key), callbackArg);
  }

  virtual void put(
      const CacheableKeyPtr& key, const CacheablePtr& value,
      const SerializablePtr& aCallbackArgument = nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    return m_realRegion->put(key, value, aCallbackArgument);
  }

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline void put(const KEYTYPE& key, const VALUETYPE& value,
                  const SerializablePtr& arg = nullptr) {
    put(createKey(key), createValue(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void put(const KEYTYPE& key, const CacheablePtr& value,
                  const SerializablePtr& arg = nullptr) {
    put(createKey(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline void put(const CacheableKeyPtr& key, const VALUETYPE& value,
                  const SerializablePtr& arg = nullptr) {
    put(key, createValue(value), arg);
  }

  virtual void putAll(
      const HashMapOfCacheable& map,
      std::chrono::milliseconds timeout = DEFAULT_RESPONSE_TIMEOUT,
      const SerializablePtr& aCallbackArgument = nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    return m_realRegion->putAll(map, timeout, aCallbackArgument);
  }

  virtual void localPut(const CacheableKeyPtr& key, const CacheablePtr& value,
                        const SerializablePtr& aCallbackArgument = nullptr) override {
    throw UnsupportedOperationException("Region.localPut()");
  }

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline void localPut(const KEYTYPE& key, const VALUETYPE& value,
                       const SerializablePtr& arg = nullptr) {
    localPut(createKey(key), createValue(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void localPut(const KEYTYPE& key, const CacheablePtr& value,
                       const SerializablePtr& arg = nullptr) {
    localPut(createKey(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline void localPut(const CacheableKeyPtr& key, const VALUETYPE& value,
                       const SerializablePtr& arg = nullptr) {
    localPut(key, createValue(value), arg);
  }

  virtual void create(
      const CacheableKeyPtr& key, const CacheablePtr& value,
      const SerializablePtr& aCallbackArgument = nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    m_realRegion->create(key, value, aCallbackArgument);
  }

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline void create(const KEYTYPE& key, const VALUETYPE& value,
                     const SerializablePtr& arg = nullptr) {
    create(createKey(key), createValue(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void create(const KEYTYPE& key, const CacheablePtr& value,
                     const SerializablePtr& arg = nullptr) {
    create(createKey(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline void create(const CacheableKeyPtr& key, const VALUETYPE& value,
                     const SerializablePtr& arg = nullptr) {
    create(key, createValue(value), arg);
  }

  virtual void localCreate(const CacheableKeyPtr& key,
                           const CacheablePtr& value,
                           const SerializablePtr& aCallbackArgument = nullptr) override {
    throw UnsupportedOperationException("Region.localCreate()");
  }

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline void localCreate(const KEYTYPE& key, const VALUETYPE& value,
                          const SerializablePtr& arg = nullptr) {
    localCreate(createKey(key), createValue(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void localCreate(const KEYTYPE& key, const CacheablePtr& value,
                          const SerializablePtr& arg = nullptr) {
    localCreate(createKey(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline void localCreate(const CacheableKeyPtr& key, const VALUETYPE& value,
                          const SerializablePtr& arg = nullptr) {
    localCreate(key, createValue(value), arg);
  }

  virtual void invalidate(
      const CacheableKeyPtr& key,
      const SerializablePtr& aCallbackArgument = nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    m_realRegion->invalidate(key, aCallbackArgument);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void invalidate(const KEYTYPE& key, const SerializablePtr& arg = nullptr) {
    invalidate(createKey(key), arg);
  }

  virtual void localInvalidate(const CacheableKeyPtr& key,
                               const SerializablePtr& aCallbackArgument = nullptr) override {
    throw UnsupportedOperationException("Region.localInvalidate()");
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void localInvalidate(const KEYTYPE& key,
                              const SerializablePtr& arg = nullptr) {
    localInvalidate(createKey(key), arg);
  }

  virtual void destroy(
      const CacheableKeyPtr& key,
      const SerializablePtr& aCallbackArgument = nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    m_realRegion->destroy(key, aCallbackArgument);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void destroy(const KEYTYPE& key, const SerializablePtr& arg = nullptr) {
    destroy(createKey(key), arg);
  }

  virtual void localDestroy(
      const CacheableKeyPtr& key,
      const SerializablePtr& aCallbackArgument = nullptr) override {
    throw UnsupportedOperationException("Region.localDestroy()");
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void localDestroy(const KEYTYPE& key,
                           const SerializablePtr& arg = nullptr) {
    localDestroy(createKey(key), arg);
  }

  virtual bool remove(
      const CacheableKeyPtr& key, const CacheablePtr& value,
      const SerializablePtr& aCallbackArgument = nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    return m_realRegion->remove(key, value, aCallbackArgument);
  }

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline bool remove(const KEYTYPE& key, const VALUETYPE& value,
                     const SerializablePtr& arg = nullptr) {
    return remove(createKey(key), createValue(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline bool remove(const KEYTYPE& key, const CacheablePtr& value,
                     const SerializablePtr& arg = nullptr) {
    return remove(createKey(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline bool remove(const CacheableKeyPtr& key, const VALUETYPE& value,
                     const SerializablePtr& arg = nullptr) {
    return remove(key, createValue(value), arg);
  }

  virtual bool removeEx(
      const CacheableKeyPtr& key,
      const SerializablePtr& aCallbackArgument = nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    return m_realRegion->removeEx(key, aCallbackArgument);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline bool removeEx(const KEYTYPE& key, const SerializablePtr& arg = nullptr) {
    return removeEx(createKey(key), arg);
  }

  virtual bool localRemove(const CacheableKeyPtr& key,
                           const CacheablePtr& value,
                           const SerializablePtr& aCallbackArgument = nullptr) override {
    throw UnsupportedOperationException("Region.localRemove()");
    return false;
  }

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline bool localRemove(const KEYTYPE& key, const VALUETYPE& value,
                          const SerializablePtr& arg = nullptr) {
    return localRemove(createKey(key), createValue(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline bool localRemove(const KEYTYPE& key, const CacheablePtr& value,
                          const SerializablePtr& arg = nullptr) {
    return localRemove(createKey(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline bool localRemove(const CacheableKeyPtr& key, const VALUETYPE& value,
                          const SerializablePtr& arg = nullptr) {
    return localRemove(key, createValue(value), arg);
  }

  virtual bool localRemoveEx(const CacheableKeyPtr& key,
                             const SerializablePtr& aCallbackArgument = nullptr) override {
    throw UnsupportedOperationException("Region.localRemoveEx()");
    return false;
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline bool localRemoveEx(const KEYTYPE& key,
                            const SerializablePtr& arg = nullptr) {
    return localRemoveEx(createKey(key), arg);
  }

  /**
   * Return all the keys in the local process for this region. This includes
   * keys for which the entry is invalid.
   */
  virtual VectorOfCacheableKey keys() override {
    throw UnsupportedOperationException("Region.keys()");
    return VectorOfCacheableKey();
  }

  virtual VectorOfCacheableKey serverKeys() override {
    GuardUserAttribures gua(m_proxyCache);
    return m_realRegion->serverKeys();
  }

  virtual VectorOfCacheable values() override {
    throw UnsupportedOperationException("Region.values()");
  }

  virtual VectorOfRegionEntry entries(bool recursive) override {
    throw UnsupportedOperationException("Region.entries()");
  }

  virtual RegionServicePtr getRegionService() const override {
    return RegionServicePtr(m_proxyCache);
  }

  virtual bool isDestroyed() const override {
    return m_realRegion->isDestroyed();
  }

  virtual bool containsValueForKey(
      const CacheableKeyPtr& keyPtr) const override {
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

  virtual bool containsKey(const CacheableKeyPtr& keyPtr) const override {
    throw UnsupportedOperationException("Region.containsKey()");
    return false;
  }

  virtual bool containsKeyOnServer(
      const CacheableKeyPtr& keyPtr) const override {
    GuardUserAttribures gua(m_proxyCache);
    return m_realRegion->containsKeyOnServer(keyPtr);
  }

  virtual VectorOfCacheableKey getInterestList() const override {
    throw UnsupportedOperationException("Region.getInterestList()");
  }

  virtual VectorOfCacheableString getInterestListRegex() const override {
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

  virtual void registerKeys(const VectorOfCacheableKey& keys,
                            bool isDurable = false,
                            bool getInitialValues = false,
                            bool receiveValues = true) override {
    throw UnsupportedOperationException("Region.registerKeys()");
  }

  virtual void unregisterKeys(const VectorOfCacheableKey& keys) override {
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
      const VectorOfCacheableKey& keys,
      const SerializablePtr& aCallbackArgument = nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    return m_realRegion->getAll_internal(keys, aCallbackArgument, false);
  }

  virtual SelectResultsPtr query(const char* predicate,
                                 std::chrono::milliseconds timeout =
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

  virtual SerializablePtr selectValue(
      const char* predicate,
      std::chrono::milliseconds timeout =
          DEFAULT_QUERY_RESPONSE_TIMEOUT) override {
    GuardUserAttribures gua(m_proxyCache);
    return m_realRegion->selectValue(predicate, timeout);
  }

  virtual void removeAll(
      const VectorOfCacheableKey& keys,
      const SerializablePtr& aCallbackArgument = nullptr) override {
    GuardUserAttribures gua(m_proxyCache);
    m_realRegion->removeAll(keys, aCallbackArgument);
  }

  virtual uint32_t size() override { return m_realRegion->size(); }

  virtual const PoolPtr& getPool() override { return m_realRegion->getPool(); }

  ProxyRegion(const ProxyCachePtr& proxyCache,
              const std::shared_ptr<RegionInternal>& realRegion)
      : Region(realRegion->getCache()) {
    m_proxyCache = proxyCache;
    m_realRegion = realRegion;
  }

  virtual ~ProxyRegion() {}

  ProxyRegion(const ProxyRegion&) = delete;
  ProxyRegion& operator=(const ProxyRegion&) = delete;

 private:

  ProxyCachePtr m_proxyCache;
  std::shared_ptr<RegionInternal> m_realRegion;
  friend class FunctionService;

  FRIEND_STD_SHARED_PTR(ProxyRegion)
};

typedef std::shared_ptr<ProxyRegion> ProxyRegionPtr;

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PROXYREGION_H_
