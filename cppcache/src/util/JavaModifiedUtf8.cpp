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

#include "JavaModifiedUtf8.hpp"

#include <codecvt>
#include <locale>
#include <set>

#include "geode/ExceptionTypes.hpp"
#include "string.hpp"

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

std::set<int> utf16_surrogate_codes = {{0xD800}, {0xDB7F}, {0xDB80}, {0xDBFF},
                                       {0xDC00}, {0xDF80}, {0xDFFF}};

bool JavaModifiedUtf8::IsValidCodePoint(uint16_t code_point) {
  return (code_point > 0x7FF) && (utf16_surrogate_codes.find(code_point) ==
                                  utf16_surrogate_codes.end());
}

ju8string JavaModifiedUtf8::fromString(const std::string& utf8) {
  ju8string jmutf8;
  size_t cursor = 0;

  while (cursor < utf8.size()) {
    auto byte1 = utf8[cursor++];
    if ((byte1 & 0x80) == 0) {
      if (byte1) {
        jmutf8 += byte1;
      } else {
        jmutf8 += static_cast<uint8_t>(0xC0);
        jmutf8 += static_cast<uint8_t>(0x80);
      }
    } else if ((byte1 & 0xE0) == 0xC0) {
      if (utf8.size() > 0 && cursor <= utf8.size() - 1) {
        auto byte2 = utf8[cursor++];
        int32_t code_point = ((byte1 & 0x1f) << 6) + (byte2 & 0x3f);
        if (code_point > 0x7F) {
          jmutf8 += byte1;
          jmutf8 += byte2;
        } else {
          throw IllegalArgumentException(
              "Invalid utf-8 string passed to conversion method (overly long "
              "ASCII character)");
        }
      } else {
        throw IllegalArgumentException(
            "Invalid utf-8 string passed to conversion method");
      }
    } else if ((byte1 & 0xF0) == 0xE0) {
      if (utf8.size() > 2 && cursor <= utf8.size() - 2) {
        auto byte2 = utf8[cursor++];
        auto byte3 = utf8[cursor++];

        uint16_t code_point =
            ((byte1 & 0x0F) << 12) + ((byte2 & 0x3F) << 6) + (byte3 & 0x3F);
        if (IsValidCodePoint(code_point)) {
          jmutf8 += byte1;
          jmutf8 += byte2;
          jmutf8 += byte3;
        } else {
          throw IllegalArgumentException(
              "Invalid utf-8 string passed to conversion method (overly long "
              "3-byte encoding)");
        }
      } else {
        throw IllegalArgumentException(
            "Invalid utf-8 string passed to conversion method");
      }
    } else if ((byte1 & 0xF8) == 0xF0) {
      if (utf8.size() > 3 && cursor <= utf8.size() - 3) {
        auto byte2 = utf8[cursor++];
        auto byte3 = utf8[cursor++];
        auto byte4 = utf8[cursor++];

        uint32_t code_point = (byte1 & 0x07) << 18;
        code_point += (byte2 & 0x3F) << 12;
        code_point += (byte3 & 0x3F) << 6;
        code_point += byte4 & 0x3F;

        if (code_point > 0xFFFF) {
          jmutf8 += static_cast<uint8_t>(0xED);
          jmutf8 +=
              static_cast<uint8_t>((0xA0 + (((code_point >> 16) - 1) & 0x0F)));
          jmutf8 += static_cast<uint8_t>((0x80 + ((code_point >> 10) & 0x3F)));

          jmutf8 += static_cast<uint8_t>(0xED);
          jmutf8 += static_cast<uint8_t>((0xB0 + ((code_point >> 6) & 0x0F)));
          jmutf8 += byte4;
        } else {
          throw IllegalArgumentException(
              "Invalid utf-8 string passed to conversion method (overly long "
              "4-byte encoding)");
        }
      } else {
        throw IllegalArgumentException(
            "Invalid utf-8 string passed to conversion method");
      }

    } else {
      throw IllegalArgumentException("Invalid utf-8 start code");
    }
  }
  return jmutf8;
}

ju8string JavaModifiedUtf8::fromString(const std::u16string& utf16) {
  ju8string jmutf8;
  jmutf8.reserve(utf16.length());

  for (auto&& c : utf16) {
    encode(c, jmutf8);
  }

  return jmutf8;
}

void JavaModifiedUtf8::encode(const char16_t c, ju8string& jmutf8) {
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

std::u32string JavaModifiedUtf8::decodeU32(const char* buf, uint16_t len) {
  std::u32string result;

  uint16_t i = 0;
  while (i < len) {
    auto byte1 = buf[i++];
    if (!(byte1 & 0x80)) {
      result += static_cast<int32_t>(byte1) & 0x000000FF;
    } else if ((i < len) && ((byte1 & 0xE0) == 0xC0)) {
      auto byte2 = buf[i++];
      if (((byte1 & 0xFF) == 0xC0) && ((byte2 & 0xFF) == 0x80)) {
        result.push_back(static_cast<int32_t>(0));
      } else {
        int32_t code_point = static_cast<int32_t>(byte1 & 0x1F) << 6;
        code_point += static_cast<int32_t>(byte2 & 0x3F);
        result.push_back(code_point);
      }
    } else if ((i < len - 4) && ((byte1 & 0xED) == 0xED)) {
      auto byte2 = buf[i++];
      auto byte3 = buf[i++];
      auto byte4 = buf[i++];
      auto byte5 = buf[i++];
      auto byte6 = buf[i++];
      if ((byte4 & 0xED) == 0xED) {
        int32_t code_point =
            0x10000 + (static_cast<int32_t>(byte2 & 0xF) << 16);
        code_point += static_cast<int32_t>(byte3 & 0x3F) << 10;
        code_point += static_cast<int32_t>(byte5 & 0xF) << 6;
        code_point += static_cast<int32_t>(byte6 & 0x3F);
        result.push_back(code_point);
      } else {
        throw IllegalArgumentException("Bad encoding in jmutf-8 string");
      }
    } else if ((i < len - 1) && ((byte1 & 0xE0) == 0xE0)) {
      auto byte2 = buf[i++];
      auto byte3 = buf[i++];
      int32_t code_point = static_cast<int32_t>(byte1 & 0xF) << 12;
      code_point += static_cast<int32_t>(byte2 & 0x3F) << 6;
      code_point += static_cast<int32_t>(byte3 & 0x3F);
      result.push_back(code_point);
    } else {
      throw IllegalArgumentException("Bad encoding in jmutf-8 string");
    }
  }

  return result;
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
