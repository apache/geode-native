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

#include "CacheableKey.hpp"
#include "CacheableString.hpp"
#include "Serializable.hpp"
#include "Serializer.hpp"
#include "internal/CacheableBuiltinTemplates.hpp"
#include "internal/CacheableKeys.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * An immutable wrapper for bool that can serve as
 * a distributable key object for caching.
 */
using CacheableBoolean =
    internal::CacheableKeyPrimitive<bool, internal::DSCode::CacheableBoolean>;
extern template class APACHE_GEODE_EXTERN_TEMPLATE_EXPORT
    internal::CacheableKeyPrimitive<bool, internal::DSCode::CacheableBoolean>;
template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(bool value) {
  return CacheableBoolean::create(value);
}
template <>
inline std::shared_ptr<Cacheable> Serializable::create(bool value) {
  return CacheableBoolean::create(value);
}

template <>
inline std::shared_ptr<Cacheable> Serializable::create(std::nullptr_t) {
  return static_cast<std::shared_ptr<Cacheable>>(nullptr);
}

/**
 * An immutable wrapper for byte that can serve as
 * a distributable key object for caching.
 */
using CacheableByte =
    internal::CacheableKeyPrimitive<int8_t, internal::DSCode::CacheableByte>;
extern template class APACHE_GEODE_EXTERN_TEMPLATE_EXPORT
    internal::CacheableKeyPrimitive<int8_t, internal::DSCode::CacheableByte>;
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
    internal::CacheableKeyPrimitive<double, internal::DSCode::CacheableDouble>;
extern template class APACHE_GEODE_EXTERN_TEMPLATE_EXPORT
    internal::CacheableKeyPrimitive<double, internal::DSCode::CacheableDouble>;
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
    internal::CacheableKeyPrimitive<float, internal::DSCode::CacheableFloat>;
extern template class APACHE_GEODE_EXTERN_TEMPLATE_EXPORT
    internal::CacheableKeyPrimitive<float, internal::DSCode::CacheableFloat>;
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
    internal::CacheableKeyPrimitive<int16_t, internal::DSCode::CacheableInt16>;
extern template class APACHE_GEODE_EXTERN_TEMPLATE_EXPORT
    internal::CacheableKeyPrimitive<int16_t, internal::DSCode::CacheableInt16>;
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
using CacheableInt32 =
    internal::CacheableKeyPrimitive<int32_t, internal::DSCode::CacheableInt32>;
extern template class APACHE_GEODE_EXTERN_TEMPLATE_EXPORT
    internal::CacheableKeyPrimitive<int32_t, internal::DSCode::CacheableInt32>;
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
    internal::CacheableKeyPrimitive<int64_t, internal::DSCode::CacheableInt64>;
extern template class APACHE_GEODE_EXTERN_TEMPLATE_EXPORT
    internal::CacheableKeyPrimitive<int64_t, internal::DSCode::CacheableInt64>;
template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(int64_t value) {
  return CacheableInt64::create(value);
}
template <>
inline std::shared_ptr<Cacheable> Serializable::create(int64_t value) {
  return CacheableInt64::create(value);
}

/**
 * An immutable wrapper for 16-bit characters that can serve as
 * a distributable key object for caching.
 */
using CacheableCharacter =
    internal::CacheableKeyPrimitive<char16_t,
                                    internal::DSCode::CacheableCharacter>;
extern template class APACHE_GEODE_EXTERN_TEMPLATE_EXPORT
    internal::CacheableKeyPrimitive<char16_t,
                                    internal::DSCode::CacheableCharacter>;
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
using CacheableBytes =
    internal::CacheableArrayPrimitive<int8_t, internal::DSCode::CacheableBytes>;
extern template class APACHE_GEODE_EXTERN_TEMPLATE_EXPORT
    internal::CacheableArrayPrimitive<int8_t, internal::DSCode::CacheableBytes>;

/**
 * An immutable wrapper for array of booleans that can serve as
 * a distributable object for caching.
 */
using BooleanArray =
    internal::CacheableArrayPrimitive<bool, internal::DSCode::BooleanArray>;
extern template class APACHE_GEODE_EXTERN_TEMPLATE_EXPORT
    internal::CacheableArrayPrimitive<bool, internal::DSCode::BooleanArray>;

/**
 * An immutable wrapper for array of wide-characters that can serve as
 * a distributable object for caching.
 */
using CharArray =
    internal::CacheableArrayPrimitive<char16_t, internal::DSCode::CharArray>;
