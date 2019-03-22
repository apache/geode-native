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
#include <unordered_set>

#include <gtest/gtest.h>

#include "TestableRecursiveMutex.hpp"
#include "util/synchronized_set.hpp"

using apache::geode::client::synchronized_set;

TEST(synchronized_setTest, emplaceLocks) {
  synchronized_set<std::unordered_set<std::string>, TestableRecursiveMutex> set;

  auto result = set.emplace("a");
  ASSERT_EQ(true, result.second);
  EXPECT_EQ("a", *result.first);
  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(1, set.mutex().lock_count_);
  EXPECT_EQ(1, set.mutex().unlock_count_);

  result = set.emplace("b");
  ASSERT_EQ(true, result.second);
  EXPECT_EQ("b", *result.first);
  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(2, set.mutex().lock_count_);
  EXPECT_EQ(2, set.mutex().unlock_count_);
}

TEST(synchronized_setTest, eraseKeyLocks) {
  synchronized_set<std::unordered_set<std::string>, TestableRecursiveMutex> set;

  set.emplace("a");
  set.mutex().resetCounters();

  auto result = set.erase("a");
  ASSERT_EQ(1, result);
  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(1, set.mutex().lock_count_);
  EXPECT_EQ(1, set.mutex().unlock_count_);
}

TEST(synchronized_setTest, beginLocks) {
  synchronized_set<std::unordered_set<std::string>, TestableRecursiveMutex> set;

  set.emplace("a");
  set.mutex().resetCounters();

  const auto& begin = set.begin();

  ASSERT_EQ("a", *begin);

  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(0, set.mutex().lock_count_);
  EXPECT_EQ(0, set.mutex().unlock_count_);
}

TEST(synchronized_setTest, beginConstLocks) {
  synchronized_set<std::unordered_set<std::string>, TestableRecursiveMutex> set;

  set.emplace("a");
  set.mutex().resetCounters();

  const auto& constMap = set;
  const auto& begin = constMap.begin();

  ASSERT_EQ("a", *begin);

  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(0, set.mutex().lock_count_);
  EXPECT_EQ(0, set.mutex().unlock_count_);
}

TEST(synchronized_setTest, cbeginLocks) {
  synchronized_set<std::unordered_set<std::string>, TestableRecursiveMutex> set;

  set.emplace("a");
  set.mutex().resetCounters();

  const auto& begin = set.cbegin();

  ASSERT_EQ("a", *begin);

  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(0, set.mutex().lock_count_);
  EXPECT_EQ(0, set.mutex().unlock_count_);
}

TEST(synchronized_setTest, endLocks) {
  synchronized_set<std::unordered_set<std::string>, TestableRecursiveMutex> set;

  set.emplace("a");
  set.mutex().resetCounters();

  const auto& begin = set.begin();
  const auto& end = set.end();

  ASSERT_NE(begin, end);

  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(0, set.mutex().lock_count_);
  EXPECT_EQ(0, set.mutex().unlock_count_);
}

TEST(synchronized_setTest, endConsLocks) {
  synchronized_set<std::unordered_set<std::string>, TestableRecursiveMutex> set;

  set.emplace("a");
  set.mutex().resetCounters();

  const auto& constMap = set;
  const auto& begin = constMap.begin();
  const auto& end = constMap.end();

  ASSERT_NE(begin, end);

  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(0, set.mutex().lock_count_);
  EXPECT_EQ(0, set.mutex().unlock_count_);
}

TEST(synchronized_setTest, cendLocks) {
  synchronized_set<std::unordered_set<std::string>, TestableRecursiveMutex> set;

  set.emplace("a");
  set.mutex().resetCounters();

  const auto& begin = set.cbegin();
  const auto& end = set.cend();

  ASSERT_NE(begin, end);

  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(0, set.mutex().lock_count_);
  EXPECT_EQ(0, set.mutex().unlock_count_);
}

