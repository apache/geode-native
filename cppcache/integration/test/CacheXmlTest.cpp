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

#include <framework/Cluster.h>
#include <framework/Framework.h>
#include <framework/Gfsh.h>
#include <framework/TestConfig.h>
#include <hacks/range.h>

#include <iostream>
#include <unordered_map>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/PoolManager.hpp>
#include <geode/QueryService.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/Struct.hpp>

namespace {

using apache::geode::client::Cache;
using apache::geode::client::CacheXmlException;

apache::geode::client::Cache createCacheUsingXmlConfig(
    const std::string& xmlFile) {
  using apache::geode::client::CacheFactory;

  CacheFactory cacheFactory;

  auto cache = cacheFactory.set("log-level", "none")
                   .set("log-file", "geode_native.log")
                   .set("statistic-sampling-enabled", "false")
                   .set("cache-xml-file", xmlFile.c_str())
                   .create();

  return cache;
}

/**
 * Example test using 2 servers and waiting for async tasks to synchronize using
 * furtures.
 */
TEST(CacheXmlTest, loadCacheXml) {
  auto cacheXml =
      std::string(getFrameworkString(FrameworkVariable::TestCacheXmlDir)) +
      "/valid_cache_refid.xml";
  auto cache = createCacheUsingXmlConfig(cacheXml);
}

/**
 * Example test using 2 servers and waiting for async tasks to synchronize using
 * furtures.
 */
TEST(CacheXmlTest, loadCacheXmlWithBadSchema) {
  auto cacheXml =
      std::string(getFrameworkString(FrameworkVariable::TestCacheXmlDir)) +
      "/bad_schema.xml";
  EXPECT_NO_THROW(createCacheUsingXmlConfig(cacheXml));
}

TEST(CacheXmlTest, loadCacheWithUnnamedPool) {
  Cluster cluster{LocatorCount{1}, ServerCount{2}};
  auto cacheXml =
      std::string(getFrameworkString(FrameworkVariable::TestCacheXmlDir)) +
      "/unnamed_pool.xml";

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();
  EXPECT_THROW(createCacheUsingXmlConfig(cacheXml), CacheXmlException);
}
}  // namespace
