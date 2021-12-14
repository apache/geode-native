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

#ifndef GEODE_CACHEABLEBUILTINTEMPLATES_H_
#define GEODE_CACHEABLEBUILTINTEMPLATES_H_

#include <cstring>

#include "../CacheableKey.hpp"
#include "../CacheableString.hpp"
#include "../Serializable.hpp"
#include "../Serializer.hpp"
#include "CacheableKeys.hpp"

namespace apache {
namespace geode {
namespace client {
namespace internal {

/**
 * Template class for primitive key types.
 */
template <typename TObj, DSCode TYPEID>
class CacheableKeyPrimitive : public virtual DataSerializablePrimitive,
                              public virtual CacheableKey {
 private:
  TObj value_;

 public:
  inline CacheableKeyPrimitive() = default;

  inline explicit CacheableKeyPrimitive(const TObj value) : value_(value) {}

  ~CacheableKeyPrimitive() noexcept override = default;

  /** Gets the contained value. */
  inline TObj value() const { return value_; }

  void toData(DataOutput& output) const override {
    apache::geode::client::serializer::writeObject(output, value_);
  }

  void fromData(DataInput& input) override {
    apache::geode::client::serializer::readObject(input, value_);
  }

  DSCode getDsCode() const override { return TYPEID; }

  std::string toString() const override { return std::to_string(value_); }

  int32_t hashcode() const override { return internal::hashcode(value_); }

  bool operator==(const CacheableKey& other) const override {
    if (auto&& otherPrimitive =
            dynamic_cast<const CacheableKeyPrimitive*>(&other)) {
      return internal::equals(value_, otherPrimitive->value_);
    }

    return false;
  }

  /** Return true if this key matches other key value. */
  inline bool operator==(const TObj other) const {
    return internal::equals(value_, other);
  }

  virtual size_t objectSize() const override {
    return sizeof(CacheableKeyPrimitive);
  }

  /** Factory function registered with serialization registry. */
  inline static std::shared_ptr<Serializable> createDeserializable() {
    return std::make_shared<CacheableKeyPrimitive<TObj, TYPEID>>();
  }

  /** Factory function to create a new default instance. */
  inline static std::shared_ptr<CacheableKeyPrimitive<TObj, TYPEID>> create() {
    return std::make_shared<CacheableKeyPrimitive<TObj, TYPEID>>();
  }

  /** Factory function to create an instance with the given value. */
  inline static std::shared_ptr<CacheableKeyPrimitive<TObj, TYPEID>> create(
      const TObj value) {
    return std::make_shared<CacheableKeyPrimitive<TObj, TYPEID>>(value);
  }
};

template <typename T, DSCode GeodeTypeId>
class CacheableArrayPrimitive : public DataSerializablePrimitive {
 protected:
  DSCode getDsCode() const override { return GeodeTypeId; }

  size_t objectSize() const override {
    return static_cast<uint32_t>(
        apache::geode::client::serializer::objectArraySize(m_value));
  }

 private:
  std::vector<T> m_value;

 public:
  inline CacheableArrayPrimitive() = default;

  template <typename TT>
  explicit CacheableArrayPrimitive(TT&& value)
      : m_value(std::forward<TT>(value)) {}

  ~CacheableArrayPrimitive() noexcept override = default;

  CacheableArrayPrimitive(const CacheableArrayPrimitive& other) = delete;

  CacheableArrayPrimitive& operator=(const CacheableArrayPrimitive& other) =
      delete;

  inline const std::vector<T>& value() const { return m_value; }

  inline int32_t length() const { return static_cast<int32_t>(m_value.size()); }

  static std::shared_ptr<Serializable> createDeserializable() {
    return std::make_shared<CacheableArrayPrimitive<T, GeodeTypeId>>();
  }

  inline static std::shared_ptr<CacheableArrayPrimitive<T, GeodeTypeId>>
  create() {
    return std::make_shared<CacheableArrayPrimitive<T, GeodeTypeId>>();
  }

  inline static std::shared_ptr<CacheableArrayPrimitive<T, GeodeTypeId>> create(
      const std::vector<T>& value) {
    return std::make_shared<CacheableArrayPrimitive<T, GeodeTypeId>>(value);
  }

  inline static std::shared_ptr<CacheableArrayPrimitive<T, GeodeTypeId>> create(
      std::vector<T>&& value) {
    return std::make_shared<CacheableArrayPrimitive<T, GeodeTypeId>>(
        std::move(value));
  }

  inline T operator[](int32_t index) const {
    if (index >= static_cast<int32_t>(m_value.size())) {
      throw OutOfRangeException(
          "CacheableArrayPrimitive::operator[]: Index out of range.");
    }
    return m_value[index];
  }

  virtual void toData(DataOutput& output) const override {
    apache::geode::client::serializer::writeArrayObject(output, m_value);
  }

  virtual void fromData(DataInput& input) override {
    m_value = apache::geode::client::serializer::readArrayObject<T>(input);
  }
};

/** Template class for container Cacheable types. */
template <typename TBase, DSCode TYPEID>
class CacheableContainerPrimitive : public DataSerializablePrimitive,
                                    public TBase {
 public:
  inline CacheableContainerPrimitive() : TBase() {}

  inline explicit CacheableContainerPrimitive(const int32_t n) : TBase(n) {}

  void toData(DataOutput& output) const override {
    apache::geode::client::serializer::writeObject(output, *this);
  }

  void fromData(DataInput& input) override {
    apache::geode::client::serializer::readObject(input, *this);
  }

  DSCode getDsCode() const override { return TYPEID; }

  size_t objectSize() const override { return serializer::objectSize(*this); }

  /** Factory function registered with serialization registry. */
  inline static std::shared_ptr<Serializable> createDeserializable() {
    return std::make_shared<CacheableContainerPrimitive>();
  }

  /** Factory function to create a default instance. */
  inline static std::shared_ptr<CacheableContainerPrimitive> create() {
    return std::make_shared<CacheableContainerPrimitive>();
  }

  /** Factory function to create an instance with the given size. */
  inline static std::shared_ptr<CacheableContainerPrimitive> create(
      const int32_t n) {
    return std::make_shared<CacheableContainerPrimitive>(n);
  }
};

}  // namespace internal
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHEABLEBUILTINTEMPLATES_H_
