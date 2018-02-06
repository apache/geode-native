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

#ifndef GEODE_REGIONATTRIBUTES_H_
#define GEODE_REGIONATTRIBUTES_H_

/**
 * @file
 */

#include <chrono>

#include "internal/geode_globals.hpp"
#include "CacheLoader.hpp"
#include "ExpirationAttributes.hpp"
#include "CacheWriter.hpp"
#include "CacheListener.hpp"
#include "PartitionResolver.hpp"
#include "Properties.hpp"
#include "Serializable.hpp"
#include "DiskPolicyType.hpp"
#include "PersistenceManager.hpp"
#include "internal/chrono/duration.hpp"

namespace apache {
namespace geode {
namespace client {

class CacheImpl;

/**
 * @class RegionAttributes RegionAttributes.hpp
 * Defines attributes for configuring a region.
 *
 * These are
 * <code>CacheListener</code>, <code>CacheLoader</code>,
 * <code>CacheWriter</code>,
 * scope expiration attributes
 * for the region itself; expiration attributes for the region entries;
 * and whether statistics are enabled for the region and its entries.
 *
 * To create an instance of this interface, use {@link
 * AttributesFactory::createRegionAttributes}.
 *
 * For compatibility rules and default values, see {@link AttributesFactory}.
 *
 * <p>Note that the <code>RegionAttributes</code> are not distributed with the
 * region.
 *
 * @see AttributesFactory
 * @see AttributesMutator
 * @see Region::getAttributes
 */
class AttributesFactory;
class AttributesMutator;
class Cache;
class Region;

class _GEODE_EXPORT RegionAttributes : public Serializable {
  /**
   * @brief public static methods
   */
 public:
  /** Gets the cache loader for the region.
   * @return  a pointer that points to the region's ,
   * <code>CacheLoader</code> , nullptr if there is no CacheLoader for this
   * region.
   */
  std::shared_ptr<CacheLoader> getCacheLoader();

  /** Gets the cache writer for the region.
   * @return  a pointer that points to the region's ,
   * <code>CacheWriter</code> , nullptr if there is no CacheWriter for this
   * region
   */
  std::shared_ptr<CacheWriter> getCacheWriter();

  /** Gets the cache listener for the region.
   * @return  a pointer that points to the region's ,
   * <code>CacheListener</code> , nullptr if there is no CacheListener defined
   * for this region.
   */
  std::shared_ptr<CacheListener> getCacheListener();

  /** Gets the partition resolver for the partition region.
   * @return  a pointer that points to the region's ,
   * <code>PartitionResolver</code> , nullptr if there is no PartitionResolver
   * defined
   * for this region.
   */
  std::shared_ptr<PartitionResolver> getPartitionResolver();

  /** Gets the <code>timeToLive</code> expiration attributes for the region as a
   * whole.
   * @return the timeToLive expiration attributes for this region
   */
  std::chrono::seconds getRegionTimeToLive() const;

  ExpirationAction getRegionTimeToLiveAction() const;

  /** Gets the idleTimeout expiration attributes for the region as a whole.
   *
   * @return the IdleTimeout expiration attributes for this region
   */
  std::chrono::seconds getRegionIdleTimeout() const;

  ExpirationAction getRegionIdleTimeoutAction() const;

  /** Gets the <code>timeToLive</code> expiration attributes for entries in this
   * region.
   * @return the timeToLive expiration attributes for entries in this region
   */
  std::chrono::seconds getEntryTimeToLive() const;

  ExpirationAction getEntryTimeToLiveAction() const;

  /** Gets the <code>idleTimeout</code> expiration attributes for entries in
   * this region.
   * @return the idleTimeout expiration attributes for entries in this region
   * @tparam Duration std::chrono::duration type to return
   */
  std::chrono::seconds getEntryIdleTimeout() const;

  ExpirationAction getEntryIdleTimeoutAction() const;

  /**
   * If true, this region will store data in the current process.
   * @return true or false, indicating cachingEnabled state.
   */
  inline bool getCachingEnabled() const { return m_caching; }

  // MAP ATTRIBUTES

