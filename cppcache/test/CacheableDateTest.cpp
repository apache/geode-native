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

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include <gtest/gtest.h>

#include <geode/CacheableDate.hpp>

using apache::geode::client::CacheableDate;

TEST(CacheableDateTest, constructFromTimeTWithDefault) {
  const CacheableDate cacheableDate;

  EXPECT_EQ(0, cacheableDate.milliseconds());
  EXPECT_EQ(0, static_cast<time_t>(cacheableDate));
}

TEST(CacheableDateTest, constructFromTimeT) {
  time_t time = 0;
  std::time(&time);
  CacheableDate cacheableDate(time);

  EXPECT_EQ(time * 1000, cacheableDate.milliseconds());
  EXPECT_EQ(time, static_cast<time_t>(cacheableDate));
}

TEST(CacheableDateTest, constructFromTimePoint) {
  const auto timePoint = CacheableDate::clock::now();
  const CacheableDate cacheableDate(timePoint);

  const auto millisecondsSinceEpoch =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          timePoint.time_since_epoch());
  EXPECT_EQ(millisecondsSinceEpoch.count(), cacheableDate.milliseconds());
  EXPECT_EQ(
      millisecondsSinceEpoch,
      static_cast<CacheableDate::time_point>(cacheableDate).time_since_epoch());

  const auto duration = static_cast<CacheableDate::duration>(cacheableDate);
  EXPECT_EQ(millisecondsSinceEpoch, duration);

  const auto time = CacheableDate::clock::to_time_t(timePoint);
  EXPECT_EQ(time, static_cast<time_t>(cacheableDate));
}

TEST(CacheableDateTest, constructFromDuration) {
  const auto duration = CacheableDate::duration(1000);
  const CacheableDate cacheableDate(duration);

  EXPECT_EQ(duration.count(), cacheableDate.milliseconds());

  auto timePoint = static_cast<CacheableDate::time_point>(cacheableDate);
  EXPECT_EQ(duration, timePoint.time_since_epoch());

  EXPECT_EQ(duration, static_cast<CacheableDate::duration>(cacheableDate));

  const auto time = CacheableDate::clock::to_time_t(timePoint);
  EXPECT_EQ(time, static_cast<time_t>(cacheableDate));
}

TEST(CacheableDateTest, toString) {
  // output from CacheableString::toString is local and timezone dependent.
  const time_t time = 0;
  const auto localtime = std::localtime(&time);
  std::stringstream stringstream;
  stringstream << std::put_time(localtime, "%c %Z");
  const auto expectedString = stringstream.str();

  const CacheableDate cacheableDate;

  auto string = cacheableDate.toString();
  EXPECT_EQ(expectedString, string);
}

TEST(CacheableDateTest, toStringWithFutureDate) {
  // output from CacheableString::toString is local and timezone dependent.
  const time_t time = 2147512508;
  const auto localtime = std::localtime(&time);
  std::stringstream stringstream;
  stringstream << std::put_time(localtime, "%c %Z");
  const auto expectedString = stringstream.str();

  const CacheableDate cacheableDate(time);

  auto string = cacheableDate.toString();
  EXPECT_EQ(expectedString, string);
}

TEST(CacheableDateTest, toStringWithPastDate) {
  // output from CacheableString::toString is local and timezone dependent.
  const auto past =
      std::chrono::system_clock::from_time_t(0) - std::chrono::hours(100);
  const time_t time = std::chrono::system_clock::to_time_t(past);
  const auto localtime = std::localtime(&time);
#if defined(_WIN32)
  const auto expectedString = std::string("invalid time");
#else
  std::stringstream stringstream;
  stringstream << std::put_time(localtime, "%c %Z");
  const auto expectedString = stringstream.str();
#endif

  const CacheableDate cacheableDate(time);

  auto string = cacheableDate.toString();
  EXPECT_EQ(expectedString, string);
}
