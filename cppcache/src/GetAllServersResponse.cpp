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
#include "GetAllServersResponse.hpp"

namespace apache {
namespace geode {
namespace client {

void GetAllServersResponse::toData(DataOutput& output) const {
  int32_t numServers = static_cast<int32_t>(m_servers.size());
  output.writeInt(numServers);
  for (int32_t i = 0; i < numServers; i++) {
    output.writeObject(m_servers.at(i));
  }
}
void GetAllServersResponse::fromData(DataInput& input) {
  int numServers = input.readInt32();
  LOG_FINER("GetAllServersResponse::fromData length = %d ", numServers);
  for (int i = 0; i < numServers; i++) {
    std::shared_ptr<ServerLocation> sLoc = std::make_shared<ServerLocation>();
    sLoc->fromData(input);
    m_servers.push_back(sLoc);
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
