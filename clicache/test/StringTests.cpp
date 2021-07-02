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

#include "String.hpp"

using namespace System;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;
using namespace Apache::Geode::Client;

[TestClass]
public ref class StringTests
{
public: 

  [TestMethod]
  void to_uft8FromStringHandlesNonAscii()
  {
    String^ str = L"foo\u2019s\U00010003";
    auto utf8 = to_utf8(str);
    Assert::IsTrue(std::string("foo\xE2\x80\x99s\xf0\x90\x80\x83") == utf8);
  }

  
  [TestMethod]
  void to_StringFromUtf8HandlesNonAscii()
  {
    std::string utf8("foo\xE2\x80\x99s\xf0\x90\x80\x83");
    auto str = to_String(utf8);
    Assert::AreEqual(gcnew String(L"foo\u2019s\U00010003"), str);
  }
};
