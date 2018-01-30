#pragma once

#ifndef GEODE_MAPWITHLOCK_H_
#define GEODE_MAPWITHLOCK_H_

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

#include <geode/internal/geode_globals.hpp>
#include <geode/CacheableKey.hpp>

#include <ace/Hash_Map_Manager.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/config-lite.h>
#include <ace/Versioned_Namespace.h>

#include <unordered_map>
#include <string>

namespace apache {
namespace geode {
namespace client {

typedef std::unordered_map<std::shared_ptr<CacheableKey>, int,
                           CacheableKey::hash, CacheableKey::equal_to>
    MapOfUpdateCounters;

class Region;
/** Map type used to hold root regions in the Cache, and subRegions. */
typedef ACE_Hash_Map_Manager_Ex<
    std::string, std::shared_ptr<Region>, ACE_Hash<std::string>,
    ACE_Equal_To<std::string>, ACE_Recursive_Thread_Mutex>
    MapOfRegionWithLock;

class CqQuery;
typedef ACE_Hash_Map_Manager_Ex<
    std::string, std::shared_ptr<CqQuery>, ACE_Hash<std::string>,
    ACE_Equal_To<std::string>, ACE_Recursive_Thread_Mutex>
    MapOfCqQueryWithLock;

/** Guard type for locking a MapOfRegionWithLock while iterating or performing
 * other composite operations. ex.. MapOfRegionGuard guard( map->mutex() );
 */
typedef ACE_Guard<ACE_Recursive_Thread_Mutex> MapOfRegionGuard;
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_MAPWITHLOCK_H_
