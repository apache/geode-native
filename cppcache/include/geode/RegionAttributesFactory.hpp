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

#ifndef GEODE_ATTRIBUTESFACTORY_H_
#define GEODE_ATTRIBUTESFACTORY_H_

#include <chrono>
#include <string>

#include "CacheListener.hpp"
#include "CacheLoader.hpp"
#include "CacheWriter.hpp"
#include "DiskPolicyType.hpp"
#include "ExceptionTypes.hpp"
#include "ExpirationAction.hpp"
#include "PartitionResolver.hpp"
#include "Pool.hpp"
#include "RegionAttributes.hpp"
#include "internal/chrono/duration.hpp"
#include "internal/geode_globals.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

/**
 * Creates instances of {@link RegionAttributes}. An
 * <code>RegionAttributesFactory</code>
 * instance maintains state for creating <code>RegionAttributes</code>
 * instances.
 * The setter methods are used to change the settings that will be used for
 * creating the next attributes instance with the {@link
 * #create}
 * method. If you create a factory with the default constructor, then the
 * factory is set up to create attributes with all default settings. You can
 * also create a factory by providing a <code>RegionAttributes</code>, which
 * will set up the new factory with the settings provided in that attributes
 * instance.
 *
 * <p>Once a <code>RegionAttributes</code> is created, it can only be modified
 * after it has been used to create a <code>Region</code>, and then only by
 * using an {@link AttributesMutator} obtained from the region.
 *
 * <h3>Attributes</h3>
 * <h4>Callbacks</h4>
 * <dl>
 * <dt>{@link CacheLoader} [<em>default:</em> nullptr]</dt>
 *     <dd>User-implemented plug-in for loading data on cache misses.<br>
 *        {@link #setCacheLoader} {@link RegionAttributes#getCacheLoader}
 *        {@link AttributesMutator#setCacheLoader}</dd>
 *
 * <dt>{@link CacheWriter} [<em>default:</em> nullptr]</dt>
 *     <dd>User-implemented plug-in for intercepting cache modifications, e.g.
 *         for writing to an external data source.<br>
 *         {@link #setCacheWriter} {@link RegionAttributes#getCacheWriter}
 *         {@link AttributesMutator#setCacheWriter}</dd>
 *
 * <dt>{@link CacheListener} [<em>default:</em> nullptr]</dt>
 *     <dd>User-implemented plug-in for receiving and handling cache related
 * events.<br>
 *         {@link #setCacheListener} {@link RegionAttributes#getCacheListener}
 *         {@link AttributesMutator#setCacheListener}</dd>
 * </dl>
 * <h4>Expiration</h4>
 * <dl>
 * <dt>RegionTimeToLive [<em>default:</em> no expiration]</dt>
 *     <dd>Expiration configuration for the entire region based on the
 *     {@link CacheStatistics#getLastModifiedTime lastModifiedTime}.<br>
 *         {@link #setRegionTimeToLive} {@link
 * RegionAttributes#getRegionTimeToLive}
 *         {@link AttributesMutator#setRegionTimeToLive}</dd>
 *
 * <dt>RegionIdleTimeout [<em>default:</em> no expiration]</dt>
 *     <dd>Expiration configuration for the entire region based on the
 *         {@link CacheStatistics#getLastAccessedTime lastAccessedTime}.<br>
 *         {@link #setRegionIdleTimeout} {@link
 * RegionAttributes#getRegionIdleTimeout}
 *         {@link AttributesMutator#setRegionIdleTimeout}</dd>
 *
 * <dt>EntryTimeToLive [<em>default:</em> no expiration]</dt>
 *     <dd>Expiration configuration for individual entries based on the
 *     {@link CacheStatistics#getLastModifiedTime lastModifiedTime}.<br>
 *         {@link #setEntryTimeToLive} {@link
 * RegionAttributes#getEntryTimeToLive}
 *         {@link AttributesMutator#setEntryTimeToLive}</dd>
 *
 * <dt>EntryIdleTimeout [<em>default:</em> no expiration]</dt>
 *     <dd>Expiration configuration for individual entries based on the
 *         {@link CacheStatistics#getLastAccessedTime lastAccessedTime}.<br>
 *         {@link #setEntryIdleTimeout} {@link
 * RegionAttributes#getEntryIdleTimeout}
 *         {@link AttributesMutator#setEntryIdleTimeout}</dd>
 * </dl>
 * <h4>Storage</h4>
 * <dl>
 *
 * <dt>InitialCapacity [<em>default:</em> <code>16</code>]</dt>
 *     <dd>The initial capacity of the map used for storing the entries.<br>
 *         {@link RegionAttributes#getInitialCapacity}</dd>
 *
 * <dt>LoadFactor [<em>default:</em> <code>0.75</code>]</dt>
 *     <dd>The load factor of the map used for storing the entries.<br>
 *         {@link RegionAttributes#getLoadFactor}</dd>
 *
 * <dt>ConcurrencyLevel [<em>default:</em> <code>16</code>]</dt>
 *     <dd>The allowed concurrency among updates to values in the region
 *         is guided by the <tt>concurrencyLevel</tt>, which is used as a hint
 *         for internal sizing. The actual concurrency will vary.
 *         Ideally, you should choose a value to accommodate as many
 *         threads as will ever concurrently modify values in the region. Using
 * a
 *         significantly higher value than you need can waste space and time,
 *         and a significantly lower value can lead to thread contention. But
 *         overestimates and underestimates within an order of magnitude do
 *         not usually have much noticeable impact. A value of one is
 *         appropriate when it is known that only one thread will modify
 *         and all others will only read.<br>
 *         {@link #setConcurrencyLevel} {@link
 * RegionAttributes#getConcurrencyLevel}</dd>
 *
 * <dt>StatisticsEnabled [<em>default:</em> <code>false</code>]</dt>
 *     <dd>Whether statistics are enabled for this region. The default
 *     is disabled, which conserves on memory.<br>
 *     {@link #setStatisticsEnabled} {@link
 * RegionAttributes#getStatisticsEnabled}</dd>
 *
 *
 * </dl>
 *
 *
 * @see RegionAttributes
 * @see AttributesMutator
 * @see Region#createSubregion(String, RegionAttributes)
 */

