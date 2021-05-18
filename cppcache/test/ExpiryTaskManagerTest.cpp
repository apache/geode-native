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

#include <geode/ExceptionTypes.hpp>

#include "ExpiryTaskManager.hpp"
#include "mock/MockExpiryTask.hpp"

using apache::geode::client::ExpiryTask;
using apache::geode::client::ExpiryTaskManager;
using apache::geode::client::IllegalStateException;
using apache::geode::client::MockExpiryTask;

TEST(ExpiryTaskManagerTest, startStop) {
  ExpiryTaskManager manager;
  EXPECT_NO_THROW(manager.start());
  EXPECT_TRUE(manager.running());
  EXPECT_NO_THROW(manager.stop());
}

TEST(ExpiryTaskManagerTest, destroyWithoutStart) { ExpiryTaskManager manager; }

TEST(ExpiryTaskManagerTest, destroyWithoutStop) {
  ExpiryTaskManager manager;
  EXPECT_NO_THROW(manager.start());
}

TEST(ExpiryTaskManagerTest, startTwice) {
  ExpiryTaskManager manager;
  EXPECT_NO_THROW(manager.start());
  EXPECT_THROW(manager.start(), IllegalStateException);
}

TEST(ExpiryTaskManagerTest, stopTwice) {
  ExpiryTaskManager manager;
  EXPECT_NO_THROW(manager.start());
  EXPECT_NO_THROW(manager.stop());
  EXPECT_THROW(manager.stop(), IllegalStateException);
}

TEST(ExpiryTaskManagerTest, schedule) {
  ExpiryTaskManager manager;
  EXPECT_NO_THROW(manager.start());

  auto task = std::make_shared<MockExpiryTask>(manager);
  auto id = manager.schedule(std::move(task), std::chrono::seconds(10));
  EXPECT_NE(id, ExpiryTask::invalid());
  EXPECT_EQ(manager.count(), 1U);
}

TEST(ExpiryTaskManagerTest, scheduleStop) {
  ExpiryTaskManager manager;
  EXPECT_NO_THROW(manager.start());

  auto task = std::make_shared<MockExpiryTask>(manager);
  auto id_first = manager.schedule(std::move(task), std::chrono::seconds(10));
  EXPECT_NE(id_first, ExpiryTask::invalid());
  EXPECT_EQ(manager.count(), 1U);

  task = std::make_shared<MockExpiryTask>(manager);
  auto id_second = manager.schedule(std::move(task), std::chrono::seconds(10));
  EXPECT_NE(id_second, ExpiryTask::invalid());
  EXPECT_EQ(manager.count(), 2U);

  EXPECT_NO_THROW(manager.stop());
  EXPECT_EQ(manager.count(), 0U);
}

TEST(ExpiryTaskManagerTest, scheduleCancelStop) {
  ExpiryTaskManager manager;
  EXPECT_NO_THROW(manager.start());

  auto task = std::make_shared<MockExpiryTask>(manager);
  auto id_first = manager.schedule(std::move(task), std::chrono::seconds(10));
  EXPECT_NE(id_first, ExpiryTask::invalid());
  EXPECT_EQ(manager.count(), 1U);

  task = std::make_shared<MockExpiryTask>(manager);
  auto id_second = manager.schedule(std::move(task), std::chrono::seconds(10));
  EXPECT_NE(id_second, ExpiryTask::invalid());
  EXPECT_EQ(manager.count(), 2U);

  EXPECT_EQ(manager.cancel(id_first), 1U);
  EXPECT_EQ(manager.cancel(id_second), 1U);
  EXPECT_NO_THROW(manager.stop());
  EXPECT_EQ(manager.count(), 0U);
}

TEST(ExpiryTaskManagerTest, scheduleWithoutStart) {
  ExpiryTaskManager manager;
  auto task = std::make_shared<MockExpiryTask>(manager);
  auto id = manager.schedule(std::move(task), std::chrono::seconds(1));
  EXPECT_EQ(id, ExpiryTask::invalid());
}

TEST(ExpiryTaskManagerTest, reset) {
  ExpiryTaskManager manager;
  EXPECT_NO_THROW(manager.start());

  auto task = std::make_shared<MockExpiryTask>(manager);
  auto id = manager.schedule(std::move(task), std::chrono::seconds(10));
  EXPECT_NE(id, ExpiryTask::invalid());
  EXPECT_EQ(manager.count(), 1U);
  EXPECT_EQ(manager.reset(id, std::chrono::seconds(1)), 1U);
}

TEST(ExpiryTaskManagerTest, resetInvalid) {
  ExpiryTaskManager manager;
  manager.start();
  auto task = std::make_shared<MockExpiryTask>(manager);
  (void)manager.schedule(std::move(task), std::chrono::seconds(1));
  EXPECT_EQ(manager.reset(10, std::chrono::seconds(1)), -1);
}

TEST(ExpiryTaskManagerTest, cancel) {
  ExpiryTaskManager manager;
  EXPECT_NO_THROW(manager.start());

  auto task = std::make_shared<MockExpiryTask>(manager);
  auto id = manager.schedule(std::move(task), std::chrono::seconds(10));
  EXPECT_NE(id, ExpiryTask::invalid());
  EXPECT_EQ(manager.count(), 1U);
  EXPECT_EQ(manager.cancel(id), 1U);
}

TEST(ExpiryTaskManagerTest, cancelInvalid) {
  ExpiryTaskManager manager;
  manager.start();
  auto task = std::make_shared<MockExpiryTask>(manager);
  (void)manager.schedule(std::move(task), std::chrono::seconds(1));
  EXPECT_EQ(manager.cancel(10), -1);
}
