#pragma once

#ifndef GEODE_CACHEFACTORY_H_
#define GEODE_CACHEFACTORY_H_

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

#include "geode_globals.hpp"
#include "geode_types.hpp"
#include "DistributedSystem.hpp"
#include "Cache.hpp"
#include "CacheAttributes.hpp"
#include "PoolFactory.hpp"
#include <map>
#include <memory>
/**
 * @file
 */

#define DEFAULT_POOL_NAME "default_geodeClientPool"

namespace apache {
namespace geode {
namespace client {

class CppCacheLibrary;
/**
 * @class CacheFactory CacheFactory.hpp
 * Top level class for configuring and using Geode on a client.This should be
 *called once to create {@link Cache}.
 *<p>
 * For the default values for the pool attributes see {@link PoolFactory}.
 * To create additional {@link Pool}s see {@link PoolManager}
 */
class CPPCACHE_EXPORT CacheFactory : public SharedBase {
 public:
  /**
   * To create the instance of {@link CacheFactory}
   * @param dsProps
   *        Properties which are applicable at client level.
   */
  static CacheFactoryPtr createCacheFactory(
      const PropertiesPtr& dsProps = NULLPTR);

  /**
   * To create the instance of {@link Cache}.
   */
  CachePtr create(DistributedSystemPtr distributedSystemPtr = NULLPTR);

  /**
   * Gets the instance of {@link Cache} produced by an
   * earlier call to {@link CacheFactory::create}.
   * @param system the <code>DistributedSystem</code> the cache was created
   * with.
   * @return the {@link Cache} associated with the specified system.
   * @throws CacheClosedException if a cache has not been created
   * or the created one is {@link Cache::isClosed closed}
   * @throws EntryNotFoundException if a cache with specified system not found
   */
//  static CachePtr getInstance(const DistributedSystemPtr& system);

  /**
   * Gets the instance of {@link Cache} produced by an
   * earlier call to {@link CacheFactory::create}, even if it has been closed.
   * @param system the <code>DistributedSystem</code> the cache was created
   * with.
   * @return the {@link Cache} associated with the specified system.
   * @throws CacheClosedException if a cache has not been created
   * @throws EntryNotFoundException if a cache with specified system is not
   * found
   */
//  static CachePtr getInstanceCloseOk(const DistributedSystemPtr& system);

  /**
   * Gets an arbitrary open instance of {@link Cache} produced by an
   * earlier call to {@link CacheFactory::create}.
   * @throws CacheClosedException if a cache has not been created
   * or the only created one is {@link Cache::isClosed closed}
   */
//  static CachePtr getAnyInstance();

  /** Returns the version of the cache implementation.
   * For the 1.0 release of Geode, the string returned is <code>1.0</code>.
   * @return the version of the cache implementation as a <code>String</code>
   */
  const char* getVersion();

  /** Returns the product description string including product name and version.
   */

   /**
  * Control whether pdx ignores fields that were unread during deserialization.
  * The default is to preserve unread fields be including their data during
  * serialization.
  * But if you configure the cache to ignore unread fields then their data will
  * be lost
  * during serialization.
  * <P>You should only set this attribute to <code>true</code> if you know this
  * member
  * will only be reading cache data. In this use case you do not need to pay the
  * cost
  * of preserving the unread fields since you will never be reserializing pdx
  * data.
  *
  * @param ignore <code>true</code> if fields not read during pdx
  * deserialization should be ignored;
  * <code>false</code>, the default, if they should be preserved.
  *
  *
  * @return this CacheFactory
  * @since 3.6
  */
  CacheFactoryPtr setPdxIgnoreUnreadFields(bool ignore);

  /** Sets the object preference to PdxInstance type.
  * When a cached object that was serialized as a PDX is read
  * from the cache a {@link PdxInstance} will be returned instead of the actual
  * domain class.
  * The PdxInstance is an interface that provides run time access to
  * the fields of a PDX without deserializing the entire PDX.
  * The PdxInstance implementation is a light weight wrapper
  * that simply refers to the raw bytes of the PDX that are kept
  * in the cache. Using this method applications can choose to
  * access PdxInstance instead of C++ object.
  * <p>Note that a PdxInstance is only returned if a serialized PDX is found in
  * the cache.
  * If the cache contains a deserialized PDX, then a domain class instance is
  * returned instead of a PdxInstance.
  *
  *  @param pdxReadSerialized true to prefer PdxInstance
  *  @return this ClientCacheFactory
  */
  CacheFactoryPtr setPdxReadSerialized(bool pdxReadSerialized);

  /**
   * Sets a geode property that will be used when creating the {link @Cache}.
   * @param name the name of the geode property
   * @param value the value of the geode property
   * @return a reference to <code>this</code>
   * @since 3.5
   */
  CacheFactoryPtr set(const char* name, const char* value);

 private:
  PropertiesPtr dsProp;
  bool ignorePdxUnreadFields;
  bool pdxReadSerialized;

  class CacheFactoryImpl;
  std::unique_ptr<CacheFactoryImpl> pimpl;

//  PoolFactoryPtr getPoolFactory();

  CachePtr create(const char* name, DistributedSystemPtr system = NULLPTR,
                  const char* cacheXml = 0,
                  const CacheAttributesPtr& attrs = NULLPTR);

  void create_(const char* name, DistributedSystemPtr& system,
                      const char* id_data, CachePtr& cptr,
                      bool ignorePdxUnreadFields, bool readPdxSerialized);

  // no instances allowed
  CacheFactory();
  CacheFactory(const PropertiesPtr dsProps);
  ~CacheFactory();


//  // Set very first time some creates cache
//  static CacheFactoryPtr s_factory;
//  std::map<std::string, CachePtr> m_cacheMap;
  void cleanup();
  friend class CppCacheLibrary;
  friend class RegionFactory;
  friend class RegionXmlCreation;
  friend class CacheXmlCreation;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHEFACTORY_H_
