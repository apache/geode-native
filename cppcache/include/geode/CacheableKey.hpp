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

#ifndef GEODE_CACHEABLEKEY_H_
#define GEODE_CACHEABLEKEY_H_

#include <functional>
#include <unordered_map>
#include <unordered_set>

#include "internal/geode_globals.hpp"
#include "Serializable.hpp"
#include "internal/functional.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

/** Represents a cacheable key */
class APACHE_GEODE_EXPORT CacheableKey : public virtual Cacheable {
 protected:
  ~CacheableKey() override = default;

 public:
  CacheableKey() = default;
  /** return true if this key matches other. */
  virtual bool operator==(const CacheableKey& other) const = 0;

  /** return the hashcode for this key. */
  virtual int32_t hashcode() const = 0;

  /**
   * Factory method that creates the key type that matches the type of value.
   *
   * For user defined derivations of CacheableKey, the method
   * apache::geode::client::CacheableKey::create may be overloaded.
   */
  template <class _T>
  static std::shared_ptr<CacheableKey> create(_T value);

  template <class _T>
  inline static std::shared_ptr<CacheableKey> create(
      const std::shared_ptr<_T>& value) {
    return value;
  }

  struct hash {
    inline std::size_t operator()(const CacheableKey& s) const {
      return s.hashcode();
    }

    inline std::size_t operator()(const CacheableKey*& s) const {
      return s->hashcode();
    }

    inline std::size_t operator()(
        const std::shared_ptr<CacheableKey>& s) const {
      return s->hashcode();
    }
  };

  struct equal_to {
    inline bool operator()(const CacheableKey& lhs,
                           const CacheableKey& rhs) const {
      return lhs == rhs;
    }

    inline bool operator()(const CacheableKey*& lhs,
                           const CacheableKey*& rhs) const {
      return (*lhs) == (*rhs);
    }

    inline bool operator()(const std::shared_ptr<CacheableKey>& lhs,
                           const std::shared_ptr<CacheableKey>& rhs) const {
      return (*lhs) == (*rhs);
    }
  };

 private:
  // Never defined.
  CacheableKey(const CacheableKey& other);
  void operator=(const CacheableKey& other);
};

using namespace apache::geode::client::internal;

using HashMapOfCacheable =
    std::unordered_map<std::shared_ptr<CacheableKey>,
                       std::shared_ptr<Cacheable>,
                       dereference_hash<std::shared_ptr<CacheableKey>>,
                       dereference_equal_to<std::shared_ptr<CacheableKey>>>;

using HashSetOfCacheableKey =
    std::unordered_set<std::shared_ptr<CacheableKey>,
                       dereference_hash<std::shared_ptr<CacheableKey>>,
                       dereference_equal_to<std::shared_ptr<CacheableKey>>>;

}  // namespace client
}  // namespace geode
}  // namespace apache

namespace std {

template <>
struct hash<apache::geode::client::CacheableKey> {
  typedef apache::geode::client::CacheableKey argument_type;
  typedef size_t result_type;
  result_type operator()(const argument_type& val) const {
    return val.hashcode();
  }
};

}  // namespace std

#endif  // GEODE_CACHEABLEKEY_H_
