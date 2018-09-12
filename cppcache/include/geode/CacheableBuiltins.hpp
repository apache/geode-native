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

#ifndef GEODE_CACHEABLEBUILTINS_H_
#define GEODE_CACHEABLEBUILTINS_H_

/** @file CacheableBuiltins.hpp
 *  @brief Contains generic template definitions for Cacheable types
 *         and instantiations for built-in types.
 */

#include <cstring>

#include "Serializable.hpp"
#include "CacheableKey.hpp"
#include "Serializer.hpp"
#include "internal/CacheableKeys.hpp"
#include "CacheableString.hpp"

namespace apache {
namespace geode {
namespace client {

using internal::DataSerializablePrimitive;

/**
 * Template class for primitive key types.
 */
template <typename TObj, DSCode TYPEID>
class APACHE_GEODE_EXPORT CacheableKeyPrimitive
    : public DataSerializablePrimitive,
      public CacheableKey {
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
    // TODO change to using dynamic_cast of this template specialization

    if (const auto otherPrimitive =
            dynamic_cast<const DataSerializablePrimitive*>(&other)) {
      if (otherPrimitive->getDsCode() != TYPEID) {
        return false;
      }
    }
    auto& otherValue = static_cast<const CacheableKeyPrimitive&>(other);
    return internal::equals(value_, otherValue.value_);
  }

  /** Return true if this key matches other key value. */
  inline bool operator==(const TObj other) const {
    return internal::equals(value_, other);
  }

  virtual size_t objectSize() const override {
    return sizeof(CacheableKeyPrimitive);
  }

  /** Factory function registered with serialization registry. */
  static std::shared_ptr<Serializable> createDeserializable() {
    return std::make_shared<CacheableKeyPrimitive>();
  }

  /** Factory function to create a new default instance. */
  inline static std::shared_ptr<CacheableKeyPrimitive> create() {
    return std::make_shared<CacheableKeyPrimitive>();
  }

