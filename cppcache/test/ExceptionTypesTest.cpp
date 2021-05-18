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

#include <gtest/gtest.h>

#include <geode/ExceptionTypes.hpp>

#include "util/exception.hpp"

using apache::geode::client::AssertionException;
using apache::geode::client::GfErrTypeThrowException;
using apache::geode::client::LowMemoryException;
using apache::geode::client::QueryExecutionLowMemoryException;

TEST(ExceptionTypesTest, getName) {
  AssertionException e("an exception message");
  EXPECT_EQ("apache::geode::client::AssertionException",
            std::string(e.getName()));
}

TEST(ExceptionTypesTest, getStackTrace) {
  AssertionException e("an exception message");
  auto s = e.getStackTrace();
  EXPECT_TRUE(!s.empty());
}

TEST(ExceptionTypesTest, lowMemory) {
  EXPECT_THROW(GfErrTypeThrowException("", GF_LOW_MEMORY_EXCEPTION),
               LowMemoryException);
}

TEST(ExceptionTypesTest, queryLowMemory) {
  EXPECT_THROW(
      GfErrTypeThrowException("", GF_QUERY_EXECUTION_LOW_MEMORY_EXCEPTION),
      QueryExecutionLowMemoryException);
}
