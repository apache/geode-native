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

#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

#include <geode/DataOutput.hpp>
#include <geode/ExceptionTypes.hpp>

#include "ByteArrayFixture.hpp"
#include "DataInputInternal.hpp"
#include "DataOutputInternal.hpp"
#include "SerializationRegistry.hpp"
#include "util/JavaModifiedUtf8.hpp"

namespace {

using apache::geode::client::ByteArray;
using apache::geode::client::CacheableString;
using apache::geode::client::DataInputInternal;
using apache::geode::client::DataOutput;
using apache::geode::client::DataOutputInternal;
using apache::geode::client::IllegalArgumentException;
using apache::geode::client::SerializationRegistry;
using apache::geode::client::internal::JavaModifiedUtf8;

class TestDataOutput : public DataOutputInternal {
 public:
  TestDataOutput()
      : DataOutputInternal(), m_byteArray(nullptr), m_serializationRegistry() {
    // NOP
  }

  ~TestDataOutput() noexcept override {
    delete m_byteArray;
    m_byteArray = nullptr;
  }

  const ByteArray& getByteArray() const {
    if (!m_byteArray) {
      m_byteArray = new ByteArray(getBuffer(), getBufferLength());
    }
    return *m_byteArray;
  }

 protected:
  virtual const SerializationRegistry& getSerializationRegistry()
      const override {
    return m_serializationRegistry;
  }

 private:
  mutable ByteArray* m_byteArray;
  SerializationRegistry m_serializationRegistry;
};

class CacheableStringTests : public ::testing::Test, public ByteArrayFixture {
 public:
 protected:
};

inline std::string to_hex(const uint8_t* bytes, size_t len) {
  std::stringstream ss;
  ss << std::setfill('0') << std::hex;
  for (size_t i(0); i < len; ++i) {
    ss << std::setw(2) << static_cast<int>(bytes[i]);
  }
  return ss.str();
}

inline std::string to_hex(const DataOutput& out) {
  return to_hex(out.getBuffer(), out.getBufferLength());
}

TEST_F(CacheableStringTests, CreateFromStdStringRValue) {
  auto s = std::string("You had me at meat tornado.");
  auto m = s;
  auto c = CacheableString::create(std::move(m));

  ASSERT_TRUE(m.empty());
  ASSERT_EQ(s, c->value());
}

TEST_F(CacheableStringTests, CreateFromStdStringLValue) {
  auto s = std::string("You had me at meat tornado.");
  auto c = CacheableString::create(s);

  ASSERT_FALSE(s.empty());
  ASSERT_EQ(s, c->value());
}

TEST_F(CacheableStringTests, TestToDataAscii) {
  auto origStr = CacheableString::create("You had me at meat tornado.");
  DataOutputInternal out;
  origStr->toData(out);

  EXPECT_EQ("001b596f7520686164206d65206174206d65617420746f726e61646f2e",
            to_hex(out));
}

TEST_F(CacheableStringTests, TestFromDataAscii) {
  std::string utf8("You had me at meat tornado.");
  auto origStr = CacheableString::create(utf8.c_str());
  DataOutputInternal out;
  origStr->toData(out);

  auto str = std::dynamic_pointer_cast<CacheableString>(
      CacheableString::createUTFDeserializable());
  DataInputInternal in(out.getBuffer(), out.getBufferLength());
  str->fromData(in);

  EXPECT_EQ(utf8, str->value());
}

TEST_F(CacheableStringTests, TestToDataNonAscii) {
  auto origStr = CacheableString::create(u8"You had me at meat tornad\u00F6.");
  DataOutputInternal out;
  origStr->toData(out);

  EXPECT_EQ("001c596f7520686164206d65206174206d65617420746f726e6164c3b62e",
            to_hex(out));
}

TEST_F(CacheableStringTests, TestFromDataNonAscii) {
  std::string utf8(u8"You had me at meat tornad\u00F6.");
  auto origStr = CacheableString::create(utf8);
  DataOutputInternal out;
  origStr->toData(out);

  auto str = std::dynamic_pointer_cast<CacheableString>(
      CacheableString::createUTFDeserializable());
  DataInputInternal in(out.getBuffer(), out.getBufferLength());
  str->fromData(in);

  EXPECT_EQ(utf8, str->value());
}

TEST_F(CacheableStringTests, TestToDataAsciiHuge) {
  size_t originalLen = std::numeric_limits<uint16_t>::max();
  std::string utf8(originalLen, 'a');
  utf8.append("a");
  originalLen++;

  auto origStr = CacheableString::create(utf8.c_str());
  TestDataOutput out;
  origStr->toData(out);

  auto&& bufLen = originalLen + 4;  // strLen (unit32_t)
  EXPECT_EQ(bufLen, out.getBufferLength());

  // 0x00010000 - length
  // 0x61 - first 'a'
  // EXPECT_MATCH("0001000061.*61", to_hex(out));
  EXPECT_BYTEARRAY_EQ("0001000061\\h{131068}61", out.getByteArray());
}

TEST_F(CacheableStringTests, TestFromDataAsciiHuge) {
  std::string utf8(std::numeric_limits<uint16_t>::max(), 'a');
  utf8.append("a");

  auto origStr = CacheableString::create(utf8.c_str());
  DataOutputInternal out;
  origStr->toData(out);

  auto str = std::dynamic_pointer_cast<CacheableString>(
      CacheableString::createDeserializableHuge());
  DataInputInternal in(out.getBuffer(), out.getBufferLength());
  str->fromData(in);

  EXPECT_EQ(utf8, str->value());
}

TEST_F(CacheableStringTests, TestToDataNonAsciiHuge) {
  std::string utf8(std::numeric_limits<uint16_t>::max(), 'a');
  utf8.append(u8"\u00E4");
  auto&& utf16 =
      std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}
          .from_bytes(utf8);

