#pragma once

#ifndef GEODE_CACHEABLE_H_
#define GEODE_CACHEABLE_H_

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

/**
 * @file
 */
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include "util/functional.hpp"

#include "geode_globals.hpp"
#include "Serializable.hpp"

namespace apache {
namespace geode {
namespace client {

typedef Serializable Cacheable;

template <typename TVALUE>
inline std::shared_ptr<Cacheable> createValue(
    const std::shared_ptr<TVALUE>& value) {
  return std::dynamic_pointer_cast<Cacheable>(value);
}

class CacheableKey;
typedef std::unordered_map<std::shared_ptr<CacheableKey>,
                           std::shared_ptr<Cacheable>,
                           dereference_hash<std::shared_ptr<CacheableKey>>,
dereference_equal_to<std::shared_ptr<CacheableKey>>>
HashMapOfCacheable;

typedef std::unordered_set<std::shared_ptr<CacheableKey>,
                           dereference_hash<std::shared_ptr<CacheableKey>>,
dereference_equal_to<std::shared_ptr<CacheableKey>>>
HashSetOfCacheableKey;


}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHEABLE_H_
