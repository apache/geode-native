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

#include <geode/Cache.hpp>
#include <Utils.hpp>
#include <geode/DataOutput.hpp>
#include <string>
#include <cstdlib>
#include <geode/GeodeTypeIds.hpp>
#include <CacheXmlParser.hpp>
#include <ace/DLL.h>
#include <ace/OS.h>
#include <geode/DataInput.hpp>
#include <geode/Properties.hpp>

using namespace apache::geode::client;
RegionAttributes::RegionAttributes()
    : Serializable(),
      m_regionTimeToLiveExpirationAction(ExpirationAction::INVALIDATE),
      m_regionIdleTimeoutExpirationAction(ExpirationAction::INVALIDATE),
      m_entryTimeToLiveExpirationAction(ExpirationAction::INVALIDATE),
      m_entryIdleTimeoutExpirationAction(ExpirationAction::INVALIDATE),
      m_lruEvictionAction(ExpirationAction::LOCAL_DESTROY),
      m_cacheWriter(nullptr),
      m_cacheLoader(nullptr),
      m_cacheListener(nullptr),
      m_partitionResolver(nullptr),
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
      m_cacheLoaderLibrary(nullptr),
      m_cacheWriterLibrary(nullptr),
      m_cacheListenerLibrary(nullptr),
      m_partitionResolverLibrary(nullptr),
      m_cacheLoaderFactory(nullptr),
      m_cacheWriterFactory(nullptr),
      m_cacheListenerFactory(nullptr),
      m_partitionResolverFactory(nullptr),
      m_diskPolicy(DiskPolicyType::NONE),
      m_endpoints(nullptr),
      m_clientNotificationEnabled(false),
      m_persistenceLibrary(nullptr),
      m_persistenceFactory(nullptr),
      m_persistenceProperties(nullptr),
      m_persistenceManager(nullptr),
      m_poolName(nullptr),
      m_isClonable(false),
      m_isConcurrencyChecksEnabled(true) {}