  auto origStr = CacheableString::create(utf8.c_str());
  TestDataOutput out;
  origStr->toData(out);

  // utf-16 units length (unit32_t) + utf-16 units (uint16_t*)
  auto&& bufLen = 4 + utf16.length() * sizeof(uint16_t);
  EXPECT_EQ(bufLen, out.getBufferLength());

  // 0x00010000 - length
  // 0x0061 - first 'a'
  // 0x006100E4 - last 'a\u00e4'
  // EXPECT_MATCH("000100000061.*006100e4", to_hex(out));
  EXPECT_BYTEARRAY_EQ("000100000061\\h{262132}006100E4", out.getByteArray());
}

TEST_F(CacheableStringTests, TestFromDataNonAsciiHuge) {
  std::string utf8(std::numeric_limits<uint16_t>::max(), 'a');
  utf8.append(u8"\u00E4");

  auto origStr = CacheableString::create(utf8);
  DataOutputInternal out;
  origStr->toData(out);

  auto str = std::dynamic_pointer_cast<CacheableString>(
      CacheableString::createUTFDeserializableHuge());
  DataInputInternal in(out.getBuffer(), out.getBufferLength());
  str->fromData(in);

  EXPECT_EQ(utf8, str->value());
}

std::vector<int> impossible_bytes[] = {
    {0xFE}, {0xFF}, {0xFE, 0xFE, 0xFF, 0xFF}};

std::vector<int> overlong_ascii_sequences[] = {
    {0xC0, 0xAF}, {0xE0, 0x80, 0xAF}, {0xF0, 0x80, 0x80, 0xAF}};

std::vector<int> maximum_overlong_sequences[] = {
    {0xC1, 0xBF}, {0xE0, 0x9F, 0xBF}, {0xF0, 0x8F, 0xBF, 0xBF}};

std::vector<int> overlong_nulls[] = {
    {0xC0, 0x80}, {0xE0, 0x80, 0x80}, {0xF0, 0x80, 0x80, 0x80}};

std::vector<int> single_utf_16_surrogates[] = {
    {0xED, 0xA0, 0x80}, {0xED, 0xAD, 0xBF}, {0xED, 0xAE, 0x80},
    {0xED, 0xAF, 0xBF}, {0xED, 0xB0, 0x80}, {0xED, 0xBE, 0x80},
    {0xED, 0xBF, 0xBF}};

