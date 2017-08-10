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

#include <gtest/gtest.h>

#include <ClientProxyMembershipIDFactory.hpp>

using namespace apache::geode::client;

TEST(ClientProxyMembershipIDFactoryTest, testCreate) {
  ClientProxyMembershipIDFactory factory("myDs");

  auto id = factory.create("myHost", 1, 2, "myClientID", 3);
  ASSERT_NE(nullptr, id);

  EXPECT_EQ("myDs", id->getDSName());
  EXPECT_EQ(1, static_cast<uint32_t>(*id->getHostAddr()));
  EXPECT_EQ(4, id->getHostAddrLen());
  EXPECT_EQ(2, id->getHostPort());

  auto uniqueTag = id->getUniqueTag();
  ASSERT_NE("", uniqueTag);
  EXPECT_EQ(std::string(":1:0:0:0:2:myDs:").append(uniqueTag),
            id->getHashKey());
  EXPECT_TRUE(std::regex_search(
      id->getDSMemberIdForThinClientUse(),
      std::regex(
          std::string("myHost(.*):2:").append(uniqueTag).append(":myDs"))));
}
