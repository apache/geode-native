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

#ifndef GEODE_EXCEPTION_H_
#define GEODE_EXCEPTION_H_

#include <stdexcept>
#include <string>
#include <unordered_map>

#include "internal/functional.hpp"
#include "internal/geode_globals.hpp"

namespace apache {
namespace geode {
namespace client {

class StackTrace;

#if defined(_MSC_VER)
// Ignore C4275 - This class extends std C++ class
#pragma warning(push)
#pragma warning(disable : 4275)
#endif

/**
 * A description of an exception that occurred during a cache operation.
 */
class APACHE_GEODE_EXPORT Exception : public std::exception {
 public:
  explicit Exception(const std::string& message);
  explicit Exception(std::string&& message);
  explicit Exception(const char* message);
  Exception(const Exception&) = default;
  Exception& operator=(const Exception&) = default;
  Exception(Exception&&) noexcept = default;
  Exception& operator=(Exception&&) = default;
  ~Exception() noexcept override;

  /**
   * Get a stacktrace string from the location the exception was created.
   */
  virtual std::string getStackTrace() const;

  /**
   * Return the name of this exception type.
   * */
  virtual std::string getName() const;

  /**
   * Get a message with details regarding this exception."
   */
  virtual const std::string& getMessage() const noexcept;

  const char* what() const noexcept override;

 private:
  std::string message_;
  std::shared_ptr<StackTrace> stack_;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_EXCEPTION_H_
