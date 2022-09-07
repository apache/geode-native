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

#ifndef GEODE_EVENT_OPERATION_H_
#define GEODE_EVENT_OPERATION_H_

#include <atomic>
#include <cinttypes>
#include <map>
#include <string>
#include <vector>

#include "TcrMessage.hpp"

namespace apache {
namespace geode {
namespace client {

enum class EventOperation {
  MARKER = 0 /* 0 */,
  CREATE /* 1 */,
  PUTALL_CREATE /* 2 */,
  GET /* 3 */,
  GET_ENTRY /* 4 */,
  CONTAINS_KEY /* 5 */,
  CONTAINS_VALUE /* 6 */,
  CONTAINS_VALUE_FOR_KEY /* 7 */,
  FUNCTION_EXECUTION /* 8 */,
  SEARCH_CREATE /* 9 */,
  LOCAL_LOAD_CREATE /* 10 */,
  NET_LOAD_CREATE /* 11 */,
  UPDATE /* 12 */,
  PUTALL_UPDATE /* 13 */,
  SEARCH_UPDATE /* 14 */,
  LOCAL_LOAD_UPDATE /* 15 */,
  NET_LOAD_UPDATE /* 16 */,
  INVALIDATE /* 17 */,
  LOCAL_INVALIDATE /* 18 */,
  DESTROY /* 19 */,
  LOCAL_DESTROY /* 20 */,
  EVICT_DESTROY /* 21 */,
  REGION_LOAD_SNAPSHOT /* 22 */,
  REGION_LOAD_DESTROY /* 23 */,
  REGION_CREATE /* 24 */,
  REGION_CLOSE /* 25 */,
  REGION_DESTROY /* 26 */,
  EXPIRE_DESTROY /* 27 */,
  EXPIRE_LOCAL_DESTROY /* 28 */,
  EXPIRE_INVALIDATE /* 29 */,
  EXPIRE_LOCAL_INVALIDATE /* 30 */,
  REGION_EXPIRE_DESTROY /* 31 */,
  REGION_EXPIRE_LOCAL_DESTROY /* 32 */,
  REGION_EXPIRE_INVALIDATE /* 33 */,
  REGION_EXPIRE_LOCAL_INVALIDATE /* 34 */,
  REGION_LOCAL_INVALIDATE /* 35 */,
  REGION_INVALIDATE /* 36 */,
  REGION_CLEAR /* 37 */,
  REGION_LOCAL_CLEAR /* 38 */,
  CACHE_CREATE /* 39 */,
  CACHE_CLOSE /* 40 */,
  FORCED_DISCONNECT /* 41 */,
  REGION_REINITIALIZE /* 42 */,
  CACHE_RECONNECT /* 43 */,
  PUT_IF_ABSENT /* 44 */,
  REPLACE /* 45 */,
  REMOVE /* 46 */,
  UPDATE_VERSION_STAMP /* 47 */,
  REMOVEALL_DESTROY /* 48 */,
  GET_FOR_REGISTER_INTEREST /* 49 */
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_EVENT_OPERATION_H_