TEST_F(CacheableStringTests, TestUtf8ToJmUtf8BadStrings) {
  std::string bad_start_code;
  bad_start_code += static_cast<int8_t>(0xF8);
  EXPECT_THROW(JavaModifiedUtf8::fromString(bad_start_code),
               IllegalArgumentException);

  std::string too_short_2byte;
  too_short_2byte += static_cast<int8_t>(0xC0);
  EXPECT_THROW(JavaModifiedUtf8::fromString(too_short_2byte),
               IllegalArgumentException);

  std::string bad_2byte_at_end = "foo";
  bad_2byte_at_end += static_cast<int8_t>(0xC0);
  EXPECT_THROW(JavaModifiedUtf8::fromString(bad_2byte_at_end),
               IllegalArgumentException);

  std::string too_long_3_byte_encode;
  too_long_3_byte_encode.push_back(0xE0);
  too_long_3_byte_encode.push_back(0x80);
  too_long_3_byte_encode.push_back(0x80);
  EXPECT_THROW(JavaModifiedUtf8::fromString(too_long_3_byte_encode),
               IllegalArgumentException);

  std::string too_short_3byte;
  too_short_3byte += static_cast<int8_t>(0xE8);
  EXPECT_THROW(JavaModifiedUtf8::fromString(too_short_3byte),
               IllegalArgumentException);

  too_short_3byte += static_cast<int8_t>(0x1);
  EXPECT_THROW(JavaModifiedUtf8::fromString(too_short_3byte),
               IllegalArgumentException);

  std::string bad_3byte_at_end = "foo";
  bad_3byte_at_end += static_cast<int8_t>(0xE8);
  EXPECT_THROW(JavaModifiedUtf8::fromString(bad_3byte_at_end),
               IllegalArgumentException);

  bad_3byte_at_end += static_cast<int8_t>(0x1);
  EXPECT_THROW(JavaModifiedUtf8::fromString(bad_3byte_at_end),
               IllegalArgumentException);

  std::string too_short_4byte;
  too_short_4byte += static_cast<int8_t>(0xF7);
  EXPECT_THROW(JavaModifiedUtf8::fromString(too_short_4byte),
               IllegalArgumentException);

  too_short_4byte += static_cast<int8_t>(0x1);
  EXPECT_THROW(JavaModifiedUtf8::fromString(too_short_4byte),
               IllegalArgumentException);

  too_short_4byte += static_cast<int8_t>(0x1);
  EXPECT_THROW(JavaModifiedUtf8::fromString(too_short_4byte),
               IllegalArgumentException);

  std::string bad_4byte_at_end = "foo";
  bad_4byte_at_end += static_cast<int8_t>(0xF7);
  EXPECT_THROW(JavaModifiedUtf8::fromString(bad_4byte_at_end),
               IllegalArgumentException);

  bad_4byte_at_end += static_cast<int8_t>(0x1);
  EXPECT_THROW(JavaModifiedUtf8::fromString(bad_4byte_at_end),
               IllegalArgumentException);

  bad_4byte_at_end += static_cast<int8_t>(0x1);
  EXPECT_THROW(JavaModifiedUtf8::fromString(bad_4byte_at_end),
               IllegalArgumentException);

  for (auto sequence : impossible_bytes) {
    std::string bad_sequence;
    for (auto byte_value : sequence) {
      bad_sequence += static_cast<int8_t>(byte_value);
    }
    EXPECT_THROW(JavaModifiedUtf8::fromString(bad_sequence),
                 IllegalArgumentException);
  }

  for (auto sequence : overlong_ascii_sequences) {
    std::string bad_sequence;
    for (auto byte_value : sequence) {
      bad_sequence += static_cast<int8_t>(byte_value);
    }
    EXPECT_THROW(JavaModifiedUtf8::fromString(bad_sequence),
                 IllegalArgumentException);
  }

  for (auto sequence : maximum_overlong_sequences) {
    std::string bad_sequence;
    for (auto byte_value : sequence) {
      bad_sequence += static_cast<int8_t>(byte_value);
    }
    EXPECT_THROW(JavaModifiedUtf8::fromString(bad_sequence),
                 IllegalArgumentException);
  }

  for (auto sequence : overlong_nulls) {
    std::string bad_sequence;
    for (auto byte_value : sequence) {
      bad_sequence += static_cast<int8_t>(byte_value);
    }
    EXPECT_THROW(JavaModifiedUtf8::fromString(bad_sequence),
                 IllegalArgumentException);
  }

  for (auto sequence : single_utf_16_surrogates) {
    std::string bad_sequence;
    for (auto byte_value : sequence) {
      bad_sequence += static_cast<int8_t>(byte_value);
    }
    EXPECT_THROW(JavaModifiedUtf8::fromString(bad_sequence),
                 IllegalArgumentException);
  }
}

