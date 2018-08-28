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

#ifndef GEODE_REGIONENTRY_H_
#define GEODE_REGIONENTRY_H_

#include "internal/geode_globals.hpp"
#include "CacheableKey.hpp"
#include "CacheStatistics.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

class RegionInternal;
class Region;
/**
 * @class RegionEntry RegionEntry.hpp
 * An object in a Region that represents an entry, i.e., a key-value pair.
 *
 * This object's
 * operations are not distributed, do not acquire any locks, and do not affect
 * <code>CacheStatistics</code>.
 *<p>
 * Unless otherwise noted, all of these methods throw a
 * <code>CacheClosedException</code> if the Cache is closed at the time of
 * invocation, or an <code>EntryDestroyedException</code> if the entry has been
 * destroyed.
 */
class APACHE_GEODE_EXPORT RegionEntry {
 public:
  /** Returns the key for this entry.
   *
   * @return the key for this entry
   */
  std::shared_ptr<CacheableKey> getKey();

  /** Returns the value of this entry in the local cache. Does not invoke
   * a <code>CacheLoader</code>,
   *
   * @return the value or <code>nullptr</code> if this entry is invalid
   */
  std::shared_ptr<Cacheable> getValue();

  /** Returns the region that contains this entry.
   *
   * @return the Region that contains this entry
   */
  std::shared_ptr<Region> getRegion();

  /** Returns the statistics for this entry.
   *
   * @return the CacheStatistics for this entry
   * @throws StatisticsDisabledException if statistics have been disabled for
   * this region
   */
  std::shared_ptr<CacheStatistics> getStatistics();

  /**
   * Returns whether this entry has been destroyed.
   * <p>Does not throw a <code>EntryDestroyedException</code> if this entry
   * has been destroyed.
   *
   * @return true if this entry has been destroyed
   */
  bool isDestroyed() const;

  /**
    * @brief constructors
    * created by region
    */
  RegionEntry(const std::shared_ptr<Region>& region,
              const std::shared_ptr<CacheableKey>& key,
              const std::shared_ptr<Cacheable>& value);

  /**
   * @brief destructor
   */
  virtual ~RegionEntry();

 private:
  std::shared_ptr<Region> m_region;
  std::shared_ptr<CacheableKey> m_key;
  std::shared_ptr<Cacheable> m_value;
  std::shared_ptr<CacheStatistics> m_statistics;
  bool m_destroyed;
  friend class RegionInternal;

};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_REGIONENTRY_H_