RegionAttributes::RegionAttributes(const RegionAttributes& rhs)
    : m_regionTimeToLiveExpirationAction(
          rhs.m_regionTimeToLiveExpirationAction),
      m_regionIdleTimeoutExpirationAction(
          rhs.m_regionIdleTimeoutExpirationAction),
      m_entryTimeToLiveExpirationAction(rhs.m_entryTimeToLiveExpirationAction),
      m_entryIdleTimeoutExpirationAction(
          rhs.m_entryIdleTimeoutExpirationAction),
      m_lruEvictionAction(rhs.m_lruEvictionAction),
      m_cacheWriter(rhs.m_cacheWriter),
      m_cacheLoader(rhs.m_cacheLoader),
      m_cacheListener(rhs.m_cacheListener),
      m_partitionResolver(rhs.m_partitionResolver),
      m_lruEntriesLimit(rhs.m_lruEntriesLimit),
      m_caching(rhs.m_caching),
      m_maxValueDistLimit(rhs.m_maxValueDistLimit),
      m_entryIdleTimeout(rhs.m_entryIdleTimeout),
      m_entryTimeToLive(rhs.m_entryTimeToLive),
      m_regionIdleTimeout(rhs.m_regionIdleTimeout),
      m_regionTimeToLive(rhs.m_regionTimeToLive),
      m_initialCapacity(rhs.m_initialCapacity),
      m_loadFactor(rhs.m_loadFactor),
      m_concurrencyLevel(rhs.m_concurrencyLevel),
      m_diskPolicy(rhs.m_diskPolicy),
      m_clientNotificationEnabled(rhs.m_clientNotificationEnabled),
      m_persistenceProperties(rhs.m_persistenceProperties),
      m_persistenceManager(rhs.m_persistenceManager),
      m_isClonable(rhs.m_isClonable),
      m_isConcurrencyChecksEnabled(rhs.m_isConcurrencyChecksEnabled) {
  if (rhs.m_cacheLoaderLibrary != nullptr) {
    size_t len = strlen(rhs.m_cacheLoaderLibrary) + 1;
    m_cacheLoaderLibrary = new char[len];
    ACE_OS::strncpy(m_cacheLoaderLibrary, rhs.m_cacheLoaderLibrary, len);
  } else {
    m_cacheLoaderLibrary = nullptr;
  }
  if (rhs.m_cacheWriterLibrary != nullptr) {
    size_t len = strlen(rhs.m_cacheWriterLibrary) + 1;
    m_cacheWriterLibrary = new char[len];
    ACE_OS::strncpy(m_cacheWriterLibrary, rhs.m_cacheWriterLibrary, len);
  } else {
    m_cacheWriterLibrary = nullptr;
  }
  if (rhs.m_cacheListenerLibrary != nullptr) {
    size_t len = strlen(rhs.m_cacheListenerLibrary) + 1;
    m_cacheListenerLibrary = new char[len];
    ACE_OS::strncpy(m_cacheListenerLibrary, rhs.m_cacheListenerLibrary, len);
  } else {
    m_cacheListenerLibrary = nullptr;
  }
  if (rhs.m_partitionResolverLibrary != nullptr) {
    size_t len = strlen(rhs.m_partitionResolverLibrary) + 1;
    m_partitionResolverLibrary = new char[len];
    ACE_OS::strncpy(m_partitionResolverLibrary, rhs.m_partitionResolverLibrary,
                    len);
  } else {
    m_partitionResolverLibrary = nullptr;
  }
  if (rhs.m_cacheLoaderFactory != nullptr) {
    size_t len = strlen(rhs.m_cacheLoaderFactory) + 1;
    m_cacheLoaderFactory = new char[len];
    ACE_OS::strncpy(m_cacheLoaderFactory, rhs.m_cacheLoaderFactory, len);
  } else {
    m_cacheLoaderFactory = nullptr;
  }
  if (rhs.m_cacheWriterFactory != nullptr) {
    size_t len = strlen(rhs.m_cacheWriterFactory) + 1;
    m_cacheWriterFactory = new char[len];
    ACE_OS::strncpy(m_cacheWriterFactory, rhs.m_cacheWriterFactory, len);
  } else {
    m_cacheWriterFactory = nullptr;
  }
  if (rhs.m_cacheListenerFactory != nullptr) {
    size_t len = strlen(rhs.m_cacheListenerFactory) + 1;
    m_cacheListenerFactory = new char[len];
    ACE_OS::strncpy(m_cacheListenerFactory, rhs.m_cacheListenerFactory, len);
  } else {
    m_cacheListenerFactory = nullptr;
  }
  if (rhs.m_partitionResolverFactory != nullptr) {
    size_t len = strlen(rhs.m_partitionResolverFactory) + 1;
    m_partitionResolverFactory = new char[len];
    ACE_OS::strncpy(m_partitionResolverFactory, rhs.m_partitionResolverFactory,
                    len);
  } else {
    m_partitionResolverFactory = nullptr;
  }
  if (rhs.m_endpoints != nullptr) {
    size_t len = strlen(rhs.m_endpoints) + 1;
    m_endpoints = new char[len];
    ACE_OS::strncpy(m_endpoints, rhs.m_endpoints, len);
  } else {
    m_endpoints = nullptr;
  }
  if (rhs.m_poolName != nullptr) {
    size_t len = strlen(rhs.m_poolName) + 1;
    m_poolName = new char[len];
    ACE_OS::strncpy(m_poolName, rhs.m_poolName, len);
  } else {
    m_poolName = nullptr;
  }
  if (rhs.m_persistenceLibrary != nullptr) {
    size_t len = strlen(rhs.m_persistenceLibrary) + 1;
    m_persistenceLibrary = new char[len];
    ACE_OS::strncpy(m_persistenceLibrary, rhs.m_persistenceLibrary, len);
  } else {
    m_persistenceLibrary = nullptr;
  }
  if (rhs.m_persistenceFactory != nullptr) {
    size_t len = strlen(rhs.m_persistenceFactory) + 1;
    m_persistenceFactory = new char[len];
    ACE_OS::strncpy(m_persistenceFactory, rhs.m_persistenceFactory, len);
  } else {
    m_persistenceFactory = nullptr;
  }
}

