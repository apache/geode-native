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

#include <atomic>
#include <mutex>
#include <unordered_map>

#include <gtest/gtest.h>

#include "TestableRecursiveMutex.hpp"
#include "internal/synchronized_map.hpp"

using apache::geode::client::synchronized_map;

TEST(SynchronizedMapTest, emplaceLocks) {
  synchronized_map<std::unordered_map<std::string, std::string>,
                   TestableRecursiveMutex>
      map;

  auto result = map.emplace("a", "A");
  ASSERT_EQ(true, result.second);
  EXPECT_EQ("a", result.first->first);
  EXPECT_EQ("A", result.first->second);
  EXPECT_EQ(0, map.mutex().recursive_depth_);
  EXPECT_EQ(1, map.mutex().lock_count_);
  EXPECT_EQ(1, map.mutex().unlock_count_);

  result = map.emplace("b", "B");
  ASSERT_EQ(true, result.second);
  EXPECT_EQ("b", result.first->first);
  EXPECT_EQ("B", result.first->second);
  EXPECT_EQ(0, map.mutex().recursive_depth_);
  EXPECT_EQ(2, map.mutex().lock_count_);
  EXPECT_EQ(2, map.mutex().unlock_count_);
}

TEST(SynchronizedMapTest, eraseKeyLocks) {
  synchronized_map<std::unordered_map<std::string, std::string>,
                   TestableRecursiveMutex>
      map;

  map.emplace("a", "A");
  map.mutex().resetCounters();

  auto result = map.erase("a");
  ASSERT_EQ(1, result);
  EXPECT_EQ(0, map.mutex().recursive_depth_);
  EXPECT_EQ(1, map.mutex().lock_count_);
  EXPECT_EQ(1, map.mutex().unlock_count_);
}

TEST(SynchronizedMapTest, beginLocks) {
  synchronized_map<std::unordered_map<std::string, std::string>,
                   TestableRecursiveMutex>
      map;

  map.emplace("a", "A");
  map.mutex().resetCounters();

  const auto& begin = map.begin();

  ASSERT_EQ("a", begin->first);

  EXPECT_EQ(0, map.mutex().recursive_depth_);
  EXPECT_EQ(0, map.mutex().lock_count_);
  EXPECT_EQ(0, map.mutex().unlock_count_);
}

TEST(SynchronizedMapTest, beginConstLocks) {
  synchronized_map<std::unordered_map<std::string, std::string>,
                   TestableRecursiveMutex>
      map;

  map.emplace("a", "A");
  map.mutex().resetCounters();

  const auto& constMap = map;
  const auto& begin = constMap.begin();

  ASSERT_EQ("a", begin->first);

  EXPECT_EQ(0, map.mutex().recursive_depth_);
  EXPECT_EQ(0, map.mutex().lock_count_);
  EXPECT_EQ(0, map.mutex().unlock_count_);
}

TEST(SynchronizedMapTest, cbeginLocks) {
  synchronized_map<std::unordered_map<std::string, std::string>,
                   TestableRecursiveMutex>
      map;

  map.emplace("a", "A");
  map.mutex().resetCounters();

  const auto& begin = map.cbegin();

  ASSERT_EQ("a", begin->first);

  EXPECT_EQ(0, map.mutex().recursive_depth_);
  EXPECT_EQ(0, map.mutex().lock_count_);
  EXPECT_EQ(0, map.mutex().unlock_count_);
}

TEST(SynchronizedMapTest, endLocks) {
  synchronized_map<std::unordered_map<std::string, std::string>,
                   TestableRecursiveMutex>
      map;

  map.emplace("a", "A");
  map.mutex().resetCounters();

  const auto& begin = map.begin();
  const auto& end = map.end();

  ASSERT_NE(begin, end);

  EXPECT_EQ(0, map.mutex().recursive_depth_);
  EXPECT_EQ(0, map.mutex().lock_count_);
  EXPECT_EQ(0, map.mutex().unlock_count_);
}

TEST(SynchronizedMapTest, endConsLocks) {
  synchronized_map<std::unordered_map<std::string, std::string>,
                   TestableRecursiveMutex>
      map;

  map.emplace("a", "A");
  map.mutex().resetCounters();

  const auto& constMap = map;
  const auto& begin = constMap.begin();
  const auto& end = constMap.end();

  ASSERT_NE(begin, end);

  EXPECT_EQ(0, map.mutex().recursive_depth_);
  EXPECT_EQ(0, map.mutex().lock_count_);
  EXPECT_EQ(0, map.mutex().unlock_count_);
}

