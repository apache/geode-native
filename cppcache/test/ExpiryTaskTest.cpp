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
#include "internal/concurrent/binary_semaphore.hpp"
#include "mock/MockExpiryTask.hpp"

using ::testing::DoAll;
using ::testing::InvokeWithoutArgs;
using ::testing::Return;
using ::testing::Sequence;

using apache::geode::client::binary_semaphore;

using apache::geode::client::ExpiryTask;
using apache::geode::client::ExpiryTaskManager;
using apache::geode::client::IllegalStateException;
using apache::geode::client::MockExpiryTask;

constexpr std::chrono::milliseconds DEFAULT_TIMEOUT{100};

TEST(ExpiryTaskTest, scheduleSingleshot) {
  binary_semaphore sem{0};
  ExpiryTaskManager manager;
  EXPECT_NO_THROW(manager.start());

  auto task = std::make_shared<MockExpiryTask>(manager);
  EXPECT_CALL(*task, on_expire())
      .Times(1)
      .WillOnce(DoAll(ReleaseSem(&sem), Return(true)));

  auto id = manager.schedule(std::move(task), std::chrono::seconds(0));
  EXPECT_NE(id, ExpiryTask::invalid());

  EXPECT_TRUE(sem.try_acquire_for(DEFAULT_TIMEOUT));

  EXPECT_NO_THROW(manager.stop());
}

TEST(ExpiryTaskTest, schedulePeriodic) {
  binary_semaphore sem{0};
  ExpiryTaskManager manager;

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
        .WillOnce(DoAll(ReleaseSem(&sem), Return(true)));
    EXPECT_CALL(*task, on_expire()).InSequence(s).WillRepeatedly(Return(true));
  }
  auto id = manager.schedule(std::move(task), std::chrono::seconds(0),
                             std::chrono::nanoseconds(1));
  EXPECT_NE(id, ExpiryTask::invalid());

  EXPECT_TRUE(sem.try_acquire_for(DEFAULT_TIMEOUT));

  EXPECT_NO_THROW(manager.stop());
}

TEST(ExpiryTaskTest, resetSingleshot) {
  binary_semaphore sem{0};
  ExpiryTaskManager manager;

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
        .WillOnce(DoAll(ReleaseSem(&sem), Return(true)));
  }

  auto id = manager.schedule(std::move(task), std::chrono::seconds(0));
  EXPECT_NE(id, ExpiryTask::invalid());

  EXPECT_TRUE(sem.try_acquire_for(DEFAULT_TIMEOUT));

  EXPECT_NO_THROW(manager.stop());
}

TEST(ExpiryTaskTest, resetPeriodic) {
  binary_semaphore sem{0};
  ExpiryTaskManager manager;

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
        .WillRepeatedly(DoAll(ReleaseSem(&sem), Return(true)));
  }
  auto id = manager.schedule(std::move(task), std::chrono::seconds(0));
  EXPECT_NE(id, ExpiryTask::invalid());

  EXPECT_TRUE(sem.try_acquire_for(DEFAULT_TIMEOUT));

  EXPECT_NO_THROW(manager.stop());
}
