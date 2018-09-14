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
#include "internal/CacheableBuiltinTemplates.hpp"
#include "CacheableString.hpp"

namespace apache {
namespace geode {
namespace client {

using internal::CacheableArrayPrimitive;
using internal::CacheableContainerPrimitive;
using internal::CacheableKeyPrimitive;

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

// The following are defined as classes to avoid the issues with MSVC++
// warning/erroring on C4503

/**
 * A mutable <code>Cacheable</code> vector wrapper that can serve as
 * a distributable object for caching.
 */
class APACHE_GEODE_EXPORT CacheableVector
    : public CacheableContainerPrimitive<
          std::vector<std::shared_ptr<Cacheable>>, DSCode::CacheableVector,
          CacheableVector> {
 public:
  using CacheableContainerPrimitive::CacheableContainerPrimitive;
};

template class CacheableContainerPrimitive<
    std::vector<std::shared_ptr<Cacheable>>, DSCode::CacheableVector,
    CacheableVector>;

/**
 * A mutable <code>CacheableKey</code> to <code>Serializable</code>
 * hash map that can serve as a distributable object for caching.
 */
class APACHE_GEODE_EXPORT CacheableHashMap
    : public CacheableContainerPrimitive<
          HashMapOfCacheable, DSCode::CacheableHashMap, CacheableHashMap> {
 public:
  using CacheableContainerPrimitive::CacheableContainerPrimitive;
};

template class CacheableContainerPrimitive<
    HashMapOfCacheable, DSCode::CacheableHashMap, CacheableHashMap>;

/**
 * A mutable <code>CacheableKey</code> hash set wrapper that can serve as
 * a distributable object for caching.
 */
// using CacheableHashSet = CacheableContainerPrimitive<HashSetOfCacheableKey,
//                                                     DSCode::CacheableHashSet>;
class APACHE_GEODE_EXPORT CacheableHashSet
    : public CacheableContainerPrimitive<
          HashSetOfCacheableKey, DSCode::CacheableHashSet, CacheableHashSet> {
 public:
  using CacheableContainerPrimitive::CacheableContainerPrimitive;
};

template class CacheableContainerPrimitive<
    HashSetOfCacheableKey, DSCode::CacheableHashSet, CacheableHashSet>;

/**
 * A mutable <code>Cacheable</code> array list wrapper that can serve as
 * a distributable object for caching.
 */
class APACHE_GEODE_EXPORT CacheableArrayList
    : public CacheableContainerPrimitive<
          std::vector<std::shared_ptr<Cacheable>>, DSCode::CacheableArrayList,
          CacheableArrayList> {
 public:
  using CacheableContainerPrimitive::CacheableContainerPrimitive;
};

template class CacheableContainerPrimitive<
    std::vector<std::shared_ptr<Cacheable>>, DSCode::CacheableArrayList,
    CacheableArrayList>;

/**
 * A mutable <code>Cacheable</code> array list wrapper that can serve as
 * a distributable object for caching.
 */
class APACHE_GEODE_EXPORT CacheableLinkedList
    : public CacheableContainerPrimitive<
          std::vector<std::shared_ptr<Cacheable>>, DSCode::CacheableLinkedList,
          CacheableLinkedList> {
 public:
  using CacheableContainerPrimitive::CacheableContainerPrimitive;
};

template class CacheableContainerPrimitive<
    std::vector<std::shared_ptr<Cacheable>>, DSCode::CacheableLinkedList,
    CacheableLinkedList>;

/**
 * A mutable <code>Cacheable</code> stack wrapper that can serve as
 * a distributable object for caching.
 */
class APACHE_GEODE_EXPORT CacheableStack
    : public CacheableContainerPrimitive<
          std::vector<std::shared_ptr<Cacheable>>, DSCode::CacheableStack,
          CacheableStack> {
 public:
  using CacheableContainerPrimitive::CacheableContainerPrimitive;
};

template class CacheableContainerPrimitive<
    HashMapOfCacheable, DSCode::CacheableStack, CacheableStack>;

/**
 * A mutable <code>CacheableKey</code> to <code>Serializable</code>
 * hash map that can serve as a distributable object for caching.
 */
class APACHE_GEODE_EXPORT CacheableHashTable
    : public CacheableContainerPrimitive<
          HashMapOfCacheable, DSCode::CacheableHashTable, CacheableHashTable> {
 public:
  using CacheableContainerPrimitive::CacheableContainerPrimitive;
};

template class CacheableContainerPrimitive<
    HashMapOfCacheable, DSCode::CacheableHashTable, CacheableHashTable>;

/**
 * A mutable <code>CacheableKey</code> to <code>Serializable</code>
 * hash map that can serve as a distributable object for caching. This is
 * provided for compability with java side, though is functionally identical
 * to <code>CacheableHashMap</code> i.e. does not provide the semantics of
 * java <code>IdentityHashMap</code>.
 */
class APACHE_GEODE_EXPORT CacheableIdentityHashMap
    : public CacheableContainerPrimitive<HashMapOfCacheable,
                                         DSCode::CacheableIdentityHashMap,
                                         CacheableIdentityHashMap> {
 public:
  using CacheableContainerPrimitive::CacheableContainerPrimitive;
};

template class CacheableContainerPrimitive<HashMapOfCacheable,
                                           DSCode::CacheableIdentityHashMap,
                                           CacheableIdentityHashMap>;

/**
 * A mutable <code>CacheableKey</code> hash set wrapper that can serve as
 * a distributable object for caching. This is provided for compability
 * with java side, though is functionally identical to
 * <code>CacheableHashSet</code> i.e. does not provide the predictable
 * iteration semantics of java <code>LinkedHashSet</code>.
 */
class APACHE_GEODE_EXPORT CacheableLinkedHashSet
    : public CacheableContainerPrimitive<HashSetOfCacheableKey,
                                         DSCode::CacheableLinkedHashSet,
                                         CacheableLinkedHashSet> {
 public:
  using CacheableContainerPrimitive::CacheableContainerPrimitive;
};

template class CacheableContainerPrimitive<HashSetOfCacheableKey,
                                           DSCode::CacheableLinkedHashSet,
                                           CacheableLinkedHashSet>;

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHEABLEBUILTINS_H_
