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

#include "LocatorListResponse.hpp"

#include <vector>

#include <geode/DataInput.hpp>

namespace apache {
namespace geode {
namespace client {

using internal::DSFid;

void LocatorListResponse::fromData(DataInput& input) {
  readList(input);
  m_isBalanced = input.readBoolean();
}

DSFid LocatorListResponse::getDSFID() const {
  return DSFid::LocatorListResponse;
}

void LocatorListResponse::readList(DataInput& input) {
  uint32_t size = input.readInt32();
  for (uint32_t i = 0; i < size; i++) {
    ServerLocation temp;
    temp.fromData(input);
    m_locators.push_back(temp);
  }
}

const std::vector<ServerLocation>& LocatorListResponse::getLocators() const {
  return m_locators;
}

bool LocatorListResponse::isBalanced() const { return m_isBalanced; }

std::shared_ptr<Serializable> LocatorListResponse::create() {
  return std::make_shared<LocatorListResponse>();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
