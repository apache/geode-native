#pragma once

#ifndef GEODE_CACHEABLEKEY_H_
#define GEODE_CACHEABLEKEY_H_

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

#include <functional>

#include "geode_globals.hpp"
#include "geode_types.hpp"
#include "Cacheable.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

/** Represents a cacheable key */
class CPPCACHE_EXPORT CacheableKey : public Cacheable {
 protected:
  /** Constructor */
  CacheableKey() : Cacheable() {}

  /** Destructor */
  virtual ~CacheableKey() {}

  FRIEND_STD_SHARED_PTR(CacheableKey)

 public:
  /** return true if this key matches other. */
  virtual bool operator==(const CacheableKey& other) const = 0;

  /** return the hashcode for this key. */
  virtual int32_t hashcode() const = 0;

  /** Copy the string form of a key into a char* buffer for logging purposes.
   *
   * Implementations should only generate a string as long as maxLength chars,
   * and return the number of chars written. buffer is expected to be large
   * enough to hold at least maxLength chars.
   *
   * The default implementation renders the classname and instance address.
   */
  virtual int32_t logString(char* buffer, int32_t maxLength) const;

  /**
   * Factory method that creates the key type that matches the type of value.
   *
   * For customer defined derivations of CacheableKey, the method
   * apache::geode::client::createKey may be overloaded. For pointer types (e.g.
   * char*)
   * the method apache::geode::client::createKeyArr may be overloaded.
   */
  template <class PRIM>
  inline static std::shared_ptr<CacheableKey> create(const PRIM value);

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
