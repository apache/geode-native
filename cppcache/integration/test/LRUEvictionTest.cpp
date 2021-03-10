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

#include <functional>
#include <thread>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/CacheableBuiltins.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionEntry.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

using apache::geode::client::Cache;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::DiskPolicyType;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

TEST(LRUEvictionTest, heapLruLimit) {
  const auto N = 2048U;
  const auto VALUE_SIZE = 1024U;
  const auto LRU_ENTRIES_LIMIT = 1024U;
  const auto MAX_NON_DECREASING_COUNT = 5U;
  const std::chrono::milliseconds DECREASING_SLEEP_INTERVAL{100};
  const auto LRU_LIMIT = VALUE_SIZE * (N - LRU_ENTRIES_LIMIT) >> 20U;

  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("statistic-sampling-enabled", "false")
                   .set("heap-lru-limit", std::to_string(LRU_LIMIT))
                   .set("heap-lru-delta", "10")
                   .create();

  auto region =
      cache.createRegionFactory(RegionShortcut::LOCAL).create("region");

  for (auto i = 0U; i < N; ++i) {
    auto key = CacheableKey::create(std::to_string(i));
    auto value = CacheableString::create(std::string(VALUE_SIZE, '_'));
    region->put(key, value);
  }

  auto counter = 0U;
  auto prev = region->size();
  decltype(prev) n;

  do {
    std::this_thread::sleep_for(DECREASING_SLEEP_INTERVAL);
    if ((n = region->size()) < prev) {
      counter = 0;
      prev = n;
    }
  } while (++counter < MAX_NON_DECREASING_COUNT && n > LRU_ENTRIES_LIMIT);

  EXPECT_LE(n, LRU_ENTRIES_LIMIT);

  // Verify that evicted entries matches LRU criteria
  auto limit = N - n;
  for (const auto& key : region->keys()) {
    EXPECT_GE(std::stoul(key->toString()), limit);
  }
}

TEST(LRUEvictionTest, lruEntriesLimit) {
  const auto N = 2048U;
  const auto LRU_ENTRIES_LIMIT = 1024U;

  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("statistic-sampling-enabled", "false")
                   .create();

  auto region = cache.createRegionFactory(RegionShortcut::LOCAL)
                    .setLruEntriesLimit(LRU_ENTRIES_LIMIT)
                    .create("region");

  auto value = CacheableString::create("value");

  for (auto i = 0U; i < N; ++i) {
    auto key = CacheableKey::create(std::to_string(i));
    region->put(key, value);
  }

  auto n = region->size();
  EXPECT_EQ(n, LRU_ENTRIES_LIMIT);

  // Verify that evicted entries matches LRU criteria
  auto limit = N - n;
  for (const auto& key : region->keys()) {
    EXPECT_GE(std::stoul(key->toString()), limit);
  }
}

TEST(LRUEvictionTest, verifyLruOrderGet) {
  const auto N = 2048U;
  const auto USE_END_OFFSET = 1024U;
  const auto USE_START_OFFSET = 512U;
  const auto LRU_ENTRIES_LIMIT = 1024U;
  const auto START_USE_AT_OFFSET = 1536U;

  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("statistic-sampling-enabled", "false")
                   .create();

  auto region = cache.createRegionFactory(RegionShortcut::LOCAL)
                    .setLruEntriesLimit(LRU_ENTRIES_LIMIT)
                    .create("region");

  for (auto i = 0U; i < START_USE_AT_OFFSET;) {
    auto key = CacheableKey::create(std::to_string(i++));
    auto value = CacheableString::create("value");
    region->put(key, value);
  }

  for (auto i = USE_START_OFFSET; i < USE_END_OFFSET;) {
    auto key = CacheableKey::create(std::to_string(i++));
    region->get(key);
  }

  for (auto i = START_USE_AT_OFFSET; i < N;) {
    auto key = CacheableKey::create(std::to_string(i++));
    auto value = CacheableString::create("value");
    region->put(key, value);
  }

  auto n = region->size();
  EXPECT_EQ(n, LRU_ENTRIES_LIMIT);

  // Verify that evicted entries matches LRU criteria
  for (const auto& entry : region->entries(false)) {
    ASSERT_TRUE(entry->getValue());
    auto key_index = std::stoul(entry->getKey()->toString());

    EXPECT_TRUE((key_index >= USE_START_OFFSET && key_index < USE_END_OFFSET) ||
                (key_index >= START_USE_AT_OFFSET && key_index < N))
        << "Key index (" << key_index << ") is not in range ["
        << USE_START_OFFSET << ", " << USE_END_OFFSET << ") or ["
        << START_USE_AT_OFFSET << ", " << N << ")";
  }
}

