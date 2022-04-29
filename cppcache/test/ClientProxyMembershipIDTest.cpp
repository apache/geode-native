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

#include <vector>

#include <boost/endian/conversion.hpp>
#include <boost/regex.hpp>

#include <gtest/gtest.h>

#include "ClientProxyMembershipID.hpp"
#include "geode/internal/functional.hpp"

using apache::geode::client::ClientProxyMembershipID;

TEST(ClientProxyMembershipIDTest, testCreate) {
  uint32_t number = boost::endian::native_to_big(1);
  const uint8_t* array = reinterpret_cast<uint8_t*>(&number);

  ClientProxyMembershipID cpmID(array, 4, 2, "myDs", "uniqueTag", 0);

  std::vector<uint8_t> vector;
  vector.assign(array, array + 4);

  EXPECT_EQ("myDs", cpmID.getDSName());
  EXPECT_EQ(vector, cpmID.getHostAddr());
  EXPECT_EQ(static_cast<uint32_t>(4), cpmID.getHostAddrLen());
  EXPECT_EQ(static_cast<uint32_t>(2), cpmID.getHostPort());

  auto uniqueTag = cpmID.getUniqueTag();
  ASSERT_NE("", uniqueTag);
  EXPECT_EQ(std::string(":0:0:0:1:2:myDs:").append(uniqueTag),
            cpmID.getHashKey());
  EXPECT_EQ(
      cpmID.hashcode(),
      apache::geode::client::internal::geode_hash<std::string>{}(":0:0:0:1") +
          static_cast<int32_t>(cpmID.getHostPort()));
  EXPECT_TRUE(boost::regex_search(
      cpmID.getClientId(),
      boost::regex(
          std::string("localhost(.*):2:").append(uniqueTag).append(":myDs"))));
}