  /** Returns the initial capacity of the entry's local cache.
   * @return the initial capacity of the entry's local cache
   */
  int getInitialCapacity() const;

  /** Returns the load factor of the entry's local cache.
   * @return the load factor of the entry's local cache
   */
  float getLoadFactor() const;

  /** Returns the concurrencyLevel of the entry's local cache.
   * @return the concurrencyLevel
   * @see AttributesFactory
   */
  uint8_t getConcurrencyLevel() const;

  /**
   * Returns the maximum number of entries this cache will hold before
   * using LRU eviction. A return value of zero, 0, indicates no limit.
   */
  uint32_t getLruEntriesLimit() const;

  /** Returns the disk policy type of the region.
   *
   * @return the <code>DiskPolicyType</code>, default is
   * DiskPolicyType::NONE.
   */
  DiskPolicyType getDiskPolicy() const;

  /**
   * Returns the ExpirationAction used for LRU Eviction, default is
   * LOCAL_DESTROY.
   */
  const ExpirationAction getLruEvictionAction() const;

  /**
   * Returns the name of the pool attached to the region.
   */
  const std::string& getPoolName() const;

  /*destructor
   *
   */
  virtual ~RegionAttributes();

  /** Serialize out to stream */
  virtual void toData(DataOutput& out) const;

  /** Initialize members from serialized data. */
  virtual void fromData(DataInput& in);

  /** Return an empty instance for deserialization. */
  static Serializable* createDeserializable();

  /** Return class id for serialization. */
  virtual int32_t classId() const;

  /** Return type id for serialization. */
  virtual int8_t typeId() const;

  // return zero deliberately
  virtual size_t objectSize() const { return 0; }

  /**
   * This method returns the path of the library from which
   * the factory function will be invoked on a cache server.
   */
  const std::string& getCacheLoaderLibrary();

  /**
   * This method returns the symbol name of the factory function from which
   * the loader will be created on a cache server.
   */
  const std::string& getCacheLoaderFactory();

  /**
   * This method returns the path of the library from which
   * the factory function will be invoked on a cache server.
   */
  const std::string& getCacheListenerLibrary();

  /**
   * This method returns the symbol name of the factory function from which
   * the loader will be created on a cache server.
   */
  const std::string& getCacheListenerFactory();

  /**
   * This method returns the path of the library from which
   * the factory function will be invoked on a cache server.
   */
  const std::string& getCacheWriterLibrary();

  /**
   * This method returns the symbol name of the factory function from which
   * the loader will be created on a cache server.
   */
  const std::string& getCacheWriterFactory();

  /**
   * This method returns the path of the library from which
   * the factory function will be invoked on a cache server.
   */
  const std::string& getPartitionResolverLibrary();

  /**
   * This method returns the symbol name of the factory function from which
   * the loader will be created on a cache server.
   */
  const std::string& getPartitionResolverFactory();

  /** Return true if all the attributes are equal to those of other. */
  bool operator==(const RegionAttributes& other) const;

  /** Return true if any of the attributes are not equal to those of other. */
  bool operator!=(const RegionAttributes& other) const;

  /** throws IllegalStateException if the attributes are not suited for
   * serialization
   * such as those that have a cache callback (listener, loader, or writer) set
   * directly instead of through the string value setters.
   */
  void validateSerializableAttributes();

  /**
   * This method returns the list of servername:portno separated by comma
   */
  const std::string& getEndpoints();

  /**
   * This method returns the setting of client notification
   */
  bool getClientNotificationEnabled() const;

  /**
   * This method returns the path of the library from which
   * the factory function will be invoked on a cache server.
   */
  const std::string& getPersistenceLibrary();

  /**
   * This method returns the symbol name of the factory function from which
   * the persistence will be created on a cache server.
   */
  const std::string& getPersistenceFactory();

  /**
   * This method returns the properties pointer which is set for persistence.
   */
  std::shared_ptr<Properties> getPersistenceProperties();

  /** Gets the persistence for the region.
   * @return  a pointer that points to the region's ,
   * <code>PersistenceManager</code> , nullptr if there is no PersistenceManager
   * for this
   * region.
   */
  std::shared_ptr<PersistenceManager> getPersistenceManager();