class APACHE_GEODE_EXPORT RegionAttributesFactory {
  /**
   * @brief public methods
   */
 public:
  /**
   *@brief constructor
   */

  /**
   * Creates a new instance of RegionAttributesFactory ready to create a
   *       <code>RegionAttributes</code> with default settings.
   */
  RegionAttributesFactory();

  /**
   * Creates a new instance of RegionAttributesFactory ready to create a
   *  <code>RegionAttributes</code> with the same settings as those in the
   *  specified <code>RegionAttributes</code>.
   * @param regionAttributes the <code>RegionAttributes</code> used to
   * initialize this RegionAttributesFactory
   */
  explicit RegionAttributesFactory(const RegionAttributes regionAttributes);

  RegionAttributesFactory(const RegionAttributesFactory&) = default;

  /**
   *@brief destructor
   */
  virtual ~RegionAttributesFactory();

  // CALLBACKS

  /**
   * Sets the cache loader for the next <code>RegionAttributes</code> created.
   * @param cacheLoader the cache loader or nullptr if no loader
   * @return a reference to <code>this</code>
   */
  RegionAttributesFactory& setCacheLoader(
      const std::shared_ptr<CacheLoader>& cacheLoader);

  /**
   * Sets the cache writer for the next <code>RegionAttributes</code> created.
   * @param cacheWriter the cache writer or nullptr if no cache writer
   * @return a reference to <code>this</code>
   */
  RegionAttributesFactory& setCacheWriter(
      const std::shared_ptr<CacheWriter>& cacheWriter);

  /**
   * Sets the CacheListener for the next <code>RegionAttributes</code> created.
   * @param aListener a user defined CacheListener, nullptr if no listener
   * @return a reference to <code>this</code>
   */
  RegionAttributesFactory& setCacheListener(
      const std::shared_ptr<CacheListener>& aListener);

