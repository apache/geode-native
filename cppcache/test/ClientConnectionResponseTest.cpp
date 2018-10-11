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

#include <ClientConnectionResponse.hpp>
#include <DataInputInternal.hpp>
#include <DataOutputInternal.hpp>

#include <gtest/gtest.h>

using apache::geode::client::ClientConnectionResponse;
using apache::geode::client::DataInputInternal;
using apache::geode::client::DataOutputInternal;

TEST(ClientConnectionResponseTest, testDefaultServerFound) {
  ClientConnectionResponse clientConnectionResponse;
  ASSERT_FALSE(clientConnectionResponse.serverFound());
}

TEST(ClientConnectionResponseTest, testReadServerFound) {
  DataOutputInternal dataOutputInternal;

  dataOutputInternal.writeBoolean(false);
  dataOutputInternal.writeBoolean(true);

  // The following are necessary to meet subsequent calls
  dataOutputInternal.writeString("hello");
  dataOutputInternal.writeInt(static_cast<int64_t>(103334));

  // Put it into a datainput so it can be read.
  DataInputInternal dataInput(dataOutputInternal.getBuffer(),
                              dataOutputInternal.getBufferLength());

  ClientConnectionResponse clientConnectionResponse;
  clientConnectionResponse.fromData(dataInput);
  ASSERT_FALSE(clientConnectionResponse.serverFound());

  clientConnectionResponse.fromData(dataInput);
  ASSERT_TRUE(clientConnectionResponse.serverFound());
}
