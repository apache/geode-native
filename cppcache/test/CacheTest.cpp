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
#include <geode/Cache.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

using apache::geode::client::CacheClosedException;
using apache::geode::client::CacheFactory;
using apache::geode::client::LogLevel;
using apache::geode::client::RegionShortcut;

/**
 * Cache should close and throw exceptions on methods called after close.
 */
TEST(CacheTest, close) {
  auto cache = CacheFactory{}.set("log-level", "none").create();
  ASSERT_FALSE(cache.isClosed());

  cache.close();
  ASSERT_TRUE(cache.isClosed());

  EXPECT_THROW(cache.close(), CacheClosedException);
  EXPECT_THROW(cache.close(true), CacheClosedException);
  EXPECT_THROW(cache.createAuthenticatedView(nullptr, "pool"),
               CacheClosedException);
  EXPECT_THROW(cache.createDataInput(nullptr, 0), CacheClosedException);
  EXPECT_THROW(cache.createDataOutput(), CacheClosedException);
  EXPECT_THROW(cache.createPdxInstanceFactory("classname"),
               CacheClosedException);
  EXPECT_THROW(cache.createRegionFactory(RegionShortcut::LOCAL),
               CacheClosedException);
  EXPECT_THROW(cache.getCacheTransactionManager(), CacheClosedException);
  EXPECT_THROW(cache.getName(), CacheClosedException);
  EXPECT_THROW(cache.getPdxIgnoreUnreadFields(), CacheClosedException);
  EXPECT_THROW(cache.getPdxReadSerialized(), CacheClosedException);
  EXPECT_THROW(cache.getPoolManager(), CacheClosedException);
  EXPECT_THROW(cache.getQueryService(), CacheClosedException);
  EXPECT_THROW(cache.getQueryService("pool"), CacheClosedException);
  EXPECT_THROW(cache.getRegion("region"), CacheClosedException);
  EXPECT_THROW(cache.getSystemProperties(), CacheClosedException);
  EXPECT_THROW(cache.getTypeRegistry(), CacheClosedException);
  EXPECT_THROW(cache.initializeDeclarativeCache(""), CacheClosedException);
  EXPECT_THROW(cache.readyForEvents(), CacheClosedException);
  EXPECT_THROW(cache.rootRegions(), CacheClosedException);
}

TEST(CacheTest, DISABLED_changeLogLevel) {
  // TODO: fix this test!  Disabling for now to make progress on replacing
  // Log class
  auto cache = CacheFactory{}.set("log-level", "info").create();
  ASSERT_EQ(cache.getLogLevel(), LogLevel::Info);
  cache.setLogLevel(LogLevel::Debug);
  ASSERT_EQ(cache.getLogLevel(), LogLevel::Debug);
}
