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
struct dereference_hash<std::shared_ptr<_T>> {
  using argument_type = std::shared_ptr<_T>;
  using result_type = std::size_t;

  result_type operator()(const argument_type& val) const {
    return std::hash<_T>{}(*val);
  }
};

template <class _T>
struct dereference_hash<_T*> {
  using argument_type = _T*;
  using result_type = std::size_t;

  result_type operator()(const argument_type& val) const {
    return std::hash<_T>{}(*val);
  }
};

template <class _T>
struct dereference_equal_to;

template <class _T>
struct dereference_equal_to<std::shared_ptr<_T>> {
  using first_argument_type = std::shared_ptr<_T>;
  using second_argument_type = std::shared_ptr<_T>;
  using result_type = bool;

  constexpr result_type operator()(const first_argument_type& lhs,
                                   const second_argument_type& rhs) const {
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
  inline int32_t operator()(const std::string& val) {
    int32_t hash = 0;

    for (auto&& it = val.cbegin(); it < val.cend(); it++) {
      auto cp = static_cast<uint32_t>(0xff & *it);
      if (cp < 0x80) {
        // 1 byte
      } else if ((cp >> 5) == 0x6) {
        // 2 bytes
        ++it;
        cp = ((cp << 6) & 0x7ff) + ((*it) & 0x3f);
      } else if ((cp >> 4) == 0xe) {
        // 3 bytes
        ++it;
        cp = ((cp << 12) & 0xffff) + (((0xff & *it) << 6) & 0xfff);
        ++it;
        cp += (*it) & 0x3f;
      } else if ((cp >> 3) == 0x1e) {
        // 4 bytes
        ++it;
        cp = ((cp << 18) & 0x1fffff) + (((0xff & *it) << 12) & 0x3ffff);
        ++it;
        cp += ((0xff & *it) << 6) & 0xfff;
        ++it;
        cp += (*it) & 0x3f;
      } else {
        // TODO throw exception
      }

      if (cp > 0xffff) {
        // surrogate pair
        hash = 31 * hash +
               static_cast<uint16_t>((cp >> 10) + (0xD800 - (0x10000 >> 10)));
        hash = 31 * hash + static_cast<uint16_t>((cp & 0x3ff) + 0xdc00u);
      } else {
        // single code unit
        hash = 31 * hash + cp;
      }
    }

    return hash;
  }
};

template <>
struct geode_hash<std::vector<int8_t>> {
  inline int32_t operator()(const std::vector<int8_t>& val) {
    int32_t hash = 0;

    for (auto&& it = val.cbegin(); it < val.cend(); it++) {
      auto cp = static_cast<uint32_t>(0xff & *it);
      if (cp < 0x80) {
        // 1 byte
      } else if ((cp >> 5) == 0x6) {
        // 2 bytes
        ++it;
        cp = ((cp << 6) & 0x7ff) + ((*it) & 0x3f);
      } else if ((cp >> 4) == 0xe) {
        // 3 bytes
        ++it;
        cp = ((cp << 12) & 0xffff) + (((0xff & *it) << 6) & 0xfff);
        ++it;
        cp += (*it) & 0x3f;
      } else if ((cp >> 3) == 0x1e) {
        // 4 bytes
        ++it;
        cp = ((cp << 18) & 0x1fffff) + (((0xff & *it) << 12) & 0x3ffff);
        ++it;
        cp += ((0xff & *it) << 6) & 0xfff;
        ++it;
        cp += (*it) & 0x3f;
      } else {
        // TODO throw exception
      }

      if (cp > 0xffff) {
        // surrogate pair
        hash = 31 * hash +
               static_cast<uint16_t>((cp >> 10) + (0xD800 - (0x10000 >> 10)));
        hash = 31 * hash + static_cast<uint16_t>((cp & 0x3ff) + 0xdc00u);
      } else {
        // single code unit
        hash = 31 * hash + cp;
      }
    }

    return hash;
  }
};

}  // namespace internal
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_UTIL_FUNCTIONAL_H_
