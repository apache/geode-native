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

#pragma once

#ifndef GEODE_SERIALIZER_H_
#define GEODE_SERIALIZER_H_

#include <memory>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "DataInput.hpp"
#include "DataOutput.hpp"
#include "internal/geode_globals.hpp"

namespace apache {
namespace geode {
namespace client {
namespace serializer {

// Read and write methods for various types

inline void writeObject(apache::geode::client::DataOutput& output,
                        uint8_t value) {
  output.write(value);
}

inline void readObject(apache::geode::client::DataInput& input,
                       uint8_t& value) {
  value = input.read();
}

inline void writeObject(apache::geode::client::DataOutput& output,
                        int8_t value) {
  output.write(value);
}

inline void readObject(apache::geode::client::DataInput& input, int8_t& value) {
  value = input.read();
}

inline void writeObject(apache::geode::client::DataOutput& output,
                        const uint8_t* bytes, int32_t len) {
  output.writeBytes(bytes, len);
}

inline void readObject(apache::geode::client::DataInput& input, uint8_t*& bytes,
                       int32_t& len) {
  input.readBytes(&bytes, &len);
}

inline void writeObject(apache::geode::client::DataOutput& output,
                        const int8_t* bytes, int32_t len) {
  output.writeBytes(bytes, len);
}

inline void readObject(apache::geode::client::DataInput& input, int8_t*& bytes,
                       int32_t& len) {
  input.readBytes(&bytes, &len);
}

inline void writeObject(apache::geode::client::DataOutput& output,
                        int16_t value) {
  output.writeInt(value);
}

inline void readObject(apache::geode::client::DataInput& input,
                       int16_t& value) {
  value = input.readInt16();
}

inline void writeObject(apache::geode::client::DataOutput& output,
                        int32_t value) {
  output.writeInt(value);
}

inline void readObject(apache::geode::client::DataInput& input,
                       int32_t& value) {
  value = input.readInt32();
}

inline void writeObject(apache::geode::client::DataOutput& output,
                        int64_t value) {
  output.writeInt(value);
}

inline void readObject(apache::geode::client::DataInput& input,
                       int64_t& value) {
  value = input.readInt64();
}

inline void writeObject(apache::geode::client::DataOutput& output,
                        uint16_t value) {
  output.writeInt(value);
}

inline void readObject(apache::geode::client::DataInput& input,
                       uint16_t& value) {
  value = input.readInt16();
}

inline void writeObject(apache::geode::client::DataOutput& output,
                        uint32_t value) {
  output.writeInt(value);
}

inline void readObject(apache::geode::client::DataInput& input,
                       uint32_t& value) {
  value = input.readInt32();
}

inline void writeObject(apache::geode::client::DataOutput& output,
                        uint64_t value) {
  output.writeInt(value);
}

inline void readObject(apache::geode::client::DataInput& input,
                       uint64_t& value) {
  value = input.readInt64();
}

inline void writeObject(apache::geode::client::DataOutput& output, bool value) {
  output.writeBoolean(value);
}

inline void readObject(apache::geode::client::DataInput& input, bool& value) {
  value = input.readBoolean();
}

inline void readObject(apache::geode::client::DataInput& input,
                       std::vector<bool>::reference value) {
  value = input.readBoolean();
}

inline void writeObject(apache::geode::client::DataOutput& output,
                        double value) {
  output.writeDouble(value);
}

inline void readObject(apache::geode::client::DataInput& input, double& value) {
  value = input.readDouble();
}

inline void writeObject(apache::geode::client::DataOutput& output,
                        float value) {
  output.writeFloat(value);
}

inline void readObject(apache::geode::client::DataInput& input, float& value) {
  value = input.readFloat();
}

inline void writeObject(apache::geode::client::DataOutput& output,
                        char16_t value) {
  output.writeInt(static_cast<int16_t>(value));
}

inline void readObject(apache::geode::client::DataInput& input,
                       char16_t& value) {
  value = input.readInt16();
}

inline void readObject(apache::geode::client::DataInput& input,
                       std::string& value) {
  value = input.readString();
}

inline void writeObject(apache::geode::client::DataOutput& output,
                        const std::string& value) {
  output.writeString(value);
}

template <typename TObj,
          typename std::enable_if<std::is_base_of<Serializable, TObj>::value,
                                  Serializable>::type* = nullptr>
inline void writeObject(apache::geode::client::DataOutput& output,
                        const std::shared_ptr<TObj>& value) {
  output.writeObject(value);
}

template <typename TObj,
          typename std::enable_if<std::is_base_of<Serializable, TObj>::value,
                                  Serializable>::type* = nullptr>
inline void readObject(apache::geode::client::DataInput& input,
                       std::shared_ptr<TObj>& value) {
  value = std::dynamic_pointer_cast<TObj>(input.readObject());
}

// For arrays

template <typename TObj, typename TLen>
inline void writeObject(apache::geode::client::DataOutput& output,
                        const TObj* array, TLen len) {
  if (array == nullptr) {
    output.write(static_cast<int8_t>(-1));
  } else {
    output.writeArrayLen(len);
    const TObj* endArray = array + len;
    while (array < endArray) {
      writeObject(output, *array++);
    }
  }
}

template <typename TObj>
inline void writeArrayObject(apache::geode::client::DataOutput& output,
                             const std::vector<TObj>& array) {
  output.writeArrayLen(static_cast<int32_t>(array.size()));
  for (auto&& obj : array) {
    writeObject(output, obj);
  }
}

template <typename TObj>
inline std::vector<TObj> readArrayObject(
    apache::geode::client::DataInput& input) {
  std::vector<TObj> array;
  int len = input.readArrayLength();
  if (len >= 0) {
    array.resize(len);
    for (auto&& obj : array) {
      readObject(input, obj);
    }
  }
  return array;
}

template <typename TObj, typename TLen>
inline void readObject(apache::geode::client::DataInput& input, TObj*& array,
                       TLen& len) {
  len = input.readArrayLength();
  if (len > 0) {
    _GEODE_NEW(array, TObj[len]);
    TObj* startArray = array;
    TObj* endArray = array + len;
    while (startArray < endArray) {
      readObject(input, *startArray++);
    }
  } else {
    array = nullptr;
  }
}

template <typename TObj,
          typename std::enable_if<!std::is_base_of<Serializable, TObj>::value,
                                  Serializable>::type* = nullptr>
inline size_t objectArraySize(const std::vector<TObj>& array) {
  return sizeof(TObj) * array.size();
}

template <typename TObj,
          typename std::enable_if<std::is_base_of<Serializable, TObj>::value,
                                  Serializable>::type* = nullptr>
inline size_t objectArraySize(const std::vector<TObj>& array) {
  size_t size = 0;
  for (auto obj : array) {
    size += obj.objectArraySize();
  }
  size += sizeof(TObj) * array.size();
  return size;
}

template <typename TObj, typename TLen,
          typename std::enable_if<!std::is_base_of<Serializable, TObj>::value,
                                  Serializable>::type* = nullptr>
inline size_t objectSize(const TObj*, TLen len) {
  return sizeof(TObj) * len;
}

template <typename TObj, typename TLen,
          typename std::enable_if<std::is_base_of<Serializable, TObj>::value,
                                  Serializable>::type* = nullptr>
inline size_t objectSize(const TObj* array, TLen len) {
  size_t size = 0;
  const TObj* endArray = array + len;
  while (array < endArray) {
    if (*array != nullptr) {
      size += (*array)->objectSize();
    }
    array++;
  }
  size += sizeof(TObj) * len;
  return size;
}

// For containers vector/hashmap/hashset

template <typename TObj, typename Allocator>
inline void writeObject(apache::geode::client::DataOutput& output,
                        const std::vector<TObj, Allocator>& value) {
  output.writeArrayLen(static_cast<int32_t>(value.size()));
  for (const auto& v : value) {
    writeObject(output, v);
  }
}

inline size_t objectSize(const std::vector<std::shared_ptr<Cacheable>>& value) {
  size_t objectSize = 0;
  for (const auto& iter : value) {
    if (iter) {
      objectSize += iter->objectSize();
    }
  }
  objectSize += sizeof(std::shared_ptr<Cacheable>) * value.size();
  return objectSize;
}

template <typename TObj, typename _tail>
inline void readObject(apache::geode::client::DataInput& input,
                       std::vector<TObj, _tail>& value) {
  int32_t len = input.readArrayLength();
  if (len >= 0) {
    TObj obj;
    for (int32_t index = 0; index < len; index++) {
      readObject(input, obj);
      value.push_back(obj);
    }
  }
}

template <typename TKey, typename TValue, typename Hash, typename KeyEqual,
          typename Allocator>
inline void writeObject(
    apache::geode::client::DataOutput& output,
    const std::unordered_map<TKey, TValue, Hash, KeyEqual, Allocator>& value) {
  output.writeArrayLen(static_cast<int32_t>(value.size()));
  for (const auto& iter : value) {
    writeObject(output, iter.first);
    writeObject(output, iter.second);
  }
}

inline size_t objectSize(const HashMapOfCacheable& value) {
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

template <typename TKey, typename TValue, typename Hash, typename KeyEqual,
          typename Allocator>
inline void readObject(
    apache::geode::client::DataInput& input,
    std::unordered_map<TKey, TValue, Hash, KeyEqual, Allocator>& value) {
  int32_t len = input.readArrayLength();
  value.reserve(len);
  std::shared_ptr<Serializable> key;
  std::shared_ptr<Serializable> val;
  for (int32_t index = 0; index < len; index++) {
    readObject(input, key);
    readObject(input, val);
    value.emplace(
        std::dynamic_pointer_cast<typename TKey::element_type>(key),
        std::dynamic_pointer_cast<typename TValue::element_type>(val));
  }
}

template <typename TKey, typename Hash, typename KeyEqual, typename Allocator>
inline void writeObject(
    apache::geode::client::DataOutput& output,
    const std::unordered_set<TKey, Hash, KeyEqual, Allocator>& value) {
  output.writeArrayLen(static_cast<int32_t>(value.size()));
  for (const auto& iter : value) {
    writeObject(output, iter);
  }
}

inline size_t objectSize(const HashSetOfCacheableKey& value) {
  auto objectSize = sizeof(std::shared_ptr<CacheableKey>) * value.size();
  for (const auto& iter : value) {
    if (iter) {
      objectSize += iter->objectSize();
    }
  }
  return objectSize;
}

template <typename TKey, typename Hash, typename KeyEqual, typename Allocator>
inline void readObject(
    apache::geode::client::DataInput& input,
    std::unordered_set<TKey, Hash, KeyEqual, Allocator>& value) {
  int32_t len = input.readArrayLength();
  if (len > 0) {
    std::shared_ptr<Serializable> key;
    for (int32_t index = 0; index < len; index++) {
      readObject(input, key);
      value.insert(std::dynamic_pointer_cast<typename TKey::element_type>(key));
    }
  }
}

// Default value for builtin types

template <typename TObj>
inline TObj zeroObject() {
  return 0;
}

template <>
inline bool zeroObject<bool>() {
  return false;
}

template <>
inline double zeroObject<double>() {
  return 0.0;
}

template <>
inline float zeroObject<float>() {
  return 0.0F;
}
}  // namespace serializer
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_SERIALIZER_H_
