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

/**
 * @file
 */

#include <functional>
#include <memory>

#include "geode_globals.hpp"

namespace apache {
namespace geode {
namespace client {

class DataOutput;
class DataInput;
class Cache;
class PdxSerializable;
class Serializable;
class CacheableString;

typedef void (*CliCallbackMethod)(Cache &cache);

/** @brief signature of functions passed to registerType. Such functions
 * should return an empty instance of the type they represent. The instance
 * will typically be initialized immediately after creation by a call to
 * fromData().
 */
using TypeFactoryMethod = std::function<Serializable *()>;

typedef PdxSerializable *(*TypeFactoryMethodPdx)();
/**
 * @class Serializable Serializable.hpp
 * This abstract base class is the superclass of all user objects
 * in the cache that can be serialized.
 */

class CPPCACHE_EXPORT Serializable
    : public std::enable_shared_from_this<Serializable> {
 public:
  /**
   *@brief serialize this object
   **/
  virtual void toData(DataOutput &output) const = 0;

  /**
   *@brief deserialize this object.
   **/
  virtual void fromData(DataInput &input) = 0;

  /**
   *@brief Return the classId of the instance being serialized.
   * This is used by deserialization to determine what instance
   * type to create and deserialize into.
   *
   * The classId must be unique within an application suite.
   * Using a negative value may result in undefined behavior.
   */
  virtual int32_t classId() const = 0;

  /**
   *@brief return the typeId byte of the instance being serialized.
   * This is used by deserialization to determine what instance
   * type to create and deserialize into.
   *
   * Note that this should not be overridden by custom implementations
   * and is reserved only for builtin types.
   */
  virtual int8_t typeId() const;

  /**
   * @brief return the Data Serialization Fixed ID type.
   * This is used to determine what instance type to create and deserialize
   * into.
   *
   * Note that this should not be overridden by custom implementations
   * and is reserved only for builtin types.
   */
  virtual int8_t DSFID() const;

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
   * the subclasses.
   * The default implementation renders the classname.
   */
  virtual std::string toString() const;

  /** Factory method that creates the Serializable object that matches the type
   * of value.
   * For customer defined derivations of Serializable, the method
   * apache::geode::client::createValue
   * may be overloaded. For pointer types (e.g. char*) the method
   * apache::geode::client::createValueArr may be overloaded.
   */
  template <class PRIM>
  inline static std::shared_ptr<Serializable> create(const PRIM value);

  /**
   * @brief destructor
   */
  virtual ~Serializable() = default;

 protected:
  Serializable() = default;
  Serializable(const Serializable &other) = default;
  Serializable &operator=(const Serializable &other) = default;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_SERIALIZABLE_H_
