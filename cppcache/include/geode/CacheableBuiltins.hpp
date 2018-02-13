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
#include "CacheableKeys.hpp"
#include "CacheableString.hpp"

namespace apache {
namespace geode {
namespace client {

/** Template CacheableKey class for primitive types. */
template <typename TObj, int8_t TYPEID, const char* TYPENAME,
          const char* SPRINTFSYM, int32_t STRSIZE>
class CacheableKeyType : public CacheableKey {
 protected:
  TObj m_value;

  inline CacheableKeyType()
      : m_value(apache::geode::client::serializer::zeroObject<TObj>()) {}

  inline CacheableKeyType(const TObj value) : m_value(value) {}

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
    char buffer[STRSIZE + 1];
    std::sprintf(buffer, SPRINTFSYM, m_value);
    return std::string(buffer);
  }

  // CacheableKey methods

  /** Return the hashcode for this key. */
  virtual int32_t hashcode() const override {
    return serializer::hashcode(m_value);
  }

  /** Return true if this key matches other. */
  virtual bool operator==(const CacheableKey& other) const override {
    if (other.typeId() != TYPEID) {
      return false;
    }
    const CacheableKeyType& otherValue =
        static_cast<const CacheableKeyType&>(other);
    return serializer::equals(m_value, otherValue.m_value);
  }

  /** Return true if this key matches other key value. */
  inline bool operator==(const TObj other) const {
    return serializer::equals(m_value, other);
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
class CacheableContainerType : public Cacheable, public TBase {
 protected:
  inline CacheableContainerType() : TBase() {}

  inline CacheableContainerType(const int32_t n) : TBase(n) {}

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
};

// Disable extern template warning on MSVC compiler
#ifdef _MSC_VER
#pragma warning(disable : 4231)
#endif

#define _GEODE_CACHEABLE_KEY_TYPE_DEF_(p, k, sz)                     \
  extern const char tName_##k[];                                     \
  extern const char tStr_##k[];                                      \
  template class _GEODE_EXPORT                                       \
      CacheableKeyType<p, GeodeTypeIds::k, tName_##k, tStr_##k, sz>; \
  typedef CacheableKeyType<p, GeodeTypeIds::k, tName_##k, tStr_##k, sz> _##k;

// use a class instead of typedef for bug #283
#define _GEODE_CACHEABLE_KEY_TYPE_(p, k, sz)                            \
  class _GEODE_EXPORT k : public _##k {                                 \
   protected:                                                           \
    inline k() : _##k() {}                                              \
    inline k(const p value) : _##k(value) {}                            \
    _GEODE_FRIEND_STD_SHARED_PTR(k)                                     \
                                                                        \
   public:                                                              \
    /** Factory function registered with serialization registry. */     \
    static std::shared_ptr<Serializable> createDeserializable() {       \
      return std::make_shared<k>();                                     \
    }                                                                   \
    /** Factory function to create a new default instance. */           \
    inline static std::shared_ptr<k> create() {                         \
      return std::make_shared<k>();                                     \
    }                                                                   \
    /** Factory function to create an instance with the given value. */ \
    inline static std::shared_ptr<k> create(const p value) {            \
      return std::make_shared<k>(value);                                \
    }                                                                   \
  };                                                                    \
  template <>                                                           \
  inline std::shared_ptr<CacheableKey> CacheableKey::create(p value) {  \
    return k::create(value);                                            \
  }                                                                     \
  template <>                                                           \
  inline std::shared_ptr<Cacheable> Serializable::create(p value) {     \
    return k::create(value);                                            \
  }

#define _GEODE_CACHEABLE_CONTAINER_TYPE_DEF_(p, c)                         \
  template class _GEODE_EXPORT CacheableContainerType<p, GeodeTypeIds::c>; \
  typedef CacheableContainerType<p, GeodeTypeIds::c> _##c;

// use a class instead of typedef for bug #283
#define _GEODE_CACHEABLE_CONTAINER_TYPE_(p, c)                         \
  class _GEODE_EXPORT c : public _##c {                                \
   protected:                                                          \
    inline c() : _##c() {}                                             \
    inline c(const int32_t n) : _##c(n) {}                             \
                                                                       \
    _GEODE_FRIEND_STD_SHARED_PTR(c)                                    \
                                                                       \
   public:                                                             \
    /** Factory function registered with serialization registry. */    \
    static std::shared_ptr<Serializable> createDeserializable() {      \
      return std::make_shared<c>();                                    \
    }                                                                  \
    /** Factory function to create a default instance. */              \
    inline static std::shared_ptr<c> create() {                        \
      return std::make_shared<c>();                                    \
    }                                                                  \
    /** Factory function to create an instance with the given size. */ \
    inline static std::shared_ptr<c> create(const int32_t n) {         \
      return std::make_shared<c>(n);                                   \
    }                                                                  \
  };

// Instantiations for the built-in CacheableKeys

_GEODE_CACHEABLE_KEY_TYPE_DEF_(bool, CacheableBoolean, 3);
/**
 * An immutable wrapper for booleans that can serve as
 * a distributable key object for caching.
 */
_GEODE_CACHEABLE_KEY_TYPE_(bool, CacheableBoolean, 3);

_GEODE_CACHEABLE_KEY_TYPE_DEF_(int8_t, CacheableByte, 15);
/**
 * An immutable wrapper for bytes that can serve as
 * a distributable key object for caching.
 */
_GEODE_CACHEABLE_KEY_TYPE_(int8_t, CacheableByte, 15);

_GEODE_CACHEABLE_KEY_TYPE_DEF_(double, CacheableDouble, 63);
/**
 * An immutable wrapper for doubles that can serve as
 * a distributable key object for caching.
 */
_GEODE_CACHEABLE_KEY_TYPE_(double, CacheableDouble, 63);

_GEODE_CACHEABLE_KEY_TYPE_DEF_(float, CacheableFloat, 63);
/**
 * An immutable wrapper for floats that can serve as
 * a distributable key object for caching.
 */
_GEODE_CACHEABLE_KEY_TYPE_(float, CacheableFloat, 63);

_GEODE_CACHEABLE_KEY_TYPE_DEF_(int16_t, CacheableInt16, 15);
/**
 * An immutable wrapper for 16-bit integers that can serve as
 * a distributable key object for caching.
 */
_GEODE_CACHEABLE_KEY_TYPE_(int16_t, CacheableInt16, 15);

_GEODE_CACHEABLE_KEY_TYPE_DEF_(int32_t, CacheableInt32, 15);
/**
 * An immutable wrapper for 32-bit integers that can serve as
 * a distributable key object for caching.
 */
_GEODE_CACHEABLE_KEY_TYPE_(int32_t, CacheableInt32, 15);

_GEODE_CACHEABLE_KEY_TYPE_DEF_(int64_t, CacheableInt64, 31);
/**
 * An immutable wrapper for 64-bit integers that can serve as
 * a distributable key object for caching.
 */
_GEODE_CACHEABLE_KEY_TYPE_(int64_t, CacheableInt64, 31);

_GEODE_CACHEABLE_KEY_TYPE_DEF_(char16_t, CacheableCharacter, 3);
/**
 * An immutable wrapper for characters that can serve as
 * a distributable key object for caching.
 */
_GEODE_CACHEABLE_KEY_TYPE_(char16_t, CacheableCharacter, 3);

// Instantiations for array built-in Cacheables

template <typename T, GeodeTypeIds::IdValues GeodeTypeId>
class _GEODE_EXPORT CacheableArray : public Cacheable {
protected:

