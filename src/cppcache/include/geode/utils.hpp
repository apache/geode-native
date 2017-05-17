#pragma once

// TODO shared_ptr rename to avoid collision with Util.hpp. Consider
// Equality.hpp?

#ifndef GEODE_UTILSX_H_
#define GEODE_UTILSX_H_

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
#include <memory>
#include <type_traits>

namespace apache {
namespace geode {
namespace client {

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

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_UTILSX_H_