#define RA_DELSTRING(x) \
  if (x != nullptr) {   \
    delete[] x;         \
  }                     \
  x = nullptr

RegionAttributes::~RegionAttributes() {
  RA_DELSTRING(m_cacheLoaderLibrary);
  RA_DELSTRING(m_cacheWriterLibrary);
  RA_DELSTRING(m_cacheListenerLibrary);
  RA_DELSTRING(m_partitionResolverLibrary);
  RA_DELSTRING(m_cacheLoaderFactory);
  RA_DELSTRING(m_cacheWriterFactory);
  RA_DELSTRING(m_cacheListenerFactory);
  RA_DELSTRING(m_partitionResolverFactory);
  RA_DELSTRING(m_endpoints);
  RA_DELSTRING(m_persistenceLibrary);
  RA_DELSTRING(m_persistenceFactory);
  RA_DELSTRING(m_poolName);
}

namespace apache {
namespace geode {
namespace client {
namespace impl {

/**
 * lib should be in the form required by ACE_DLL, typically just like specifying
 * a
 * lib in java System.loadLibrary( "x" ); Where x is a component of the name
 * lib<x>.so on unix, or <x>.dll on windows.
 */
void* getFactoryFunc(const char* lib, const char* funcName) {
  ACE_DLL dll;
  if (dll.open(lib, ACE_DEFAULT_SHLIB_MODE, 0) == -1) {
    // error...
    char msg[1000];
    ACE_OS::snprintf(msg, 1000, "cannot open library: %s", lib);
    throw IllegalArgumentException(msg);
  }
  void* func = dll.symbol(funcName);
  if (func == nullptr) {
    char msg[1000];
    ACE_OS::snprintf(msg, 1000, "cannot find factory function %s in library %s",
                     funcName, lib);
    throw IllegalArgumentException(msg);
  }
  return func;
}
}  // namespace impl
}  // namespace client
}  // namespace geode
}  // namespace apache

CacheLoaderPtr RegionAttributes::getCacheLoader() {
  if ((m_cacheLoader == nullptr) && (m_cacheLoaderLibrary != nullptr)) {
    if (CacheXmlParser::managedCacheLoaderFn != nullptr &&
        strchr(m_cacheLoaderFactory, '.') != nullptr) {
      // this is a managed library
      m_cacheLoader.reset((*CacheXmlParser::managedCacheLoaderFn)(
          m_cacheLoaderLibrary, m_cacheLoaderFactory));
    } else {
      CacheLoader* (*funcptr)();
      funcptr = reinterpret_cast<CacheLoader* (*)()>(
          apache::geode::client::impl::getFactoryFunc(m_cacheLoaderLibrary,
                                                      m_cacheLoaderFactory));
      m_cacheLoader.reset(funcptr());
    }
  }
  return m_cacheLoader;
}

CacheWriterPtr RegionAttributes::getCacheWriter() {
  if ((m_cacheWriter == nullptr) && (m_cacheWriterLibrary != nullptr)) {
    if (CacheXmlParser::managedCacheWriterFn != nullptr &&
        strchr(m_cacheWriterFactory, '.') != nullptr) {
      // this is a managed library
      m_cacheWriter.reset((*CacheXmlParser::managedCacheWriterFn)(
          m_cacheWriterLibrary, m_cacheWriterFactory));
    } else {
      CacheWriter* (*funcptr)();
      funcptr = reinterpret_cast<CacheWriter* (*)()>(
          apache::geode::client::impl::getFactoryFunc(m_cacheWriterLibrary,
                                                      m_cacheWriterFactory));
      m_cacheWriter.reset(funcptr());
    }
  }
  return m_cacheWriter;
}

CacheListenerPtr RegionAttributes::getCacheListener() {
  if ((m_cacheListener == nullptr) && (m_cacheListenerLibrary != nullptr)) {
    if (CacheXmlParser::managedCacheListenerFn != nullptr &&
        strchr(m_cacheListenerFactory, '.') != nullptr) {
      // LOGDEBUG( "RegionAttributes::getCacheListener: Trying to create
      // instance from managed library." );
      // this is a managed library
      m_cacheListener.reset((*CacheXmlParser::managedCacheListenerFn)(
          m_cacheListenerLibrary, m_cacheListenerFactory));
    } else {
      CacheListener* (*funcptr)();
      funcptr = reinterpret_cast<CacheListener* (*)()>(
          apache::geode::client::impl::getFactoryFunc(m_cacheListenerLibrary,
                                                      m_cacheListenerFactory));
      m_cacheListener.reset(funcptr());
    }
  }
  return m_cacheListener;
}

