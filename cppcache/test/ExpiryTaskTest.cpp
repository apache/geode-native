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
#include "gmock_extensions.h"
#include "mock/MockExpiryTask.hpp"

using ::testing::DoAll;
using ::testing::InvokeWithoutArgs;
using ::testing::Return;
using ::testing::Sequence;

using apache::geode::client::ExpiryTask;
using apache::geode::client::ExpiryTaskManager;
using apache::geode::client::IllegalStateException;
using apache::geode::client::MockExpiryTask;

constexpr std::chrono::milliseconds DEFAULT_TIMEOUT{100};

TEST(ExpiryTaskTest, scheduleSingleshot) {
  std::mutex cv_mutex;
  ExpiryTaskManager manager;
  std::condition_variable cv;
  std::unique_lock<std::mutex> lock(cv_mutex);

  EXPECT_NO_THROW(manager.start());

  auto task = std::make_shared<MockExpiryTask>(manager);
  EXPECT_CALL(*task, on_expire())
      .Times(1)
      .WillOnce(DoAll(CvNotifyOne(&cv), Return(true)));

  auto id = manager.schedule(std::move(task), std::chrono::seconds(0));
  EXPECT_NE(id, ExpiryTask::invalid());

  EXPECT_EQ(cv.wait_for(lock, DEFAULT_TIMEOUT), std::cv_status::no_timeout);

  EXPECT_NO_THROW(manager.stop());
}

TEST(ExpiryTaskTest, schedulePeriodic) {
  std::mutex cv_mutex;
  ExpiryTaskManager manager;
  std::condition_variable cv;
  std::unique_lock<std::mutex> lock(cv_mutex);

  EXPECT_NO_THROW(manager.start());

  auto task = std::make_shared<MockExpiryTask>(manager);
  {
    Sequence s;
    EXPECT_CALL(*task, on_expire())
        .Times(1)
        .InSequence(s)
        .WillOnce(Return(true));
    EXPECT_CALL(*task, on_expire())
        .Times(1)
        .InSequence(s)
        .WillOnce(DoAll(CvNotifyOne(&cv), Return(true)));
    EXPECT_CALL(*task, on_expire()).InSequence(s).WillRepeatedly(Return(true));
  }
  auto id = manager.schedule(std::move(task), std::chrono::seconds(0),
                             std::chrono::nanoseconds(1));
  EXPECT_NE(id, ExpiryTask::invalid());

  EXPECT_EQ(cv.wait_for(lock, DEFAULT_TIMEOUT), std::cv_status::no_timeout);

  EXPECT_NO_THROW(manager.stop());
}

TEST(ExpiryTaskTest, resetSingleshot) {
  std::mutex cv_mutex;
  ExpiryTaskManager manager;
  std::condition_variable cv;
  std::unique_lock<std::mutex> lock(cv_mutex);

  EXPECT_NO_THROW(manager.start());

  auto task = std::make_shared<MockExpiryTask>(manager);
  auto& task_ref = *task;

  {
    Sequence s;
    EXPECT_CALL(task_ref, on_expire())
        .Times(1)
        .InSequence(s)
        .WillOnce(InvokeWithoutArgs([&task_ref] {
          task_ref.reset(std::chrono::nanoseconds(1));
          return false;
        }));
    EXPECT_CALL(task_ref, on_expire())
        .Times(1)
        .InSequence(s)
        .WillOnce(DoAll(CvNotifyOne(&cv), Return(true)));
  }

  auto id = manager.schedule(std::move(task), std::chrono::seconds(0));
  EXPECT_NE(id, ExpiryTask::invalid());

  EXPECT_EQ(cv.wait_for(lock, DEFAULT_TIMEOUT), std::cv_status::no_timeout);

  EXPECT_NO_THROW(manager.stop());
}

TEST(ExpiryTaskTest, resetPeriodic) {
  std::mutex cv_mutex;
  ExpiryTaskManager manager;
  std::condition_variable cv;
  std::unique_lock<std::mutex> lock(cv_mutex);

  EXPECT_NO_THROW(manager.start());

  auto task = std::make_shared<MockExpiryTask>(manager);
  auto& task_ref = *task;

  {
    Sequence s;
    EXPECT_CALL(task_ref, on_expire())
        .Times(1)
        .InSequence(s)
        .WillOnce(InvokeWithoutArgs([&task_ref] {
          task_ref.reset(std::chrono::nanoseconds(1));
          return false;
        }));
    EXPECT_CALL(task_ref, on_expire())
        .InSequence(s)
        .WillRepeatedly(DoAll(CvNotifyOne(&cv), Return(true)));
  }
  auto id = manager.schedule(std::move(task), std::chrono::seconds(0));
  EXPECT_NE(id, ExpiryTask::invalid());

  EXPECT_EQ(cv.wait_for(lock, DEFAULT_TIMEOUT), std::cv_status::no_timeout);

  EXPECT_NO_THROW(manager.stop());
}
