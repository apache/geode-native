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

static thread_local std::string threadLocalExceptionMessage;

void setThreadLocalExceptionMessage(std::string exMsg) {
  threadLocalExceptionMessage = std::move(exMsg);
}

const std::string& getThreadLocalExceptionMessage() {
  return threadLocalExceptionMessage;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
