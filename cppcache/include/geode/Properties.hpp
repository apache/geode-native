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

#ifndef GEODE_PROPERTIES_H_
#define GEODE_PROPERTIES_H_

/**
 * @file
 */

#include <string>
#include <memory>
#include <unordered_map>

#include "internal/geode_globals.hpp"
#include "internal/DataSerializablePrimitive.hpp"
#include "internal/chrono/duration.hpp"
#include "CacheableKey.hpp"

namespace apache {
namespace geode {
namespace client {

using namespace apache::geode::internal::chrono::duration;

class DataInput;
class DataOutput;
class CacheableString;

/**
 * @class Properties Properties.hpp
 * Contains a set of (key, value) pair properties with key being the name of
 * the property; value, the value of the property.
 */
class APACHE_GEODE_EXPORT Properties
    : public internal::DataSerializablePrimitive {
 public:
  class Visitor {
   public:
    virtual void visit(const std::shared_ptr<CacheableKey>& key,
                       const std::shared_ptr<Cacheable>& value) = 0;
    virtual ~Visitor() {}
  };

  /**
   * Return the value for the given key, or nullptr if not found.
   *
   * @throws NullPointerException if the key is null
   */
  std::shared_ptr<CacheableString> find(const std::string& key);

  /**
   * Return the value for the given <code>CacheableKey</code>,
   * or nullptr if not found.
   *
   * @throws NullPointerException if the key is nullptr
   */
  std::shared_ptr<Cacheable> find(const std::shared_ptr<CacheableKey>& key);

  /**
   * Add or update the string value for key.
   *
   * @throws NullPointerException if the key is null
   */
  void insert(std::string key, std::string value);

  /**
   * Add or update the int value for key.
   *
   * @throws NullPointerException if the key is null
   */
  void insert(std::string key, const int value);

  /**
   * Add or update Cacheable value for CacheableKey
   *
   * @throws NullPointerException if the key is nullptr
   */
  void insert(const std::shared_ptr<CacheableKey>& key,
              const std::shared_ptr<Cacheable>& value);

  template <class _Rep, class _Period>
  void insert(std::string key,
              const std::chrono::duration<_Rep, _Period>& value) {
    insert(key, to_string(value));
  }

  /**
   * Remove the key from the collection.
   *
   * @throws NullPointerException if the key is null
   */
  void remove(const std::string& key);

  /**
   * Remove the <code>CacheableKey</code> from the collection.
   *
   * @throws NullPointerException if the key is nullptr
   */
  void remove(const std::shared_ptr<CacheableKey>& key);

  /** Execute the Visitor's <code>visit( const char* key, const char* value
   * )</code>
   * method for each entry in the collection.
   */
  void foreach (Visitor& visitor) const;

  /** Return the number of entries in the collection. */
  size_t getSize() const;

  /** Add the contents of other to this instance, replacing any existing
   * values with those from other.
   */
  void addAll(const std::shared_ptr<Properties>& other);

  /** Factory method, returns an empty collection. */
  static std::shared_ptr<Properties> create();

  /** Read property values from a file, overriding what is currently
   * in the properties object.
   */
  void load(const std::string& fileName);

  /** Return an empty instance for deserialization. */
  static std::shared_ptr<Serializable> createDeserializable();

  void toData(DataOutput& output) const override;

  void fromData(DataInput& input) override;

  DSCode getDsCode() const override;

  ~Properties() override = default;
  Properties(const Properties&) = delete;
  Properties& operator=(const Properties&) = delete;
  Properties() = default;

 private:
  HashMapOfCacheable m_map;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PROPERTIES_H_