PartitionResolverPtr RegionAttributes::getPartitionResolver() {
  if ((m_partitionResolver == nullptr) &&
      (m_partitionResolverLibrary != nullptr)) {
    if (CacheXmlParser::managedPartitionResolverFn != nullptr &&
        strchr(m_partitionResolverFactory, '.') != nullptr) {
      // LOGDEBUG( "RegionAttributes::getCacheListener: Trying to create
      // instance from managed library." );
      // this is a managed library
      m_partitionResolver.reset((*CacheXmlParser::managedPartitionResolverFn)(
          m_partitionResolverLibrary, m_partitionResolverFactory));
    } else {
      PartitionResolver* (*funcptr)();
      funcptr = reinterpret_cast<PartitionResolver* (*)()>(
          apache::geode::client::impl::getFactoryFunc(
              m_partitionResolverLibrary, m_partitionResolverFactory));
      m_partitionResolver.reset(funcptr());
    }
  }
  return m_partitionResolver;
}

PersistenceManagerPtr RegionAttributes::getPersistenceManager() {
  if ((m_persistenceManager == nullptr) && (m_persistenceLibrary != nullptr)) {
    if (CacheXmlParser::managedPartitionResolverFn != nullptr &&
        strchr(m_persistenceFactory, '.') != nullptr) {
      LOGDEBUG(
          "RegionAttributes::getPersistenceManager: Trying to create instance "
          "from managed library.");
      // this is a managed library
      m_persistenceManager.reset((*CacheXmlParser::managedPersistenceManagerFn)(
          m_persistenceLibrary, m_persistenceFactory));
    } else {
      PersistenceManager* (*funcptr)();
      funcptr = reinterpret_cast<PersistenceManager* (*)()>(
          apache::geode::client::impl::getFactoryFunc(m_persistenceLibrary,
                                                      m_persistenceFactory));
      m_persistenceManager.reset(funcptr());
    }
  }
  return m_persistenceManager;
}
const char* RegionAttributes::getCacheLoaderFactory() {
  return m_cacheLoaderFactory;
}

const char* RegionAttributes::getCacheWriterFactory() {
  return m_cacheWriterFactory;
}

const char* RegionAttributes::getCacheListenerFactory() {
  return m_cacheListenerFactory;
}

const char* RegionAttributes::getPartitionResolverFactory() {
  return m_partitionResolverFactory;
}

const char* RegionAttributes::getPersistenceFactory() {
  return m_persistenceFactory;
}
const char* RegionAttributes::getCacheLoaderLibrary() {
  return m_cacheLoaderLibrary;
}

const char* RegionAttributes::getCacheWriterLibrary() {
  return m_cacheWriterLibrary;
}

const char* RegionAttributes::getCacheListenerLibrary() {
  return m_cacheListenerLibrary;
}

const char* RegionAttributes::getPartitionResolverLibrary() {
  return m_partitionResolverLibrary;
}

const char* RegionAttributes::getEndpoints() { return m_endpoints; }
bool RegionAttributes::getClientNotificationEnabled() const {
  return m_clientNotificationEnabled;
}
const char* RegionAttributes::getPersistenceLibrary() {
  return m_persistenceLibrary;
}

PropertiesPtr RegionAttributes::getPersistenceProperties() {
  return m_persistenceProperties;
}

int RegionAttributes::getRegionTimeToLive() { return m_regionTimeToLive; }

ExpirationAction::Action RegionAttributes::getRegionTimeToLiveAction() {
  return m_regionTimeToLiveExpirationAction;
}

int RegionAttributes::getRegionIdleTimeout() { return m_regionIdleTimeout; }

