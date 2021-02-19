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

#include <geode/internal/CacheableKeys.hpp>

using apache::geode::client::internal::hashcode;

TEST(CacheableKeysTest, boolDifferentHashCodes) {
  EXPECT_NE(hashcode(false), hashcode(true))
      << "Two different bool values have different hashcodes";
}

TEST(CacheableKeysTest, int8DifferentHashCodes) {
  EXPECT_NE(hashcode(static_cast<int8_t>(37)),
            hashcode(static_cast<int8_t>(42)))
      << "Two different int8_t values have different hashcodes";
}

TEST(CacheableKeysTest, int16DifferentHashCodes) {
  EXPECT_NE(hashcode(static_cast<int8_t>(37)),
            hashcode(static_cast<int8_t>(42)))
      << "Two different int16_t values have different hashcodes";
}

TEST(CacheableKeysTest, int32DifferentHashCodes) {
  EXPECT_NE(hashcode(static_cast<int8_t>(37)),
            hashcode(static_cast<int8_t>(42)))
      << "Two different int32_t values have different hashcodes";
}

TEST(CacheableKeysTest, int64DifferentHashCodes) {
  EXPECT_NE(hashcode(static_cast<int8_t>(37)),
            hashcode(static_cast<int8_t>(42)))
      << "Two different int64_t values have different hashcodes";
}

TEST(CacheableKeysTest, floatDifferentHashCodes) {
  EXPECT_NE(hashcode(37.F), hashcode(42.F))
      << "Two different float values have different hashcodes";
}

TEST(CacheableKeysTest, doubleDifferentHashCodes) {
  EXPECT_NE(hashcode(37.), hashcode(42.))
      << "Two different double values have different hashcodes";
}
