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

#ifndef GEODE_CACHELISTENERMOCK_HPP_
#define GEODE_CACHELISTENERMOCK_HPP_

#include <gmock/gmock.h>

#include <geode/CacheListener.hpp>

namespace apache {
namespace geode {
namespace client {
class CacheListenerMock : public CacheListener {
 public:
  MOCK_METHOD(void, afterCreate, (const EntryEvent&), (override));
  MOCK_METHOD(void, afterUpdate, (const EntryEvent&), (override));
  MOCK_METHOD(void, afterInvalidate, (const EntryEvent&), (override));
  MOCK_METHOD(void, afterDestroy, (const EntryEvent&), (override));
  MOCK_METHOD(void, afterRegionInvalidate, (const RegionEvent&), (override));
  MOCK_METHOD(void, afterRegionDestroy, (const RegionEvent&), (override));
  MOCK_METHOD(void, afterRegionClear, (const RegionEvent&), (override));
  MOCK_METHOD(void, afterRegionLive, (const RegionEvent&), (override));
  MOCK_METHOD(void, close, (Region&), (override));
  MOCK_METHOD(void, afterRegionDisconnected, (Region&), (override));
};

using Nice_MockListener =
    ::testing::NiceMock<CacheListenerMock>;  // Ignores uninteresting calls
using Naggy_MockListener =
    ::testing::NaggyMock<CacheListenerMock>;  // Warns on all uninteresting calls
using Strict_MockListener =
    ::testing::StrictMock<CacheListenerMock>;  // Uninteresting calls are test failures
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHELISTENERMOCK_HPP_
