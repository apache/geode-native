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

#include "internal/JavaModifiedUtf8.hpp"

#include <codecvt>
#include <locale>

#include "internal/string.hpp"

namespace apache {
namespace geode {
namespace client {
namespace internal {
size_t JavaModifiedUtf8::encodedLength(const std::string& utf8) {
  if (utf8.empty()) {
    return 0;
  }

  return encodedLength(to_utf16(utf8));
}

size_t JavaModifiedUtf8::encodedLength(const std::u16string& utf16) {
  return encodedLength(utf16.data(), utf16.length());
}

size_t JavaModifiedUtf8::encodedLength(const char16_t* data, size_t length) {
  size_t encodedLen = 0;
  while (length-- > 0) {
    const char16_t c = *(data++);
    if (c == 0) {
      // NUL
      encodedLen += 2;
    } else if (c < 0x80) {
      // ASCII
      encodedLen++;
    } else if (c < 0x800) {
      encodedLen += 2;
    } else {
      encodedLen += 3;
    }
  }
  return encodedLen;
}

std::string JavaModifiedUtf8::fromString(const std::string& utf8) {
  return fromString(to_utf16(utf8));
}

std::string JavaModifiedUtf8::fromString(const std::u16string& utf16) {
  std::string jmutf8;
  jmutf8.reserve(utf16.length());

  for (auto&& c : utf16) {
    encode(c, jmutf8);
  }

  return jmutf8;
}

void JavaModifiedUtf8::encode(const char16_t c, std::string& jmutf8) {
  if (c == 0) {
    // NUL
    jmutf8 += static_cast<uint8_t>(0xc0);
    jmutf8 += static_cast<uint8_t>(0x80);
  } else if (c < 0x80) {
    // ASCII character
    jmutf8 += static_cast<uint8_t>(c);
  } else if (c < 0x800) {
    jmutf8 += static_cast<uint8_t>(0xC0 | c >> 6);
    jmutf8 += static_cast<uint8_t>(0x80 | (c & 0x3F));
  } else {
    jmutf8 += static_cast<uint8_t>(0xE0 | c >> 12);
    jmutf8 += static_cast<uint8_t>(0x80 | ((c >> 6) & 0x3F));
    jmutf8 += static_cast<uint8_t>(0x80 | (c & 0x3F));
  }
}

std::u16string JavaModifiedUtf8::decode(const char* buf, uint16_t len) {
  std::u16string value;
  const auto end = buf + len;
  while (buf < end) {
    value += decodeJavaModifiedUtf8Char(&buf);
  }
  return value;
}

char16_t JavaModifiedUtf8::decodeJavaModifiedUtf8Char(const char** pbuf) {
  char16_t c;

  // get next byte unsigned
  int32_t b = **pbuf & 0xff;
  (*pbuf)++;
  int32_t k = b >> 5;
  // classify based on the high order 3 bits
  switch (k) {
    case 6: {
      // two byte encoding
      // 110yyyyy 10xxxxxx
      // use low order 6 bits
      int32_t y = b & 0x1f;
      // use low order 6 bits of the next byte
      // It should have high order bits 10, which we don't check.
      int32_t x = **pbuf & 0x3f;
      (*pbuf)++;
      // 00000yyy yyxxxxxx
      c = (y << 6 | x);
      break;
    }
    case 7: {
      // three byte encoding
      // 1110zzzz 10yyyyyy 10xxxxxx
      // use low order 4 bits
      int32_t z = b & 0x0f;
      // use low order 6 bits of the next byte
      // It should have high order bits 10, which we don't check.
      int32_t y = **pbuf & 0x3f;
      (*pbuf)++;
      // use low order 6 bits of the next byte
      // It should have high order bits 10, which we don't check.
      int32_t x = **pbuf & 0x3f;
      (*pbuf)++;
      // zzzzyyyy yyxxxxxx
      c = static_cast<char16_t>(z << 12 | y << 6 | x);
      break;
    }
    default:
      // one byte encoding
      // 0xxxxxxx
      // use just low order 7 bits
      // 00000000 0xxxxxxx
      c = static_cast<char16_t>(b & 0x7f);
      break;
  }
  return c;
}

}  // namespace internal
}  // namespace client
}  // namespace geode
}  // namespace apache
