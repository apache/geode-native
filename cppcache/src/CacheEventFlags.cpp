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

#include "CacheEventFlags.hpp"

namespace apache {
namespace geode {
namespace client {

const CacheEventFlags CacheEventFlags::NORMAL(CacheEventFlags::GF_NORMAL);
const CacheEventFlags CacheEventFlags::LOCAL(CacheEventFlags::GF_LOCAL);
const CacheEventFlags CacheEventFlags::NOTIFICATION(
    CacheEventFlags::GF_NOTIFICATION);
const CacheEventFlags CacheEventFlags::NOTIFICATION_UPDATE(
    CacheEventFlags::GF_NOTIFICATION_UPDATE);
const CacheEventFlags CacheEventFlags::EVICTION(CacheEventFlags::GF_EVICTION);
const CacheEventFlags CacheEventFlags::EXPIRATION(
    CacheEventFlags::GF_EXPIRATION);
const CacheEventFlags CacheEventFlags::CACHE_CLOSE(
    CacheEventFlags::GF_CACHE_CLOSE);
const CacheEventFlags CacheEventFlags::NOCACHEWRITER(
    CacheEventFlags::GF_NOCACHEWRITER);
const CacheEventFlags CacheEventFlags::NOCALLBACKS(
    CacheEventFlags::GF_NOCALLBACKS);

}  // namespace client
}  // namespace geode
}  // namespace apache
