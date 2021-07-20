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

#include <string>
#include <codecvt>

#include <msclr/marshal.h>
#include <msclr/marshal_cppstd.h>

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      inline std::string to_utf8(const std::wstring& utf16) {
        return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.to_bytes(utf16);
      }

      inline std::string to_utf8(String^ utf16) {
        return to_utf8(msclr::interop::marshal_as<std::wstring>(utf16));
      }

      inline std::wstring to_wstring(const std::string& utf8) {
        return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes(utf8);
      }

      inline String^ to_String(const std::string& utf8) {
        return msclr::interop::marshal_as<String^>(to_wstring(utf8));
      }

    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