  /** TODO
   * Returns the name of the {@link Pool} that this region
   * will use to communicate with servers, if any.
   * @return the name of the client-server {@link Pool}
   */
  const std::string& getPoolName() { return m_poolName; }
  bool getCloningEnabled() { return m_isClonable; }

  /**
   * Returns true if concurrent update checks are turned on for this region.
   * <p>
   * @return true if concurrent update checks are turned on
   */
  bool getConcurrencyChecksEnabled() { return m_isConcurrencyChecksEnabled; }
  const RegionAttributes& operator=(const RegionAttributes&) = delete;

 private:
  void setCacheListener(const std::string& libpath,
                        const std::string& factoryFuncName);
  void setCacheLoader(const std::string& libpath,
                      const std::string& factoryFuncName);
  void setCacheWriter(const std::string& libpath,
                      const std::string& factoryFuncName);
  void setPartitionResolver(const std::string& libpath,
                            const std::string& factoryFuncName);
  void setPersistenceManager(const std::string& lib, const std::string& func,
                             const std::shared_ptr<Properties>& config);
  void setEndpoints(const std::string& endpoints);
  void setPoolName(const std::string& poolName);
  void setCloningEnabled(bool isClonable);
  void setCachingEnabled(bool enable);
  void setLruEntriesLimit(int limit);
  void setDiskPolicy(DiskPolicyType diskPolicy);
  void setConcurrencyChecksEnabled(bool enable);

  inline bool getEntryExpiryEnabled() const {
    return (m_entryTimeToLive > std::chrono::seconds::zero() ||
            m_entryIdleTimeout > std::chrono::seconds::zero());
  }

  inline bool getRegionExpiryEnabled() const {
    return (m_regionTimeToLive > std::chrono::seconds::zero() ||
            m_regionIdleTimeout > std::chrono::seconds::zero());
  }

  // will be created by the factory
  RegionAttributes(const RegionAttributes& rhs);
  RegionAttributes();

  ExpirationAction m_regionTimeToLiveExpirationAction;
  ExpirationAction m_regionIdleTimeoutExpirationAction;
  ExpirationAction m_entryTimeToLiveExpirationAction;
  ExpirationAction m_entryIdleTimeoutExpirationAction;
  ExpirationAction m_lruEvictionAction;
  std::shared_ptr<CacheWriter> m_cacheWriter;
  std::shared_ptr<CacheLoader> m_cacheLoader;
  std::shared_ptr<CacheListener> m_cacheListener;
  std::shared_ptr<PartitionResolver> m_partitionResolver;
  uint32_t m_lruEntriesLimit;
  bool m_caching;
  uint32_t m_maxValueDistLimit;
  std::chrono::seconds m_entryIdleTimeout;
  std::chrono::seconds m_entryTimeToLive;
  std::chrono::seconds m_regionIdleTimeout;
  std::chrono::seconds m_regionTimeToLive;
  uint32_t m_initialCapacity;
  float m_loadFactor;
  uint8_t m_concurrencyLevel;
  std::string m_cacheLoaderLibrary;
  std::string m_cacheWriterLibrary;
  std::string m_cacheListenerLibrary;
  std::string m_partitionResolverLibrary;
  std::string m_cacheLoaderFactory;
  std::string m_cacheWriterFactory;
  std::string m_cacheListenerFactory;
  std::string m_partitionResolverFactory;
  DiskPolicyType m_diskPolicy;
  std::string m_endpoints;
  bool m_clientNotificationEnabled;
  std::string m_persistenceLibrary;
  std::string m_persistenceFactory;
  std::shared_ptr<Properties> m_persistenceProperties;
  std::shared_ptr<PersistenceManager> m_persistenceManager;
  std::string m_poolName;
  bool m_isClonable;
  bool m_isConcurrencyChecksEnabled;
  friend class AttributesFactory;
  friend class AttributesMutator;
  friend class Cache;
  friend class CacheImpl;
  friend class Region;
  friend class RegionInternal;
  friend class RegionXmlCreation;

 private:
  _GEODE_FRIEND_STD_SHARED_PTR(RegionAttributes)
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_REGIONATTRIBUTES_H_
