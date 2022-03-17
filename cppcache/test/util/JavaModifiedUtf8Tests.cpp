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

#include <internal/JavaModifiedUtf8.hpp>
#include <string>

#include <gtest/gtest.h>

#include "../ByteArray.hpp"

using apache::geode::client::ByteArray;
using apache::geode::client::internal::JavaModifiedUtf8;

TEST(JavaModifiedUtf8Tests, EncodedLengthFromUtf8) {
  EXPECT_EQ(27, JavaModifiedUtf8::encodedLength("You had me at meat tornado!"));
}

TEST(JavaModifiedUtf8Tests, EncodedLengthUtf16) {
  EXPECT_EQ(27,
            JavaModifiedUtf8::encodedLength(u"You had me at meat tornado."));
}

TEST(JavaModifiedUtf8Tests, DecodeToString) {
  const char* buf = "You had me at meat tornado!";
  EXPECT_EQ(JavaModifiedUtf8::decode(buf, 27),
            std::u16string(u"You had me at meat tornado!"));
}

TEST(JavaModifiedUtf8Tests, DecodeStringWithInlineNullChar) {
  auto expected = std::u16string(u"You had me at");
  expected.push_back(0);
  expected.append(u"meat tornad\u00F6!\U000F0000");

  auto buf = ByteArray::fromString(
      "596F7520686164206D65206174C0806D65617420746F726E6164C3B621EDAE80EDB080");
  std::u16string actual =
      JavaModifiedUtf8::decode(reinterpret_cast<const char*>(buf.get()), 35);
  EXPECT_EQ(expected, actual);
}
