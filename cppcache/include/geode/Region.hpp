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

#ifndef GEODE_REGION_H_
#define GEODE_REGION_H_

#include <chrono>
#include <iosfwd>
#include <memory>

#include "internal/geode_globals.hpp"
#include "CacheStatistics.hpp"
#include "CacheableBuiltins.hpp"
#include "CacheableKey.hpp"
#include "CacheableString.hpp"
#include "CacheableString.hpp"
#include "ExceptionTypes.hpp"
#include "Serializable.hpp"
#include "RegionAttributesFactory.hpp"
#include "AttributesMutator.hpp"
#include "CacheListener.hpp"
#include "CacheLoader.hpp"
#include "CacheWriter.hpp"
#include "CacheableKey.hpp"
#include "PartitionResolver.hpp"
#include "Query.hpp"
#include "RegionAttributes.hpp"
#include "RegionEntry.hpp"

namespace apache {
namespace geode {
namespace client {

class Pool;
class AttributesMutator;
class Cache;
class CacheStatistics;
class CacheableKey;
class RegionAttributes;
class RegionEntry;
class RegionService;
class SelectResults;
class Serializable;

static constexpr std::chrono::milliseconds DEFAULT_RESPONSE_TIMEOUT =
    std::chrono::seconds(15);

/**
 * @class Region Region.hpp
 *
 * This class manages subregions and cached data. Each region
 * can contain multiple subregions and entries for data.
 * Regions provide a hierachical name space
 * within the cache. Also, a region can be used to group cached
 * objects for management purposes.
 *
 * Entries managed by the region are key-value pairs. A set of region attributes
 * is associated with the region when it is created.
 *
 * The Region interface basically contains two set of APIs: Region management
 * APIs and (potentially) distributed operations on entries. Non-distributed
 * operations on entries  are provided by <code>RegionEntry</code>.
 *
 * Each <code>Cache</code> defines regions called the root regions.
 * User applications can use the root regions to create subregions
 * for isolated name space and object grouping.
 *
 * A region's name can be any String, except that it should not contain
 * the region name separator, a forward slash (/).
 *
 * <code>Regions</code>  can be referenced by a relative path name from any
 * region
 * higher in the hierarchy in {@link Region::getSubregion}. You can get the
 * relative
 * path from the root region with {@link Region::getFullPath}. The name
 * separator is used to concatenate all the region names together from the root,
 * starting with the root's subregions.
 *
 * @see RegionAttributes
 */
class APACHE_GEODE_EXPORT Region : public std::enable_shared_from_this<Region> {
  /** @brief Public Methods
   */
 public:
  /** return single name of region. The storage is backed by the region. */
  virtual const std::string& getName() const = 0;
  // virtual uint64_t getUpdateReceived() const { return 0; };

  /** return the full path of the region as can be used to lookup the
   * region from Cache::getRegion. The storage is backed by the region.
   */
  virtual const std::string& getFullPath() const = 0;

  /** Returns the parent region, or nullptr if a root region.
   * @throws RegionDestroyedException
   */
  virtual std::shared_ptr<Region> getParentRegion() const = 0;

  /** Return the RegionAttributes for this region.
   */
  virtual const RegionAttributes& getAttributes() const = 0;

  /** Return the a mutator object for changing a subset of the region
   * attributes.
   * @throws RegionDestroyedException.
   */
  virtual std::shared_ptr<AttributesMutator> getAttributesMutator() const = 0;

  // virtual void updateAccessOrModifiedTime() = 0;

  virtual std::shared_ptr<CacheStatistics> getStatistics() const = 0;

