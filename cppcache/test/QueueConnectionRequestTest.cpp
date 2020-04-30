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

#include <QueueConnectionRequest.hpp>

#include <boost/process/environment.hpp>

#include <gtest/gtest.h>

#include <geode/DataOutput.hpp>

#include "ByteArrayFixture.hpp"
#include "DataOutputInternal.hpp"

namespace apache {
namespace geode {
namespace client {

class QueueConnectionRequestTest : public ::testing::Test,
                                   public ByteArrayFixture {};

TEST_F(QueueConnectionRequestTest, testToData) {
  DataOutputInternal dataOutput;
  ACE_INET_Addr addr(10, "localhost");
  ServerLocation srv("server", 10);
  std::set<ServerLocation> servLoc;
  servLoc.insert(srv);
  std::string dsName = "dsName";
  std::string randNum = "randNum";

  ClientProxyMembershipID qCR(dsName, randNum, "name", addr, 10, "id-1",
                              std::chrono::seconds(0));

  QueueConnectionRequest queueConnReq(qCR, servLoc, -1, false);
  queueConnReq.toData(dataOutput);

  EXPECT_BYTEARRAY_EQ(
      "2A0000012631015C047F000001000000022A00046E616D65000000302E\\h{8}"
      "0D002A000664734E616D652A000772616E644E756D2D00000001FFFFFFFF000000012A00"
      "067365727665720000000A00",
      ByteArray(dataOutput.getBuffer(), dataOutput.getBufferLength()));
}

}  // namespace client
}  // namespace geode
}  // namespace apache
