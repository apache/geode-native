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

#ifndef GEODE_UTIL_STRING_H_
#define GEODE_UTIL_STRING_H_

#include <codecvt>
#include <locale>
#include <string>

namespace apache {
namespace geode {
namespace client {

inline std::u16string to_utf16(const std::string& utf8) {
// TODO string replace with boost
#if _MSC_VER >= 1900
  /*
   * Workaround for missing std:codecvt identifier.
   * https://connect.microsoft.com/VisualStudio/feedback/details/1403302
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

inline std::string to_utf8(const std::u16string& utf16) {
// TODO string replace with boost
#if _MSC_VER >= 1900
  /*
   * Workaround for missing std:codecvt identifier.
   * https://connect.microsoft.com/VisualStudio/feedback/details/1403302
   */
  auto data = reinterpret_cast<const int16_t*>(utf16.data());
  return std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t>{}
      .to_bytes(data, data + utf16.size());
#else
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}
      .to_bytes(utf16);
#endif
}

inline std::string to_utf8(const std::u32string& ucs4) {
// TODO string replace with boost
#if _MSC_VER >= 1900
  /*
   * Workaround for missing std:codecvt identifier.
   * https://connect.microsoft.com/VisualStudio/feedback/details/1403302
   */
  auto data = reinterpret_cast<const int32_t*>(ucs4.data());
  return std::wstring_convert<std::codecvt_utf8<int32_t>, int32_t>{}.to_bytes(
      data, data + ucs4.size());
#else
  return std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.to_bytes(
      ucs4);
#endif
}

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_UTIL_STRING_H_
