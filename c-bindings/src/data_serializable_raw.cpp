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

#include <iterator>

// C++ client public headers
#include "geode/DataInput.hpp"
#include "geode/DataOutput.hpp"
#include "geode/internal/DataSerializablePrimitive.hpp"

// C client public headers
#include "data_serializable_raw.hpp"

// C client private headers

using apache::geode::client::DataInput;
using apache::geode::client::DataOutput;

namespace apache {
namespace geode {
namespace client {
namespace internal {

DataSerializableRaw::DataSerializableRaw(const int8_t* data, size_t size) {
  bytes_.reserve(size);
  std::copy(data, data + size, std::back_inserter(bytes_));
}

std::shared_ptr<DataSerializableRaw> DataSerializableRaw::create(
    const int8_t* data, size_t size) {
  return std::make_shared<DataSerializableRaw>(data, size);
}

void DataSerializableRaw::toData(DataOutput& dataOutput) const {
  dataOutput.writeBytesOnly(bytes_.data() + dsCodeSize_,
                            bytes_.size() - dsCodeSize_);
}

void DataSerializableRaw::fromData(DataInput& dataInput) {
  dataInput.readBytesOnly(bytes_.data() + dsCodeSize_,
                          bytes_.size() - dsCodeSize_);
}

DSCode DataSerializableRaw::getDsCode() const {
  return static_cast<DSCode>(bytes_[0]);
}

bool DataSerializableRaw::operator==(const CacheableKey& other) const {
  if (auto otherKey = dynamic_cast<const DataSerializableRaw*>(&other)) {
    return bytes_ == otherKey->bytes_;
  }

  return false;
}

int32_t DataSerializableRaw::hashcode() const {
  if (hashCode_ == 0) {
    hashCode_ = internal::geode_hash<std::vector<int8_t>>{}(bytes_);
  }
  return 0;
}

}  // namespace internal
}  // namespace client
}  // namespace geode
}  // namespace apache