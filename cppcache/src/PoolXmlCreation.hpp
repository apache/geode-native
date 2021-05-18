#pragma once

#ifndef GEODE_POOLXMLCREATION_H_
#define GEODE_POOLXMLCREATION_H_

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

#include <string>
#include <vector>

#include <geode/ExceptionTypes.hpp>
#include <geode/Pool.hpp>
#include <geode/PoolFactory.hpp>
#include <geode/internal/geode_globals.hpp>

#include "PoolXmlCreation.hpp"

namespace apache {
namespace geode {
namespace client {
class Cache;

/**
 * Represents a {@link Pool} that is created declaratively.
 *
 * @since 3.0
 */
class PoolXmlCreation {
 private:
  /** An <code>RegionAttributesFactory</code> for creating default
   * <code>PoolAttribute</code>s */
  std::shared_ptr<PoolFactory> poolFactory;

  /** The name of this pool */
  std::string poolName;

 public:
  ~PoolXmlCreation();
  /**
   * Creates a new <code>PoolXmlCreation</code> with the given pool name.
   */
  PoolXmlCreation(std::string name, std::shared_ptr<PoolFactory> factory);

  /**
   * Creates a {@link Pool} using the
   * description provided by this <code>PoolXmlCreation</code>.
   *
   * @throws OutOfMemoryException if the memory allocation failed
   * @throws NotConnectedException if the cache is not connected
   * @throws InvalidArgumentException if the attributePtr is nullptr.
   * or if PoolAttributes is null or if poolName is null or
   * the empty string
   * @throws PoolExistsException
   * @throws CacheClosedException if the cache is closed
   *         at the time of region creation
   * @throws UnknownException otherwise
   *
   */
  std::shared_ptr<Pool> create();
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_POOLXMLCREATION_H_
