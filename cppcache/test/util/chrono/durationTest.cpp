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

#include <algorithm>
#include <chrono>

#include <gtest/gtest.h>

#include <geode/internal/chrono/duration.hpp>

#include "util/chrono/duration_bounds.hpp"

using apache::geode::internal::chrono::duration::_ceil;
using apache::geode::internal::chrono::duration::from_string;
using apache::geode::internal::chrono::duration::to_string;
using apache::geode::util::chrono::duration::assert_bounds;

TEST(util_chrono_durationTest, ceil) {
  EXPECT_EQ(std::chrono::seconds(1),
            _ceil<std::chrono::seconds>(std::chrono::milliseconds(1)));
  EXPECT_EQ(std::chrono::milliseconds(1),
            _ceil<std::chrono::milliseconds>(std::chrono::milliseconds(1)));
  EXPECT_EQ(std::chrono::microseconds(1000),
            _ceil<std::chrono::milliseconds>(std::chrono::milliseconds(1)));
}

TEST(util_chrono_durationTest, to_string) {
  EXPECT_EQ("42h", to_string(std::chrono::hours(42)));
  EXPECT_EQ("42min", to_string(std::chrono::minutes(42)));
  EXPECT_EQ("42s", to_string(std::chrono::seconds(42)));
  EXPECT_EQ("42ms", to_string(std::chrono::milliseconds(42)));
  EXPECT_EQ("42us", to_string(std::chrono::microseconds(42)));
  EXPECT_EQ("42ns", to_string(std::chrono::nanoseconds(42)));
  EXPECT_EQ("0ns", to_string(std::chrono::nanoseconds(0)));
  EXPECT_EQ("-42ns", to_string(std::chrono::nanoseconds(-42)));

  EXPECT_EQ("100<<unknown units>>",
            to_string(std::chrono::duration<int, std::ratio<1, 100000>>(100)));
}

TEST(util_chrono_durationTest, from_string) {
  EXPECT_EQ(std::chrono::hours(42), from_string("42h"));
  EXPECT_EQ(std::chrono::minutes(42), from_string("42min"));
  EXPECT_EQ(std::chrono::seconds(42), from_string("42s"));
  EXPECT_EQ(std::chrono::milliseconds(42), from_string("42ms"));
  EXPECT_EQ(std::chrono::microseconds(42), from_string("42us"));
  EXPECT_EQ(std::chrono::nanoseconds(42), from_string("42ns"));
  EXPECT_EQ(std::chrono::nanoseconds(0), from_string("0ns"));
  EXPECT_EQ(std::chrono::nanoseconds(-42), from_string("-42ns"));
}

TEST(util_chrono_durationTest, from_stringWithCeil) {
  EXPECT_EQ(std::chrono::hours(42), from_string<std::chrono::hours>("42h"));
  EXPECT_EQ(std::chrono::hours(1), from_string<std::chrono::hours>("42min"));
  EXPECT_EQ(std::chrono::minutes(1), from_string<std::chrono::minutes>("42s"));
  EXPECT_EQ(std::chrono::seconds(1), from_string<std::chrono::seconds>("42ms"));
  EXPECT_EQ(std::chrono::milliseconds(1),
            from_string<std::chrono::milliseconds>("42us"));
  EXPECT_EQ(std::chrono::microseconds(1),
            from_string<std::chrono::microseconds>("42ns"));
  EXPECT_EQ(std::chrono::seconds(2),
            from_string<std::chrono::seconds>("2000ms"));
}

TEST(util_chrono_durationTest, from_stringException) {
  ASSERT_THROW(from_string("42"), std::invalid_argument);
}

TEST(util_chrono_durationTest, assert_bounds) {
  auto protocolTimeoutLimit = assert_bounds<int32_t, std::milli, 0>{};

  ASSERT_NO_THROW(protocolTimeoutLimit(std::chrono::milliseconds(2147483647)));

  ASSERT_THROW(protocolTimeoutLimit(std::chrono::milliseconds(2147483648)),
               apache::geode::client::IllegalArgumentException);

  ASSERT_THROW(protocolTimeoutLimit(std::chrono::hours(2400)),
               apache::geode::client::IllegalArgumentException);

  ASSERT_NO_THROW(protocolTimeoutLimit(std::chrono::milliseconds(0)));

  ASSERT_THROW(protocolTimeoutLimit(std::chrono::milliseconds(-2)),
               apache::geode::client::IllegalArgumentException);

  ASSERT_THROW(protocolTimeoutLimit(std::chrono::hours(-2400)),
               apache::geode::client::IllegalArgumentException);
}
