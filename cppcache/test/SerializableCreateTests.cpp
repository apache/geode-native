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

#include <geode/Serializable.hpp>
#include <geode/CacheableString.hpp>
#include <geode/CacheableBuiltins.hpp>

using namespace apache::geode::client;

TEST(SerializableCreateTests, forArrayOf_char) {
  const auto serializable = Serializable::create("test");
  ASSERT_TRUE(nullptr != serializable);
  auto&& cacheableString =
      std::dynamic_pointer_cast<CacheableString>(serializable);
  ASSERT_TRUE(nullptr != cacheableString);
  ASSERT_EQ(cacheableString->value(), "test");
}

TEST(SerializableCreateTests, forArrayOf_char16_t) {
  const auto serializable = Serializable::create(u"test");
  ASSERT_TRUE(nullptr != serializable);
  auto&& cacheableString =
      std::dynamic_pointer_cast<CacheableString>(serializable);
  ASSERT_TRUE(nullptr != cacheableString);
  ASSERT_EQ(cacheableString->value(), "test");
}

TEST(SerializableCreateTests, forArrayOf_char32_t) {
  const auto serializable = Serializable::create(U"test");
  ASSERT_TRUE(nullptr != serializable);
  auto&& cacheableString =
      std::dynamic_pointer_cast<CacheableString>(serializable);
  ASSERT_TRUE(nullptr != cacheableString);
  ASSERT_EQ(cacheableString->value(), "test");
}

TEST(SerializableCreateTests, forArrayOf_wchar_t) {
  const auto serializable = Serializable::create(L"test");
  ASSERT_TRUE(nullptr != serializable);
  auto&& cacheableString =
      std::dynamic_pointer_cast<CacheableString>(serializable);
  ASSERT_TRUE(nullptr != cacheableString);
  ASSERT_EQ(cacheableString->value(), "test");
}

TEST(SerializableCreateTests, for_uint8_t) {
  const auto serializable = Serializable::create(static_cast<uint8_t>(1));
  ASSERT_TRUE(nullptr != serializable);
  auto&& cacheableByte = std::dynamic_pointer_cast<CacheableByte>(serializable);
  ASSERT_TRUE(nullptr != cacheableByte);
  ASSERT_EQ(cacheableByte->value(), 1);
}

TEST(SerializableCreateTests, for_int16_t) {
  const auto serializable = Serializable::create(static_cast<int16_t>(1));
  ASSERT_TRUE(nullptr != serializable);
  auto&& cacheableInt16 =
      std::dynamic_pointer_cast<CacheableInt16>(serializable);
  ASSERT_TRUE(nullptr != cacheableInt16);
  ASSERT_EQ(cacheableInt16->value(), 1);
}

TEST(SerializableCreateTests, for_int32_t) {
  const auto serializable = Serializable::create(static_cast<int32_t>(1));
  ASSERT_TRUE(nullptr != serializable);
  auto&& cacheableInt32 =
      std::dynamic_pointer_cast<CacheableInt32>(serializable);
  ASSERT_TRUE(nullptr != cacheableInt32);
  ASSERT_EQ(cacheableInt32->value(), 1);
}

TEST(SerializableCreateTests, for_int64_t) {
  const auto serializable = Serializable::create(static_cast<int64_t>(1));
  ASSERT_TRUE(nullptr != serializable);
  auto&& cacheableInt64 =
      std::dynamic_pointer_cast<CacheableInt64>(serializable);
  ASSERT_TRUE(nullptr != cacheableInt64);
  ASSERT_EQ(cacheableInt64->value(), 1);
}

TEST(SerializableCreateTests, for_char16_t) {
  const auto serializable = Serializable::create(u'a');
  ASSERT_TRUE(nullptr != serializable);
  auto&& cacheableCharacter =
      std::dynamic_pointer_cast<CacheableCharacter>(serializable);
  ASSERT_TRUE(nullptr != cacheableCharacter);
  ASSERT_EQ(cacheableCharacter->value(), u'a');
}

TEST(SerializableCreateTests, for_float) {
  const auto serializable = Serializable::create(1.1f);
  ASSERT_TRUE(nullptr != serializable);
  auto&& cacheableFloat =
      std::dynamic_pointer_cast<CacheableFloat>(serializable);
  ASSERT_TRUE(nullptr != cacheableFloat);
  ASSERT_EQ(cacheableFloat->value(), 1.1f);
}

TEST(SerializableCreateTests, for_double) {
  const auto serializable = Serializable::create(1.1);
  ASSERT_TRUE(nullptr != serializable);
  auto&& cacheableDouble =
      std::dynamic_pointer_cast<CacheableDouble>(serializable);
  ASSERT_TRUE(nullptr != cacheableDouble);
  ASSERT_EQ(cacheableDouble->value(), 1.1);
}

TEST(SerializableCreateTests, for_bool) {
  const auto serializable = Serializable::create(true);
  ASSERT_TRUE(nullptr != serializable);
  auto&& cacheableBoolean =
      std::dynamic_pointer_cast<CacheableBoolean>(serializable);
  ASSERT_TRUE(nullptr != cacheableBoolean);
  ASSERT_EQ(cacheableBoolean->value(), true);
}