TEST(SynchronizedMapTest, cendLocks) {
  synchronized_map<std::unordered_map<std::string, std::string>,
                   TestableRecursiveMutex>
      map;

  map.emplace("a", "A");
  map.mutex().resetCounters();

  const auto& begin = map.cbegin();
  const auto& end = map.cend();

  ASSERT_NE(begin, end);

  EXPECT_EQ(0, map.mutex().recursive_depth_);
  EXPECT_EQ(0, map.mutex().lock_count_);
  EXPECT_EQ(0, map.mutex().unlock_count_);
}

TEST(SynchronizedMapTest, emptyLocks) {
  synchronized_map<std::unordered_map<std::string, std::string>,
                   TestableRecursiveMutex>
      map;

  ASSERT_TRUE(map.empty());
  EXPECT_EQ(0, map.mutex().recursive_depth_);
  EXPECT_EQ(1, map.mutex().lock_count_);
  EXPECT_EQ(1, map.mutex().unlock_count_);

  map.emplace("a", "A");
  map.mutex().resetCounters();

  ASSERT_FALSE(map.empty());
  EXPECT_EQ(0, map.mutex().recursive_depth_);
  EXPECT_EQ(1, map.mutex().lock_count_);
  EXPECT_EQ(1, map.mutex().unlock_count_);
}

TEST(SynchronizedMapTest, sizeLocks) {
  synchronized_map<std::unordered_map<std::string, std::string>,
                   TestableRecursiveMutex>
      map;

  ASSERT_EQ(0, map.size());
  EXPECT_EQ(0, map.mutex().recursive_depth_);
  EXPECT_EQ(1, map.mutex().lock_count_);
  EXPECT_EQ(1, map.mutex().unlock_count_);

  map.emplace("a", "A");
  map.mutex().resetCounters();

  ASSERT_EQ(1, map.size());
  EXPECT_EQ(0, map.mutex().recursive_depth_);
  EXPECT_EQ(1, map.mutex().lock_count_);
  EXPECT_EQ(1, map.mutex().unlock_count_);
}

TEST(SynchronizedMapTest, clearLocks) {
  synchronized_map<std::unordered_map<std::string, std::string>,
                   TestableRecursiveMutex>
      map;

  map.emplace("a", "A");
  map.mutex().resetCounters();

  map.clear();
  EXPECT_EQ(0, map.mutex().recursive_depth_);
  EXPECT_EQ(1, map.mutex().lock_count_);
  EXPECT_EQ(1, map.mutex().unlock_count_);
  EXPECT_TRUE(map.empty());
}

TEST(SynchronizedMapTest, findNotLocked) {
  synchronized_map<std::unordered_map<std::string, std::string>,
                   TestableRecursiveMutex>
      map;

  map.emplace("a", "A");
  map.mutex().resetCounters();

  {
    std::lock_guard<decltype(map)::mutex_type> lock(map.mutex());
    const auto& entry = map.find("a");
    EXPECT_EQ(1, map.mutex().recursive_depth_);
    EXPECT_EQ(1, map.mutex().lock_count_);
    EXPECT_EQ(0, map.mutex().unlock_count_);
    EXPECT_EQ("a", entry->first);
  }
  EXPECT_EQ(0, map.mutex().recursive_depth_);
  EXPECT_EQ(1, map.mutex().lock_count_);
  EXPECT_EQ(1, map.mutex().unlock_count_);
}

TEST(SynchronizedMapTest, findConstNotLocked) {
  synchronized_map<std::unordered_map<std::string, std::string>,
                   TestableRecursiveMutex>
      map;

  map.emplace("a", "A");
  map.mutex().resetCounters();

  {
    auto&& lock = map.make_lock();
    const auto& constMap = map;
    const auto& entry = constMap.find("a");
    EXPECT_EQ(1, map.mutex().recursive_depth_);
    EXPECT_EQ(1, map.mutex().lock_count_);
    EXPECT_EQ(0, map.mutex().unlock_count_);
    EXPECT_EQ("a", entry->first);
  }
  EXPECT_EQ(0, map.mutex().recursive_depth_);
  EXPECT_EQ(1, map.mutex().lock_count_);
  EXPECT_EQ(1, map.mutex().unlock_count_);
}