std::pair<std::vector<int>, std::vector<int>> lowest_boundary_sequences[] = {
    {{0x00}, {0xC0, 0x80}},
    {{0xD0, 0x80}, {0xD0, 0x80}},
    {{0xE0, 0xA0, 0x80}, {0xE0, 0xA0, 0x80}},
    {{0xF0, 0x90, 0x80, 0x80}, {0xED, 0xA0, 0x80, 0xED, 0xB0, 0x80}}};

std::pair<std::vector<int>, std::vector<int>> highest_boundary_sequences[] = {
    {{0x7F}, {0x7F}},
    {{0xDF, 0xCF}, {0xDF, 0xCF}},
    {{0xEF, 0xBF, 0xBF}, {0xEF, 0xBF, 0xBF}},
    {{0xF7, 0xBF, 0xBF, 0xBF}, {0xED, 0xAE, 0xBF, 0xED, 0xBF, 0xBF}},
};

std::pair<std::vector<int>, std::vector<int>> other_boundary_sequences[] = {
    {{0xED, 0x9F, 0xBF}, {0xED, 0x9F, 0xBF}},
    {{0xEE, 0x80, 0x80}, {0xEE, 0x80, 0x80}},
    {{0xEF, 0xBF, 0xBD}, {0xEF, 0xBF, 0xBD}},
};

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

TEST_F(CacheableStringTests, TestUtf8ToJmUtf8Boundaries) {
  std::string utf8;
  std::string expected;
  utf8.push_back(0);

  auto jmutf8 = JavaModifiedUtf8::fromString(utf8);

  for (decltype(ARRAY_SIZE(lowest_boundary_sequences)) i = 0;
       i < ARRAY_SIZE(lowest_boundary_sequences); i++) {
    utf8.clear();
    expected.clear();
    for (auto byte_value : std::get<0>(lowest_boundary_sequences[i])) {
      utf8 += static_cast<int8_t>(byte_value);
    }
    for (auto byte_value : std::get<1>(lowest_boundary_sequences[i])) {
      expected += static_cast<int8_t>(byte_value);
    }
    jmutf8 = JavaModifiedUtf8::fromString(utf8);
    EXPECT_EQ(expected.size(), jmutf8.size());
    EXPECT_TRUE(!memcmp(&expected[0], &jmutf8[0], expected.size()));
  }

  for (decltype(ARRAY_SIZE(lowest_boundary_sequences)) i = 0;
       i < ARRAY_SIZE(highest_boundary_sequences); i++) {
    utf8.clear();
    expected.clear();
    for (auto byte_value : std::get<0>(highest_boundary_sequences[i])) {
      utf8 += static_cast<int8_t>(byte_value);
    }
    for (auto byte_value : std::get<1>(highest_boundary_sequences[i])) {
      expected += static_cast<int8_t>(byte_value);
    }
    jmutf8 = JavaModifiedUtf8::fromString(utf8);
    EXPECT_EQ(expected.size(), jmutf8.size());
    EXPECT_TRUE(!memcmp(&expected[0], &jmutf8[0], expected.size()));
  }

  for (decltype(ARRAY_SIZE(lowest_boundary_sequences)) i = 0;
       i < ARRAY_SIZE(other_boundary_sequences); i++) {
    utf8.clear();
    expected.clear();
    for (auto byte_value : std::get<0>(other_boundary_sequences[i])) {
      utf8 += static_cast<int8_t>(byte_value);
    }
    for (auto byte_value : std::get<1>(other_boundary_sequences[i])) {
      expected += static_cast<int8_t>(byte_value);
    }
    jmutf8 = JavaModifiedUtf8::fromString(utf8);
    EXPECT_EQ(expected.size(), jmutf8.size());
    EXPECT_TRUE(!memcmp(&expected[0], &jmutf8[0], expected.size()));
  }
}

}  // namespace
