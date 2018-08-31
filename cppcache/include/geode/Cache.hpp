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

#ifndef GEODE_CACHE_H_
#define GEODE_CACHE_H_

#include <string>

#include "internal/geode_globals.hpp"
#include "GeodeCache.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

class AuthenticatedView;
class AuthInitialize;
class CacheFactory;
class CacheImpl;
class CacheRegionHelper;
class CacheTransactionManager;
class DataInput;
class DataOutput;
class Pool;
class PoolFactory;
class PoolManager;
class Properties;
class QueryService;
class Region;
class RegionFactory;
class TypeRegistry;
enum class RegionShortcut;

/**
 * @class Cache Cache.hpp
 *
 * Caches are obtained from the create method on the {@link CacheFactory#create}
 * class
 * <p>
 * When a cache will no longer be used, it should be {@link #close closed}.
 * Once it {@link Cache::isClosed is closed} any attempt to use it
 * will cause a <code>CacheClosedException</code> to be thrown.
 *
 * <p>A cache can have multiple root regions, each with a different name.
 *
 */
class APACHE_GEODE_EXPORT Cache : public GeodeCache {
  /**
   * @brief public methods
   */
 public:
  /**
   * Returns the {@link RegionFactory} to create the region.
   * Before creating the Region, one can set region attributes using this
   * instance.
   *
   * @param regionShortcut
   *        To create the region specific type, @see RegionShortcut
   */
  virtual RegionFactory createRegionFactory(RegionShortcut regionShortcut);

  /**
   * Initializes the cache from an xml file
   *
   * @param cacheXml
   *        Valid cache.xml file
   */
  void initializeDeclarativeCache(const std::string& cacheXml) override;

  /** Returns the name of this cache.
   * @return the string name of this cache
   */
  const std::string& getName() const override;

  /**
   * Indicates if this cache has been closed.
   * After a new cache object is created, this method returns false;
   * After the close is called on this cache object, this method
   * returns true.
   *
   * @return true, if this cache is closed; false, otherwise
   */
  bool isClosed() const override;

  /**
   * Returns the type registry that this cache was
   * {@link CacheFactory::create created} with.
   */
  TypeRegistry& getTypeRegistry() const;

  /**
   * Terminates this object cache and releases all the local resources.
   * After this cache is closed, any further
   * method call on this cache or any region object will throw
   * <code>CacheClosedException</code>, unless otherwise noted.
   * If Cache instance created from Pool(pool is in multiuser mode), then it
   * reset user related security data.
   * @throws CacheClosedException,  if the cache is already closed.
   */
  void close() override;

  /**
   * Terminates this object cache and releases all the local resources.
   * After this cache is closed, any further
   * method call on this cache or any region object will throw
   * <code>CacheClosedException</code>, unless otherwise noted.
   * If Cache instance created from Pool(pool is in multiuser mode), then it
   * reset user related security data.
   * @param keepalive whether to keep a durable client's queue alive
   * @throws CacheClosedException,  if the cache is already closed.
   */
  virtual void close(bool keepalive);

  /** Look up a region with the full path from root.
   *
   * If Pool attached with Region is in multiusersecure mode then don't use
   * return instance of region as no credential are attached with this instance.
   * Get region from RegionService instance of Cache.@see
   * Cache#createAuthenticatedView(std::shared_ptr<Properties>).
   *
   * @param path the region's name, such as <code>AuthRegion</code>.
   * @returns region, or nullptr if no such region exists.
   */
  std::shared_ptr<Region> getRegion(const std::string& path) const override;

  /**
   * Returns a set of root regions in the cache. This set is a snapshot and
   * is not backed by the Cache. The vector passed in is cleared and the
   * regions are added to it.
   *
   * @param regions the returned set of
   * regions
   */
  std::vector<std::shared_ptr<Region>> rootRegions() const override;

  /**
   * Gets the QueryService from which a new Query can be obtained.
   * @returns A smart pointer to the QueryService.
   */
  std::shared_ptr<QueryService> getQueryService() override;

  /**
   * Gets the QueryService from which a new Query can be obtained.
   * @param poolName
   *        Pass poolname if pool is created from cache.xml or {@link
   * PoolManager}
   * @returns A smart pointer to the QueryService.
   */
  virtual std::shared_ptr<QueryService> getQueryService(
      const std::string& poolName) const;

  /**
   * Send the "client ready" message to the server from a durable client.
   */
  virtual void readyForEvents();

  /**
   * Creates an authenticated cache using the given user security properties.
   * Multiple instances with different user properties can be created with a
   * single client cache.
   *
   * Application must use this instance to do operations, when
   * multiuser-authentication is set to true.
   *
   * @see RegionService
   * @see PoolFactory#setMultiuserAuthentication(boolean)
   * @return the {@link RegionService} instance associated with a user and given
   *         properties.
   * @throws UnsupportedOperationException
   *           when invoked with multiuser-authentication as false.
   *
   * @param userSecurityProperties
   *        the security properties of a user.
   *
   * @param poolName
   *        the pool that the users should be authenticated against. Set if
   * there are more than one Pool in Cache.
   */

  virtual AuthenticatedView createAuthenticatedView(
      const std::shared_ptr<Properties>& userSecurityProperties,
      const std::string& poolName);

  /**
   * Get the CacheTransactionManager instance for this Cache.
   * @return The CacheTransactionManager instance.
   * @throws CacheClosedException if the cache is closed.
   */
  virtual std::shared_ptr<CacheTransactionManager> getCacheTransactionManager()
      const;

  /**
   * Returns whether Cache saves unread fields for Pdx types.
   */
  bool getPdxIgnoreUnreadFields() const override;

  /**
   * Returns whether { @link PdxInstance} is preferred for PDX types instead of
   * C++ object.
   */
  bool getPdxReadSerialized() const override;

  /**
   * Returns a factory that can create a {@link PdxInstance}.
   * @param className the fully qualified class name that the PdxInstance will
   * become
   * when it is fully deserialized.
   * @throws IllegalStateException if the className is nullptr or invalid.
   * @return the factory
   */
  PdxInstanceFactory createPdxInstanceFactory(
      const std::string& className) const override;

  virtual DataInput createDataInput(const uint8_t* m_buffer, size_t len) const;

  virtual DataOutput createDataOutput() const;

  virtual PoolManager& getPoolManager() const;

  SystemProperties& getSystemProperties() const override;

  Cache() = delete;
  virtual ~Cache();
  Cache(const Cache& other) = delete;
  Cache& operator=(const Cache& other) = delete;
  Cache(Cache&& other) noexcept;
  Cache& operator=(Cache&& other) noexcept;

 private:
  /**
   * @brief constructors
   */
  Cache(const std::shared_ptr<Properties>& dsProp, bool ignorePdxUnreadFields,
        bool readPdxSerialized,
        const std::shared_ptr<AuthInitialize>& authInitialize);

  std::unique_ptr<CacheImpl> m_cacheImpl;

 protected:
  static bool isPoolInMultiuserMode(std::shared_ptr<Region> regionPtr);

  friend class CacheFactory;
  friend class CacheRegionHelper;
  friend class FunctionService;
  friend class CacheXmlCreation;
  friend class RegionXmlCreation;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHE_H_
