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

inline std::string to_hex(const uint8_t* bytes, size_t len) {
  std::stringstream ss;
  ss << std::setfill('0') << std::hex;
  for (size_t i(0); i < len; ++i) {
    ss << std::setw(2) << static_cast<int>(bytes[i]);
  }
  return ss.str();
}

inline std::string to_hex(const DataOutput& out) {
  return to_hex(out.getBuffer(), out.getBufferLength());
}

inline std::string int_to_hex_string(const int value) {
  char hex[10];
  sprintf(hex, "%x", value);
  return std::string(hex);
}

TEST(QueueConnectionRequestTest, testToData) {
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

  auto pid = int_to_hex_string(boost::this_process::get_id());
  auto expectedQueueConnectionRequest =
      "2a0000012631015c047f000001000000022a00046e616d65000000302e0000" + pid +
      "0d002a000664734e616d652a000772616e644e756d2d00000001ffffffff000000012a00"
      "067365727665720000000a00";

  EXPECT_EQ(expectedQueueConnectionRequest, to_hex(dataOutput));
}

}  // namespace client
}  // namespace geode
}  // namespace apache
