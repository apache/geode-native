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

#include <geode/RegionAttributesFactory.hpp>

using apache::geode::client::ExpirationAction;
using apache::geode::client::RegionAttributesFactory;

TEST(RegionAttributesFactoryTest, setEntryIdleTimeoutSeconds) {
  RegionAttributesFactory regionAttributesFactory;
  auto regionAttributes = regionAttributesFactory
                              .setEntryIdleTimeout(ExpirationAction::DESTROY,
                                                   std::chrono::seconds(10))
                              .create();
  EXPECT_EQ(std::chrono::seconds(10), regionAttributes.getEntryIdleTimeout());
}

TEST(RegionAttributesFactoryTest, setEntryTimeToLiveSeconds) {
  RegionAttributesFactory regionAttributesFactory;
  auto regionAttributes = regionAttributesFactory
                              .setEntryTimeToLive(ExpirationAction::DESTROY,
                                                  std::chrono::seconds(10))
                              .create();
  EXPECT_EQ(std::chrono::seconds(10), regionAttributes.getEntryTimeToLive());
}

TEST(RegionAttributesFactoryTest, setRegionIdleTimeoutSeconds) {
  RegionAttributesFactory regionAttributesFactory;
  auto regionAttributes = regionAttributesFactory
                              .setRegionIdleTimeout(ExpirationAction::DESTROY,
                                                    std::chrono::seconds(10))
                              .create();
  EXPECT_EQ(std::chrono::seconds(10), regionAttributes.getRegionIdleTimeout());
}

TEST(RegionAttributesFactoryTest, setRegionTimeToLiveSeconds) {
  RegionAttributesFactory regionAttributesFactory;
  auto regionAttributes = regionAttributesFactory
                              .setRegionTimeToLive(ExpirationAction::DESTROY,
                                                   std::chrono::seconds(10))
                              .create();
  EXPECT_EQ(std::chrono::seconds(10), regionAttributes.getRegionTimeToLive());
}

TEST(RegionAttributesFactoryTest, setInitialCapacity) {
  RegionAttributesFactory* regionAttributesFactory =
      new RegionAttributesFactory();
  EXPECT_NE(regionAttributesFactory, nullptr);
  auto regionAttributes = regionAttributesFactory->setLruEntriesLimit(2)
                              .setInitialCapacity(5)
                              .create();
  EXPECT_EQ(regionAttributes.getInitialCapacity(), 5);
}

TEST(RegionAttributesFactoryTest, setLruEntriesLimit) {
  RegionAttributesFactory* regionAttributesFactory =
      new RegionAttributesFactory();
  EXPECT_NE(regionAttributesFactory, nullptr);
  auto regionAttributes = regionAttributesFactory->setLruEntriesLimit(2)
                              .setInitialCapacity(5)
                              .create();
  EXPECT_EQ(regionAttributes.getLruEntriesLimit(), 2u);
}
