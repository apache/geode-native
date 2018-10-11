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

#ifndef GEODE_HASHMAPOFEXCEPTION_H_
#define GEODE_HASHMAPOFEXCEPTION_H_

#include <memory>
#include <unordered_map>

#include <geode/internal/functional.hpp>

namespace apache {
namespace geode {
namespace client {

using internal::dereference_equal_to;
using internal::dereference_hash;

class CacheableKey;
class Exception;

typedef std::unordered_map<std::shared_ptr<CacheableKey>,
                           std::shared_ptr<Exception>,
                           dereference_hash<std::shared_ptr<CacheableKey>>,
                           dereference_equal_to<std::shared_ptr<CacheableKey>>>
    HashMapOfException;

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_HASHMAPOFEXCEPTION_H_
