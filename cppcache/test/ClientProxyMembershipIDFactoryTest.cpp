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

#include <regex>

#include <boost/endian/conversion.hpp>
#include <vector>

#include <gtest/gtest.h>

#include "ClientProxyMembershipIDFactory.hpp"

using apache::geode::client::ClientProxyMembershipIDFactory;

TEST(ClientProxyMembershipIDFactoryTest, testCreate) {
  ClientProxyMembershipIDFactory factory("myDs");

  uint32_t number = boost::endian::native_to_big(1);
  const uint8_t* array = reinterpret_cast<uint8_t*>(&number);

  auto id = factory.create("myHost", array, 4, 2, "myClientID",
                           std::chrono::seconds(3));
  ASSERT_NE(nullptr, id);

  std::vector<uint8_t> vector;
  vector.assign(array, array + 4);

  EXPECT_EQ("myDs", id->getDSName());
  EXPECT_EQ(vector, id->getHostAddr());
  EXPECT_EQ(static_cast<uint32_t>(4), id->getHostAddrLen());
  EXPECT_EQ(static_cast<uint32_t>(2), id->getHostPort());

  auto uniqueTag = id->getUniqueTag();
  ASSERT_NE("", uniqueTag);
  EXPECT_EQ(std::string(":0:0:0:1:2:myDs:").append(uniqueTag),
            id->getHashKey());
  EXPECT_TRUE(std::regex_search(
      id->getDSMemberIdForThinClientUse(),
      std::regex(
          std::string("myHost(.*):2:").append(uniqueTag).append(":myDs"))));
}
