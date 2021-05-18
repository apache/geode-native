#pragma once

#ifndef GEODE_FWKLIB_FWKEXCEPTION_H_
#define GEODE_FWKLIB_FWKEXCEPTION_H_

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

/**
 * @file    FwkException.hpp
 * @since   1.0
 * @version 1.0
 * @see
 *
 */

// ----------------------------------------------------------------------------

#include <string>

// ----------------------------------------------------------------------------

#define FWKEXCEPTION(x)                                              \
  do {                                                               \
    std::ostringstream os;                                           \
    os << x << " In file: " << __FILE__ << " at line: " << __LINE__; \
    throw apache::geode::client::testframework::FwkException(        \
        os.str().c_str());                                           \
  } while (0)

namespace apache {
namespace geode {
namespace client {
namespace testframework {
// ----------------------------------------------------------------------------

/** @class FwkException
 * @brief Framework exception handler
 */
class FwkException {
 public:
  /** @brief exception message to handle */
  inline explicit FwkException(const std::string& sMessage)
      : m_sMessage(sMessage) {}

  /** @brief exception message to handle */
  inline explicit FwkException(const char* pszMessage)
      : m_sMessage(pszMessage) {}

  /** @brief get message */
  inline const char* getMessage() const { return m_sMessage.c_str(); }

  /** @brief get message */
  inline const char* what() const { return m_sMessage.c_str(); }

 private:
  std::string m_sMessage;
};

// ----------------------------------------------------------------------------

}  // namespace  testframework
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_FWKLIB_FWKEXCEPTION_H_
