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

#ifndef GEODE_SERIALIZABLE_H_
#define GEODE_SERIALIZABLE_H_

#include <functional>
#include <memory>
#include <string>

#include "internal/geode_globals.hpp"

namespace apache {
namespace geode {
namespace client {

class DataOutput;
class DataInput;
class Cache;
class PdxSerializable;
class Serializable;

/** @brief signature of functions passed to registerType. Such functions
 * should return an empty instance of the type they represent. The instance
 * will typically be initialized immediately after creation by a call to
 * fromData().
 */
using TypeFactoryMethod = std::function<std::shared_ptr<Serializable>()>;

using TypeFactoryMethodPdx = std::function<std::shared_ptr<PdxSerializable>()>;

/**
 * @class Serializable Serializable.hpp
 * This base class is the superclass of all user objects
 * in the cache that can be serialized.
 */
class APACHE_GEODE_EXPORT Serializable {
 public:
  /**
   *@brief return the size in bytes of the instance being serialized.
   * This is used to determine whether the cache is using up more
   * physical memory than it has been configured to use. The method can
   * return zero if the user does not require the ability to control
   * cache memory utilization.
   * Note that you must implement this only if you use the HeapLRU feature.
   */
  virtual size_t objectSize() const;

  /**
   * Display this object as 'string', which depends on the implementation in
   * the subclasses. The default implementation renders the classname.
   */
  virtual std::string toString() const;

  /** Factory method that creates the Serializable object that matches the type
   * of value.
   * For user defined derivations of Serializable, the method
   * apache::geode::client::Serializable::create may be overloaded.
   */
  template <class _T>
  static std::shared_ptr<Serializable> create(_T value);

  template <class _T>
  inline static std::shared_ptr<Serializable> create(
      const std::shared_ptr<_T>& value) {
    return value;
  }

  virtual ~Serializable() noexcept = default;

 protected:
};

typedef Serializable Cacheable;

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_SERIALIZABLE_H_