ExpirationAction::Action RegionAttributes::getRegionIdleTimeoutAction() {
  return m_regionIdleTimeoutExpirationAction;
}

int RegionAttributes::getEntryTimeToLive() { return m_entryTimeToLive; }

ExpirationAction::Action RegionAttributes::getEntryTimeToLiveAction() {
  return m_entryTimeToLiveExpirationAction;
}

int RegionAttributes::getEntryIdleTimeout() { return m_entryIdleTimeout; }

ExpirationAction::Action RegionAttributes::getEntryIdleTimeoutAction() {
  return m_entryIdleTimeoutExpirationAction;
}

int RegionAttributes::getInitialCapacity() const { return m_initialCapacity; }

float RegionAttributes::getLoadFactor() const { return m_loadFactor; }

uint8_t RegionAttributes::getConcurrencyLevel() const {
  return m_concurrencyLevel;
}

const ExpirationAction::Action RegionAttributes::getLruEvictionAction() const {
  return m_lruEvictionAction;
}

uint32_t RegionAttributes::getLruEntriesLimit() const {
  return m_lruEntriesLimit;
}

DiskPolicyType::PolicyType RegionAttributes::getDiskPolicy() const {
  return m_diskPolicy;
}
const char* RegionAttributes::getPoolName() const { return m_poolName; }
Serializable* RegionAttributes::createDeserializable() {
  return new RegionAttributes();
}

int32_t RegionAttributes::classId() const { return 0; }

int8_t RegionAttributes::typeId() const {
  return GeodeTypeIds::RegionAttributes;
}

namespace apache {
namespace geode {
namespace client {
namespace impl {

void writeBool(DataOutput& out, bool field) {
  out.write(static_cast<int8_t>(field ? 1 : 0));
}

void readBool(DataInput& in, bool* field) {
  *field = in.read() ? true : false;
}

void writeCharStar(DataOutput& out, const char* field) {
  if (field == nullptr) {
    out.writeBytes(reinterpret_cast<const int8_t*>(""),
                   static_cast<uint32_t>(0));
  } else {
    out.writeBytes((int8_t*)field, static_cast<uint32_t>(strlen(field)) + 1);
  }
}

/** this one allocates the memory and modifies field to point to it. */
void readCharStar(DataInput& in, char** field) {
  GF_D_ASSERT(*field == nullptr);
  int32_t memlen = in.readArrayLen();
  if (memlen != 0) {
    *field = new char[memlen];
    in.readBytesOnly(reinterpret_cast<int8_t*>(*field), memlen);
  }
}

}  // namespace impl
}  // namespace client
}  // namespace geode
}  // namespace apache

void RegionAttributes::toData(DataOutput& out) const {
  out.writeInt(static_cast<int32_t>(m_regionTimeToLive));
  out.writeInt(static_cast<int32_t>(m_regionTimeToLiveExpirationAction));
  out.writeInt(static_cast<int32_t>(m_regionIdleTimeout));
  out.writeInt(static_cast<int32_t>(m_regionIdleTimeoutExpirationAction));
  out.writeInt(static_cast<int32_t>(m_entryTimeToLive));
  out.writeInt(static_cast<int32_t>(m_entryTimeToLiveExpirationAction));
  out.writeInt(static_cast<int32_t>(m_entryIdleTimeout));
  out.writeInt(static_cast<int32_t>(m_entryIdleTimeoutExpirationAction));
  out.writeInt(static_cast<int32_t>(m_initialCapacity));
  out.writeFloat(m_loadFactor);
  out.writeInt(static_cast<int32_t>(m_maxValueDistLimit));
  out.writeInt(static_cast<int32_t>(m_concurrencyLevel));
  out.writeInt(static_cast<int32_t>(m_lruEntriesLimit));
  out.writeInt(static_cast<int32_t>(m_lruEvictionAction));

  apache::geode::client::impl::writeBool(out, m_caching);
  apache::geode::client::impl::writeBool(out, m_clientNotificationEnabled);

  apache::geode::client::impl::writeCharStar(out, m_cacheLoaderLibrary);
  apache::geode::client::impl::writeCharStar(out, m_cacheLoaderFactory);
  apache::geode::client::impl::writeCharStar(out, m_cacheWriterLibrary);
  apache::geode::client::impl::writeCharStar(out, m_cacheWriterFactory);
  apache::geode::client::impl::writeCharStar(out, m_cacheListenerLibrary);
  apache::geode::client::impl::writeCharStar(out, m_cacheListenerFactory);
  apache::geode::client::impl::writeCharStar(out, m_partitionResolverLibrary);
  apache::geode::client::impl::writeCharStar(out, m_partitionResolverFactory);
  out.writeInt(static_cast<int32_t>(m_diskPolicy));
  apache::geode::client::impl::writeCharStar(out, m_endpoints);
  apache::geode::client::impl::writeCharStar(out, m_persistenceLibrary);
  apache::geode::client::impl::writeCharStar(out, m_persistenceFactory);
  out.writeObject(m_persistenceProperties);
  apache::geode::client::impl::writeCharStar(out, m_poolName);
  apache::geode::client::impl::writeBool(out, m_isConcurrencyChecksEnabled);
}

