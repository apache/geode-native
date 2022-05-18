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

#include <boost/asio.hpp>
#include <boost/process/environment.hpp>

#include <gtest/gtest.h>

#include <geode/DataOutput.hpp>

#include "ByteArrayFixture.hpp"
#include "ClientProxyMembershipID.hpp"
#include "DataOutputInternal.hpp"

namespace apache {
namespace geode {
namespace client {

class QueueConnectionRequestTest : public ::testing::Test,
                                   public ByteArrayFixture {};

TEST_F(QueueConnectionRequestTest, testToData) {
  namespace bip = boost::asio::ip;

  DataOutputInternal dataOutput;
  auto address = bip::make_address("127.0.0.1");
  ServerLocation srv("server", 10);
  std::set<ServerLocation> servLoc;
  servLoc.insert(srv);
  const std::string dsName = "dsName";
  const std::string randString = "randNum";
  const std::string hostname = "name";
  const std::string durableClientId = "id-1";

  const ClientProxyMembershipID qCR(dsName, randString, hostname, address, 10,
                                    durableClientId);

  QueueConnectionRequest queueConnReq(qCR, servLoc, -1, false);
  queueConnReq.toData(dataOutput);

  EXPECT_BYTEARRAY_EQ(
      "570000012631015C047F000001000000025700046E616D65000000302E\\h{8}"
      "0D0057000664734E616D6557000772616E644E756D7D00000001FFFFFFFF000000015700"
      "067365727665720000000A00",
      ByteArray(dataOutput.getBuffer(), dataOutput.getBufferLength()));
}

}  // namespace client
}  // namespace geode
}  // namespace apache
