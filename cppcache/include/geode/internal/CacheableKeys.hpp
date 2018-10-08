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

#ifndef GEODE_CACHEABLEKEYS_H_
#define GEODE_CACHEABLEKEYS_H_

#include "geode_globals.hpp"

namespace apache {
namespace geode {
namespace client {
namespace internal {

template <typename TObj>
inline bool equals(const TObj& x, const TObj& y) {
  return (x == y);
}

inline int32_t hashcode(const bool value) {
  if (value) {
    return 1231;
  } else {
    return 1237;
  }
}

inline int32_t hashcode(const int8_t value) {
  return static_cast<int32_t>(value);
}

inline int32_t hashcode(const int16_t value) {
  return static_cast<int32_t>(value);
}

inline int32_t hashcode(const int32_t value) { return value; }

inline int32_t hashcode(const int64_t value) {
  int32_t hash = static_cast<int32_t>(value);
  hash = hash ^ static_cast<int32_t>(value >> 32);
  return hash;
}

inline int32_t hashcode(const float value) {
  union float_int32_t {
    float f;
    int32_t u;
  } v;
  v.f = value;
  return v.u;
}

inline int32_t hashcode(const double value) {
  union double_int64_t {
    double d;
    int64_t u;
  } v;
  v.d = value;
  return hashcode(v.u);
}
}  // namespace internal
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHEABLEKEYS_H_