  /**
   * Sets the PartitionResolver for the next <code>RegionAttributes</code>
   * created.
   * @param aResolver a user defined PartitionResolver, nullptr if no resolver
   * @return a reference to <code>this</code>
   */
  RegionAttributesFactory& setPartitionResolver(
      const std::shared_ptr<PartitionResolver>& aResolver);

  /**
   * Sets the library path for the library that will be invoked for the loader
   * of the region.
   * @return a reference to <code>this</code>
   */
  RegionAttributesFactory& setCacheLoader(const std::string& libpath,
                                          const std::string& factoryFuncName);

  /**
   * Sets the library path for the library that will be invoked for the writer
   * of the region.
   * @return a reference to <code>this</code>
   */

  RegionAttributesFactory& setCacheWriter(const std::string& libpath,
                                          const std::string& factoryFuncName);

  /**
   * Sets the library path for the library that will be invoked for the listener
   * of the region.
   * @return a reference to <code>this</code>
   */
  RegionAttributesFactory& setCacheListener(const std::string& libpath,
                                            const std::string& factoryFuncName);

  /**
   * Sets the library path for the library that will be invoked for the
   * partition resolver of the region.
   * @return a reference to <code>this</code>
   */
  RegionAttributesFactory& setPartitionResolver(
      const std::string& libpath, const std::string& factoryFuncName);

  // EXPIRATION ATTRIBUTES

  /**
   * Sets the idleTimeout expiration attributes for region entries for the next
   * <code>RegionAttributes</code> created. Will expire in no less than
   * <code>idleTimeout</code>. Actual time may be longer depending on clock
   * resolution.
   *
   * @param action the expiration action for entries in this region.
   * @param idleTimeout the idleTimeout for entries in this region.
   * @return a reference to <code>this</code>
   */
  RegionAttributesFactory& setEntryIdleTimeout(
      ExpirationAction action, std::chrono::seconds idleTimeout);

  /**
   * Sets the timeToLive expiration attributes for region entries for the next
   * <code>RegionAttributes</code> created. Will expire in no less than
   * <code>timeToLive</code>, actual time may be longer depending on clock
   * resolution.
   *
   * @param action the expiration action for entries in this region.
   * @param timeToLive the timeToLive for entries in this region.
   * @return a reference to <code>this</code>
   */
  RegionAttributesFactory& setEntryTimeToLive(ExpirationAction action,
                                              std::chrono::seconds timeToLive);

  /**
   * Sets the idleTimeout expiration attributes for the region itself for the
   * next <code>RegionAttributes</code> created. Will expire in no less than
   * <code>idleTimeout</code>, actual time may be longer depending on clock
   * resolution.
   *
   * @param action the expiration action for entries in this region.
   * @param idleTimeout the idleTimeout for the region as a whole.
   * @return a reference to <code>this</code>
   */
  RegionAttributesFactory& setRegionIdleTimeout(
      ExpirationAction action, std::chrono::seconds idleTimeout);

