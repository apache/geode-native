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

#ifndef GEODE_REGIONFACTORY_H_
#define GEODE_REGIONFACTORY_H_

#include <chrono>

#include "RegionAttributesFactory.hpp"
#include "internal/chrono/duration.hpp"
#include "internal/geode_globals.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {
class CacheImpl;

/**
 * @class RegionFactory RegionFactory.hpp
 *
 * Provides for the configuration and creation of instances of Region.
 */
class APACHE_GEODE_EXPORT RegionFactory {
 public:
  RegionFactory() = delete;
  ~RegionFactory() = default;
  RegionFactory(const RegionFactory& nocopy) = delete;
  RegionFactory(RegionFactory&& move) = default;

  /**
   * Create a {@link Region} of the given <code>name</code>.
   * @param name
   *        the name of the Region.
   * @throws RegionExistsException if a region is already in
   * this cache
   * @throws CacheClosedException if the cache is closed
   */
  std::shared_ptr<Region> create(std::string name);

  /** Sets the cache loader for the next <code>RegionAttributes</code> created.
   * @param cacheLoader the cache loader or nullptr if no loader
   * @return a reference to <code>this</code>
   */
  RegionFactory& setCacheLoader(
      const std::shared_ptr<CacheLoader>& cacheLoader);

  /** Sets the cache writer for the next <code>RegionAttributes</code> created.
   * @param cacheWriter the cache writer or nullptr if no cache writer
   * @return a reference to <code>this</code>
   */
  RegionFactory& setCacheWriter(
      const std::shared_ptr<CacheWriter>& cacheWriter);

  /** Sets the CacheListener for the next <code>RegionAttributes</code> created.
   * @param aListener a user defined CacheListener, nullptr if no listener
   * @return a reference to <code>this</code>
   */
  RegionFactory& setCacheListener(
      const std::shared_ptr<CacheListener>& aListener);

  /** Sets the PartitionResolver for the next <code>RegionAttributes</code>
   * created.
   * @param aResolver a user defined PartitionResolver, nullptr if no resolver
   * @return a reference to <code>this</code>
   */
  RegionFactory& setPartitionResolver(
      const std::shared_ptr<PartitionResolver>& aResolver);

  /**
   * Sets the library path for the library that will be invoked for the loader
   * of the region.
   * @return a reference to <code>this</code>
   */
  RegionFactory& setCacheLoader(const std::string& libpath,
                                const std::string& factoryFuncName);

  /**
   * Sets the library path for the library that will be invoked for the writer
   * of the region.
   * @return a reference to <code>this</code>
   */
  RegionFactory& setCacheWriter(const std::string& libpath,
                                const std::string& factoryFuncName);

  /**
   * Sets the library path for the library that will be invoked for the listener
   * of the region.
   * @return a reference to <code>this</code>
   */
  RegionFactory& setCacheListener(const std::string& libpath,
                                  const std::string& factoryFuncName);

  /**
   * Sets the library path for the library that will be invoked for the
   * partition resolver of the region.
   * @return a reference to <code>this</code>
   */
  RegionFactory& setPartitionResolver(const std::string& libpath,
                                      const std::string& factoryFuncName);

  // EXPIRATION ATTRIBUTES

  /** Sets the idleTimeout expiration attributes for region entries for the next
   * <code>RegionAttributes</code> created.
   * @param action the expiration action for entries in this region.
   * @param idleTimeout the idleTimeout for entries in this region.
   * @return a reference to <code>this</code>
   */
  RegionFactory& setEntryIdleTimeout(ExpirationAction action,
                                     std::chrono::seconds idleTimeout);

  /** Sets the timeToLive expiration attributes for region entries for the next
   * <code>RegionAttributes</code> created.
   * @param action the expiration action for entries in this region.
   * @param timeToLive the timeToLive for entries in this region.
   * @return a reference to <code>this</code>
   */
  RegionFactory& setEntryTimeToLive(ExpirationAction action,
                                    std::chrono::seconds timeToLive);

  /** Sets the idleTimeout expiration attributes for the region itself for the
   * next <code>RegionAttributes</code> created.
   * @param action the expiration action for entries in this region.
   * @param idleTimeout the idleTimeout for the region as a whole.
   * @return a reference to <code>this</code>
   */
  RegionFactory& setRegionIdleTimeout(ExpirationAction action,
                                      std::chrono::seconds idleTimeout);

  /** Sets the timeToLive expiration attributes for the region itself for the
   * next <code>RegionAttributes</code> created.
   * @param action the expiration action for entries in this region.
   * @param timeToLive the timeToLive for the region as a whole.
   * @return a reference to <code>this</code>
   */
  RegionFactory& setRegionTimeToLive(ExpirationAction action,
                                     std::chrono::seconds timeToLive);

