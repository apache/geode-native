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

#include <geode/Serializer.hpp>

namespace apache {
namespace geode {
namespace client {
namespace serializer {

// Read and write methods for various types

void writeObject(apache::geode::client::DataOutput& output, uint8_t value) {
  output.write(value);
}

void readObject(apache::geode::client::DataInput& input, uint8_t& value) {
  value = input.read();
}

void writeObject(apache::geode::client::DataOutput& output, int8_t value) {
  output.write(value);
}

void readObject(apache::geode::client::DataInput& input, int8_t& value) {
  value = input.read();
}

void writeObject(apache::geode::client::DataOutput& output,
                 const uint8_t* bytes, int32_t len) {
  output.writeBytes(bytes, len);
}

void readObject(apache::geode::client::DataInput& input, uint8_t*& bytes,
                int32_t& len) {
  input.readBytes(&bytes, &len);
}

void writeObject(apache::geode::client::DataOutput& output, const int8_t* bytes,
                 int32_t len) {
  output.writeBytes(bytes, len);
}

void readObject(apache::geode::client::DataInput& input, int8_t*& bytes,
                int32_t& len) {
  input.readBytes(&bytes, &len);
}

void writeObject(apache::geode::client::DataOutput& output, int16_t value) {
  output.writeInt(value);
}

void readObject(apache::geode::client::DataInput& input, int16_t& value) {
  value = input.readInt16();
}

void writeObject(apache::geode::client::DataOutput& output, int32_t value) {
  output.writeInt(value);
}

void readObject(apache::geode::client::DataInput& input, int32_t& value) {
  value = input.readInt32();
}

void writeObject(apache::geode::client::DataOutput& output, int64_t value) {
  output.writeInt(value);
}

void readObject(apache::geode::client::DataInput& input, int64_t& value) {
  value = input.readInt64();
}

void writeObject(apache::geode::client::DataOutput& output, uint16_t value) {
  output.writeInt(value);
}

void readObject(apache::geode::client::DataInput& input, uint16_t& value) {
  value = input.readInt16();
}

void writeObject(apache::geode::client::DataOutput& output, uint32_t value) {
  output.writeInt(value);
}

void readObject(apache::geode::client::DataInput& input, uint32_t& value) {
  value = input.readInt32();
}

void writeObject(apache::geode::client::DataOutput& output, uint64_t value) {
  output.writeInt(value);
}

void readObject(apache::geode::client::DataInput& input, uint64_t& value) {
  value = input.readInt64();
}

void writeObject(apache::geode::client::DataOutput& output, bool value) {
  output.writeBoolean(value);
}

void readObject(apache::geode::client::DataInput& input, bool& value) {
  value = input.readBoolean();
}

void readObject(apache::geode::client::DataInput& input,
                std::vector<bool>::reference value) {
  value = input.readBoolean();
}

void writeObject(apache::geode::client::DataOutput& output, double value) {
  output.writeDouble(value);
}

void readObject(apache::geode::client::DataInput& input, double& value) {
  value = input.readDouble();
}

void writeObject(apache::geode::client::DataOutput& output, float value) {
  output.writeFloat(value);
}

void readObject(apache::geode::client::DataInput& input, float& value) {
  value = input.readFloat();
}

void writeObject(apache::geode::client::DataOutput& output, char16_t value) {
  output.writeInt(static_cast<int16_t>(value));
}

void readObject(apache::geode::client::DataInput& input, char16_t& value) {
  value = input.readInt16();
}

void readObject(apache::geode::client::DataInput& input, std::string& value) {
  value = input.readString();
}

void writeObject(apache::geode::client::DataOutput& output,
                 const std::string& value) {
  output.writeString(value);
}

size_t objectSize(const std::vector<std::shared_ptr<Cacheable>>& value) {
  size_t objectSize = 0;
  for (const auto& iter : value) {
    if (iter) {
      objectSize += iter->objectSize();
    }
  }
  objectSize += sizeof(std::shared_ptr<Cacheable>) * value.size();
  return objectSize;
}

size_t objectSize(const HashMapOfCacheable& value) {
  auto objectSize = (sizeof(std::shared_ptr<CacheableKey>) +
                     sizeof(std::shared_ptr<Cacheable>)) *
                    value.size();
  for (const auto& iter : value) {
    objectSize += iter.first->objectSize();
    if (iter.second) {
      objectSize += iter.second->objectSize();
    }
  }
  return objectSize;
}

size_t objectSize(const HashSetOfCacheableKey& value) {
  auto objectSize = sizeof(std::shared_ptr<CacheableKey>) * value.size();
  for (const auto& iter : value) {
    if (iter) {
      objectSize += iter->objectSize();
    }
  }
  return objectSize;
}

template <>
bool zeroObject<bool>() {
  return false;
}

template <>
double zeroObject<double>() {
  return 0.0;
}

template <>
float zeroObject<float>() {
  return 0.0F;
}

}  // namespace serializer
}  // namespace client
}  // namespace geode
}  // namespace apache
