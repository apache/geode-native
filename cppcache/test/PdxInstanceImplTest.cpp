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

#include <gtest/gtest.h>

#include <geode/AuthenticatedView.hpp>
#include <geode/Properties.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "CacheImpl.hpp"
#include "PdxInstanceImpl.hpp"
#include "statistics/StatisticsFactory.hpp"

using apache::geode::client::Cache;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheImpl;
using apache::geode::client::CachePerfStats;
using apache::geode::client::PdxInstanceImpl;
using apache::geode::client::Properties;
using apache::geode::statistics::StatisticsFactory;

#define __1K__ 1024
#define __100K__ (100 * __1K__)
#define __1M__ (__1K__ * __1K__)

//
// Test to check for memory leak in PdxInstanceImpl::updatePdxStream.  This
// method was leaking a buffer equivalent in size to the passed-in buffer on
// each call.  Test will still pass without the fix in updatePdxStream, but it
// will take *much* longer to complete (60sec+ vs ~4sec on a typical machine).
// Still, to actually verify the fix, you will have to run this test under a
// memory-checking framework of some kind, such as Valgrind or XCode's
// Instruments.
//
TEST(PdxInstanceImplTest, updatePdxStream) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);
  auto buffer = std::vector<uint8_t>(__1M__, 0xcc);
  auto len = static_cast<int32_t>(buffer.size());
  PdxInstanceImpl pdxInstanceImpl(
      buffer.data(), len, 0xdeadbeef, cacheImpl.getCachePerfStats(),
      *(cacheImpl.getPdxTypeRegistry()), cacheImpl, false);

  for (auto i = 0; i < __100K__; i++) {
    try {
      pdxInstanceImpl.updatePdxStream(buffer.data(), len);
    } catch (const std::exception&) {
      GTEST_FAIL();
    }
  }
}
