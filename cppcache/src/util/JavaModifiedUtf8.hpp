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

#include <string>

namespace apache {
namespace geode {
namespace client {
namespace internal {

struct ju8type_traits : std::char_traits<char> {};
typedef std::basic_string<char, ju8type_traits> ju8string;

struct JavaModifiedUtf8 {
  /**
   * Calculate the length of the given UTF-8 string when encoded in Java
   * Modified UTF-8.
   */
  static size_t encodedLength(const std::string& utf8);

  /**
   * Calculate the length of the given UTF-16 string when encoded in Java
   * Modified UTF-8.
   */
  static size_t encodedLength(const std::u16string& utf16);

  static size_t encodedLength(const char16_t* data, size_t length);

  /**
   * Converts given UTF-8 string to Java Modified UTF-8 string.
   */
  static ju8string fromString(const std::string& utf8);
  /**
   * Converts given UTF-16 string to Java Modified UTF-8 string.
   */
  static ju8string fromString(const std::u16string& utf16);

  /**
   * Converts a single UTF-16 code unit into Java Modified UTF-8 code units.
   */
  static void encode(const char16_t c, ju8string& jmutf8);

  static std::u16string decode(const char* buf, uint16_t len);

  static std::u32string decodeU32(const char* buf, uint16_t len);

  static char16_t decodeJavaModifiedUtf8Char(const char** pbuf);
};

}  // namespace internal
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_UTIL_JAVAMODIFIEDUTF8_H_
