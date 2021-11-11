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

#ifndef GEODE_GTEST_EXTENSIONS_H_
#define GEODE_GTEST_EXTENSIONS_H_

#include <iostream>
#include <limits>
#include <string>

#include <boost/regex.hpp>

#include <gtest/gtest.h>

#include <geode/internal/chrono/duration.hpp>

namespace std {
namespace chrono {

/**
 * Exports gtest's PrintTo function for std::chrono::duration.
 */
template <typename Rep, typename Per>
void PrintTo(const ::std::chrono::duration<Rep, Per>& value,
             ::std::ostream* os) {
  *os << apache::geode::internal::chrono::duration::to_string(value);
}

}  // namespace chrono
}  // namespace std

namespace apache {
namespace geode {
namespace testing {

inline std::string squash(const std::string& str, size_t maxLength = 60) {
  if (str.length() <= maxLength) {
    return str;
  }

  return str.substr(0, maxLength / 2)
      .append("...")
      .append(str.substr(str.length() - maxLength / 2));
}

#define EXPECT_MATCH(r, s) \
  EXPECT_PRED_FORMAT2(::apache::geode::testing::regexMatch, r, s)
#define ASSERT_MATCH(r, s) \
  ASSERT_PRED_FORMAT2(::apache::geode::testing::regexMatch, r, s)

inline ::testing::AssertionResult regexMatch(const char* s1_expression,
                                             const char* /*s2_expression*/,
                                             const boost::regex& regex,
                                             const std::string& source) {
  if (!boost::regex_match(source, regex)) {
    return ::testing::AssertionFailure()
           << squash(source) << " !~ " << s1_expression;
  }

  return ::testing::AssertionSuccess();
}

inline ::testing::AssertionResult regexMatch(const char* /*s1_expression*/,
                                             const char* /*s2_expression*/,
                                             const std::string& regex,
                                             const std::string& source) {
  if (!boost::regex_match(source, boost::regex(regex))) {
    return ::testing::AssertionFailure()
           << squash(source) << " !~ /" << regex << "/";
  }

  return ::testing::AssertionSuccess();
}

}  // namespace testing
}  // namespace geode
}  // namespace apache

#endif  // GEODE_GTEST_EXTENSIONS_H_
