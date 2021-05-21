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
/* The mock itself creates a circular reference, because the mock retains the
   method parameters for verification. Events contain shared pointers to the
   Region, and the Region contains a shared pointer to the mock.
   
   If you don't break the cycle, then none of the mock, the region, or the
   events see destruction; you get the test framework reporting a memory leak,
   the method calls aren't verified, and your test will pass with a false
   positive.
   
   The best way to resolve the issue is to explicitly call
   ::testing::Mock::VerifyAndClearExpectations on this mock before your intended
   point of destruction - typically it'll be one of the last lines of your test.
   The only other way is to use an AttributeModifier on the region to remove the
   CacheListener, this mock.

   This problem can be permenently fixed if we change the shared_pointer<Region>
   within the event objects to a weak_pointer<Region>, but that necessitates an
   ABI change and a point minor release.
*/
class CacheListenerMock : public CacheListener {
 public:
  MOCK_METHOD1(afterCreate, void(const EntryEvent&));
  MOCK_METHOD1(afterUpdate, void(const EntryEvent&));
  MOCK_METHOD1(afterInvalidate, void(const EntryEvent&));
  MOCK_METHOD1(afterDestroy, void(const EntryEvent&));
  MOCK_METHOD1(afterRegionInvalidate, void(const RegionEvent&));
  MOCK_METHOD1(afterRegionDestroy, void(const RegionEvent&));
  MOCK_METHOD1(afterRegionClear, void(const RegionEvent&));
  MOCK_METHOD1(afterRegionLive, void(const RegionEvent&));
  MOCK_METHOD1(close, void(Region&));
  MOCK_METHOD1(afterRegionDisconnected, void(Region&));
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