TEST(SynchronizedMapTest, iteratorNotLocked) {
  synchronized_map<std::unordered_map<std::string, std::string>,
                   TestableRecursiveMutex>
      map;

  map.emplace("a", "A");
  map.emplace("b", "B");
  map.mutex().resetCounters();

  auto& mutex = map.mutex();
  EXPECT_EQ(0, mutex.recursive_depth_);
  EXPECT_EQ(0, mutex.recursive_depth_);
  EXPECT_EQ(0, map.mutex().lock_count_);
  EXPECT_EQ(0, map.mutex().unlock_count_);

  {
    for (const auto& entry : map) {
      EXPECT_EQ(0, mutex.recursive_depth_);
      EXPECT_EQ(0, map.mutex().lock_count_);
      EXPECT_EQ(0, map.mutex().unlock_count_);
    }
  }
  EXPECT_EQ(0, mutex.recursive_depth_);
  EXPECT_EQ(0, map.mutex().lock_count_);
  EXPECT_EQ(0, map.mutex().unlock_count_);

  {
    std::lock_guard<decltype(map)::mutex_type> lock(mutex);
    for (const auto& entry : map) {
      EXPECT_EQ(1, mutex.recursive_depth_);
      EXPECT_EQ(1, map.mutex().lock_count_);
      EXPECT_EQ(0, map.mutex().unlock_count_);
    }
  }
  EXPECT_EQ(0, mutex.recursive_depth_);
  EXPECT_EQ(1, map.mutex().lock_count_);
  EXPECT_EQ(1, map.mutex().unlock_count_);
}

TEST(SynchronizedMapTest, makeLockDefault) {
  synchronized_map<std::unordered_map<std::string, std::string>,
                   TestableRecursiveMutex>
      map;

  {
    auto&& lock = map.make_lock();
    EXPECT_EQ(1, map.mutex().recursive_depth_);
    EXPECT_EQ(1, map.mutex().lock_count_);
    EXPECT_EQ(0, map.mutex().unlock_count_);
  }
  EXPECT_EQ(0, map.mutex().recursive_depth_);
  EXPECT_EQ(1, map.mutex().lock_count_);
  EXPECT_EQ(1, map.mutex().unlock_count_);
}

TEST(SynchronizedMapTest, makeLockWithUniqueLock) {
  synchronized_map<std::unordered_map<std::string, std::string>,
                   TestableRecursiveMutex>
      map;

  {
    auto&& lock = map.make_lock<std::unique_lock>();
    EXPECT_EQ(1, map.mutex().recursive_depth_);
    EXPECT_EQ(1, map.mutex().lock_count_);
    EXPECT_EQ(0, map.mutex().unlock_count_);
  }
  EXPECT_EQ(0, map.mutex().recursive_depth_);
  EXPECT_EQ(1, map.mutex().lock_count_);
  EXPECT_EQ(1, map.mutex().unlock_count_);
}

TEST(SynchronizedMapTest, makeLockWithUniqueLockDefered) {
  synchronized_map<std::unordered_map<std::string, std::string>,
                   TestableRecursiveMutex>
      map;

  {
    auto&& lock = map.make_lock<std::unique_lock>(std::defer_lock);
    EXPECT_EQ(0, map.mutex().recursive_depth_);
    EXPECT_EQ(0, map.mutex().lock_count_);
    EXPECT_EQ(0, map.mutex().unlock_count_);
    lock.lock();
    EXPECT_EQ(1, map.mutex().recursive_depth_);
    EXPECT_EQ(1, map.mutex().lock_count_);
    EXPECT_EQ(0, map.mutex().unlock_count_);
  }
  EXPECT_EQ(0, map.mutex().recursive_depth_);
  EXPECT_EQ(1, map.mutex().lock_count_);
  EXPECT_EQ(1, map.mutex().unlock_count_);
}

TEST(SynchronizedMapTest, insertIteratorIteratorLocks) {
  std::unordered_map<std::string, std::string> source = {{"a", "A"},
                                                         {"b", "B"}};

  synchronized_map<std::unordered_map<std::string, std::string>,
                   TestableRecursiveMutex>
      map;

  map.insert(source.begin(), source.end());
  EXPECT_EQ(0, map.mutex().recursive_depth_);
  EXPECT_EQ(1, map.mutex().lock_count_);
  EXPECT_EQ(1, map.mutex().unlock_count_);
  EXPECT_EQ(2, map.size());
}
