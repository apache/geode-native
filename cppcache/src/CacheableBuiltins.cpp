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

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <ace/OS.h>

#include <geode/internal/geode_globals.hpp>
#include <geode/CacheableBuiltins.hpp>

namespace apache {
namespace geode {
namespace client {

#define _GF_CACHEABLE_KEY_DEF_(k, s) \
  const char tName_##k[] = #k;       \
  const char tStr_##k[] = s;

_GF_CACHEABLE_KEY_DEF_(CacheableBoolean, "%" PRIi8);
_GF_CACHEABLE_KEY_DEF_(CacheableByte, "%" PRIi8);
_GF_CACHEABLE_KEY_DEF_(CacheableDouble, "%lf");
_GF_CACHEABLE_KEY_DEF_(CacheableFloat, "%f");
_GF_CACHEABLE_KEY_DEF_(CacheableInt16, "%" PRIi16);
_GF_CACHEABLE_KEY_DEF_(CacheableInt32, "%" PRIi32);
_GF_CACHEABLE_KEY_DEF_(CacheableInt64, "%" PRIi64);
_GF_CACHEABLE_KEY_DEF_(CacheableCharacter, "%lc");

template <>
std::shared_ptr<Cacheable> createValue(CacheableBoolean::value_type value) {
  return CacheableBoolean::create(value);
}

template <>
std::shared_ptr<Cacheable> createValue(CacheableByte::value_type value) {
  return CacheableByte::create(value);
}

template <>
std::shared_ptr<Cacheable> createValue(CacheableDouble::value_type value) {
  return CacheableDouble::create(value);
}

template <>
std::shared_ptr<Cacheable> createValue(CacheableFloat::value_type value) {
  return CacheableFloat::create(value);
}

template <>
std::shared_ptr<Cacheable> createValue(CacheableInt16::value_type value) {
  return CacheableInt16::create(value);
}

template <>
std::shared_ptr<Cacheable> createValue(CacheableInt32::value_type value) {
  return CacheableInt32::create(value);
}

template <>
std::shared_ptr<Cacheable> createValue(CacheableInt64::value_type value) {
  return CacheableInt64::create(value);
}

template <>
std::shared_ptr<Cacheable> createValue(CacheableCharacter::value_type value) {
  return CacheableCharacter::create(value);
}

template <>
std::shared_ptr<CacheableKey> createKey(CacheableBoolean::value_type value) {
  return CacheableBoolean::create(value);
}

template <>
std::shared_ptr<CacheableKey> createKey(CacheableByte::value_type value) {
  return CacheableByte::create(value);
}

template <>
std::shared_ptr<CacheableKey> createKey(CacheableDouble::value_type value) {
  return CacheableDouble::create(value);
}

template <>
std::shared_ptr<CacheableKey> createKey(CacheableFloat::value_type value) {
  return CacheableFloat::create(value);
}

template <>
std::shared_ptr<CacheableKey> createKey(CacheableInt16::value_type value) {
  return CacheableInt16::create(value);
}

template <>
std::shared_ptr<CacheableKey> createKey(CacheableInt32::value_type value) {
  return CacheableInt32::create(value);
}

template <>
std::shared_ptr<CacheableKey> createKey(CacheableInt64::value_type value) {
  return CacheableInt64::create(value);
}

template <>
std::shared_ptr<CacheableKey> createKey(CacheableCharacter::value_type value) {
  return CacheableCharacter::create(value);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
