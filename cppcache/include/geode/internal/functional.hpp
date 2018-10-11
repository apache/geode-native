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

#ifndef GEODE_UTIL_FUNCTIONAL_H_
#define GEODE_UTIL_FUNCTIONAL_H_

#include <codecvt>
#include <functional>
#include <locale>
#include <memory>
#include <string>
#include <type_traits>

namespace apache {
namespace geode {
namespace client {
namespace internal {

template <class _T>
struct dereference_hash;

template <class _T>
struct dereference_hash<std::shared_ptr<_T>>
    : public std::unary_function<std::shared_ptr<_T>, size_t> {
  size_t operator()(const std::shared_ptr<_T>& val) const {
    return std::hash<_T>{}(*val);
  }
};

template <class _T>
struct dereference_hash<_T*> : public std::unary_function<_T*, size_t> {
  typedef _T* argument_type;
  size_t operator()(const argument_type& val) const {
    return std::hash<_T>{}(*val);
  }
};

template <class _T>
struct dereference_equal_to;

template <class _T>
struct dereference_equal_to<std::shared_ptr<_T>>
    : public std::binary_function<std::shared_ptr<_T>, std::shared_ptr<_T>,
                                  bool> {
  constexpr bool operator()(const std::shared_ptr<_T>& lhs,
                            const std::shared_ptr<_T>& rhs) const {
    return std::equal_to<_T>{}(*lhs, *rhs);
  }
};

template <class _T>
struct dereference_equal_to<_T*> : std::equal_to<_T*> {
  typedef _T* first_argument_type;
  typedef _T* second_argument_type;
  constexpr bool operator()(const first_argument_type& lhs,
                            const second_argument_type& rhs) const {
    return std::equal_to<_T>{}(*lhs, *rhs);
  }
};

/**
 * Hashes based on the same algorithm used in the Geode server.
 *
 * @tparam _T class type to hash.
 */
template <class _T>
struct geode_hash {
  typedef _T argument_type;
  int32_t operator()(const argument_type& val);
};

/**
 * Hashes like java.lang.String
 */
template <>
struct geode_hash<std::u16string> {
  inline int32_t operator()(const std::u16string& val) {
    int32_t hash = 0;
    for (auto&& c : val) {
      hash = 31 * hash + c;
    }
    return hash;
  }
};

/**
 * Hashes like java.lang.String
 */
template <>
struct geode_hash<std::string> {
  int32_t operator()(const std::string& val);
};

}  // namespace internal
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_UTIL_FUNCTIONAL_H_
