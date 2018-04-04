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

#include <geode/CacheableBuiltins.hpp>

namespace apache {
namespace geode {
namespace client {

#define _GF_CACHEABLE_KEY_DEF_(k) const char tName_##k[] = #k;

_GF_CACHEABLE_KEY_DEF_(CacheableBoolean);
_GF_CACHEABLE_KEY_DEF_(CacheableByte);
_GF_CACHEABLE_KEY_DEF_(CacheableDouble);
_GF_CACHEABLE_KEY_DEF_(CacheableFloat);
_GF_CACHEABLE_KEY_DEF_(CacheableInt16);
_GF_CACHEABLE_KEY_DEF_(CacheableInt32);
_GF_CACHEABLE_KEY_DEF_(CacheableInt64);
_GF_CACHEABLE_KEY_DEF_(CacheableCharacter);
}  // namespace client
}  // namespace geode
}  // namespace apache
