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

#ifndef GEODE_GEMFIRECACHE_H_
#define GEODE_GEMFIRECACHE_H_

#include "RegionService.hpp"
#include "internal/geode_globals.hpp"

namespace apache {
namespace geode {
namespace client {

class SystemProperties;

/**
 * GeodeCache represents the singleton cache that must be created
 * in order to connect to Geode server.
 * Users must create a {@link Cache}.
 * Instances of this interface are created using one of the following methods:
 * <ul>
 * <li> {@link ClientCacheFactory#create()} creates a client instance of {@link
 * Cache}.
 * </ul>
 *
 */

class APACHE_GEODE_EXPORT GeodeCache : public RegionService {
 public:
  ~GeodeCache() override = 0;

  /** Returns the name of this cache.
   * @return the string name of this cache
   */
  virtual const std::string& getName() const = 0;

  /** Initialize the cache by the contents of an xml file
   * @param  cacheXml
   *         The xml file
   * @throws OutOfMemoryException
   * @throws CacheXmlException
   *         Something went wrong while parsing the XML
   * @throws IllegalStateException
   *         If xml file is well-flrmed but not valid
   * @throws RegionExistsException if a region is already in
   *         this cache
   * @throws CacheClosedException if the cache is closed
   *         at the time of region creation
   * @throws UnknownException otherwise
   */
  virtual void initializeDeclarativeCache(const std::string& cacheXml) = 0;

  virtual SystemProperties& getSystemProperties() const = 0;

  /**
   * Returns whether Cache saves unread fields for Pdx types.
   */
  virtual bool getPdxIgnoreUnreadFields() const = 0;

  /**
   * Returns whether { @link PdxInstance} is preferred for PDX types instead of
   * C++ object.
   */
  virtual bool getPdxReadSerialized() const = 0;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_GEMFIRECACHE_H_
