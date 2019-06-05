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

#include "ProxyRegion.hpp"

namespace apache {
namespace geode {
namespace client {

const std::string& ProxyRegion::getName() const {
  return m_realRegion->getName();
}

const std::string& ProxyRegion::getFullPath() const {
  return m_realRegion->getFullPath();
}

std::shared_ptr<Region> ProxyRegion::getParentRegion() const {
  return m_realRegion->getParentRegion();
}

const RegionAttributes& ProxyRegion::getAttributes() const {
  return m_realRegion->getAttributes();
}

std::shared_ptr<AttributesMutator> ProxyRegion::getAttributesMutator() const {
  throw UnsupportedOperationException("Region.getAttributesMutator()");
}

std::shared_ptr<CacheStatistics> ProxyRegion::getStatistics() const {
  return m_realRegion->getStatistics();
}

void ProxyRegion::invalidateRegion(const std::shared_ptr<Serializable>&) {
  throw UnsupportedOperationException("Region.invalidateRegion()");
}

void ProxyRegion::localInvalidateRegion(const std::shared_ptr<Serializable>&) {
  throw UnsupportedOperationException("Region.localInvalidateRegion()");
}

void ProxyRegion::destroyRegion(
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  GuardUserAttributes gua(m_authenticatedView);
  m_realRegion->destroyRegion(aCallbackArgument);
}

void ProxyRegion::clear(
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  GuardUserAttributes gua(m_authenticatedView);
  m_realRegion->clear(aCallbackArgument);
}

void ProxyRegion::localClear(const std::shared_ptr<Serializable>&) {
  throw UnsupportedOperationException("localClear()");
}

void ProxyRegion::localDestroyRegion(const std::shared_ptr<Serializable>&) {
  throw UnsupportedOperationException("Region.localDestroyRegion()");
}

std::shared_ptr<Region> ProxyRegion::getSubregion(const std::string& path) {
  LOGDEBUG("ProxyRegion getSubregion");
  auto rPtr = std::static_pointer_cast<RegionInternal>(
      m_realRegion->getSubregion(path));

  if (rPtr == nullptr) {
    return std::move(rPtr);
  }

  return std::make_shared<ProxyRegion>(*m_authenticatedView, rPtr);
}

std::shared_ptr<Region> ProxyRegion::createSubregion(const std::string&,
                                                     RegionAttributes) {
  throw UnsupportedOperationException("createSubregion()");
}

std::vector<std::shared_ptr<Region>> ProxyRegion::subregions(
    const bool recursive) {
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

std::shared_ptr<RegionEntry> ProxyRegion::getEntry(
    const std::shared_ptr<CacheableKey>& key) {
  return m_realRegion->getEntry(key);
}

std::shared_ptr<Cacheable> ProxyRegion::get(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  GuardUserAttributes gua(m_authenticatedView);
  return m_realRegion->get(key, aCallbackArgument);
}

void ProxyRegion::put(const std::shared_ptr<CacheableKey>& key,
         const std::shared_ptr<Cacheable>& value,
         const std::shared_ptr<Serializable>& aCallbackArgument) {
  GuardUserAttributes gua(m_authenticatedView);
  return m_realRegion->put(key, value, aCallbackArgument);
}

void ProxyRegion::putAll(const HashMapOfCacheable& map,
            std::chrono::milliseconds timeout,
            const std::shared_ptr<Serializable>& aCallbackArgument) {
  GuardUserAttributes gua(m_authenticatedView);
  return m_realRegion->putAll(map, timeout, aCallbackArgument);
}

void ProxyRegion::localPut(const std::shared_ptr<CacheableKey>&,
              const std::shared_ptr<Cacheable>&,
              const std::shared_ptr<Serializable>&) {
  throw UnsupportedOperationException("Region.localPut()");
}

void ProxyRegion::create(const std::shared_ptr<CacheableKey>& key,
            const std::shared_ptr<Cacheable>& value,
            const std::shared_ptr<Serializable>& aCallbackArgument) {
  GuardUserAttributes gua(m_authenticatedView);
  m_realRegion->create(key, value, aCallbackArgument);
}

void ProxyRegion::localCreate(const std::shared_ptr<CacheableKey>&,
                 const std::shared_ptr<Cacheable>&,
                 const std::shared_ptr<Serializable>&) {
  throw UnsupportedOperationException("Region.localCreate()");
}

void ProxyRegion::invalidate(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  GuardUserAttributes gua(m_authenticatedView);
  m_realRegion->invalidate(key, aCallbackArgument);
}

void ProxyRegion::localInvalidate(const std::shared_ptr<CacheableKey>&,
                     const std::shared_ptr<Serializable>&) {
  throw UnsupportedOperationException("Region.localInvalidate()");
}

void ProxyRegion::destroy(const std::shared_ptr<CacheableKey>& key,
             const std::shared_ptr<Serializable>& aCallbackArgument) {
  GuardUserAttributes gua(m_authenticatedView);
  m_realRegion->destroy(key, aCallbackArgument);
}

void ProxyRegion::localDestroy(const std::shared_ptr<CacheableKey>&,
                  const std::shared_ptr<Serializable>&) {
  throw UnsupportedOperationException("Region.localDestroy()");
}

bool ProxyRegion::remove(const std::shared_ptr<CacheableKey>& key,
            const std::shared_ptr<Cacheable>& value,
            const std::shared_ptr<Serializable>& aCallbackArgument) {
  GuardUserAttributes gua(m_authenticatedView);
  return m_realRegion->remove(key, value, aCallbackArgument);
}

bool ProxyRegion::removeEx(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  GuardUserAttributes gua(m_authenticatedView);
  return m_realRegion->removeEx(key, aCallbackArgument);
}

bool ProxyRegion::localRemove(const std::shared_ptr<CacheableKey>&,
                 const std::shared_ptr<Cacheable>&,
                 const std::shared_ptr<Serializable>&) {
  throw UnsupportedOperationException("Region.localRemove()");
}

bool ProxyRegion::localRemoveEx(const std::shared_ptr<CacheableKey>&,
                   const std::shared_ptr<Serializable>&) {
  throw UnsupportedOperationException("Region.localRemoveEx()");
}

std::vector<std::shared_ptr<CacheableKey>> ProxyRegion::keys() {
  throw UnsupportedOperationException("Region.keys()");
}

std::vector<std::shared_ptr<CacheableKey>> ProxyRegion::serverKeys() {
  GuardUserAttributes gua(m_authenticatedView);
  return m_realRegion->serverKeys();
}

std::vector<std::shared_ptr<Cacheable>> ProxyRegion::values() {
  throw UnsupportedOperationException("Region.values()");
}

std::vector<std::shared_ptr<RegionEntry>> ProxyRegion::entries(bool) {
  throw UnsupportedOperationException("Region.entries()");
}

RegionService& ProxyRegion::getRegionService() const { return *m_authenticatedView; }

bool ProxyRegion::isDestroyed() const { return m_realRegion->isDestroyed(); }

bool ProxyRegion::containsValueForKey(const std::shared_ptr<CacheableKey>&) const {
  throw UnsupportedOperationException("Region.containsValueForKey()");
}

bool ProxyRegion::containsKey(const std::shared_ptr<CacheableKey>&) const {
  throw UnsupportedOperationException("Region.containsKey()");
}

bool ProxyRegion::containsKeyOnServer(const std::shared_ptr<CacheableKey>& keyPtr) const {
  GuardUserAttributes gua(m_authenticatedView);
  return m_realRegion->containsKeyOnServer(keyPtr);
}

std::vector<std::shared_ptr<CacheableKey>> ProxyRegion::getInterestList() const {
  throw UnsupportedOperationException("Region.getInterestList()");
}

std::vector<std::shared_ptr<CacheableString>> ProxyRegion::getInterestListRegex() const {
  throw UnsupportedOperationException("Region.getInterestListRegex()");
}

void ProxyRegion::registerKeys(const std::vector<std::shared_ptr<CacheableKey>>&, bool, bool,
                  bool) {
  throw UnsupportedOperationException("Region.registerKeys()");
}

void ProxyRegion::unregisterKeys(const std::vector<std::shared_ptr<CacheableKey>>&) {
  throw UnsupportedOperationException("Region.unregisterKeys()");
}

void ProxyRegion::registerAllKeys(bool, bool, bool) {
  throw UnsupportedOperationException("Region.registerAllKeys()");
}

void ProxyRegion::unregisterAllKeys() {
  throw UnsupportedOperationException("Region.unregisterAllKeys()");
}

void ProxyRegion::registerRegex(const std::string&, bool, bool, bool) {
  throw UnsupportedOperationException("Region.registerRegex()");
}

void ProxyRegion::unregisterRegex(const std::string&) {
  throw UnsupportedOperationException("Region.unregisterRegex()");
}

HashMapOfCacheable ProxyRegion::getAll(
    const std::vector<std::shared_ptr<CacheableKey>>& keys,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  GuardUserAttributes gua(m_authenticatedView);
  return m_realRegion->getAll_internal(keys, aCallbackArgument, false);
}

std::shared_ptr<SelectResults> ProxyRegion::query(
    const std::string& predicate,
    std::chrono::milliseconds timeout) {
  GuardUserAttributes gua(m_authenticatedView);
  return m_realRegion->query(predicate, timeout);
}

bool ProxyRegion::existsValue(
    const std::string& predicate,
    std::chrono::milliseconds timeout) {
  GuardUserAttributes gua(m_authenticatedView);
  return m_realRegion->existsValue(predicate, timeout);
}

std::shared_ptr<Serializable> ProxyRegion::selectValue(
    const std::string& predicate,
    std::chrono::milliseconds timeout) {
  GuardUserAttributes gua(m_authenticatedView);
  return m_realRegion->selectValue(predicate, timeout);
}

void ProxyRegion::removeAll(
    const std::vector<std::shared_ptr<CacheableKey>>& keys,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  GuardUserAttributes gua(m_authenticatedView);
  m_realRegion->removeAll(keys, aCallbackArgument);
}

uint32_t ProxyRegion::size() { return m_realRegion->size(); }

const std::shared_ptr<Pool>& ProxyRegion::getPool() const { return m_realRegion->getPool(); }

ProxyRegion::ProxyRegion(AuthenticatedView& authenticatedView,
            const std::shared_ptr<RegionInternal>& realRegion)
    : Region(authenticatedView.m_cacheImpl) {
  m_authenticatedView = &authenticatedView;
  m_realRegion = realRegion;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
