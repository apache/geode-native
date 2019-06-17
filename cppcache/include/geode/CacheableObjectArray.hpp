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

#ifndef GEODE_CACHEABLEOBJECTARRAY_H_
#define GEODE_CACHEABLEOBJECTARRAY_H_

#include <memory>
#include <vector>

#include "Serializable.hpp"
#include "internal/DSCode.hpp"
#include "internal/DataSerializablePrimitive.hpp"
#include "internal/geode_globals.hpp"

/** @file
 */

namespace apache {
namespace geode {
namespace client {

class DataInput;
class DataOutput;
class Serializable;

/**
 * Implements an immutable Vector of <code>Cacheable</code> objects
 * that can serve as a distributable object for caching.
 */

class APACHE_GEODE_EXPORT CacheableObjectArray
    : public internal::DataSerializablePrimitive,
      public std::vector<std::shared_ptr<Cacheable>> {
 public:
  /** Constructor, used for deserialization. */
  CacheableObjectArray();

  /** Create a vector with n elements allocated. */
  CacheableObjectArray(int32_t n);

  ~CacheableObjectArray() noexcept override = default;

  CacheableObjectArray& operator=(const CacheableObjectArray& other) = delete;
  CacheableObjectArray(const CacheableObjectArray& other) = delete;

  void toData(DataOutput& output) const override;

  virtual void fromData(DataInput& input) override;

  /**
   * @brief creation function for java Object[]
   */
  static std::shared_ptr<Serializable> createDeserializable();

  internal::DSCode getDsCode() const override;

  /**
   * Factory method for creating the default instance of CacheableObjectArray.
   */
  static std::shared_ptr<CacheableObjectArray> create();

  /**
   * Factory method for creating an instance of CacheableObjectArray with
   * given size.
   */
  static std::shared_ptr<CacheableObjectArray> create(int32_t n);

  virtual size_t objectSize() const override;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHEABLEOBJECTARRAY_H_
