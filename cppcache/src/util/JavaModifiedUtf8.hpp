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

#pragma once

#ifndef GEODE_UTIL_JAVAMODIFIEDUTF8_H_
#define GEODE_UTIL_JAVAMODIFIEDUTF8_H_

#include <codecvt>
#include <locale>
#include <string>

#include "string.hpp"

namespace apache {
namespace geode {
namespace client {
namespace internal {

using namespace apache::geode::client;

struct JavaModifiedUtf8 {
  /**
   * Calculate the length of the given UTF-8 string when encoded in Java
   * Modified UTF-8.
   */
  inline static size_t encodedLength(const std::string& utf8) {
    if (utf8.empty()) {
      return 0;
    }

    // TODO string optimize for direct calculation
    return encodedLength(to_utf16(utf8));
  }

  /**
   * Calculate the length of the given UTF-16 string when encoded in Java
   * Modified UTF-8.
   */
  inline static size_t encodedLength(const std::u16string& utf16) {
    return encodedLength(utf16.data(), utf16.length());
  }

  inline static size_t encodedLength(const char16_t* data, size_t length) {
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

  /**
   * Converts given UTF-8 string to Java Modified UTF-8 string.
   */
  inline static std::string fromString(const std::string& utf8) {
    return fromString(to_utf16(utf8));
  }

  /**
   * Converts given UTF-16 string to Java Modified UTF-8 string.
   */
  inline static std::string fromString(const std::u16string& utf16) {
    std::string jmutf8;
    jmutf8.reserve(utf16.length());

    for (auto&& c : utf16) {
      encode(c, jmutf8);
    }

    return jmutf8;
  }

  /**
   * Converts a single UTF-16 code unit into Java Modified UTF-8 code units.
   */
  inline static void encode(const char16_t c, std::string& jmutf8) {
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
};

}  // namespace internal
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_UTIL_JAVAMODIFIEDUTF8_H_
