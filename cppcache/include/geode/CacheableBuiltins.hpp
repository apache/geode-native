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

/** Template CacheableKey class for primitive types. */
template <typename TObj, int8_t TYPEID>
class APACHE_GEODE_EXPORT CacheableKeyType : public CacheableKey {
 protected:
  TObj m_value;

  inline CacheableKeyType()
      : m_value(apache::geode::client::serializer::zeroObject<TObj>()) {}

  inline CacheableKeyType(const TObj value) : m_value(value) {}

  using KEYTYPE = CacheableKeyType<TObj, TYPEID>;
  _GEODE_FRIEND_STD_SHARED_PTR(KEYTYPE)

 public:
  /** Gets the contained value. */
  inline TObj value() const { return m_value; }

  // Cacheable methods

  /** Serialize this object to given <code>DataOutput</code>. */
  virtual void toData(DataOutput& output) const override {
    apache::geode::client::serializer::writeObject(output, m_value);
  }

  /** Deserialize this object from given <code>DataInput</code>. */
  virtual void fromData(DataInput& input) override {
    apache::geode::client::serializer::readObject(input, m_value);
  }

  /**
   * Return the classId of the instance being serialized.
   *
   * This is used by deserialization to determine what instance
   * type to create and deserialize into.
   */
  virtual int32_t classId() const override { return 0; }

  /**
   * Return the typeId byte of the instance being serialized.
   *
   * This is used by deserialization to determine what instance
   * type to create and deserialize into.
   */
  virtual int8_t typeId() const override { return TYPEID; }

  /** Return a string representation of the object. */
  virtual std::string toString() const override {
    return std::to_string(m_value);
  }

  // CacheableKey methods

  /** Return the hashcode for this key. */
  virtual int32_t hashcode() const override {
    return internal::hashcode(m_value);
  }

  /** Return true if this key matches other. */
  virtual bool operator==(const CacheableKey& other) const override {
    if (other.typeId() != TYPEID) {
      return false;
    }
    const CacheableKeyType& otherValue =
        static_cast<const CacheableKeyType&>(other);
    return internal::equals(m_value, otherValue.m_value);
  }

  /** Return true if this key matches other key value. */
  inline bool operator==(const TObj other) const {
    return internal::equals(m_value, other);
  }

  /**
   * Return the size in bytes of the instance being serialized.
   *
   * This is used to determine whether the cache is using up more
   * physical memory than it has been configured to use. The method can
   * return zero if the user does not require the ability to control
   * cache memory utilization.
   */
  virtual size_t objectSize() const override {
    return sizeof(CacheableKeyType);
  }

