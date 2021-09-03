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
#include <BucketServerLocation.hpp>
#include <ClientMetadata.hpp>
#include <ClientMetadataService.hpp>

#include <gtest/gtest.h>

#include "gmock/gmock.h"
#include "mock/ClientMetadataMock.hpp"

using ::testing::AtLeast;
using ::testing::Return;

namespace apache {
namespace geode {
namespace client {

TEST(ClientMetadataServiceTest, testWhenBucketLocationIsNotAvailable) {
  auto mock = std::make_shared<ClientMetadataMock>();

  ClientMetadataService::BucketSet bucketSet({1, 2, 3});

  std::vector<std::shared_ptr<BucketServerLocation>> emptyBucketList;

  EXPECT_CALL(*mock, adviseServerLocations(3))
      .WillOnce(Return(emptyBucketList));

  ASSERT_EQ(nullptr, ClientMetadataService::pruneNodes(mock, bucketSet));
}

TEST(ClientMetadataServiceTest, testBucketAvailable) {
  auto mock = std::make_shared<ClientMetadataMock>();

  ClientMetadataService::BucketSet bucketSet({0, 1});

  auto bucket =
      std::make_shared<BucketServerLocation>(0, 2, "server1", true, 1);
  auto bucket1 =
      std::make_shared<BucketServerLocation>(1, 23, "server2", true, 1);

  std::vector<std::shared_ptr<BucketServerLocation>>
      m_bucketServerLocationsList;
  m_bucketServerLocationsList.push_back(bucket);

  std::vector<std::shared_ptr<BucketServerLocation>>
      m_bucketServerLocationsList1;
  m_bucketServerLocationsList1.push_back(bucket1);

  EXPECT_CALL(*mock, adviseServerLocations)
      .WillOnce(Return(m_bucketServerLocationsList))
      .WillOnce(Return(m_bucketServerLocationsList1));

  ASSERT_EQ(2, ClientMetadataService::pruneNodes(mock, bucketSet)->size());
}
}  // namespace client
}  // namespace geode
}  // namespace apache