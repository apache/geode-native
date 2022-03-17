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

#include "internal/string.hpp"

#include <cctype>
#include <codecvt>
#include <locale>

#include "internal/type_traits.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * Native codecvt_mode endianess
 */
constexpr std::codecvt_mode codecvt_mode_native_endian =
    endian::native == endian::little ? std::little_endian
                                     : static_cast<std::codecvt_mode>(0);

std::u16string to_utf16(const std::string& utf8) {
#if defined(_MSC_VER) && _MSC_VER >= 1900
  /*
   * Workaround for missing std:codecvt identifier.
   * https://stackoverflow.com/questions/32055357/visual-studio-c-2015-stdcodecvt-with-char16-t-or-char32-t
   */
  auto int16String =
      std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t>{}
          .from_bytes(utf8);
  return std::u16string(reinterpret_cast<const char16_t*>(int16String.data()),
                        int16String.size());
#else
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}
      .from_bytes(utf8);
#endif
}

std::u16string to_utf16(const std::u32string& ucs4) {
#if defined(_MSC_VER) && _MSC_VER >= 1900
  /*
   * Workaround for missing std:codecvt identifier.
   * https://stackoverflow.com/questions/32055357/visual-studio-c-2015-stdcodecvt-with-char16-t-or-char32-t
   */
  auto data = reinterpret_cast<const int32_t*>(ucs4.data());
  auto bytes =
      std::wstring_convert<
          std::codecvt_utf16<int32_t, 0x10ffff, codecvt_mode_native_endian>,
          int32_t>{}
          .to_bytes(data, data + ucs4.size());
#else
  auto bytes =
      std::wstring_convert<
          std::codecvt_utf16<char32_t, 0x10ffff, codecvt_mode_native_endian>,
          char32_t>{}
          .to_bytes(ucs4);
#endif

  return std::u16string(reinterpret_cast<const char16_t*>(bytes.c_str()),
                        bytes.length() / sizeof(char16_t));
}

std::u16string to_utf16(const char32_t* ucs4, size_t len) {
#if defined(_MSC_VER) && _MSC_VER >= 1900
  /*
   * Workaround for missing std:codecvt identifier.
   * https://stackoverflow.com/questions/32055357/visual-studio-c-2015-stdcodecvt-with-char16-t-or-char32-t
   */
  auto data = reinterpret_cast<const int32_t*>(ucs4);
  auto bytes =
      std::wstring_convert<
          std::codecvt_utf16<int32_t, 0x10ffff, codecvt_mode_native_endian>,
          int32_t>{}
          .to_bytes(data, data + len);
#else
  auto bytes =
      std::wstring_convert<
          std::codecvt_utf16<char32_t, 0x10ffff, codecvt_mode_native_endian>,
          char32_t>{}
          .to_bytes(ucs4, ucs4 + len);
#endif

  return std::u16string(reinterpret_cast<const char16_t*>(bytes.c_str()),
                        bytes.length() / sizeof(char16_t));
}

std::u32string to_ucs4(const std::u16string& utf16) {
#if defined(_MSC_VER) && _MSC_VER >= 1900
  /*
   * Workaround for missing std:codecvt identifier.
   * https://stackoverflow.com/questions/32055357/visual-studio-c-2015-stdcodecvt-with-char16-t-or-char32-t
   */
  auto data = reinterpret_cast<const char*>(utf16.data());
  auto tmp =
      std::wstring_convert<
          std::codecvt_utf16<int32_t, 0x10ffff, codecvt_mode_native_endian>,
          int32_t>{}
          .from_bytes(data, data + (utf16.length() * sizeof(char16_t)));
  return std::u32string(reinterpret_cast<const char32_t*>(tmp.data()),
                        tmp.length());
#else
  auto data = reinterpret_cast<const char*>(utf16.data());
  return std::wstring_convert<
             std::codecvt_utf16<char32_t, 0x10ffff, codecvt_mode_native_endian>,
             char32_t>{}
      .from_bytes(data, data + (utf16.length() * sizeof(char16_t)));
#endif
}

std::string to_utf8(const std::u16string& utf16) {
#if defined(_MSC_VER) && _MSC_VER >= 1900
  /*
   * Workaround for missing std:codecvt identifier.
   * https://stackoverflow.com/questions/32055357/visual-studio-c-2015-stdcodecvt-with-char16-t-or-char32-t
   */
  auto data = reinterpret_cast<const int16_t*>(utf16.data());
  return std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t>{}
      .to_bytes(data, data + utf16.size());
#else
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}
      .to_bytes(utf16);
#endif
}

std::string to_utf8(const std::u32string& ucs4) {
#if defined(_MSC_VER) && _MSC_VER >= 1900
  /*
   * Workaround for missing std:codecvt identifier.
   * https://stackoverflow.com/questions/32055357/visual-studio-c-2015-stdcodecvt-with-char16-t-or-char32-t
   */
  auto data = reinterpret_cast<const int32_t*>(ucs4.data());
  return std::wstring_convert<std::codecvt_utf8<int32_t>, int32_t>{}.to_bytes(
      data, data + ucs4.size());
#else
  return std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.to_bytes(
      ucs4);
#endif
}

bool equal_ignore_case(const std::string& str1, const std::string& str2) {
  return ((str1.size() == str2.size()) &&
          std::equal(str1.begin(), str1.end(), str2.begin(),
                     [](const char& c1, const char& c2) {
                       return (c1 == c2 ||
                               std::toupper(c1) == std::toupper(c2));
                     }));
}

}  // namespace client
}  // namespace geode
}  // namespace apache