  /** Factory function registered with serialization registry. */
  static std::shared_ptr<Serializable> createDeserializable() {
    return std::make_shared<CacheableKeyType<TObj, TYPEID>>();
  }
  /** Factory function to create a new default instance. */
  inline static std::shared_ptr<CacheableKeyType> create() {
    return std::make_shared<CacheableKeyType>();
  }
  /** Factory function to create an instance with the given value. */
  inline static std::shared_ptr<CacheableKeyType> create(const TObj value) {
    return std::make_shared<CacheableKeyType>(value);
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

/** Template class for container Cacheable types. */
template <typename TBase, int8_t TYPEID>
class APACHE_GEODE_EXPORT CacheableContainerType : public Cacheable, public TBase {
 protected:
  inline CacheableContainerType() : TBase() {}

  inline CacheableContainerType(const int32_t n) : TBase(n) {}

  using CONTAINTERTYPE = CacheableContainerType<TBase, TYPEID>;
  _GEODE_FRIEND_STD_SHARED_PTR(CONTAINTERTYPE)

 public:
  // Cacheable methods

  /** Serialize this object to the given <code>DataOutput</code>. */
  virtual void toData(DataOutput& output) const override {
    apache::geode::client::serializer::writeObject(output, *this);
  }

  /** Deserialize this object from the given <code>DataInput</code>. */
  virtual void fromData(DataInput& input) override {
    apache::geode::client::serializer::readObject(input, *this);
  }

  /**
   * Return the classId of the instance being serialized.
   *
   * This is used by deserialization to determine what instance
   * type to create and deserialize into.
   */
  virtual int32_t classId() const override { return 0; }

  /**
   * Return the typeId byte of the instance being serialized.
   *
   * This is used by deserialization to determine what instance
   * type to create and deserialize into.
   */
  virtual int8_t typeId() const override { return TYPEID; }

  /**
   * Return the size in bytes of the instance being serialized.
   *
   * This is used to determine whether the cache is using up more
   * physical memory than it has been configured to use. The method can
   * return zero if the user does not require the ability to control
   * cache memory utilization.
   */
  virtual size_t objectSize() const override {
    return sizeof(CacheableContainerType) + serializer::objectSize(*this);
  }
  /** Factory function registered with serialization registry. */
  static std::shared_ptr<Serializable> createDeserializable() {
    return std::make_shared<CONTAINTERTYPE>();
  }
  /** Factory function to create a default instance. */
  inline static std::shared_ptr<CONTAINTERTYPE> create() {
    return std::make_shared<CONTAINTERTYPE>();
  }
  /** Factory function to create an instance with the given size. */
  inline static std::shared_ptr<CONTAINTERTYPE> create(const int32_t n) {
    return std::make_shared<CONTAINTERTYPE>(n);
  }
};

// Disable extern template warning on MSVC compiler
#ifdef _MSC_VER
#pragma warning(disable : 4231)
#endif

// Instantiations for the built-in CacheableKeys
/**
 * An immutable wrapper for booleans that can serve as
 * a distributable key object for caching.
 */
using CacheableBoolean = CacheableKeyType<bool, GeodeTypeIds::CacheableBoolean>;
template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(bool value) {
  return CacheableBoolean::create(value);
}
template <>
inline std::shared_ptr<Cacheable> Serializable::create(bool value) {
  return CacheableBoolean::create(value);
}

/**
 * An immutable wrapper for bytes that can serve as
 * a distributable key object for caching.
 */
using CacheableByte = CacheableKeyType<int8_t, GeodeTypeIds::CacheableByte>;
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
using CacheableDouble = CacheableKeyType<double, GeodeTypeIds::CacheableDouble>;
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
using CacheableFloat = CacheableKeyType<float, GeodeTypeIds::CacheableFloat>;
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
using CacheableInt16 = CacheableKeyType<int16_t, GeodeTypeIds::CacheableInt16>;
template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(int16_t value) {
  return CacheableInt16::create(value);
}
template <>
inline std::shared_ptr<Cacheable> Serializable::create(int16_t value) {
  return CacheableInt16::create(value);
}

/**
 * An immutable wrapper for 32-bit integers that can serve as
 * a distributable key object for caching.
 */
using CacheableInt32 = CacheableKeyType<int32_t, GeodeTypeIds::CacheableInt32>;
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
using CacheableInt64 = CacheableKeyType<int64_t, GeodeTypeIds::CacheableInt64>;
template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(int64_t value) {
  return CacheableInt64::create(value);
}
template <>
inline std::shared_ptr<Cacheable> Serializable::create(int64_t value) {
  return CacheableInt64::create(value);
}

/**
 * An immutable wrapper for characters that can serve as
 * a distributable key object for caching.
 */
using CacheableCharacter = CacheableKeyType<char16_t, GeodeTypeIds::CacheableCharacter>;
template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(char16_t value) {
  return CacheableCharacter::create(value);
}
template <>
inline std::shared_ptr<Cacheable> Serializable::create(char16_t value) {
  return CacheableCharacter::create(value);
}

// Instantiations for array built-in Cacheables

template <typename T, GeodeTypeIds::IdValues GeodeTypeId>
class APACHE_GEODE_EXPORT CacheableArray : public Cacheable {
 protected:
  inline CacheableArray() = default;

  CacheableArray(const CacheableArray& other) = delete;

  CacheableArray& operator=(const CacheableArray& other) = delete;

  template <typename TT>
  CacheableArray(TT&& value) : m_value(std::forward<TT>(value)) {}

  virtual int32_t classId() const override { return 0; }

  virtual int8_t typeId() const override { return GeodeTypeId; }

  virtual size_t objectSize() const override {
    return static_cast<uint32_t>(
        apache::geode::client::serializer::objectArraySize(m_value));
  }

 private:
  _GEODE_FRIEND_STD_SHARED_PTR(CacheableArray)

  std::vector<T> m_value;

 public:
  inline const std::vector<T>& value() const { return m_value; }

  inline int32_t length() const { return static_cast<int32_t>(m_value.size()); }

  static std::shared_ptr<Serializable> createDeserializable() {
    return std::make_shared<CacheableArray<T, GeodeTypeId>>();
  }

  inline static std::shared_ptr<CacheableArray<T, GeodeTypeId>> create() {
    return std::make_shared<CacheableArray<T, GeodeTypeId>>();
  }

  inline static std::shared_ptr<CacheableArray<T, GeodeTypeId>> create(
      const std::vector<T>& value) {
    return std::make_shared<CacheableArray<T, GeodeTypeId>>(value);
  }

  inline static std::shared_ptr<CacheableArray<T, GeodeTypeId>> create(
      std::vector<T>&& value) {
    return std::make_shared<CacheableArray<T, GeodeTypeId>>(std::move(value));
  }

  inline T operator[](int32_t index) const {
    if (index >= static_cast<int32_t>(m_value.size())) {
      throw OutOfRangeException(
          "CacheableArray::operator[]: Index out of range.");
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
using CacheableBytes = CacheableArray<int8_t, GeodeTypeIds::CacheableBytes>;

/**
 * An immutable wrapper for array of booleans that can serve as
 * a distributable object for caching.
 */
using BooleanArray = CacheableArray<bool, GeodeTypeIds::BooleanArray>;

/**
 * An immutable wrapper for array of wide-characters that can serve as
 * a distributable object for caching.
 */
using CharArray = CacheableArray<char16_t, GeodeTypeIds::CharArray>;

/**
 * An immutable wrapper for array of doubles that can serve as
 * a distributable object for caching.
 */
using CacheableDoubleArray =
    CacheableArray<double, GeodeTypeIds::CacheableDoubleArray>;

/**
 * An immutable wrapper for array of floats that can serve as
 * a distributable object for caching.
 */
using CacheableFloatArray =
    CacheableArray<float, GeodeTypeIds::CacheableFloatArray>;

/**
 * An immutable wrapper for array of 16-bit integers that can serve as
 * a distributable object for caching.
 */
using CacheableInt16Array =
    CacheableArray<int16_t, GeodeTypeIds::CacheableInt16Array>;

/**
 * An immutable wrapper for array of 32-bit integers that can serve as
 * a distributable object for caching.
 */
using CacheableInt32Array =
    CacheableArray<int32_t, GeodeTypeIds::CacheableInt32Array>;

/**
 * An immutable wrapper for array of 64-bit integers that can serve as
 * a distributable object for caching.
 */
using CacheableInt64Array =
    CacheableArray<int64_t, GeodeTypeIds::CacheableInt64Array>;

/**
 * An immutable wrapper for array of strings that can serve as
 * a distributable object for caching.
 */
using CacheableStringArray = CacheableArray<std::shared_ptr<CacheableString>,
                                            GeodeTypeIds::CacheableStringArray>;

// Instantiations for container types (Vector/HashMap/HashSet) Cacheables

/**
 * A mutable <code>Cacheable</code> vector wrapper that can serve as
 * a distributable object for caching.
 */
using CacheableVector =
    CacheableContainerType<std::vector<std::shared_ptr<Cacheable>>,
                           GeodeTypeIds::CacheableVector>;

/**
 * A mutable <code>CacheableKey</code> to <code>Serializable</code>
 * hash map that can serve as a distributable object for caching.
 */
using CacheableHashMap =
    CacheableContainerType<HashMapOfCacheable,
                           GeodeTypeIds::CacheableHashMap>;

/**
 * A mutable <code>CacheableKey</code> hash set wrapper that can serve as
 * a distributable object for caching.
 */
using CacheableHashSet =
    CacheableContainerType<HashSetOfCacheableKey,
                           GeodeTypeIds::CacheableHashSet>;

/**
 * A mutable <code>Cacheable</code> array list wrapper that can serve as
 * a distributable object for caching.
 */
using CacheableArrayList =
    CacheableContainerType<std::vector<std::shared_ptr<Cacheable>>,
                           GeodeTypeIds::CacheableArrayList>;

// linketlist for JSON formattor issue
/**
 * A mutable <code>Cacheable</code> array list wrapper that can serve as
 * a distributable object for caching.
 */
using CacheableLinkedList =
    CacheableContainerType<std::vector<std::shared_ptr<Cacheable>>,
                           GeodeTypeIds::CacheableLinkedList>;

/**
 * A mutable <code>Cacheable</code> stack wrapper that can serve as
 * a distributable object for caching.
 */
using CacheableStack =
    CacheableContainerType<std::vector<std::shared_ptr<Cacheable>>,
                           GeodeTypeIds::CacheableStack>;

/**
 * A mutable <code>CacheableKey</code> to <code>Serializable</code>
 * hash map that can serve as a distributable object for caching.
 */
using CacheableHashTable =
    CacheableContainerType<HashMapOfCacheable,
                           GeodeTypeIds::CacheableHashTable>;

/**
 * A mutable <code>CacheableKey</code> to <code>Serializable</code>
 * hash map that can serve as a distributable object for caching. This is
 * provided for compability with java side, though is functionally identical
 * to <code>CacheableHashMap</code> i.e. does not provide the semantics of
 * java <code>IdentityHashMap</code>.
 */
using CacheableIdentityHashMap =
    CacheableContainerType<HashMapOfCacheable,
                           GeodeTypeIds::CacheableIdentityHashMap>;

/**
 * A mutable <code>CacheableKey</code> hash set wrapper that can serve as
 * a distributable object for caching. This is provided for compability
 * with java side, though is functionally identical to
 * <code>CacheableHashSet</code> i.e. does not provide the predictable
 * iteration semantics of java <code>LinkedHashSet</code>.
 */
using CacheableLinkedHashSet =
    CacheableContainerType<HashSetOfCacheableKey,
                           GeodeTypeIds::CacheableLinkedHashSet>;

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHEABLEBUILTINS_H_
