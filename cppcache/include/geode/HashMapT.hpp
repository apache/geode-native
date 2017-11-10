#pragma once

#ifndef GEODE_HASHMAPT_H_
#define GEODE_HASHMAPT_H_

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

#include <unordered_map>
#include <memory>

#include "geode_globals.hpp"
#include "Cacheable.hpp"
#include "CacheableKey.hpp"
#include "Exception.hpp"
#include "util/functional.hpp"

namespace apache {
namespace geode {
namespace client {

typedef std::unordered_map<CacheableKeyPtr, CacheablePtr,
                           dereference_hash<CacheableKeyPtr>,
                           dereference_equal_to<CacheableKeyPtr>>
    HashMapOfCacheable;
typedef std::shared_ptr<HashMapOfCacheable> HashMapOfCacheablePtr;

typedef std::unordered_map<CacheableKeyPtr, ExceptionPtr,
                           dereference_hash<CacheableKeyPtr>,
                           dereference_equal_to<CacheableKeyPtr>>
    HashMapOfException;
typedef std::shared_ptr<HashMapOfException> HashMapOfExceptionPtr;

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_HASHMAPT_H_
