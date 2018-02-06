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

#include <gtest/gtest.h>

#include <geode/CacheableKey.hpp>
#include <geode/CacheableString.hpp>
#include <geode/CacheableBuiltins.hpp>
#include <geode/CacheableDate.hpp>

using namespace apache::geode::client;

TEST(CacheableKeyCreateTests, forArrayOf_constchar) {
  const auto cacheableKey = CacheableKey::create("test");
  ASSERT_TRUE(nullptr != cacheableKey);
  auto&& cacheableString =
      std::dynamic_pointer_cast<CacheableString>(cacheableKey);
  ASSERT_TRUE(nullptr != cacheableString);
  EXPECT_EQ(cacheableString->value(), "test");
}

TEST(CacheableKeyCreateTests, forArrayOf_char) {
  char* test = new char[10];
  strcpy(test, "test");
  const auto cacheableKey = CacheableKey::create(test);
  ASSERT_TRUE(nullptr != cacheableKey);
  auto&& cacheableString =
      std::dynamic_pointer_cast<CacheableString>(cacheableKey);
  ASSERT_TRUE(nullptr != cacheableString);
  EXPECT_EQ(cacheableString->value(), "test");
}

TEST(CacheableKeyCreateTests, forArrayOf_char16_t) {
  const auto cacheableKey = CacheableKey::create(u"test");
  ASSERT_TRUE(nullptr != cacheableKey);
  auto&& cacheableString =
      std::dynamic_pointer_cast<CacheableString>(cacheableKey);
  ASSERT_TRUE(nullptr != cacheableString);
  EXPECT_EQ(cacheableString->value(), "test");
}

TEST(CacheableKeyCreateTests, forArrayOf_char32_t) {
  const auto cacheableKey = CacheableKey::create(U"test");
  ASSERT_TRUE(nullptr != cacheableKey);
  auto&& cacheableString =
      std::dynamic_pointer_cast<CacheableString>(cacheableKey);
  ASSERT_TRUE(nullptr != cacheableString);
  ASSERT_EQ(cacheableString->value(), "test");
}

TEST(CacheableKeyCreateTests, forArrayOf_wchar_t) {
  const auto cacheableKey = CacheableKey::create(L"test");
  ASSERT_TRUE(nullptr != cacheableKey);
  auto&& cacheableString =
      std::dynamic_pointer_cast<CacheableString>(cacheableKey);
  ASSERT_TRUE(nullptr != cacheableString);
  EXPECT_EQ(cacheableString->value(), "test");
}

TEST(CacheableKeyCreateTests, for_int8_t) {
  const auto cacheableKey = CacheableKey::create(static_cast<int8_t>(1));
  ASSERT_TRUE(nullptr != cacheableKey);
  auto&& cacheableByte = std::dynamic_pointer_cast<CacheableByte>(cacheableKey);
  ASSERT_TRUE(nullptr != cacheableByte);
  EXPECT_EQ(cacheableByte->value(), 1);
}

TEST(CacheableKeyCreateTests, for_int16_t) {
  const auto cacheableKey = CacheableKey::create(static_cast<int16_t>(1));
  ASSERT_TRUE(nullptr != cacheableKey);
  auto&& cacheableInt16 =
      std::dynamic_pointer_cast<CacheableInt16>(cacheableKey);
  ASSERT_TRUE(nullptr != cacheableInt16);
  ASSERT_EQ(cacheableInt16->value(), 1);
}

TEST(CacheableKeyCreateTests, for_int32_t) {
  const auto cacheableKey = CacheableKey::create(static_cast<int32_t>(1));
  ASSERT_TRUE(nullptr != cacheableKey);
  auto&& cacheableInt32 =
      std::dynamic_pointer_cast<CacheableInt32>(cacheableKey);
  ASSERT_TRUE(nullptr != cacheableInt32);
  ASSERT_EQ(cacheableInt32->value(), 1);
}

TEST(CacheableKeyCreateTests, for_int64_t) {
  const auto cacheableKey = CacheableKey::create(static_cast<int64_t>(1));
  ASSERT_TRUE(nullptr != cacheableKey);
  auto&& cacheableInt64 =
      std::dynamic_pointer_cast<CacheableInt64>(cacheableKey);
  ASSERT_TRUE(nullptr != cacheableInt64);
  EXPECT_EQ(cacheableInt64->value(), 1);
}

TEST(CacheableKeyCreateTests, for_char16_t) {
  const auto cacheableKey = CacheableKey::create(u'a');
  ASSERT_TRUE(nullptr != cacheableKey);
  auto&& cacheableCharacter =
      std::dynamic_pointer_cast<CacheableCharacter>(cacheableKey);
  ASSERT_TRUE(nullptr != cacheableCharacter);
  EXPECT_EQ(cacheableCharacter->value(), u'a');
}

TEST(CacheableKeyCreateTests, for_float) {
  const auto cacheableKey = CacheableKey::create(1.1f);
  ASSERT_TRUE(nullptr != cacheableKey);
  auto&& cacheableFloat =
      std::dynamic_pointer_cast<CacheableFloat>(cacheableKey);
  ASSERT_TRUE(nullptr != cacheableFloat);
  EXPECT_EQ(cacheableFloat->value(), 1.1f);
}

TEST(CacheableKeyCreateTests, for_double) {
  const auto cacheableKey = CacheableKey::create(1.1);
  ASSERT_TRUE(nullptr != cacheableKey);
  auto&& cacheableDouble =
      std::dynamic_pointer_cast<CacheableDouble>(cacheableKey);
  ASSERT_TRUE(nullptr != cacheableDouble);
  EXPECT_EQ(cacheableDouble->value(), 1.1);
}

TEST(CacheableKeyCreateTests, for_bool) {
  const auto cacheableKey = CacheableKey::create(true);
  ASSERT_TRUE(nullptr != cacheableKey);
  auto&& cacheableBoolean =
      std::dynamic_pointer_cast<CacheableBoolean>(cacheableKey);
  ASSERT_TRUE(nullptr != cacheableBoolean);
  EXPECT_EQ(cacheableBoolean->value(), true);
}

TEST(CacheableKeyCreateTests, for_timepoint) {
  auto time = std::chrono::system_clock::now();
  const auto cacheableKey = CacheableKey::create(time);
  ASSERT_TRUE(nullptr != cacheableKey);
  auto&& cacheableDate = std::dynamic_pointer_cast<CacheableDate>(cacheableKey);
  ASSERT_TRUE(nullptr != cacheableDate);
  auto tmp = static_cast<typename CacheableDate::time_point>(*cacheableDate);
  EXPECT_EQ(std::chrono::duration_cast<std::chrono::milliseconds>(
                time.time_since_epoch()),
            std::chrono::duration_cast<std::chrono::milliseconds>(
                tmp.time_since_epoch()));
}