extern template class APACHE_GEODE_EXTERN_TEMPLATE_EXPORT
    internal::CacheableArrayPrimitive<char16_t, internal::DSCode::CharArray>;

/**
 * An immutable wrapper for array of doubles that can serve as
 * a distributable object for caching.
 */
using CacheableDoubleArray =
    internal::CacheableArrayPrimitive<double,
                                      internal::DSCode::CacheableDoubleArray>;
extern template class APACHE_GEODE_EXTERN_TEMPLATE_EXPORT
    internal::CacheableArrayPrimitive<double,
                                      internal::DSCode::CacheableDoubleArray>;

/**
 * An immutable wrapper for array of floats that can serve as
 * a distributable object for caching.
 */
using CacheableFloatArray =
    internal::CacheableArrayPrimitive<float,
                                      internal::DSCode::CacheableFloatArray>;
extern template class APACHE_GEODE_EXTERN_TEMPLATE_EXPORT
    internal::CacheableArrayPrimitive<float,
                                      internal::DSCode::CacheableFloatArray>;

/**
 * An immutable wrapper for array of 16-bit integers that can serve as
 * a distributable object for caching.
 */
using CacheableInt16Array =
    internal::CacheableArrayPrimitive<int16_t,
                                      internal::DSCode::CacheableInt16Array>;
extern template class APACHE_GEODE_EXTERN_TEMPLATE_EXPORT
    internal::CacheableArrayPrimitive<int16_t,
                                      internal::DSCode::CacheableInt16Array>;

/**
 * An immutable wrapper for array of 32-bit integers that can serve as
 * a distributable object for caching.
 */
using CacheableInt32Array =
    internal::CacheableArrayPrimitive<int32_t,
                                      internal::DSCode::CacheableInt32Array>;
extern template class APACHE_GEODE_EXTERN_TEMPLATE_EXPORT
    internal::CacheableArrayPrimitive<int32_t,
                                      internal::DSCode::CacheableInt32Array>;

/**
 * An immutable wrapper for array of 64-bit integers that can serve as
 * a distributable object for caching.
 */
using CacheableInt64Array =
    internal::CacheableArrayPrimitive<int64_t,
                                      internal::DSCode::CacheableInt64Array>;
extern template class APACHE_GEODE_EXTERN_TEMPLATE_EXPORT
    internal::CacheableArrayPrimitive<int64_t,
                                      internal::DSCode::CacheableInt64Array>;

/**
 * An immutable wrapper for array of strings that can serve as
 * a distributable object for caching.
 */
using CacheableStringArray =
    internal::CacheableArrayPrimitive<std::shared_ptr<CacheableString>,
                                      internal::DSCode::CacheableStringArray>;
extern template class APACHE_GEODE_EXTERN_TEMPLATE_EXPORT
    internal::CacheableArrayPrimitive<std::shared_ptr<CacheableString>,
                                      internal::DSCode::CacheableStringArray>;

// The following are defined as classes to avoid the issues with MSVC++
// warning/erroring on C4503

/**
 * A mutable <code>Cacheable</code> vector wrapper that can serve as
 * a distributable object for caching.
 */
class APACHE_GEODE_EXPORT CacheableVector
    : public internal::CacheableContainerPrimitive<
          std::vector<std::shared_ptr<Cacheable>>,
          internal::DSCode::CacheableVector, CacheableVector> {
 public:
  using CacheableContainerPrimitive::CacheableContainerPrimitive;
};

template class internal::CacheableContainerPrimitive<
    std::vector<std::shared_ptr<Cacheable>>, internal::DSCode::CacheableVector,
    CacheableVector>;

/**
 * A mutable <code>CacheableKey</code> to <code>Serializable</code>
 * hash map that can serve as a distributable object for caching.
 */
class APACHE_GEODE_EXPORT CacheableHashMap
    : public internal::CacheableContainerPrimitive<
          HashMapOfCacheable, internal::DSCode::CacheableHashMap,
          CacheableHashMap> {
 public:
  using CacheableContainerPrimitive::CacheableContainerPrimitive;
};

template class internal::CacheableContainerPrimitive<
    HashMapOfCacheable, internal::DSCode::CacheableHashMap, CacheableHashMap>;

/**
 * A mutable <code>CacheableKey</code> hash set wrapper that can serve as
 * a distributable object for caching.
 */
