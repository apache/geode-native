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

#include <cstdlib>

#include <ace/OS.h>
#include <ace/TSS_T.h>
#include <boost/core/demangle.hpp>

#include <geode/CacheableString.hpp>
#include <geode/Exception.hpp>

#include "StackTrace.hpp"

namespace apache {
namespace geode {
namespace client {

Exception::Exception(const std::string& message)
    : message_(message), stack_(std::make_shared<StackTrace>()) {}

Exception::Exception(std::string&& message)
    : message_(std::move(message)), stack_(std::make_shared<StackTrace>()) {}

Exception::Exception(const char* message)
    : message_(message), stack_(std::make_shared<StackTrace>()) {}

const std::string& Exception::getMessage() const noexcept { return message_; }

const char* Exception::what() const noexcept { return message_.c_str(); }

Exception::~Exception() noexcept {}

std::string Exception::getName() const {
  return boost::core::demangle(typeid(*this).name());
}

std::string Exception::getStackTrace() const {
  return stack_ ? stack_->getString() : "No stack available.";
}

// class to store/clear last server exception in TSS area

class TSSExceptionString {
 private:
  std::string m_exMsg;

 public:
  TSSExceptionString() : m_exMsg() {}
  virtual ~TSSExceptionString() {}

  inline std::string& str() { return m_exMsg; }

  static ACE_TSS<TSSExceptionString> s_tssExceptionMsg;
};

ACE_TSS<TSSExceptionString> TSSExceptionString::s_tssExceptionMsg;

void setTSSExceptionMessage(const char* exMsg) {
  TSSExceptionString::s_tssExceptionMsg->str().clear();
  if (exMsg != nullptr) {
    TSSExceptionString::s_tssExceptionMsg->str().append(exMsg);
  }
}

const char* getTSSExceptionMessage() {
  return TSSExceptionString::s_tssExceptionMsg->str().c_str();
}
}  // namespace client
}  // namespace geode
}  // namespace apache