  // PERSISTENCE
  /**
   * Sets the library path for the library that will be invoked for the
   * persistence of the region.
   * If the region is being created from a client on a server, or on a server
   * directly, then
   * this must be used to set the PersistenceManager.
   * @return a reference to <code>this</code>
   */
  RegionFactory& setPersistenceManager(
      const std::string& libpath, const std::string& factoryFuncName,
      const std::shared_ptr<Properties>& config = nullptr);

  /** Sets the PersistenceManager for the next <code>RegionAttributes</code>
   * created.
   * @param persistenceManager a user defined PersistenceManager, nullptr if no
   * persistenceManager
   * @return a reference to <code>this</code>
   */
  RegionFactory& setPersistenceManager(
      const std::shared_ptr<PersistenceManager>& persistenceManager,
      const std::shared_ptr<Properties>& config = nullptr);

  // MAP ATTRIBUTES
  /** Sets the entry initial capacity for the next <code>RegionAttributes</code>
   * created. This value
   * is used in initializing the map that holds the entries.
   * @param initialCapacity the initial capacity of the entry map
   * @return a reference to <code>this</code>
   * @throws IllegalArgumentException if initialCapacity is negative.
   */
  RegionFactory& setInitialCapacity(int initialCapacity);

  /** Sets the entry load factor for the next <code>RegionAttributes</code>
   * created. This value is
   * used in initializing the map that holds the entries.
   * @param loadFactor the load factor of the entry map
   * @return a reference to <code>this</code>
   * @throws IllegalArgumentException if loadFactor is nonpositive
   */
  RegionFactory& setLoadFactor(float loadFactor);

  /** Sets the concurrency level tof the next <code>RegionAttributes</code>
   * created. This value is used in initializing the map that holds the entries.
   * @param concurrencyLevel the concurrency level of the entry map
   * @return a reference to <code>this</code>
   * @throws IllegalArgumentException if concurrencyLevel is nonpositive
   */
  RegionFactory& setConcurrencyLevel(uint8_t concurrencyLevel);

  /**
   * Sets a limit on the number of entries that will be held in the cache.
   * If a new entry is added while at the limit, the cache will evict the
   * least recently used entry. Defaults to 0, meaning no LRU actions will
   * used.
   * @param entriesLimit number of enteries to keep in region
   * @return a reference to <code>this</code>
   */
  RegionFactory& setLruEntriesLimit(const uint32_t entriesLimit);

  /** Sets the Disk policy type for the next <code>RegionAttributes</code>
   * created.
   * @param diskPolicy the type of disk policy to use for the region
   * @return a reference to <code>this</code>
   * @throws IllegalArgumentException if diskPolicyType is Invalid
   */
  RegionFactory& setDiskPolicy(const DiskPolicyType diskPolicy);

  /**
   * Set caching enabled flag for this region. If set to false, then no data is
   * stored
   * in the local process, but events and distributions will still occur, and
   * the region can still be used to put and remove, etc...
   * The default if not set is 'true', 'false' is illegal for regions of 'local'
   * scope.
   * This also requires that interestLists are turned off for the region.
   * @param cachingEnabled if true, cache data for this region in this process.
   * @return a reference to <code>this</code>
   */
  RegionFactory& setCachingEnabled(bool cachingEnabled);

  /*
   * Set the PoolName to attach the Region with that Pool.
   * Use only when Cache ha more than one Pool
   * @param name
   *        the name of the Pool to which region will be attached.
   * @return a reference to <code>this</code>
   */
  RegionFactory& setPoolName(const std::string& name);

  /*
   * Set boolean to enable/disable cloning while applying delta.
   * @param isClonable whether to enable cloning or not.
   * @return a reference to <code>this</code>
   */
  RegionFactory& setCloningEnabled(bool isClonable);

  /**
   * Enables or disables concurrent modification checks
   * @since 7.0
   * @param enable whether to perform concurrency checks on
   * operations
   * @return a reference to <code>this</code>
   */
  RegionFactory& setConcurrencyChecksEnabled(bool enable);

 private:
  RegionFactory(apache::geode::client::RegionShortcut preDefinedRegion,
                CacheImpl* cacheImpl);

  void setRegionShortcut();

  RegionShortcut m_preDefinedRegion;

  std::shared_ptr<RegionAttributesFactory> m_regionAttributesFactory;

  CacheImpl* m_cacheImpl;

  friend class CacheImpl;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_REGIONFACTORY_H_
