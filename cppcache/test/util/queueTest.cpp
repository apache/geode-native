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

#include <deque>

#include <gtest/gtest.h>

#include "internal/queue.hpp"

using apache::geode::client::queue::coalesce;

TEST(queueTest, coalesce) {
  auto queue = std::deque<int32_t>({1, 1, 1, 2, 3, 4});

  coalesce(queue, 1);
  EXPECT_EQ(2, queue.front());
  EXPECT_EQ(3, queue.size());

  coalesce(queue, 3);
  EXPECT_EQ(2, queue.front());
  EXPECT_EQ(3, queue.size());
}
