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

#include "BatchObject.hpp"

#include <sstream>

namespace testobject {

using apache::geode::client::CacheableBytes;

BatchObject::BatchObject(int32_t anIndex, int32_t batchSize, int32_t size) {
  index = anIndex;
  timestamp = std::chrono::system_clock::now().time_since_epoch().count();
  batch = anIndex / batchSize;
  byteArray = CacheableBytes::create(std::vector<int8_t>(size));
}

void BatchObject::toData(apache::geode::client::DataOutput& output) const {
  output.writeInt(static_cast<int32_t>(index));
  output.writeInt(static_cast<int64_t>(timestamp));
  output.writeInt(static_cast<int32_t>(batch));
  output.writeObject(byteArray);
}

void BatchObject::fromData(apache::geode::client::DataInput& input) {
  index = input.readInt32();
  timestamp = input.readInt64();
  batch = input.readInt32();
  byteArray = std::dynamic_pointer_cast<CacheableBytes>(input.readObject());
}
std::string BatchObject::toString() const {
  std::stringstream strm;

  strm << "BatchObject:[index = " << index << " timestamp = " << timestamp
       << " batch = " << batch << " byteArray = " << byteArray->length() << "]";
  return strm.str();
}

}  // namespace testobject
