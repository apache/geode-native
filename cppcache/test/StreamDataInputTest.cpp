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

#include <gmock/gmock-actions.h>
#include <gmock/gmock-matchers.h>

#include <gtest/gtest.h>

#include "CacheImpl.hpp"
#include "Connector.hpp"
#include "GetAllServersResponse.hpp"
#include "ServerLocation.hpp"
#include "StreamDataInput.hpp"
#include "geode/DataOutput.hpp"
#include "mock/ConnectorMock.hpp"

namespace {

using apache::geode::client::CacheImpl;
using apache::geode::client::Connector;
using apache::geode::client::ConnectorMock;
using apache::geode::client::DataOutput;
using apache::geode::client::GetAllServersResponse;
using apache::geode::client::Serializable;
using apache::geode::client::ServerLocation;
using apache::geode::client::StreamDataInput;
using apache::geode::client::TimeoutException;

using ::testing::_;
using ::testing::DoAll;
using ::testing::Eq;
using ::testing::Return;
using ::testing::SetArrayArgument;
using ::testing::SizeIs;

constexpr size_t kReadBuffSize = 3000;
constexpr size_t kStreamBufferSize = 10000;

ACTION_P(WaitMs, milliseconds) {
  std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
  return 0;
}

TEST(StreamDataInputTest, ObjectSizeGreaterThanReadBufferSize) {
  std::unique_ptr<ConnectorMock> connector =
      std::unique_ptr<ConnectorMock>(new ConnectorMock());

  unsigned int numServers = 100;
  std::vector<std::shared_ptr<ServerLocation> > servers(numServers);

  for (unsigned int i = 0; i < numServers; i++) {
    servers[i] = std::make_shared<ServerLocation>(
        std::string("this.is.a.quite.long.hostname.and.the.reason.is.that.it."
                    "is.used.for.testing:") += std::to_string(2000 + i));
  }

  GetAllServersResponse getAllServersResponse(servers);

  auto cache =
      std::make_shared<CacheImpl>(nullptr, nullptr, false, false, nullptr);

  auto dataOutput = cache->createDataOutput();

  getAllServersResponse.toData(dataOutput);

  auto buffer = dataOutput.getBuffer();
  auto dataOutputBufferLength = dataOutput.getBufferLength();

  // Gossip header
  uint8_t streamBuffer[kStreamBufferSize];
  streamBuffer[0] = 1;
  streamBuffer[1] = 0xd6;
  memcpy(streamBuffer + 2, buffer, dataOutputBufferLength);

  auto streamBufferLength = dataOutputBufferLength + 2;

  auto timeout = std::chrono::milliseconds(1000);
  EXPECT_CALL(*connector, getRemoteEndpoint())
      .WillRepeatedly(Return("locator:9999"));

  EXPECT_CALL(*connector, receive_nothrowiftimeout(_, _, _))
      .WillOnce(
          DoAll(SetArrayArgument<0>(streamBuffer, streamBuffer + kReadBuffSize),
                Return(kReadBuffSize)))
      .WillOnce(DoAll(SetArrayArgument<0>(streamBuffer + kReadBuffSize,
                                          streamBuffer + 2 * kReadBuffSize),
                      Return(kReadBuffSize)))
      .WillOnce(DoAll(SetArrayArgument<0>(streamBuffer + 2 * kReadBuffSize,
                                          &streamBuffer[streamBufferLength]),
                      Return(streamBufferLength - (2 * kReadBuffSize))));

  StreamDataInput streamDataInput(timeout, std::move(connector), cache.get(),
                                  nullptr);

  auto object = streamDataInput.readObject();

  auto response = std::dynamic_pointer_cast<GetAllServersResponse>(object);

  ASSERT_THAT(response->getServers(), SizeIs(servers.size()));
  for (unsigned int i = 0; i < servers.size(); i++) {
    ASSERT_THAT(response->getServers()[i]->getEpString(),
                Eq(servers[i]->getEpString()));
  }
}

TEST(StreamDataInputTest, TimeoutWhenReading) {
  auto connector = std::unique_ptr<ConnectorMock>(new ConnectorMock());

  auto cache =
      std::make_shared<CacheImpl>(nullptr, nullptr, false, false, nullptr);

  EXPECT_CALL(*connector, getRemoteEndpoint())
      .WillRepeatedly(Return("locator:9999"));

  EXPECT_CALL(*connector, receive_nothrowiftimeout(_, _, _))
      .WillOnce(WaitMs(2));

  auto timeout = std::chrono::milliseconds(1);
  StreamDataInput streamDataInput(timeout, std::move(connector), cache.get(),
                                  nullptr);

  ASSERT_THROW(streamDataInput.readObject(), TimeoutException);
}

}  // namespace
