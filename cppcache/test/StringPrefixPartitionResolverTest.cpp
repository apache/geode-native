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

#include <geode/CacheableKey.hpp>
#include <geode/EntryEvent.hpp>
#include <geode/StringPrefixPartitionResolver.hpp>

using apache::geode::client::CacheableKey;
using apache::geode::client::EntryEvent;
using apache::geode::client::IllegalArgumentException;
using apache::geode::client::StringPrefixPartitionResolver;

TEST(StringPrefixPartitionResolverTest, testGetName) {
  EXPECT_EQ(StringPrefixPartitionResolver{}.getName(),
            "StringPrefixPartitionResolver");
}

TEST(StringPrefixPartitionResolverTest, testWithNullKey) {
  StringPrefixPartitionResolver pr;
  EntryEvent event{nullptr, nullptr, nullptr, nullptr, nullptr, false};

  auto key = pr.getRoutingObject(event);
  EXPECT_FALSE(key);
}

TEST(StringPrefixPartitionResolverTest, testWithDefaultDelimiter) {
  StringPrefixPartitionResolver pr;
  auto key = CacheableKey::create("prefix|suffix");
  EntryEvent event{nullptr, key, nullptr, nullptr, nullptr, false};

  key = pr.getRoutingObject(event);
  EXPECT_TRUE(key);
  EXPECT_EQ(key->toString(), "prefix");
}

TEST(StringPrefixPartitionResolverTest, testWithCustomDelimiter) {
  StringPrefixPartitionResolver pr{"$#"};
  auto key = CacheableKey::create("prefix$#suffix");
  EntryEvent event{nullptr, key, nullptr, nullptr, nullptr, false};

  key = pr.getRoutingObject(event);
  EXPECT_TRUE(key);
  EXPECT_EQ(key->toString(), "prefix");
}

TEST(StringPrefixPartitionResolverTest, testNoDelimiterFound) {
  StringPrefixPartitionResolver pr;
  auto key = CacheableKey::create("prefix-suffix");
  EntryEvent event{nullptr, key, nullptr, nullptr, nullptr, false};

  EXPECT_THROW(pr.getRoutingObject(event), IllegalArgumentException);
}
