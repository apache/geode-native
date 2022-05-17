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

#include <geode/internal/functional.hpp>

using apache::geode::client::internal::geode_hash;

TEST(string, geodeHash) {
  auto&& hash = geode_hash<std::string>{};

  EXPECT_EQ(0, hash(""));
  EXPECT_EQ(97, hash("a"));
  EXPECT_EQ(122, hash("z"));
  EXPECT_EQ(48, hash("0"));
  EXPECT_EQ(57, hash("9"));
  EXPECT_EQ(0, hash("G0&PI2<"));
  EXPECT_EQ(1077910243, hash("supercalifragilisticexpialidocious"));

  EXPECT_EQ(1544552287, hash(u8"You had me at meat tornad\u00F6!\U000F0000"));

  auto str = std::string("You had me at");
  str.push_back(0);
  str.append(u8"meat tornad\u00F6!\U000F0000");

  EXPECT_EQ(701776767, hash(str));
}
