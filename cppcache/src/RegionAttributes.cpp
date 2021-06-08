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

#include <cstdlib>
#include <string>

#include <geode/Cache.hpp>
#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>
#include <geode/PersistenceManager.hpp>
#include <geode/Properties.hpp>

#include "CacheXmlParser.hpp"
#include "Utils.hpp"

namespace apache {
namespace geode {
namespace client {

using apache::geode::client::Utils;

RegionAttributes::RegionAttributes()
    : Serializable(),
      m_regionTimeToLiveExpirationAction(ExpirationAction::INVALIDATE),
      m_regionIdleTimeoutExpirationAction(ExpirationAction::INVALIDATE),
      m_entryTimeToLiveExpirationAction(ExpirationAction::INVALIDATE),
      m_entryIdleTimeoutExpirationAction(ExpirationAction::INVALIDATE),
      m_lruEvictionAction(ExpirationAction::LOCAL_DESTROY),
      m_lruEntriesLimit(0),
      m_caching(true),
      m_maxValueDistLimit(100 * 1024),
      m_entryIdleTimeout(0),
      m_entryTimeToLive(0),
      m_regionIdleTimeout(0),
      m_regionTimeToLive(0),
      m_initialCapacity(10000),
      m_loadFactor(0.75),
      m_concurrencyLevel(16),
      m_diskPolicy(DiskPolicyType::NONE),
      m_clientNotificationEnabled(false),
      m_persistenceProperties(nullptr),
      m_persistenceManager(nullptr),
      m_isClonable(false),
      m_isConcurrencyChecksEnabled(true) {}

RegionAttributes::~RegionAttributes() noexcept = default;

std::shared_ptr<CacheLoader> RegionAttributes::getCacheLoader() const {
  if (!m_cacheLoader && !m_cacheLoaderLibrary.empty()) {
    if (CacheXmlParser::managedCacheLoaderFn_ &&
        m_cacheLoaderFactory.find('.') != std::string::npos) {
      // this is a managed library
      m_cacheLoader.reset((
          CacheXmlParser::managedCacheLoaderFn_)(m_cacheLoaderLibrary.c_str(),
                                                 m_cacheLoaderFactory.c_str()));
    } else {
      auto funcptr = Utils::getFactoryFunction<CacheLoader*()>(
          m_cacheLoaderLibrary, m_cacheLoaderFactory);
      m_cacheLoader.reset(funcptr());
    }
  }
  return m_cacheLoader;
}

std::shared_ptr<CacheWriter> RegionAttributes::getCacheWriter() const {
  if (!m_cacheWriter && !m_cacheWriterLibrary.empty()) {
    if (CacheXmlParser::managedCacheWriterFn_ &&
        m_cacheWriterFactory.find('.') != std::string::npos) {
      // this is a managed library
      m_cacheWriter.reset((
          CacheXmlParser::managedCacheWriterFn_)(m_cacheWriterLibrary.c_str(),
                                                 m_cacheWriterFactory.c_str()));
    } else {
      auto funcptr = Utils::getFactoryFunction<CacheWriter*()>(
          m_cacheWriterLibrary, m_cacheWriterFactory);
      m_cacheWriter.reset(funcptr());
    }
  }
  return m_cacheWriter;
}

std::shared_ptr<CacheListener> RegionAttributes::getCacheListener() const {
  if (!m_cacheListener && !m_cacheListenerLibrary.empty()) {
    if (CacheXmlParser::managedCacheListenerFn_ &&
        m_cacheListenerFactory.find('.') != std::string::npos) {
      // this is a managed library
      m_cacheListener.reset(
          (CacheXmlParser::managedCacheListenerFn_)(m_cacheListenerLibrary
                                                        .c_str(),
                                                    m_cacheListenerFactory
                                                        .c_str()));
    } else {
      auto funcptr = Utils::getFactoryFunction<CacheListener*()>(
          m_cacheListenerLibrary, m_cacheListenerFactory);
      m_cacheListener.reset(funcptr());
    }
  }
  return m_cacheListener;
}

std::shared_ptr<PartitionResolver> RegionAttributes::getPartitionResolver()
    const {
  if (!m_partitionResolver && (!m_partitionResolverLibrary.empty() ||
                               !m_partitionResolverFactory.empty())) {
    if (CacheXmlParser::managedPartitionResolverFn_ &&
        m_partitionResolverFactory.find('.') != std::string::npos) {
      // this is a managed library
      m_partitionResolver.reset((
          CacheXmlParser::
              managedPartitionResolverFn_)(m_partitionResolverLibrary.c_str(),
                                           m_partitionResolverFactory.c_str()));
    } else {
      auto funcptr = Utils::getFactoryFunction<PartitionResolver*()>(
          m_partitionResolverLibrary, m_partitionResolverFactory);
      m_partitionResolver.reset(funcptr());
    }
  }
  return m_partitionResolver;
}

std::shared_ptr<PersistenceManager> RegionAttributes::getPersistenceManager()
    const {
  if (!m_persistenceManager && !m_persistenceLibrary.empty()) {
    if (CacheXmlParser::managedPersistenceManagerFn_ &&
        m_persistenceFactory.find('.') != std::string::npos) {
      // this is a managed library
      m_persistenceManager.reset(
          (CacheXmlParser::managedPersistenceManagerFn_)(m_persistenceLibrary
                                                             .c_str(),
                                                         m_persistenceFactory
                                                             .c_str()));
    } else {
      auto funcptr = Utils::getFactoryFunction<PersistenceManager*()>(
          m_persistenceLibrary, m_persistenceFactory);
      m_persistenceManager.reset(funcptr());
    }
  }
  return m_persistenceManager;
}

const std::string& RegionAttributes::getCacheLoaderFactory() const {
  return m_cacheLoaderFactory;
}

const std::string& RegionAttributes::getCacheWriterFactory() const {
  return m_cacheWriterFactory;
}

const std::string& RegionAttributes::getCacheListenerFactory() const {
  return m_cacheListenerFactory;
}

const std::string& RegionAttributes::getPartitionResolverFactory() const {
  return m_partitionResolverFactory;
}

const std::string& RegionAttributes::getPersistenceFactory() const {
  return m_persistenceFactory;
}
const std::string& RegionAttributes::getCacheLoaderLibrary() const {
  return m_cacheLoaderLibrary;
}

const std::string& RegionAttributes::getCacheWriterLibrary() const {
  return m_cacheWriterLibrary;
}

const std::string& RegionAttributes::getCacheListenerLibrary() const {
  return m_cacheListenerLibrary;
}

const std::string& RegionAttributes::getPartitionResolverLibrary() const {
  return m_partitionResolverLibrary;
}

const std::string& RegionAttributes::getEndpoints() const {
  return m_endpoints;
}

bool RegionAttributes::getClientNotificationEnabled() const {
  return m_clientNotificationEnabled;
}

const std::string& RegionAttributes::getPersistenceLibrary() const {
  return m_persistenceLibrary;
}

std::shared_ptr<Properties> RegionAttributes::getPersistenceProperties() const {
  return m_persistenceProperties;
}

std::chrono::seconds RegionAttributes::getRegionTimeToLive() const {
  return m_regionTimeToLive;
}

ExpirationAction RegionAttributes::getRegionTimeToLiveAction() const {
  return m_regionTimeToLiveExpirationAction;
}

std::chrono::seconds RegionAttributes::getRegionIdleTimeout() const {
  return m_regionIdleTimeout;
}

ExpirationAction RegionAttributes::getRegionIdleTimeoutAction() const {
  return m_regionIdleTimeoutExpirationAction;
}

std::chrono::seconds RegionAttributes::getEntryTimeToLive() const {
  return m_entryTimeToLive;
}

ExpirationAction RegionAttributes::getEntryTimeToLiveAction() const {
  return m_entryTimeToLiveExpirationAction;
}

std::chrono::seconds RegionAttributes::getEntryIdleTimeout() const {
  return m_entryIdleTimeout;
}

ExpirationAction RegionAttributes::getEntryIdleTimeoutAction() const {
  return m_entryIdleTimeoutExpirationAction;
}

int RegionAttributes::getInitialCapacity() const { return m_initialCapacity; }

float RegionAttributes::getLoadFactor() const { return m_loadFactor; }

uint8_t RegionAttributes::getConcurrencyLevel() const {
  return m_concurrencyLevel;
}

ExpirationAction RegionAttributes::getLruEvictionAction() const {
  return m_lruEvictionAction;
}

uint32_t RegionAttributes::getLruEntriesLimit() const {
  return m_lruEntriesLimit;
}

DiskPolicyType RegionAttributes::getDiskPolicy() const { return m_diskPolicy; }

std::shared_ptr<Serializable> RegionAttributes::createDeserializable() {
  return std::make_shared<RegionAttributes>();
}

namespace impl {

void writeBool(DataOutput& out, bool field) {
  out.write(static_cast<int8_t>(field ? 1 : 0));
}

void readBool(DataInput& in, bool* field) { *field = in.read() ? true : false; }

void writeString(DataOutput& out, const std::string& field) {
  out.writeBytes(reinterpret_cast<int8_t*>(const_cast<char*>(field.c_str())),
                 static_cast<uint32_t>(field.length()) + 1);
}

void readString(DataInput& in, std::string& field) {
  // length including null terminator
  auto len = in.readArrayLength();
  // currentBufferPosition is read-only and we are only reading, cast away const
  field = std::string(const_cast<char*>(reinterpret_cast<const char*>(
                          in.currentBufferPosition())),
                      len - 1);
  in.advanceCursor(len);
}

}  // namespace impl

void RegionAttributes::toData(DataOutput& out) const {
  out.writeInt(static_cast<int32_t>(m_regionTimeToLive.count()));
  out.writeInt(static_cast<int32_t>(m_regionTimeToLiveExpirationAction));
  out.writeInt(static_cast<int32_t>(m_regionIdleTimeout.count()));
  out.writeInt(static_cast<int32_t>(m_regionIdleTimeoutExpirationAction));
  out.writeInt(static_cast<int32_t>(m_entryTimeToLive.count()));
  out.writeInt(static_cast<int32_t>(m_entryTimeToLiveExpirationAction));
  out.writeInt(static_cast<int32_t>(m_entryIdleTimeout.count()));
  out.writeInt(static_cast<int32_t>(m_entryIdleTimeoutExpirationAction));
  out.writeInt(static_cast<int32_t>(m_initialCapacity));
  out.writeFloat(m_loadFactor);
  out.writeInt(static_cast<int32_t>(m_maxValueDistLimit));
  out.writeInt(static_cast<int32_t>(m_concurrencyLevel));
  out.writeInt(static_cast<int32_t>(m_lruEntriesLimit));
  out.writeInt(static_cast<int32_t>(m_lruEvictionAction));

  apache::geode::client::impl::writeBool(out, m_caching);
  apache::geode::client::impl::writeBool(out, m_clientNotificationEnabled);

  apache::geode::client::impl::writeString(out, m_cacheLoaderLibrary);
  apache::geode::client::impl::writeString(out, m_cacheLoaderFactory);
  apache::geode::client::impl::writeString(out, m_cacheWriterLibrary);
  apache::geode::client::impl::writeString(out, m_cacheWriterFactory);
  apache::geode::client::impl::writeString(out, m_cacheListenerLibrary);
  apache::geode::client::impl::writeString(out, m_cacheListenerFactory);
  apache::geode::client::impl::writeString(out, m_partitionResolverLibrary);
  apache::geode::client::impl::writeString(out, m_partitionResolverFactory);
  out.writeInt(static_cast<int32_t>(m_diskPolicy));
  apache::geode::client::impl::writeString(out, m_endpoints);
  apache::geode::client::impl::writeString(out, m_persistenceLibrary);
  apache::geode::client::impl::writeString(out, m_persistenceFactory);
  out.writeObject(m_persistenceProperties);
  apache::geode::client::impl::writeString(out, m_poolName);
  apache::geode::client::impl::writeBool(out, m_isConcurrencyChecksEnabled);
}

void RegionAttributes::fromData(DataInput& in) {
  m_regionTimeToLive = std::chrono::seconds(in.readInt32());
  m_regionTimeToLiveExpirationAction =
      static_cast<ExpirationAction>(in.readInt32());
  m_regionIdleTimeout = std::chrono::seconds(in.readInt32());
  m_regionIdleTimeoutExpirationAction =
      static_cast<ExpirationAction>(in.readInt32());
  m_entryTimeToLive = std::chrono::seconds(in.readInt32());
  m_entryTimeToLiveExpirationAction =
      static_cast<ExpirationAction>(in.readInt32());
  m_entryIdleTimeout = std::chrono::seconds(in.readInt32());
  m_entryIdleTimeoutExpirationAction =
      static_cast<ExpirationAction>(in.readInt32());
  m_initialCapacity = in.readInt32();
  m_loadFactor = in.readFloat();
  m_maxValueDistLimit = in.readInt32();
  m_concurrencyLevel = in.readInt32();
  m_lruEntriesLimit = in.readInt32();
  m_lruEvictionAction = static_cast<ExpirationAction>(in.readInt32());

  apache::geode::client::impl::readBool(in, &m_caching);
  apache::geode::client::impl::readBool(in, &m_clientNotificationEnabled);

  apache::geode::client::impl::readString(in, m_cacheLoaderLibrary);
  apache::geode::client::impl::readString(in, m_cacheLoaderFactory);
  apache::geode::client::impl::readString(in, m_cacheWriterLibrary);
  apache::geode::client::impl::readString(in, m_cacheWriterFactory);
  apache::geode::client::impl::readString(in, m_cacheListenerLibrary);
  apache::geode::client::impl::readString(in, m_cacheListenerFactory);
  apache::geode::client::impl::readString(in, m_partitionResolverLibrary);
  apache::geode::client::impl::readString(in, m_partitionResolverFactory);
  m_diskPolicy = static_cast<DiskPolicyType>(in.readInt32());
  apache::geode::client::impl::readString(in, m_endpoints);
  apache::geode::client::impl::readString(in, m_persistenceLibrary);
  apache::geode::client::impl::readString(in, m_persistenceFactory);
  m_persistenceProperties =
      std::dynamic_pointer_cast<Properties>(in.readObject());
  apache::geode::client::impl::readString(in, m_poolName);
  apache::geode::client::impl::readBool(in, &m_isConcurrencyChecksEnabled);
}

/** Return true if all the attributes are equal to those of other. */
bool RegionAttributes::operator==(const RegionAttributes& other) const {
  if (m_regionTimeToLive != other.m_regionTimeToLive) return false;
  if (m_regionTimeToLiveExpirationAction !=
      other.m_regionTimeToLiveExpirationAction) {
    return false;
  }
  if (m_regionIdleTimeout != other.m_regionIdleTimeout) return false;
  if (m_regionIdleTimeoutExpirationAction !=
      other.m_regionIdleTimeoutExpirationAction) {
    return false;
  }
  if (m_entryTimeToLive != other.m_entryTimeToLive) return false;
  if (m_entryTimeToLiveExpirationAction !=
      other.m_entryTimeToLiveExpirationAction) {
    return false;
  }
  if (m_entryIdleTimeout != other.m_entryIdleTimeout) return false;
  if (m_entryIdleTimeoutExpirationAction !=
      other.m_entryIdleTimeoutExpirationAction) {
    return false;
  }
  if (m_initialCapacity != other.m_initialCapacity) return false;
  if (m_loadFactor != other.m_loadFactor) return false;
  if (m_maxValueDistLimit != other.m_maxValueDistLimit) return false;
  if (m_concurrencyLevel != other.m_concurrencyLevel) return false;
  if (m_lruEntriesLimit != other.m_lruEntriesLimit) return false;
  if (m_lruEvictionAction != other.m_lruEvictionAction) return false;
  if (m_caching != other.m_caching) return false;
  if (m_clientNotificationEnabled != other.m_clientNotificationEnabled) {
    return false;
  }

  if (m_cacheLoaderLibrary != other.m_cacheLoaderLibrary) {
    return false;
  }
  if (m_cacheLoaderFactory != other.m_cacheLoaderFactory) {
    return false;
  }
  if (m_cacheWriterLibrary != other.m_cacheWriterLibrary) {
    return false;
  }
  if (m_cacheWriterFactory != other.m_cacheWriterFactory) {
    return false;
  }
  if (m_cacheListenerLibrary != other.m_cacheListenerLibrary) {
    return false;
  }
  if (m_cacheListenerFactory != other.m_cacheListenerFactory) {
    return false;
  }
  if (m_partitionResolverLibrary != other.m_partitionResolverLibrary) {
    return false;
  }
  if (m_partitionResolverFactory != other.m_partitionResolverFactory) {
    return false;
  }
  if (m_diskPolicy != other.m_diskPolicy) {
    return false;
  }
  if (m_endpoints != other.m_endpoints) {
    return false;
  }
  if (m_persistenceLibrary != other.m_persistenceLibrary) {
    return false;
  }
  if (m_persistenceFactory != other.m_persistenceFactory) {
    return false;
  }
  if (m_isConcurrencyChecksEnabled != other.m_isConcurrencyChecksEnabled) {
    return false;
  }

  return true;
}

/** Return true if any of the attributes are not equal to those of other. */
bool RegionAttributes::operator!=(const RegionAttributes& other) const {
  return !(*this == other);
}

/* Throws IllegalStateException when attributes targetted for use on a server do
 * not meet requirements. */
void RegionAttributes::validateSerializableAttributes() {
  if (m_cacheLoader != nullptr) {
    throw IllegalStateException(
        "CacheLoader must be set with setCacheLoader(library, factory) in "
        "members of type SERVER");
  }
  if (m_cacheWriter != nullptr) {
    throw IllegalStateException(
        "CacheWriter must be set with setCacheWriter(library, factory) in "
        "members of type SERVER");
  }
  if (m_cacheListener != nullptr) {
    throw IllegalStateException(
        "CacheListener must be set with setCacheListener(library, factory) in "
        "members of type SERVER");
  }
  if (m_partitionResolver != nullptr) {
    throw IllegalStateException(
        "PartitionResolver must be set with setPartitionResolver(library, "
        "factory) in members of type SERVER");
  }
  if (m_persistenceManager != nullptr) {
    throw IllegalStateException(
        "persistenceManager must be set with setPersistenceManager(library, "
        "factory,config) in members of type SERVER");
  }
}

void RegionAttributes::setCacheListener(const std::string& lib,
                                        const std::string& func) {
  m_cacheListenerLibrary = lib;
  m_cacheListenerFactory = func;
}

void RegionAttributes::setPartitionResolver(const std::string& lib,
                                            const std::string& func) {
  m_partitionResolverLibrary = lib;
  m_partitionResolverFactory = func;
}

void RegionAttributes::setCacheLoader(const std::string& lib,
                                      const std::string& func) {
  m_cacheLoaderLibrary = lib;
  m_cacheLoaderFactory = func;
}

void RegionAttributes::setCacheWriter(const std::string& lib,
                                      const std::string& func) {
  m_cacheWriterLibrary = lib;
  m_cacheWriterFactory = func;
}

void RegionAttributes::setPersistenceManager(
    const std::string& lib, const std::string& func,
    const std::shared_ptr<Properties>& config) {
  m_persistenceLibrary = lib;
  m_persistenceFactory = func;
  m_persistenceProperties = config;
}

void RegionAttributes::setEndpoints(const std::string& endpoints) {
  m_endpoints = endpoints;
}

void RegionAttributes::setPoolName(const std::string& poolName) {
  m_poolName = poolName;
}

void RegionAttributes::setCachingEnabled(bool enable) { m_caching = enable; }

void RegionAttributes::setLruEntriesLimit(int limit) {
  m_lruEntriesLimit = limit;
}
void RegionAttributes::setDiskPolicy(DiskPolicyType diskPolicy) {
  m_diskPolicy = diskPolicy;
}

void RegionAttributes::setCloningEnabled(bool isClonable) {
  m_isClonable = isClonable;
}

void RegionAttributes::setConcurrencyChecksEnabled(bool enable) {
  m_isConcurrencyChecksEnabled = enable;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
