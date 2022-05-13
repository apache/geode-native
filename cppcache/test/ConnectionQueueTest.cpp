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

#include <gmock/gmock.h>

#include <future>
#include <thread>

#include <gtest/gtest.h>

#include "ConnectionQueue.hpp"
#include "gtest_extensions.h"

using ::testing::Eq;
using ::testing::Ge;
using ::testing::Gt;
using ::testing::IsEmpty;
using ::testing::IsFalse;
using ::testing::IsNull;
using ::testing::IsTrue;
using ::testing::Lt;
using ::testing::Not;
using ::testing::SizeIs;

using apache::geode::client::ConnectionQueue;

struct TestObjectState {
  bool closed;
  bool destructed;
};

class TestObject {
 private:
  TestObjectState* const state_;

 public:
  TestObject() : state_(nullptr) {}
  explicit TestObject(TestObjectState* state) : state_(state) {}
  ~TestObject() {
    if (state_) state_->destructed = true;
  }
  void close() {
    if (state_) state_->closed = true;
  }
};

TEST(ConnectionQueueTest, constructedEmpty) {
  ConnectionQueue<TestObject> queue;
  EXPECT_THAT(queue, IsEmpty());
  EXPECT_THAT(queue, SizeIs(0));
}

TEST(ConnectionQueueTest, putOnOpenedIgnoresOpenQueue) {
  ConnectionQueue<TestObject> queue;
  queue.put(new TestObject(), false);
  EXPECT_THAT(queue, Not(IsEmpty()));
  EXPECT_THAT(queue, SizeIs(1));
  queue.put(new TestObject(), true);
  EXPECT_THAT(queue, Not(IsEmpty()));
  EXPECT_THAT(queue, SizeIs(2));
}

TEST(ConnectionQueueTest, putOnClosedDoesNotPutAndObjectClosedAndDestructed) {
  ConnectionQueue<TestObject> queue;
  queue.close();
  TestObjectState state;
  queue.put(new TestObject(&state), false);
  EXPECT_THAT(queue, IsEmpty());
  EXPECT_THAT(queue, SizeIs(0));
  EXPECT_THAT(state.closed, IsTrue());
  EXPECT_THAT(state.destructed, IsTrue());
}

TEST(ConnectionQueueTest, putOnClosedPutsWithOpenQueueTrue) {
  ConnectionQueue<TestObject> queue;
  queue.close();
  queue.put(new TestObject(), true);
  EXPECT_THAT(queue, Not(IsEmpty()));
  EXPECT_THAT(queue, SizeIs(1));
}

TEST(ConnectionQueueTest, closesAndBecomesEmptyAndObjectClosedAndDestructed) {
  ConnectionQueue<TestObject> queue;
  TestObjectState state;
  queue.put(new TestObject(&state), false);
  queue.close();
  EXPECT_THAT(queue, IsEmpty());
  EXPECT_THAT(queue, SizeIs(0));
  EXPECT_THAT(state.closed, IsTrue());
  EXPECT_THAT(state.destructed, IsTrue());
}

TEST(ConnectionQueueTest, put) {
  ConnectionQueue<TestObject> queue;
  queue.put(new TestObject(), false);
  EXPECT_THAT(queue, Not(IsEmpty()));
  EXPECT_THAT(queue, SizeIs(1));
  queue.put(new TestObject(), false);
  EXPECT_THAT(queue, Not(IsEmpty()));
  EXPECT_THAT(queue, SizeIs(2));
}

TEST(ConnectionQueueTest, getNoWaitWith1EntryEmptiesQueue) {
  ConnectionQueue<TestObject> queue;
  const auto expected = new TestObject();
  queue.put(expected, true);
  const auto actual = queue.getNoWait();
  EXPECT_THAT(queue, IsEmpty());
  EXPECT_THAT(queue, SizeIs(0));
  EXPECT_THAT(actual, Eq(expected));
  delete expected;
}

TEST(ConnectionQueueTest, getNoWaitOnEmptyReturnsNullptr) {
  ConnectionQueue<TestObject> queue;
  auto actual = queue.getNoWait();
  EXPECT_THAT(actual, IsNull());
}

TEST(ConnectionQueueTest, getUntilOnEmptyReturnsNullptr) {
  ConnectionQueue<TestObject> queue;
  const auto actual = queue.getUntil(std::chrono::seconds::zero());
  EXPECT_THAT(actual, IsNull());
}

TEST(ConnectionQueueTest, getUntilOnEmptyWaitsAndReturnsNullptr) {
  ConnectionQueue<TestObject> queue;
  const auto wait = std::chrono::seconds(5);
  const auto start = std::chrono::steady_clock::now();
  const auto actual = queue.getUntil(wait);
  const auto elapsed = std::chrono::steady_clock::now() - start;
  EXPECT_THAT(actual, IsNull());
  EXPECT_THAT(elapsed, Ge(wait));
}

TEST(ConnectionQueueTest, getUntilWith1EntryDoesNotWait) {
  ConnectionQueue<TestObject> queue;
  const auto expected = new TestObject();
  queue.put(expected, true);
  const auto wait = std::chrono::seconds(5);
  const auto start = std::chrono::steady_clock::now();
  const auto actual = queue.getUntil(wait);
  const auto elapsed = std::chrono::steady_clock::now() - start;
  EXPECT_THAT(actual, Eq(expected));
  EXPECT_THAT(elapsed, Lt(wait));
  delete expected;
}

TEST(ConnectionQueueTest,
     getUntilOnEmptyWaitsAndReturnsObjectPutByAnotherThread) {
  using std::chrono::minutes;
  using std::chrono::seconds;
  using std::chrono::steady_clock;

  ConnectionQueue<TestObject> queue;

  const auto pause = seconds(1);
  const auto expected = new TestObject();

  auto task1 = std::async(std::launch::async, [&] {
    const auto wait = seconds(5);
    const auto start = steady_clock::now();
    const auto actual = queue.getUntil(wait);
    const auto elapsed = steady_clock::now() - start;

    EXPECT_THAT(actual, Eq(expected));
    EXPECT_THAT(elapsed, Lt(wait));
    EXPECT_THAT(elapsed, Gt(pause));

    delete expected;
  });

  std::this_thread::sleep_for(pause);
  queue.put(expected, true);
  ASSERT_THAT(task1.wait_for(minutes(1)), Eq(std::future_status::ready));
}

TEST(ConnectionQueueTest, getUntilOnEmptyWaitsAndReturnsNullWhenClosed) {
  using std::chrono::minutes;
  using std::chrono::seconds;
  using std::chrono::steady_clock;

  ConnectionQueue<TestObject> queue;

  const auto pause = seconds(1);
  const auto expected = new TestObject();

  auto task1 = std::async(std::launch::async, [&] {
    const auto wait = seconds(5);
    const auto start = steady_clock::now();
    const auto actual = queue.getUntil(wait);
    const auto elapsed = steady_clock::now() - start;

    EXPECT_THAT(actual, IsNull());
    EXPECT_THAT(elapsed, Lt(wait));
    EXPECT_THAT(elapsed, Gt(pause));

    delete expected;
  });

  std::this_thread::sleep_for(pause);
  queue.close();
  ASSERT_THAT(task1.wait_for(minutes(1)), Eq(std::future_status::ready));
}
