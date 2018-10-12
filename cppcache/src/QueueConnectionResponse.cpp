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

#include "QueueConnectionResponse.hpp"

#include <geode/DataInput.hpp>

#include "ServerLocation.hpp"

namespace apache {
namespace geode {
namespace client {

using internal::DSFid;

void QueueConnectionResponse::fromData(DataInput& input) {
  m_durableQueueFound = input.readBoolean();
  readList(input);
}

DSFid QueueConnectionResponse::getDSFID() const {
  return DSFid::QueueConnectionResponse;
}

void QueueConnectionResponse::readList(DataInput& input) {
  auto size = input.readInt32();
  for (decltype(size) i = 0; i < size; i++) {
    ServerLocation temp;
    temp.fromData(input);
    m_list.push_back(temp);
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
