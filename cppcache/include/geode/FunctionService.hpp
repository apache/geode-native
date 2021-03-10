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

#ifndef GEODE_FUNCTIONSERVICE_H_
#define GEODE_FUNCTIONSERVICE_H_

#include <memory>

#include "Execution.hpp"
#include "internal/geode_globals.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {
class Pool;
class Region;
class RegionService;
class Execution;

/**
 * @class FunctionService FunctionService.hpp
 * entry point for function execution
 * @see Execution
 */
class APACHE_GEODE_EXPORT FunctionService {
 public:
  /**
   * Returns a {@link Execution} object that can be used to execute a data
   * dependent function on the specified Region.<br>
   * When invoked from a Geode client, the method returns an Execution
   * instance that sends a message to one of the connected servers as specified
   * by the {@link Pool} for the region. Depending on the filters setup on the
   * {@link Execution}, the function is executed on all Geode members that
   * define the data region, or a subset of members.
   * {@link Execution::withFilter(filter)}).
   *
   * @param region
   * If Pool is multiusersecure mode then one need to pass nstance of Region
   * from RegionService.
   *
   * @return Execution
   * @throws NullPointerException
   *                 if the region passed in is nullptr
   */
  static Execution onRegion(const std::shared_ptr<Region>& region);

  /**
   * Returns a {@link Execution} object that can be used to execute a data
   * independent function on a server in the provided {@link Pool}.
   * <p>
   * If the server goes down while dispatching or executing the function, an
   * Exception will be thrown.
   * @param pool from which to chose a server for execution
   * @return Execution
   * @throws NullPointerException
   *                 if Pool instance passed in is nullptr
   * @throws UnsupportedOperationException
   *                 if Pool is in multiusersecure Mode
   */
  inline static Execution onServer(const std::shared_ptr<Pool>& pool) {
    return onServerWithPool(pool);
  }

  /**
   * Returns a {@link Execution} object that can be used to execute a data
   * independent function on a server where Cache is attached.
   * <p>
   * If the server goes down while dispatching or executing the function, an
   * Exception will be thrown.
   * @param cache
   *        cache from which to chose a server for execution
   * @return Execution
   * @throws NullPointerException
   *                 if Pool instance passed in is nullptr
   * @throws UnsupportedOperationException
   *                 if Pool is in multiusersecure Mode
   */
  inline static Execution onServer(RegionService& regionService) {
    return onServerWithCache(regionService);
  }

  /**
   * Returns a {@link Execution} object that can be used to execute a data
   * independent function on all the servers in the provided {@link Pool}.
   * If one of the servers goes down while dispatching or executing the function
   * on the server, an Exception will be thrown.
   *
   * @param pool the set of servers to execute the function
   * @return Execution
   * @throws NullPointerException
   *                 if Pool instance passed in is nullptr
   * @throws UnsupportedOperationException
   *                 if Pool is in multiusersecure Mode
   */
  inline static Execution onServers(const std::shared_ptr<Pool>& pool) {
    return onServersWithPool(pool);
  }

  /**
   * Returns a {@link Execution} object that can be used to execute a data
   * independent function on all the servers where Cache is attached.
   * If one of the servers goes down while dispatching or executing the function
   * on the server, an Exception will be thrown.
   *
   * @param cache
   *        the {@link Cache} where function need to execute.
   * @return Execution
   * @throws NullPointerException
   *                 if Pool instance passed in is nullptr
   * @throws UnsupportedOperationException
   *                 if Pool is in multiusersecure Mode
   */
  inline static Execution onServers(RegionService& regionService) {
    return onServersWithCache(regionService);
  }

  virtual ~FunctionService() noexcept = default;

 private:
  static Execution onServerWithPool(const std::shared_ptr<Pool>& pool);

  static Execution onServerWithCache(RegionService& regionService);

  static Execution onServersWithPool(const std::shared_ptr<Pool>& pool);

  static Execution onServersWithCache(RegionService& regionService);
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_FUNCTIONSERVICE_H_
