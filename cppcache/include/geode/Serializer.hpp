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

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <type_traits>

#include "geode_globals.hpp"
#include "DataOutput.hpp"
#include "DataInput.hpp"
#include "VectorT.hpp"
#include "HashMapT.hpp"
#include "HashSetT.hpp"
#include "GeodeTypeIds.hpp"

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
                        wchar_t value) {
  output.writeInt(static_cast<int16_t>(value));
}

inline void readObject(apache::geode::client::DataInput& input,
                       char16_t& value) {
  value = input.readInt16();
}

inline void writeObject(apache::geode::client::DataOutput& output,
                        const char* value, uint32_t length) {
  output.writeASCII(value, length);
}

template <typename TLen>
inline void readObject(apache::geode::client::DataInput& input, char*& value,
                       TLen& length) {
  uint16_t len;
  input.readASCII(&value, &len);
  length = len;
}

inline void writeObject(apache::geode::client::DataOutput& output,
                        const char* value) {
  output.writeASCII(value);
}

inline void readObject(apache::geode::client::DataInput& input, char*& value) {
  input.readASCII(&value);
}

inline void writeObject(apache::geode::client::DataOutput& output,
                        const wchar_t* value, uint32_t length) {
  output.writeUTF(value, length);
}

template <typename TLen>
inline void readObject(apache::geode::client::DataInput& input, wchar_t*& value,
                       TLen& length) {
  uint16_t len;
  input.readUTF(&value, &len);
  length = len;
}

inline void writeObject(apache::geode::client::DataOutput& output,
                        const wchar_t* value) {
  output.writeUTF(value);
}

inline void readObject(apache::geode::client::DataInput& input,
                       wchar_t*& value) {
  input.readUTF(&value);
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
  value = input.readObject<TObj>(true);
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

template <typename TObj, typename TLen>
inline void readObject(apache::geode::client::DataInput& input, TObj*& array,
                       TLen& len) {
  len = input.readArrayLen();
  if (len > 0) {
    GF_NEW(array, TObj[len]);
    TObj* startArray = array;
    TObj* endArray = array + len;
    while (startArray < endArray) {
      readObject(input, *startArray++);
    }
  } else {
    array = nullptr;
  }
}

template <typename TObj, typename TLen,
          typename std::enable_if<!std::is_base_of<Serializable, TObj>::value,
                                  Serializable>::type* = nullptr>
inline uint32_t objectSize(const TObj* array, TLen len) {
  return (uint32_t)(sizeof(TObj) * len);
}

template <typename TObj, typename TLen,
          typename std::enable_if<std::is_base_of<Serializable, TObj>::value,
                                  Serializable>::type* = nullptr>
inline uint32_t objectSize(const TObj* array, TLen len) {
  uint32_t size = 0;
  const TObj* endArray = array + len;
  while (array < endArray) {
    if (*array != nullptr) {
      size += (*array)->objectSize();
    }
    array++;
  }
  size += (uint32_t)(sizeof(TObj) * len);
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

inline uint32_t objectSize(const VectorOfCacheable& value) {
  size_t objectSize = 0;
  for (const auto& iter : value) {
    if (iter) {
      objectSize += iter->objectSize();
    }
  }
  objectSize += sizeof(CacheablePtr) * value.size();
  return static_cast<uint32_t>(objectSize);
}

template <typename TObj, typename _tail>
inline void readObject(apache::geode::client::DataInput& input,
                       std::vector<TObj, _tail>& value) {
  int32_t len = input.readArrayLen();
  if (len >= 0) {
    TObj obj;
    for (int32_t index = 0; index < len; index++) {
      readObject(input, obj);
      value.push_back(obj);
    }
  }
}

template <typename TKey, typename TValue, typename Hash, typename KeyEqual, typename Allocator>
inline void writeObject(
    apache::geode::client::DataOutput& output,
    const std::unordered_map<TKey, TValue, Hash, KeyEqual, Allocator>& value) {
  output.writeArrayLen(static_cast<int32_t>(value.size()));
  for (const auto& iter : value) {
    writeObject(output, iter.first);
    writeObject(output, iter.second);
  }
}

inline uint32_t objectSize(const HashMapOfCacheable& value) {
  uint32_t objectSize = 0;
  for (const auto& iter : value) {
    objectSize += iter.first->objectSize();
    if (iter.second) {
      objectSize += iter.second->objectSize();
    }
  }
  objectSize += static_cast<uint32_t>(
      (sizeof(CacheableKeyPtr) + sizeof(CacheablePtr)) * value.size());
  return objectSize;
}

template <typename TKey, typename TValue, typename Hash, typename KeyEqual, typename Allocator>
inline void readObject(apache::geode::client::DataInput& input,
                       std::unordered_map<TKey, TValue, Hash, KeyEqual, Allocator>& value) {
  int32_t len = input.readArrayLen();
  value.reserve(len);
  TKey key;
  TValue val;
  for (int32_t index = 0; index < len; index++) {
    readObject(input, key);
    readObject(input, val);
    value.emplace(key, val);
  }
}

template <typename TKey, typename Hash, typename KeyEqual, typename Allocator>
inline void writeObject(apache::geode::client::DataOutput& output,
                        const std::unordered_set<TKey, Hash, KeyEqual, Allocator>& value) {
  output.writeArrayLen(static_cast<int32_t>(value.size()));
  for (const auto& iter : value) {
    writeObject(output, iter);
  }
}

inline uint32_t objectSize(const HashSetOfCacheableKey& value) {
  uint32_t objectSize = 0;
  for (const auto& iter : value) {
    if (iter) {
      objectSize += iter->objectSize();
    }
  }
  objectSize += static_cast<uint32_t>(sizeof(CacheableKeyPtr) * value.size());
  return objectSize;
}

template <typename TKey, typename Hash, typename KeyEqual, typename Allocator>
inline void readObject(apache::geode::client::DataInput& input,
                       std::unordered_set<TKey, Hash, KeyEqual, Allocator>& value) {
  int32_t len = input.readArrayLen();
  if (len > 0) {
    TKey key;
    for (int32_t index = 0; index < len; index++) {
      readObject(input, key);
      value.insert(key);
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
