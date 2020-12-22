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
#include "PositionKey.hpp"

#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>

namespace DataSerializableTest {

void PositionKey::toData(DataOutput& output) const {
  output.writeInt(positionId_);
}

void PositionKey::fromData(apache::geode::client::DataInput& input) {
  positionId_ = input.readInt64();
}

bool PositionKey::operator==(const CacheableKey& other) const {
  return positionId_ ==
         (static_cast<const PositionKey&>(other)).getPositionId();
}

int PositionKey::hashcode() const {
  int prime = 31;
  int result = prime * static_cast<int32_t>(positionId_);
  return result;
}

}  // namespace DataSerializableTest
