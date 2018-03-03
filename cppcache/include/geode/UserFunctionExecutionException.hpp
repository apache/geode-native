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

#ifndef GEODE_USERFUNCTIONEXECUTIONEXCEPTION_H_
#define GEODE_USERFUNCTIONEXECUTIONEXCEPTION_H_

#include <memory>
#include <string>

#include "CacheableString.hpp"
#include "Serializable.hpp"

namespace apache {
namespace geode {
namespace client {

class UserFunctionExecutionException;
class DataInput;
class DataOutput;

/**
 * @brief UserFunctionExecutionException class is used to encapsulate geode
 * sendException in case of Function execution.
 */
class APACHE_GEODE_EXPORT UserFunctionExecutionException : public Serializable {
 public:
  explicit UserFunctionExecutionException(std::string message)
      : m_message(std::move(message)) {}
  UserFunctionExecutionException(const UserFunctionExecutionException& other) =
      delete;
  void operator=(const UserFunctionExecutionException& other) = delete;

  ~UserFunctionExecutionException() override = default;

  /**
   * @brief return as std::string the Exception message returned from geode
   * sendException api.
   */
  const std::string& getMessage() const { return m_message; }

  std::string toString() const override { return this->getMessage(); }

  /**
   * @brief return as std::string the Exception name returned from geode
   * sendException api.
   */
  const std::string& getName() const {
    static const std::string name = "UserFunctionExecutionException";
    return name;
  }

 private:
  std::string m_message;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_USERFUNCTIONEXECUTIONEXCEPTION_H_