  /** Invalidates this region. The invalidation will cascade to
   * all the subregions and cached entries. After
   * the <code>invalidateRegion</code> , the region and the entries in it still
   * exist. In order to remove all the entries and the region,
   * <code>destroyRegion</code> should be used.
   *
   * @param aCallbackArgument a user-defined parameter to pass to callback
   * events triggered by this method. Can be nullptr. If it is sent on the wire,
   * it has to be Serializable.
   * @throws CacheListenerException if CacheListener throws an exception; if
   * this occurs some subregions may have already been successfully invalidated
   * @throws RegionDestroyedException if the region is no longer valid
   * @see   destroyRegion
   * @see   CacheListener::afterRegionInvalidate
   * This operation is not distributed.
   */
  virtual void invalidateRegion(
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;

  /** Invalidates this region. The invalidation will cascade to
  * all the subregions and cached entries. After
  * the <code>invalidateRegion</code> , the region and the entries in it still
  * exist. In order to remove all the entries and the region,
  * <code>destroyRegion</code> should be used. The region invalidate will not be
  distributed
  * to other caches
  *
  * @param aCallbackArgument a user-defined parameter to pass to callback events
  *        triggered by this method.
  *        Can be nullptr. If it is sent on the wire, it has to be Serializable.
  * @throws CacheListenerException if CacheListener throws an exception; if this
  *         occurs some subregions may have already been successfully
  invalidated
  * @throws RegionDestroyedException if the region is no longer valid
  * @see   destroyRegion
  * @see   CacheListener::afterRegionInvalidate

  */
  virtual void localInvalidateRegion(
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;

  /** Destroys the whole region and provides a user-defined parameter
   * object to any <code>CacheWriter</code> invoked in the process.
   * Destroy cascades to all entries
   * and subregions. After the destroy, this region object cannot be used
   * any more. Any attempt to use this region object will get a
   * <code>RegionDestroyedException</code> exception.
   *
   * The region destroy not only destroys the local region but also destroys the
   * server region. However, if server region destroy fails throwing back
   * <code>CacheServerException</code> or security exception,
   * the local region is still destroyed.
   *
   * @param aCallbackArgument a user-defined parameter to pass to callback
   * events triggered by this call. Can be nullptr. If it is sent on the wire,
   * it has to be Serializable.
   * @throws CacheWriterException if CacheWriter aborts the operation; if this
   *         occurs some subregions may have already been successfully
   * destroyed.
   * @throws CacheListenerException if CacheListener throws an exception; if
   * this occurs some subregions may have already been successfully invalidated
   * @throws CacheServerException If an exception is received from the Java
   * cache server. Only for Native Client regions.
   * @throws NotConnectedException if not connected to the geode system because
   * the client
   *         cannot establish usable connections to any of the servers given to
   * it.
   *         For pools configured with locators, if no locators are available,
   * the cause
   *         of NotConnectedException is set to NoAvailableLocatorsException.
   * @throws MessageExcepton If the message received from server could not be
   *         handled. This will be the case when an unregistered typeId is
   *         received in the reply or reply is not well formed.
   *         More information can be found in the log.
   * @throws TimeoutException if operation timed out
   * @see  invalidateRegion
   */
  virtual void destroyRegion(
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;
  /**
   * Removes all entries from this region and provides a user-defined parameter
   * object to any <code>CacheWriter</code> or <code>CacheListener</code>
   * invoked in the process.
   * @see CacheListener#afterRegionClear
   * @see CacheWriter#beforeRegionClear
   */
  virtual void clear(
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;
  /**
   * Removes all entries from this region and provides a user-defined parameter
   * object to any <code>CacheWriter</code> or <code>CacheListener</code>
   * invoked in the process. Clear will not be distributed to other caches.
   * @see CacheListener#afterRegionClear
   * @see CacheWriter#beforeRegionClear
   */
  virtual void localClear(
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;

  /** Destroys the whole region and provides a user-defined parameter
   * object to any <code>CacheWriter</code> invoked in the process.
   * Destroy cascades to all entries
   * and subregions. After the destroy, this region object cannot be used
   * any more. Any attempt to use this region object will get a
   * <code>RegionDestroyedException</code> exception. The region destroy is not
   * distributed to other caches.
   *
   * @param aCallbackArgument a user-defined parameter to pass to callback
   * events triggered by this call. Can be nullptr. If it is sent on the wire,
   * it has to be Serializable.
   * @throws CacheWriterException if CacheWriter aborts the operation; if this
   *         occurs some subregions may have already been successfully
   * destroyed.
   * @throws CacheListenerException if CacheListener throws an exception; if
   * this occurs some subregions may have already been successfully invalidated
   *
   * @see  localInvalidateRegion
   */
  virtual void localDestroyRegion(
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;

  /** Returns the subregion identified by the path, nullptr if no such subregion
   */
  virtual std::shared_ptr<Region> getSubregion(const std::string& path) = 0;

  /** Creates a subregion with the specified attributes */
  virtual std::shared_ptr<Region> createSubregion(
      const std::string& subregionName, RegionAttributes aRegionAttributes) = 0;

  /** Populates the passed in std::vector<std::shared_ptr<Region>> with
   * subregions of the current region
   * @param recursive determines whether the method recursively fills in
   * subregions
   * @param[out] sr subregions
   * @throws RegionDestroyedException
   */
  virtual std::vector<std::shared_ptr<Region>> subregions(
      const bool recursive) = 0;

  /** Return the meta-object RegionEntry for key.
   * @throws IllegalArgumentException, RegionDestroyedException.
   */
  virtual std::shared_ptr<RegionEntry> getEntry(
      const std::shared_ptr<CacheableKey>& key) = 0;

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline std::shared_ptr<RegionEntry> getEntry(const KEYTYPE& key) {
    return getEntry(CacheableKey::create(key));
  }

  /** Returns the value associated with the specified key, passing the callback
   * argument to any cache loaders that are invoked in the
   * operation.
   * If the value is not present locally then it is requested from the java
   *server.
   * If even that is unsuccessful then a local CacheLoader will be invoked if
   *there is one.
   * The value returned by get is not copied, so multi-threaded applications
   * should not modify the value directly, but should use the update methods.
   *<p>
   * Updates the {@link CacheStatistics::getLastAccessedTime},
   * {@link CacheStatistics::getHitCount}, {@link
   *CacheStatistics::getMissCount}, and {@link
   *CacheStatistics::getLastModifiedTime} (if a new value is loaded) for this
   *region and the entry.
   *
   * @param key whose associated value is to be returned. The key Object must
   * implement the equals and hashCode methods.
   * @param aCallbackArgument an argument passed into the CacheLoader if
   * loader is used. If it is sent on the wire, it has to be Serializable.
   *
   * @throws IllegalArgumentException if key is nullptr or aCallbackArgument is
   *         not serializable and a remote CacheLoader needs to be invoked
   * @throws CacheLoaderException if CacheLoader throws an exception
   * @throws CacheServerException If an exception is received from the Java
   *cache server. Only for Native Client regions.
   * @throws NotConnectedException if it is not connected to the cache because
   *the client
   *         cannot establish usable connections to any of the servers given to
   *it.
   *         For pools configured with locators, if no locators are available,
   *the cause
   *         of NotConnectedException is set to NoAvailableLocatorsException.
   * @throws MessageExcepton If the message received from server could not be
   *         handled. This will be the case when an unregistered typeId is
   *         received in the reply or reply is not well formed.
   *         More information can be found in the log.
   * @throws TimeoutException if operation timed out
   * @throws RegionDestroyedException if the method is called on a destroyed
   *region
   **/
  virtual std::shared_ptr<Cacheable> get(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline std::shared_ptr<Cacheable> get(
      const KEYTYPE& key,
      const std::shared_ptr<Serializable>& callbackArg = nullptr) {
    return get(CacheableKey::create(key), callbackArg);
  }

  /** Places a new value into an entry in this region with the specified key,
   * providing a user-defined parameter
   * object to any <code>CacheWriter</code> invoked in the process.
   * The same parameter is also passed to the <code>CacheListener</code>,
   * if one is defined for this <code>Region</code>, invoked in the process.
   * If there is already an entry associated with the specified key in this
   * region,
   * the entry's previous value is overwritten.
   * The new put value is propogated to the java server to which it is connected
   * with.
   * <p>Updates the {@link CacheStatistics::getLastAccessedTime} and
   * {@link CacheStatistics::getLastModifiedTime} for this region and the entry.
   *
   * If remote server put fails throwing back a
   * <code>CacheServerException</code> or security exception, then local put is
   * tried to rollback. However, if the entry has overflowed/evicted/expired
   * then the rollback is aborted since it may be due to a more recent
   * notification or update by another thread.
   *
   * @param key a key smart pointer associated with the value to be put into
   * this region.
   * @param value the value to be put into the cache
   * @param aCallbackArgument an argument that is passed to the callback
   * function
   *
   * @throws IllegalArgumentException if key or value is nullptr
   * @throws CacheWriterException if CacheWriter aborts the operation
   * @throws CacheListenerException if CacheListener throws an exception
   * @throws RegionDestroyedException if region no longer valid
   * @throws CacheServerException If an exception is received from the Java
   * cache server.
   * @throws NotConnectedException if it is not connected to the cache because
   * the client
   *         cannot establish usable connections to any of the servers given to
   * it
   *         For pools configured with locators, if no locators are available,
   * the cause
   *         of NotConnectedException is set to NoAvailableLocatorsException.
   * @throws MessageExcepton If the message received from server could not be
   *         handled. This will be the case when an unregistered typeId is
   *         received in the reply or reply is not well formed.
   *         More information can be found in the log.
   * @throws TimeoutException if operation timed out
   * @throws OutOfMemoryException if  not enoough memory for the value
   */
  virtual void put(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline void put(const KEYTYPE& key, const VALUETYPE& value,
                  const std::shared_ptr<Serializable>& arg = nullptr) {
    put(CacheableKey::create(key), Serializable::create(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void put(const KEYTYPE& key, const std::shared_ptr<Cacheable>& value,
                  const std::shared_ptr<Serializable>& arg = nullptr) {
    put(CacheableKey::create(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline void put(const std::shared_ptr<CacheableKey>& key,
                  const VALUETYPE& value,
                  const std::shared_ptr<Serializable>& arg = nullptr) {
    put(key, Serializable::create(value), arg);
  }

  /**
   * Places a set of new values in this region with the specified keys
   * given as a map of key/value pairs.
   * If there is already an entry associated with a specified key in this
   * region, the entry's previous value is overwritten.
   * <p>Updates the {@link CacheStatistics::getLastAccessedTime} and
   * {@link CacheStatistics::getLastModifiedTime} for this region and
   * the entries.
   *
   * @param map: A hashmap containing key-value pairs
   * @param timeout: The time to wait for the response, optional.
   * @param aCallbackArgument an argument that is passed to the callback
   * functions.
   * It is ignored if nullptr. It must be serializable if this operation is
   * distributed.
   * @throws IllegalArgumentException If timeout exceeds 2147483647ms.
   * @since 8.1
   */
  virtual void putAll(
      const HashMapOfCacheable& map,
      std::chrono::milliseconds timeout = DEFAULT_RESPONSE_TIMEOUT,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;

  /**
   * Places a new value into an entry in this region with the specified key
   * in the local cache only, providing a user-defined parameter
   * object to any <code>CacheWriter</code> invoked in the process.
   * The same parameter is also passed to the <code>CacheListener</code>,
   * if one is defined for this <code>Region</code>, invoked in the process.
   * If there is already an entry associated with the specified key in this
   * region,
   * the entry's previous value is overwritten.
   * <p>Updates the {@link CacheStatistics::getLastAccessedTime} and
   * {@link CacheStatistics::getLastModifiedTime} for this region and the entry.
   *
   * @param key a key smart pointer associated with the value to be put into
   * this region.
   * @param value the value to be put into the cache
   * @param aCallbackArgument an argument that is passed to the callback
   * functions
   *
   * @throws IllegalArgumentException if key or value is nullptr
   * @throws CacheWriterException if CacheWriter aborts the operation
   * @throws CacheListenerException if CacheListener throws an exception
   * @throws RegionDestroyedException if region no longer valid
   * @throws OutOfMemoryException if not enoough memory for the value
   */
  virtual void localPut(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline void localPut(const KEYTYPE& key, const VALUETYPE& value,
                       const std::shared_ptr<Serializable>& arg = nullptr) {
    localPut(CacheableKey::create(key), Serializable::create(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void localPut(const KEYTYPE& key,
                       const std::shared_ptr<Cacheable>& value,
                       const std::shared_ptr<Serializable>& arg = nullptr) {
    localPut(CacheableKey::create(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline void localPut(const std::shared_ptr<CacheableKey>& key,
                       const VALUETYPE& value,
                       const std::shared_ptr<Serializable>& arg = nullptr) {
    localPut(key, Serializable::create(value), arg);
  }

  /** Creates a new entry in this region with the specified key and value,
   * providing a user-defined parameter
   * object to any <code>CacheWriter</code> invoked in the process.
   * The same parameter is also passed to the <code>CacheListener</code>,
   * if one is defined for this <code>Region</code>, invoked in the process.
   * The new entry is propogated to the java server also to which it is
   * connected with. <p>Updates the {@link CacheStatistics::getLastAccessedTime}
   * and
   * {@link CacheStatistics::getLastModifiedTime} for this region and the entry.
   * <p>
   *
   * If remote server put fails throwing back a
   * <code>CacheServerException</code> or security exception, then local put is
   * tried to rollback. However, if the entry has overflowed/evicted/expired
   * then the rollback is aborted since it may be due to a more recent
   * notification or update by another thread.
   *
   * @param key the key smart pointer for which to create the entry in this
   * region.
   * @param value the value for the new entry, which may be nullptr meaning
   *              the new entry starts as if it had been locally invalidated.
   * @param aCallbackArgument a user-defined parameter to pass to callback
   * events triggered by this method. Can be nullptr. Should be serializable if
   *        passed to remote callback events
   * @throws IllegalArgumentException if key is nullptr or if the key, value, or
   *         aCallbackArgument do not meet serializability requirements
   * @throws CacheWriterException if CacheWriter aborts the operation
   * @throws CacheListenerException if CacheListener throws an exception
   * @throws RegionDestroyedException if region is no longer valid
   * @throws CacheServerException If an exception is received from the Java
   * cache server. Only for Native Client regions.
   * @throws NotConnectedException if it is not connected to the cache because
   * the client
   *         cannot establish usable connections to any of the servers given to
   * it
   *         For pools configured with locators, if no locators are available,
   * the cause
   *         of NotConnectedException is set to NoAvailableLocatorsException.
   * @throws MessageExcepton If the message received from server could not be
   *         handled. This will be the case when an unregistered typeId is
   *         received in the reply or reply is not well formed.
   *         More information can be found in the log.
   * @throws TimeoutException if the operation timed out
   * @throws OutOfMemoryException if no memory for new entry
   * @throws EntryExistsException if an entry with this key already exists
   */
  virtual void create(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline void create(const KEYTYPE& key, const VALUETYPE& value,
                     const std::shared_ptr<Serializable>& arg = nullptr) {
    create(CacheableKey::create(key), Serializable::create(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void create(const KEYTYPE& key,
                     const std::shared_ptr<Cacheable>& value,
                     const std::shared_ptr<Serializable>& arg = nullptr) {
    create(CacheableKey::create(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline void create(const std::shared_ptr<CacheableKey>& key,
                     const VALUETYPE& value,
                     const std::shared_ptr<Serializable>& arg = nullptr) {
    create(key, Serializable::create(value), arg);
  }

  /** Creates a new entry in this region with the specified key and value
   * in the local cache only, providing a user-defined parameter
   * object to any <code>CacheWriter</code> invoked in the process.
   * The same parameter is also passed to the <code>CacheListener</code>,
   * if one is defined for this <code>Region</code>, invoked in the process.
   * <p>Updates the {@link CacheStatistics::getLastAccessedTime} and
   * {@link CacheStatistics::getLastModifiedTime} for this region and the entry.
   * <p>
   *
   * @param key the key smart pointer for which to create the entry in this
   * region.
   * @param value the value for the new entry, which may be nullptr meaning
   *              the new entry starts as if it had been locally invalidated.
   * @param aCallbackArgument a user-defined parameter to pass to callback
   * events
   *        triggered by this method. Can be nullptr. Should be serializable if
   *        passed to remote callback events
   *
   * @throws IllegalArgumentException if key or value is nullptr
   * @throws CacheWriterException if CacheWriter aborts the operation
   * @throws CacheListenerException if CacheListener throws an exception
   * @throws RegionDestroyedException if region is no longer valid
   * @throws OutOfMemoryException if no memory for new entry
   * @throws EntryExistsException if an entry with this key already exists
   */
  virtual void localCreate(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline void localCreate(const KEYTYPE& key, const VALUETYPE& value,
                          const std::shared_ptr<Serializable>& arg = nullptr) {
    localCreate(CacheableKey::create(key), Serializable::create(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void localCreate(const KEYTYPE& key,
                          const std::shared_ptr<Cacheable>& value,
                          const std::shared_ptr<Serializable>& arg = nullptr) {
    localCreate(CacheableKey::create(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline void localCreate(const std::shared_ptr<CacheableKey>& key,
                          const VALUETYPE& value,
                          const std::shared_ptr<Serializable>& arg = nullptr) {
    localCreate(key, Serializable::create(value), arg);
  }

  /** Invalidates the entry with the specified key,
   * and provides a user-defined argument to the <code>CacheListener</code>.
   * Invalidate only removes the value from the entry, the key is kept intact.
   * To completely remove the entry, destroy should be used.
   * The invalidate is not propogated to the Geode cache server to which it is
   * connected with.
   * <p>Updates the {@link CacheStatistics::getLastAccessedTime} and
   * {@link CacheStatistics::getLastModifiedTime} for this region and the entry.
   * <p>
   *
   * @param key the key of the value to be invalidated
   * @param aCallbackArgument a user-defined parameter to pass to callback
   * events triggered by this method. Can be nullptr. Should be serializable if
   *        passed to remote callback events
   * @throws IllegalArgumentException if key is nullptr
   * @throws CacheListenerException if CacheListener throws an exception
   * @throws EntryNotFoundException if this entry does not exist in this region
   * locally
   * @throws RegionDestroyedException if the region is destroyed
   * @see destroy
   * @see CacheListener::afterInvalidate
   */
  virtual void invalidate(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void invalidate(const KEYTYPE& key,
                         const std::shared_ptr<Serializable>& arg = nullptr) {
    invalidate(CacheableKey::create(key), arg);
  }

  /** Invalidates the entry with the specified key in the local cache only,
   * and provides a user-defined argument to the <code>CacheListener</code>.
   * Invalidate only removes the value from the entry, the key is kept intact.
   * To completely remove the entry, destroy should be used.
   * <p>Updates the {@link CacheStatistics::getLastAccessedTime} and
   * {@link CacheStatistics::getLastModifiedTime} for this region and the entry.
   * <p>
   *
   * @param key the key of the value to be invalidated
   * @param aCallbackArgument a user-defined parameter to pass to callback
   * events triggered by this method. Can be nullptr. Should be serializable if
   *        passed to remote callback events
   * @throws IllegalArgumentException if key is nullptr
   * @throws CacheListenerException if CacheListener throws an exception
   * @throws EntryNotFoundException if this entry does not exist in this region
   * locally
   * @throws RegionDestroyedException if the region is destroyed
   * @see destroy
   * @see CacheListener::afterInvalidate
   */
  virtual void localInvalidate(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void localInvalidate(
      const KEYTYPE& key, const std::shared_ptr<Serializable>& arg = nullptr) {
    localInvalidate(CacheableKey::create(key), arg);
  }

  /** Destroys the entry with the specified key, and provides a user-defined
   * parameter object to any <code>CacheWriter</code> invoked in the process.
   * The same parameter is also passed to the <code>CacheListener</code>,
   * if one is defined for this <code>Region</code>, invoked in the process.
   * Destroy removes
   * not only the value, but also the key and entry from this region.
   *
   * The destroy is propogated to the Geode cache server to which it is
   * connected with. If the destroy fails due to an exception on server
   * throwing back <code>CacheServerException</code> or security exception,
   * then the local entry is still destroyed.
   *
   * <p>Updates the {@link CacheStatistics::getLastAccessedTime} and
   * {@link CacheStatistics::getLastModifiedTime} for this region and the entry.
   * <p>
   *
   * @param key the key of the entry to destroy
   * @param aCallbackArgument a user-defined parameter to pass to callback
   * events triggered by this method. Can be nullptr. If it is sent on the wire,
   * it has to be Serializable.
   * @throws IllegalArgumentException if key is nullptr
   * @throws CacheWriterException if CacheWriter aborts the operation
   * @throws CacheListenerException if CacheListener throws an exception
   * @throws CacheServerException If an exception is received from the Geode
   * cache server.
   *         Only for Native Client regions.
   * @throws NotConnectedException if it is not connected to the cache because
   * the client
   *         cannot establish usable connections to any of the servers given to
   * it
   *         For pools configured with locators, if no locators are available,
   * the cause
   *         of NotConnectedException is set to NoAvailableLocatorsException.
   * @throws MessageExcepton If the message received from server could not be
   *         handled. This will be the case when an unregistered typeId is
   *         received in the reply or reply is not well formed.
   *         More information can be found in the log.
   * @throws TimeoutException if the operation timed out
   * @throws RegionDestroyedException if the region is destroyed.
   * @throws EntryNotFoundException if the entry does not exist in this region.
   * @see invalidate
   * @see CacheListener::afterDestroy
   * @see CacheWriter::beforeDestroy
   */
  virtual void destroy(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void destroy(const KEYTYPE& key,
                      const std::shared_ptr<Serializable>& arg = nullptr) {
    destroy(CacheableKey::create(key), arg);
  }

  /** Destroys the entry with the specified key in the local cache only,
   * and provides a user-defined parameter object to any
   * <code>CacheWriter</code> invoked in the process.
   * The same parameter is also passed to the <code>CacheListener</code>,
   * if one is defined for this <code>Region</code>, invoked in the process.
   * Destroy removes
   * not only the value but also the key and entry from this region.
   * <p>
   * <p>Updates the {@link CacheStatistics::getLastAccessedTime} and
   * {@link CacheStatistics::getLastModifiedTime} for this region and the entry.
   * <p>
   *
   * @param key the key of the entry to destroy.
   * @param aCallbackArgument the callback for user to pass in, default is
   * nullptr.
   * @throws IllegalArgumentException if key is nullptr
   * @throws CacheWriterException if CacheWriter aborts the operation
   * @throws CacheListenerException if CacheListener throws an exception
   * @throws EntryNotFoundException if the entry does not exist in this region
   * locally
   * @see invalidate
   * @see CacheListener::afterDestroy
   * @see CacheWriter::beforeDestroy
   */
  virtual void localDestroy(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline void localDestroy(const KEYTYPE& key,
                           const std::shared_ptr<Serializable>& arg = nullptr) {
    localDestroy(CacheableKey::create(key), arg);
  }

  /** Removes the entry with the specified key, value and provides a
   * user-defined
   * parameter object to any <code>CacheWriter</code> invoked in the process.
   * The same parameter is also passed to the <code>CacheListener</code> and
   * <code>CacheWriter</code>,
   * if one is defined for this <code>Region</code>, invoked in the process.
   * remove removes
   * not only the value, but also the key and entry from this region.
   *
   * The remove is propogated to the Geode cache server to which it is
   * connected with. If the destroy fails due to an exception on server
   * throwing back <code>CacheServerException</code> or security exception,
   * then the local entry is still removed.
   *
   * <p>Updates the {@link CacheStatistics::getLastAccessedTime} and
   * {@link CacheStatistics::getLastModifiedTime} for this region and the entry.
   * <p>
   *
   * @param key the key of the entry to remove
   * @param value the value of the key to remove, it can be nullptr.
   * @param aCallbackArgument a user-defined parameter to pass to callback
   * events triggered by this method. Can be nullptr. If it is sent on the wire,
   * it has to be Serializable.
   * @throws IllegalArgumentException if key is nullptr
   * @throws CacheWriterException if CacheWriter aborts the operation
   * @throws CacheListenerException if CacheListener throws an exception
   * @throws CacheServerException If an exception is received from the Geode
   * cache server.
   *         Only for Native Client regions.
   * @throws NotConnectedException if it is not connected to the cache because
   * the client
   *         cannot establish usable connections to any of the servers given to
   * it
   *         For pools configured with locators, if no locators are available,
   * the cause
   *         of NotConnectedException is set to NoAvailableLocatorsException.
   * @throws MessageExcepton If the message received from server could not be
   *         handled. This will be the case when an unregistered typeId is
   *         received in the reply or reply is not well formed.
   *         More information can be found in the log.
   * @throws TimeoutException if the operation timed out
   * @throws RegionDestroyedException if the region is destroyed.
   * @return the boolean true if an entry(key, value)has been removed or
   * false if an entry(key, value) has not been removed.
   * @see destroy
   * @see CacheListener::afterDestroy
   * @see CacheWriter::beforeDestroy
   */
  virtual bool remove(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline bool remove(const KEYTYPE& key, const VALUETYPE& value,
                     const std::shared_ptr<Serializable>& arg = nullptr) {
    return remove(CacheableKey::create(key), Serializable::create(value), arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline bool remove(const KEYTYPE& key,
                     const std::shared_ptr<Cacheable>& value,
                     const std::shared_ptr<Serializable>& arg = nullptr) {
    return remove(CacheableKey::create(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline bool remove(const std::shared_ptr<CacheableKey>& key,
                     const VALUETYPE& value,
                     const std::shared_ptr<Serializable>& arg = nullptr) {
    return remove(key, Serializable::create(value), arg);
  }

  bool remove(const std::shared_ptr<CacheableKey>& key) {
    return removeEx(key);
  }

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE>
  inline bool remove(const KEYTYPE& key) {
    return remove(CacheableKey::create(key));
  }

  /** Removes the entry with the specified key and provides a user-defined
   * parameter object to any <code>CacheWriter</code> invoked in the process.
   * The same parameter is also passed to the <code>CacheListener</code> and
   * <code>CacheWriter</code>,
   * if one is defined for this <code>Region</code>, invoked in the process.
   * remove removes
   * not only the value, but also the key and entry from this region.
   *
   * The remove is propogated to the Geode cache server to which it is
   * connected with. If the destroy fails due to an exception on server
   * throwing back <code>CacheServerException</code> or security exception,
   * then the local entry is still removed.
   *
   * <p>Updates the {@link CacheStatistics::getLastAccessedTime} and
   * {@link CacheStatistics::getLastModifiedTime} for this region and the entry.
   * <p>
   *
   * @param key the key of the entry to remove
   * @param aCallbackArgument a user-defined parameter to pass to callback
   * events triggered by this method. Can be nullptr. If it is sent on the wire,
   * it has to be Serializable.
   * @throws IllegalArgumentException if key is nullptr
   * @throws CacheWriterException if CacheWriter aborts the operation
   * @throws CacheListenerException if CacheListener throws an exception
   * @throws CacheServerException If an exception is received from the Geode
   * cache server.
   *         Only for Native Client regions.
   * @throws NotConnectedException if it is not connected to the cache because
   * the client
   *         cannot establish usable connections to any of the servers given to
   * it
   *         For pools configured with locators, if no locators are available,
   * the cause
   *         of NotConnectedException is set to NoAvailableLocatorsException.
   * @throws MessageExcepton If the message received from server could not be
   *         handled. This will be the case when an unregistered typeId is
   *         received in the reply or reply is not well formed.
   *         More information can be found in the log.
   * @throws TimeoutException if the operation timed out
   * @throws RegionDestroyedException if the region is destroyed.
   * @return the boolean true if an entry(key, value)has been removed or
   * false if an entry(key, value) has not been removed.
   * @see destroy
   * @see CacheListener::afterDestroy
   * @see CacheWriter::beforeDestroy
   */
  virtual bool removeEx(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline bool removeEx(const KEYTYPE& key,
                       const std::shared_ptr<Serializable>& arg = nullptr) {
    return removeEx(CacheableKey::create(key), arg);
  }

  /** Removes the entry with the specified key and value in the local cache
   * only,
   * and provides a user-defined parameter object to any
   * <code>CacheWriter</code> invoked in the process.
   * The same parameter is also passed to the <code>CacheListener</code> and
   * <code>CacheWriter</code>,
   * if one is defined for this <code>Region</code>, invoked in the process.
   * Remove removes
   * not only the value but also the key and entry from this region.
   * <p>
   * <p>Updates the {@link CacheStatistics::getLastAccessedTime} and
   * {@link CacheStatistics::getLastModifiedTime} for this region and the entry.
   * <p>
   *
   * @param key the key of the entry to remove.
   * @param value the value of the entry to remove.
   * @param aCallbackArgument the callback for user to pass in, default is
   * nullptr.
   * @throws IllegalArgumentException if key is nullptr
   * @throws CacheWriterException if CacheWriter aborts the operation
   * @throws CacheListenerException if CacheListener throws an exception
   * @return the boolean true if an entry(key, value)has been removed or
   * false if an entry(key, value) has not been removed.
   * @see destroy
   * @see CacheListener::afterDestroy
   * @see CacheWriter::beforeDestroy
   */
  virtual bool localRemove(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;

  /** Convenience method allowing both key and value to be a const char* */
  template <class KEYTYPE, class VALUETYPE>
  inline bool localRemove(const KEYTYPE& key, const VALUETYPE& value,
                          const std::shared_ptr<Serializable>& arg = nullptr) {
    return localRemove(CacheableKey::create(key), Serializable::create(value),
                       arg);
  }

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline bool localRemove(const KEYTYPE& key,
                          const std::shared_ptr<Cacheable>& value,
                          const std::shared_ptr<Serializable>& arg = nullptr) {
    return localRemove(CacheableKey::create(key), value, arg);
  }

  /** Convenience method allowing value to be a const char* */
  template <class VALUETYPE>
  inline bool localRemove(const std::shared_ptr<CacheableKey>& key,
                          const VALUETYPE& value,
                          const std::shared_ptr<Serializable>& arg = nullptr) {
    return localRemove(key, Serializable::create(value), arg);
  }

  /** Removes the entry with the specified key in the local cache only,
   * and provides a user-defined parameter object to any
   * <code>CacheWriter</code> invoked in the process.
   * The same parameter is also passed to the <code>CacheListener</code> and
   * <code>CacheWriter</code>,
   * if one is defined for this <code>Region</code>, invoked in the process.
   * Remove removes
   * not only the value but also the key and entry from this region.
   * <p>
   * <p>Updates the {@link CacheStatistics::getLastAccessedTime} and
   * {@link CacheStatistics::getLastModifiedTime} for this region and the entry.
   * <p>
   *
   * @param key the key of the entry to remove.
   * @param aCallbackArgument the callback for user to pass in, default is
   * nullptr.
   * @throws IllegalArgumentException if key is nullptr
   * @throws CacheWriterException if CacheWriter aborts the operation
   * @throws CacheListenerException if CacheListener throws an exception
   * @return the boolean true if an entry(key, value)has been removed or
   * false if an entry(key, value) has not been removed.
   * @see destroy
   * @see CacheListener::afterDestroy
   * @see CacheWriter::beforeDestroy
   */

  virtual bool localRemoveEx(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;

  /** Convenience method allowing key to be a const char* */
  template <class KEYTYPE>
  inline bool localRemoveEx(
      const KEYTYPE& key, const std::shared_ptr<Serializable>& arg = nullptr) {
    return localRemoveEx(CacheableKey::create(key), arg);
  }

  /**
   * Return all the keys in the local process for this region. This includes
   * keys for which the entry is invalid.
   */
  virtual std::vector<std::shared_ptr<CacheableKey>> keys() = 0;

  /**
   * Return the set of keys defined in the server process associated to this
   * client and region. If a server has the region defined as a mirror, then
   * this will be the entire keyset for the region across all PEER in the
   * distributed system.
   * The vector v will contain only the server keys. Any prior contents in the
   * vector will be removed.
   * @throws CacheServerException If an exception is received from the Geode
   * cache server.
   *         Only for Native Client regions.
   * @throws NotConnectedException if it is not connected to the cache because
   * the client
   *         cannot establish usable connections to any of the servers given to
   * it
   *         For pools configured with locators, if no locators are available,
   * the cause
   *         of NotConnectedException is set to NoAvailableLocatorsException.
   * @throws MessageExcepton If the message received from server could not be
   *         handled. This will be the case when an unregistered typeId is
   *         received in the reply or reply is not well formed.
   *         More information can be found in the log.
   * @throws TimeoutException if there is a timeout getting the keys
   * @throws UnsupportedOperationException if the member type is not CLIENT
   *                                       or region is not a native client one.
   */
  virtual std::vector<std::shared_ptr<CacheableKey>> serverKeys() = 0;

  /**
   * Return all values in the local process for this region. No value is
   * included for entries that are invalidated.
   */
  virtual std::vector<std::shared_ptr<Cacheable>> values() = 0;

  virtual std::vector<std::shared_ptr<RegionEntry>> entries(bool recursive) = 0;

  /**
   * Returns the <code>cache</code> associated with this region.
   * @return the cache
   */
  virtual RegionService& getRegionService() const = 0;

  virtual bool isDestroyed() const = 0;

  /**
   * This operations checks for the value in the local cache .
   * It is not propagated to the Geode cache server
   * to which it is connected.
   */
  virtual bool containsValueForKey(
      const std::shared_ptr<CacheableKey>& keyPtr) const = 0;

  /**
   * Convenience method allowing key to be a const char*
   * This operations checks for the value in the local cache .
   * It is not propagated to the Geode cache server
   * to which it is connected.
   */
  template <class KEYTYPE>
  inline bool containsValueForKey(const KEYTYPE& key) const {
    return containsValueForKey(CacheableKey::create(key));
  }

  /**
   * Only the client's cache is searched for the key. It does not go to the java
   * server
   * to which it is connected with.
   */
  virtual bool containsKey(
      const std::shared_ptr<CacheableKey>& keyPtr) const = 0;
  /**
   * The cache of the server, to which it is connected with, is searched
   * for the key to see if the key is present.
   * @throws UnsupportedOperationException if the region's scope is
   * ScopeType::LOCAL.
   */
  virtual bool containsKeyOnServer(
      const std::shared_ptr<CacheableKey>& keyPtr) const = 0;
  /**
   * Returns the list of keys on which this client is interested and will be
   * notified of changes.
   * @throws UnsupportedOperationException if the region's scope is
   * ScopeType::LOCAL.
   */
  virtual std::vector<std::shared_ptr<CacheableKey>> getInterestList()
      const = 0;
  /**
   * Returns the list of regular expresssions on which this client is
   * interested and will be notified of changes.
   * @throws UnsupportedOperationException if the region's scope is
   * ScopeType::LOCAL.
   */
  virtual std::vector<std::shared_ptr<CacheableString>> getInterestListRegex()
      const = 0;
  /**
   * Convenience method allowing key to be a const char*
   * This operations checks for the key in the local cache .
   * It is not propagated to the Geode cache server
   * to which it is connected.
   */
  template <class KEYTYPE>
  inline bool containsKey(const KEYTYPE& key) const {
    return containsKey(CacheableKey::create(key));
  }

  /**
   * Registers an array of keys for getting updates from the server.
   * Valid only for a Native Client region when client notification
   * ( {@link RegionAttributesFactory::setClientNotification} ) is true.
   *
   * @param keys the array of keys
   * @param isDurable flag to indicate whether this is a durable registration
   * @param getInitialValues true to populate the cache with values of the keys
   *   that were registered on the server
   * @param receiveValues whether to act like notify-by-subscription is set
   *
   * @throws IllegalArgumentException If the array of keys is empty.
   * @throws IllegalStateException If already registered interest for all keys.
   * @throws EntryNotFoundException If an exception occurs while obtaining
   *   values from server after register interest is complete. The actual cause
   *   of the exception can be obtained using <code>Exception::getCause</code>.
   *   If an application wants to undo the registration on server, or take
   *   some other steps for the incomplete cache population then this is
   *   the exception that should be caught.
   * @throws UnsupportedOperationException If the region is not a Native Client
   * region or
   * {@link RegionAttributesFactory::setClientNotification} is false.
   * @throws CacheServerException If an exception is received from the Java
   * cache server.
   * @throws NotConnectedException if it is not connected to the cache because
   * the client
   *         cannot establish usable connections to any of the servers given to
   * it
   *         For pools configured with locators, if no locators are available,
   * the cause
   *         of NotConnectedException is set to NoAvailableLocatorsException.
   * @throws RegionDestroyedException If region destroy is pending.
   * @throws UnknownException For other exceptions.
   * @throws TimeoutException if operation timed out
   */
  virtual void registerKeys(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      bool isDurable = false, bool getInitialValues = false,
      bool receiveValues = true) = 0;

  /**
   * Unregisters an array of keys to stop getting updates for them.
   * Valid only for a Native Client region when client notification
   * ( {@link RegionAttributesFactory::setClientNotification} ) is true.
   *
   * @param keys the array of keys
   *
   * @throws IllegalArgumentException If the array of keys is empty.
   * @throws IllegalStateException If no keys were previously registered.
   * @throws UnsupportedOperationException If the region is not a Native Client
   * region or
   * {@link RegionAttributesFactory::setClientNotification} is false.
   * @throws CacheServerException If an exception is received from the Java
   * cache server.
   * @throws NotConnectedException if it is not connected to the cache because
   * the client
   *         cannot establish usable connections to any of the servers given to
   * it
   *         For pools configured with locators, if no locators are available,
   * the cause
   *         of NotConnectedException is set to NoAvailableLocatorsException.
   * @throws RegionDestroyedException If region destroy is pending.
   * @throws UnknownException For other exceptions.
   * @throws TimeoutException if operation timed out
   */
  virtual void unregisterKeys(
      const std::vector<std::shared_ptr<CacheableKey>>& keys) = 0;

  /**
   * Registers to get updates for all keys from the server.
   * Valid only for a Native Client region when client notification
   * ( {@link RegionAttributesFactory::setClientNotification} ) is true.
   *
   * @param isDurable flag to indicate whether this is a durable registration
   * @param getInitialValues true to populate the cache with values of all keys
   *   from the server
   * @param receiveValues whether to act like notify-by-subscription is set
   *
   * @throws EntryNotFoundException If an exception occurs while obtaining
   *   values from server after register interest is complete. The actual cause
   *   of the exception can be obtained using <code>Exception::getCause</code>.
   *   If an application wants to undo the registration on server, or take
   *   some other steps for the incomplete cache population then this is
   *   the exception that should be caught.
   * @throws UnsupportedOperationException If the region is not a Native Client
   * region or
   * {@link RegionAttributesFactory::setClientNotification} is false.
   * @throws CacheServerException If an exception is received from the Java
   * cache server.
   * @throws NotConnectedException if it is not connected to the cache because
   * the client
   *         cannot establish usable connections to any of the servers given to
   * it
   *         For pools configured with locators, if no locators are available,
   * the cause
   *         of NotConnectedException is set to NoAvailableLocatorsException.
   * @throws RegionDestroyedException If region destroy is pending.
   * @throws UnknownException For other exceptions.
   * @throws TimeoutException if operation timed out
   */
  virtual void registerAllKeys(bool isDurable = false,
                               bool getInitialValues = false,
                               bool receiveValues = true) = 0;

  /**
   * Registers to get updates for all keys from the server.
   * Valid only for a Native Client region when client notification
   * ( {@link RegionAttributesFactory::setClientNotification} ) is true.
   *
   * @throws IllegalStateException If not previously registered all keys.
   * @throws UnsupportedOperationException If the region is not a Native Client
   * region or
   * {@link RegionAttributesFactory::setClientNotification} is false.
   * @throws CacheServerException If an exception is received from the Java
   * cache server.
   * @throws NotConnectedException if it is not connected to the cache because
   * the client
   *         cannot establish usable connections to any of the servers given to
   * it
   *         For pools configured with locators, if no locators are available,
   * the cause
   *         of NotConnectedException is set to NoAvailableLocatorsException.
   * @throws RegionDestroyedException If region destroy is pending.
   * @throws UnknownException For other exceptions.
   * @throws TimeoutException if operation timed out
   */
  virtual void unregisterAllKeys() = 0;

  /**
   * Registers a regular expression to match with keys to get updates from the
   * server.
   * Valid only for a Native Client region when client notification
   * ( {@link RegionAttributesFactory::setClientNotification} ) is true.
   *
   * @param regex The regular expression string.
   * @param isDurable flag to indicate whether this is a durable registration
   * @param resultKeys If non-nullptr then the keys that match the regular
   *   expression on the server are returned. The vector is cleared at the
   *   start to discard any existing keys in the vector.
   * @param getInitialValues true to populate the cache with values of the keys
   *   that were registered on the server
   * @param receiveValues whether to act like notify-by-subscription is set
   *
   * @throws IllegalArgumentException If regex is empty.
   * @throws IllegalStateException If already registered interest for all keys.
   * @throws EntryNotFoundException If an exception occurs while obtaining
   *   values from server after register interest is complete. The actual cause
   *   of the exception can be obtained using <code>Exception::getCause</code>.
   *   If an application wants to undo the registration on server, or take
   *   some other steps for the incomplete cache population then this is
   *   the exception that should be caught.
   * @throws UnsupportedOperationException If the region is not a Native Client
   * region or
   * {@link RegionAttributesFactory::setClientNotification} is false.
   * @throws CacheServerException If an exception is received from the Java
   * cache server.
   * @throws NotConnectedException if it is not connected to the cache because
   * the client
   *         cannot establish usable connections to any of the servers given to
   * it
   *         For pools configured with locators, if no locators are available,
   * the cause
   *         of NotConnectedException is set to NoAvailableLocatorsException.
   * @throws MessageExcepton If the message received from server could not be
   *         handled. This will be the case when an unregistered typeId is
   *         received in the reply or reply is not well formed.
   *         More information can be found in the log.
   * @throws RegionDestroyedException If region destroy is pending.
   * @throws UnknownException For other exceptions.
   * @throws TimeoutException if operation timed out
   */
  virtual void registerRegex(const std::string& regex, bool isDurable = false,
                             bool getInitialValues = false,
                             bool receiveValues = true) = 0;

  /**
   * Unregisters a regular expression to stop getting updates for keys from the
   * server.
   * Valid only for a Native Client region when client notification
   * ( {@link RegionAttributesFactory::setClientNotification} ) is true.
   *
   * @param regex The regular expression string.
   *
   * @throws IllegalArgumentException If regex is empty.
   * @throws IllegalStateException If not previously registered this regular
   * expression string.
   * @throws UnsupportedOperationException If the region is not a Native Client
   * region or
   * {@link RegionAttributesFactory::setClientNotification} is false.
   * @throws CacheServerException If an exception is received from the Java
   * cache server.
   * @throws NotConnectedException if it is not connected to the cache because
   * the client
   *         cannot establish usable connections to any of the servers given to
   * it
   *         For pools configured with locators, if no locators are available,
   * the cause
   *         of NotConnectedException is set to NoAvailableLocatorsException.
   * @throws RegionDestroyedException If region destroy is pending.
   * @throws UnknownException For other exceptions.
   * @throws TimeoutException if operation timed out
   */
  virtual void unregisterRegex(const std::string& regex) = 0;

  /**
   * Gets values for an array of keys from the local cache or server.
   * If value for a key is not present locally then it is requested from the
   * java server. The value returned is not copied, so multi-threaded
   * applications should not modify the value directly,
   * but should use the update methods.
   *<p>
   * Updates the {@link CacheStatistics::getLastAccessedTime},
   * {@link CacheStatistics::getHitCount} and {@link
   *CacheStatistics::getMissCount}
   * for this region and the entry.
   *
   * @param keys the array of keys
   * @param values Output parameter that provides the map of keys to
   *   respective values. It is ignored if nullptr, and when nullptr then at
   *least
   *   the <code>addToLocalCache</code> parameter should be true and caching
   *   should be enabled for the region to get values into the region
   *   otherwise an <code>IllegalArgumentException</code> is thrown.
   * @param exceptions Output parameter that provides the map of keys
   *   to any exceptions while obtaining the key. It is ignored if nullptr.
   * @since 8.1
   * @param aCallbackArgument an argument that is passed to the callback
   *functions.
   * It may be nullptr. Must be serializable if this operation is distributed.
   * @throws IllegalArgumentException If the array of keys is empty. Other
   *   invalid case is when the <code>values</code> parameter is nullptr, and
   *   either <code>addToLocalCache</code> is false or caching is disabled
   *   for this region.
   * @throws CacheServerException If an exception is received from the Java
   *   cache server while processing the request.
   * @throws NotConnectedException if it is not connected to the cache because
   *   the client cannot establish usable connections to any of the given
   *servers For pools configured with locators, if no locators are available,
   *the cause of NotConnectedException is set to
   *NoAvailableLocatorsException.
   * @throws RegionDestroyedException If region destroy is pending.
   * @throws TimeoutException if operation timed out.
   * @throws UnknownException For other exceptions.
   *
   * @see get
   */
  virtual HashMapOfCacheable getAll(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;

  /**
   * Executes the query on the server based on the predicate.
   * Valid only for a Native Client region.
   *
   * @param predicate The query predicate (just the WHERE clause) or the entire
   * query to execute.
   * @param timeout The time to wait for the query response, optional.
   *
   * @throws IllegalArgumentException If timeout exceeds 2147483647ms.
   * @throws QueryException if some query error occurred at the server.
   * @throws CacheServerException If an exception is received from the Java
   * cache server.
   * @throws NotConnectedException if a server connection error occurs.
   *         For pools configured with locators, if no locators are available,
   * the cause
   *         of NotConnectedException is set to NoAvailableLocatorsException.
   * @throws MessageExcepton If the message received from server could not be
   *         handled. This will be the case when an unregistered typeId is
   *         received in the reply or reply is not well formed.
   *         More information can be found in the log.
   * @throws TimeoutException if operation timed out
   * @throws CacheClosedException if the cache has been closed
   *
   * @returns A smart pointer to the SelectResults which can either be a
   * ResultSet or a StructSet.
   */
  virtual std::shared_ptr<SelectResults> query(
      const std::string& predicate,
      std::chrono::milliseconds timeout = DEFAULT_QUERY_RESPONSE_TIMEOUT) = 0;

  /**
   * Executes the query on the server based on the predicate and returns whether
   * any result exists.
   * Valid only for a Native Client region.
   * @param predicate The query predicate (just the WHERE clause) or the entire
   * query to execute.
   * @param timeout The time to wait for the response, optional.
   * @throws IllegalArgumentException If timeout exceeds 2147483647ms.
   * @throws QueryException if some query error occurred at the server.
   * @throws NotConnectedException if a server connection error occurs.
   *         For pools configured with locators, if no locators are available,
   *         the cause of NotConnectedException is set to
   *         NoAvailableLocatorsException.
   * @throws MessageExcepton If the message received from server could not be
   *         handled. This will be the case when the reply is not well formed.
   *         More information can be found in the log.
   * @throws TimeoutException if operation timed out
   * @throws CacheClosedException if the cache has been closed
   * @returns true if the result size is non-zero, false otherwise.
   */
  virtual bool existsValue(
      const std::string& predicate,
      std::chrono::milliseconds timeout = DEFAULT_QUERY_RESPONSE_TIMEOUT) = 0;

  /**
   * Executes the query on the server based on the predicate and returns a
   * single result value. Valid only for a Native Client region.
   * @param predicate The query predicate (just the WHERE clause) or the entire
   * query to execute.
   * @param timeout The time to wait for the response, optional.
   * @throws IllegalArgumentException If timeout exceeds 2147483647ms.
   * @throws QueryException if some query error occurred at the server, or more
   * than one result items are available.
   * @throws NotConnectedException if a server connection error occurs.
   *         For pools configured with locators, if no locators are available,
   * the cause
   *         of NotConnectedException is set to NoAvailableLocatorsException.
   * @throws MessageExcepton If the message received from server could not be
   *         handled. This will be the case when an unregistered typeId is
   *         received in the reply or reply is not well formed.
   *         More information can be found in the log.
   * @throws TimeoutException if operation timed out
   * @throws CacheClosedException if the cache has been closed
   * @returns A smart pointer to the single ResultSet or StructSet item, or
   * nullptr of no results are available.
   */
  virtual std::shared_ptr<Serializable> selectValue(
      const std::string& predicate,
      std::chrono::milliseconds timeout = DEFAULT_QUERY_RESPONSE_TIMEOUT) = 0;

  /**
   * Removes all of the entries for the specified keys from this region.
   * The effect of this call is equivalent to that of calling {@link #destroy}
   * on this region once for each key in the specified collection. If an entry
   * does not exist that key is skipped; EntryNotFoundException is not thrown.
   * <p>Updates the {@link CacheStatistics::getLastAccessedTime} and
   * {@link CacheStatistics::getLastModifiedTime} for this region and
   * the entries.
   * @since 8.1
   * @param keys the keys to remove from this region.
   * @param aCallbackArgument an argument that is passed to the callback
   * functions.
   *  It is ignored if nullptr. It must be serializable if this operation is
   * distributed.
   * @throws IllegalArgumentException If the array of keys is empty.
   * @throws CacheServerException If an exception is received from the Java
   *   cache server while processing the request.
   * @throws NotConnectedException if it is not connected to the cache because
   *   the client cannot establish usable connections to any of the given
   * servers For pools configured with locators, if no locators are available,
   * the cause of NotConnectedException is set to
   * NoAvailableLocatorsException.
   * @throws RegionDestroyedException If region destroy is pending.
   * @throws TimeoutException if operation timed out.
   * @throws UnknownException For other exceptions.
   * @see destroy
   */
  virtual void removeAll(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) = 0;

  /**
   * Get the size of region. For native client regions, this will give the
   * number of entries in the local cache and not on the servers.
   */
  virtual uint32_t size() = 0;

  virtual const std::shared_ptr<Pool>& getPool() const = 0;

  Cache& getCache();

  Region(const Region&) = delete;
  Region& operator=(const Region&) = delete;

 protected:
  Region(CacheImpl* cacheImpl);
  virtual ~Region();

  CacheImpl* m_cacheImpl;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_REGION_H_
