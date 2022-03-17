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

#include <cctype>
#include <codecvt>
#include <locale>
#include <string>

#include "internal/type_traits.hpp"

namespace apache {
namespace geode {
namespace client {

std::u16string to_utf16(const std::string& utf8);

std::u16string to_utf16(const std::u32string& ucs4);

std::u16string to_utf16(const char32_t* ucs4, size_t len);

std::u32string to_ucs4(const std::u16string& utf16);

std::string to_utf8(const std::u16string& utf16);

std::string to_utf8(const std::u32string& ucs4);

bool equal_ignore_case(const std::string& str1, const std::string& str2);

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_UTIL_STRING_H_
