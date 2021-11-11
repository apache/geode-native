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

#include "Serializable.hpp"
#include <geode/CacheableString.hpp>

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

using Apache::Geode::Client::Serializable;

[TestClass]
public ref class SerializableTests
{
public: 

  [TestMethod]
  void GetCacheableStringHandlesNonAsciiCharacters()
  {
    auto cacheableString = Serializable::GetCacheableString(gcnew String(L"foo\u2019s\U00010003"));
    Assert::IsFalse(cacheableString.get() == nullptr);
    Assert::IsTrue(std::string("foo\xE2\x80\x99s\xf0\x90\x80\x83") == cacheableString->value());
  }

    [TestMethod]
  void GetStringHandlesNonAsciiCharacters()
  {
    auto cacheableString = apache::geode::client::CacheableString::create("foo\xE2\x80\x99s\xf0\x90\x80\x83");
    auto str = Serializable::getString(cacheableString);
    Assert::AreEqual(gcnew String(L"foo\u2019s\U00010003"), str);
  }

};