  /** Factory function to create an instance with the given value. */
  inline static std::shared_ptr<CacheableKeyPrimitive> create(
      const TObj value) {
    return std::make_shared<CacheableKeyPrimitive>(value);
  }
};

/** Function to copy an array from source to destination. */
template <typename TObj>
inline void copyArray(TObj* dest, const TObj* src, size_t length) {
  std::memcpy(dest, src, length * sizeof(TObj));
}

/**
 * Function to copy an array of <code>std::shared_ptr</code>s from
 * source to destination.
 */
template <typename TObj>
inline void copyArray(std::shared_ptr<TObj>* dest,
                      const std::shared_ptr<TObj>* src, size_t length) {
  for (size_t index = 0; index < length; index++) {
    dest[index] = src[index];
  }
}

/**
 * An immutable wrapper for bool that can serve as
 * a distributable key object for caching.
 */
using CacheableBoolean =
    CacheableKeyPrimitive<bool, internal::DSCode::CacheableBoolean>;
template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(bool value) {
  return CacheableBoolean::create(value);
}
template <>
inline std::shared_ptr<Cacheable> Serializable::create(bool value) {
  return CacheableBoolean::create(value);
}

/**
 * An immutable wrapper for byte that can serve as
 * a distributable key object for caching.
 */
using CacheableByte =
    CacheableKeyPrimitive<int8_t, internal::DSCode::CacheableByte>;
template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(int8_t value) {
  return CacheableByte::create(value);
}
template <>
inline std::shared_ptr<Cacheable> Serializable::create(int8_t value) {
  return CacheableByte::create(value);
}

/**
 * An immutable wrapper for doubles that can serve as
 * a distributable key object for caching.
 */
using CacheableDouble =
    CacheableKeyPrimitive<double, internal::DSCode::CacheableDouble>;
template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(double value) {
  return CacheableDouble::create(value);
}
template <>
inline std::shared_ptr<Cacheable> Serializable::create(double value) {
  return CacheableDouble::create(value);
}

/**
 * An immutable wrapper for floats that can serve as
 * a distributable key object for caching.
 */
using CacheableFloat =
    CacheableKeyPrimitive<float, internal::DSCode::CacheableFloat>;
template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(float value) {
  return CacheableFloat::create(value);
}
template <>
inline std::shared_ptr<Cacheable> Serializable::create(float value) {
  return CacheableFloat::create(value);
}

/**
 * An immutable wrapper for 16-bit integers that can serve as
 * a distributable key object for caching.
 */
using CacheableInt16 =
    CacheableKeyPrimitive<int16_t, internal::DSCode::CacheableInt16>;
template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(int16_t value) {
  return CacheableInt16::create(value);
}
template <>
inline std::shared_ptr<Cacheable> Serializable::create(int16_t value) {
  return CacheableInt16::create(value);
}

/**
 * An immutable wrapper for 132-bit integers that can serve as
 * a distributable key object for caching.
 */
using CacheableInt32 =
    CacheableKeyPrimitive<int32_t, internal::DSCode::CacheableInt32>;
template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(int32_t value) {
  return CacheableInt32::create(value);
}
template <>
inline std::shared_ptr<Cacheable> Serializable::create(int32_t value) {
  return CacheableInt32::create(value);
}

/**
 * An immutable wrapper for 64-bit integers that can serve as
 * a distributable key object for caching.
 */
using CacheableInt64 =
    CacheableKeyPrimitive<int64_t, internal::DSCode::CacheableInt64>;
template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(int64_t value) {
  return CacheableInt64::create(value);
}
template <>
inline std::shared_ptr<Cacheable> Serializable::create(int64_t value) {
  return CacheableInt64::create(value);
}

/**
 * An immutable wrapper for 64-bit integers that can serve as
 * a distributable key object for caching.
 */
using CacheableCharacter =
    CacheableKeyPrimitive<char16_t, internal::DSCode::CacheableCharacter>;
template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(char16_t value) {
  return CacheableCharacter::create(value);
}
template <>
inline std::shared_ptr<Cacheable> Serializable::create(char16_t value) {
  return CacheableCharacter::create(value);
}

// Instantiations for array built-in Cacheables

template <typename T, DSCode GeodeTypeId>
class APACHE_GEODE_EXPORT CacheableArrayPrimitive
    : public DataSerializablePrimitive {
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
  CacheableArrayPrimitive(TT&& value) : m_value(std::forward<TT>(value)) {}

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

/**
 * An immutable wrapper for byte arrays that can serve as
 * a distributable object for caching.
 */
using CacheableBytes = CacheableArrayPrimitive<int8_t, DSCode::CacheableBytes>;

/**
 * An immutable wrapper for array of booleans that can serve as
 * a distributable object for caching.
 */
using BooleanArray = CacheableArrayPrimitive<bool, DSCode::BooleanArray>;

/**
 * An immutable wrapper for array of wide-characters that can serve as
 * a distributable object for caching.
 */
using CharArray = CacheableArrayPrimitive<char16_t, DSCode::CharArray>;

/**
 * An immutable wrapper for array of doubles that can serve as
 * a distributable object for caching.
 */
using CacheableDoubleArray =
    CacheableArrayPrimitive<double, DSCode::CacheableDoubleArray>;

/**
 * An immutable wrapper for array of floats that can serve as
 * a distributable object for caching.
 */
using CacheableFloatArray =
    CacheableArrayPrimitive<float, DSCode::CacheableFloatArray>;

/**
 * An immutable wrapper for array of 16-bit integers that can serve as
 * a distributable object for caching.
 */
using CacheableInt16Array =
    CacheableArrayPrimitive<int16_t, DSCode::CacheableInt16Array>;

/**
 * An immutable wrapper for array of 32-bit integers that can serve as
 * a distributable object for caching.
 */
using CacheableInt32Array =
    CacheableArrayPrimitive<int32_t, DSCode::CacheableInt32Array>;

/**
 * An immutable wrapper for array of 64-bit integers that can serve as
 * a distributable object for caching.
 */
using CacheableInt64Array =
    CacheableArrayPrimitive<int64_t, DSCode::CacheableInt64Array>;

/**
 * An immutable wrapper for array of strings that can serve as
 * a distributable object for caching.
 */
using CacheableStringArray =
    CacheableArrayPrimitive<std::shared_ptr<CacheableString>,
                            DSCode::CacheableStringArray>;

/** Template class for container Cacheable types. */
template <typename TBase, DSCode TYPEID>
class APACHE_GEODE_EXPORT CacheableContainerPrimitive
    : public DataSerializablePrimitive,
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

  size_t objectSize() const override {
    return sizeof(CacheableContainerPrimitive) + serializer::objectSize(*this);
  }

  /** Factory function registered with serialization registry. */
  static std::shared_ptr<Serializable> createDeserializable() {
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

/**
 * A mutable <code>Cacheable</code> vector wrapper that can serve as
 * a distributable object for caching.
 */
using CacheableVector =
    CacheableContainerPrimitive<std::vector<std::shared_ptr<Cacheable>>,
                                DSCode::CacheableVector>;

/**
 * A mutable <code>CacheableKey</code> to <code>Serializable</code>
 * hash map that can serve as a distributable object for caching.
 */
using CacheableHashMap =
    CacheableContainerPrimitive<HashMapOfCacheable, DSCode::CacheableHashMap>;

/**
 * A mutable <code>CacheableKey</code> hash set wrapper that can serve as
 * a distributable object for caching.
 */
using CacheableHashSet = CacheableContainerPrimitive<HashSetOfCacheableKey,
                                                     DSCode::CacheableHashSet>;

/**
 * A mutable <code>Cacheable</code> array list wrapper that can serve as
 * a distributable object for caching.
 */
using CacheableArrayList =
    CacheableContainerPrimitive<std::vector<std::shared_ptr<Cacheable>>,
                                DSCode::CacheableArrayList>;

/**
 * A mutable <code>Cacheable</code> array list wrapper that can serve as
 * a distributable object for caching.
 */
using CacheableLinkedList =
    CacheableContainerPrimitive<std::vector<std::shared_ptr<Cacheable>>,
                                DSCode::CacheableLinkedList>;

/**
 * A mutable <code>Cacheable</code> stack wrapper that can serve as
 * a distributable object for caching.
 */
using CacheableStack =
    CacheableContainerPrimitive<std::vector<std::shared_ptr<Cacheable>>,
                                DSCode::CacheableStack>;

/**
 * A mutable <code>CacheableKey</code> to <code>Serializable</code>
 * hash map that can serve as a distributable object for caching.
 */
using CacheableHashTable =
    CacheableContainerPrimitive<HashMapOfCacheable, DSCode::CacheableHashTable>;

/**
 * A mutable <code>CacheableKey</code> to <code>Serializable</code>
 * hash map that can serve as a distributable object for caching. This is
 * provided for compability with java side, though is functionally identical
 * to <code>CacheableHashMap</code> i.e. does not provide the semantics of
 * java <code>IdentityHashMap</code>.
 */
using CacheableIdentityHashMap =
    CacheableContainerPrimitive<HashMapOfCacheable,
                                DSCode::CacheableIdentityHashMap>;

/**
 * A mutable <code>CacheableKey</code> hash set wrapper that can serve as
 * a distributable object for caching. This is provided for compability
 * with java side, though is functionally identical to
 * <code>CacheableHashSet</code> i.e. does not provide the predictable
 * iteration semantics of java <code>LinkedHashSet</code>.
 */
using CacheableLinkedHashSet =
    CacheableContainerPrimitive<HashSetOfCacheableKey,
                                DSCode::CacheableLinkedHashSet>;

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHEABLEBUILTINS_H_
