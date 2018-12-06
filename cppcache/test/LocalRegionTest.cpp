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
using apache::geode::client::RegionAttributesFactory;
using apache::geode::client::RegionShortcut;

/**
 * Cache should close and throw exceptions on methods called after close.
 */
TEST(LocalRegionTest, subRegions) {
  auto cache = CacheFactory{}.set("log-level", "none").create();

  auto rootRegions = cache.rootRegions();
  EXPECT_TRUE(rootRegions.empty());

  auto rootRegion1 =
      cache.createRegionFactory(RegionShortcut::LOCAL).create("rootRegion1");
  EXPECT_NE(nullptr, rootRegion1);

  auto subRegion11 = rootRegion1->createSubregion(
      "subRegion11", RegionAttributesFactory().create());
  EXPECT_NE(nullptr, subRegion11);

  auto subRegion12 = rootRegion1->createSubregion(
      "subRegion12", RegionAttributesFactory().create());
  EXPECT_NE(nullptr, subRegion12);

  auto subRegions1 = rootRegion1->subregions(true);
  EXPECT_EQ(2, subRegions1.size());

  auto rootRegion2 =
      cache.createRegionFactory(RegionShortcut::LOCAL).create("rootRegion2");
  EXPECT_NE(nullptr, rootRegion2);

  auto subRegion21 = rootRegion2->createSubregion(
      "subRegion21", RegionAttributesFactory().create());
  EXPECT_NE(nullptr, subRegion21);

  auto subRegion211 = subRegion21->createSubregion(
      "subRegion211", RegionAttributesFactory().create());
  EXPECT_NE(nullptr, subRegion211);

  auto subRegions2 = rootRegion2->subregions(true);
  EXPECT_EQ(2, subRegions2.size());

  auto subRegions2NonRecursive = rootRegion2->subregions(false);
  EXPECT_EQ(1, subRegions2NonRecursive.size());

  auto rootRegion3 =
      cache.createRegionFactory(RegionShortcut::LOCAL).create("rootRegion3");
  EXPECT_NE(nullptr, rootRegion3);

  auto subRegions3 = rootRegion3->subregions(true);
  EXPECT_EQ(0, subRegions3.size());
}