  inline CacheableArray() = default;

  CacheableArray(const CacheableArray& other) = delete;

  CacheableArray& operator=(const CacheableArray& other) = delete;

  template <typename TT>
  CacheableArray(TT&& value) : m_value(std::forward<TT>(value)) {}

  virtual int32_t classId() const override { return 0; }

  virtual int8_t typeId() const override {
    return GeodeTypeId;
  }

  virtual size_t objectSize() const override {
    return static_cast<uint32_t>(
        apache::geode::client::serializer::objectArraySize(m_value));
  }

private:

  _GEODE_FRIEND_STD_SHARED_PTR(CacheableArray)

  std::vector<T> m_value;

public:

  inline const std::vector<T>& value() const {
    return m_value;
  }

  inline int32_t length() const {
    return m_value.size();
  }

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

  inline T operator[](uint32_t index) const {
    if (static_cast<int32_t>(index) >= m_value.size()) {
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
using CacheableDoubleArray = CacheableArray<double, GeodeTypeIds::CacheableDoubleArray>;

/**
 * An immutable wrapper for array of floats that can serve as
 * a distributable object for caching.
 */
using CacheableFloatArray = CacheableArray<float, GeodeTypeIds::CacheableFloatArray>;

/**
 * An immutable wrapper for array of 16-bit integers that can serve as
 * a distributable object for caching.
 */
using CacheableInt16Array = CacheableArray<int16_t, GeodeTypeIds::CacheableInt16Array>;

/**
 * An immutable wrapper for array of 32-bit integers that can serve as
 * a distributable object for caching.
 */
using CacheableInt32Array = CacheableArray<int32_t, GeodeTypeIds::CacheableInt32Array>;

/**
 * An immutable wrapper for array of 64-bit integers that can serve as
 * a distributable object for caching.
 */
using CacheableInt64Array = CacheableArray<int64_t, GeodeTypeIds::CacheableInt64Array>;

/**
 * An immutable wrapper for array of strings that can serve as
 * a distributable object for caching.
 */
using CacheableStringArray = CacheableArray<std::shared_ptr<CacheableString>, GeodeTypeIds::CacheableStringArray>;

// Instantiations for container types (Vector/HashMap/HashSet) Cacheables

_GEODE_CACHEABLE_CONTAINER_TYPE_DEF_(std::vector<std::shared_ptr<Cacheable>>,
                                     CacheableVector);
/**
 * A mutable <code>Cacheable</code> vector wrapper that can serve as
 * a distributable object for caching.
 */
_GEODE_CACHEABLE_CONTAINER_TYPE_(std::vector<std::shared_ptr<Cacheable>>,
                                 CacheableVector);

_GEODE_CACHEABLE_CONTAINER_TYPE_DEF_(HashMapOfCacheable, CacheableHashMap);
/**
 * A mutable <code>CacheableKey</code> to <code>Serializable</code>
 * hash map that can serve as a distributable object for caching.
 */
_GEODE_CACHEABLE_CONTAINER_TYPE_(HashMapOfCacheable, CacheableHashMap);

_GEODE_CACHEABLE_CONTAINER_TYPE_DEF_(HashSetOfCacheableKey, CacheableHashSet);
/**
 * A mutable <code>CacheableKey</code> hash set wrapper that can serve as
 * a distributable object for caching.
 */
_GEODE_CACHEABLE_CONTAINER_TYPE_(HashSetOfCacheableKey, CacheableHashSet);

_GEODE_CACHEABLE_CONTAINER_TYPE_DEF_(std::vector<std::shared_ptr<Cacheable>>,
                                     CacheableArrayList);
/**
 * A mutable <code>Cacheable</code> array list wrapper that can serve as
 * a distributable object for caching.
 */
_GEODE_CACHEABLE_CONTAINER_TYPE_(std::vector<std::shared_ptr<Cacheable>>,
                                 CacheableArrayList);

// linketlist for JSON formattor issue
_GEODE_CACHEABLE_CONTAINER_TYPE_DEF_(std::vector<std::shared_ptr<Cacheable>>,
                                     CacheableLinkedList);
/**
 * A mutable <code>Cacheable</code> array list wrapper that can serve as
 * a distributable object for caching.
 */
_GEODE_CACHEABLE_CONTAINER_TYPE_(std::vector<std::shared_ptr<Cacheable>>,
                                 CacheableLinkedList);

_GEODE_CACHEABLE_CONTAINER_TYPE_DEF_(std::vector<std::shared_ptr<Cacheable>>,
                                     CacheableStack);
/**
 * A mutable <code>Cacheable</code> stack wrapper that can serve as
 * a distributable object for caching.
 */
_GEODE_CACHEABLE_CONTAINER_TYPE_(std::vector<std::shared_ptr<Cacheable>>,
                                 CacheableStack);

_GEODE_CACHEABLE_CONTAINER_TYPE_DEF_(HashMapOfCacheable, CacheableHashTable);
/**
 * A mutable <code>CacheableKey</code> to <code>Serializable</code>
 * hash map that can serve as a distributable object for caching.
 */
_GEODE_CACHEABLE_CONTAINER_TYPE_(HashMapOfCacheable, CacheableHashTable);

_GEODE_CACHEABLE_CONTAINER_TYPE_DEF_(HashMapOfCacheable,
                                     CacheableIdentityHashMap);
/**
 * A mutable <code>CacheableKey</code> to <code>Serializable</code>
 * hash map that can serve as a distributable object for caching. This is
 * provided for compability with java side, though is functionally identical
 * to <code>CacheableHashMap</code> i.e. does not provide the semantics of
 * java <code>IdentityHashMap</code>.
 */
_GEODE_CACHEABLE_CONTAINER_TYPE_(HashMapOfCacheable, CacheableIdentityHashMap);

_GEODE_CACHEABLE_CONTAINER_TYPE_DEF_(HashSetOfCacheableKey,
                                     CacheableLinkedHashSet);
/**
 * A mutable <code>CacheableKey</code> hash set wrapper that can serve as
 * a distributable object for caching. This is provided for compability
 * with java side, though is functionally identical to
 * <code>CacheableHashSet</code> i.e. does not provide the predictable
 * iteration semantics of java <code>LinkedHashSet</code>.
 */
_GEODE_CACHEABLE_CONTAINER_TYPE_(HashSetOfCacheableKey, CacheableLinkedHashSet);

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHEABLEBUILTINS_H_