  /**
   * Sets the timeToLive expiration attributes for the region itself for the
   * next <code>RegionAttributes</code> created. Will expire in no less than
   * <code>timeToLive</code>, actual time may be longer depending on clock
   * resolution.
   *
   * @param action the expiration action for entries in this region.
   * @param timeToLive the timeToLive for the region as a whole.
   * @return a reference to <code>this</code>
   */
  RegionAttributesFactory& setRegionTimeToLive(ExpirationAction action,
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
  RegionAttributesFactory& setPersistenceManager(
      const std::string& libpath, const std::string& factoryFuncName,
      const std::shared_ptr<Properties>& config = nullptr);

  /** Sets the PersistenceManager for the next <code>RegionAttributes</code>
   * created.
   * @param persistenceManager a user defined PersistenceManager, nullptr if no
   * resolver
   * @return a reference to <code>this</code>
   */
  RegionAttributesFactory& setPersistenceManager(
      const std::shared_ptr<PersistenceManager>& persistenceManager,
      const std::shared_ptr<Properties>& config = nullptr);

  // MAP ATTRIBUTES
  /**
   * Sets the entry initial capacity for the next <code>RegionAttributes</code>
   * created. This value
   * is used in initializing the map that holds the entries.
   * @param initialCapacity the initial capacity of the entry map
   * @return a reference to <code>this</code>
   * @throws IllegalArgumentException if initialCapacity is negative.
   */
  RegionAttributesFactory& setInitialCapacity(int32_t initialCapacity);

  /**
   * Sets the entry load factor for the next <code>RegionAttributes</code>
   * created. This value is
   * used in initializing the map that holds the entries.
   * @param loadFactor the load factor of the entry map
   * @return a reference to <code>this</code>
   * @throws IllegalArgumentException if loadFactor is nonpositive
   */
  RegionAttributesFactory& setLoadFactor(float loadFactor);

  /**
   * Sets the concurrency level tof the next <code>RegionAttributes</code>
   * created. This value is used in initializing the map that holds the entries.
   * @param concurrencyLevel the concurrency level of the entry map
   * @return a reference to <code>this</code>
   * @throws IllegalArgumentException if concurrencyLevel is nonpositive
   */
  RegionAttributesFactory& setConcurrencyLevel(uint8_t concurrencyLevel);

  /**
   * Sets a limit on the number of entries that will be held in the cache.
   * If a new entry is added while at the limit, the cache will evict the
   * least recently used entry. Defaults to 0, meaning no LRU actions will
   * used.
   * @return a reference to <code>this</code>
   */
  RegionAttributesFactory& setLruEntriesLimit(const uint32_t entriesLimit);

  /**
   * Sets the Disk policy type for the next <code>RegionAttributes</code>
   * created.
   * @param diskPolicy the type of disk policy to use for the region
   * @return a reference to <code>this</code>
   * @throws IllegalArgumentException if diskPolicyType is Invalid
   */
  RegionAttributesFactory& setDiskPolicy(const DiskPolicyType diskPolicy);

  /**
   * Set caching enabled flag for this region. If set to false, then no data is
   * stored
   * in the local process, but events and distributions will still occur, and
   * the region can still be used to put and remove, etc...
   * The default if not set is 'true'.
   * This also requires that interestLists are turned off for the region.
   * @param cachingEnabled if true, cache data for this region in this process.
   * @return a reference to <code>this</code>
   */
  RegionAttributesFactory& setCachingEnabled(bool cachingEnabled);

  /**
   * Sets the pool name attribute.
   * This causes regions that use these attributes
   * to be a client region which communicates with the
   * servers that the connection pool communicates with.
   * <p>If this attribute is set to <code>null</code> or <code>""</code>
   * then the connection pool is disabled causing regions that use these
   * attributes
   * to be communicate with peers instead of servers.
   * <p>The named connection pool must exist on the cache at the time these
   * attributes are used to create a region. See {@link
   * PoolManager#createFactory}
   * for how to create a connection pool.
   * @return a reference to <code>this</code>
   * @param name the name of the connection pool to use; if <code>null</code>
   * or <code>""</code> then the connection pool is disabled for regions
   * using these attributes.
   */
  RegionAttributesFactory& setPoolName(const std::string& name);

  /**
   * Sets cloning on region
   * @param isClonable whether region is clonable or not
   * @return a reference to <code>this</code>
   * @see RegionAttributes#getCloningEnabled()
   */
  RegionAttributesFactory& setCloningEnabled(bool isClonable);

  /**
   * Enables or disables concurrent modification checks
   * @since 7.0
   * @param concurrencyChecksEnabled whether to perform concurrency checks on
   * operations
   * @return a reference to <code>this</code>
   */
  RegionAttributesFactory& setConcurrencyChecksEnabled(
      bool concurrencyChecksEnabled);

  // FACTORY METHOD

  /**
   * Creates a <code>RegionAttributes</code> with the current settings.
   * @return the newly created <code>RegionAttributes</code>
   * @throws IllegalStateException if the current settings violate the
   * compatibility rules
   * @return a reference to <code>this</code>
   */
  RegionAttributes create();

 private:
  RegionAttributesFactory& operator=(const RegionAttributesFactory& other) =
      default;
  RegionAttributes m_regionAttributes;
  static void validateAttributes(RegionAttributes& attrs);
};  // namespace client

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_ATTRIBUTESFACTORY_H_
