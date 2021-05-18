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

#include <DataInputInternal.hpp>
#include <TcrMessage.hpp>

#include <gtest/gtest.h>

using apache::geode::client::DataInput;
using apache::geode::client::DataInputInternal;
using apache::geode::client::DSCode;
using apache::geode::client::MessageException;
using apache::geode::client::Region;
using apache::geode::client::Serializable;
using apache::geode::client::TcrMessage;
using apache::geode::client::TcrMessageHelper;
using apache::geode::client::ThinClientBaseDM;

namespace {
class TcrMessageTestFixture : public TcrMessage {
 public:
  TcrMessageTestFixture() : TcrMessage() {}
  ~TcrMessageTestFixture() noexcept override = default;
};
}  // namespace

TEST(TcrMessageHelperTest, readChunkPartHeaderExpectsAnObject) {
  TcrMessageTestFixture msg;

  uint8_t fakeBuffer[5] = {0, 0, 0, 1};

  auto input = DataInputInternal(fakeBuffer, sizeof(fakeBuffer));

  uint32_t partLength;

  EXPECT_THROW(TcrMessageHelper::readChunkPartHeader(
                   msg, input,
                   "TcrMessageHelperTest, readChunkPartHeaderExpectsAnObject",
                   partLength, 0),
               MessageException);
}

TEST(TcrMessageHelperTest, readChunkPartHeaderExceptionChunkHack) {
  TcrMessageTestFixture msg;

  uint8_t fakeBuffer[] = {
      0, 0, 0, 1, 1, static_cast<uint8_t>(DSCode::JavaSerializable),
      0, 0, 0, 0, 0};

  auto input = DataInputInternal(fakeBuffer, sizeof(fakeBuffer));

  uint32_t partLength;

  EXPECT_EQ(TcrMessageHelper::readChunkPartHeader(
                msg, input,
                "TcrMessageHelperTest, readChunkPartHeaderExceptionChunkHack",
                partLength, 64),
            TcrMessageHelper::ChunkObjectType::EXCEPTION);
}