TEST(synchronized_setTest, emptyLocks) {
  synchronized_set<std::unordered_set<std::string>, TestableRecursiveMutex> set;

  ASSERT_TRUE(set.empty());
  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(1, set.mutex().lock_count_);
  EXPECT_EQ(1, set.mutex().unlock_count_);

  set.emplace("a");
  set.mutex().resetCounters();

  ASSERT_FALSE(set.empty());
  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(1, set.mutex().lock_count_);
  EXPECT_EQ(1, set.mutex().unlock_count_);
}

TEST(synchronized_setTest, sizeLocks) {
  synchronized_set<std::unordered_set<std::string>, TestableRecursiveMutex> set;

  ASSERT_EQ(0, set.size());
  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(1, set.mutex().lock_count_);
  EXPECT_EQ(1, set.mutex().unlock_count_);

  set.emplace("a");
  set.mutex().resetCounters();

  ASSERT_EQ(1, set.size());
  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(1, set.mutex().lock_count_);
  EXPECT_EQ(1, set.mutex().unlock_count_);
}

TEST(synchronized_setTest, clearLocks) {
  synchronized_set<std::unordered_set<std::string>, TestableRecursiveMutex> set;

  set.emplace("a");
  set.mutex().resetCounters();

  set.clear();
  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(1, set.mutex().lock_count_);
  EXPECT_EQ(1, set.mutex().unlock_count_);
  EXPECT_TRUE(set.empty());
}

TEST(synchronized_setTest, findNotLocked) {
  synchronized_set<std::unordered_set<std::string>, TestableRecursiveMutex> set;

  set.emplace("a");
  set.mutex().resetCounters();

  {
    std::lock_guard<decltype(set)::mutex_type> lock(set.mutex());
    const auto& entry = set.find("a");
    EXPECT_EQ(1, set.mutex().recursive_depth_);
    EXPECT_EQ(1, set.mutex().lock_count_);
    EXPECT_EQ(0, set.mutex().unlock_count_);
    EXPECT_EQ("a", *entry);
  }
  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(1, set.mutex().lock_count_);
  EXPECT_EQ(1, set.mutex().unlock_count_);
}

TEST(synchronized_setTest, findConstNotLocked) {
  synchronized_set<std::unordered_set<std::string>, TestableRecursiveMutex> set;

  set.emplace("a");
  set.mutex().resetCounters();

  {
    auto&& lock = set.make_lock();
    const auto& constMap = set;
    const auto& entry = constMap.find("a");
    EXPECT_EQ(1, set.mutex().recursive_depth_);
    EXPECT_EQ(1, set.mutex().lock_count_);
    EXPECT_EQ(0, set.mutex().unlock_count_);
    EXPECT_EQ("a", *entry);
  }
  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(1, set.mutex().lock_count_);
  EXPECT_EQ(1, set.mutex().unlock_count_);
}

TEST(synchronized_setTest, iteratorNotLocked) {
  synchronized_set<std::unordered_set<std::string>, TestableRecursiveMutex> set;

  set.emplace("a");
  set.emplace("b");
  set.mutex().resetCounters();

  auto& mutex = set.mutex();
  EXPECT_EQ(0, mutex.recursive_depth_);
  EXPECT_EQ(0, mutex.recursive_depth_);
  EXPECT_EQ(0, set.mutex().lock_count_);
  EXPECT_EQ(0, set.mutex().unlock_count_);

  {
    for (const auto& entry : set) {
      EXPECT_EQ(0, mutex.recursive_depth_);
      EXPECT_EQ(0, set.mutex().lock_count_);
      EXPECT_EQ(0, set.mutex().unlock_count_);
    }
  }
  EXPECT_EQ(0, mutex.recursive_depth_);
  EXPECT_EQ(0, set.mutex().lock_count_);
  EXPECT_EQ(0, set.mutex().unlock_count_);

  {
    std::lock_guard<decltype(set)::mutex_type> lock(mutex);
    for (const auto& entry : set) {
      EXPECT_EQ(1, mutex.recursive_depth_);
      EXPECT_EQ(1, set.mutex().lock_count_);
      EXPECT_EQ(0, set.mutex().unlock_count_);
    }
  }
  EXPECT_EQ(0, mutex.recursive_depth_);
  EXPECT_EQ(1, set.mutex().lock_count_);
  EXPECT_EQ(1, set.mutex().unlock_count_);
}

