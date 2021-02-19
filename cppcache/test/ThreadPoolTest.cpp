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
#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

#include <gtest/gtest.h>

#include "ThreadPool.hpp"

using apache::geode::client::Callable;
using apache::geode::client::ThreadPool;

class TestCallable : public Callable {
 public:
  TestCallable() : called_(0) {}

  void call() override {
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    called_++;
    condition_.notify_all();
  }

  size_t called_;
  std::mutex mutex_;
  std::condition_variable condition_;
};

TEST(ThreadPoolTest, callableIsCalled) {
  ThreadPool threadPool(1);

  auto c = std::make_shared<TestCallable>();
  threadPool.perform(c);
  std::unique_lock<decltype(c->mutex_)> lock(c->mutex_);
  c->condition_.wait(lock, [&] { return c->called_ > 0; });

  ASSERT_EQ(1, c->called_);
}
