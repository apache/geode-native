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

#include "MapEntry.hpp"

#include "MapEntryT.hpp"

namespace apache {
namespace geode {
namespace client {

void EntryFactory::newMapEntry(ExpiryTaskManager*,
                               const std::shared_ptr<CacheableKey>& key,
                               std::shared_ptr<MapEntryImpl>& result) const {
  if (m_concurrencyChecksEnabled) {
    result = MapEntryT<VersionedMapEntryImpl, 0, 0>::create(key);
  } else {
    result = MapEntryT<MapEntryImpl, 0, 0>::create(key);
  }
}

ExpEntryProperties::ExpEntryProperties(ExpiryTaskManager* expiryTaskManager)
    : m_lastAccessTime(0),
      m_lastModifiedTime(0),
      m_expiryTaskId(-1),
      m_expiryTaskManager(expiryTaskManager) {
  // The reactor always gives +ve id while scheduling.
  // -1 will indicate that an expiry task has not been scheduled
  // for this entry. // TODO confirm
}

ExpEntryProperties::time_point ExpEntryProperties::getLastAccessTime() const {
  return time_point(std::chrono::system_clock::duration(m_lastAccessTime));
}

ExpEntryProperties::time_point ExpEntryProperties::getLastModifiedTime() const {
  return time_point(std::chrono::system_clock::duration(m_lastModifiedTime));
}

//  moved time initialization outside of constructor to avoid
// the costly gettimeofday call in MapSegment spinlock
void ExpEntryProperties::initStartTime() {
  time_point currentTime = std::chrono::system_clock::now();
  m_lastAccessTime = currentTime.time_since_epoch().count();
  m_lastModifiedTime = currentTime.time_since_epoch().count();
}

void ExpEntryProperties::updateLastAccessTime(time_point currTime) {
  m_lastAccessTime = currTime.time_since_epoch().count();
}

void ExpEntryProperties::updateLastModifiedTime(time_point currTime) {
  m_lastModifiedTime = currTime.time_since_epoch().count();
}

void ExpEntryProperties::setExpiryTaskId(ExpiryTaskManager::id_type id) {
  m_expiryTaskId = id;
}

ExpiryTaskManager::id_type ExpEntryProperties::getExpiryTaskId() const {
  return m_expiryTaskId;
}

void ExpEntryProperties::cancelExpiryTaskId(
    const std::shared_ptr<CacheableKey>& key) const {
  LOGDEBUG("Cancelling expiration task for key [%s] with id [%d]",
           Utils::nullSafeToString(key).c_str(), m_expiryTaskId);
  m_expiryTaskManager->cancelTask(m_expiryTaskId);
}

MapEntry::MapEntry(bool) {}

ExpEntryProperties::ExpEntryProperties(bool)
    : m_lastAccessTime(0), m_lastModifiedTime(0) {}

void MapEntryImpl::getKeyI(std::shared_ptr<CacheableKey>& result) const {
  result = m_key;
}

void MapEntryImpl::getValueI(std::shared_ptr<Cacheable>& result) const {
  // If value is destroyed, then this returns nullptr
  if (CacheableToken::isDestroyed(m_value)) {
    result = nullptr;
  } else {
    result = m_value;
  }
}

void MapEntryImpl::setValueI(const std::shared_ptr<Cacheable>& value) {
  m_value = value;
}

void MapEntryImpl::getKey(std::shared_ptr<CacheableKey>& result) const {
  getKeyI(result);
}

void MapEntryImpl::getValue(std::shared_ptr<Cacheable>& result) const {
  getValueI(result);
}

void MapEntryImpl::setValue(const std::shared_ptr<Cacheable>& value) {
  setValueI(value);
}

std::shared_ptr<MapEntryImpl> MapEntryImpl::getImplPtr() {
  return shared_from_this();
}

LRUEntryProperties& MapEntryImpl::getLRUProperties() {
  throw FatalInternalException(
      "MapEntry::getLRUProperties called for "
      "non-LRU MapEntry");
}

ExpEntryProperties& MapEntryImpl::getExpProperties() {
  throw FatalInternalException(
      "MapEntry::getExpProperties called for "
      "non-expiration MapEntry");
}

VersionStamp& MapEntryImpl::getVersionStamp() {
  throw FatalInternalException(
      "MapEntry::getVersionStamp called for "
      "non-versioned MapEntry");
}

void MapEntryImpl::cleanup(const CacheEventFlags) {}

MapEntryImpl::MapEntryImpl(bool) : MapEntry(true) {}

MapEntryImpl::MapEntryImpl(const std::shared_ptr<CacheableKey>& key)
    : MapEntry(), m_key(key) {}

VersionStamp& VersionedMapEntryImpl::getVersionStamp() { return *this; }

VersionedMapEntryImpl::VersionedMapEntryImpl(bool) : MapEntryImpl(true) {}

VersionedMapEntryImpl::VersionedMapEntryImpl(
    const std::shared_ptr<CacheableKey>& key)
    : MapEntryImpl(key) {}

EntryFactory::EntryFactory(const bool concurrencyChecksEnabled)
    : m_concurrencyChecksEnabled(concurrencyChecksEnabled) {}

}  // namespace client
}  // namespace geode
}  // namespace apache