void RegionAttributes::fromData(DataInput& in) {
  m_regionTimeToLive = in.readInt32();
  m_regionTimeToLiveExpirationAction = static_cast<ExpirationAction::Action>(in.readInt32());
  m_regionIdleTimeout = in.readInt32();
  m_regionIdleTimeoutExpirationAction = static_cast<ExpirationAction::Action>(in.readInt32());
  m_entryTimeToLive = in.readInt32();
  m_entryTimeToLiveExpirationAction = static_cast<ExpirationAction::Action>(in.readInt32());
  m_entryIdleTimeout = in.readInt32();
  m_entryIdleTimeoutExpirationAction = static_cast<ExpirationAction::Action>(in.readInt32());
  m_initialCapacity = in.readInt32();
  m_loadFactor = in.readFloat();
  m_maxValueDistLimit = in.readInt32();
  m_concurrencyLevel = in.readInt32();
  m_lruEntriesLimit = in.readInt32();
  m_lruEvictionAction = static_cast<ExpirationAction::Action>(in.readInt32());

  apache::geode::client::impl::readBool(in, &m_caching);
  apache::geode::client::impl::readBool(in, &m_clientNotificationEnabled);

  apache::geode::client::impl::readCharStar(in, &m_cacheLoaderLibrary);
  apache::geode::client::impl::readCharStar(in, &m_cacheLoaderFactory);
  apache::geode::client::impl::readCharStar(in, &m_cacheWriterLibrary);
  apache::geode::client::impl::readCharStar(in, &m_cacheWriterFactory);
  apache::geode::client::impl::readCharStar(in, &m_cacheListenerLibrary);
  apache::geode::client::impl::readCharStar(in, &m_cacheListenerFactory);
  apache::geode::client::impl::readCharStar(in, &m_partitionResolverLibrary);
  apache::geode::client::impl::readCharStar(in, &m_partitionResolverFactory);
  m_diskPolicy = static_cast<DiskPolicyType::PolicyType>(in.readInt32());
  apache::geode::client::impl::readCharStar(in, &m_endpoints);
  apache::geode::client::impl::readCharStar(in, &m_persistenceLibrary);
  apache::geode::client::impl::readCharStar(in, &m_persistenceFactory);
  m_persistenceProperties = in.readObject<Properties>(true);
  apache::geode::client::impl::readCharStar(in, &m_poolName);
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

  if (0 != compareStringAttribute(m_cacheLoaderLibrary,
                                  other.m_cacheLoaderLibrary)) {
    return false;
  }
  if (0 != compareStringAttribute(m_cacheLoaderFactory,
                                  other.m_cacheLoaderFactory)) {
    return false;
  }
  if (0 != compareStringAttribute(m_cacheWriterLibrary,
                                  other.m_cacheWriterLibrary)) {
    return false;
  }
  if (0 != compareStringAttribute(m_cacheWriterFactory,
                                  other.m_cacheWriterFactory)) {
    return false;
  }
  if (0 != compareStringAttribute(m_cacheListenerLibrary,
                                  other.m_cacheListenerLibrary)) {
    return false;
  }
  if (0 != compareStringAttribute(m_cacheListenerFactory,
                                  other.m_cacheListenerFactory)) {
    return false;
  }
  if (0 != compareStringAttribute(m_partitionResolverLibrary,
                                  other.m_partitionResolverLibrary)) {
    return false;
  }
  if (0 != compareStringAttribute(m_partitionResolverFactory,
                                  other.m_partitionResolverFactory)) {
    return false;
  }
  if (m_diskPolicy != other.m_diskPolicy) return false;
  if (0 != compareStringAttribute(m_endpoints, other.m_endpoints)) return false;
  if (0 != compareStringAttribute(m_persistenceLibrary,
                                  other.m_persistenceLibrary)) {
    return false;
  }
  if (0 != compareStringAttribute(m_persistenceFactory,
                                  other.m_persistenceFactory)) {
    return false;
  }
  if (m_isConcurrencyChecksEnabled != other.m_isConcurrencyChecksEnabled) {
    return false;
  }

  return true;
}

