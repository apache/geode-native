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

#include "LRUEntryProperties.hpp"
#include "LRUQueue.hpp"
#include "mock/MapEntryImplMock.hpp"

using ::testing::ReturnRef;

using apache::geode::client::CacheableKey;
using apache::geode::client::LRUEntryProperties;
using apache::geode::client::LRUQueue;
using apache::geode::client::MapEntryImplMock;
using apache::geode::client::internal::hashcode;

TEST(LRUQueueTest, create) {
  LRUQueue queue;
  EXPECT_EQ(queue.size(), 0U);
}

TEST(LRUQueueTest, popEmpty) {
  LRUQueue queue;
  EXPECT_FALSE(queue.pop());
}

TEST(LRUQueueTest, pushAndPop) {
  LRUQueue queue;
  const auto N = 5U;
  LRUEntryProperties properties[N];

  // Push entries
  for (auto i = 0U; i < N;) {
    auto key = CacheableKey::create("key-" + std::to_string(i));
    auto entry = std::make_shared<MapEntryImplMock>(key);
    EXPECT_CALL(*entry, getLRUProperties())
        .Times(2)
        .WillRepeatedly(ReturnRef(properties[i]));

    queue.push(entry);
    EXPECT_EQ(queue.size(), ++i);
  }

  // Pop entries and verify its order is the right one
  for (auto i = 0U; i < N; ++i) {
    auto entry = queue.pop();
    EXPECT_TRUE(entry);

    std::shared_ptr<CacheableKey> key;
    entry->getKeyI(key);
    EXPECT_EQ(key->toString(), "key-" + std::to_string(i));
  }
}

TEST(LRUQueueTest, pushAndRemove) {
  LRUQueue queue;
  LRUEntryProperties properties;
  auto key = CacheableKey::create("key");
  auto entry = std::make_shared<MapEntryImplMock>(key);
  EXPECT_CALL(*entry, getLRUProperties())
      .Times(2)
      .WillRepeatedly(ReturnRef(properties));

  queue.push(entry);
  EXPECT_EQ(queue.size(), 1U);

  queue.remove(entry);
  EXPECT_EQ(queue.size(), 0U);
}

TEST(LRUQueueTest, pushMoveAndPop) {
  LRUQueue queue;
  const auto N = 5U;
  const auto MOVE_IDX = 2U;
  LRUEntryProperties properties[N];
  std::shared_ptr<MapEntryImplMock> entries[N];
  std::list<std::shared_ptr<CacheableKey>> keys;

  // Push entries
  for (auto i = 0U; i < N;) {
    auto key = CacheableKey::create("key-" + std::to_string(i));
    auto entry = entries[i] = std::make_shared<MapEntryImplMock>(key);
    EXPECT_CALL(*entry, getLRUProperties())
        .Times(i != MOVE_IDX ? 2 : 3)
        .WillRepeatedly(ReturnRef(properties[i]));

    queue.push(entry);
    keys.push_back(key);
    EXPECT_EQ(queue.size(), ++i);
  }

  // Move `MOVE_IDX`-th entry to the end
  {
    queue.move_to_end(entries[MOVE_IDX]);

    auto&& iter = keys.begin();
    std::advance(iter, MOVE_IDX);
    keys.push_back(*iter);
    keys.erase(iter);
  }

  // Pop entries and verify its order is the right one
  auto k_iter = keys.begin();
  for (auto i = 0U; i < N; ++i) {
    auto entry = queue.pop();
    EXPECT_TRUE(entry);

    std::shared_ptr<CacheableKey> key;
    entry->getKeyI(key);
    EXPECT_EQ(*key, **k_iter++);
  }
}

TEST(LRUQueueTest, pushAndClear) {
  LRUQueue queue;
  const auto N = 5U;
  LRUEntryProperties properties[N];

  for (auto i = 0U; i < N;) {
    auto key = CacheableKey::create("key-" + std::to_string(i));
    auto entry = std::make_shared<MapEntryImplMock>(key);
    EXPECT_CALL(*entry, getLRUProperties())
        .Times(2)
        .WillRepeatedly(ReturnRef(properties[i]));

    queue.push(entry);
    EXPECT_EQ(queue.size(), ++i);
  }

  queue.clear();
  EXPECT_EQ(queue.size(), 0U);
}