TEST(LRUEvictionTest, verifyLruOrderInvalidate) {
  const auto N = 2048U;
  const auto LRU_ENTRIES_LIMIT = 1024U;

  const auto INVALIDATE_1_START_OFFSET = 0U;
  const auto INVALIDATE_1_END_OFFSET = 512U;
  const auto INVALIDATE_1_USE_AT_OFFSET = 1024U;
  const auto INVALIDATE_2_USE_AT_OFFSET = 1536U;

  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("statistic-sampling-enabled", "false")
                   .create();

  auto region = cache.createRegionFactory(RegionShortcut::LOCAL)
                    .setLruEntriesLimit(LRU_ENTRIES_LIMIT)
                    .create("region");

  for (auto i = 0U; i < INVALIDATE_1_USE_AT_OFFSET;) {
    auto key = CacheableKey::create(std::to_string(i++));
    auto value = CacheableString::create("value");
    region->put(key, value);
  }

  for (auto i = INVALIDATE_1_START_OFFSET; i < INVALIDATE_1_END_OFFSET;) {
    auto key = CacheableKey::create(std::to_string(i++));
    region->invalidate(key);
  }

  for (auto i = INVALIDATE_1_USE_AT_OFFSET; i < INVALIDATE_2_USE_AT_OFFSET;) {
    auto key = CacheableKey::create(std::to_string(i++));
    auto value = CacheableString::create("value");
    region->put(key, value);
  }

  for (auto i = INVALIDATE_1_USE_AT_OFFSET; i < INVALIDATE_2_USE_AT_OFFSET;) {
    auto key = CacheableKey::create(std::to_string(i++));
    region->invalidate(key);
  }

  for (auto i = INVALIDATE_2_USE_AT_OFFSET; i < N;) {
    auto key = CacheableKey::create(std::to_string(i++));
    auto value = CacheableString::create("value");
    region->put(key, value);
  }

  auto n = region->size();
  EXPECT_EQ(n, LRU_ENTRIES_LIMIT);

  // Verify that evicted entries matches LRU criteria
  for (const auto& entry : region->entries(false)) {
    auto key_index = std::stoul(entry->getKey()->toString());

    auto valid = entry->getValue() != nullptr;
    bool in_range = (key_index >= INVALIDATE_1_END_OFFSET &&
                     key_index < INVALIDATE_1_USE_AT_OFFSET) ||
                    (key_index >= INVALIDATE_2_USE_AT_OFFSET && key_index < N);

    EXPECT_EQ(in_range, valid)
        << "Assertion verifying key index (" << key_index << ") is in range ["
        << INVALIDATE_1_END_OFFSET << ", " << INVALIDATE_1_USE_AT_OFFSET
        << ") or [" << INVALIDATE_2_USE_AT_OFFSET << ", " << N
        << ") not matching validity test (" << (valid ? "true" : "false")
        << ")";
  }
}

TEST(LRUEvictionTest, verifyLruOrderDestroy) {
  const auto N = 2048U;
  const auto LRU_ENTRIES_LIMIT = 1024U;

  const auto INVALIDATE_1_START_OFFSET = 0U;
  const auto INVALIDATE_1_END_OFFSET = 512U;
  const auto INVALIDATE_1_USE_AT_OFFSET = 1024U;
  const auto INVALIDATE_2_USE_AT_OFFSET = 1536U;

  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("statistic-sampling-enabled", "false")
                   .create();

  auto region = cache.createRegionFactory(RegionShortcut::LOCAL)
                    .setLruEntriesLimit(LRU_ENTRIES_LIMIT)
                    .create("region");

  for (auto i = 0U; i < INVALIDATE_1_USE_AT_OFFSET;) {
    auto key = CacheableKey::create(std::to_string(i++));
    auto value = CacheableString::create("value");
    region->put(key, value);
  }

  for (auto i = INVALIDATE_1_START_OFFSET; i < INVALIDATE_1_END_OFFSET;) {
    auto key = CacheableKey::create(std::to_string(i++));
    region->destroy(key);
  }

  for (auto i = INVALIDATE_1_USE_AT_OFFSET; i < INVALIDATE_2_USE_AT_OFFSET;) {
    auto key = CacheableKey::create(std::to_string(i++));
    auto value = CacheableString::create("value");
    region->put(key, value);
  }

  for (auto i = INVALIDATE_1_USE_AT_OFFSET; i < INVALIDATE_2_USE_AT_OFFSET;) {
    auto key = CacheableKey::create(std::to_string(i++));
    region->destroy(key);
  }

  for (auto i = INVALIDATE_2_USE_AT_OFFSET; i < N;) {
    auto key = CacheableKey::create(std::to_string(i++));
    auto value = CacheableString::create("value");
    region->put(key, value);
  }

  auto n = region->size();
  EXPECT_EQ(n, LRU_ENTRIES_LIMIT);

  // Verify that evicted entries matches LRU criteria
  for (const auto& entry : region->entries(false)) {
    auto key_index = std::stoul(entry->getKey()->toString());

    auto valid = entry->getValue() != nullptr;
    bool in_range = (key_index >= INVALIDATE_1_END_OFFSET &&
                     key_index < INVALIDATE_1_USE_AT_OFFSET) ||
                    (key_index >= INVALIDATE_2_USE_AT_OFFSET && key_index < N);

    EXPECT_EQ(in_range, valid)
        << "Assertion verifying key index (" << key_index << ") is in range ["
        << INVALIDATE_1_END_OFFSET << ", " << INVALIDATE_1_USE_AT_OFFSET
        << ") or [" << INVALIDATE_2_USE_AT_OFFSET << ", " << N
        << ") not matching validity test (" << (valid ? "true" : "false")
        << ")";
  }
}