int32_t RegionAttributes::compareStringAttribute(char* attributeA,
                                                 char* attributeB) {
  if (attributeA == nullptr && attributeB == nullptr) {
    return 0;
  } else if (attributeA == nullptr && attributeB != nullptr) {
    return -1;
  } else if (attributeA != nullptr && attributeB == nullptr) {
    return -1;
  }
  return (strcmp(attributeA, attributeB));
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

void RegionAttributes::setCacheListener(const char* lib, const char* func) {
  GF_R_ASSERT(lib != nullptr);
  GF_R_ASSERT(func != nullptr);
  copyStringAttribute(m_cacheListenerLibrary, lib);
  copyStringAttribute(m_cacheListenerFactory, func);
}

void RegionAttributes::setPartitionResolver(const char* lib, const char* func) {
  GF_R_ASSERT(lib != nullptr);
  GF_R_ASSERT(func != nullptr);
  copyStringAttribute(m_partitionResolverLibrary, lib);
  copyStringAttribute(m_partitionResolverFactory, func);
}

void RegionAttributes::setCacheLoader(const char* lib, const char* func) {
  GF_R_ASSERT(lib != nullptr);
  GF_R_ASSERT(func != nullptr);
  copyStringAttribute(m_cacheLoaderLibrary, lib);
  copyStringAttribute(m_cacheLoaderFactory, func);
}

void RegionAttributes::setCacheWriter(const char* lib, const char* func) {
  GF_R_ASSERT(lib != nullptr);
  GF_R_ASSERT(func != nullptr);
  copyStringAttribute(m_cacheWriterLibrary, lib);
  copyStringAttribute(m_cacheWriterFactory, func);
}

void RegionAttributes::setPersistenceManager(const char* lib, const char* func,
                                             const PropertiesPtr& config) {
  GF_R_ASSERT(lib != nullptr);
  GF_R_ASSERT(func != nullptr);
  copyStringAttribute(m_persistenceLibrary, lib);
  copyStringAttribute(m_persistenceFactory, func);
  m_persistenceProperties = config;
}

void RegionAttributes::setEndpoints(const char* endpoints) {
  copyStringAttribute(m_endpoints, endpoints);
}

void RegionAttributes::setPoolName(const char* poolName) {
  copyStringAttribute(m_poolName, poolName);
}

void RegionAttributes::setCachingEnabled(bool enable) { m_caching = enable; }

void RegionAttributes::setLruEntriesLimit(int limit) {
  m_lruEntriesLimit = limit;
}
void RegionAttributes::setDiskPolicy(DiskPolicyType::PolicyType diskPolicy) {
  m_diskPolicy = diskPolicy;
}

void RegionAttributes::copyStringAttribute(char*& lhs, const char* rhs) {
  if (lhs != nullptr) {
    delete[] lhs;
  }
  lhs = Utils::copyString(rhs);
}

void RegionAttributes::setCloningEnabled(bool isClonable) {
  m_isClonable = isClonable;
}

void RegionAttributes::setConcurrencyChecksEnabled(bool enable) {
  m_isConcurrencyChecksEnabled = enable;
}
