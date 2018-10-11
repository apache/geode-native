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

#ifndef GEODE_SELECTRESULTS_H_
#define GEODE_SELECTRESULTS_H_

/**
 * @file
 */

#include "CacheableBuiltins.hpp"
#include "ExceptionTypes.hpp"
#include "Serializable.hpp"
#include "internal/geode_globals.hpp"

namespace apache {
namespace geode {
namespace client {

class SelectResultsIterator;

/**
 * @class SelectResults SelectResults.hpp
 *
 * A SelectResults is obtained by executing a Query on the server.
 * This can either be a ResultSet or a StructSet.
 */
class APACHE_GEODE_EXPORT SelectResults {
 public:
  virtual ~SelectResults() noexcept = default;

  /**
   * Get the size of the SelectResults.
   *
   * @returns the number of items in the SelectResults.
   */
  virtual size_t size() const = 0;

  /**
   * Index operator to directly access an item in the SelectResults.
   *
   * @param index the index number of the required item.
   * @throws IllegalArgumentException if the index is out of bounds.
   * @returns A smart pointer to the item indexed.
   */
  virtual const std::shared_ptr<Serializable> operator[](
      size_t index) const = 0;

  /**
   * Interface of an iterator for <code>SelectResults</code>.
   */
  typedef std::vector<std::shared_ptr<Cacheable>>::iterator iterator;

  /**
   * Get an iterator pointing to the start of <code>SelectResults</code>.
   */
  virtual iterator begin() = 0;

  /**
   * Get an iterator pointing to the end of <code>SelectResults</code>.
   */
  virtual iterator end() = 0;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_SELECTRESULTS_H_