TEST(synchronized_setTest, make_lockDefault) {
  synchronized_set<std::unordered_set<std::string>, TestableRecursiveMutex> set;

  {
    auto&& lock = set.make_lock();
    EXPECT_EQ(1, set.mutex().recursive_depth_);
    EXPECT_EQ(1, set.mutex().lock_count_);
    EXPECT_EQ(0, set.mutex().unlock_count_);
  }
  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(1, set.mutex().lock_count_);
  EXPECT_EQ(1, set.mutex().unlock_count_);
}

TEST(synchronized_setTest, make_lock_WithUniqueLock) {
  synchronized_set<std::unordered_set<std::string>, TestableRecursiveMutex> set;

  {
    auto&& lock = set.make_lock<std::unique_lock>();
    EXPECT_EQ(1, set.mutex().recursive_depth_);
    EXPECT_EQ(1, set.mutex().lock_count_);
    EXPECT_EQ(0, set.mutex().unlock_count_);
  }
  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(1, set.mutex().lock_count_);
  EXPECT_EQ(1, set.mutex().unlock_count_);
}

TEST(synchronized_setTest, make_lock_WithUniqueLockDefered) {
  synchronized_set<std::unordered_set<std::string>, TestableRecursiveMutex> set;

  {
    auto&& lock = set.make_lock<std::unique_lock>(std::defer_lock);
    EXPECT_EQ(0, set.mutex().recursive_depth_);
    EXPECT_EQ(0, set.mutex().lock_count_);
    EXPECT_EQ(0, set.mutex().unlock_count_);
    lock.lock();
    EXPECT_EQ(1, set.mutex().recursive_depth_);
    EXPECT_EQ(1, set.mutex().lock_count_);
    EXPECT_EQ(0, set.mutex().unlock_count_);
  }
  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(1, set.mutex().lock_count_);
  EXPECT_EQ(1, set.mutex().unlock_count_);
}

TEST(synchronized_setTest, insertIteratorIteratorLocks) {
  std::unordered_set<std::string> source = {"a", "b"};

  synchronized_set<std::unordered_set<std::string>, TestableRecursiveMutex> set;

  set.insert(source.begin(), source.end());
  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(1, set.mutex().lock_count_);
  EXPECT_EQ(1, set.mutex().unlock_count_);
  EXPECT_EQ(2, set.size());
}

TEST(synchronized_setTest, insertRvalue) {
  synchronized_set<std::unordered_set<std::string>, TestableRecursiveMutex> set;

  set.insert("a");
  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(1, set.mutex().lock_count_);
  EXPECT_EQ(1, set.mutex().unlock_count_);
  EXPECT_EQ(1, set.size());
}

TEST(synchronized_setTest, insertLvalue) {
  synchronized_set<std::unordered_set<std::string>, TestableRecursiveMutex> set;

  std::string value = "a";
  set.insert(value);
  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(1, set.mutex().lock_count_);
  EXPECT_EQ(1, set.mutex().unlock_count_);
  EXPECT_EQ(1, set.size());
}

TEST(synchronized_setTest, compilesWithStdSet) {
  synchronized_set<std::set<std::string>, TestableRecursiveMutex> set;

  auto result = set.emplace("a");
  ASSERT_EQ(true, result.second);
  EXPECT_EQ("a", *result.first);
  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(1, set.mutex().lock_count_);
  EXPECT_EQ(1, set.mutex().unlock_count_);

  result = set.emplace("b");
  ASSERT_EQ(true, result.second);
  EXPECT_EQ("b", *result.first);
  EXPECT_EQ(0, set.mutex().recursive_depth_);
  EXPECT_EQ(2, set.mutex().lock_count_);
  EXPECT_EQ(2, set.mutex().unlock_count_);
}
