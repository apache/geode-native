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

#ifndef GEODE_POOLMANAGER_H_
#define GEODE_POOLMANAGER_H_

#include <iosfwd>
#include <memory>
#include <string>
#include <unordered_map>

#include "Cache.hpp"
#include "Pool.hpp"
#include "PoolFactory.hpp"
#include "Region.hpp"
#include "geode_base.hpp"
#include "geode_globals.hpp"

namespace apache {
namespace geode {
namespace client {

class CacheImpl;
class FunctionService;
class PoolManagerImpl;
class ThinClientPoolDM;
class Cache;
class Pool;
class PoolFactory;
class Region;
class RegionFactory;

typedef std::unordered_map<std::string, std::shared_ptr<Pool>> HashMapOfPools;

/**
 * Manages creation and access to {@link Pool connection pools} for clients.
 * <p>
 * To create a pool get a factory by calling {@link #createFactory}.
 * <p>
 * To find an existing pool by name call {@link #find}.
 * <p>
 * To get rid of all created pools call {@link #close}.
 *
 *
 */
class CPPCACHE_EXPORT PoolManager {
 public:

  /**
   * Creates a new {@link PoolFactory pool factory},
   * which is used to configure and create new {@link Pool}s.
   * @return the new pool factory
   */
  std::shared_ptr<PoolFactory> createFactory() const;

  /**
   * Returns a map containing all the pools in this manager.
   * The keys are pool names
   * and the values are {@link Pool} instances.
   * <p> The map contains the pools that this manager knows of at the time of
   * this call.
   * The map is free to be changed without affecting this manager.
   * @return a Map that is a snapshot of all the pools currently known to this
   * manager.
   */
  const HashMapOfPools& getAll() const;

  /**
   * Find by name an existing connection pool returning
   * the existing pool or <code>nullptr</code> if it does not exist.
   * @param name is the name of the connection pool
   * @return the existing connection pool or <code>nullptr</code> if it does not
   * exist.
   */
  std::shared_ptr<Pool> find(const std::string& name) const;

  /**
   * Find the pool used by the given region.
   * @param region is the region that is using the pool.
   * @return the pool used by that region or <code> nullptr </code> if the
   * region does
   * not have a pool.
   */
  std::shared_ptr<Pool> find(std::shared_ptr<Region> region) const;

  /**
   * Unconditionally destroys all created pools that are in this manager.
   * @param keepAlive defines whether the server should keep the durable
   * client's subscriptions alive for the <code>durable-client-timeout</code>.
   * @see DistributedSystem#connect for a description of
   * <code>durable-client-timeout</code>.
   */
  void close(bool keepAlive = false);

 private:
  void removePool(const std::string& name);

  void addPool(std::string name, const std::shared_ptr<Pool>& pool);

  std::shared_ptr<Pool> getDefaultPool() const;

  std::shared_ptr<PoolManagerImpl> m_pimpl;

  PoolManager(CacheImpl* cache);

  friend Cache;
  friend CacheImpl;
  friend RegionFactory;
  friend PoolFactory;
  friend ThinClientPoolDM;
  friend FunctionService;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_POOLMANAGER_H_
