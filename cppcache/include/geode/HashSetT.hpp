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

#ifndef GEODE_HASHSETT_H_
#define GEODE_HASHSETT_H_

#include <unordered_set>
#include <memory>

#include "geode_globals.hpp"
#include "CacheableKey.hpp"
#include "util/functional.hpp"

namespace apache {
namespace geode {
namespace client {

typedef std::unordered_set<std::shared_ptr<CacheableKey>,
                           dereference_hash<std::shared_ptr<CacheableKey>>,
                           dereference_equal_to<std::shared_ptr<CacheableKey>>>
    HashSetOfCacheableKey;

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_HASHSETT_H_
