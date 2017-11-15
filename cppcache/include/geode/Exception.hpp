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

#include <string>
#include <unordered_map>
#include "geode_globals.hpp"
#include "util/functional.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

#define GF_EX_MSG_LIMIT 2048

class DistributedSystem;
class CacheableString;
class StackTrace;
/**
 * @class Exception Exception.hpp
 * A description of an exception that occurred during a cache operation.
 */
class CPPCACHE_EXPORT Exception {
  /**
   * @brief public methods
   */
 public:
  /** Creates an exception.
   * @param  msg1 message pointer, this is copied into the exception.
   * @param  msg2 optional extra message pointer, appended to msg1.
   * @param  forceTrace enables a stacktrace for this exception regardless of
   * stacktrace-enabled system property.
   * @param  cause optional cause of the exception which can be later
   *               retrieved using <code>getCause</code>
   **/
  Exception(const char* msg1, const char* msg2 = nullptr,
            bool forceTrace = false, const std::shared_ptr<Exception>& cause = nullptr);

  explicit Exception(const std::string& msg1);

  /** Creates an exception as a copy of the given other exception.
   * @param  other the original exception.
   *
   **/
  Exception(const Exception& other) noexcept = default;
  Exception(Exception&& other) noexcept = default;

  /** Create a clone of this exception. */
  virtual Exception* clone() const;

  /**
   * @brief destructor
   */
  virtual ~Exception();

  /** Returns the message pointer
   *
   * @return  message pointer
   */
  virtual const char* getMessage() const;
  /** Show the message pointer
   *
   */
  virtual void showMessage() const;

  /** On some platforms, print a stacktrace from the location the exception
   * was created.
   */
  virtual void printStackTrace() const;

#ifndef _SOLARIS
  /** On some platforms, get a stacktrace string from the location the
   * exception was created.
   */
  virtual size_t getStackTrace(char* buffer, size_t maxLength) const;
#endif

  /** Return the name of this exception type. */
  virtual const char* getName() const;

  /**
   * Throw polymorphically; this allows storing an exception object
   * pointer and throwing it later.
   */
  virtual void raise() { throw *this; }

  inline std::shared_ptr<Exception> getCause() const { return m_cause; }

 protected:
  /** internal constructor used to clone this exception */
  Exception(const std::shared_ptr<CacheableString>& message,
            const std::shared_ptr<StackTrace>& stack,
            const std::shared_ptr<Exception>& cause);

  static bool s_exceptionStackTraceEnabled;

  std::shared_ptr<CacheableString> m_message;  // error message
  std::shared_ptr<StackTrace> m_stack;
  std::shared_ptr<Exception> m_cause;

 private:
  static void setStackTraces(bool stackTraceEnabled);

  friend class DistributedSystem;
};

class CacheableKey;
typedef std::unordered_map<std::shared_ptr<CacheableKey>,
                           std::shared_ptr<Exception>,
                           dereference_hash<std::shared_ptr<CacheableKey>>,
dereference_equal_to<std::shared_ptr<CacheableKey>>>
HashMapOfException;


}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_EXCEPTION_H_
