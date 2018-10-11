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

#include <geode/CacheListener.hpp>
#include <geode/EntryEvent.hpp>
#include <geode/Region.hpp>
#include <geode/RegionEvent.hpp>

namespace apache {
namespace geode {
namespace client {

CacheListener::CacheListener() {}

CacheListener::~CacheListener() {}

void CacheListener::close(Region&) {}

void CacheListener::afterCreate(const EntryEvent&) {}

void CacheListener::afterUpdate(const EntryEvent&) {}

void CacheListener::afterInvalidate(const EntryEvent&) {}

void CacheListener::afterDestroy(const EntryEvent&) {}

void CacheListener::afterRegionInvalidate(const RegionEvent&) {}

void CacheListener::afterRegionDestroy(const RegionEvent&) {}

void CacheListener::afterRegionClear(const RegionEvent&) {}

void CacheListener::afterRegionLive(const RegionEvent&) {}

void CacheListener::afterRegionDisconnected(Region&) {}

}  // namespace client
}  // namespace geode
}  // namespace apache
