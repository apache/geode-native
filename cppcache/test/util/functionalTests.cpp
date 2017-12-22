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

#include <string>

#include <gtest/gtest.h>

#include <geode/util/functional.hpp>

using namespace apache::geode::client;

TEST(string, geode_hash) {
  auto&& hash = geode_hash<std::string>{};

  EXPECT_EQ(0, hash(""));
  EXPECT_EQ(97, hash("a"));
  EXPECT_EQ(122, hash("z"));
  EXPECT_EQ(48, hash("0"));
  EXPECT_EQ(57, hash("9"));
  EXPECT_EQ(1077910243, hash("supercalifragilisticexpialidocious"));
}