class APACHE_GEODE_EXPORT CacheableHashSet
    : public internal::CacheableContainerPrimitive<
          HashSetOfCacheableKey, internal::DSCode::CacheableHashSet,
          CacheableHashSet> {
 public:
  using CacheableContainerPrimitive::CacheableContainerPrimitive;
};

template class internal::CacheableContainerPrimitive<
    HashSetOfCacheableKey, internal::DSCode::CacheableHashSet,
    CacheableHashSet>;

/**
 * A mutable <code>Cacheable</code> array list wrapper that can serve as
 * a distributable object for caching.
 */
class APACHE_GEODE_EXPORT CacheableArrayList
    : public internal::CacheableContainerPrimitive<
          std::vector<std::shared_ptr<Cacheable>>,
          internal::DSCode::CacheableArrayList, CacheableArrayList> {
 public:
  using CacheableContainerPrimitive::CacheableContainerPrimitive;
};

template class internal::CacheableContainerPrimitive<
    std::vector<std::shared_ptr<Cacheable>>,
    internal::DSCode::CacheableArrayList, CacheableArrayList>;

/**
 * A mutable <code>Cacheable</code> array list wrapper that can serve as
 * a distributable object for caching.
 */
class APACHE_GEODE_EXPORT CacheableLinkedList
    : public internal::CacheableContainerPrimitive<
          std::vector<std::shared_ptr<Cacheable>>,
          internal::DSCode::CacheableLinkedList, CacheableLinkedList> {
 public:
  using CacheableContainerPrimitive::CacheableContainerPrimitive;
};

template class internal::CacheableContainerPrimitive<
    std::vector<std::shared_ptr<Cacheable>>,
    internal::DSCode::CacheableLinkedList, CacheableLinkedList>;

/**
 * A mutable <code>Cacheable</code> stack wrapper that can serve as
 * a distributable object for caching.
 */
class APACHE_GEODE_EXPORT CacheableStack
    : public internal::CacheableContainerPrimitive<
          std::vector<std::shared_ptr<Cacheable>>,
          internal::DSCode::CacheableStack, CacheableStack> {
 public:
  using CacheableContainerPrimitive::CacheableContainerPrimitive;
};

template class internal::CacheableContainerPrimitive<
    HashMapOfCacheable, internal::DSCode::CacheableStack, CacheableStack>;

/**
 * A mutable <code>CacheableKey</code> to <code>Serializable</code>
 * hash map that can serve as a distributable object for caching.
 */
class APACHE_GEODE_EXPORT CacheableHashTable
    : public internal::CacheableContainerPrimitive<
          HashMapOfCacheable, internal::DSCode::CacheableHashTable,
          CacheableHashTable> {
 public:
  using CacheableContainerPrimitive::CacheableContainerPrimitive;
};

template class internal::CacheableContainerPrimitive<
    HashMapOfCacheable, internal::DSCode::CacheableHashTable,
    CacheableHashTable>;

/**
 * A mutable <code>CacheableKey</code> to <code>Serializable</code>
 * hash map that can serve as a distributable object for caching. This is
 * provided for compability with java side, though is functionally identical
 * to <code>CacheableHashMap</code> i.e. does not provide the semantics of
 * java <code>IdentityHashMap</code>.
 */
class APACHE_GEODE_EXPORT CacheableIdentityHashMap
    : public internal::CacheableContainerPrimitive<
          HashMapOfCacheable, internal::DSCode::CacheableIdentityHashMap,
          CacheableIdentityHashMap> {
 public:
  using CacheableContainerPrimitive::CacheableContainerPrimitive;
};

template class internal::CacheableContainerPrimitive<
    HashMapOfCacheable, internal::DSCode::CacheableIdentityHashMap,
    CacheableIdentityHashMap>;

/**
 * A mutable <code>CacheableKey</code> hash set wrapper that can serve as
 * a distributable object for caching. This is provided for compability
 * with java side, though is functionally identical to
 * <code>CacheableHashSet</code> i.e. does not provide the predictable
 * iteration semantics of java <code>LinkedHashSet</code>.
 */
class APACHE_GEODE_EXPORT CacheableLinkedHashSet
    : public internal::CacheableContainerPrimitive<
          HashSetOfCacheableKey, internal::DSCode::CacheableLinkedHashSet,
          CacheableLinkedHashSet> {
 public:
  using CacheableContainerPrimitive::CacheableContainerPrimitive;
};

template class internal::CacheableContainerPrimitive<
    HashSetOfCacheableKey, internal::DSCode::CacheableLinkedHashSet,
    CacheableLinkedHashSet>;

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHEABLEBUILTINS_H_
